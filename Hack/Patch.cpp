#include "Patch.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"
#include "Menu.h"


const char* Patch::GetName()
{
	return "Patch";
}


void Patch::MinecraftInit()
{
	for (auto i : mGameData.basePtr.SorceBoardPackets) {
		scoreBoard.addFunction(mGameData.libinfo.head + i);
	}
	scoreBoard.killAll();
}

void Patch::UiInit()
{
	auto tv = mAndroid->newTextView();
	UIUtils::updateTextViewData(tv,"租赁服兼容模式(enabled)", "#00FFFF", 13);
	Android::Window* win = mAndroid->newWindow();
	win->addView(*tv);
	win->setCanMove(true);

	bool* check = new bool;
	*check = true;
	tv->setOnClickListener([=](Android::View*) {
		if (scoreBoard.Functions.size() == 0) {
			return;
		}
		if (*check == false) {
			*check = true;
			scoreBoard.killAll();

			UIUtils::updateTextViewData(tv, "租赁服兼容模式(enabled)", "#00FFFF", 13);
		}
		else {
			*check = false;
			scoreBoard.rebuildAll();
			UIUtils::updateTextViewData(tv, "租赁服兼容模式(disabled)", "#00FFFF", 13);
		}
	});
}

