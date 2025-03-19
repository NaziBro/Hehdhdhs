#include "World.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

World::World()
{
	ModuleType = "World";
	UIType = 1;
}

const char* World::GetName()
{
	return "World";
}


void World::OnCmd(std::vector<std::string>* cmd)
{

	 if ((*cmd)[0] == ".SetTime") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getLevel()->setTime(atoi((*cmd)[1].c_str()));
	}
}


void World::initViews() {
	Android::EditText* World_time = mAndroid->newEditText();
	UIUtils::updateEditTextData(World_time, "Time(0~24000)");

	Android::TextView* World_setTime = mAndroid->newTextView();
	UIUtils::updateTextViewData(World_setTime, "SetTime", "#FF0000", 19);
	World_setTime->setOnClickListener([=](Android::View*) {
		if (World_time->text == "")return;
		mAndroid->Toast("修改Time成功");
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getLevel()->setTime(UIUtils::et_getInt(World_time));
			});
	});

	SecondWindowList.push_back(World_time);
	SecondWindowList.push_back(World_setTime);
}