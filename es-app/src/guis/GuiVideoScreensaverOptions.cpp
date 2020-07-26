#include "guis/GuiVideoScreensaverOptions.h"

#include "components/OptionListComponent.h"
#include "components/SliderComponent.h"
#include "components/SwitchComponent.h"
#include "guis/GuiMsgBox.h"
#include "Settings.h"

GuiVideoScreensaverOptions::GuiVideoScreensaverOptions(Window* window, const char* title) : GuiScreensaverOptions(window, title)
{
	// timeout to swap videos
	auto swap = std::make_shared<SliderComponent>(mWindow, 10.f, 1000.f, 1.f, "s");
	swap->setValue((float)(Settings::getInstance()->getInt("ScreenSaverSwapVideoTimeout") / (1000)));
	addWithLabel("다음 시간 후 동영상 변경(초)", swap);
	addSaveFunc([swap] {
		int playNextTimeout = (int)Math::round(swap->getValue()) * (1000);
		Settings::getInstance()->setInt("ScreenSaverSwapVideoTimeout", playNextTimeout);
		PowerSaver::updateTimeouts();
	});

	auto stretch_screensaver = std::make_shared<SwitchComponent>(mWindow);
	stretch_screensaver->setState(Settings::getInstance()->getBool("StretchVideoOnScreenSaver"));
	addWithLabel("화면보호기 동영상 크기 늘리기", stretch_screensaver);
	addSaveFunc([stretch_screensaver] { Settings::getInstance()->setBool("StretchVideoOnScreenSaver", stretch_screensaver->getState()); });

#ifdef _RPI_
	auto ss_omx = std::make_shared<SwitchComponent>(mWindow);
	ss_omx->setState(Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
	addWithLabel("화면보호기에 OMX 플레이어 사용", ss_omx);
	addSaveFunc([ss_omx, this] { Settings::getInstance()->setBool("ScreenSaverOmxPlayer", ss_omx->getState()); });
#endif

	// Render Video Game Name as subtitles
	auto ss_info = std::make_shared< OptionListComponent<std::string> >(mWindow, "게임 정보 표시", false);
	std::vector<std::string> info_type;
	info_type.push_back("always");
	info_type.push_back("start & end");
	info_type.push_back("never");
	for(auto it = info_type.cbegin(); it != info_type.cend(); it++)
		ss_info->add(*it, *it, Settings::getInstance()->getString("ScreenSaverGameInfo") == *it);
	addWithLabel("화면보호기에 게임 정보 표시", ss_info);
	addSaveFunc([ss_info, this] { Settings::getInstance()->setString("ScreenSaverGameInfo", ss_info->getSelected()); });

#ifdef _RPI_
	ComponentListRow row;

	// Set subtitle position
	auto ss_omx_subs_align = std::make_shared< OptionListComponent<std::string> >(mWindow, "GAME INFO ALIGNMENT", false);
	std::vector<std::string> align_mode;
	align_mode.push_back("left");
	align_mode.push_back("center");
	for(auto it = align_mode.cbegin(); it != align_mode.cend(); it++)
		ss_omx_subs_align->add(*it, *it, Settings::getInstance()->getString("SubtitleAlignment") == *it);
	addWithLabel("게임 정보 정렬", ss_omx_subs_align);
	addSaveFunc([ss_omx_subs_align, this] { Settings::getInstance()->setString("SubtitleAlignment", ss_omx_subs_align->getSelected()); });

	// Set font size
	auto ss_omx_font_size = std::make_shared<SliderComponent>(mWindow, 1.f, 64.f, 1.f, "h");
	ss_omx_font_size->setValue((float)(Settings::getInstance()->getInt("SubtitleSize")));
	addWithLabel("게임 정보 폰트 크기", ss_omx_font_size);
	addSaveFunc([ss_omx_font_size] {
		int subSize = (int)Math::round(ss_omx_font_size->getValue());
		Settings::getInstance()->setInt("SubtitleSize", subSize);
	});

	auto ss_video_mute = std::make_shared<SwitchComponent>(mWindow);
	ss_video_mute->setState(Settings::getInstance()->getBool("ScreenSaverVideoMute"));
	addWithLabel("MUTE SCREENSAVER AUDIO", ss_video_mute);
	addSaveFunc([ss_video_mute] { Settings::getInstance()->setBool("ScreenSaverVideoMute", ss_video_mute->getState()); });

	// Define subtitle font
	auto ss_omx_font_file = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, "폰트 파일 경로", ss_omx_font_file, Settings::getInstance()->getString("SubtitleFont"));
	addSaveFunc([ss_omx_font_file] {
		Settings::getInstance()->setString("SubtitleFont", ss_omx_font_file->getValue());
	});

	// Define subtitle italic font
	auto ss_omx_italic_font_file = std::make_shared<TextComponent>(mWindow, "", Font::get(FONT_SIZE_SMALL), 0x777777FF);
	addEditableTextComponent(row, "기울임꼴 폰트 경로", ss_omx_italic_font_file, Settings::getInstance()->getString("SubtitleItalicFont"));
	addSaveFunc([ss_omx_italic_font_file] {
		Settings::getInstance()->setString("SubtitleItalicFont", ss_omx_italic_font_file->getValue());
	});
#endif

#ifndef _RPI_
	auto captions_compatibility = std::make_shared<SwitchComponent>(mWindow);
	captions_compatibility->setState(Settings::getInstance()->getBool("CaptionsCompatibility"));
	addWithLabel("문자 표시를 위해 호환가능한 저해상도 사용", captions_compatibility);
	addSaveFunc([captions_compatibility] { Settings::getInstance()->setBool("CaptionsCompatibility", captions_compatibility->getState()); });
#endif
}

GuiVideoScreensaverOptions::~GuiVideoScreensaverOptions()
{
}

void GuiVideoScreensaverOptions::save()
{
#ifdef _RPI_
	bool startingStatusNotRisky = (Settings::getInstance()->getString("ScreenSaverGameInfo") == "never" || !Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
#endif
	GuiScreensaverOptions::save();

#ifdef _RPI_
	bool endStatusRisky = (Settings::getInstance()->getString("ScreenSaverGameInfo") != "never" && Settings::getInstance()->getBool("ScreenSaverOmxPlayer"));
	if (startingStatusNotRisky && endStatusRisky) {
		// if before it wasn't risky but now there's a risk of problems, show warning
		mWindow->pushGui(new GuiMsgBox(mWindow,
		"OMX 플레이어를 사용하며 게임 정보를 표시할 시 일부 TV에서 깜빡임이 발생합니다. 그런 일이 발생할 경우:\n\n-\"게임 정보 표시\"옵션을 끄거나;\n- 파이 설정에서 \"오버스캔\"을 끄면 도움이 될 수 있습니다:\nRetroPie > Raspi-Config > Advanced Options > Overscan > \"No\".\n- 화면보호기에 OMX 플레이어를 사용하지 않는 방법도 있습니다.",
			"알겠습니다!", [] { return; }));
	}
#endif
}
