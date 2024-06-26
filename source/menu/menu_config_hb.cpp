
#include "menu.hpp"

static u8 curPage;

void CMenu::_showConfigHB(bool instant)
{
	_hideCheckboxes(true); // reset checkboxes
	m_btnMgr.show(m_configBtnBack, instant);

	for(u8 i = 0; i < ARRAY_SIZE(m_configLblUser); ++i)
		if(m_configLblUser[i] != -1)
			m_btnMgr.show(m_configLblUser[i]);
	
	/** MAIN PAGE **/
	if(curPage == MAIN_SETTINGS)
	{
		m_btnMgr.setText(m_configLblTitle, _t("homebrew", L"Homebrew"));
		m_btnMgr.show(m_configLblTitle);

		//! Refresh coverflow list and cover cache
		m_btnMgr.setText(m_configLbl[2], _t("home2", L"Refresh coverflow list"));
		m_btnMgr.setText(m_configBtn[2], _t("cfgne6", L"Start"));
		//! Install HB app using FTP
		m_btnMgr.setText(m_configLbl[3], _t("cfg850", L"Install homebrew app via FTP"));
		m_btnMgr.show(m_configBtnGo[3], instant);
		//! Homebrew app list
		m_btnMgr.setText(m_configLbl[4], _t("cfg817", L"Manage Homebrew app list"));	
		//! Adjust homebrew CF
		m_btnMgr.setText(m_configLbl[5], _t("cfgc4", L"Adjust coverflow"));
		//! CF box mode
		m_btnMgr.setText(m_configLbl[6], _t("cfg726", L"Covers box mode"));
		m_checkboxBtn[6] = m_cfg.getOptBool(homebrew_domain, "box_mode", 0) == 0 ? m_configChkOff[6] : m_configChkOn[6]; // default false
		//! CF smallbox
		m_btnMgr.setText(m_configLbl[7], _t("cfghb2", L"Coverflow smallbox"));
		m_checkboxBtn[7] = m_cfg.getOptBool(homebrew_domain, "smallbox", 1) == 0 ? m_configChkOff[7] : m_configChkOn[7]; // default true

		for(u8 i = 2; i < 8; ++i)
		{
			m_btnMgr.show(m_configLbl[i], instant);
			if(i == 2)
				m_btnMgr.show(m_configBtn[i], instant);
			else if(i < 6)
				m_btnMgr.show(m_configBtnGo[i], instant);
			else
				m_btnMgr.show(m_checkboxBtn[i], instant);
		}
	}
	/** HOMEBREW APP LIST **/
	else if(curPage == GAME_LIST)
	{
		m_btnMgr.setText(m_configLblTitle, _t("cfg817", L"Manage Homebrew app list"));
		m_btnMgr.show(m_configLblTitle);

		//! Homebrew partition
		m_btnMgr.setText(m_configLbl[4], _t("cfghb3", L"Homebrew partition"));
		const char *partitionname = DeviceName[m_cfg.getInt(homebrew_domain, "partition", 0)];
		m_btnMgr.setText(m_configLblVal[4], upperCase(partitionname));
		//! Dump homebrew app coverflow list
		m_btnMgr.setText(m_configLbl[5], _t("cfg783", L"Dump coverflow list"));
		m_btnMgr.setText(m_configBtn[5], _t("cfgne6", L"Start"));

		for(u8 i = 4; i < 6; ++i)
		{
			m_btnMgr.show(m_configLbl[i], instant);
			if(i == 4)
			{
				m_btnMgr.show(m_configLblVal[i], instant);
				m_btnMgr.show(m_configBtnM[i], instant);
				m_btnMgr.show(m_configBtnP[i], instant);
			}
			else
				m_btnMgr.show(m_configBtn[i], instant);
		}
	}
}

void CMenu::_configHB(void)
{
	curPage = MAIN_SETTINGS;
	
	SetupInput();
	_showConfigHB();

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
				 _showConfigHB();
			}
		}
		else if(BTN_A_OR_2_PRESSED)
		{
			if((m_btnMgr.selected(m_configBtnBack) && curPage != MAIN_SETTINGS))
			{
				_hideConfig(true);
				curPage = MAIN_SETTINGS;
				 _showConfigHB();
			}
			/** MAIN PAGE **/
			else if(curPage == MAIN_SETTINGS)
			{
				if(m_btnMgr.selected(m_configBtnBack))
					break;
				else
				{
					if(m_btnMgr.selected(m_configBtn[2])) // refresh list
					{
						m_cfg.setBool(homebrew_domain, "update_cache", true);
						if(m_current_view & COVERFLOW_PLUGIN)
							m_cfg.setBool(plugin_domain, "update_cache", true);
						m_refreshGameList = true;
						break;
					}
					else if(m_btnMgr.selected(m_configBtnGo[3]))
					{
						_hideConfig(true);
						_FTP();
						_showConfigHB();
					}
					else if(m_btnMgr.selected(m_configBtnGo[4])) // game list
					{
						_hideConfig(true);
						curPage = GAME_LIST;
						_showConfigHB();
					}
					else if(m_btnMgr.selected(m_configBtnGo[5])) // adjust CF
					{
						_hideConfig();
						m_prev_view = m_current_view;
						m_current_view = COVERFLOW_HOMEBREW;
						_showCF(true);
						_hideMain(true); // quick fix
						CoverFlow.fade(0);
						if(_cfTheme())
						{
							m_exit = true;
							m_reload = true;
							break; // reboot if CF was modified due to possible memory leak with cf_theme
						}
						m_current_view = m_prev_view;
						_showCF(true);
						CoverFlow.fade(2); // to avoid CF flashing after _showCF()
						_hideMain(true); // quick fix
						_showConfigHB();
					}
					else if(m_btnMgr.selected(m_checkboxBtn[6])) // box mode
					{
						m_refreshGameList = true;
						m_cfg.setBool(homebrew_domain, "box_mode", !m_cfg.getBool(homebrew_domain, "box_mode"));
						_showConfigHB(true);
						m_btnMgr.setSelected(m_checkboxBtn[6]);
					}
					else if(m_btnMgr.selected(m_checkboxBtn[7])) // small box
					{
						m_refreshGameList = true;
						m_cfg.setBool(homebrew_domain, "update_cache", true);
						m_cfg.setBool(homebrew_domain, "smallbox", !m_cfg.getBool(homebrew_domain, "smallbox"));
						_showConfigHB(true);
						m_btnMgr.setSelected(m_checkboxBtn[7]);
					}
				}
			}
			/** HOMEBREW APP LIST **/
			else if(curPage == GAME_LIST)
			{
				if(m_btnMgr.selected(m_configBtnP[4]) || m_btnMgr.selected(m_configBtnM[4])) // HB partition
				{
					s8 direction = m_btnMgr.selected(m_configBtnP[4]) ? 1 : -1;
					_setPartition(direction, COVERFLOW_HOMEBREW);
					if(m_current_view & COVERFLOW_HOMEBREW || (m_current_view & COVERFLOW_PLUGIN && m_plugin.GetEnabledStatus(HB_PMAGIC)))
						m_refreshGameList = true;
					_showConfigHB(true);
				}
				else if(m_btnMgr.selected(m_configBtn[5])) // dump list
				{
					_dumpGameList();
					_showConfigHB();
				}

			}
		}
	}
	_hideConfig(true);
}
