
#include <string.h>
#include <unistd.h>
#include <malloc.h>
#include <ogc/machine/processor.h>
#include <algorithm>
#include "memory/mem2.hpp"
#include "video.hpp"
#include "pngu.h"
#include "hw/Gekko.h"
#include "WiiMovie.hpp"
#include "gecko/gecko.hpp"
#include "loader/sys.h"
#include "loader/utils.h"
#include "list/ListGenerator.hpp"
#include "text.hpp"
#include "fileOps/fileOps.h"

#define DEFAULT_FIFO_SIZE	(256 * 1024)

const float CVideo::_jitter2[2][2] = {
	{ 0.246490f, 0.249999f },
	{ -0.246490f, -0.249999f }
};

const float  CVideo::_jitter3[3][2] = {
	{ -0.373411f, -0.250550f },
	{ 0.256263f, 0.368119f },
	{ 0.117148f, -0.117570f }
};

const float CVideo::_jitter4[4][2] = {
	{ -0.208147f, 0.353730f },
	{ 0.203849f, -0.353780f },
	{ -0.292626f, -0.149945f },
	{ 0.296924f, 0.149994f }
};

const float CVideo::_jitter5[5][2] = {
	{ 0.5f, 0.5f },
	{ 0.3f, 0.1f },
	{ 0.7f, 0.9f },
	{ 0.9f, 0.3f },
	{ 0.1f, 0.7f }
};

const float CVideo::_jitter6[6][2] = {
	{ 0.4646464646f, 0.4646464646f },
	{ 0.1313131313f, 0.7979797979f },
	{ 0.5353535353f, 0.8686868686f },
	{ 0.8686868686f, 0.5353535353f },
	{ 0.7979797979f, 0.1313131313f },
	{ 0.2020202020f, 0.2020202020f }
};

const float CVideo::_jitter8[8][2] = {
	{ -0.334818f, 0.435331f },
	{ 0.286438f, -0.393495f },
	{ 0.459462f, 0.141540f },
	{ -0.414498f, -0.192829f },
	{ -0.183790f, 0.082102f },
	{ -0.079263f, -0.317383f },
	{ 0.102254f, 0.299133f },
	{ 0.164216f, -0.054399f }
};

GXRModeObj TVPal574IntDfScale =
{
    VI_TVMODE_PAL_INT,      // viDisplayMode
    640,             // fbWidth
    480,             // efbHeight
    574,             // xfbHeight
    (VI_MAX_WIDTH_PAL - 640)/2,         // viXOrigin
    (VI_MAX_HEIGHT_PAL - 574)/2,        // viYOrigin
    640,             // viWidth
    574,             // viHeight
    VI_XFBMODE_DF,   // xFBmode
    GX_FALSE,        // field_rendering
    GX_FALSE,        // aa

    // sample points arranged in increasing Y order
	{
		{6,6},{6,6},{6,6},  // pix 0, 3 sample points, 1/12 units, 4 bits each
		{6,6},{6,6},{6,6},  // pix 1
		{6,6},{6,6},{6,6},  // pix 2
		{6,6},{6,6},{6,6}   // pix 3
	},
    // vertical filter[7], 1/64 units, 6 bits each
	{
		 8,         // line n-1
		 8,         // line n-1
		10,         // line n
		12,         // line n
		10,         // line n
		 8,         // line n+1
		 8          // line n+1
	}
};

vector<TexData> m_defaultWaitMessages;
vector<TexData> m_loadListWaitMessages;

CVideo m_vid;

CVideo::CVideo(void) :
	m_rmode(NULL), m_frameBuf(), m_curFB(0), m_fifo(NULL),
	m_yScale(0.0f), m_xfbHeight(0), m_wide(false),
	m_width2D(640), m_height2D(480), m_x2D(0), m_y2D(0), m_aa(0), m_aaAlpha(false),
	m_aaWidth(0), m_aaHeight(0), m_screensaver_alpha(0), m_showWaitMessage(false), 
	m_WaitThreadRunning(false), m_showingWaitMessages(false)
{
	memset(m_frameBuf, 0, sizeof m_frameBuf);
}

void CVideo::init(void)
{
	/* General Video Init */
	VIDEO_Init();
	m_wide = CONF_GetAspectRatio() == CONF_ASPECT_16_9;
	m_rmode = VIDEO_GetPreferredMode(NULL);
	u32 type = CONF_GetVideo();
	m_50hz = false;
	if(m_rmode == &TVPal528IntDf)
	{
		m_rmode= &TVPal574IntDfScale;
		m_50hz = true;
	}
	m_rmode->viWidth = m_wide ? 700 : 672;

	// CONF_VIDEO_NTSC and CONF_VIDEO_MPAL and m_rmode TVEurgb60Hz480IntDf are the same max height and width.
	if(type == CONF_VIDEO_PAL && m_rmode != &TVEurgb60Hz480IntDf)
	{
		m_rmode->viHeight = VI_MAX_HEIGHT_PAL;
		m_rmode->viXOrigin = (VI_MAX_WIDTH_PAL - m_rmode->viWidth) / 2;
		m_rmode->viYOrigin = (VI_MAX_HEIGHT_PAL - m_rmode->viHeight) / 2;
	}
	else
	{
		m_rmode->viHeight = VI_MAX_HEIGHT_NTSC;
		m_rmode->viXOrigin = (VI_MAX_WIDTH_NTSC - m_rmode->viWidth) / 2;
		m_rmode->viYOrigin = (VI_MAX_HEIGHT_NTSC - m_rmode->viHeight) / 2;
	}

	s8 hoffset = 0;  // Use horizontal offset set in wii menu.
	if(CONF_GetDisplayOffsetH(&hoffset) == 0)
		m_rmode->viXOrigin += hoffset;

	/* Widescreen Fix by tueidj, WiiU Check by crediar, thanks alot */
	if(m_wide && AHBPROT_Patched() && IsOnWiiU())
	{
		write32(0xd8006a0, 0x30000004);
		mask32(0xd8006a8, 0, 2);
	}

	/* GX Init */
	m_frameBuf[0] = MEM_K0_TO_K1(MEM1_memalign(32, VIDEO_GetFrameBufferSize(m_rmode)));
	m_frameBuf[1] = MEM_K0_TO_K1(MEM1_memalign(32, VIDEO_GetFrameBufferSize(m_rmode)));
	m_curFB = 0;

	m_fifo = MEM1_memalign(32, DEFAULT_FIFO_SIZE);
	memset(m_fifo, 0, DEFAULT_FIFO_SIZE);
	
	GX_Init(m_fifo, DEFAULT_FIFO_SIZE);
	GX_SetCopyClear(CColor(0), 0x00FFFFFF);
	
	_setViewPort(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	
	m_yScale = GX_GetYScaleFactor(m_rmode->efbHeight, m_rmode->xfbHeight);
	m_xfbHeight = GX_SetDispCopyYScale(m_yScale);
	GX_SetScissor(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	GX_SetDispCopySrc(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	GX_SetDispCopyDst(m_rmode->fbWidth, m_xfbHeight);
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
	GX_SetFieldMode(m_rmode->field_rendering, ((m_rmode->viHeight == 2 * m_rmode->xfbHeight) ? GX_ENABLE : GX_DISABLE));
	
	GX_SetCullMode(GX_CULL_NONE);
	GX_CopyDisp(m_frameBuf[m_curFB], GX_TRUE); // uncommented when added small covers transparency
	GX_SetDispCopyGamma(GX_GM_1_0);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGB8, 0);
	GX_SetNumTexGens(0);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORDNULL, GX_TEXMAP_NULL, GX_COLOR0A0);
	GX_SetNumChans(0);
	GX_SetZCompLoc(GX_ENABLE);
	
	setup2DProjection();
	
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	for(u32 i = 0; i < 8; i++)
		GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0+i, GX_TEX_ST, GX_F32, 0);
		
	m_stencil = MEM1_memalign(32, CVideo::_stencilWidth * CVideo::_stencilHeight);
	memset(m_stencil, 0, CVideo::_stencilWidth * CVideo::_stencilHeight);

	/* Configure Video */
	VIDEO_Configure(m_rmode);
	_clearScreen();
	VIDEO_SetBlack(FALSE);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	if(m_rmode->viTVMode & VI_NON_INTERLACE)
		VIDEO_WaitVSync();
}

void CVideo::cleanup(void)
{
	// gprintf("Cleaning up video...\n");

	hideWaitMessage();
	_clearScreen();

	VIDEO_SetBlack(TRUE);
	VIDEO_Flush();

	GX_DrawDone();
	GX_AbortFrame();

	for(u8 i = 0; i < sizeof m_aaBuffer / sizeof m_aaBuffer[0]; ++i)
	{
		if(m_aaBuffer[i])
			free(m_aaBuffer[i]);
		m_aaBuffer[i] = NULL;
		m_aaBufferSize[i] = 0;
	}
	for(u8 i = 0; i < m_defaultWaitMessages.size(); i++)
	{
		if(m_defaultWaitMessages[i].data != NULL)
			free(m_defaultWaitMessages[i].data);
		m_defaultWaitMessages[i].data = NULL;
	}
	for(u8 i = 0; i < m_loadListWaitMessages.size(); i++)
	{
		if(m_loadListWaitMessages[i].data != NULL)
			free(m_loadListWaitMessages[i].data);
		m_loadListWaitMessages[i].data = NULL;
	}
	MEM1_free(MEM_K1_TO_K0(m_frameBuf[0]));
	m_frameBuf[0] = NULL;
	MEM1_free(MEM_K1_TO_K0(m_frameBuf[1]));
	m_frameBuf[1] = NULL;
	MEM1_free(m_stencil);
	m_stencil = NULL;
	MEM1_free(m_fifo);
	m_fifo = NULL;
}

void CVideo::_clearScreen()
{
	VIDEO_ClearFrameBuffer(m_rmode, m_frameBuf[0], COLOR_BLACK);
	VIDEO_ClearFrameBuffer(m_rmode, m_frameBuf[1], COLOR_BLACK);
	render();
	render();
}

void CVideo::prepare(void)
{
	if(m_rmode->aa) 
		GX_SetPixelFmt(GX_PF_RGB565_Z16, GX_ZC_LINEAR); // 16 bit color (565)
	else
		GX_SetPixelFmt(GX_PF_RGB8_Z24, GX_ZC_LINEAR); // 24 bit color (888)
	_setViewPort(0.f, 0.f, (float)m_rmode->fbWidth, (float)m_rmode->efbHeight);
	GX_SetScissor(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void CVideo::setup2DProjection(bool setViewPort, bool noScale)
{
	Mtx44 projMtx;
	float width2D = noScale ? 640.f : (float)m_width2D;
	float height2D = noScale ? 480.f : (float)m_height2D;
	float x = noScale ? 0.f : (float)(640 - width2D) * 0.5f + (float)m_x2D;
	float y = noScale ? 0.f : (float)(480 - height2D) * 0.5f + (float)m_y2D;

	if (setViewPort)
		_setViewPort(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
	guOrtho(projMtx, y, height2D + y, x, width2D + x, 0.f, 1000.0f);
	GX_LoadProjectionMtx(projMtx, GX_ORTHOGRAPHIC);
}

void CVideo::_setViewPort(float x, float y, float w, float h)
{
	m_vpX = x;
	m_vpY = y;
	m_vpW = w;
	m_vpH = h;
	GX_SetViewport(x, y, w, h, 0.f, 1.f);
}

void CVideo::shiftViewPort(float x, float y)
{
	GX_SetViewport(m_vpX + x, m_vpY + y, m_vpW, m_vpH, 0.f, 1.f);
}

void CVideo::set2DViewport(unsigned int w, unsigned int h, int x, int y)
{
	m_width2D = std::min(std::max(512u, w), 800u);
	m_height2D = std::min(std::max(384u, h), 600u);

	m_x2D = std::min(std::max(-50, x), 50);
	m_y2D = std::min(std::max(-50, y), 50);
}

/* this takes what is drawn in the frame buffer and copies it to make a new texture/image set as whatever TexData &tex is */
/* this is used by updatebg to fade old bg into new bg */
/* and coverflow after drawing bg and covers to make mirror bg with blur */
void CVideo::renderToTexture(TexData &tex, bool clear)
{
	if(tex.data == NULL)
	{
		tex.dataSize = GX_GetTexBufferSize(tex.width, tex.height, tex.format, GX_FALSE, 0);
		tex.data = (u8*)MEM2_alloc(tex.dataSize);
		if(tex.data == NULL)
			return;
	}
	GX_DrawDone();
	GX_SetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
	GX_SetTexCopySrc(0, 0, tex.width, tex.height);
	GX_SetTexCopyDst(tex.width, tex.height, tex.format, GX_FALSE);
	GX_CopyTex(tex.data, clear ? GX_TRUE : GX_FALSE);
	GX_PixModeSync();
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
	DCFlushRange(tex.data, tex.dataSize);
	GX_SetScissor(0, 0, m_rmode->fbWidth, m_rmode->efbHeight);
}

void CVideo::prepareAAPass(int aaStep)
{
	float x = 0.f;
	float y = 0.f;
	u32 w = m_aaWidth <= 0 ? m_rmode->fbWidth : (u32)m_aaWidth;
	u32 h = m_aaHeight <= 0 ? m_rmode->efbHeight: (u32)m_aaHeight;
	switch (m_aa)
	{
		case 2:
			x += CVideo::_jitter2[aaStep][0];
			y += CVideo::_jitter2[aaStep][1];
			break;
		case 3:
			x += CVideo::_jitter3[aaStep][0];
			y += CVideo::_jitter3[aaStep][1];
			break;
		case 4:
			x += CVideo::_jitter4[aaStep][0];
			y += CVideo::_jitter4[aaStep][1];
			break;
		case 5:
			x += CVideo::_jitter5[aaStep][0];
			y += CVideo::_jitter5[aaStep][1];
			break;
		case 6:
			x += CVideo::_jitter6[aaStep][0];
			y += CVideo::_jitter6[aaStep][1];
			break;
		case 8:
			x += CVideo::_jitter8[aaStep][0];
			y += CVideo::_jitter8[aaStep][1];
			break;
	}
	GX_SetPixelFmt(m_aaAlpha ? GX_PF_RGBA6_Z24 : GX_PF_RGB8_Z24, GX_ZC_LINEAR); // rgba6 is 6 bit color with 6 bit alpha = 24bit (6666)
	_setViewPort(x, y, (float)w, (float)h);
	GX_SetScissor(0, 0, w, h);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void CVideo::renderAAPass(int aaStep)
{
	u8 texFmt = GX_TF_RGBA8;
	u32 w = m_aaWidth <= 0 ? m_rmode->fbWidth : (u32)m_aaWidth;
	u32 h = m_aaHeight <= 0 ? m_rmode->efbHeight: (u32)m_aaHeight;
	u32 bufLen = GX_GetTexBufferSize(w, h, texFmt, GX_FALSE, 0);

	if (!m_aaBuffer[aaStep] || m_aaBufferSize[aaStep] < bufLen)
	{
		m_aaBuffer[aaStep] = (u8*)MEM2_alloc(bufLen);
		if (m_aaBuffer[aaStep] != NULL)
			m_aaBufferSize[aaStep] = bufLen;
	}
	if (!m_aaBuffer[aaStep] || m_aaBufferSize[aaStep] < bufLen)
		return;
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_TRUE);
	GX_DrawDone();
	GX_SetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
	GX_SetTexCopySrc(0, 0, w, h);
	GX_SetTexCopyDst(w, h, texFmt, GX_FALSE);
	GX_CopyTex(m_aaBuffer[aaStep], GX_TRUE);
	GX_PixModeSync();
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
}

void CVideo::drawAAScene(bool fs)
{
	GXTexObj texObj[8];
	Mtx modelViewMtx;
	u8 texFmt = GX_TF_RGBA8;
	u32 tw = m_aaWidth <= 0 ? m_rmode->fbWidth : (u32)m_aaWidth;
	u32 th = m_aaHeight <= 0 ? m_rmode->efbHeight: (u32)m_aaHeight;
	float w = fs ? 640.f : (float)tw;
	float h = fs ? 480.f : (float)th;
	float x = 0.f;
	float y = 0.f;
	int aa;

	if (m_aa <= 0 || m_aa > 8)
		return;

	for (aa = 0; aa < m_aa; ++aa)
		if (!m_aaBuffer[aa])
			break;
	if (aa == 7)
		aa = 6;

	GX_SetNumChans(0);
	for (int i = 0; i < aa; ++i)
	{
		GX_InitTexObj(&texObj[i], m_aaBuffer[i], tw , th, texFmt, GX_CLAMP, GX_CLAMP, GX_FALSE);
		GX_LoadTexObj(&texObj[i], GX_TEXMAP0 + i);
	}
	GX_SetNumTexGens(1);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetTevKColor(GX_KCOLOR0, CColor(0xFF / 1, 0xFF / 5, 0xFF, 0xFF));	// Renders better gradients than 0xFF / aa
	GX_SetTevKColor(GX_KCOLOR1, CColor(0xFF / 2, 0xFF / 6, 0xFF, 0xFF));
	GX_SetTevKColor(GX_KCOLOR2, CColor(0xFF / 3, 0xFF / 7, 0xFF, 0xFF));
	GX_SetTevKColor(GX_KCOLOR3, CColor(0xFF / 4, 0xFF / 8, 0xFF, 0xFF));
	for (int i = 0; i < aa; ++i)
	{
		GX_SetTevKColorSel(GX_TEVSTAGE0 + i, GX_TEV_KCSEL_K0_R + i);
		GX_SetTevKAlphaSel(GX_TEVSTAGE0 + i, GX_TEV_KASEL_K0_R + i);
		GX_SetTevOrder(GX_TEVSTAGE0 + i, GX_TEXCOORD0, GX_TEXMAP0 + i, GX_COLORNULL);
		GX_SetTevColorIn(GX_TEVSTAGE0 + i, i == 0 ? GX_CC_ZERO : GX_CC_CPREV, GX_CC_TEXC, GX_CC_KONST, GX_CC_ZERO);
		GX_SetTevAlphaIn(GX_TEVSTAGE0 + i, i == 0 ? GX_CA_ZERO : GX_CA_APREV, GX_CA_TEXA, GX_CA_KONST, GX_CA_ZERO);
		GX_SetTevColorOp(GX_TEVSTAGE0 + i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
		GX_SetTevAlphaOp(GX_TEVSTAGE0 + i, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	}
	GX_SetNumTevStages(aa);

	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position3f32(x, y, 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32(x + w, y, 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32(x + w, y + h, 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32(x, y + h, 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
	GX_SetNumChans(1);
	GX_SetNumTexGens(1);
	GX_SetNumTevStages(1);
}

void CVideo::render(void)
{
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_CopyDisp(MEM_K1_TO_K0(m_frameBuf[m_curFB]), GX_TRUE);
	DCFlushRange(m_frameBuf[m_curFB], 2 * m_rmode->fbWidth * m_rmode->xfbHeight);
	GX_DrawDone();
	VIDEO_SetNextFramebuffer(m_frameBuf[m_curFB]);
	VIDEO_Flush();
	VIDEO_WaitVSync();
	m_curFB ^= 1;
	GX_InvalidateTexAll();
}

/* wait animation control */
extern const u8 wfsplash_png[];
extern const u8 wait_usb_off_png[];
extern const u8 wait_usb_on_png[];
extern const u8 wait_loadlist_1_png[];
extern const u8 wait_loadlist_2_png[];
extern const u8 wait_loadlist_3_png[];
extern const u8 wait_loadlist_4_png[];
extern const u8 wait_default_1_png[];
extern const u8 wait_default_2_png[];
extern const u8 wait_default_3_png[];
extern const u8 wait_default_4_png[];
extern const u8 wait_default_5_png[];
extern const u8 wait_default_6_png[];
extern const u8 wait_default_7_png[];
extern const u8 wait_default_8_png[];

static lwp_t waitThread = LWP_THREAD_NULL;
u8 CVideo::waitMessageStack[2048] ATTRIBUTE_ALIGN(32);
const u32 CVideo::waitMessageStackSize = 2048;

bool custom = false;
bool waitLoop = true; //
static vector<string> waitImgs;

static void GrabWaitFiles(char *FullPath)
{
	waitImgs.push_back(FullPath);
}

void CVideo::setCustomWaitImgs(const char *path, bool wait_loop)
{
	waitImgs.clear();
	if(path != NULL && fsop_FolderExist(path))
	{
		GetFiles(path, stringToVector(".png|.jpg", '|'), GrabWaitFiles, false, 1);
		if(waitImgs.size() > 0)
		{
			sort(waitImgs.begin(), waitImgs.end());
			custom = true;
			waitLoop = wait_loop;
		}
	}
}

void CVideo::waitMessage(float delay) // set wait images from custom or internal
{
	if(m_defaultWaitMessages.size() == 0)
	{
		if(custom)
		{
			u8 waitImgs_cnt = waitImgs.size();
			TexData m_wTextures[waitImgs_cnt];
			for(u8 i = 0; i < waitImgs_cnt; i++)
			{
				TexHandle.fromImageFile(m_wTextures[i], waitImgs[i].c_str());
			}
			for(u8 i = 0; i < waitImgs_cnt; i++)
				m_defaultWaitMessages.push_back(m_wTextures[i]);
		}
		else
		{
			TexData m_wTextures[8];
			TexHandle.fromPNG(m_wTextures[0], wait_default_1_png);
			TexHandle.fromPNG(m_wTextures[1], wait_default_2_png);
			TexHandle.fromPNG(m_wTextures[2], wait_default_3_png);
			TexHandle.fromPNG(m_wTextures[3], wait_default_4_png);
			TexHandle.fromPNG(m_wTextures[4], wait_default_5_png);
			TexHandle.fromPNG(m_wTextures[5], wait_default_6_png);
			TexHandle.fromPNG(m_wTextures[6], wait_default_7_png);
			TexHandle.fromPNG(m_wTextures[7], wait_default_8_png);
			for(u8 i = 0; i < 8; i++)
				m_defaultWaitMessages.push_back(m_wTextures[i]);
		}
	}
	waitMessage(m_defaultWaitMessages, delay);
}

void CVideo::waitMessage(const vector<TexData> &tex, float delay) // start wait images and wii slot light threads
{
	hideWaitMessage();

	if(tex.size() == 0)
	{
		m_waitMessages = m_defaultWaitMessages;
		m_waitMessageDelay = 0.15f;
	}
	else
	{
		m_waitMessages = tex;
		m_waitMessageDelay = custom ? 0.15f : delay; //
	}

	if(m_waitMessages.size() == 1)
	{
		waitMessage(m_waitMessages[0]); // draws frame image using function below (for one frame image only) but no render?
		render();
	}
	else if(m_waitMessages.size() > 1) // if more than one frame
	{
		m_WaitThreadRunning = true;
		/* changing light */
		wiiLightSetLevel(0); // LED
		wiiLightStartThread(); // start thread in gekko.c that pulses the wii disc slot light on and off - LED
		/* onscreen animation */
		m_showWaitMessage = true; // start wait images thread to animate them
		LWP_CreateThread(&waitThread, _showWaitMessages, this, waitMessageStack, waitMessageStackSize, LWP_PRIO_HIGHEST);
	}
}

void * CVideo::_showWaitMessages(void *obj) // wait images thread
{
	CVideo *m = static_cast<CVideo *>(obj);
	m->m_showingWaitMessages = true;
	u32 frames = m->m_waitMessageDelay * 50; // set delay start
	// u32 waitFrames = frames; // set delay counter to delay start
	u32 waitFrames = 0; // no start delay

	s8 fadeDirection = 1; // LED
	s8 PNGfadeDirection = 1; // set frames movement direction
	s16 currentLightLevel = 0; // LED

	vector<TexData>::iterator waitItr = m->m_waitMessages.begin(); // set start frame image
	m->_clearScreen();

	m->prepare();
	m->setup2DProjection();

	// gprintf("Wait Message Thread: Start\nDelay: %d, Images: %d\n", waitFrames, m->m_waitMessages.size());
	while(m->m_showWaitMessage)
	{
		/* LED */	
		currentLightLevel += fadeDirection * 5;
		if(currentLightLevel >= 255) 
		{
			currentLightLevel = 255;
			fadeDirection = -1;
		}
		else if(currentLightLevel <= 0)
		{
			currentLightLevel = 0;
			fadeDirection = 1;
		}
		wiiLightSetLevel(currentLightLevel);
		
		if(waitFrames == 0) // if delay count reaches 0
		{
			m->waitMessage(*waitItr); // draw frame image
			waitItr += PNGfadeDirection; // move to next image
			if(waitLoop && waitItr == m->m_waitMessages.end())
				waitItr = m->m_waitMessages.begin();
			else if(!waitLoop && (waitItr + 1 == m->m_waitMessages.end() || waitItr == m->m_waitMessages.begin()))
				PNGfadeDirection *= (-1); // change direction if at beginning or end
			waitFrames = frames; // reset delay count
			m->render();
		}
		else
			VIDEO_WaitVSync();
		waitFrames--; // decrement delay count
	}
	// gprintf("Wait Message Thread: End\n");
	m->m_showingWaitMessages = false;
	return NULL;
}

/* stop wait images thread and wii disc slot light thread */
void CVideo::hideWaitMessage()
{
	m_showWaitMessage = false;
	if(waitThread != LWP_THREAD_NULL)
	{
		/* end animation */
		if(LWP_ThreadIsSuspended(waitThread))
			LWP_ResumeThread(waitThread);
		while(m_showingWaitMessages)
			usleep(50);
		LWP_JoinThread(waitThread, NULL);
		/* end light thread */
		wiiLightEndThread(); // LED
		m_WaitThreadRunning = false;
	}
	waitThread = LWP_THREAD_NULL;
}

/* draws wait animation image centered on screen */
/* can be used to draw any texture centered on screen */
void CVideo::waitMessage(const TexData &tex)
{
	Mtx modelViewMtx;
	GXTexObj texObj;

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(1);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
	GX_SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_InitTexObj(&texObj, tex.data, tex.width, tex.height, tex.format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	u32 texWidth = m_wide ? tex.width * .75 : tex.width;
	GX_Position3f32((float)((640 - texWidth) / 2), (float)((480 - tex.height) / 2), 0.f);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position3f32((float)((640 + texWidth) / 2), (float)((480 - tex.height) / 2), 0.f);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position3f32((float)((640 + texWidth) / 2), (float)((480 + tex.height) / 2), 0.f);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position3f32((float)((640 - texWidth) / 2), (float)((480 + tex.height) / 2), 0.f);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
}

/* draw and render startup splash screen */
void CVideo::startImage(void)
{
	TexData splashTex;
	TexHandle.fromPNG(splashTex, wfsplash_png);
	waitMessage(splashTex);
	render();
	TexHandle.Cleanup(splashTex);
}

/* draw and render usb splash screen */
void CVideo::usbImage(bool usb_mounted)
{
	TexData usbTex;
	TexHandle.fromPNG(usbTex, usb_mounted ? wait_usb_on_png : wait_usb_off_png);
	waitMessage(usbTex);
	render();
	TexHandle.Cleanup(usbTex);
}

/* draw and render load list splash screen */
void CVideo::loadListImage(void)
{
	if(custom)
	{
		waitMessage(m_defaultWaitMessages, 0.15f);
		return;
	}
	
	if(m_loadListWaitMessages.size() == 0)
	{
		TexData loadListTex[4];
		TexHandle.fromPNG(loadListTex[0], wait_loadlist_1_png);
		TexHandle.fromPNG(loadListTex[1], wait_loadlist_2_png);
		TexHandle.fromPNG(loadListTex[2], wait_loadlist_3_png);
		TexHandle.fromPNG(loadListTex[3], wait_loadlist_4_png);
		for(u8 i = 0; i < 4; i++)
			m_loadListWaitMessages.push_back(loadListTex[i]);
	}
	waitMessage(m_loadListWaitMessages, 0.15f);
}

/* save screenshot */
s32 CVideo::TakeScreenshot(const char *path)
{
	IMGCTX ctx = PNGU_SelectImageFromDevice(path);
	s32 ret = PNGU_EncodeFromYCbYCr(ctx, m_rmode->fbWidth, m_rmode->xfbHeight, m_frameBuf[m_curFB], 1);
	PNGU_ReleaseImageContext (ctx);
	return ret;
}

/* used by coverflow to draw cover texture. used in mainloopcommon() in menu.cpp */
/* this draws a texture in the upper left at 0,0 */
void DrawTexture(TexData * &tex)
{
	if(tex == NULL)
		return;
	Mtx modelViewMtx;
	GXTexObj texObj;

	GX_SetNumChans(1);
	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
	GX_SetNumTexGens(1);
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_TRUE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_LEQUAL, GX_TRUE);
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);
	GX_InitTexObj(&texObj, tex->data, tex->width, tex->height, tex->format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);
	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position2f32(0.f, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position2f32(tex->width, 0.f);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position2f32(tex->width, tex->height);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position2f32(0.f, tex->height);
	GX_Color4u8(0xFF, 0xFF, 0xFF, 0xFF);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
}

/* draws movie frame texture when playing movie in menu_game.cpp */
struct movieP normalMoviePos = { 385, 25, 640, 170 }; // top right banner frame
struct movieP zoomedMoviePos = { 0, 0, 640, 480 }; // full screen
struct movieP currentMoviePos = { 310, 120, 630, 360 }; // middle right

/* draws a texture/image at a position and width and height specified by struct movieP currentMoviePos */
/* set currentMoviePos before calling this. example - {x, y, x+w, y+h} */
void DrawTexturePos(const TexData *tex, bool fullScreen)
{
	currentMoviePos = fullScreen ? zoomedMoviePos : normalMoviePos;
	Mtx modelViewMtx;
	GXTexObj texObj;

	GX_ClearVtxDesc();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
	GX_SetVtxDesc(GX_VA_TEX0, GX_DIRECT);
	GX_SetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);

	GX_SetNumTexGens(1);
	GX_SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
	GX_SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
	GX_SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLORNULL);
	GX_SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY);
	GX_SetBlendMode(GX_BM_NONE, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
	GX_SetAlphaUpdate(GX_FALSE);
	GX_SetCullMode(GX_CULL_NONE);
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_FALSE);

	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);

	GX_InitTexObj(&texObj, tex->data, tex->width, tex->height, tex->format, GX_CLAMP, GX_CLAMP, GX_FALSE);
	GX_LoadTexObj(&texObj, GX_TEXMAP0);

	GX_Begin(GX_QUADS, GX_VTXFMT0, 4);
	GX_Position2f32(currentMoviePos.x1, currentMoviePos.y1);
	GX_TexCoord2f32(0.f, 0.f);
	GX_Position2f32(currentMoviePos.x2, currentMoviePos.y1);
	GX_TexCoord2f32(1.f, 0.f);
	GX_Position2f32(currentMoviePos.x2, currentMoviePos.y2);
	GX_TexCoord2f32(1.f, 1.f);
	GX_Position2f32(currentMoviePos.x1, currentMoviePos.y2);
	GX_TexCoord2f32(0.f, 1.f);
	GX_End();
}

/* screen saver and full banner frame */
void DrawRectangle(f32 x, f32 y, f32 width, f32 height, GXColor color)
{
	Mtx modelViewMtx;
	guMtxIdentity(modelViewMtx);
	GX_LoadPosMtxImm(modelViewMtx, GX_PNMTX0);

	GX_SetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
	GX_ClearVtxDesc();
	GX_InvVtxCache();
	GX_SetVtxDesc(GX_VA_POS, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_CLR0, GX_DIRECT);
	GX_SetVtxDesc(GX_VA_TEX0, GX_NONE);

	int i;
	f32 x2 = x + width;
	f32 y2 = y + height;
	guVector v[] = { { x, y, 0.0f }, { x2, y, 0.0f }, { x2, y2, 0.0f }, { x, y2, 0.0f }, { x, y, 0.0f } };

	GX_Begin(GX_TRIANGLEFAN, GX_VTXFMT0, 4);
	for(i = 0; i < 4; i++)
	{
		GX_Position3f32(v[i].x, v[i].y, v[i].z);
		GX_Color4u8(color.r, color.g, color.b, color.a);
	}
	GX_End();
	GX_SetTevOp(GX_TEVSTAGE0, GX_MODULATE);
}

void CVideo::screensaver(u32 no_input, u32 max_no_input)
{
	if(no_input == 0) // no idle time so reset alpha to 0 for next time and don't draw the rectangle on this pass
	{
		m_screensaver_alpha = 0;
		return;
	}	
	if(no_input > max_no_input) // if idle time > max idle draw rectangle full screen increasing alpha each time
	{
		DrawRectangle(0, 0, 640, 480, (GXColor){0,0,0,m_screensaver_alpha});
		if(m_screensaver_alpha < 150)
			m_screensaver_alpha+=2;
	}
}

/* misc settings */

void CColor::blend(const CColor &src)
{
	if (src.a == 0) return;
	r = (u8)(((int)src.r * (int)src.a + (int)r * (0xFF - (int)src.a)) / 0xFF);
	g = (u8)(((int)src.g * (int)src.a + (int)g * (0xFF - (int)src.a)) / 0xFF);
	b = (u8)(((int)src.b * (int)src.a + (int)b * (0xFF - (int)src.a)) / 0xFF);
}

CColor CColor::interpolate(const CColor &c1, const CColor &c2, u8 n)
{
	CColor c;
	c.r = (u8)(((int)c2.r * (int)n + (int)c1.r * (0xFF - (int)n)) / 0xFF);
	c.g = (u8)(((int)c2.g * (int)n + (int)c1.g * (0xFF - (int)n)) / 0xFF);
	c.b = (u8)(((int)c2.b * (int)n + (int)c1.b * (0xFF - (int)n)) / 0xFF);
	c.a = (u8)(((int)c2.a * (int)n + (int)c1.a * (0xFF - (int)n)) / 0xFF);
	return c;
}

void CVideo::setAA(u8 aa, bool alpha, int width, int height)
{
	if (aa <= 8 && aa != 7 && width <= m_rmode->fbWidth && height <= m_rmode->efbHeight && ((width | height) & 3) == 0)
	{
		m_aa = aa;
		m_aaAlpha = alpha;
		m_aaWidth = width;
		m_aaHeight = height;
	}
}

/* stencil used by coverflow */

const int CVideo::_stencilWidth = 128;
const int CVideo::_stencilHeight = 128;

static inline u32 coordsI8(u32 x, u32 y, u32 w)// used by stencilval below
{ 
	return (((y >> 2) * (w >> 3) + (x >> 3)) << 5) + ((y & 3) << 3) + (x & 7);
}

int CVideo::stencilVal(int x, int y)
{
	if ((u32)x >= m_rmode->fbWidth || (u32)y >= m_rmode->efbHeight)
		return 0;
	x = x * CVideo::_stencilWidth / 640;
	y = y * CVideo::_stencilHeight / 480;
	u32 i = coordsI8(x, y, (u32)CVideo::_stencilWidth);
	if (i >= (u32)(CVideo::_stencilWidth * CVideo::_stencilHeight))
		return 0;
	return ((u8*)m_stencil)[i];
}

void CVideo::prepareStencil(void)
{
	GX_SetPixelFmt(GX_PF_Y8, GX_ZC_LINEAR);
	_setViewPort(0.f, 0.f, (float)CVideo::_stencilWidth, (float)CVideo::_stencilHeight);
	GX_SetScissor(0, 0, CVideo::_stencilWidth, CVideo::_stencilHeight);
	GX_InvVtxCache();
	GX_InvalidateTexAll();
}

void CVideo::renderStencil(void)
{
	GX_DrawDone();
	GX_SetZMode(GX_DISABLE, GX_ALWAYS, GX_TRUE);
	GX_SetColorUpdate(GX_TRUE);
	GX_SetCopyFilter(GX_FALSE, NULL, GX_FALSE, NULL);
	GX_SetTexCopySrc(0, 0, CVideo::_stencilWidth, CVideo::_stencilHeight);
	GX_SetTexCopyDst(CVideo::_stencilWidth, CVideo::_stencilHeight, GX_CTF_R8, GX_FALSE);
	GX_CopyTex(m_stencil, GX_TRUE);
	GX_PixModeSync();
	DCFlushRange(m_stencil, CVideo::_stencilWidth * CVideo::_stencilHeight);
	GX_SetCopyFilter(m_rmode->aa, m_rmode->sample_pattern, GX_TRUE, m_rmode->vfilter);
}
