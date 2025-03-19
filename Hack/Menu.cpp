#include "Menu.h"
#include "HackSDK.h"

void UIModule::initViews()
{

}

const char* UIModule::getMenuName()
{
	return GetName();
}

void UIModule::openSecondWindow()
{
	if (MenuCall) {
		if (fast_btn == nullptr) {
			fast_btn = mAndroid->newButton();

			Json::Value v1;
			v1["Text"] = getMenuName();
			v1["TextColor"] = "#00FF00";
			v1["TextSize"] = 14;
			fast_btn->updateData(v1);

			fast_btn->setOnClickListener([=](Android::View* v) {
				MenuCall();

				});
		}
	}


	if (secondWindowState == false) {
		SecondWindowBack = mAndroid->newTextView();
		UIUtils::updateTextViewData(SecondWindowBack, "Close", "#FF0000", 17);
		SecondWindowBack->setOnClickListener([=](Android::View*) {
			closeSecondWindow();
			});

		secondWindowState = true;

		SecondWindow = mAndroid->newWindow();
		SecondWindow->setCanMove(true);
		SecondWindow->setOrientation(1);
		SecondWindow->setCanShowKeyboard(true);
		SecondWindow->addView(*SecondWindowBack);
		if (enable_ptr != nullptr || MenuCall != nullptr) {
			FastButton = mAndroid->newTextView();
			UIUtils::updateTextViewData(FastButton, "FastButton", "#FF0000", 17);
			FastButton->setOnClickListener([=](Android::View*) {
				changeFastButtonState();
				});
			SecondWindow->addView(*FastButton);
		}
		for (auto it = SecondWindowList.begin(); it != SecondWindowList.end(); it = it + 1) {
			SecondWindow->addView(**it);
		}
	}
}

void UIModule::closeSecondWindow()
{
	secondWindowState = false;
	SecondWindow->deleteWindow();
	FastButton->release();
	SecondWindowBack->release();
}

void UIModule::changeFastButtonState()
{
	if (FastButtonWindow == nullptr) {
		mAndroid->Toast(getMenuName() + std::string(" 快捷按钮已开启"));
		FastButtonWindow = mAndroid->newWindow();
		FastButtonWindow->setOrientation(1);
		FastButtonWindow->setCanMove(true);
		if (MenuCall) {
			FastButtonWindow->addView(*fast_btn);
		}
		else if (enable_ptr) {
			moduleManager->getModule<MenuController>()->getMenuByName(ModuleType)->list->removeView(*fast_sw);
			FastButtonWindow->addView(*fast_sw);
		}
	}
	else {
		mAndroid->Toast(getMenuName() + std::string(" 快捷按钮已关闭"));
		FastButtonWindow->removeView(*fast_sw);
		FastButtonWindow->deleteWindow();
		FastButtonWindow = nullptr;
		moduleManager->getModule<MenuController>()->getMenuByName(ModuleType)->list->addView(*fast_sw);
	}
}


Menu::Menu(std::string const& typeName) {
	Name = typeName;
	initViews(typeName);
	startMainWindow();
}

void Menu::showList() {
	MainWindow->addView(*list);

}

void Menu::hideList()
{
	MainWindow->removeView(*list);

}

void Menu::startMainWindow()
{
	MainWindow = mAndroid->newWindow();
	MainWindow->setCanMove(true);
	MainWindow->setOrientation(1);
}

void Menu::showMainWindow()
{
	MainWindow->addView(*MainView2);
}
void Menu::hideMainWindow()
{
	MainWindow->removeView(*MainView2);
	if (listState) {
		hideList();
	}
}


void Menu::initViews(std::string const& typeName) {


	MainView2 = mAndroid->newTextView();

	MainView2->setOnClickListener([=](Android::View*) {
		if (listState == false) {
			showList();
			listState = true;
		}
		else {
			listState = false;
			hideList();
		}
		});
	UIUtils::updateTextViewData(MainView2, typeName, "#C1D2F0", "#FFFFFF", 17);
	Json::Value Gravity;
	Gravity["Gravity"] = 17;
	MainView2->updateData(Gravity);

	list = mAndroid->newLinearLayout();

	list->setBackground(moduleManager->getModule<MenuController>()->moduleTypes[typeName]);

	list->setOrientation(1);
	std::vector<UIModule*> uiModules = moduleManager->getModules<UIModule>();
	for (auto it = uiModules.begin(); it != uiModules.end(); it = it + 1) {
		UIModule* uiModule = *it;
		if (uiModule->ModuleType != typeName)continue;

		Android::TextView* tv = mAndroid->newTextView();


		uiModule->initViews();


		UIUtils::updateTextViewData(tv, uiModule->getMenuName(), "#00FFFF", 13);
		if (uiModule->UIType == 0) {
			uiModule->fast_sw = mAndroid->newLinearLayout();
			uiModule->fast_sw->setOrientation(0);

			list->addView(*uiModule->fast_sw);

			uiModule->fast_sw->addView(*tv);
			Android::Switch* sw = mAndroid->newSwitch();
			uiModule->fast_sw->addView(*sw);
			sw->setOnCheckedChangeListener([=](Android::Switch*, bool check) {
				*uiModule->enable_ptr = check;

				});
			tv->setOnClickListener([=](Android::View* v) {
				uiModule->openSecondWindow();
				});
		}
		else {
			list->addView(*tv);

			tv->setOnClickListener([=](Android::View* v) {
				if (uiModule->UIType == 1) {
					uiModule->openSecondWindow();
				}
				else if (uiModule->UIType == 2) {
					uiModule->MenuCall();
				}
				});
		}

		tv->setOnLongClickListener([=](Android::View* v) {
			if (uiModule->UIType == 1 || uiModule->UIType == 0) {
				uiModule->openSecondWindow();
			}
			else {
				uiModule->changeFastButtonState();
			}
			});
	}
}

void MenuController::startUI(std::string const& iconPath)
{
	for (auto i : moduleTypes) {
		Menu* menu = new Menu(i.first);
		menuList.push_back(menu);
	}
	Android::Window* window = mAndroid->newWindow();
	window->setCanMove(true);
	window->setOrientation(1);
	Android::ImageView* view = mAndroid->newImageView(iconPath);
	window->addView(*view);

	auto clickListener = [=](Android::View* v) {
		if (showMenuWindows == false) {
			showMenuWindows = true;
			for (auto i : menuList) {
				i->showMainWindow();
			}
		}
		else {
			showMenuWindows = false;
			for (auto i : menuList) {
				i->hideMainWindow();
			}
		}

	};

	view->setOnClickListener(clickListener);
	Android::TextView* view1 = mAndroid->newTextView();
	window->addView(*view1);
	UIUtils::updateTextViewData(view1, "备用悬浮窗", "#FF0000", 17);
	view1->setOnClickListener(clickListener);
}

Menu* MenuController::getMenuByName(std::string const& name)
{
	for (auto i : menuList) {
		if (i->Name == name) {
			return i;
		}
	}
	return nullptr;
}

const char* MenuController::GetName()
{
	return "MenuController";
}

void MenuController::OnRender(MinecraftUIRenderContext* ctx)
{
	auto uiModules = moduleManager->getModules<UIModule>();
	float h = 0.0f;
	for (auto i : uiModules) {
		if (i->enable_ptr != nullptr) {
			if (*i->enable_ptr) {
				std::stringstream name;
				name << "§l§a" << i->getMenuName();
				HackSDK::drawText(*ctx, vec2_t(0.0f, h), name.str(), mce::Color(0, 0, 0), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont(), 1.1f);
				h = h + 10.0f;
			}
		}
	}
}
