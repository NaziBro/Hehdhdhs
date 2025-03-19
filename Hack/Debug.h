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
#include "Android.h"
#include "Protocol.h"


#include "FunctionTool/FunctionTool.h"

class Debug :public Module {
private:
	void createDebugWindow();

public:
	BinaryPacket* hytCrashPacket = nullptr;
	bool test = false;
	bool godmode = false;
	Vec3 godPos;
	bool floodPacket = false;
	bool debugButton = false;

	std::string spammer_text = "";
	int spammer_tick = 0;

	virtual void UiInit() override;
	virtual void MinecraftInit() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnCmd(std::string const& cmd) override;
	virtual void OnSendPacket(Packet* packet) override;
	virtual void OnTick(Actor* m) override;
	virtual void OnReceiveData(std::string* data) override;
	virtual void OnSendData(std::string* data) override;
	virtual void AfterSendData(std::string* data) override;


	bool HookMCPacket = false;
	bool HookRecvPacket = false;

	std::string rakNetIP;
	uint32_t rakNetPort;

	std::mutex pk_mutex;

	bool showPacketInfo = true;
	std::vector<std::string> HidePacketNames = { "MoveActorDeltaPacket","MobEffectPacket","SetScorePacket","SetActorDataPacket","BlockUpdatePacket","RequestChunkRadiusPacket","MovePlayerPacket", "TransportPacket" ,"LevelEventPacket","ActorEventPacket"};


	std::mutex dpk_mutex;

	std::vector<std::string> DropPacketNames;

	bool HookAction = false;

	FunctionTool debugFuns;

}; 