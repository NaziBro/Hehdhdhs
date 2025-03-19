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

class Killaura :public UIModule {
public:
	Killaura();

	void attackActor(Actor* actor);
	virtual const char* GetName() override;
	virtual void OnTick(Actor* act) override;
	virtual void MinecraftInit() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void initViews() override;

	int cps_boost = 1;
	bool enabled = false;
	float Range = 5.0f;
	bool PlayerMode = true;
	int cps = 10;
	bool attackAll = false;
	bool antibot = true;
	bool back = false;
	bool spin = false;
	bool up = false;
	bool swing = true;

	bool handleAttack = false;
	bool attacked = false;
	int spinAngle = 0;
};