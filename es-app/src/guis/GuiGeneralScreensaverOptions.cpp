#include "guis/GuiGeneralScreensaverOptions.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "guis/GuiSlideshowScreensaverOptions.h"
#include "guis/GuiVideoScreensaverOptions.h"
#include "Settings.h"

GuiGeneralScreensaverOptions::GuiGeneralScreensaverOptions(Window* window, const char* title) : GuiScreensaverOptions(window, title)
{
	// screensaver time
	auto screensaver_time = std::make_shared<SliderComponent>(mWindow, 0.f, 30.f, 1.f, "m");
	screensaver_time->setValue((float)(Settings::getInstance()->getInt("ScreenSaverTime") / Settings::ONE_MINUTE_IN_MS));
	addWithLabel("화면보호기 실행시간", screensaver_time);
	addSaveFunc([screensaver_time] {
		Settings::getInstance()->setInt("ScreenSaverTime", (int)Math::round(screensaver_time->getValue()) * Settings::ONE_MINUTE_IN_MS);
		PowerSaver::updateTimeouts();
	});

	// Allow ScreenSaver Controls - ScreenSaverControls
	auto ss_controls = std::make_shared<SwitchComponent>(mWindow);
	ss_controls->setState(Settings::getInstance()->getBool("ScreenSaverControls"));
	addWithLabel("화면보호기 제어", ss_controls);
	addSaveFunc([ss_controls] { Settings::getInstance()->setBool("ScreenSaverControls", ss_controls->getState()); });

	// screensaver behavior
	auto screensaver_behavior = std::make_shared< OptionListComponent<std::string> >(mWindow, "화면보호기 방식", false);
	std::vector<std::string> screensavers;
	screensavers.push_back("dim");
	screensavers.push_back("black");
	screensavers.push_back("random video");
	screensavers.push_back("slideshow");
	for(auto it = screensavers.cbegin(); it != screensavers.cend(); it++)
		screensaver_behavior->add(*it, *it, Settings::getInstance()->getString("ScreenSaverBehavior") == *it);
	addWithLabel("화면보호기 방식", screensaver_behavior);
	addSaveFunc([this, screensaver_behavior] {
		if (Settings::getInstance()->getString("ScreenSaverBehavior") != "random video" && screensaver_behavior->getSelected() == "random video") {
			// if before it wasn't risky but now there's a risk of problems, show warning
			mWindow->pushGui(new GuiMsgBox(mWindow,
			"\"무작위 동영상\" 화면보호기는 게임목록 상의 동영상을 보여줍니다.\n\n동영상이 없거나 목록에서 동영상이 없는 게임을 계속해서 선택될 경우 기본 검은 화면 보호기로 변경됩니다.\n\n\"UI 설정\" > \"동영상 화면보호기\" 메뉴에서 더 많은 옵션을 확인하세요.",
				"확인", [] { return; }));
		}
		Settings::getInstance()->setString("ScreenSaverBehavior", screensaver_behavior->getSelected());
		PowerSaver::updateTimeouts();
	});

	ComponentListRow row;

	// show filtered menu
	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, "동영상 화면보호기 설정", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.addElement(makeArrow(mWindow), false);
	row.makeAcceptInputHandler(std::bind(&GuiGeneralScreensaverOptions::openVideoScreensaverOptions, this));
	addRow(row);

	row.elements.clear();
	row.addElement(std::make_shared<TextComponent>(mWindow, "슬라이드쇼 화면보호기 설정", Font::get(FONT_SIZE_MEDIUM), 0x777777FF), true);
	row.addElement(makeArrow(mWindow), false);
	row.makeAcceptInputHandler(std::bind(&GuiGeneralScreensaverOptions::openSlideshowScreensaverOptions, this));
	addRow(row);

	// system sleep time
	float stepw = 5.f;
	float max =  120.f;
	auto system_sleep_time = std::make_shared<SliderComponent>(mWindow, 0.f, max, stepw, "m");
	system_sleep_time->setValue((float)(Settings::getInstance()->getInt("SystemSleepTime") / Settings::ONE_MINUTE_IN_MS));
	addWithLabel("다음 시간 후 시스템 슬립", system_sleep_time);
	addSaveFunc([this, system_sleep_time, screensaver_time, max, stepw] {
		if (screensaver_time->getValue() > system_sleep_time->getValue() && system_sleep_time->getValue() > 0) {
			int steps = Math::min(1 + (int)(screensaver_time->getValue() / stepw), (int)(max/stepw));
			int adj_system_sleep_time = steps*stepw;
			system_sleep_time->setValue((float)adj_system_sleep_time);
			std::string msg = "";
			if (!Settings::getInstance()->getBool("SystemSleepTimeHintDisplayed")) {
				msg += "참고: 시스템 슬립 시간을 설정하면 사용자 정의된 스크립트를 수행합니다.";
				msg += "\nRetropie/Emulationstation 위키에서 자세한 정보를 확인하세요.";
				Settings::getInstance()->setBool("SystemSleepTimeHintDisplayed", true);
			}
			if (msg.length() > 0) {
				msg += "\n\n";
			}
			msg += "시스템 슬립 지연이 설정되었으나 화면보호기 기동 시간보다 짧습니다.";
			msg	+= "\n\n시스템 슬립 시간을 " + std::to_string(adj_system_sleep_time) + " 분으로 변경해주세요.";
			mWindow->pushGui(new GuiMsgBox(mWindow, msg, "확인", [] { return; }));
		}
		Settings::getInstance()->setInt("SystemSleepTime", (int)Math::round(system_sleep_time->getValue()) * Settings::ONE_MINUTE_IN_MS);
	});
}

GuiGeneralScreensaverOptions::~GuiGeneralScreensaverOptions()
{
}

void GuiGeneralScreensaverOptions::openVideoScreensaverOptions() {
	mWindow->pushGui(new GuiVideoScreensaverOptions(mWindow, "동영상 화면보호기"));
}

void GuiGeneralScreensaverOptions::openSlideshowScreensaverOptions() {
    mWindow->pushGui(new GuiSlideshowScreensaverOptions(mWindow, "슬라이드쇼 화면보호기"));
}
