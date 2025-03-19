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

class ESP :public UIModule {
private:
	std::vector<Actor*> fetchObjects();
	void createTextView(std::string const& name, bool* b);
public:
	ESP();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void initViews() override;
	virtual void OnRender(MinecraftUIRenderContext* ctx) override;
	virtual void OnRender3D(ScreenContext* ctx) override;

	bool renderMob = false;
	bool renderHealth = false;
	bool renderBox = false;
	bool renderLine = false;
}; 