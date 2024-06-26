
#include "menu.hpp"

s16 m_codeBtnKey[10];
s16 m_codeLblUser[4];

bool CMenu::_code(char code[4])
{
	u8 n = 0;
	wchar_t codeLbl[] = L"_ _ _ _";
	memset(code, 0, 4);
	bool erase = false;
	
	SetupInput();
	
	m_btnMgr.show(m_configBtnBack);
	m_btnMgr.setText(m_configLblTitle, codeLbl);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if(m_codeLblUser[i] != -1)
			m_btnMgr.show(m_codeLblUser[i]);
	
	if(m_locked)
		m_btnMgr.setText(m_configBtnCenter, _t("cfg835", L"Disable"));
	else
	{
		if(m_cfg.getString(general_domain, "parent_code", "") == "")
			m_btnMgr.setText(m_configBtnCenter, _t("cfg834", L"Enable"));
		else
		{
			erase = true;
			m_btnMgr.setText(m_configBtnCenter, _t("cd2", L"Erase"));
			m_btnMgr.setText(m_configLblDialog, _t("cfg836", L"Remove child lock?"));
			m_btnMgr.show(m_configLblDialog);
		}
	}
	m_btnMgr.show(m_configBtnCenter);
	
	if(!erase)
	{
		m_btnMgr.show(m_configLblTitle);
		for(u8 i = 0; i < 10; ++i)
			m_btnMgr.show(m_codeBtnKey[i]);
	}

	while(!m_exit)
	{
		_mainLoopCommon();
		if(BTN_HOME_PRESSED)
		{
			n = 0;
			break;
		}
		else if(ShowPointer() || erase || n == 4)
		{
			if(BTN_LEFT_REV_PRESSED || BTN_UP_PRESSED)
				m_btnMgr.up();
			else if(BTN_RIGHT_REV_PRESSED || BTN_DOWN_PRESSED)
				m_btnMgr.down();
			else if(BTN_B_OR_1_PRESSED)
			{
				n = 0;
				break;
			}
			if(BTN_A_OR_2_PRESSED)
			{
				if(m_btnMgr.selected(m_configBtnBack))
				{
					n = 0;
					break;
				}
				else if(m_btnMgr.selected(m_configBtnCenter))
				{
					if(erase)
					{
						memset(code, 0, 4);
						m_cfg.remove(general_domain, "parent_code");
						n = 0;
						break;
					}
					else if(n == 4)
						break;
				}
				else if(n < 4 && !erase)
					for(int i = 0; i < 10; ++i)
						if(m_btnMgr.selected(m_codeBtnKey[i]))
						{
							codeLbl[n * 2] = 'X';
							code[n++] = '0' + i;
							m_btnMgr.setText(m_configLblTitle, codeLbl);
							break;
						}
			}
		}
		else // (!ShowPointer() && !erase && n < 4)
		{
			// Map buttons to numbers
			int c = -1;
			if(BTN_UP_PRESSED) c = 0;
			else if(BTN_LEFT_PRESSED) c = 1;
			else if(BTN_RIGHT_PRESSED) c = 2;
			else if(BTN_DOWN_PRESSED) c = 3;
			else if(BTN_MINUS_PRESSED) c = 4;
			else if(BTN_PLUS_PRESSED) c = 5;
			else if(BTN_A_PRESSED) c = 6;
			else if(BTN_B_PRESSED) c = 7;
			else if(BTN_1_PRESSED) c = 8;
			else if(BTN_2_PRESSED) c = 9;
			
			if(c != -1)
			{
				codeLbl[n * 2] = 'X';
				code[n++] = '0' + c;
				m_btnMgr.setText(m_configLblTitle, codeLbl);
			}
		}
	}
	
	_hideConfigFull(true);
	for(u8 i = 0; i < 10; ++i)
		m_btnMgr.hide(m_codeBtnKey[i]);
	
	for(u8 i = 0; i < ARRAY_SIZE(m_codeLblUser); ++i)
		if(m_codeLblUser[i] != -1)
			m_btnMgr.hide(m_codeLblUser[i]);

	return n == 4;
}

void CMenu::_initCodeMenu()
{
	_addUserLabels(m_codeLblUser, ARRAY_SIZE(m_codeLblUser), "CODE");

	m_codeBtnKey[0] = _addButton("CODE/0_BTN", theme.btnFont, L"0", 270, 320, 100, 50, theme.btnFontColor);

	for(int i = 0; i < 10; ++i)
	{
		char *codeText = fmt_malloc("CODE/%i_BTN", i);
		if(codeText == NULL) 
			continue;
		if(i > 0)
		{
			int x = i - 1;
			int y = x / 3;
			x %= 3;
			x = 160 + x * 110;
			y = 240 - y * 80;
			m_codeBtnKey[i] = _addButton(codeText, theme.btnFont, wfmt(L"%i", i), x, y, 100, 50, theme.btnFontColor);
		}
		_setHideAnim(m_codeBtnKey[i], codeText, 0, 0, 0.f, 0.f);
		MEM2_free(codeText);
	}
	// _hideCode(true);
}
