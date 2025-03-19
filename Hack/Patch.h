#pragma once

#include <stdio.h>
#include <jni.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include "MCPE/SDK/Actor.h"
#include "MCPE/SDK/Packet.h"
#include "FunctionTool/FunctionTool.h"
#include "MCPE/SDK/Render.h"
#include "ModuleManager.h"
#include "Menu.h"

class Patch :public Module {
public:
	virtual const char* GetName() override;
	virtual void MinecraftInit() override;
	virtual void UiInit() override;

	FunctionTool scoreBoard;
}; 