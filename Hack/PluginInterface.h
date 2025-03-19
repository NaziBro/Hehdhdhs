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
#include <map>
#include <functional>

class PluginInterface {
public:
	PluginInterface();
	std::map<std::string, void*> functions;

	std::vector<std::string> registerNamespace;
	void addFunction(std::string const& name, void* ptr);
	
	void registerFunctions();
};

class PluginHelper :public Module {
public:
	std::vector< std::function<void(Actor*)>> OnTick_callBack;
	virtual void OnTick(Actor* act) override;

	std::vector< std::function<void(Packet*)>> OnSendPacket_callBack;
	virtual void OnSendPacket(Packet* packet) override;

	std::vector< std::function<void(MinecraftUIRenderContext*)>> OnRender_callBack;
	virtual void OnRender(MinecraftUIRenderContext* ctx) override;

	std::vector< std::function<void(std::vector<std::string>* cmd)>> OnCmd_callBack;
	virtual void OnCmd(std::vector<std::string>* cmd) override;

	std::vector< std::function<void(Actor* cmd)>> OnAttack_callBack;
	virtual void OnAttack(GameMode* object, Actor* act) override;


	std::vector< std::function<void(BlockPos const&,int)>> OnBuild_callBack;
	virtual void OnBuild(GameMode* object, BlockPos const& pos, int rot) override;

	std::vector< std::function<void(std::string const&)>> OnNewGame_callBack;
	virtual void OnNewGame(std::string const& ip) override;

	std::vector< std::function<void(std::string const&)>> OnSendChat_callBack;
	virtual void OnSendChat(std::string const& msg) override;
};