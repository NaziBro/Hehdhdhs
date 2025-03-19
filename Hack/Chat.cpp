#include "Chat.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include <math.h>
#include "Chat.h"
#include "Network.h"
#include "HackSDK.h"
#include "MCPE/MCHook.h"
#include "Utils.hpp"

Chat::Chat()
{
	addConfig(&antiSpamming, "antiSpamming");
	addConfig(&HideChat, "HideChat");

	ModuleType = "Other";
	UIType = 1;
}

const char* Chat::GetName()
{
	return "Chat";
}

void Chat::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".HideChat") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			HideChat = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			HideChat = false;
		}
	}
	else if ((*cmd)[0] == ".AntiSpamming") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			antiSpamming = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			antiSpamming = false;
		}
	}
}

void Chat::initViews()
{
	Android::TextView* Chat_HideChat = mAndroid->newTextView();
	UIUtils::updateTextViewData(Chat_HideChat, "HideChat", "#FF0000", 19);
	Chat_HideChat->setOnClickListener([](Android::View*) {
		Chat* chat = moduleManager->getModule<Chat>();
		if (chat->HideChat) {
			mAndroid->Toast("HideChat false!");
			chat->HideChat = false;
		}
		else {
			mAndroid->Toast("HideChat true!");
			chat->HideChat = true;
		}
	});

	Android::TextView* Chat_antiSpamming = mAndroid->newTextView();
	UIUtils::updateTextViewData(Chat_antiSpamming, "antiSpamming", "#FF0000", 19);
	Chat_antiSpamming->setOnClickListener([=](Android::View*) {
		if (antiSpamming) {
			mAndroid->Toast("AntiSpamming false!");
			antiSpamming = false;
		}
		else {
			mAndroid->Toast("AntiSpamming true!");
			antiSpamming = true;
		}
	});

	SecondWindowList.push_back(Chat_HideChat);
	SecondWindowList.push_back(Chat_antiSpamming);
}

void Chat::OnReceiveData(std::string* data)
{
	

	BinaryPacketReader reader(data);
	Text t;
	if (t.read(reader)) {
		if (HideChat) {
			MCHook::dropReceivePacketThisTime = true;
		}
		else if (antiSpamming) {
			if (Utils::match(t.message.c_str(), "§") > 7 || t.message.size() > 55 * 3) {
				t.message = "尝试刷屏已被屏蔽";
				*data = t.write().data;
			}
			if (Utils::match(t.source.c_str(), "§") > 7 || t.source.size() > 16 * 3) {
				t.source = "尝试刷屏已被屏蔽";
				*data = t.write().data;
			}
		}
		else {
			Network* network = moduleManager->getModule<Network>();
			if (network) {
				if (network->showChat(&t) == false) {
					MCHook::dropReceivePacketThisTime = true;
				}
			}
		}
	}
}
