#pragma once

#include <stdio.h>
#include <jni.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include "json/json.h"
#include <stdlib.h>
#include <sys/time.h>
#include "MCPE/SDK/Actor.h"
#include "MCPE/SDK/Packet.h"
#include "MCPE/SDK/Render.h"
#include "FunctionTool/FunctionTool.h"
#include "ModuleManager.h"
#include <mutex>

class PosEvent;
class AreaEvent;
class ImportEvent;
class DumpEvent;
class ImportEventProvider;

class FastBuilder:public Module{
public:
	std::mutex mtx;

	virtual void OnTick(Actor* act) override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnMCMainThreadTick(MinecraftGame* m) override;
	virtual void OnRender(MinecraftUIRenderContext* ctx) override;
	virtual void MinecraftInit() override;
	virtual void OnReceiveData(std::string* data) override;
	virtual void OnSendPacket(Packet* packet) override;

	bool shouldInitTime = false;
	struct timeval lastTime;
	void importTick();
	void updateBlock(int n);

	void startImport();
	void finishImport();

	FastBuilder();
	ImportEvent* importEvent = nullptr;
	DumpEvent* event = nullptr;
	ImportEventProvider* importEventProvider = nullptr;

	PosEvent* posEvent = nullptr;
	AreaEvent* areaEvent = nullptr;
	int entityCheckTick = 0;

}; 