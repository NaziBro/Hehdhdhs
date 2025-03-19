#include "AC.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include <thread>

AC::AC()
{
	addConfig(&cps, "cps");

	ModuleType = "PVP";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* AC::GetName()
{
	return "AC";
}

void AC::OnTick(Actor* act)
{
	if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer() != act)return;

	if (handleClick == true) {
		mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->handleBuildOrAttackButtonPress();
		handleClick = false;
	}
}


void AC::MinecraftInit()
{
	std::thread([=]() {
		while (true) {
			if (enabled == true) {
				int sleepTime = 1000000 / cps;
				usleep(sleepTime);
				handleClick = true;
			}
			else {
				usleep(100000);
			}
		}
	}).detach();
}

void AC::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".AC") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;
		}
		else if ((*cmd)[1] == "config") {
			if (cmd->size() < 3)return;
			cps = atof((*cmd)[2].c_str());
			if (cps > 20) {
				mGameData.getNowMinecraftGame()->getPrimaryGuiData()->displaySystemMessage("[AC]CPS is too high,reset 20 cps!", "");
				cps = 20;
			}
			moduleManager->executedCMD = true;
		}
	}
}


void AC::initViews() {

	Android::EditText* AC_CPS = mAndroid->newEditText();
	UIUtils::updateEditTextData(AC_CPS, "CPS");

	Android::TextView* AC_Save = mAndroid->newTextView();
	UIUtils::updateTextViewData(AC_Save, "Save", "#FF0000", 19);
	AC_Save->setOnClickListener([=](Android::View*) {
		if (AC_CPS->text == "")return;
		mAndroid->Toast("AC配置已保存");
		cps = UIUtils::et_getInt(AC_CPS);
		if (cps > 20) {
			cps = 20;
		}
	});

	SecondWindowList.push_back(AC_CPS);
	SecondWindowList.push_back(AC_Save);

}