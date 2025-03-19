#pragma once

#include <stdio.h>
#include <jni.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "MCPE/SDK/LocalPlayer.h"
#include "MCPE/SDK/Packet.h"
#include "MCPE/SDK/Render.h"
#include "ModuleManager.h"
#include "Menu.h"
#include "Protocol.h"
#include <map>
#include "FunctionTool/FunctionTool.h"

class Server :public UIModule {
public:
	Server();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void initViews() override;
	virtual void OnTick(Actor* act) override;
	virtual void OnNewGame(std::string const& ip) override;

	void startAttack(void);
	void stopAttack(void);
	void ban(std::string const& name);
	void unBan(std::string const& name);

	std::map<std::string, BinaryPacket*> BanList;
	int sendTimes = 20;
	std::string banMessage;
	std::string attackMessage;
	std::mutex BanList_mtx;
	BinaryPacket* attack = nullptr;
	int dosTimes = 60;

}; 