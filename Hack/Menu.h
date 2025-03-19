#pragma once

#include "ModuleManager.h"
#include "Android.h"


class UIModule :public Module {
public:
	int UIType;
	bool* enable_ptr = nullptr;
	std::function<void(void)> MenuCall = nullptr;

	virtual void initViews();
	virtual const char* getMenuName();
	std::vector<Android::View*> SecondWindowList;

	Android::TextView* FastButton = nullptr;
	void openSecondWindow();
	void closeSecondWindow();
	Android::LinearLayout* fast_sw = nullptr;
	Android::Button* fast_btn = nullptr;
	Android::Window* FastButtonWindow = nullptr;
	Android::TextView* SecondWindowBack;
	Android::Window* SecondWindow;
	bool secondWindowState = false;

	std::string ModuleType;

	void changeFastButtonState();
};
class Menu {
public:
	Menu(std::string const& typeName);

	void initViews(std::string const& typeName);

	std::string Name;
	Android::Window* MainWindow;
	Android::TextView* MainView2;
	void startMainWindow();

	void showMainWindow();
	void hideMainWindow();

	void showList();
	void hideList();
	bool listState = false;

	Android::LinearLayout* list;

};
class MenuController :public Module {
public:
	std::map<std::string, std::string> moduleTypes = { {"PVP","https://s1.ax1x.com/2022/10/03/xQBi0P.jpg"},{"Movement","https://gchat.qpic.cn/gchatpic_new/0/0-0-4551C933984FAAFB022CB2756E3A28B2/0"},{"Render","https://gchat.qpic.cn/gchatpic_new/0/0-0-14224E11B6EAD0534BCDFFB366D57676/0"},{"World","https://s1.ax1x.com/2022/10/03/xQBNc9.jpg"},{"Other","https://gchat.qpic.cn/gchatpic_new/0/0-0-4399AA3542B61440645A7EF8AF4BB5CF/0"},{"Item","https://s3.bmp.ovh/imgs/2022/05/29/ab999f083d52ed49.jpg"} };
	std::vector<Menu*> menuList;
	bool showMenuWindows = false;
	void startUI(std::string const& iconPath);
	Menu* getMenuByName(std::string const& name);

	virtual const char* GetName() override;
	virtual void OnRender(MinecraftUIRenderContext* ctx) override;
};