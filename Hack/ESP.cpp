#include "ESP.h"
#include "MCPE/SDK/mce.h"
#include "MCPE/GameData.h"
#include "Android.h"
#include "HackSDK.h"

std::vector<Actor*> ESP::fetchObjects()
{
	std::vector<Actor*> result;
	LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
	Dimension* _dimension = lp->getDimension();
	if (_dimension == nullptr)return result;
	EntityIdMap* map = _dimension->getEntityIdMap();
	if (map == nullptr)return result;
	if (renderMob == true) {
		map->forEachEntity([&](Actor& object) {
			result.push_back(&object);
		});
	}
	_dimension->forEachPlayer([&](Player& object) {
		if (object.getRuntimeID() != lp->getRuntimeID()) {
			result.push_back(&object);
		}
		return true;
	});
	return result;
}


ESP::ESP()
{
	addConfig(&renderMob, "renderMob");
	addConfig(&renderHealth, "renderHealth");
	addConfig(&renderBox, "renderBox");
	addConfig(&renderLine, "renderLine");

	ModuleType = "Render";
	UIType = 1;
}

const char* ESP::GetName()
{
	return "ESP";
}

void ESP::OnCmd(std::vector<std::string>* cmd)
{
	if ((*cmd)[0] == ".ESP") {
		if (cmd->size() < 5)return;
		if ((*cmd)[1] == "true") {
			renderMob = true;
		}
		else if ((*cmd)[1] == "false") {
			renderMob = false;
		}

		if ((*cmd)[2] == "true") {
			renderLine = true;
		}
		else if ((*cmd)[2] == "false") {
			renderLine = false;
		}

		if ((*cmd)[3] == "true") {
			renderBox = true;
		}
		else if ((*cmd)[3] == "false") {
			renderBox = false;
		}

		if ((*cmd)[4] == "true") {
			renderHealth = true;
		}
		else if ((*cmd)[4] == "false") {
			renderHealth = false;
		}
	}
}


void ESP::createTextView(std::string const& name, bool* b)
{
	Android::TextView* tv = mAndroid->newTextView();
	UIUtils::updateTextViewData(tv, name, "#FF0000", 19);
	tv->setOnClickListener([=](Android::View*) {
		if (*b) {
			mAndroid->Toast(name + " disabled!");
			*b = false;
		}
		else {
			mAndroid->Toast(name + " enabled!");
			*b = true;
		}
	});
	SecondWindowList.push_back(tv);
}

void ESP::initViews() {
	createTextView("renderMob", &renderMob);
	createTextView("renderLine", &renderLine);
	createTextView("renderBox", &renderBox);
	createTextView("renderHP", &renderHealth);
}

#include "Utils.hpp"

void ESP::OnRender(MinecraftUIRenderContext* ctx)
{

	if (renderLine || renderBox || renderHealth) {

		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		if (lp) {
			auto list = fetchObjects();
			glmatrixf* matrix = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix();
			vec3_t cam((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
			vec2_t fov = mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getFov();
			float screenWidth = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenWdith;
			float screenHeight = mGameData.getNowMinecraftGame()->getPrimaryGuiData()->ScreenHeight;
			
			vec2_t display(screenWidth, screenHeight);
			for (auto a : list) {
				if (renderLine) {
					vec2_t start(screenWidth / 2.0f, screenHeight / 2.0f);
					vec2_t end;
					vec3_t origin((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
					mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix()->OWorldToScreen(origin, vec3_t(a->getPos()->x, a->getPos()->y, a->getPos()->z), end, fov, display);
					
					HackSDK::drawLine(*ctx, start, end, 0.45f, mce::Color(0.0f, 1.0f, 0.0f));
				}
				if (renderHealth) {
					vec2_t text;
					vec3_t origin((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);
					mGameData.getNowMinecraftGame()->getPrimaryClientInstance()->getMatrix()->OWorldToScreen(origin, vec3_t(a->getPos()->x, a->getPos()->y +1.5f, a->getPos()->z), text, fov, display);
					if (text.x > 0.01f || text.y > 0.01f) {
						text.x = text.x - 8.0f;
						std::stringstream os;
						os << "§l§b" << a->getHealth() << "/" << a->getMaxHealth();
						HackSDK::drawText(*ctx, text, os.str(), mce::Color(0, 0, 0), mGameData.getNowMinecraftGame()->getUnicodeFontHandle()->getFont());
					}
				}
			}
		}
	}
}

void ESP::OnRender3D(ScreenContext* ctx)
{
	if (renderBox) {

		LocalPlayer* lp = mGameData.getNowMinecraftGame()->getPrimaryLocalPlayer();
		if (lp) {
			vec3_t origin((lp->PosData.from.x + lp->PosData.to.x) / 2, (lp->PosData.from.y + lp->PosData.to.y) / 2 + 0.75f, (lp->PosData.from.z + lp->PosData.to.z) / 2);

			auto list = fetchObjects();
			for (auto a : list) {
				HackSDK::drawBoxReal3D(*ctx, vec3_t(a->PosData.from.x, a->PosData.from.y, a->PosData.from.z), vec3_t(a->PosData.to.x, a->PosData.to.y, a->PosData.to.z), mce::Color(1.0f, 0.0f, 0.0f), origin);
			}
		}
	}
}
