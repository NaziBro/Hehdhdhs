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

class ClientPacketManager :public UIModule {
public:
	ClientPacketManager();
	virtual void initViews() override;
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnSendPacket(Packet* packet) override;

	int packetNum = 10;
	std::string packetName;
	bool packetBoost = false;
	bool noPacket = false;
}; 