
#ifndef _DESTROY_H
#define _DESTROY_H

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

class Destroy:public UIModule {
public:
	Destroy();
	virtual const char* GetName() override;
	MCJson::Value* value;
	void addJson(void);
	virtual void MinecraftInit() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
};

#endif