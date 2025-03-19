#include "NightVision.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"
#include "Android.h"

NightVision::NightVision()
{
	addConfig(&enabled, "enabled");

	ModuleType = "Render";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* NightVision::GetName()
{
	return "NightVision";
}



void NightVision::OnTick(Actor* obj)
{
	if (enabled == false || obj != mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer())return;
	HackSDK::addServerPacket(MobEffect(obj->getRuntimeID(), 1, 16, 1, false, 4));
}

void NightVision::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".NightVision") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;
		}
	}
}

