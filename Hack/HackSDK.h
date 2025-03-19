#pragma once

#include <mutex>
#include <vector>
#include "Menu.h"
#include "Protocol.h"

class HackSDK :public UIModule {
private:
	std::vector<std::function<void(void)>> lpTickEvents;
	std::recursive_mutex lpTickEventsMtx;


	std::vector<std::function<void(void)>> mcTickEvents;
	std::recursive_mutex mcTickEventsMtx;
public:
	virtual void initViews() override;
	HackSDK();
	virtual const char* GetName() override;
	virtual const char* getMenuName() override;
	virtual void MinecraftInit() override;
	virtual void OnTick(Actor* act) override;
	virtual void OnMCMainThreadTick(MinecraftGame* mcgame) override;
	virtual void OnNewGame(std::string const& ip) override;

	bool autoTransfer = true;


	static void destroyBlock(BlockPos const& bp, int rot);
	static bool onGroundCheck();
	static bool isRealPlayer(Player* p);
	static void FastHook(uint64_t addr, void* now, void** old);
	static void Enchant(ItemStackBase* item, short id, short lvl);
	static std::vector<Actor*> getActorList();
	static void drawText(MinecraftUIRenderContext& ctx, const vec2_t pos, std::string const& text, const mce::Color& color, Font* font, float textSize = 1, float alpha = 1);
	static void drawLine(MinecraftUIRenderContext& ctx, const vec2_t& start, const vec2_t& end, float lineWidth, mce::Color const& color);
	static void drawLineReal3D(ScreenContext& ctx, const vec3_t& start, const vec3_t& end, mce::Color const& color, vec3_t  const& origin);
	static void drawLine3D(MinecraftUIRenderContext& ctx, glmatrixf* matrix, vec3_t const& cam, vec2_t const& fov, vec2_t const& screenSize, vec3_t const& start, vec3_t const& end, float lineWdith, mce::Color const& color);
	static void drawBoxReal3D(ScreenContext& ctx, vec3_t const& from, vec3_t const& to, mce::Color const& color, vec3_t  const& origin);

	static void drawBox3D(MinecraftUIRenderContext& ctx, glmatrixf* matrix, vec3_t const& cam, vec2_t const& fov, vec2_t const& display, vec3_t const& from, vec3_t const& to, float lineWdith, mce::Color const& color);
	static Player* getPlayerByName(std::string const& name);

	static void* getGlobalPtr(std::string const& fName);
	static void setGlobalPtr(std::string const& fName, void* ptr);

	void transferItem(ItemStack& old, ItemStack& now, uint32_t slot);

	inline void transferItem(ItemStack& old, ItemInstance& now, uint32_t slot) {
		ItemStack now_stack(now);
		transferItem(old, now_stack, slot);
	}

	virtual void OnCmd(std::vector<std::string>* cmd) override;

	void addLocalPlayerTickEvent(std::function<void(void)> call);
	void runOnMCMainThread(std::function<void(void)> call);

	enum TransferType {
		InventoryActionMode,
		ContainerMode,
		ServerMode
	};

	TransferType transferType = ContainerMode;
	Player* remoteLocalPlayer;
	ItemInstance* lastItem = nullptr;

	static void lookAt(Vec3 pos, float smooth = 1.0f);

	static void sendBinaryPacket(BinaryPacket const& packet);
	static void sendOriginalPacket(OriginalPacket const& packet);
	static void sendData(std::string const& data);
	static void flushData();

	static bool TryBuildBlock(BlockPos const& bp);
	static void destroy(BlockPos const& start, BlockPos const& end, int8_t rot);

	static void addServerPacket(std::string const& data);
	static void addServerPacket(BinaryPacket const& data);
	static void addServerPacket(OriginalPacket const& data);
};

