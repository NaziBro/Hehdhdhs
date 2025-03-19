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

class ActorEventManager :public UIModule {
public:
	void deleteMobs();
	std::string int64_len(int64_t t);
	void playAnimation(bool self,std::string const& animation);
	void dimensionBag(int dim);

	ActorEventManager();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void initViews() override;
}; 