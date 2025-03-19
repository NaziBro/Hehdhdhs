#include "Network.h"
#include "Script.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include <math.h>
#include "Zip/unzip.h"
#include <thread>
#include "Raknet/MessageIdentifiers.h"
#include "Raknet/RakPeerInterface.h"
#include "Raknet/RakNetTypes.h"
#include "Raknet/BitStream.h"
#include "Raknet/SuperFastHash.h"
#include "HackSDK.h"

class RaknetClient {
public:

	enum ChatMessagesDefine {
		VSClientMsg = ID_USER_PACKET_ENUM
	};

	bool connect(std::string ip, int port) {
		client = RakNet::RakPeerInterface::GetInstance();
		if (client == nullptr) {
			return false;
		}
		RakNet::SocketDescriptor sd;
		int start_up = client->Startup(1, &sd, 1);
		if (start_up > 0) {
			return false;
		}
		const char* password = "VSClient";

		bool rs = (client->Connect(ip.c_str(), port, password, strlen(password)) == RakNet::CONNECTION_ATTEMPT_STARTED);
		if (rs == false) {
			return false;
		}

		isConnected = true;

		return true;
	}

	void disconnect(void) {
		client->Shutdown(300);
		RakNet::RakPeerInterface::DestroyInstance(client);
		isConnected = false;
	}

	void sendJsonTo(Json::Value const& value) {
		std::string str = value.toStyledString();
		for (int i = 0; i != str.size(); i = i + 1) {
			str[i] = (str[i] ^ 10);
		}
		uint32_t len = str.size() + 1;
		char* buffer = new char[len];
		buffer[0] = VSClientMsg;
		memcpy(buffer + 1, str.c_str(), str.size());
		client->Send(buffer, len, HIGH_PRIORITY, RELIABLE_ORDERED, 0, RakNet::UNASSIGNED_SYSTEM_ADDRESS, true);
		delete[] buffer;
	}
	Json::Value getJsonFromVSPacket(RakNet::Packet* p) {

		std::stringstream ss;
		for (int i = 1; i < p->length; i = i + 1) {
			ss << p->data[i];
		}

		std::string str = ss.str();
		for (int i = 0; i != str.size(); i = i + 1) {
			str[i] = (str[i] ^ 10);
		}
		
		Json::Reader reader;
		Json::Value value;
		reader.parse(str, value);
		return value;
	}

	std::vector<Json::Value> recv() {
		RakNet::Packet* net_packet;
		std::vector<Json::Value> res;
		for (net_packet = client->Receive(); net_packet; client->DeallocatePacket(net_packet), net_packet = client->Receive())
		{
			switch (net_packet->data[0])
			{
			case VSClientMsg:
			{
				Json::Value v = getJsonFromVSPacket(net_packet);
				res.push_back(v);
				break;
			}
			case ID_NO_FREE_INCOMING_CONNECTIONS: {
				isConnected = false;
				break;
			}
			case ID_DISCONNECTION_NOTIFICATION: {
				isConnected = false;
				break;
			}
			case ID_CONNECTION_ATTEMPT_FAILED: {
				isConnected = false;
				break;
			}
			case ID_CONNECTION_LOST: {
				isConnected = false;
				break;
			}
			}


		}
		return res;
	}
	bool isConnected = false;
	RakNet::RakPeerInterface* client;
};


void Network::login(Json::Value value) {

	std::string ServerState = value["ServerState"].asString();
	if (ServerState != "Open") {
		crash("Server is close");
	}
	if (Version != value["Version"].asString()) {
		crash("Version not correct!");
	}
	
	moduleManager->canUseCommand = value["Commands"].asBool();

	for (int i = 0; i < value["GameID"].size(); ++i) {
		GameIDs.push_back(value["GameID"][i].asString());
	}

	if (value["CheckUserID"].asBool() == true) {
		bool flag = false;
		for (auto i : value["UserID"]) {
			if (i.asString() == Android::getHWId()) {
				flag = true;
				break;
			}
		}
		if (flag == false) {
			remove("/sdcard/FBID.txt");
			std::stringstream id;
			id << "您的ID为:" << Android::getHWId();
			Utils::WriteStringToFile("/sdcard/FBID.txt", id.str());
			crash("No ID!");
		}
	}

	needToCheckActor = value["CheckPlayerName"].asBool();
	
	ad.init(value["AD"]);

	moduleManager->getModule<MenuController>()->startUI(value["MainUIIconUrl"].asString());

	isLogined = true;


	if (value["UseDataPack"].asBool() == true) {
		dataDir = value["dataDir"].asString();
		dataPackZip = dataDir + + "/"+packName;
		unPackPath = dataDir + +"/UnPacked";
		PackHash = value["DataPackHash"].asUInt();
		if (opendir(dataDir.c_str()) == 0) {
			Utils::folder_mkdirs(dataDir.c_str());
			if (opendir(dataDir.c_str()) == 0) {
				Network::crash("sdcard error");
			}
		}

		bool shouldDownloadResource = false;

		if (access(dataPackZip.c_str(), F_OK) == 0) {
			if (SuperFastHashFile(dataPackZip.c_str()) != PackHash) {
				shouldDownloadResource = true;
			}
			else {
				unZipData();
			}
		}
		else {
			shouldDownloadResource = true;
		}
		if (shouldDownloadResource) {

			std::thread([=]() {
				Utils::download(ip.c_str(),packName, dataPackZip.c_str());
				if (SuperFastHashFile(dataPackZip.c_str()) != PackHash) {
					crash("download error");
				}
				unZipData();
			}).detach();

		}
	}

	
}

void Network::applyMessage(Json::Value value) {

#ifndef LocalEdition
	std::string action = value["Action"].asString();
	if (action == "Login") {
		if(isLogined == false){
			login(value);
		}
	}
	else if (action == "Say") {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		if (mGameData.getNowMinecraftGame()->getPrimaryClientInstance() == nullptr)return;
		if (mGameData.getNowMinecraftGame()->getPrimaryGuiData() == nullptr)return;

		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		
		HackSDK::sendOriginalPacket(Text::createChat(lp->getName(), value["Text"].asString(), "1", "1"));
		NewChat(value["Text"].asString());
	}
	else if (action == "Display") {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		if (mGameData.getNowMinecraftGame()->getPrimaryClientInstance() == nullptr)return;
		if (mGameData.getNowMinecraftGame()->getPrimaryGuiData() == nullptr)return;
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(value["Text"].asString(), "");
	}
	else if (action == "ListenChat") {
		if (value["State"].asString() == "true") {
			reportChat = true;
		}
		else if (value["State"].asString() == "false") {
			reportChat = false;
		}
	}
	else if (action == "Kick") {
		crash("Server kick!");
	}
	else if (action == "VSChat") {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		if (mGameData.getNowMinecraftGame()->getPrimaryClientInstance() == nullptr)return;
		if (mGameData.getNowMinecraftGame()->getPrimaryGuiData() == nullptr)return;
		std::string PlayerID = value["PlayerID"].asString();
		std::string Chat = value["Message"].asString();
		std::stringstream sys_message;
		if (value["Msg"].asBool() == false) {
			if (value["ServerIP"].asString() == serverIP)return;
			sys_message << "[VS用户<" << PlayerID << ">]:" << Chat;
		}
		else {
			sys_message << "[VS用户<" << PlayerID << "> 悄悄对你说]:" << Chat;
		}
		mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(sys_message.str(), "");
	}
	else if (action == "LoadLua") {
		std::string LuaCode = value["Code"].asString();
		std::string LuaID = value["ID"].asString();
		if (moduleManager->getModule<Script>())moduleManager->getModule<Script>()->runString(LuaCode, LuaID);
	}
	else if (action == "ExitLua") {
		std::string LuaID = value["ID"].asString();
		if (moduleManager->getModule<Script>())moduleManager->getModule<Script>()->exit(atoi(LuaID.c_str()));
	}
#endif
}

void Network::unZipData()
{
	Utils::DeleteFile(unPackPath.c_str());
	HZIP hz = OpenZip(dataPackZip.c_str(), NULL);
	SetUnzipBaseDir(hz,unPackPath.c_str());
	ZIPENTRY ze;
	GetZipItem(hz, -1, &ze);
	int numitems = ze.index;
	for (int zi = 0; zi < numitems; zi++)
	{
		ZIPENTRY ze;
		GetZipItem(hz, zi, &ze);
		UnzipItem(hz, zi, ze.name);
	}
	CloseZip(hz);
	
}

Network::Network()
{

	ModuleType = "Other";
	UIType = 1;
	
}

Network::~Network()
{
	delete client;
}

void Network::initViews()
{
	Android::EditText* NetworkMod_id = mAndroid->newEditText();
	UIUtils::updateEditTextData(NetworkMod_id, "id");

	Android::TextView *NetworkMod_get = mAndroid->newTextView();
	UIUtils::updateTextViewData(NetworkMod_get, "GetMod", "#FF0000", 19);
	NetworkMod_get->setOnClickListener([=](Android::View*) {
		mAndroid->Toast("Try get mod " +NetworkMod_id->text);
		tryGetMod(NetworkMod_id->text);
	});


	SecondWindowList.push_back(NetworkMod_id);
	SecondWindowList.push_back(NetworkMod_get);
}

void Network::UiInit()
{
	client = new RaknetClient;
	
#ifdef LocalEdition
	moduleManager->getModule<MenuController>()->startUI("");
	moduleManager->canUseCommand = true;
#endif
	std::thread([=]() {
		if (client->connect(ip, port) == false) {
			crash("Can not connect to the server!");
		}

		Json::Value v;
		v["Action"] = "Login";
		client->sendJsonTo(v);

		while (true) {
			sleep(1);
			std::vector<Json::Value> res = client->recv();
			if (client->isConnected == false) {
				crash("Disconnect from the server!");
			}
			if (res.size() == 0) {
				client->sendJsonTo(v);

				continue;
			}
			else {
				for (int i = 0; i < res.size(); i = i + 1) {
					applyMessage(res[i]);
				}
				break;
			}
		}
		std::thread([=]() {
			while (true) {
				usleep(30 * 1000);
				std::vector<Json::Value> res = client->recv();
				if (client->isConnected == false) {
					crash("Disconnect from the server!");
				}
				if (res.size() == 0) {
					continue;
				}
				else {
					for (int i = 0; i < res.size(); i = i + 1) {
						applyMessage(res[i]);
					}
				}
			}
		}).detach();
	}).detach();
}

void Network::NewInfo(std::string id, std::string ip) {
	serverIP = ip;

	Json::Value v;
	v["Action"] = "PlayerInfo";
	v["ServerIP"] = ip;
	v["PlayerID"] = id;
	client->sendJsonTo(v);
}



void Network::ReportChat(std::string chat) {
	if (reportChat == false)return;
	Json::Value v;
	v["Action"] = "ChatMessage";
	v["Message"] = chat;
	client->sendJsonTo(v);
}

void Network::VSChat(std::string chat) {
	Json::Value v;
	v["Action"] = "VSChat";
	v["Message"] = chat;
	v["Msg"] = false;
	client->sendJsonTo(v);
}

void Network::NewChat(std::string chat) {
	ReportChat(chat);
	VSChat(chat);
}

void Network::VSChat(std::string name, std::string message) {
	Json::Value v;
	v["Action"] = "VSChat";
	v["TargetName"] = name;
	v["Message"] = message;
	v["Msg"] = true;
	client->sendJsonTo(v);
}

void Network::tryGetMod(std::string id) {
	Json::Value v;
	v["Action"] = "GetMod";
	v["ModID"] = id;
	client->sendJsonTo(v);
}

void Network::sendMessageToServer(std::string const& message)
{
	Json::Value v;
	v["Action"] = "ClientMessage";
	v["Message"] = message;
	client->sendJsonTo(v);
}

void Network::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".VSMsg") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		std::string message;
		for (auto it = cmd->begin() + 1; it != cmd->end(); it = it + 1) {
			message = message + *it;
			if (it != cmd->end() - 1) {
				message = message + " ";
			}
		}
		VSChat(mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getName(), message);
	}
}

void Network::OnSendChat(std::string const& msg)
{
	NewChat(msg);
}

void Network::OnTick(Actor* act)
{

	if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer() != act)return;
	ad.tick();

	if (isLogined == false) {
		crash("Not login!");
	}

	if (needToCheckActor == false)return;
	
	bool v = false;
	for (auto it = GameIDs.begin(); it != GameIDs.end(); it = it + 1) {
		if (*it == mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getName()) {
			v = true;
			break;
		}
	}
	if (v == false) {
		crash("GameID not correct!");
	}
}

void Network::OnSendData(std::string* data)
{
	Login loginPacket;
	if (loginPacket.readFromString(data)) {
		NewInfo(loginPacket.getPlayerName(), loginPacket.getServerIP());
	}
}

const char* Network::GetName()
{
	return "Network";
}

bool Network::showChat(Text* packet)
{
	if (ad.sendAD == true && ad.showAD == false) {
		if (packet->source == mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getName()) {
			return false;
		}
	}
	return true;
}


void Network::AD::init(Json::Value v)
{
	sendAD = v["sendAD"].asBool();
	ADTime = v["ADTime"].asInt();
	showAD = v["showAD"].asBool();
	ADText = v["ADText"].asString();
}

void Network::AD::tick()
{
	if (sendAD == false) {
		return;
	}
	ADTick = ADTick + 1;
	if (ADTick > ADTime * 20) {
		ADTick = 0;

		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();

		HackSDK::sendOriginalPacket(Text::createChat(lp->getName(), ADText, "1234567", "666"));
	}
}
