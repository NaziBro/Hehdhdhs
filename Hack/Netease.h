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
#include "FunctionTool/FunctionTool.h"

class Netease :public UIModule {
public:
	Netease();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void MinecraftInit() override;
	virtual void initViews() override;
	virtual void OnNewGame(std::string const& ip) override;

	int k = 0;
	uint64_t time_v;
	Android::TextView* ip;
	std::string currentIP;

	bool hookRnPyMessage = false;
}; 