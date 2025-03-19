#include "ModuleManager.h"
#include "Android.h"
#include "Utils.hpp"
#include  "Killaura.h"
#include "ForceOpenSign.h"
#include "ActorEventManager.h"
#include "PyManager.h"
#include  "Script.h"
#include  "Menu.h"
#include "Teleport.h"
#include  "JetPack.h"
#include  "HackSDK.h"
#include  "Network.h"
#include  "Chat.h"
#include  "SkinManager.h"
#include "AC.h"
#include "Bhop.h"
#include "NightVision.h"
#include "ClickDestroy.h"
#include "PluginInterface.h"
#include "Music.h"
#include "Enchant.h"
#include  "AutoDestroy.h"
#include  "Debug.h"
#include "ResourceManager.h"
#include  "Destroy.h"
#include  "AntiVoid.h"
#include "Server.h"
#include "Netease.h"
#include "Hitbox.h"
#include "NoKnockback.h"
#include "Patch.h"
#include "Pos.h"
#include "Water.h"
#include "FastBuilder.h"
#include "ItemTool.h"
#include "Scaffold.h"
#include "ClientPacketManager.h"
#include "ShowCommand.h"
#include "FreeCam.h"
#include "Xray.h"
#include "AutoAim.h"
#include "ESP.h"
#include "World.h"
#include "MCPE/Listener.h"

void ModuleManager::comandResultMessage(std::string const& message)
{

	MCHook::noMessageThisTime = true;
	mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage(message, "");
}


ModuleManager* moduleManager = new ModuleManager;

void EventListener(Json::Value const& s) {
	moduleManager->handleMCEvent(s);
}

void ModuleManager::initListener()
{
	Listener::addFunc(EventListener);
}

void ModuleManager::initModules()
{
	loader = new ConfigLoader(this);
	
	ModuleList.push_back(std::shared_ptr<Module>(new Patch()));
	ModuleList.push_back(std::shared_ptr<Module>(new Scaffold()));
	ModuleList.push_back(std::shared_ptr<Module>(new Killaura()));
	ModuleList.push_back(std::shared_ptr<Module>(new Bhop()));
	ModuleList.push_back(std::shared_ptr<Module>(new ESP()));
	ModuleList.push_back(std::shared_ptr<Module>(new AutoAim()));
	ModuleList.push_back(std::shared_ptr<Module>(new AutoDrestroy()));
	ModuleList.push_back(std::shared_ptr<Module>(new ClickDestroy()));
	ModuleList.push_back(std::shared_ptr<Module>(new ForceOpenSign()));
	ModuleList.push_back(std::shared_ptr<Module>(new ShowCommand()));
	ModuleList.push_back(std::shared_ptr<Module>(new Water()));
	ModuleList.push_back(std::shared_ptr<Module>(new Netease()));
	ModuleList.push_back(std::shared_ptr<Module>(new FreeCam()));
	ModuleList.push_back(std::shared_ptr<Module>(new World()));
	ModuleList.push_back(std::shared_ptr<Module>(new ActorEventManager()));
	ModuleList.push_back(std::shared_ptr<Module>(new Server()));
	ModuleList.push_back(std::shared_ptr<Module>(new Debug()));
	ModuleList.push_back(std::shared_ptr<Module>(new MenuController()));
	ModuleList.push_back(std::shared_ptr<Module>(new Script()));
	ModuleList.push_back(std::shared_ptr<Module>(new Xray()));
	ModuleList.push_back(std::shared_ptr<Module>(new AntiVoid()));
	ModuleList.push_back(std::shared_ptr<Module>(new Teleport()));
	ModuleList.push_back(std::shared_ptr<Module>(new JetPack()));
	ModuleList.push_back(std::shared_ptr<Module>(new HackSDK()));
	ModuleList.push_back(std::shared_ptr<Module>(new Network()));
	ModuleList.push_back(std::shared_ptr<Module>(new Chat()));
	ModuleList.push_back(std::shared_ptr<Module>(new SkinManager()));
	ModuleList.push_back(std::shared_ptr<Module>(new AC()));
	ModuleList.push_back(std::shared_ptr<Module>(new Enchant()));
	ModuleList.push_back(std::shared_ptr<Module>(new Destroy()));
	ModuleList.push_back(std::shared_ptr<Module>(new Hitbox()));
	ModuleList.push_back(std::shared_ptr<Module>(new NightVision()));
	ModuleList.push_back(std::shared_ptr<Module>(new NoKnockback()));
	ModuleList.push_back(std::shared_ptr<Module>(new Pos()));
	ModuleList.push_back(std::shared_ptr<Module>(new ItemTool()));
	ModuleList.push_back(std::shared_ptr<Module>(new PluginHelper()));
	ModuleList.push_back(std::shared_ptr<Module>(new ClientPacketManager()));
	ModuleList.push_back(std::shared_ptr<Module>(new ResourceManager()));
	ModuleList.push_back(std::shared_ptr<Module>(new Music()));
	ModuleList.push_back(std::shared_ptr<Module>(new FastBuilder()));
	ModuleList.push_back(std::shared_ptr<Module>(new PyManager()));
	
	loader->load();
}

void ModuleManager::jniOnLoad(JavaVM* vm)
{
	mAndroid = new Android(vm);
	initUI();
}

void ModuleManager::executeModuleCommand(std::string const& s)
{
	std::vector<std::string> res;
	std::string result;
	std::stringstream input(s);
	while (input >> result)res.push_back(result);

	if (res[0] == ".SaveConfig") {
		comandResultMessage("[HackCMD]Config saved!");
		loader->save();
		return;
	}

	if (canUseCommand) {
		std::string CMDStr = s;
		const char* CMDHead = ".";
		if (CMDStr.compare(0, strlen(CMDHead), CMDHead) == 0) {
			executedCMD = false;
			for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
				(*it)->OnCmd(CMDStr);
			}

			for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
				(*it)->OnCmd(&res);
			}

			if (executedCMD == true) {
				comandResultMessage("[HackCMD]Command executed success!");

			}
			else {
				comandResultMessage("[HackCMD]Command executed error!");
			}
		}
		else {
			for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
				(*it)->OnSendChat(CMDStr);
			}
		}
	}
	else {
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->OnSendChat(s);
		}
	}
}

void ModuleManager::initUI()
{
	for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
		(*it)->UiInit();
	}
}

void ModuleManager::handleMCEvent(Json::Value const& value)
{

	if (value.isMember("MCEvent") && value["MCEvent"] == "Init") {
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->MinecraftInit();
		}
	}
	else if (value.isMember("MCEvent") && value["MCEvent"] == "BeforeHook") {
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->BeforeMCHook();
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "Actor::normalTick" && value["MCCall"]["State"] == "BeforeCall") {
		Actor* object = reinterpret_cast<Actor*>(value["MCCall"]["Args"]["object"].asInt64());
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->OnTick(object);
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "ClientInstanceScreenModel::sendChatMessage" && value["MCCall"]["State"] == "BeforeCall") {
		executeModuleCommand(value["MCCall"]["Args"]["ChatMessage"].asString());
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "MinecraftGame::_update" && value["MCCall"]["State"] == "BeforeCall") {
		MinecraftGame* object = reinterpret_cast<MinecraftGame*>(value["MCCall"]["Args"]["object"].asInt64());
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->OnMCMainThreadTick(object);
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "LoopbackPacketSender::sendToServer") {
		Packet* networkPacketPtr = reinterpret_cast<Packet*>(value["MCCall"]["Args"]["data"].asInt64());
		if (value["MCCall"]["State"] == "BeforeCall") {
			for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
				(*it)->OnSendPacket(networkPacketPtr);
			}
		}
		else {
			for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
				(*it)->AfterPacket(networkPacketPtr);
			}
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "ScreenView::setupAndRender" && value["MCCall"]["State"] == "BeforeCall") {
		MinecraftUIRenderContext* ctx = reinterpret_cast<MinecraftUIRenderContext*>(value["MCCall"]["Args"]["ctxPtr"].asInt64());
		if (ctx->checkVTable()) {
			for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
				(*it)->OnRender(ctx);
			}
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "GameMode::attack" && value["MCCall"]["State"] == "BeforeCall") {
		GameMode* object = reinterpret_cast<GameMode*>(value["MCCall"]["Args"]["object"].asInt64());
		Actor* target = reinterpret_cast<Actor*>(value["MCCall"]["Args"]["target"].asInt64());
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->OnAttack(object, target);
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "GameMode::buildBlock" && value["MCCall"]["State"] == "BeforeCall") {
		GameMode* object = reinterpret_cast<GameMode*>(value["MCCall"]["Args"]["object"].asInt64());
		BlockPos& buildPos = *reinterpret_cast<BlockPos*>(value["MCCall"]["Args"]["buildPos"].asInt64());
		int8_t rot = (int8_t)(value["MCCall"]["Args"]["buildRot"].asUInt());
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->OnBuild(object, buildPos, rot);
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "LevelRenderer::renderLevel" && value["MCCall"]["State"] == "AfterCall") {
		ScreenContext* ctx = reinterpret_cast<ScreenContext*>(value["MCCall"]["Args"]["ctxPtr"].asInt64());
		for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
			(*it)->OnRender3D(ctx);
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "NetworkHandler::Connection::receivePacket" && value["MCCall"]["State"] == "AfterCall") {
		NetworkHandler::Connection* conn = reinterpret_cast<NetworkHandler::Connection*>(value["MCCall"]["Args"]["object"].asInt64());
		if (value["MCCall"]["Result"].asInt() == 0) {
			auto networkHandler = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getPacketSender()->handler;
			if (conn == networkHandler->getConnection(networkHandler->getServerNetworkIdentifer())) {
				std::string* data = reinterpret_cast<std::string*>(value["MCCall"]["Args"]["data"].asInt64());
				for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
					(*it)->OnReceiveData(data);
				}
			}
		}
	}
	else if (value.isMember("MCCall") && value["MCCall"]["Type"] == "NetworkHandler::_sendInternal") {
		NetworkHandler* handler = reinterpret_cast<NetworkHandler*>(value["MCCall"]["Args"]["object"].asInt64());
		
		auto clientHandler = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getPacketSender()->handler;
		if (clientHandler == handler) {
			std::string* data = reinterpret_cast<std::string*>(value["MCCall"]["Args"]["data"].asInt64());
			if (value["MCCall"]["State"] == "BeforeCall") {
				for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
					(*it)->OnSendData(data);
				}
			}
			else if (value["MCCall"]["State"] == "AfterCall") {
				for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
					(*it)->AfterSendData(data);
				}
			}
			Login loginPacket;
			if (loginPacket.readFromString(data)) {
				for (auto it = ModuleList.begin(); it != ModuleList.end(); it = it + 1) {
					(*it)->OnNewGame(loginPacket.getServerIP());
				}
			}
		}
	}
}

ModuleManager::ModuleManager()
{
	initListener();
	initModules();
}


const char* Module::GetName()
{
	return "DefaultName";
}

void Module::OnTick(Actor* act)
{
}

void Module::OnSendPacket(Packet* packet)
{
}

void Module::AfterPacket(Packet* packet)
{
}

void Module::OnRender(MinecraftUIRenderContext* ctx)
{
}

void Module::OnRender3D(ScreenContext* ctx)
{
}

void Module::OnCmd(std::string const& cmd)
{
}

void Module::OnCmd(std::vector<std::string>* cmd)
{
}

void Module::OnAttack(GameMode* object, Actor* act)
{
}

void Module::OnBuild(GameMode* object, BlockPos const& pos, int rot)
{
}

void Module::OnNewGame(std::string const& ip)
{
}

void Module::OnSendChat(std::string const& msg)
{
}

void Module::OnMCMainThreadTick(MinecraftGame* mcgame)
{
}

void Module::OnReceiveData(std::string* data)
{
}

void Module::OnSendData(std::string* data)
{
}

void Module::AfterSendData(std::string* data)
{
}

void Module::BeforeMCHook()
{
}

void Module::UiInit()
{
}

void Module::loadConfig(Json::Value const& config)
{
	if (config.isMember(GetName())) {
		Json::Value subConf = config[GetName()];
		Json::Value::Members mem = subConf.getMemberNames();
		for (auto it = mem.begin(); it != mem.end(); it++) {
			auto  t = subConf[*it].type();
			if (getConfigObject(*it)) {
				if (t == Json::ValueType::intValue || t == Json::ValueType::uintValue) {
					*(int*)getConfigObject(*it)->ptr = subConf[*it].asInt();
				}
				else if (t == Json::ValueType::realValue) {
					*(float*)getConfigObject(*it)->ptr = subConf[*it].asFloat();
				}
				else if (t == Json::ValueType::booleanValue) {
					*(bool*)getConfigObject(*it)->ptr = subConf[*it].asBool();
				}
				else if (t == Json::ValueType::stringValue) {
					*(std::string*)getConfigObject(*it)->ptr = subConf[*it].asString();
				}
			}
		}
	}
}

void Module::saveConfig(Json::Value& config)
{
	for (auto i : configList) {
		if (i->moduleName == GetName()) {
			if (i->type == typeid(int).name() || i->type == typeid(uint32_t).name()) {
				config[GetName()][i->configName] = *(int*)i->ptr;
			}
			else if (i->type == typeid(float).name()) {
				config[GetName()][i->configName] = *(float*)i->ptr;
			}
			else if (i->type == typeid(bool).name()) {
				config[GetName()][i->configName] = *(bool*)i->ptr;
			}
			else if (i->type == typeid(std::string).name()) {
				config[GetName()][i->configName] = *(std::string*)i->ptr;
			}
		}
	}
}

ConfigObject* Module::getConfigObject(std::string const& name)
{
	for (auto i : configList) {
		if (i->configName == name) {
			return i;
		}
	}
	logMessage("can not get config object " + name);
	return nullptr;
}

void Module::logMessage(std::string const& message)
{
	const char* path = "/sdcard/ClientLog.txt";
	std::stringstream os;
	struct tm* t;
	struct timeval tv;
	struct timezone tz;
	gettimeofday(&tv, &tz);
	t = localtime(&tv.tv_sec);
	os << "<" << GetName() << ">" << "<" << 1900 + t->tm_year << ":" << t->tm_mon << ":" << t->tm_mday << ":" << t->tm_hour << ":" << t->tm_min << ":" << t->tm_sec << ">" << message;
	Utils::WriteStringToFile(path, os.str());
}

void Module::MinecraftInit()
{
}

ConfigObject::ConfigObject(void* val, std::string name, Module* m, std::string typeName)
{
	ptr = val;
	configName = name;
	type = typeName;
	moduleName = m->GetName();
}

ConfigLoader::ConfigLoader(ModuleManager* m)
{
	manager = m;
}

std::string ConfigLoader::getPath()
{
	return "/sdcard/moduleConfig.txt";
}

void ConfigLoader::load()
{
	Json::Reader reader;
	Json::Value value;
	reader.parse(Utils::readFileIntoString(getPath().c_str()), value);
	for (auto i : manager->ModuleList) {
		i->loadConfig(value);
	}
}

void ConfigLoader::save()
{
	Json::Value value;
	for (auto i : manager->ModuleList) {
		i->saveConfig(value);
	}
	Utils::DeleteFile(getPath().c_str());
	Utils::WriteStringToFile(getPath().c_str(), value.toStyledString());
}
