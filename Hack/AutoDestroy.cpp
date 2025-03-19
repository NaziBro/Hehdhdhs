#include "AutoDestroy.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"


AutoDrestroy::AutoDrestroy()
{
	addConfig(&Range, "Range");
	addConfig(&blockName, "blockName");

	ModuleType = "World";

	UIType = 0;
	enable_ptr = &enabled;
}

const char* AutoDrestroy::GetName()
{
	return "AutoDrestroy";
}



void AutoDrestroy::OnTick(Actor* act)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (act != lp)return;

	if (enabled) {
		BlockSource* bs = lp->getRegion();
		int lpx = (int)(lp->PosData.from.x);
		int lpy = (int)(lp->PosData.from.y);
		int lpz = (int)(lp->PosData.from.z);
		for (int x = lpx - Range; x != lpx + Range + 1; x = x + 1) {
			for (int z = lpz - Range; z != lpz + Range + 1; z = z + 1) {
				for (int y = lpy - Range; y != lpy + Range + 1; y = y + 1) {
					Block* block = bs->getBlock(x, y, z);

					if (strstr(block->_BlockLegacy->name.c_str(), blockName.c_str())) {
						BlockPos p;
						p.x = x;
						p.y = y;
						p.z = z;

						HackSDK::destroyBlock(p, 1);
					}
				}
			}
		}
	}
}


void AutoDrestroy::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".AutoDestroy") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;;
		}
		else if ((*cmd)[1] == "config") {
			if (cmd->size() < 4)return;
			moduleManager->executedCMD = true;
			Range = atoi((*cmd)[2].c_str());
			blockName = (*cmd)[3];
		}
	}
}


void AutoDrestroy::initViews() {

	Android::EditText* AutoDestroy_blockName = mAndroid->newEditText();
	UIUtils::updateEditTextData(AutoDestroy_blockName, "blockName");

	Android::EditText* AutoDestroy_Range = mAndroid->newEditText();
	UIUtils::updateEditTextData(AutoDestroy_Range, "Range");

	Android::TextView *AutoDestroy_save = mAndroid->newTextView();
	UIUtils::updateTextViewData(AutoDestroy_save, "Save", "#FF0000", 19);
	AutoDestroy_save->setOnClickListener([=](Android::View*) {
		if (AutoDestroy_blockName->text == "" ||AutoDestroy_Range->text == "")return;
		mAndroid->Toast("AutoDestroy配置已保存");
		blockName = AutoDestroy_blockName->text;
		Range = UIUtils::et_getInt(AutoDestroy_Range);

	});

	

	SecondWindowList.push_back(AutoDestroy_save);
	SecondWindowList.push_back(AutoDestroy_blockName);
	SecondWindowList.push_back(AutoDestroy_Range);
}