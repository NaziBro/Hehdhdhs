#include "AntiVoid.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"


AntiVoid::AntiVoid()
{


	addConfig(&fallSpeed, "fallSpeed");

	ModuleType = "Movement";

	UIType = 0;
	enable_ptr = &enabled;
}

const char* AntiVoid::GetName()
{
	return "AntiVoid";
}


void AntiVoid::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".AntiVoid") {
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
			fallSpeed = atof((*cmd)[2].c_str());
			moduleManager->executedCMD = true;
		}
	}
}

void AntiVoid::OnTick(Actor* act)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (act != lp || enabled == false)return;
	
	Vec3 below(*lp->getPos());
	below.y = below.y - 2.0f;

	if (lp->getRegion()->getBlock(below)->_BlockLegacy->getBlockItemId() != 0) {
		save = below;
		save.y = save.y + lp->height;
	}
	if (lp->motion.y <  -fallSpeed) {
		lp->teleportTo(save, true, 0, 1);
	}
}

void AntiVoid::initViews()
{
	Android::EditText* AntiVoid_FallSpeed = mAndroid->newEditText();
	UIUtils::updateEditTextData(AntiVoid_FallSpeed, "FallSpeed");

	Android::TextView* AntiVoid_Save = mAndroid->newTextView();
	UIUtils::updateTextViewData(AntiVoid_Save, "Save", "#FF0000", 19);
	AntiVoid_Save->setOnClickListener([=](Android::View*) {
		if (AntiVoid_FallSpeed->text == "")return;
		mAndroid->Toast("AntiVoid配置已保存");
		fallSpeed = UIUtils::et_getFloat(AntiVoid_FallSpeed);
	});

	SecondWindowList.push_back(AntiVoid_FallSpeed);
	SecondWindowList.push_back(AntiVoid_Save);
}

