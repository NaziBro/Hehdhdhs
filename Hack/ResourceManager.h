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

class ResourceManager :public UIModule {
private:
	void reloadResourceInfo();
public:
	std::string getResourceInfoPath();
	Json::Value getResourceInfo();
	void saveResourceInfo(Json::Value const& value);

	std::mutex mtx;
	std::vector<std::string> resourceList;

	void addPack(std::string zipPath);
	void removePack(std::string zipPath);
	virtual const char* GetName() override;
	virtual void MinecraftInit() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void initViews() override;
	ResourceManager();
}; 