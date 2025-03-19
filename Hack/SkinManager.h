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

class SkinManager :public UIModule {
public:
	SkinManager();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void initViews() override;
	virtual void MinecraftInit() override;
	bool Invisibility = false;

	//bool DIYSkin = false;
	//std::string SkinPath = "";
};