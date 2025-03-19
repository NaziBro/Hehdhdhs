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

class Hitbox :public UIModule {
public:
	Hitbox();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void initViews() override;
	virtual void OnTick(Actor* act) override;

	bool enabled = false;
	float r = 3.0f;
	float distance = 10.0f;
	bool PlayerMode = true;
}; 