#pragma once

#include <stdio.h>
#include <jni.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "MCPE/SDK/Actor.h"
#include "MCPE/SDK/Packet.h"
#include "MCPE/SDK/Render.h"
#include "ModuleManager.h"
#include "Menu.h"
#include "MCPE/GameData.h"
#include "Lua/lua.hpp"
#include "json/json.h"
#include <mutex>

class ScriptRuntime;

class Script :public UIModule {
public:
	Script();
	~Script();
	virtual const char* GetName() override;
	virtual void UiInit() override;
	virtual void OnTick(Actor* act) override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void MinecraftInit() override;
	void loadDefaultLua();
	virtual void initViews() override;
	virtual void OnBuild(GameMode* object, BlockPos const& pos, int rot) override;
	virtual void OnRender(MinecraftUIRenderContext* ctx) override;
	virtual void OnAttack(GameMode* object, Actor* act) override;
	virtual void OnSendPacket(Packet* packet) override;
	virtual void OnNewGame(std::string const& ip) override;


	void load(std::string const& filePath);

	ScriptRuntime* runtime;

	void runString(std::string Code, std::string luaID);
	void exit(int id);
};