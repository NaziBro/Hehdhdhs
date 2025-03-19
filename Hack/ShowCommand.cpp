#include "ShowCommand.h"
#include "MCPE/GameData.h"
#include "MCPE/SDK/mce.h"
#include "HackSDK.h"
#include "Android.h"

ShowCommand::ShowCommand()
{
	ModuleType = "Render";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* ShowCommand::GetName()
{
	return "ShowCommand";
}

void ShowCommand::OnRender(MinecraftUIRenderContext* ctx)
{
	if (enabled) {
		if (tick == 20) {
			updateBlocks();
			tick = 0;
		}
		tick = tick + 1;
		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		if (lp) {
			glmatrixf* matrix = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix();
			vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
			vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
			float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
			float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
			vec2_t display(screenWidth, screenHeight);
			BlockSource* bs = lp->getRegion();
			for (auto i : cmdBlocks) {
				BlockActor* ba = bs->getBlockEntity(i);
				if (ba) {
					CompoundTag blockData;
					ba->save(blockData);
					if (blockData.getString("id") == "CommandBlock") {

						vec2_t text;
						vec3_t origin((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
						mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix()->OWorldToScreen(origin, vec3_t((float)i.x + 0.5f, (float)i.y + 1.0f, (float)i.z + 0.5f), text, fov, display);
						if (text.x > 0.01f || text.y > 0.01f) {
							text.x = text.x - 8.0f;
							std::stringstream os;
							os << "§l§e" << blockData.getString("Command");
							HackSDK::drawText(*ctx, text, os.str(), mce::Color(0, 0, 0), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont());
						}
					}
				}
			}
		}
	}
}

void ShowCommand::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".ShowCommand") {
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
}

void ShowCommand::updateBlocks()
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (lp) {
		BlockPos start(*lp->getPos());
		BlockSource* bs = lp->getRegion();
		if (bs) {
			for (int x = start.x - 25; x <= start.x + 25; x = x + 1) {
				for (int y = start.y - 25; y <= start.y + 25; y = y + 1) {
					for (int z = start.z - 25; z <= start.z + 25; z = z + 1) {
						BlockPos bp(x, y, z);
						BlockActor* ba = bs->getBlockEntity(bp);
						if (ba) {
							CompoundTag blockData;
							ba->save(blockData);
							if (blockData.getString("id") == "CommandBlock") {
								cmdBlocks.push_back(bp);
							}
						}
					}
				}
			}
		}
	}
}

