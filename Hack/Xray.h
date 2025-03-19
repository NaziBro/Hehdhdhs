#pragma once

#include <stdio.h>
#include <jni.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <thread>
#include "MCPE/SDK/Actor.h"
#include "MCPE/SDK/Packet.h"
#include "MCPE/SDK/Render.h"
#include "ModuleManager.h"
#include "Menu.h"

class Xray :public UIModule {
private:
	class BlockInfo {
	public:
		std::vector<BlockPos> blocks;
		std::string name;
		mce::Color renderColor;
		inline BlockInfo(std::string const& a, mce::Color b) {
			name = a;
			renderColor = b;
		}
	};
	std::vector<BlockInfo> list;

	void updateBlock();
	void clear();
	void sort();
	void check();
	int getUpdateTime();
	int tick = 0;

	std::mutex mtx;

public:
	Xray();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnRender3D(ScreenContext* ctx) override;
	virtual void OnNewGame(std::string const& ip) override;
	virtual void OnTick(Actor* act) override;
	virtual void initViews() override;

	bool enabled = false;
	int range = 50;
	int maxBlocks = 20;
}; 