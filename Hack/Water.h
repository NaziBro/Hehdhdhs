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

class Water :public UIModule {
private:
	std::vector<BlockPos> data;
	void addData(int x, int y, int z);
	void removeData(int x, int y, int z);
public:
	Water();
	virtual const char* GetName() override;
	virtual void OnTick(Actor* act) override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;

	bool enabled = false;
}; 