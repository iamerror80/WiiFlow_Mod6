
#include "menu.hpp"
#include "loader/cios.h"

extern const u8 english_txt[];
static const wstringEx ENGLISH_TXT_W((const char*)english_txt);

s16 m_aboutLblInfo;
s16 m_aboutLblUser[4];
s16 m_aboutLblIOS;

bool showHelp;
int thanks_h;

void CMenu::_about(bool help)
{
	showHelp = help;
	int pixels_to_skip = 10;
	int amount_of_skips = 0;
	int xtra_skips = 0;
	int thanks_th = 0;
	
	_setBg(m_configBg, m_configBg);

	SetupInput();
	_textAbout();
	m_btnMgr.reset(m_aboutLblInfo, true);
	_showAbout();
	m_btnMgr.getTotalHeight(m_aboutLblInfo, thanks_th);
	m_btnMgr.moveBy(m_aboutLblInfo, 0, -1);

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED || BTN_B_OR_1_PRESSED || (BTN_A_OR_2_PRESSED && m_btnMgr.selected(m_configBtnBack)))
			break;
		else if((BTN_DOWN_PRESSED || BTN_DOWN_HELD || BTN_LEFT_PRESSED || BTN_LEFT_HELD) && thanks_th > thanks_h)
		{
			if((thanks_th - amount_of_skips * pixels_to_skip) >= thanks_h)
			{
				m_btnMgr.moveBy(m_aboutLblInfo, 0, -pixels_to_skip);
				amount_of_skips++;
			}
			else if((thanks_th - amount_of_skips * pixels_to_skip) < thanks_h && xtra_skips == 0)
			{
				xtra_skips = pixels_to_skip - ((thanks_th - amount_of_skips * pixels_to_skip) - thanks_h);
				m_btnMgr.moveBy(m_aboutLblInfo, 0, -xtra_skips);
			}
		}
		else if(BTN_UP_PRESSED || BTN_UP_HELD || BTN_RIGHT_PRESSED || BTN_RIGHT_HELD)
		{
			if(xtra_skips > 0)
			{
				m_btnMgr.moveBy(m_aboutLblInfo, 0, xtra_skips);
				xtra_skips = 0;
			}
			else if (amount_of_skips > 0)
			{
				m_btnMgr.moveBy(m_aboutLblInfo, 0, pixels_to_skip);
				amount_of_skips--;
			}
		}
	}
	_hideAbout(true);
}

void CMenu::_hideAbout(bool instant)
{
	m_btnMgr.hide(m_configLblTitle, instant);
	m_btnMgr.hide(m_configBtnBack, instant);
	m_btnMgr.hide(m_aboutLblIOS, instant);
	m_btnMgr.hide(m_aboutLblInfo, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
		if(m_aboutLblUser[i] != -1)
			m_btnMgr.hide(m_aboutLblUser[i], instant);
}

void CMenu::_showAbout(void)
{
	m_btnMgr.show(m_configLblTitle);
	m_btnMgr.show(m_configBtnBack);
	if(!m_txt_view)
		m_btnMgr.show(m_aboutLblIOS);
	m_btnMgr.show(m_aboutLblInfo,false);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_aboutLblUser); ++i)
		if(m_aboutLblUser[i] != -1)
			m_btnMgr.show(m_aboutLblUser[i]);
}

void CMenu::_initAboutMenu()
{
	_addUserLabels(m_aboutLblUser, 0, 2, "ABOUT");

	m_aboutLblInfo = _addText("ABOUT/INFO", theme.txtFont, L"", 40, 95, 560, 285, theme.txtFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP);
	_addUserLabels(m_aboutLblUser, 2, 2, "ABOUT");

	m_aboutLblIOS = _addLabel("ABOUT/IOS", theme.lblFont, L"", 40, 415, 180, 24, theme.lblFontColor, FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE);

	_setHideAnim(m_aboutLblInfo, "ABOUT/INFO", 0, 100, 0.f, 0.f);
	_setHideAnim(m_aboutLblIOS, "ABOUT/IOS", 0, 0, -2.f, 0.f);

	// _hideAbout(true);
	thanks_h = m_theme.getInt("ABOUT/INFO", "height", 290);
}

void CMenu::_textAbout(void)
{
	/* Show text file (in file explorer) */
	if(m_txt_view)
	{
		wstringEx txt_file_name;
		txt_file_name.fromUTF8(strrchr(m_txt_path, '/') + 1);
		m_btnMgr.setText(m_configLblTitle, txt_file_name);
		wstringEx txt_file_content;
		u32 txt_size = 0;
		char *txt_mem = (char*)MEM2_alloc(MAX_MSG_SIZE);
		txt_mem = (char*)fsop_ReadFile(m_txt_path, &txt_size);
		if(txt_mem != NULL)
		{
			if(*(txt_mem+txt_size) != '\0')
				*(txt_mem+txt_size) = '\0';
			txt_file_content.fromUTF8(txt_mem);
			m_btnMgr.setText(m_aboutLblInfo, txt_file_content);
			MEM2_free(txt_mem);
		}
		txt_mem = NULL;
		return;
	}
	
	/* Show help guide */
	if(showHelp) 
	{
		m_btnMgr.setText(m_configLblTitle, _t("about10", L"Help guide"));
		wstringEx help_text;
		u32 txt_size = 0;
		char *txt_mem = (char*)MEM2_alloc(MAX_MSG_SIZE);
		txt_mem = (char*)fsop_ReadFile(fmt("%s/%s.txt", m_helpDir.c_str(), lowerCase(m_curLanguage).c_str()), &txt_size);
		if(txt_mem != NULL)
		{
			if(*(txt_mem+txt_size) != '\0')
				*(txt_mem+txt_size) = '\0';
			help_text.fromUTF8(txt_mem);
			m_btnMgr.setText(m_aboutLblInfo, help_text);
			MEM2_free(txt_mem);
		}
		else
			m_btnMgr.setText(m_aboutLblInfo, ENGLISH_TXT_W);
		txt_mem = NULL;
		//! Display current video mode
		string s = sfmt("%s %c0Hz", CONF_GetVideo() == CONF_VIDEO_PAL ? "PAL" : "NTSC", (CONF_GetEuRGB60() || CONF_GetVideo() != CONF_VIDEO_PAL) ? '6' : '5');
		m_btnMgr.setText(m_aboutLblIOS, s);
		m_btnMgr.show(m_aboutLblIOS);
		return; 
	}

	/* Show credits and current cIOS */
	m_btnMgr.setText(m_configLblTitle, wfmt(L"%s %s", APP_NAME, APP_VERSION));

	wstringEx developers(wfmt(_fmt("about6", L"Current Developers:\n%s"), DEVELOPERS));
	wstringEx pDevelopers(wfmt(_fmt("about7", L"Past Developers:\n%s"), PAST_DEVELOPERS));

	wstringEx origLoader(wfmt(_fmt("about1", L"Original Loader By:\n%s"), LOADER_AUTHOR));
	wstringEx origGUI(wfmt(_fmt("about2", L"Original GUI By:\n%s"), GUI_AUTHOR));

	wstringEx codethx(wfmt(_fmt("about8", L"Bits of Code Obtained From:\n%s"), THANKS_CODE));
	wstringEx sites(wfmt(_fmt("about9", L"Supporting Websites:\n%s"), THANKS_SITES));

	wstringEx translator(wfmt(L"%s", m_loc.getWString(m_curLanguage, "translation_author").toUTF8().c_str()));
	wstringEx thanks(wfmt(_fmt("about4", L"Thanks To:\n%s"), THANKS));
	if(translator.size() > 3)
		thanks.append(translator);

	m_btnMgr.setText(m_aboutLblInfo,
		wfmt(L"%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s\n\n%s",
		developers.toUTF8().c_str(),
		pDevelopers.toUTF8().c_str(),
		origLoader.toUTF8().c_str(),
		origGUI.toUTF8().c_str(),
		codethx.toUTF8().c_str(),
		sites.toUTF8().c_str(),
		thanks.toUTF8().c_str())
	);
	const char *IOS_Name = NULL;
	switch(IOS_GetType(CurrentIOS.Version))
	{
		case IOS_TYPE_D2X:
			IOS_Name = "D2X";
			break;
		case IOS_TYPE_WANIN:
			IOS_Name = "Waninkoko";
			break;
		case IOS_TYPE_HERMES:
		case IOS_TYPE_KWIIRK:
			IOS_Name = "Hermes";
			break;
		default:
			break;
	}
	if(IOS_Name == NULL)
		m_btnMgr.setText(m_aboutLblIOS, wfmt(L"IOS%i v%i", CurrentIOS.Version, 
			CurrentIOS.Revision));
	else
		m_btnMgr.setText(m_aboutLblIOS, wfmt(L"%s IOS%i[%i] v%d.%d", IOS_Name, CurrentIOS.Version,
			CurrentIOS.Base, CurrentIOS.Revision, CurrentIOS.SubRevision));
}
