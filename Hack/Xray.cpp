#include "Xray.h"
#include "MCPE/GameData.h"
#include "HackSDK.h"
#include "MCPE/SDK/mce.h"
#include <algorithm>

void Xray::updateBlock()
{
	mtx.lock();
	clear();
	BlockPos start(*mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getPos());
	BlockSource* bs = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion();
	for (int x = start.x - range; x <= start.x + range; x = x + 1) {
		for (int y = 1; y <= 254; y = y + 1) {
			for (int z = start.z - range; z <= start.z + range; z = z + 1) {
				Block* block = bs->getBlock(x, y, z);
				if (block) {
					for (auto it = list.begin(); it != list.end();it = it + 1) {
						if (strstr(block->_BlockLegacy->name.c_str(), it->name.c_str())) {
							it->blocks.push_back(BlockPos(x, y, z));
							break;
						}
					}
				}
			}
		}
	}
	mtx.unlock();
}

void Xray::clear()
{
	for (auto it = list.begin(); it != list.end();it = it + 1) {
		it->blocks.clear();
	}
}

void Xray::sort()
{
	struct Compare {
		Vec3 orgin;
		Compare(Vec3 const& a) {
			orgin = a;
		}
		bool operator()(BlockPos const& a, BlockPos const& b) {

			float d1 = sqrtf(powf(orgin.x - a.x, 2) + powf(orgin.y - a.y, 2) + powf(orgin.z - a.z, 2));
			float d2 = sqrtf(powf(orgin.x - b.x, 2) + powf(orgin.y - b.y, 2) + powf(orgin.z - b.z, 2));
			return d1 < d2;
		}

	};

	Compare compare(*mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getPos());

	for (auto it = list.begin(); it != list.end();it = it + 1) {
		std::sort(it->blocks.begin(), it->blocks.end(), compare);
	}
}

void Xray::check()
{
	BlockSource* bs = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion();
	class BlockChecker {
	public:
		std::vector<BlockPos> *bp;
		std::string name;
		BlockSource* bs;
		BlockChecker(std::vector<BlockPos>* a, std::string b,BlockSource* c) {
			bp = a;
			name = b;
			bs = c;
		}
		void checkBlocks() {

			for (auto it = bp->begin();it != bp->end();it = it + 1) {
				bool flag = false;
				Block* bl = bs->getBlock(*it);
				if (bl) {
					if (strstr(bl->_BlockLegacy->name.c_str(), name.c_str())) {
						flag = true;
					}
				}
				if (flag == false) {
					bp->erase(it);
					it = it - 1;
				}
			}
		}
	};

	for (auto it = list.begin(); it != list.end();it = it + 1) {
		BlockChecker ck(&it->blocks, it->name,bs);
		ck.checkBlocks();
	}
}

int Xray::getUpdateTime()
{
	return (range / 2) * 20 - 1;
}


Xray::Xray()
{

	addConfig(&range, "range");
	addConfig(&maxBlocks, "maxBlocks");

	ModuleType = "Render";
	UIType = 0;
	enable_ptr = &enabled;

	list.push_back(BlockInfo("coal_ore", mce::Color(0, 0, 0)));
	list.push_back(BlockInfo("iron_ore", mce::Color(1.0f, 1.0f, 1.0f)));
	list.push_back(BlockInfo("gold_ore", mce::Color(1.0f, 204.0f / 255.0f, 0)));
	list.push_back(BlockInfo("diamond_ore", mce::Color(0, 0, 1.0f)));
	list.push_back(BlockInfo("emerald_ore", mce::Color(0, 1.0f, 0)));
}

const char* Xray::GetName()
{
	return "Xray";
}


void Xray::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".Xray") {
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
			if (cmd->size() < 4)return;
			range = atoi((*cmd)[2].c_str());
			maxBlocks = atoi((*cmd)[3].c_str());
			moduleManager->executedCMD = true;
		}
	}
}


void Xray::OnRender3D(ScreenContext* ctx)
{
	if (enabled) {
		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		if (lp && lp->getRegion()) {
			mtx.lock();
			vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
			
			check();
			sort();

			for (auto it = list.begin(); it != list.end();it = it + 1) {
				for (auto i = it->blocks.begin();i != it->blocks.end();i = i + 1) {
					if (i > it->blocks.begin() + maxBlocks)break;
					vec3_t from(i->x, i->y, i->z);
					vec3_t to(i->x + 1.0f, i->y + 1.0f, i->z + 1.0f);
					HackSDK::drawBoxReal3D(*ctx,from, to, it->renderColor, cam);
				}
			}
			mtx.unlock();
		}
	}
}

void Xray::OnNewGame(std::string const& ip)
{
	clear();
	tick = getUpdateTime();
}

void Xray::OnTick(Actor* act)
{
	if (enabled) {
			if (act == mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()) {
				tick = tick + 1;
				if (tick > getUpdateTime()) {
					tick = 0;
						updateBlock();
					
				}
			}
		
	}
	else {
		tick = getUpdateTime();
	}
}

void Xray::initViews()
{
	Android::EditText* Xray_Range = mAndroid->newEditText();
	UIUtils::updateEditTextData(Xray_Range, "Range");


	Android::EditText* Xray_MaxBlock = mAndroid->newEditText();
	UIUtils::updateEditTextData(Xray_MaxBlock, "MaxBlockNum");

	Android::TextView* Xray_Save = mAndroid->newTextView();
	UIUtils::updateTextViewData(Xray_Save, "Save", "#FF0000", 19);
	Xray_Save->setOnClickListener([=](Android::View*) {
		if (Xray_Range->text == "" || Xray_MaxBlock->text == "")return;
		mAndroid->Toast("Xray配置已保存");
		range = UIUtils::et_getInt(Xray_Range);
		maxBlocks = UIUtils::et_getInt(Xray_MaxBlock);
	});

	SecondWindowList.push_back(Xray_Range);
	SecondWindowList.push_back(Xray_MaxBlock);
	SecondWindowList.push_back(Xray_Save);
}


