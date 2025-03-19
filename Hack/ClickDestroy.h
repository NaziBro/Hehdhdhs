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

class ClickDestroy :public UIModule {
public:
	ClickDestroy();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnBuild(GameMode* object, BlockPos const& pos, int rot) override;
	virtual void initViews() override;

	bool enabled = false;
	int range = 5;

}; 