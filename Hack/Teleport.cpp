#include "Teleport.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"

Teleport::Teleport()
{
	ModuleType = "Movement";
	UIType = 1;
}

const char* Teleport::GetName()
{
	return "Teleport";
}

void TP(float x, float y, float z) {

	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	Vec3 pos(x, y, z);
	lp->teleportTo(pos, true, 0, 1);
}


void Teleport::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".TP") {
		if (cmd->size() > 4) {
			float x = atof((*cmd)[1].c_str());
			float y = atof((*cmd)[2].c_str());
			float z = atof((*cmd)[3].c_str());
			moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
				TP(x, y, z);
			});
		}
	}
}

void Teleport::initViews()
{
	Android::EditText* Teleport_X = mAndroid->newEditText();
	UIUtils::updateEditTextData(Teleport_X, "X");

	Android::EditText* Teleport_Y = mAndroid->newEditText();
	UIUtils::updateEditTextData(Teleport_Y, "Y");

	Android::EditText* Teleport_Z = mAndroid->newEditText();
	UIUtils::updateEditTextData(Teleport_Z, "Z");

	Android::TextView* Teleport_apply = mAndroid->newTextView();
	UIUtils::updateTextViewData(Teleport_apply, "TP", "#FF0000", 19);
	Teleport_apply->setOnClickListener([=](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		if (Teleport_X->text == "" || Teleport_Y->text == "" || Teleport_Z->text == "")return;
		if (mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->isGliding() == false) {
			mAndroid->Toast("您未处于滑翔状态,传送可能失败!");
		}
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			float x = UIUtils::et_getFloat(Teleport_X);
			float y = UIUtils::et_getFloat(Teleport_Y);
			float z = UIUtils::et_getFloat(Teleport_Z);
			TP(x, y, z);
		});
	});

	SecondWindowList.push_back(Teleport_X);
	SecondWindowList.push_back(Teleport_Y);
	SecondWindowList.push_back(Teleport_Z);
	SecondWindowList.push_back(Teleport_apply);
}

