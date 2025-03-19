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
#include <sys/mman.h>
#include "Protocol.h"
#include "Menu.h"
#include "Utils.hpp"
#include <fcntl.h>
#include "MCPE/GameData.h"

class RaknetClient;

class Network :public UIModule {
private:


	std::vector<std::string> GameIDs;
	bool needToCheckActor = true;

	bool reportChat = false;
	bool isLogined = false;
	RaknetClient* client;
	void login(Json::Value value);
	void applyMessage(Json::Value value);
	void ReportChat(std::string chat);
	void VSChat(std::string chat);
	void NewChat(std::string chat);
	void VSChat(std::string name, std::string message);
	void tryGetMod(std::string id);

	class AD {
	public:
		bool sendAD = false;
		int ADTime = 0;
		std::string ADText;
		bool showAD = false;
		int ADTick = 0;

		void init(Json::Value v);
		void tick();
	};

	AD ad;

	void unZipData();

public:
	Network();
	~Network();
	virtual void initViews() override;
	virtual void UiInit() override;

	std::string Version = "16.0";
	std::string ip = "103.167.54.212";

	int port = 2555;


	static inline void crash(std::string reason) {
#ifndef LocalEdition
		moduleManager->getModule<Network>()->logMessage(reason);
		delete moduleManager;
#endif

		return;
	}

	void NewInfo(std::string id, std::string ip);
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnSendChat(std::string const& msg) override;
	virtual void OnTick(Actor* act) override;
	virtual void OnSendData(std::string* data) override;
	virtual const char* GetName() override;
	void sendMessageToServer(std::string const& message); 
	bool showChat(Text* packet);

	const char* packName = "data.zip";
	std::string unPackPath;
	std::string dataDir;
	std::string dataPackZip;
	uint32_t  PackHash;

	std::string serverIP;
};