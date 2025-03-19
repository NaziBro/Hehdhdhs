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

class AutoAim :public UIModule {
public:
	AutoAim();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnTick(Actor* act) override;
	virtual void initViews() override;

	bool enabled = false;
	float smooth = 1.0f;
	bool antibot = true;
	float Range = 5.0f;
}; 