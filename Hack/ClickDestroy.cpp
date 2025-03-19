#include "ClickDestroy.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

ClickDestroy::ClickDestroy()
{
	addConfig(&range, "range");

	ModuleType = "World";
	UIType = 0;
	enable_ptr = &enabled;
}



void ClickDestroy::OnBuild(GameMode* object, BlockPos const& pos, int rot)
{

	if (enabled == false || mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode != object)return;
	
		MCHook::noBuildThisTime = true;
		if (range == 0)return;
		BlockPos start;
		start.x = pos.x - range + 1;
		start.y = pos.y - range + 1;
		start.z = pos.z - range + 1;
		BlockPos end;
		end.x = pos.x + range - 1;
		end.y = pos.y + range - 1;
		end.z = pos.z + range - 1;
		HackSDK::destroy(start, end, rot);
	
}

const char* ClickDestroy::GetName()
{
	return "ClickDestroy";
}


void ClickDestroy::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".ClickDestroy") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "config") {
			if (cmd->size() < 3)return;
			moduleManager->executedCMD = true;
			range = atoi((*cmd)[2].c_str());
		}
		else if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;
		}
	}
}


void ClickDestroy::initViews() {
	Android::EditText* Click_Range = mAndroid->newEditText();
	UIUtils::updateEditTextData(Click_Range, "Range");

	Android::TextView * Click_save = mAndroid->newTextView();
	UIUtils::updateTextViewData(Click_save, "Save", "#FF0000", 19);
	Click_save->setOnClickListener([=](Android::View*) {
		if (Click_Range->text == "")return;
		mAndroid->Toast("Click配置已保存");
		range = UIUtils::et_getInt(Click_Range);
	});

	SecondWindowList.push_back(Click_save);
	SecondWindowList.push_back(Click_Range);
}