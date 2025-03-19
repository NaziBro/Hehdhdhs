#include "NoKnockback.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

NoKnockback::NoKnockback()
{
	addConfig(&enabled, "enabled");

	ModuleType = "PVP";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* NoKnockback::getMenuName()
{
	return "NoKnockback(NetworkGame)";
}

const char* NoKnockback::GetName()
{
	return "NoKnockback";
}



void NoKnockback::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".NoKnockback") {
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

void NoKnockback::OnReceiveData(std::string* data)
{
	if (enabled) {
		BinaryPacketReader reader(data);
		if (reader.getId() == 0x28) {
			MCHook::dropReceivePacketThisTime = true;
		}
	}
}

