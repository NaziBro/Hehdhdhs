#include "Water.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "Utils.hpp"

void Water::addData(int x, int y, int z)
{
	BlockPos p;
	p.x = x;
	p.y = y;
	p.z = z;
	for (auto it = data.begin(); it != data.end(); it = it + 1) {
		if (*it == p) {
			return;
		}
	}
	data.push_back(p);
}

void Water::removeData(int x, int y, int z)
{
	for (auto it = data.begin(); it != data.end();) {
		BlockPos p = *it;
		int px = p.x;
		int py = p.y;
		int pz = p.z;
		float dis = sqrtf((x - px) * (x - px) + (y - py) * (y - py) + (z - pz) * (z - pz));
		if (dis > 5.0f) {
			LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
			BlockSource* bs = lp->getRegion();
			Item* water = Item::getItem(8);
			BlockLegacy* water_ = **(water->getLegacyBlock());
			Block* water__ = water_->getStateFromLegacyData(0);
			bs->setBlock(px, py, pz, *water__, 3);

			data.erase(it);
		}
		else {
			it = it + 1;
		}
	}
}

Water::Water()
{
	ModuleType = "Movement";
	UIType = 0;
	enable_ptr = &enabled;
}

const char* Water::GetName()
{
	return "WaterWalk";
}



void Water::OnTick(Actor* obj)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (obj != lp)return;
	BlockSource* bs = lp->getRegion();
	int lpx = (int)(lp->PosData.from.x);
	int lpy = (int)(lp->PosData.from.y);
	int lpz = (int)(lp->PosData.from.z);

	if (!enabled) {
		if (bs) {
			removeData(lpx, lpy, lpz);
		}
		return;
	}

	Block* block = bs->getBlock(lpx, lpy + 1, lpz);
	int id = block->_BlockLegacy->getBlockItemId();
	if (id == 9 || id == 8) {
		goto BREAK;
	}

	for (int x = lpx - 3; x != lpx + 3; x = x + 1) {
		for (int z = lpz - 3; z != lpz + 3; z = z + 1) {


			int y = lpy - 1;
			block = bs->getBlock(x, y, z);
			id = block->_BlockLegacy->getBlockItemId();
			if (id == 9 || id == 8) {
				Item* glass = Item::getItem(20);
				
				BlockLegacy* glass_ = **(glass->getLegacyBlock());
				Block* glass__ = glass_->getStateFromLegacyData(0);
				bs->setBlock(x, y, z, *glass__, 3);
				addData(x, y, z);
			}
		}
	}

BREAK:

	removeData(lpx, lpy, lpz);
}

void Water::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".WaterWalk") {
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

