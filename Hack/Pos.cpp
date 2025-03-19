#include "Pos.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

Pos::Pos()
{
	addConfig(&enabled, "enabled");

	ModuleType = "Render";

	UIType = 2;
	MenuCall = ([=]() {
		if (enabled) {
			mAndroid->Toast("ShowPos disabled");
			ShowPos(false);
		}
		else {
			mAndroid->Toast("ShowPos enabled");
			ShowPos(true);
		}
	});
}

const char* Pos::GetName()
{
	return "Pos";
}



void Pos::MinecraftInit()
{
	f.addFunction(mGameData.libinfo.head + mGameData.basePtr.CISM_shouldDisplayPlayerPosition);
	f.killAll();
}

void Pos::ShowPos(bool v) {
	enabled = v;
	if (v == true) {
		f.killAll(1);
	}
	else {
		f.killAll(0);
	}
}


void Pos::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".ShowPos") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			ShowPos(true);
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			ShowPos(false);
		}
	}
}

