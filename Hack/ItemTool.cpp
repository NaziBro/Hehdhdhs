#include "ItemTool.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

ItemTool::ItemTool()
{
	ModuleType = "Item";
	UIType = 1;
}

const char* ItemTool::GetName()
{
	return "Item";
}

void setItem(int id, int aux, int count) {
	ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
	int slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();
	if (item->count() == 0 || item->getId() == 0) {
		ItemInstance new_item(id, aux, count);
		moduleManager->getModule<HackSDK>()->transferItem(*item, new_item, slot);
	}
	else {
		ItemStack old_data(*item);
		item->_setItem(id);
		item->setAux(aux);
		item->set(count);
		moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
	}
}

void dupeItem(void) {
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	ItemStack* item = lp->getSelectedItem();

	int slot = lp->getSupplies()->getFirstEmptySlot();

	moduleManager->getModule<HackSDK>()->transferItem(*mGameData.emptyItem, *item, slot);
}

void giveItem(int id, int aux, int count) {
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	int slot = lp->getSupplies()->getFirstEmptySlot();

	ItemInstance new_item(id, count, aux);
	moduleManager->getModule<HackSDK>()->transferItem(*mGameData.emptyItem, new_item, slot);
}

void ItemTool::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".SetItem") {
		if (cmd->size() < 3)return;
		moduleManager->executedCMD = true;
		int itemID = atoi((*cmd)[1].c_str());
		int itemAux = (short)atoi((*cmd)[2].c_str());
		int itemCount = 1;
		if (cmd->size() > 3) {
			itemCount = atoi((*cmd)[3].c_str());
		}
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			setItem(itemID, itemAux, itemCount);
		});	
		
	}
	else if ((*cmd)[0] == ".Give") {
		if (cmd->size() < 3)return;
		moduleManager->executedCMD = true;
		int itemID = atoi((*cmd)[1].c_str());
		int itemAux = (short)atoi((*cmd)[2].c_str());
		int itemCount = 1;
		if (cmd->size() > 3) {
			itemCount = atoi((*cmd)[3].c_str());
		}
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			giveItem(itemID, itemAux, itemCount);
			});
	}
	else if ((*cmd)[0] == ".Dupe") {
		moduleManager->executedCMD = true;
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			dupeItem();
			});
	}
}


void ItemTool::initViews() {

	Android::EditText* Item_itemID = mAndroid->newEditText();
	UIUtils::updateEditTextData(Item_itemID, "ID");

	Android::EditText* Item_itemCount = mAndroid->newEditText();
	UIUtils::updateEditTextData(Item_itemCount, "Count");

	Android::EditText* Item_itemAux = mAndroid->newEditText();
	UIUtils::updateEditTextData(Item_itemAux, "Aux");

	Android::TextView* Item_set = mAndroid->newTextView();
	UIUtils::updateTextViewData(Item_set, "Set", "#FF0000", 19);
	Item_set->setOnClickListener([=](Android::View*) {
		if (Item_itemID->text == "" || Item_itemCount->text == "" || Item_itemAux->text == "")return;
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			setItem(UIUtils::et_getInt(Item_itemID), UIUtils::et_getInt(Item_itemAux), UIUtils::et_getInt(Item_itemCount));
			});
		mAndroid->Toast("设置物品成功");
	});

	Android::TextView* Item_give = mAndroid->newTextView();
	UIUtils::updateTextViewData(Item_give, "Give", "#FF0000", 19);
	Item_give->setOnClickListener([=](Android::View*) {
		if (Item_itemID->text == "" || Item_itemCount->text == "" || Item_itemAux->text == "")return;
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			giveItem(UIUtils::et_getInt(Item_itemID), UIUtils::et_getInt(Item_itemAux), UIUtils::et_getInt(Item_itemCount));
			});
		mAndroid->Toast("添加物品成功");
	});
	
	Android::TextView* Item_dupe = mAndroid->newTextView();
	UIUtils::updateTextViewData(Item_dupe, "Dupe", "#FF0000", 19);
	Item_dupe->setOnClickListener([=](Android::View*) {
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			dupeItem();
			});
		mAndroid->Toast("复制物品成功");
	});
	

	SecondWindowList.push_back(Item_itemID);
	SecondWindowList.push_back(Item_itemCount);
	SecondWindowList.push_back(Item_itemAux);
	SecondWindowList.push_back(Item_set);
	SecondWindowList.push_back(Item_give);
	SecondWindowList.push_back(Item_dupe);
}