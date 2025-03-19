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

class RecordEvent;
class PlayEvent;

class Music :public UIModule {
public:

	Music();
	virtual const char* GetName() override;
	virtual void OnCmd(std::vector<std::string>* cmd) override;
	virtual void OnTick(Actor* act) override;
	virtual void initViews() override;
	virtual void OnReceiveData(std::string* data) override;

	bool startRecord(std::string const& path);
	void stopRecord();
	void playMusic(std::string const& path);
	
	void startLoopMusic(std::string const& path);
	void stopLoopMusic(std::string const& path);

	std::mutex mtx;
	RecordEvent* record = nullptr;
	std::vector<PlayEvent*> play_list;

	float playSpeed = 1.0f;

	void updatePlaySpeed();
};
