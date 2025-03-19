#include <sstream>
#include "HackSDK.h"
#include "Utils.hpp"
#include "inlineHook/And64InlineHook.hpp"
#include <string>
#include <memory>
#include "MCPE/SDK/mce.h"

uint64_t(*old_ContainerManagerController__updatePreviewItem)(ContainerManagerController*, ItemInstance&, ItemInstance const&, std::string const&);

uint64_t now_ContainerManagerController__updatePreviewItem(ContainerManagerController* thiz, ItemInstance& i1, ItemInstance const& i2, std::string const& type) {
	if (moduleManager->getModule<HackSDK>()->lastItem) {
		return (*old_ContainerManagerController__updatePreviewItem)(thiz, i1, *moduleManager->getModule<HackSDK>()->lastItem, type);
	}
	return (*old_ContainerManagerController__updatePreviewItem)(thiz, i1, i2, type);

}

void HackSDK::initViews()
{


	Android::TextView* Transfer_InventoryActionMode = mAndroid->newTextView();
	UIUtils::updateTextViewData(Transfer_InventoryActionMode, "InventoryActionMode", "#FF0000", 19);
	Transfer_InventoryActionMode->setOnClickListener([=](Android::View*) {
		transferType = InventoryActionMode;
		mAndroid->Toast("TransferType::InventoryActionMode");
		});

	Android::TextView* Transfer_ContainerMode = mAndroid->newTextView();
	UIUtils::updateTextViewData(Transfer_ContainerMode, "ContainerMode", "#FF0000", 19);
	Transfer_ContainerMode->setOnClickListener([=](Android::View*) {
		transferType = ContainerMode;
		mAndroid->Toast("TransferType::ContainerMode");
		});

	Android::TextView* Transfer_ServerMode = mAndroid->newTextView();
	UIUtils::updateTextViewData(Transfer_ServerMode, "ServerMode", "#FF0000", 19);
	Transfer_ServerMode->setOnClickListener([=](Android::View*) {
		transferType = ServerMode;
		mAndroid->Toast("TransferType::ServerMode");
		});

	Android::TextView* Transfer_DeleteContainerItem = mAndroid->newTextView();
	UIUtils::updateTextViewData(Transfer_DeleteContainerItem, "DeleteContainerItem", "#FF0000", 19);
	Transfer_DeleteContainerItem->setOnClickListener([=](Android::View*) {
		delete lastItem;
		lastItem = nullptr;
		});

	SecondWindowList.push_back(Transfer_ServerMode);
	SecondWindowList.push_back(Transfer_InventoryActionMode);
	SecondWindowList.push_back(Transfer_ContainerMode);
	SecondWindowList.push_back(Transfer_DeleteContainerItem);
}

HackSDK::HackSDK()
{
	ModuleType = "Item";
	UIType = 1;
}

const char* HackSDK::GetName()
{
	return "HackSDK";
}

const char* HackSDK::getMenuName()
{
	return "TransferType";
}

void HackSDK::MinecraftInit()
{

	HackSDK::FastHook(mGameData.basePtr.ContainerManagerController__updatePreviewItem, (void*)now_ContainerManagerController__updatePreviewItem, (void**)&old_ContainerManagerController__updatePreviewItem);
}

void HackSDK::OnTick(Actor* act)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	if (lp) {
		if (act->getRuntimeID() == lp->getRuntimeID()) {
			if (act != lp) {
				remoteLocalPlayer = (Player*)act;
			}
		}
	}

	if (act != lp)return;
	lpTickEventsMtx.lock();
	for (auto it = lpTickEvents.begin(); it != lpTickEvents.end(); it = it + 1) {
		(*it)();
	}
	lpTickEvents.clear();
	lpTickEventsMtx.unlock();
}

void HackSDK::OnMCMainThreadTick(MinecraftGame* mcgame)
{
	mcTickEventsMtx.lock();
	for (auto it = mcTickEvents.begin(); it != mcTickEvents.end(); it = it + 1) {
		(*it)();
	}
	mcTickEvents.clear();
	mcTickEventsMtx.unlock();
}

void HackSDK::OnNewGame(std::string const& ip)
{
	remoteLocalPlayer = nullptr;
	delete lastItem;
	lastItem = nullptr;
}

void HackSDK::destroyBlock(BlockPos const& bp, int rot)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();

	lp->getPlayerActionComponent()->addStartDestroyBlock(bp, rot);
	lp->getPlayerActionComponent()->addPredictDestroyBlock(bp, rot);
	lp->getPlayerActionComponent()->addCrackBlock(bp, rot);
	lp->getPlayerActionComponent()->addStopDestroyBlock(bp, rot);

	lp->sendInput();

	lp->gamemode->destroyBlock(bp, rot);
}

bool HackSDK::onGroundCheck()
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	Vec3 below(*lp->getPos());
	below.y = below.y - 2.0f;

	if (lp->getRegion()->getBlock(below)->_BlockLegacy->getBlockItemId() != 0) {
		return true;
	}
	return false;
}

bool HackSDK::isRealPlayer(Player* p)
{
	std::string name = p->getName();
	if (name.size() < 2)return false;
	if (strstr(name.c_str(), "商") && strstr(name.c_str(), "店"))return false;
	if (p->getHealth() <= 0 || p->getHealth() > 10000)return false;
	return true;
}


void HackSDK::FastHook(uint64_t addr, void* now, void** old)
{
	A64HookFunction((void*)(mGameData.libinfo.head + addr), now, old);
}


void HackSDK::transferItem(ItemStack& old, ItemStack& now, uint32_t slot)
{
	if (autoTransfer == false) {
		return;
	}

	if (transferType == InventoryActionMode) {


		InventorySource source1(0, 0, 0);
		InventoryAction action1(&source1, slot, old, *mGameData.emptyItem);
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->manager.addAction(action1, false);

		InventorySource source2(99999, -5, 0);
		InventoryAction action2(&source2, 0, *mGameData.emptyItem, old);
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->manager.addAction(action2, false);

		InventorySource source3(99999, -5, 0);
		InventoryAction action3(&source3, 0, now, *mGameData.emptyItem);
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->manager.addAction(action3, false);

		InventorySource source4(0, 0, 0);
		InventoryAction action4(&source4, slot, *mGameData.emptyItem, now);
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->manager.addAction(action4, false);

		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->updateInventoryTransactions();
	}
	else if (transferType == ContainerMode) {
		if (lastItem) {
			delete lastItem;
		}
		lastItem = new ItemInstance(now);
	}
	else if (transferType == ServerMode) {
		mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getSupplies()->setItem(slot, now, 0, false);
		if (remoteLocalPlayer) {
			remoteLocalPlayer->getSupplies()->setItem(slot, now, 0, false);
		}

	}
}

void HackSDK::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".AutoTransfer") {
		if (cmd->size() < 2)return;
		if ((*cmd)[1] == "true") {
			moduleManager->executedCMD = true;
			autoTransfer = true;
		}
		else if ((*cmd)[1] == "false") {
			moduleManager->executedCMD = true;
			autoTransfer = false;
		}
	}
	else if ((*cmd)[0] == ".TransferType") {
		if (cmd->size() < 2)return;
		transferType = (TransferType)atoi((*cmd)[1].c_str());
		autoTransfer = false;
		moduleManager->executedCMD = true;
	}
	else if ((*cmd)[0] == ".DeleteContainerItem") {
		moduleManager->executedCMD = true;
		delete lastItem;
		lastItem = nullptr;
	}
}

void HackSDK::addLocalPlayerTickEvent(std::function<void(void)> call)
{
	lpTickEventsMtx.lock();
	lpTickEvents.push_back(call);
	lpTickEventsMtx.unlock();
}

void HackSDK::runOnMCMainThread(std::function<void(void)> call)
{
	mcTickEventsMtx.lock();
	mcTickEvents.push_back(call);
	mcTickEventsMtx.unlock();
}

void HackSDK::lookAt(Vec3 pos, float smooth)
{
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	Vec3* orgin = lp->getPos();
	if (orgin->x == pos.x) {
		pos.x = pos.x + 0.001f;
	}
	if (orgin->z == pos.z) {
		pos.z = pos.z + 0.001f;
	}
	float ZOP = 0.0f;


	float PH = fabsf(pos.x - orgin->x);
	float OH = fabsf(pos.z - orgin->z);
	float POH = atanf(PH / OH) * 180.0f / PI;

	if (pos.z > orgin->z) {//Z正半轴
		if (pos.x > orgin->x) {//第一象限
			ZOP = -POH;
		}
		else {//第四象限
			ZOP = POH;
		}
	}
	else {//Z负半轴
		if (pos.x > orgin->x) {//第二象限
			ZOP = -(180.0f - POH);
		}
		else {//第三象限
			ZOP = 180.0f - POH;
		}
	}
	
	
	float yaw = ZOP - lp->rot.yaw;

	float PT = fabsf(orgin->y - pos.y);
	float OT = sqrtf(powf(orgin->x - pos.x, 2) + powf(orgin->z - pos.z, 2));
	float POT = atanf(PT / OT) * 180.0f / PI;
	if (orgin->y < pos.y) {
		POT = -POT;
	}
	
	float pitch = lp->rot.pitch - POT;

	Vec2 rot(pitch / smooth, yaw / smooth);
	lp->_applyTurnDelta(rot);
}

void HackSDK::sendBinaryPacket(BinaryPacket const& packet)
{
	sendData(packet.data);
}

void HackSDK::sendOriginalPacket(OriginalPacket const& packet)
{
	sendBinaryPacket(packet.write());
}



void HackSDK::sendData(std::string const& data)
{
	ClientInstance* client = mGameData.getNowMinecraftGame()->getPrimaryClientInstance();
	if (client) {
		LoopbackPacketSender* sender = client->getPacketSender();
		if (sender) {
			NetworkHandler* handler = sender->handler;
			if (handler) {
				auto conn = handler->getConnection(handler->getServerNetworkIdentifer());
				if (conn) {
					conn->sendPacket(data, handler->getCompress());
				}
			}
		}
	}
}

void HackSDK::flushData()
{
	ClientInstance* client = mGameData.getNowMinecraftGame()->getPrimaryClientInstance();
	if (client) {
		LoopbackPacketSender* sender = client->getPacketSender();
		if (sender) {
			NetworkHandler* handler = sender->handler;
			if (handler) {
				auto conn = handler->getConnection(handler->getServerNetworkIdentifer());
				if (conn) {
					conn->flushPacket();
				}
			}
		}
	}
}

bool HackSDK::TryBuildBlock(BlockPos const& blockBelow)
{
	Block* block = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion()->getBlock(blockBelow);
	BlockLegacy* blockLegacy = block->_BlockLegacy;
	if (blockLegacy->getMaterial()->isReplaceable()) {
		vec3_ti blok(blockBelow.x, blockBelow.y, blockBelow.z);

		static std::vector<vec3_ti*> checklist;
		if (checklist.empty()) {
			checklist.push_back(new vec3_ti(0, -1, 0));
			checklist.push_back(new vec3_ti(0, 1, 0));

			checklist.push_back(new vec3_ti(0, 0, -1));
			checklist.push_back(new vec3_ti(0, 0, 1));

			checklist.push_back(new vec3_ti(-1, 0, 0));
			checklist.push_back(new vec3_ti(1, 0, 0));
		}

		bool foundCandidate = false;
		int i = 0;
		for (auto current : checklist) {
			vec3_ti calc = blok.sub(*current);
			BlockPos calcBP(Vec3(calc.x, calc.y, calc.z));
			if (!((mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getRegion()->getBlock(calcBP)->_BlockLegacy))->getMaterial()->isReplaceable()) {
				foundCandidate = true;
				blok = calc;
				break;
			}
			i++;
		}

		if (foundCandidate) {
			BlockPos bp(Vec3(blok.x, blok.y, blok.z));
			mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode->buildBlock(bp, i);

			return true;
		}

	}

	return false;
}

void HackSDK::Enchant(ItemStackBase* item, short id, short lvl) {
	if (item->userData == 0) {
		CompoundTag* newUserData = new CompoundTag;
		item->userData = newUserData;
	}
	CompoundTag* nbt = item->userData;
	if (nbt->contains("ench")) {
		ListTag* enchList = nbt->getList("ench");
		size_t size = enchList->size();
		bool flag = false;
		for (int i = 0; i < size; ++i) {
			CompoundTag* ench = enchList->getCompound(i);
			if (ench->getShort("id") == id) {
				ench->remove("id");
				ench->remove("lvl");
				ench->putShort("id", id);
				ench->putShort("lvl", lvl);
				flag = true;
			}
		}
		
		if (flag == false) {
			std::unique_ptr<CompoundTag> enchData(new CompoundTag);
			enchData->putShort("id", id);
			enchData->putShort("lvl", lvl);
			enchList->add(std::move(enchData));
		}
	}
	else {
		std::unique_ptr<CompoundTag> enchData(new CompoundTag);
		enchData->putShort("id", id);
		enchData->putShort("lvl", lvl);
		std::unique_ptr<ListTag> enchList(new ListTag);
		enchList->add(std::move(enchData));
		nbt->put("ench", std::move(enchList));
	}
}

std::vector<Actor*> HackSDK::getActorList()
{

	std::vector<Actor*> all;
	Dimension* dim = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getDimension();
	dim->forEachPlayer([&](Player& a) {
		all.push_back(&a);
		return true;
		});
	dim->getEntityIdMap()->forEachEntity([&](Actor& a) {
		all.push_back(&a);
		});
	return all;
}

void HackSDK::drawText(MinecraftUIRenderContext& ctx, const vec2_t pos, std::string const& text, const mce::Color& color, Font* font, float textSize, float alpha)
{
	float posF[4];
	posF[0] = pos.x;
	posF[1] = pos.x + 1000;
	posF[2] = pos.y;
	posF[3] = pos.y + 1000;
	TextMeasureData textMeasure{};
	memset(&textMeasure, 0, sizeof(TextMeasureData));
	textMeasure.textSize = textSize;
	ctx.drawText(*font, posF, text, color, alpha, textMeasure);
}

void HackSDK::drawLine(MinecraftUIRenderContext& ctx, const vec2_t& start, const vec2_t& end, float lineWidth, mce::Color const& color)
{
	ScreenContext* screenCtx = ctx.getScreenContext();

	float modX = 0 - (start.y - end.y);
	float modY = start.x - end.x;

	float len = sqrtf(modX * modX + modY * modY);

	modX /= len;
	modY /= len;
	modX *= lineWidth;
	modY *= lineWidth;


	screenCtx->tesselator->begin(3, 6);
	screenCtx->tesselator->color(color);

	screenCtx->tesselator->vertex(start.x + modX, start.y + modY, 0);
	screenCtx->tesselator->vertex(start.x - modX, start.y - modY, 0);
	screenCtx->tesselator->vertex(end.x - modX, end.y - modY, 0);

	screenCtx->tesselator->vertex(start.x + modX, start.y + modY, 0);
	screenCtx->tesselator->vertex(end.x + modX, end.y + modY, 0);
	screenCtx->tesselator->vertex(end.x - modX, end.y - modY, 0);

	mce::MaterialPtr* uiMaterial = mce::MaterialPtr::getUIMaterialPtr();

	MeshHelpers::renderMeshImmediately(*screenCtx, *(screenCtx->tesselator), *uiMaterial);
}

void HackSDK::drawLineReal3D(ScreenContext& ctx, const vec3_t& start, const vec3_t& end, mce::Color const& color, const vec3_t& origin)
{
	auto myTess = ctx.tesselator;

	myTess->begin(4, 2);
	myTess->color(color);

	auto start1 = start.sub(origin);
	auto end1 = end.sub(origin);

	myTess->vertex(start1.x, start1.y, start1.z);
	myTess->vertex(end1.x, end1.y, end1.z);

	mce::MaterialPtr* uiMaterial = mce::MaterialPtr::getUIMaterialPtr();

	MeshHelpers::renderMeshImmediately(ctx, *ctx.tesselator, *uiMaterial);
}

void HackSDK::drawLine3D(MinecraftUIRenderContext& ctx, glmatrixf* matrix, vec3_t const& cam, vec2_t const& fov, vec2_t const& screenSize, vec3_t const& start, vec3_t const& end, float lineWdith, mce::Color const& color)
{
	vec2_t start2;
	if (matrix->OWorldToScreen(cam, start, start2, fov, screenSize) == true) {
		vec2_t end2;
		if (matrix->OWorldToScreen(cam, end, end2, fov, screenSize) == true) {
			drawLine(ctx, start2, end2, lineWdith, color);
		}
	}
}

void HackSDK::drawBoxReal3D(ScreenContext& ctx, vec3_t const& from, vec3_t const& to, mce::Color const& color, vec3_t const& origin)
{
	drawLineReal3D(ctx, from, vec3_t(to.x, from.y, from.z), color,origin);
	drawLineReal3D(ctx, from, vec3_t(from.x, to.y, from.z), color, origin);
	drawLineReal3D(ctx, from, vec3_t(from.x, from.y, to.z), color, origin);
	drawLineReal3D(ctx, vec3_t(to.x, from.y, from.z), vec3_t(to.x, to.y, from.z), color, origin);
	drawLineReal3D(ctx, vec3_t(to.x, from.y, from.z), vec3_t(to.x, from.y, to.z), color, origin);
	drawLineReal3D(ctx, vec3_t(from.x, to.y, from.z), vec3_t(to.x, to.y, from.z), color, origin);
	drawLineReal3D(ctx, vec3_t(from.x, to.y, from.z), vec3_t(from.x, to.y, to.z), color, origin);
	drawLineReal3D(ctx, to, vec3_t(to.x, to.y, from.z), color, origin);
	drawLineReal3D(ctx, to, vec3_t(to.x, from.y, to.z), color, origin);
	drawLineReal3D(ctx, to, vec3_t(from.x, to.y, to.z), color, origin);
	drawLineReal3D(ctx, vec3_t(from.x, from.y, to.z), vec3_t(from.x, to.y, to.z), color, origin);
	drawLineReal3D(ctx, vec3_t(from.x, from.y, to.z), vec3_t(to.x, from.y, to.z),  color, origin);
}

void HackSDK::drawBox3D(MinecraftUIRenderContext& ctx, glmatrixf* matrix, vec3_t const& cam, vec2_t const& fov, vec2_t const& display, vec3_t const& from, vec3_t const& to, float lineWdith, mce::Color const& color)
{
	drawLine3D(ctx, matrix, cam, fov, display, from, vec3_t(to.x, from.y, from.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, from, vec3_t(from.x, to.y, from.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, from, vec3_t(from.x, from.y, to.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, vec3_t(to.x, from.y, from.z), vec3_t(to.x, to.y, from.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, vec3_t(to.x, from.y, from.z), vec3_t(to.x, from.y, to.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, vec3_t(from.x, to.y, from.z), vec3_t(to.x, to.y, from.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, vec3_t(from.x, to.y, from.z), vec3_t(from.x, to.y, to.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, to, vec3_t(to.x, to.y, from.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, to, vec3_t(to.x, from.y, to.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, to, vec3_t(from.x, to.y, to.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, vec3_t(from.x, from.y, to.z), vec3_t(from.x, to.y, to.z), lineWdith, color);
	drawLine3D(ctx, matrix, cam, fov, display, vec3_t(from.x, from.y, to.z), vec3_t(to.x, from.y, to.z), lineWdith, color);
}

Player* HackSDK::getPlayerByName(std::string const& name)
{
	Player* p = nullptr;
	mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->getDimension()->forEachPlayer([&](Player& object) {
		if (object.getName() == name) {
			p = &object;
		}
		return true;
		});
	return p;
}

void* HackSDK::getGlobalPtr(std::string const& fName)
{
	std::stringstream addrPath;
	addrPath << "/data/data/" << GameData::getPackageName() << "/" << fName;
	std::stringstream addr;
	addr << Utils::readFileIntoString(addrPath.str().c_str());
	void* ptr;
	addr >> ptr;
	return ptr;
}

void HackSDK::destroy(BlockPos const& start, BlockPos const& end, int8_t rot) {
	int x = 0;
	int y = 0;
	int z = 0;
	GameMode* gm = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer()->gamemode;
	for (x = start.x; x <= end.x; x = x + 1) {
		for (y = start.y; y <= end.y; y = y + 1) {
			for (z = start.z; z <= end.z; z = z + 1) {
				gm->destroyBlock({x,y,z},rot);
			}
		}
	}
}

void HackSDK::addServerPacket(std::string const& data)
{
	ClientInstance* client = mGameData.getNowMinecraftGame()->getPrimaryClientInstance();
	if (client) {
		LoopbackPacketSender* sender = client->getPacketSender();
		if (sender) {
			NetworkHandler* handler = sender->handler;
			if (handler) {
				auto conn = handler->getConnection(handler->getServerNetworkIdentifer());
				if (conn) {
					MCHook::addReceivePacket(data, conn);
				}
			}
		}
	}
}

void HackSDK::addServerPacket(BinaryPacket const& data)
{
	addServerPacket(data.data);
}

void HackSDK::addServerPacket(OriginalPacket const& data)
{
	addServerPacket(data.write());
}

void HackSDK::setGlobalPtr(std::string const& fName, void* ptr)
{
	std::stringstream addrPath;
	addrPath << "/data/data/" << GameData::getPackageName() << "/" << fName;
	if (access(addrPath.str().c_str(), F_OK) == 0) {
		remove(addrPath.str().c_str());
	}

	std::stringstream addr;
	addr << ptr;

	Utils::WriteStringToFile(addrPath.str().c_str(), addr.str());
}
