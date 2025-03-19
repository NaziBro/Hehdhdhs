#include "Enchant.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

Enchant::Enchant()
{
	ModuleType = "Item";
	UIType = 1;
}

const char* Enchant::GetName()
{
	return "Enchant";
}

void removeEnchants() {
	ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
	if (item->getId() == 0)return;
	ItemStack old_data(*item);
	CompoundTag* nbt = item->userData;
	if (nbt == nullptr)return;
	if (nbt->contains("ench") == true) {
		nbt->remove("ench");
	}
	uint32_t slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();
	moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
}

void allEnchants() {
	ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
	if (item->getId() == 0)return;
	ItemStack old_data(*item);
	for (int i = 0; i <= 100; i = i + 1) {
		HackSDK::Enchant(item, i, 32767);
	}
	uint32_t slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();
	moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
}

void applyEnchant(int id,int lvl) {
	ItemStack* item = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItem();
	if (item->getId() == 0)return;
	ItemStack old_data(*item);
	HackSDK::Enchant(item, id, lvl);
	uint32_t slot = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSelectedItemSlot();
	moduleManager->getModule<HackSDK>()->transferItem(old_data, *item, slot);
}

void Enchant::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Enchant") {
		if (cmd->size() < 2)return;
		else if ((*cmd)[1] == "rm") {
			moduleManager->executedCMD = true;
			moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
				removeEnchants();
			});

		}
		else if ((*cmd)[1] == "all") {
			moduleManager->executedCMD = true;
			moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
				allEnchants();
			});
		}
		else {
			if (cmd->size() < 3)return;
			moduleManager->executedCMD = true;
			int id = (short)atoi((*cmd)[1].c_str());
			int lvl = (short)atoi((*cmd)[2].c_str());
			moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
				applyEnchant(id, lvl);
			});
		}
	}
}


void Enchant::initViews() {
	Android::EditText* Enchant_id = mAndroid->newEditText();
	UIUtils::updateEditTextData(Enchant_id, "id");

	Android::EditText* Enchant_lvl = mAndroid->newEditText();
	UIUtils::updateEditTextData(Enchant_lvl, "lvl");

	Android::TextView* Enchant_add = mAndroid->newTextView();
	UIUtils::updateTextViewData(Enchant_add, "AddEnchant", "#FF0000", 19);
	Enchant_add->setOnClickListener([=](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		if (Enchant_id->text == "" || Enchant_lvl->text == "")return;
		mAndroid->Toast("添加附魔成功");
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			applyEnchant(UIUtils::et_getInt(Enchant_id), UIUtils::et_getInt(Enchant_lvl));
		});
	});

	Android::TextView* Enchant_rm = mAndroid->newTextView();
	UIUtils::updateTextViewData(Enchant_rm, "RemoveEnchant", "#FF0000", 19);
	Enchant_rm->setOnClickListener([](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		mAndroid->Toast("移除附魔成功");
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			removeEnchants();
			});
	});

	Android::TextView* Enchant_all = mAndroid->newTextView();
	UIUtils::updateTextViewData(Enchant_all, "AllEnchant", "#FF0000", 19);
	Enchant_all->setOnClickListener([](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame() == false)return;
		mAndroid->Toast("添加全部附魔成功");
		moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
			allEnchants();
			});
	});

	SecondWindowList.push_back(Enchant_add);
	SecondWindowList.push_back(Enchant_id);
	SecondWindowList.push_back(Enchant_lvl);
	SecondWindowList.push_back(Enchant_rm);
	SecondWindowList.push_back(Enchant_all);
}