#include "ForceOpenSign.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"
#include "Android.h"

ForceOpenSign::ForceOpenSign()
{
	ModuleType = "World";
	UIType = 0;
	enable_ptr = &enabled;
}

void ForceOpenSign::editChunkSigns(std::string const& text)
{

	BlockPos start(*mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getPos());
	BlockSource* bs = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion();
	for (int x = start.x - 70; x <= start.x + 70; x = x + 1) {
		for (int y = -64; y <= 254; y = y + 1) {
			for (int z = start.z - 70; z <= start.z + 70; z = z + 1) {
				BlockPos bp(x, y, z);
				BlockActor* ba = bs->getBlockEntity(bp);
				if (ba) {
					CompoundTag blockData;
					ba->save(blockData);
					if (blockData.getString("id") == "Sign") {
						CompoundTag tag;
						tag.putString("Text", text);
						tag.putInt("x", x);
						tag.putInt("y", y);
						tag.putInt("z", z);
						tag.putBoolean("isMovable", true);
						tag.putString("id", "Sign");
						tag.putString("TextOwner", "VS");
						tag.putBoolean("IgnoreLighting", false);
						tag.putInt("SignTextColor", -16777216);
						BlockActorDataPacket packet(bp, tag);
						mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->sendNetworkPacket(packet);
					}
				}
			}
		}
	}
}

const char* ForceOpenSign::GetName()
{
	return "ForceOpenSign";
}

void ForceOpenSign::OnBuild(GameMode* object, BlockPos const& pos, int rot)
{
	if (enabled && mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode == object) {
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->openSign(pos);
	}
}


void ForceOpenSign::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".ForceOpenSign") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			enabled = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			enabled = false;
		}
	}
	else if ((*cmd)[0] == ".EditChunkSigns") {
		if (cmd->size() < 2)return;
		moduleManager->executedCMD = true;
		editChunkSigns((*cmd)[1]);
	}
}

void ForceOpenSign::initViews()
{
	Android::EditText* ForceOpenSign_Text = mAndroid->newEditText();
	UIUtils::updateEditTextData(ForceOpenSign_Text, "Text");

	Android::TextView* ForceOpenSign_editChunkSigns = mAndroid->newTextView();
	UIUtils::updateTextViewData(ForceOpenSign_editChunkSigns, "EditChunkSigns", "#FF0000", 19);
	ForceOpenSign_editChunkSigns->setOnClickListener([=](Android::View*) {
		if (mGameData.getNowMinecraftGame()->isInGame()) {
			mAndroid->Toast("修改区块告示牌成功!");
			moduleManager->getModule<HackSDK>()->addLocalPlayerTickEvent([=]() {
				editChunkSigns(ForceOpenSign_Text->text);
			});
		}
	});


	SecondWindowList.push_back(ForceOpenSign_editChunkSigns);
	SecondWindowList.push_back(ForceOpenSign_Text);
}

