
#include "menu.hpp"

#define NB_BUTTONS 4 // 4 not 2

static u8 curPage;
static u8 start_pos;
static u8 WiiBtn;
static u8 GCBtn;
static u8 ChanBtn;
static u8 HBBtn;
static u8 PlBtn;
static u8 PlBtn2;

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

void CMenu::_showConfigPlugin(bool instant)
{
	_hideCheckboxes(true); // reset checkboxes
	m_btnMgr.show(m_configBtnBack);

	//! check if wii or channel or gc or hb plugin is enabled to get the onscreen array size	
	u8 max_line = NB_BUTTONS - 1;
	bool wii_enabled = false;
	bool chan_enabled = false;
	bool gc_enabled= false;
	bool hb_enabled = false;
	bool pl_enabled = false;
	WiiBtn = 0;
	GCBtn = 0;
	ChanBtn = 0;
	HBBtn = 0;
	PlBtn = 0;
	PlBtn2 = 0;
	
	if(m_plugin.GetEnabledStatus(WII_PMAGIC)) // plugin wii
	{
		++max_line;
		wii_enabled = true;
	}
	if(m_plugin.GetEnabledStatus(ENAND_PMAGIC) || m_plugin.GetEnabledStatus(NAND_PMAGIC)) // plugin channels (emunand + realnand)
	{
		++max_line;
		chan_enabled = true;
	}
	if(m_plugin.GetEnabledStatus(GC_PMAGIC)) // plugin gamecube
	{
		++max_line;
		gc_enabled = true;
	}
	if(m_plugin.GetEnabledStatus(HB_PMAGIC)) // plugin homebrew
	{
		++max_line;
		hb_enabled = true;
	}
	u8 pos = 0;
	if(enabledPluginsCount == 1)
	{
		while(m_plugin.PluginExist(pos) && !m_plugin.GetEnabledStatus(pos)) { ++pos; }
		u32 magic = m_plugin.GetPluginMagic(pos);
		//! if magic is not wii, gc, emunand, realnand, scumm or hb
		if(magic != WII_PMAGICN && magic != GC_PMAGICN && magic != ENAND_PMAGICN && magic != NAND_PMAGICN && magic != SCUMM_PMAGICN && magic != HB_PMAGICN)
		{
			max_line = max_line + 2;
			pl_enabled = true;
		}
	}
	
	start_pos = 4 - (max_line / 2);
	if(wii_enabled)
		WiiBtn = start_pos + NB_BUTTONS;
	if(chan_enabled)
		ChanBtn = start_pos + NB_BUTTONS + wii_enabled;
	if(gc_enabled)
		GCBtn = start_pos + NB_BUTTONS + wii_enabled + chan_enabled;
	if(hb_enabled)
		HBBtn = start_pos + NB_BUTTONS + wii_enabled + chan_enabled + gc_enabled;
	if(pl_enabled)
	{
		PlBtn = start_pos + NB_BUTTONS + wii_enabled + chan_enabled + gc_enabled + hb_enabled;
		// PlBtn2 = PlBtn + 1;
	}

	for(u8 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if(m_configLblUser[i] != -1)
			m_btnMgr.show(m_configLblUser[i]);
	
	/** MAIN PAGE **/
	if(curPage == MAIN_SETTINGS)
	{
		m_btnMgr.setText(m_configLblTitle, _t("plugins", L"Plugins"));
		m_btnMgr.show(m_configLblTitle);
		
		//! Refresh coverflow list and cover cache
		m_btnMgr.setText(m_configLbl[start_pos], _t("home2", L"Refresh coverflow list"));
		m_btnMgr.setText(m_configBtn[start_pos], _t("cfgne6", L"Start"));
		//! Install plugin game using FTP
		m_btnMgr.setText(m_configLbl[start_pos + 1], _t("cfg849", L"Install game via FTP"));
		//! Game location
		m_btnMgr.setText(m_configLbl[start_pos + 2], _t("cfg816", L"Manage Plugin game list"));
		//! Select plugins
		m_btnMgr.setText(m_configLbl[start_pos + 3], _t("cfgpl1", L"Select plugins"));		

		for(u8 i = start_pos; i < (start_pos + 4); ++i)
		{
			m_btnMgr.show(m_configLbl[i], instant);
			if(i == start_pos)
				m_btnMgr.show(m_configBtn[i], instant);
			else
				m_btnMgr.show(m_configBtnGo[i], instant);
		}

		//! The following buttons will only show up if their corresponding plugin is selected
		if(wii_enabled) // shortcut to wii settings
		{
			m_btnMgr.setText(m_configLbl[WiiBtn], _t("wii", L"Wii"));
			m_btnMgr.show(m_configLbl[WiiBtn], instant);
			m_btnMgr.show(m_configBtnGo[WiiBtn], instant);
		}
		if(chan_enabled) // shortcut to channels settings
		{
			m_btnMgr.setText(m_configLbl[ChanBtn], _t("wiichannels", L"Wii channels"));
			m_btnMgr.show(m_configLbl[ChanBtn], instant);
			m_btnMgr.show(m_configBtnGo[ChanBtn], instant);
		}
		if(gc_enabled) // shortcut to gamecube settings
		{
			m_btnMgr.setText(m_configLbl[GCBtn], _t("gc", L"GameCube"));
			m_btnMgr.show(m_configLbl[GCBtn], instant);
			m_btnMgr.show(m_configBtnGo[GCBtn], instant);
		}
		if(hb_enabled) // shortcut to homebrew settings
		{
			m_btnMgr.setText(m_configLbl[HBBtn], _t("homebrew", L"Homebrew"));
			m_btnMgr.show(m_configLbl[HBBtn], instant);
			m_btnMgr.show(m_configBtnGo[HBBtn], instant);
		}
		if(pl_enabled) // shortcut to edit current enabled plugin
		{
			string pluginName = m_plugin.GetPluginName(pos).toUTF8();
			if(pluginName.size() > 25)
				pluginName = pluginName.substr(0, 25) + "...";
			m_btnMgr.setText(m_configLbl[PlBtn], wfmt(_fmt("cfg844", L"Edit %s"), pluginName.c_str()));
			m_btnMgr.show(m_configLbl[PlBtn], instant);
			m_btnMgr.show(m_configBtnGo[PlBtn], instant);
		}
	}
	/** PLUGIN GAME LOCATION **/
	else if(curPage == GAME_LIST)
	{
		m_btnMgr.setText(m_configLblTitle, _t("cfg816", L"Manage Plugin game list"));
		m_btnMgr.show(m_configLblTitle);
		//! Plugin default partition
		m_btnMgr.setText(m_configLbl[3], _t("part4", L"Plugins default partition"));
		const char *partitionname = DeviceName[m_cfg.getInt(plugin_domain, "partition", 0)];
		m_btnMgr.setText(m_configLblVal[3], upperCase(partitionname));
		//! Edit plugins
		m_btnMgr.setText(m_configLbl[4], _t("smedit7", L"Edit plugins"));
		//! Use plugin database names
		m_btnMgr.setText(m_configLbl[5], _t("cfg727", L"Use plugin database titles"));
		m_checkboxBtn[5] = m_cfg.getOptBool(plugin_domain, "database_titles", 1) == 0 ? m_configChkOff[5] : m_configChkOn[5]; // default true
		//! Dump plugin game coverflow list
		m_btnMgr.setText(m_configLbl[6], _t("cfg783", L"Dump coverflow list"));
		m_btnMgr.setText(m_configBtn[6], _t("cfgne6", L"Start"));

		for(u8 i = 3; i < 7; ++i)
		{
			m_btnMgr.show(m_configLbl[i], instant);
			if(i == 3)
			{
				m_btnMgr.show(m_configLblVal[i], instant);
				m_btnMgr.show(m_configBtnM[i], instant);
				m_btnMgr.show(m_configBtnP[i], instant);
			}
			else if(i == 4)
				m_btnMgr.show(m_configBtnGo[i], instant);
			else if(i == 5)
				m_btnMgr.show(m_checkboxBtn[i], instant);
			else
				m_btnMgr.show(m_configBtn[i], instant);
		}
	}
}

void CMenu::_configPlugin(void)
{
	bool cur_db_titles = m_cfg.getBool(plugin_domain, "database_titles", true);
	bool prev_db_titles = cur_db_titles;
	curPage = MAIN_SETTINGS;
	start_pos = 0;

	SetupInput();
	_showConfigPlugin();
	
	while(!m_exit)
	{
		_mainLoopCommon(true);
		if(BTN_HOME_HELD)
			break;
		else if(BTN_LEFT_REV_PRESSED || BTN_UP_PRESSED)
			m_btnMgr.up();
		else if(BTN_RIGHT_REV_PRESSED || BTN_DOWN_PRESSED)
			m_btnMgr.down();
		else if(BTN_B_OR_1_PRESSED)
		{
			if(curPage == MAIN_SETTINGS)
				break;
			else
			{
				_hideConfig(true);
				curPage = MAIN_SETTINGS;
				 _showConfigPlugin();
			}
		}
		else if(BTN_A_OR_2_PRESSED)
		{
			if((m_btnMgr.selected(m_configBtnBack) && curPage != MAIN_SETTINGS))
			{
				_hideConfig(true);
				curPage = MAIN_SETTINGS;
				 _showConfigPlugin();
			}
			/** MAIN PAGE **/
			else if(curPage == MAIN_SETTINGS)
			{
				if(m_btnMgr.selected(m_configBtnBack))
					break;
				else
				{
					if(m_btnMgr.selected(m_configBtn[start_pos])) // refresh list
					{
						m_cfg.setBool(plugin_domain, "update_cache", true);
						m_refreshGameList = true;
						break;
					}
					else if(m_btnMgr.selected(m_configBtnGo[start_pos + 1]))
					{
						_hideConfig(true);
						_FTP();
						_showConfigPlugin();
					}
					else if(m_btnMgr.selected(m_configBtnGo[start_pos + 2])) //  game location
					{
						_hideConfig(true);
						curPage = GAME_LIST;
						_showConfigPlugin();
					}
					else if(m_btnMgr.selected(m_configBtnGo[start_pos + 3])) // select plugins
					{
						_hideConfig(true);
						_setBg(m_configBg, m_configBg);
						_PluginSettings();
						_setMainBg();
						_showConfigPlugin();
					}
					else if(m_btnMgr.selected(m_configBtnGo[WiiBtn])) // wii settings
					{
						_hideConfig(true);
						_configWii();
						if(m_refreshGameList)
							break;
						_showConfigPlugin();
					}
					else if(m_btnMgr.selected(m_configBtnGo[ChanBtn])) // channels settings
					{
						_hideConfig(true);
						_configNandEmu();
						if(m_refreshGameList)
							break;
						_showConfigPlugin();
					}
					else if(m_btnMgr.selected(m_configBtnGo[GCBtn])) // gamecube settings
					{
						_hideConfig(true);
						_configGC();
						if(m_refreshGameList)
							break;
						_showConfigPlugin();
					}
					else if(m_btnMgr.selected(m_configBtnGo[HBBtn])) // homebrew settings
					{
						_hideConfig(true);
						_configHB();
						if(m_refreshGameList)
							break;
						_showConfigPlugin();
					}
					else if(m_btnMgr.selected(m_configBtnGo[PlBtn])) //  single plugin edition
					{
						_hideConfig(true);
						u8 pos = 0;
						while(m_plugin.PluginExist(pos) && !m_plugin.GetEnabledStatus(pos)) { ++pos; }
						_configPluginEditor(pos); //
						_showConfigPlugin();
					}
				}
			}
			/** PLUGIN GAME LOCATION **/
			else if(curPage == GAME_LIST)
			{
				if(m_btnMgr.selected(m_configBtnP[3]) || m_btnMgr.selected(m_configBtnM[3])) // plugin default partition
				{
					s8 direction = m_btnMgr.selected(m_configBtnP[3]) ? 1 : -1;
					_setPartition(direction, COVERFLOW_PLUGIN);
					if(m_current_view & COVERFLOW_PLUGIN)
						m_refreshGameList = true;
					_showConfigPlugin(true);
				}
				else if(m_btnMgr.selected(m_configBtnGo[4])) // Edit plugins
				{
					_hideConfig(true);
					_setBg(m_configBg, m_configBg);
					_checkboxesMenu(4); // SM editor mode 4 (EDIT_PLUGIN)
					_setMainBg();
					_showConfigPlugin();
				}
				else if(m_btnMgr.selected(m_checkboxBtn[5])) // plugin database titles
				{
					cur_db_titles = !cur_db_titles;
					m_cfg.setBool(plugin_domain, "database_titles", cur_db_titles);
					_showConfigPlugin(true);
					m_btnMgr.setSelected(m_checkboxBtn[5]);
				}
				else if(m_btnMgr.selected(m_configBtn[6])) // dump list
				{
					_dumpGameList();
					_showConfigPlugin();
				}
			}
		}
	}

	if(prev_db_titles != cur_db_titles)
	{
		// delete all plugin cache files
		u8 pos = 0;
		while(m_plugin.PluginExist(pos))
		{
			strncpy(m_plugin.PluginMagicWord, fmt("%08x", m_plugin.GetPluginMagic(pos)), 8); //
			for(u8 i = 0; i < MAXDEVICES; ++i)
			{
				string cachedListFile(fmt("%s/%s_%s.db", m_listCacheDir.c_str(), DeviceName[i], m_plugin.PluginMagicWord));
				fsop_deleteFile(cachedListFile.c_str());
			}
			++pos;
		}
		if(m_current_view & COVERFLOW_PLUGIN)
			m_refreshGameList = true;
	}
	
	_hideConfig(true);
}
