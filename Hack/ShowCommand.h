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
#include <vector>
class ShowCommand :public UIModule {
public:
	ShowCommand();
	virtual const char* GetName() override;
	virtual void OnRender(MinecraftUIRenderContext* ctx) override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	void updateBlocks();

	std::vector<BlockPos> cmdBlocks;

	int tick = 20;
	bool enabled = false;
}; 