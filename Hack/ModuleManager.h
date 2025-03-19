#pragma once

#include <string>
#include <jni.h>
#include "json/json.h"
#include "MCPE/MCHook.h"
#include "MCPE/GameData.h"

#define LocalEdition

class Module;
class ModuleManager;

class ConfigObject {
public:
	void* ptr;
	std::string configName;
	std::string type;
	std::string moduleName;
	ConfigObject(void* val, std::string name, Module* m, std::string typeName);
};


class Module {
public:
	std::vector<ConfigObject*> configList;

	template <typename TRet>
	void addConfig(TRet* object, std::string const& name) {
		ConfigObject* conf = new ConfigObject(object, name, this, typeid(TRet).name());
		configList.push_back(conf);
	}

	void loadConfig(Json::Value const& config);
	void saveConfig(Json::Value& config);
	ConfigObject* getConfigObject(std::string const& name);

	void logMessage(std::string const& message);
	virtual void MinecraftInit();
	virtual void UiInit();
	virtual const char* GetName();
	virtual void OnTick(Actor* act);
	virtual void OnSendPacket(Packet* packet);
	virtual void AfterPacket(Packet* packet);
	virtual void OnRender(MinecraftUIRenderContext* ctx);
	virtual void OnRender3D(ScreenContext* ctx);
	virtual void OnCmd(std::vector<std::string>* cmd);
	virtual void OnCmd(std::string const& cmd);
	virtual void OnAttack(GameMode* object, Actor* act);
	virtual void OnBuild(GameMode* object, BlockPos const& pos, int rot);
	virtual void OnNewGame(std::string const& ip);
	virtual void OnSendChat(std::string const& msg);
	virtual void OnMCMainThreadTick(MinecraftGame* mcgame);
	virtual void OnReceiveData(std::string* data);
	virtual void OnSendData(std::string* data);
	virtual void AfterSendData(std::string* data);
	virtual void BeforeMCHook();
};


class ConfigLoader {
private:
	ModuleManager* manager;
public:
	ConfigLoader(ModuleManager* m);
	std::string getPath();
	void load();
	void save();
};

class ModuleManager {
private:
	void comandResultMessage(std::string const& message);

	void initListener();
	void initModules();
	void initUI();
public:
	void handleMCEvent(Json::Value const& s);

	ConfigLoader* loader;

	void jniOnLoad(JavaVM* vm);
	void executeModuleCommand(std::string const& s);
	
	ModuleManager();
	std::vector<std::shared_ptr<Module>> ModuleList;

	template <typename TRet>
	TRet* getModule() {
		for (auto pMod : ModuleList) {
			if (auto pRet = dynamic_cast<typename std::remove_pointer<TRet>::type*>(pMod.get())) {

				return pRet;
			}
		}
		return nullptr;
	};

	template <typename TRet>
	std::vector<TRet*> getModules() {
		std::vector<TRet*> result;
		for (auto pMod : ModuleList) {
			if (auto pRet = dynamic_cast<typename std::remove_pointer<TRet>::type*>(pMod.get())) {
				result.push_back(pRet);
			}
		}
		return result;
	};

	bool executedCMD = false;
	bool canUseCommand = false;
};

extern ModuleManager* moduleManager;