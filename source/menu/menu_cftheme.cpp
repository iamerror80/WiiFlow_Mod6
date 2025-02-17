
#include "menu.hpp"

#define BTNS_DELAY 100

s16 m_cfThemeBtnAlt;
s16 m_cfThemeBtnSelect;
s16 m_cfThemeBtnWide;
s16 m_cfThemeLblParam;
s16 m_cfThemeBtnParamM;
s16 m_cfThemeBtnParamP;
s16 m_cfThemeBtnCopy;
s16 m_cfThemeBtnPaste;
s16 m_cfThemeBtnSave;
s16 m_cfThemeBtnCancel;
s16 m_cfThemeLblVal[4 * 4];
s16 m_cfThemeBtnValM[4 * 4];
s16 m_cfThemeBtnValP[4 * 4];
s16 m_cfThemeLblValTxt[4];
s16 m_cfThemeLblUser[4];

const CMenu::SCFParamDesc CMenu::_cfParams[] = {
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D },
		CMenu::SCFParamDesc::PDD_BOTH, true, "Camera", { "Position", "Aim", "Oscillation speed", "Oscillation scale" },
		{ "camera_pos", "camera_aim", "camera_osc_speed", "camera_osc_amp" },
		{ 0.05f, 0.05f, 0.05f, 0.05f },
		{ { -15.f, 15.f }, { -15.f, 15.f }, { -15.f, 15.f }, { -15.f, 15.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D },
		CMenu::SCFParamDesc::PDD_BOTH, true, "Cover Position", { "Left", "Center", "Right", "Row Center" },
		{ "left_pos", "center_pos", "right_pos", "row_center_pos" },
		{ 0.05f, 0.05f, 0.05f, 0.05f },
		{ { -15.f, 15.f }, { -15.f, 15.f }, { -15.f, 15.f }, { -15.f, 15.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Cover Angle", { "Left", "Center", "Right", "Row Center" },
		{ "left_angle", "center_angle", "right_angle", "row_center_angle" },
		{ 1.f, 1.f, 1.f, 1.f },
		{ { -1080.f, 1080.f }, { -1080.f, 1080.f }, { -1080.f, 1080.f }, { -1080.f, 1080.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Cover Spacer", { "Left", "Right", "Left Relative Angle", "Right Relative Angle" },
		{ "left_spacer", "right_spacer", "left_delta_angle", "right_delta_angle" },
		{ 0.05f, 0.05f, 1.f, 1.f },
		{ { -10.f, 10.f }, { -10.f, 10.f }, { -1080.f, 1080.f }, { -1080.f, 1080.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Cover Oscillation", { "Speed", "Angle", "Speed", "Scale" },
		{ "cover_osc_speed", "cover_osc_amp", "cover_pos_osc_speed", "cover_pos_osc_amp" },
		{ 0.25f, 0.25f, 0.05f, 0.05f },
		{ { 0.f, 50.f }, { -90.f, 90.f }, { 0.f, 50.f }, { -4.f, 4.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_BOTH, true, "Title Position", { "Left", "Center", "Right", "" },
		{ "text_left_pos", "text_center_pos", "text_right_pos", "" },
		{ 0.05f, 0.05f, 0.05f, 0.f },
		{ { -15.f, 15.f }, { -15.f, 15.f }, { -15.f, 15.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_BOTH, true, "Title Angle", { "Left", "Center", "Right", "" },
		{ "text_left_angle", "text_center_angle", "text_right_angle", "" },
		{ 1.f, 1.f, 1.f, 0.f },
		{ { -1080.f, 1080.f }, { -1080.f, 1080.f }, { -1080.f, 1080.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_TXTSTYLE, CMenu::SCFParamDesc::PDT_TXTSTYLE },
		CMenu::SCFParamDesc::PDD_BOTH, true, "Title Width", { "Center", "Side", "Center Style", "Side Style" },
		{ "text_center_wrap_width", "text_side_wrap_width", "text_center_style", "text_side_style" },
		{ 10.f, 10.f, 1.f, 1.f },
		{ { 50.f, 3000.f }, { 50.f, 3000.f }, { 0.f, 8.f }, { 0.f, 8.f } } },
	{ { CMenu::SCFParamDesc::PDT_INT, CMenu::SCFParamDesc::PDT_INT, CMenu::SCFParamDesc::PDT_EMPTY, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_NORMAL, true, "Dimensions", { "Rows", "Columns", "", "" },
		{ "rows", "columns", "", "" },
		{ 2.f, 2.f, 0.f, 0.f },
		{ { 1.f, 9.f }, { 3.f, 21.f }, { 0.f, 0.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_EMPTY, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Row Angle", { "Top", "Bottom", "", "" },
		{ "top_angle", "bottom_angle", "", "" },
		{ 1.f, 1.f, 0.f, 0.f },
		{ { -1080.f, 1080.f }, { -1080.f, 1080.f }, { 0.f, 0.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Row Spacer", { "Top", "Bottom", "Top Relative Angle", "Bottom Relative Angle" },
		{ "top_spacer", "bottom_spacer", "top_delta_angle", "bottom_delta_angle" },
		{ 0.05f, 0.05f, 1.f, 1.f },
		{ { -15.f, 15.f }, { -15.f, 15.f }, { -1080.f, 1080.f }, { -1080.f, 1080.f } } },
	{ { CMenu::SCFParamDesc::PDT_COLOR, CMenu::SCFParamDesc::PDT_COLOR, CMenu::SCFParamDesc::PDT_COLOR, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Colors", { "Inner", "Outer", "Pointer", "" },
		{ "color_beg", "color_end", "color_off", "" },
		{ 1.f, 1.f, 1.f, 0.f },
		{ { 0.f, 255.f }, { 0.f, 255.f }, { 0.f, 255.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_BOOL, CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_NORMAL, false, "Mirror Effect", { "Intensity", "Blur", "Title Intensity", "" },
		{ "mirror_alpha", "mirror_blur", "title_mirror_alpha", "" },
		{ 0.01f, 1.f, 0.01f, 0.f },
		{ { 0.f, 1.f }, { 0.f, 1.f }, { 0.f, 1.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_NORMAL, false, "Shadow", { "Scale", "X offset", "Y offset", "" },
		{ "shadow_scale", "shadow_x" , "shadow_y", "" },
		{ 0.01f, 0.125f, 0.125f, 0.f },
		{ { 0.5f, 1.5f }, { -5.f, 5.f }, { -5.f, 5.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_COLOR, CMenu::SCFParamDesc::PDT_COLOR, CMenu::SCFParamDesc::PDT_COLOR, CMenu::SCFParamDesc::PDT_COLOR },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Shadow Colors", { "Center", "Inner", "Outer", "Pointer" },
		{ "color_shadow_center", "color_shadow_beg" , "color_shadow_end", "color_shadow_off" },
		{ 1.f, 1.f, 1.f, 1.f },
		{ { 0.f, 255.f }, { 0.f, 255.f }, { 0.f, 255.f }, { 0.f, 255.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D },
		CMenu::SCFParamDesc::PDD_BOTH, false, "Cover Scale", { "Left", "Center", "Right", "Row Center" },
		{ "left_scale", "center_scale" , "right_scale", "row_center_scale" },
		{ 0.01f, 0.01f, 0.01f, 0.01f },
		{ { 0.1f, 4.f }, { 0.1f, 4.f }, { 0.1f, 4.f }, { 0.1f, 4.f } } },
	{ { CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_V3D, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_SELECTED, true, "Flipped Cover", { "Position", "Angle", "Scale", "" },
		{ "flip_pos", "flip_angle" , "flip_scale", "" },
		{ 0.05f, 1.f, 0.01f, 0.f },
		{ { -15.f, 15.f }, { -1080.f, 1080.f }, { 0.1f, 4.f }, { 0.f, 0.f } } },
	{ { CMenu::SCFParamDesc::PDT_INT, CMenu::SCFParamDesc::PDT_INT, CMenu::SCFParamDesc::PDT_INT, CMenu::SCFParamDesc::PDT_FLOAT },
		CMenu::SCFParamDesc::PDD_NORMAL, false, "Tweaks", { "Max FSAA", "Blur resolution", "Blur radius", "Blur factor" },
		{ "max_fsaa", "blur_resolution" , "blur_radius", "blur_factor" },
		{ 1.f, 1.f, 1.f, 0.1f },
		{ { 2.f, 8.f }, { 0.f, 3.f }, { 1.f, 3.f }, { 1.f, 2.f } } },
	{ { CMenu::SCFParamDesc::PDT_FLOAT, CMenu::SCFParamDesc::PDT_INT, CMenu::SCFParamDesc::PDT_BOOL, CMenu::SCFParamDesc::PDT_EMPTY },
		CMenu::SCFParamDesc::PDD_NORMAL, false, "Textures", { "LOD bias", "Anisotropy", "Edge LOD", "" },
		{ "tex_lod_bias", "tex_aniso" , "tex_edge_lod", "" },
		{ 0.1f, 1.f, 1.f, 0.f },
		{ { -3.f, 0.5f }, { 0.f, 2.f }, { 0.f, 1.f }, { 0.f, 0.f } } }
};

template <class T> static inline T loopNum(T i, T s)
{
	return (i + s) % s;
}

static const u16 g_txtStyles[9] = {
	FTGX_JUSTIFY_LEFT | FTGX_ALIGN_TOP,
	FTGX_JUSTIFY_LEFT | FTGX_ALIGN_MIDDLE,
	FTGX_JUSTIFY_LEFT | FTGX_ALIGN_BOTTOM,
	FTGX_JUSTIFY_CENTER | FTGX_ALIGN_TOP,
	FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE,
	FTGX_JUSTIFY_CENTER | FTGX_ALIGN_BOTTOM,
	FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_TOP,
	FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_MIDDLE,
	FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_BOTTOM
};

static int styleToIdx(u16 s)
{
	for(int i = 0; i < 9; ++i)
		if(g_txtStyles[i] == s)
			return i;
	return 0;
}

static string styleToTxt(u16 s)
{
	string ts;
	if((s & FTGX_JUSTIFY_CENTER) != 0)
		ts += 'C';
	else if((s & FTGX_JUSTIFY_RIGHT) != 0)
		ts += 'R';
	else
		ts += 'L';
	if((s & FTGX_ALIGN_MIDDLE) != 0)
		ts += 'M';
	else if((s & FTGX_ALIGN_BOTTOM) != 0)
		ts += 'B';
	else
		ts += 'T';
	return ts;
}

void CMenu::_hideCFTheme(bool instant)
{
	m_btnMgr.hide(m_cfThemeBtnAlt, instant);
	m_btnMgr.hide(m_cfThemeBtnSelect, instant);
	m_btnMgr.hide(m_cfThemeBtnWide, instant);
	m_btnMgr.hide(m_cfThemeLblParam, instant);
	m_btnMgr.hide(m_cfThemeBtnParamM, instant);
	m_btnMgr.hide(m_cfThemeBtnParamP, instant);
	m_btnMgr.hide(m_cfThemeBtnSave, instant);
	m_btnMgr.hide(m_cfThemeBtnCancel, instant);
	m_btnMgr.hide(m_cfThemeBtnCopy, instant);
	m_btnMgr.hide(m_cfThemeBtnPaste, instant);

	for(u8 i = 0; i < 16; ++i)
	{
		m_btnMgr.hide(m_cfThemeLblVal[i], instant);
		m_btnMgr.hide(m_cfThemeBtnValM[i], instant);
		m_btnMgr.hide(m_cfThemeBtnValP[i], instant);
	}
	for(u8 i = 0; i < 4; ++i)
		m_btnMgr.hide(m_cfThemeLblValTxt[i], instant);
	//! added user labels to add an opacity layer
	for(u8 i = 0; i < ARRAY_SIZE(m_cfThemeLblUser); ++i)
		if(m_cfThemeLblUser[i] != -1)
			m_btnMgr.hide(m_cfThemeLblUser[i], instant);
}

void CMenu::_showCFTheme(u32 curParam, int version, bool wide)
{
	const CMenu::SCFParamDesc &p = CMenu::_cfParams[curParam];
	bool selected = CoverFlow.selected();
	string domUnsel(fmt("%s_%i", cf_domain, version));
	string domSel(fmt("%s_%i_S", cf_domain, version));

	CoverFlow.simulateOtherScreenFormat(p.scrnFmt && wide != m_vid.wide());

	m_btnMgr.show(m_cfThemeBtnSave);
	m_btnMgr.show(m_cfThemeBtnCancel);
	m_btnMgr.show(m_cfThemeBtnCopy);
	m_btnMgr.show(m_cfThemeBtnPaste);
	m_btnMgr.show(m_cfThemeBtnAlt);
	m_btnMgr.setText(m_cfThemeBtnAlt, wstringEx(sfmt("%i", version)));
//	if (p.domain == CMenu::SCFParamDesc::PDD_BOTH)
		m_btnMgr.show(m_cfThemeBtnSelect);
//	else
//		m_btnMgr.hide(m_cfThemeBtnSelect);
	m_btnMgr.setText(m_cfThemeBtnSelect, selected ? L"X" : L"");
	if(p.scrnFmt)
		m_btnMgr.show(m_cfThemeBtnWide);
	else
		m_btnMgr.hide(m_cfThemeBtnWide);
	m_btnMgr.setText(m_cfThemeBtnWide, wide ? L"16:9" : L"4:3");
	m_btnMgr.show(m_cfThemeLblParam);
	m_btnMgr.show(m_cfThemeBtnParamM);
	m_btnMgr.show(m_cfThemeBtnParamP);
	m_btnMgr.setText(m_cfThemeLblParam, string(p.name));
	
	for(u8 i = 0; i < ARRAY_SIZE(m_cfThemeLblUser); ++i)
		if(m_cfThemeLblUser[i] != -1)
			m_btnMgr.show(m_cfThemeLblUser[i]);

	for(int i = 0; i < 4; ++i)
	{
		string domain = (p.domain != CMenu::SCFParamDesc::PDD_NORMAL && selected) || p.domain == CMenu::SCFParamDesc::PDD_SELECTED ? domSel : domUnsel;
		int k = i * 4;
		string key(p.key[i]);
		if(!wide && p.scrnFmt && (p.paramType[i] == CMenu::SCFParamDesc::PDT_V3D || p.paramType[i] == CMenu::SCFParamDesc::PDT_FLOAT || p.paramType[i] == CMenu::SCFParamDesc::PDT_INT))
			key += "_4_3";
		if(p.paramType[i] != CMenu::SCFParamDesc::PDT_EMPTY)
		{
			m_btnMgr.show(m_cfThemeLblValTxt[i]);
			m_btnMgr.setText(m_cfThemeLblValTxt[i], string(p.valName[i]));
		}
		else
			m_btnMgr.hide(m_cfThemeLblValTxt[i]);
		
		switch(p.paramType[i])
		{
			case CMenu::SCFParamDesc::PDT_EMPTY:
				for(int j = 0; j < 4; ++j)
				{
					m_btnMgr.hide(m_cfThemeLblVal[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValM[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValP[k + j]);
				}
				break;
			case CMenu::SCFParamDesc::PDT_FLOAT:
			{
				float val = m_coverflow.getFloat(domain, key);
				string v = sfmt("%.2f", val);
				if(v == "-0.00") v = "0.00";
				// should we use wstringEx(sfmt())
				m_btnMgr.setText(m_cfThemeLblVal[k], v);
				for (int j = 1; j < 4; ++j)
				{
					m_btnMgr.hide(m_cfThemeLblVal[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValM[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValP[k + j]);
				}
				m_btnMgr.show(m_cfThemeLblVal[k]);
				m_btnMgr.show(m_cfThemeBtnValM[k]);
				m_btnMgr.show(m_cfThemeBtnValP[k]);
				break;
			}
			case CMenu::SCFParamDesc::PDT_V3D:
			{
				Vector3D v(m_coverflow.getVector3D(domain, key));
				string x = sfmt("%.2f", v.x);
				if(x == "-0.00") x = "0.00";
				string y = sfmt("%.2f", v.y);
				if(y == "-0.00") y = "0.00";
				string z = sfmt("%.2f", v.z);
				if(z == "-0.00") z = "0.00";
				m_btnMgr.setText(m_cfThemeLblVal[k + 0], x);
				m_btnMgr.setText(m_cfThemeLblVal[k + 1], y);
				m_btnMgr.setText(m_cfThemeLblVal[k + 2], z);
				for(int j = 0; j < 3; ++j)
				{
					m_btnMgr.show(m_cfThemeLblVal[k + j]);
					m_btnMgr.show(m_cfThemeBtnValM[k + j]);
					m_btnMgr.show(m_cfThemeBtnValP[k + j]);
				}
				m_btnMgr.hide(m_cfThemeLblVal[k + 3]);
				m_btnMgr.hide(m_cfThemeBtnValM[k + 3]);
				m_btnMgr.hide(m_cfThemeBtnValP[k + 3]);
				break;
			}
			case CMenu::SCFParamDesc::PDT_COLOR:
			{
				CColor color(m_coverflow.getColor(domain, key));
				m_btnMgr.setText(m_cfThemeLblVal[k + 0], sfmt("%02X", color.r));
				m_btnMgr.setText(m_cfThemeLblVal[k + 1], sfmt("%02X", color.g));
				m_btnMgr.setText(m_cfThemeLblVal[k + 2], sfmt("%02X", color.b));
				m_btnMgr.setText(m_cfThemeLblVal[k + 3], sfmt("%02X", color.a));
				for(int j = 0; j < 4; ++j)
				{
					m_btnMgr.show(m_cfThemeLblVal[k + j]);
					m_btnMgr.show(m_cfThemeBtnValM[k + j]);
					m_btnMgr.show(m_cfThemeBtnValP[k + j]);
				}
				break;
			}
			case CMenu::SCFParamDesc::PDT_BOOL:
				m_btnMgr.setText(m_cfThemeLblVal[k], m_coverflow.getBool(domain, key) ? L"On" : L"Off");
				for(int j = 1; j < 4; ++j)
				{
					m_btnMgr.hide(m_cfThemeLblVal[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValM[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValP[k + j]);
				}
				m_btnMgr.show(m_cfThemeLblVal[k]);
				m_btnMgr.show(m_cfThemeBtnValM[k]);
				m_btnMgr.show(m_cfThemeBtnValP[k]);
				break;
			case CMenu::SCFParamDesc::PDT_INT:
				m_btnMgr.setText(m_cfThemeLblVal[k], sfmt("%i", m_coverflow.getInt(domain, key)));
				for(int j = 1; j < 4; ++j)
				{
					m_btnMgr.hide(m_cfThemeLblVal[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValM[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValP[k + j]);
				}
				m_btnMgr.show(m_cfThemeLblVal[k]);
				m_btnMgr.show(m_cfThemeBtnValM[k]);
				m_btnMgr.show(m_cfThemeBtnValP[k]);
				break;
			case CMenu::SCFParamDesc::PDT_TXTSTYLE:
				m_btnMgr.setText(m_cfThemeLblVal[k], styleToTxt(_textStyle(domain.c_str(), key.c_str(), CoverFlow.selected() ? FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_TOP : FTGX_JUSTIFY_CENTER | FTGX_ALIGN_BOTTOM, true)));
				for(int j = 1; j < 4; ++j)
				{
					m_btnMgr.hide(m_cfThemeLblVal[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValM[k + j]);
					m_btnMgr.hide(m_cfThemeBtnValP[k + j]);
				}
				m_btnMgr.show(m_cfThemeLblVal[k]);
				m_btnMgr.show(m_cfThemeBtnValM[k]);
				m_btnMgr.show(m_cfThemeBtnValP[k]);
				break;
		}
	}
}

bool CMenu::_cfTheme(void)
{
	u32 curParam = 0;
	int cfVersion = 1;
	bool wide = m_vid.wide();
	int copyVersion = 0;
	bool copySelected = false;
	bool copyWide = wide;
	int showBtnsDelay = 0;

	SetupInput();
	_initCF();
	_getCustomBgTex();
	_setMainBg();
	_showCFTheme(curParam, cfVersion, wide);
	_loadCFLayout(cfVersion, false, wide != m_vid.wide());
	CoverFlow.applySettings(true);
	
	while(!m_exit)
	{
		_mainLoopCommon(true);
		if(BTN_HOME_PRESSED || (BTN_A_OR_2_PRESSED && m_btnMgr.selected(m_cfThemeBtnCancel)))
		{
			if(_error(_t("errcfg9", L"Quit without saving?"), true))
			{
				m_coverflow.clear();
				m_coverflow.unload();
				m_coverflow.load(fmt("%s/%s.ini", m_coverflowsDir.c_str(), m_themeName.c_str()));
				if(!m_coverflow.loaded())
					m_coverflow.load(fmt("%s/default.ini", m_coverflowsDir.c_str()));
				break;
			}
			_getCustomBgTex();
			_setMainBg();
			_showCFTheme(curParam, cfVersion, wide);
		}
		else if(BTN_LEFT_REV_PRESSED || BTN_UP_PRESSED)
		{
			m_btnMgr.up();
			showBtnsDelay = BTNS_DELAY;
		}
		else if(BTN_RIGHT_REV_PRESSED || BTN_DOWN_PRESSED)
		{
			m_btnMgr.down();
			showBtnsDelay = BTNS_DELAY;
		}

		bool sel = CoverFlow.selected();
		if(BTN_B_OR_1_HELD)
		{
			if(BTN_PLUS_PRESSED || BTN_MINUS_PRESSED)
			{
				s8 direction = BTN_PLUS_PRESSED ? 1 : -1;
				curParam = loopNum(curParam + direction, (u32)ARRAY_SIZE(CMenu::_cfParams));
				if(CMenu::_cfParams[curParam].domain == CMenu::SCFParamDesc::PDD_SELECTED)
					CoverFlow.select();
				_showCFTheme(curParam, cfVersion, wide);
			}
		}
		else if(!sel)
		{
			if(BTN_PLUS_PRESSED)
				CoverFlow.pageDown();
			else if(BTN_MINUS_PRESSED)
				CoverFlow.pageUp();
		}
		if((BTN_UP_REPEAT || BTN_LEFT_REPEAT) && ShowPointer())
			CoverFlow.left();
		else if((BTN_DOWN_REPEAT || BTN_RIGHT_REPEAT) && ShowPointer())
			CoverFlow.right();
		if(sel && !CoverFlow.selected())
			CoverFlow.select();
		if(BTN_A_OR_2_PRESSED)
		{
			if(m_btnMgr.selected(m_cfThemeBtnSave))
			{
				const char *cfCfgFile = fmt("%s/%s.ini", m_coverflowsDir.c_str(), m_themeName.c_str());
				if(_error(wfmt(_fmt("errcfg8", L"Save changes to %s?"), cfCfgFile), true))
				{
					CoverFlow.stopCoverLoader();
					m_coverflow.save();
					_error(_t("errboot8", L"Wiiflow needs rebooting to apply changes."));
					m_exit = true;
					m_reload = true;
					return true;
				}
				_getCustomBgTex();
				_setMainBg();
				_showCFTheme(curParam, cfVersion, wide);
			}
			else if(m_btnMgr.selected(m_cfThemeBtnAlt))
			{
				cfVersion = 1 + loopNum((u8)cfVersion, m_numCFVersions);
				_showCFTheme(curParam, cfVersion, wide);
				_loadCFLayout(cfVersion, false, wide != m_vid.wide());
				CoverFlow.applySettings(true);
				showBtnsDelay = BTNS_DELAY;
			}
			else if(m_btnMgr.selected(m_cfThemeBtnSelect))
			{
				if(CoverFlow.selected())
					CoverFlow.cancel();
				else
					CoverFlow.select();
				_showCFTheme(curParam, cfVersion, wide);
				_loadCFLayout(cfVersion, true, wide != m_vid.wide());
				CoverFlow.applySettings(true);
				showBtnsDelay = BTNS_DELAY;
			}
			else if(m_btnMgr.selected(m_cfThemeBtnWide))
			{
				wide = !wide;
				_showCFTheme(curParam, cfVersion, wide);
				_loadCFLayout(cfVersion, true, wide != m_vid.wide());
				CoverFlow.applySettings(true);
				showBtnsDelay = BTNS_DELAY;
			}
			else if(m_btnMgr.selected(m_cfThemeBtnParamP) || m_btnMgr.selected(m_cfThemeBtnParamM))
			{
				s8 direction = m_btnMgr.selected(m_cfThemeBtnParamP) ? 1 : -1;
				curParam = loopNum(curParam + direction, (u32)ARRAY_SIZE(CMenu::_cfParams));
				if (CMenu::_cfParams[curParam].domain == CMenu::SCFParamDesc::PDD_SELECTED)
					CoverFlow.select();
				_showCFTheme(curParam, cfVersion, wide);
				showBtnsDelay = BTNS_DELAY;
			}
			else if(m_btnMgr.selected(m_cfThemeBtnCopy))
			{
				copyVersion = cfVersion;
				copySelected = CoverFlow.selected();
				copyWide = wide;
			}
			else if(copyVersion > 0 && m_btnMgr.selected(m_cfThemeBtnPaste))
			{
				string domSrc(fmt(_cfDomain(copySelected), copyVersion));
				string domDst(fmt(_cfDomain(CoverFlow.selected()), cfVersion));
				if(copyVersion != cfVersion || copySelected != CoverFlow.selected())
					m_coverflow.copyDomain(domDst, domSrc);
				else if(copyWide != wide)
				{
					for(u32 i = 0; i < ARRAY_SIZE(CMenu::_cfParams); ++i)
					{
						const CMenu::SCFParamDesc &p = CMenu::_cfParams[i];
						if(p.scrnFmt)
						{
							for(int k = 0; k < 4; ++k)
							{
								string keySrc(p.key[k]);
								string keyDst(p.key[k]);
								if(wide)
									keySrc += "_4_3";
								else
									keyDst += "_4_3";
								if(p.paramType[k] == CMenu::SCFParamDesc::PDT_FLOAT)
									m_coverflow.setFloat(domDst, keyDst, m_coverflow.getFloat(domSrc, keySrc));
								else if(p.paramType[k] == CMenu::SCFParamDesc::PDT_V3D)
									m_coverflow.setVector3D(domDst, keyDst, m_coverflow.getVector3D(domSrc, keySrc));
								else if(p.paramType[k] == CMenu::SCFParamDesc::PDT_INT)
									m_coverflow.setInt(domDst, keyDst, m_coverflow.getInt(domSrc, keySrc));
							}
						}
					}
					_showCFTheme(curParam, cfVersion, wide);
					_loadCFLayout(cfVersion, true, wide != m_vid.wide());
					CoverFlow.applySettings(true);
				}
			}
		}
		if(BTN_A_REPEAT || BTN_A_OR_2_PRESSED)
		{
			for(int i = 0; i < 16; ++i)
				if(m_btnMgr.selected(m_cfThemeBtnValM[i]) || m_btnMgr.selected(m_cfThemeBtnValP[i]))
				{
					_cfParam(m_btnMgr.selected(m_cfThemeBtnValP[i]), i, CMenu::_cfParams[curParam], cfVersion, wide);
					_showCFTheme(curParam, cfVersion, wide);
					_loadCFLayout(cfVersion, true, wide != m_vid.wide());
					CoverFlow.applySettings(true);
					showBtnsDelay = BTNS_DELAY;
					break;
				}
		}
		for(int chan = WPAD_MAX_WIIMOTES-1; chan >= 0; chan--)
		{
			if(WPadIR_Valid(chan) || m_show_pointer[chan])
			{
				showBtnsDelay = BTNS_DELAY;
				break;
			}
		}
		if(showBtnsDelay)
			showBtnsDelay--;
		if(showBtnsDelay == 0)
			_hideCFTheme();
		else
			_showCFTheme(curParam, cfVersion, wide);
		CoverFlow.flip(true, curParam == 16);
	}
	_hideCFTheme();
	_loadCFLayout(1);
	CoverFlow.clear();
	CoverFlow.simulateOtherScreenFormat(false);
	
	return false;
}

void CMenu::_cfParam(bool inc, int i, const CMenu::SCFParamDesc &p, int cfVersion, bool wide)
{
	int k = i / 4;
	string key(p.key[k]);
	const char *d = _cfDomain((p.domain != CMenu::SCFParamDesc::PDD_NORMAL && CoverFlow.selected()) || p.domain == CMenu::SCFParamDesc::PDD_SELECTED);
	string domain(fmt(d, cfVersion));
	float step = p.step[k];
	if(!wide && p.scrnFmt && (p.paramType[k] == CMenu::SCFParamDesc::PDT_V3D || p.paramType[k] == CMenu::SCFParamDesc::PDT_FLOAT || p.paramType[k] == CMenu::SCFParamDesc::PDT_INT))
		key += "_4_3";
	if(!inc)
		step = -step;
	switch(p.paramType[k])
	{
		case CMenu::SCFParamDesc::PDT_EMPTY:
			break;
		case CMenu::SCFParamDesc::PDT_FLOAT:
		{
			float val = m_coverflow.getFloat(domain, key);
			m_coverflow.setFloat(domain, key, min(max(p.minMaxVal[k][0], val + step), p.minMaxVal[k][1]));
			break;
		}
		case CMenu::SCFParamDesc::PDT_V3D:
		{
			Vector3D v(m_coverflow.getVector3D(domain, key));
			switch(i % 4)
			{
				case 0:
					v.x = min(max(p.minMaxVal[k][0], v.x + step), p.minMaxVal[k][1]);
					break;
				case 1:
					v.y = min(max(p.minMaxVal[k][0], v.y + step), p.minMaxVal[k][1]);
					break;
				case 2:
					v.z = min(max(p.minMaxVal[k][0], v.z + step), p.minMaxVal[k][1]);
					break;
			}
			m_coverflow.setVector3D(domain, key, v);
			break;
		}
		case CMenu::SCFParamDesc::PDT_COLOR:
		{
			CColor color(m_coverflow.getColor(domain, key));
			switch(i % 4)
			{
				case 0:
					color.r = min(max(0, color.r + (int)step), 0xFF);
					break;
				case 1:
					color.g = min(max(0, color.g + (int)step), 0xFF);
					break;
				case 2:
					color.b = min(max(0, color.b + (int)step), 0xFF);
					break;
				case 3:
					color.a = min(max(0, color.a + (int)step), 0xFF);
					break;
			}
			m_coverflow.setColor(domain, key, color);
			break;
		}
		case CMenu::SCFParamDesc::PDT_BOOL:
		{
			m_coverflow.setBool(domain, key, !m_coverflow.getBool(domain, key));
			break;
		}
		case CMenu::SCFParamDesc::PDT_INT:
		{
			int val = m_coverflow.getInt(domain, key);
			m_coverflow.setInt(domain, key, min(max((int)p.minMaxVal[k][0], val + (int)step), (int)p.minMaxVal[k][1]));
			break;
		}
		case CMenu::SCFParamDesc::PDT_TXTSTYLE:
		{
			int i = styleToIdx(_textStyle(domain.c_str(), key.c_str(), CoverFlow.selected() ? FTGX_JUSTIFY_RIGHT | FTGX_ALIGN_TOP : FTGX_JUSTIFY_CENTER | FTGX_ALIGN_BOTTOM, true));
			i = loopNum(i + (int)step, 9);
			m_coverflow.setString(domain, key, styleToTxt(g_txtStyles[i]));
			break;
		}
	}
}

const char *CMenu::_cfDomain(bool selected)
{
	return selected ? fmt("%s_%%i_S", cf_domain) : fmt("%s_%%i", cf_domain);
}

void CMenu::_initCFThemeMenu()
{
	TexData emptyTex;
	int x;
	int y;
	_addUserLabels(m_cfThemeLblUser, ARRAY_SIZE(m_cfThemeLblUser), "CFTHEME");
	m_cfThemeBtnAlt = _addButton("CFTHEME/ALT_BTN", theme.btnFont, L"", 20, 20, 80, 32, theme.btnFontColor);
	m_cfThemeBtnSelect = _addButton("CFTHEME/SELECT_BTN", theme.btnFont, L"", 100, 20, 80, 32, theme.btnFontColor);
	m_cfThemeLblParam = _addLabel("CFTHEME/PARAM_BTN", theme.btnFont, L"", 212, 20, 192, 32, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
	m_cfThemeBtnParamM = _addPicButton("CFTHEME/PARAM_MINUS", theme.btnTexMinus, theme.btnTexMinusS, 180, 20, 32, 32);
	m_cfThemeBtnParamP = _addPicButton("CFTHEME/PARAM_PLUS", theme.btnTexPlus, theme.btnTexPlusS, 404, 20, 32, 32);
	m_cfThemeBtnWide = _addButton("CFTHEME/WIDE_BTN", theme.btnFont, L"", 436, 20, 80, 32, theme.btnFontColor);
	m_cfThemeBtnSave = _addButton("CFTHEME/SAVE_BTN", theme.btnFont, L"Save", 516, 20, 104, 32, theme.btnFontColor);
	m_cfThemeBtnCancel = _addButton("CFTHEME/CANCEL_BTN", theme.btnFont, L"Cancel", 516, 54, 104, 32, theme.btnFontColor);
	m_cfThemeBtnCopy = _addButton("CFTHEME/COPY_BTN", theme.btnFont, L"Copy", 20, 54, 80, 32, theme.btnFontColor);
	m_cfThemeBtnPaste = _addButton("CFTHEME/PASTE_BTN", theme.btnFont, L"Paste", 100, 54, 80, 32, theme.btnFontColor);

	for(int i = 0; i < 16; ++i)
	{
		char *domain = fmt_malloc("CFTHEME/VAL%i%c_%%s", i / 3 + 1, (char)(i % 3) + 'A');
		if(domain == NULL) 
			continue;
		x = 20 + (i / 4) * 150;
		y = 340 + (i % 4) * 32;
		m_cfThemeLblVal[i] = _addLabel(fmt(domain, "BTN"), theme.btnFont, L"", x + 32, y, 86, 32, theme.btnFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_MIDDLE, theme.btnTexC);
		m_cfThemeBtnValM[i] = _addPicButton(fmt(domain, "MINUS"), theme.btnTexMinus, theme.btnTexMinusS, x, y, 32, 32);
		m_cfThemeBtnValP[i] = _addPicButton(fmt(domain, "PLUS"), theme.btnTexPlus, theme.btnTexPlusS, x + 118, y, 32, 32);
		MEM2_free(domain);
	}
	for(int i = 0; i < 4; ++i)
		m_cfThemeLblValTxt[i] = _addLabel(fmt("CFTHEME/VAL%i_LBL", i + 1), theme.lblFont, L"", 20 + i * 150, 98, 150, 240, theme.lblFontColor, FTGX_JUSTIFY_CENTER | FTGX_ALIGN_BOTTOM, emptyTex);
	_hideCFTheme(true);
}
