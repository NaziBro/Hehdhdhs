#include "AutoAim.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"

AutoAim::AutoAim()
{
	addConfig(&smooth, "smooth");
	addConfig(&antibot, "antibot");
	addConfig(&Range, "Range");


	ModuleType = "PVP";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* AutoAim::GetName()
{
	return "AutoAim";
}


void AutoAim::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".AutoAim") {
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
			if (cmd->size() < 5)return;
			smooth = atof((*cmd)[2].c_str());
			if ((*cmd)[3] == "true") {
				antibot = true;
			}
			else if ((*cmd)[3] == "false") {
				antibot = false;
			}
			Range = atof((*cmd)[4].c_str());
			moduleManager->executedCMD = true;
		}
	}
}

void AutoAim::OnTick(Actor* act)
{
	if (enabled == false)return;
	if (act != mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer())return;
	Player* nearest = nullptr;
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	float mindist = Range;
	lp->getDimension()->forEachPlayer([&](Player& object) {
		if (&object != lp) {
			if (antibot) {
				if (HackSDK::isRealPlayer(&object) == false) {
					return true;
				}
			}
			float dist = lp->distanceTo(object);
			if (dist < mindist) {
				mindist = dist;
				nearest = &object;
			}

		}
		return true;
		});
	if (nearest) {
		HackSDK::lookAt(*nearest->getPos(), smooth);
	}
}

void AutoAim::initViews()
{
	Android::TextView* AutoAim_AntiBot = mAndroid->newTextView();
	UIUtils::updateTextViewData(AutoAim_AntiBot, "AntiBot", "#FF0000", 19);
	AutoAim_AntiBot->setOnClickListener([=](Android::View*) {
		if (antibot) {
			antibot = false;
			mAndroid->Toast("AutoAim::AntiBot disabled");
		}
		else {
			antibot = true;
			mAndroid->Toast("AutoAim::AntiBot enabled");
		}
		});

	Android::EditText* AutoAim_Smooth = mAndroid->newEditText();
	UIUtils::updateEditTextData(AutoAim_Smooth, "smooth");

	Android::EditText* AutoAim_Range = mAndroid->newEditText();
	UIUtils::updateEditTextData(AutoAim_Range, "Range");

	Android::TextView* AutoAim_Save = mAndroid->newTextView();
	UIUtils::updateTextViewData(AutoAim_Save, "Save", "#FF0000", 19);
	AutoAim_Save->setOnClickListener([=](Android::View*) {
		if (AutoAim_Smooth->text == "" || AutoAim_Range->text == "")return;
		mAndroid->Toast("AutoAim配置已保存");
		Range = UIUtils::et_getFloat(AutoAim_Range);
		smooth = UIUtils::et_getFloat(AutoAim_Smooth);
		});
	SecondWindowList.push_back(AutoAim_AntiBot);
	SecondWindowList.push_back(AutoAim_Smooth);
	SecondWindowList.push_back(AutoAim_Range);
	SecondWindowList.push_back(AutoAim_Save);
}

