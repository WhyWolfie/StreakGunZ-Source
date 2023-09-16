#include "stdafx.h"

#include "ZGameClient.h"
#include "ZCombatInterface.h"
#include "ZGameInterface.h"
#include "ZInterfaceItem.h"
#include "ZInterfaceListener.h"
#include "ZApplication.h"
#include "ZCharacter.h"
#include "ZCharacterManager.h"
#include "RealSpace2.h"
#include "MComboBox.h"
#include "RTypes.h"
#include "ZScreenEffectManager.h"
#include "ZActionDef.h"
#include "ZEffectFlashBang.h"
#include "ZConfiguration.h"
#include "ZPost.h"
#include "ZWeaponScreenEffect.h"
#include "MemPool.h"
#include "ZMyInfo.h"
#include "ZCharacterView.h"
#include "ZItemSlotView.h"
#include <algorithm>
#include <Sensapi.h>
#pragma comment(lib, "Sensapi.lib")

#include "ZApplication.h"
#include "ZCombatQuestScreen.h"
#include "ZBmNumLabel.h"
#include "ZModule_QuestStatus.h"
#include "ZLocale.h"

#include "ZRuleDuel.h"
#include "ZRuleDuelTournament.h"
#include "ZRuleDeathMatch.h"
#include "ZInput.h"
#include "ZRuleQuestChallenge.h"
#include "MagicBox/ZRuleDropGunGame.h"

#include "MMatchSpyMode.h"
#include "MMatchSpyMap.h"

#ifdef _RADAR
#include "ZRadar.h"
#endif

// Custom: Disable NHN Auth
//#include "ZNHN_USA_Report.h"

using namespace std;


// CONSTANTS
#define BACKGROUND_COLOR1					0xff202020
#define BACKGROUND_COLOR2					0xff000000
#define BACKGROUND_COLOR_MYCHAR_DEATH_MATCH	MINT_ARGB(255*40/100,140,180,255)
#define BACKGROUND_COLOR_MYCHAR_RED_TEAM	MINT_ARGB(255*40/100,255,50,50)
#define BACKGROUND_COLOR_MYCHAR_BLUE_TEAM	MINT_ARGB(255*40/100,50,50,255)

#define BACKGROUND_COLOR_COMMANDER			MINT_ARGB(255*40/100,255,88,255)

#define TEXT_COLOR_TITLE			0xFFAAAAAA
#define TEXT_COLOR_DEATH_MATCH		0xfffff696
#define TEXT_COLOR_DEATH_MATCH_DEAD	0xff807b4b
#define TEXT_COLOR_BLUE_TEAM		0xff8080ff
#define TEXT_COLOR_BLUE_TEAM_DEAD	0xff606080
#define TEXT_COLOR_RED_TEAM			0xffff8080
#define TEXT_COLOR_RED_TEAM_DEAD	0xff806060
#define TEXT_COLOR_SPECTATOR		0xff808080
#define TEXT_COLOR_CLAN_NAME		0xffffffff

// Custom: FPS mod
bool g_bShownSprintMsg = false;
DWORD g_dwSprintMsgShowTime = 0;

// Custom: CTF
bool g_bShowCTFMsg = false;
DWORD g_dwCTFMsgShowTime = 0;
static char g_szCTFMsg[128] = { 0, };

// Custom: RTD
bool g_bShowRTDMsg = false;
DWORD g_dwRTDMsgShowTime = 0;
static char g_szRTDMsg[128] = { 0, };

// Custom: Scoreboard
DWORD g_dwScoreboardLastModTime = 0;

struct ZScoreBoardItem : public CMemPoolSm<ZScoreBoardItem>{
	MUID uidUID;
	char szFlag[16];
	char szLevel[16];
	char szName[64];
	char szClan[CLAN_NAME_LENGTH];
	int nDuelQueueIdx;
	int	nClanID;
	int nTeam;
	bool bDeath;
	int nExp;
	int nKills;
	int nDeaths;
	int nPing;
	int nDTLastWeekGrade;
	int nCountry;
	bool bMyChar;
	bool bCommander;
	bool bGameRoomUser;
	int nHitsDealt;
	int nHitsReceived;
	int nHitsfailed;
#ifdef _TAGVIP
	char szGrade[100];
#endif
	float nDmg;
	MCOLOR SpColor;
	bool  bSpColor;
	int  nScore;
	int nRank;

	ZScoreBoardItem( const MUID& _uidUID, char* _szFlag, char* _szLevel, char *_szName,char *_szClan,int _nTeam,bool _bDeath,int _nExp,int _nKills,int _nDeaths,int _nPing,int _nDTLastWeekGrade, int nCountry, bool _bMyChar,bool _bGameRoomUser,float _nDamage, int _nScore, int _nRank, bool _bCommander = false)
	{
		uidUID=_uidUID;
		strcpy(szFlag, szFlag);
		strcpy(szLevel,_szLevel);
		strcpy(szName,_szName);
		strcpy(szClan,_szClan);
		nTeam=_nTeam;
		bDeath=_bDeath;
		nExp=_nExp;
		nKills=_nKills;
		nDeaths=_nDeaths;
		nPing=_nPing;
		bMyChar = _bMyChar;
		bCommander = _bCommander;
		bSpColor = false;
		SpColor = MCOLOR(0,0,0);
		bGameRoomUser = _bGameRoomUser;
		nDTLastWeekGrade = _nDTLastWeekGrade;
		nCountry = nCountry;
		nDmg = _nDamage;
		nScore = _nScore;
		nRank = _nRank;
	}
	ZScoreBoardItem() {
		bSpColor = false;
		SpColor = MCOLOR(0, 0, 0);
	}

	void SetColor(MCOLOR c) { 
		SpColor = c;
		bSpColor = true;
	}

	MCOLOR GetColor() {
		return SpColor;
	}
};

ZCombatInterface::ZCombatInterface(const char* szName, MWidget* pParent, MListener* pListener)
: ZInterface(szName, pParent, pListener)
{
	m_fElapsed = 0;

	m_nBulletSpare = 0;
	m_nBulletCurrMagazine = 0;
	m_nMagazine = 0;
	memset(m_szItemName, 0, sizeof(m_szItemName));

//	m_pScoreBoard = NULL;
	m_pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_bMenuVisible = false;
	m_bIsFrozen = false;
	
	m_bPickTarget = false;
	m_pLastItemDesc = NULL;
	
	m_bReserveFinish = false;
	
	m_pTargetLabel = NULL;
	m_szTargetName[0] = 0;

	m_nBulletImageIndex = 0;
	m_nMagazineImageIndex = 0;

	m_nReserveFinishTime = 0;

	m_pWeaponScreenEffect = NULL;

	m_pResultPanel=NULL;
	m_pResultPanel_Team = NULL;
	m_pResultLeft = NULL;
	m_pResultRight = NULL;

	m_pQuestScreen = NULL;

//	m_bKickPlayerListVisible = false;

	m_nClanIDRed = 0;
	m_nClanIDBlue = 0;
	m_szRedClanName[0] = 0;
	m_szBlueClanName[0] = 0;

	m_bNetworkAlive = true;		// ÀÎÅÍ³Ý ¿¬°áµÇ¾ûÜÖÀ½
	m_dLastTimeTick = 0;
	m_dAbuseHandicapTick = 0;

	m_bSkipUIDrawByRule = false;
	
	m_nScoreBoardIndexStart = 0;
	m_dwLastScoreBoardScrollTime = 0;

	m_pInfectedOverlay = NULL;
	m_pInfectedWidescreenOverlay = NULL;

	m_bSpyLocationOpened = false;
	m_pSpyIcon = NULL;

	m_dwSpyTimer = 0;

	m_bSpyNoticePlaying = false;
	m_dwSpyNoticePlayStartedTime = 0;

}

ZCombatInterface::~ZCombatInterface()
{
	OnDestroy();	
}

bool ZCombatInterface::OnCreate()
{
	ZGetGame()->m_pMyCharacter->EnableAccumulationDamage(false);

	m_Observer.Create(ZApplication::GetGameInterface()->GetCamera(), 
					  ZApplication::GetGameInterface()->GetIDLResource());


	m_pTargetLabel = new MLabel("", this, this);
	m_pTargetLabel->SetTextColor(0xffff0000);
	m_pTargetLabel->SetSize(100, 30);

	ShowInfo(true);

	m_pResultBgImg = NULL;
	m_bDrawScoreBoard = false;

	EnableInputChat(false);

	m_Chat.Create( "CombatChatOutput",true);
	m_Chat.ShowOutput(ZGetConfiguration()->GetViewGameChat());
	m_Chat.m_pChattingOutput->ReleaseFocus();

	m_AdminMsg.Create( "CombatChatOutputAdmin",false);
	MFont* pFont = MFontManager::Get( "FONTb11b");
	m_AdminMsg.SetFont( pFont);
	m_AdminMsg.m_pChattingOutput->ReleaseFocus();

	if (ZGetMyInfo()->IsAdminGrade()) {
		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		if (pCache && pCache->GetUGrade()==MMUG_EVENTMASTER && pCache->CheckFlag(MTD_PlayerFlags_AdminHide)) {
			ShowChatOutput(false);
		}
	}

	m_ppIcons[0]=MBitmapManager::Get("medal_A.tga");
	m_ppIcons[1]=MBitmapManager::Get("medal_U.tga");
	m_ppIcons[2]=MBitmapManager::Get("medal_E.tga");
	m_ppIcons[3]=MBitmapManager::Get("medal_F.tga");
	m_ppIcons[4]=MBitmapManager::Get("medal_H.tga");

	for (int i = 0; i < MMATCH_GAMETYPE_MAX; ++i)
		m_pGameModeBitmap[i] = NULL;

	m_nScoreBoardIndexStart = 0;
	m_dwLastScoreBoardScrollTime = 0;
	g_dwScoreboardLastModTime = 0;

	m_CrossHair.Create();
	m_CrossHair.ChangeFromOption();

	m_pWeaponScreenEffect = new ZWeaponScreenEffect;
	m_pWeaponScreenEffect->Create();

#ifdef _RADAR
	if (strstr(ZGetGameClient()->GetChannelName(), "Ladder"))
	{
		m_pRadar = new ZRadar();
		m_pRadar->OnCreate();
	}
#endif

	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
	{
		m_pQuestScreen = new ZCombatQuestScreen();
	}
	
	// Custom: CTF
	if ( IsGameRuleCTF(ZGetGame()->GetMatch()->GetMatchType()) )
	{
		// For the arrow
		ZGetScreenEffectManager()->CreateQuestRes();
	}

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatTDMInfo");
	if ( pWidget)
	{
		// Custom: CTF
		if ( ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()) )
		{
			int nMargin[ BMNUM_NUMOFCHARSET] = { 13,9,13,13,13,13,13,13,13,13,8,10,8 };

			ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Blue");
			if ( pBmNumLabel)
			{
				pBmNumLabel->SetAlignmentMode( MAM_HCENTER);
				pBmNumLabel->SetCharMargin( nMargin);
				pBmNumLabel->SetNumber( 0);
			}

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Red");
			if ( pBmNumLabel)
			{
				pBmNumLabel->SetAlignmentMode( MAM_HCENTER);
				pBmNumLabel->SetIndexOffset( 16);
				pBmNumLabel->SetCharMargin( nMargin);
				pBmNumLabel->SetNumber( 0);
			}

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Max");
			if ( pBmNumLabel)
			{
				pBmNumLabel->SetAlignmentMode( MAM_HCENTER);
				pBmNumLabel->SetIndexOffset( 32);
				int nMargin2[ BMNUM_NUMOFCHARSET] = { 18,12,18,18,18,18,18,18,18,18,18,18,18 };
				pBmNumLabel->SetCharMargin( nMargin2);
				pBmNumLabel->SetNumber( 0);
			}

			pWidget->Show( true);


			MWidget *pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_RedWin");
			if ( pPicture)
				pPicture->Show( true);
			pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_BlueWin");
			if ( pPicture)
				pPicture->Show( true);
		}
		else
		{
			pWidget->Show( false);

			MWidget *pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_RedWin");
			if ( pPicture)
				pPicture->Show( false);
			pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_BlueWin");
			if ( pPicture)
				pPicture->Show( false);
		}
	}
	
	// Custom: Infected
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
	{
		SAFE_DELETE(m_pInfectedOverlay);
		SAFE_DELETE(m_pInfectedWidescreenOverlay);
		m_pInfectedOverlay = new MBitmapR2;
		m_pInfectedWidescreenOverlay = new MBitmapR2;
		m_pInfectedOverlay->Create("zombie_overlay.dds", RGetDevice(), "interface/Default/Combat/zombie_overlay.dds");
		m_pInfectedWidescreenOverlay->Create("zombie_overlay_ws.dds", RGetDevice(), "interface/Default/Combat/zombie_overlay_ws.dds");
	}
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		// µà¾óÅä³Ê¸ÕÆ® ³²Àº½Ã°£ Ç¥½Ã
		MWidget *pPicture = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_TimeBack");
		if ( pPicture)
			pPicture->Show( true);

		ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "DT_RemainTime");

		if(pBmNumLabel)
		{
			pBmNumLabel->SetAlignmentMode( MAM_HCENTER);
			pBmNumLabel->SetIndexOffset( 32);
			int nMargin[ BMNUM_NUMOFCHARSET] = { 18,12,18,18,18,18,18,18,18,18,18,18,18 };
			pBmNumLabel->SetCharMargin(nMargin);
			pBmNumLabel->SetNumber(0);
		}

		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDTInfo");
		if ( pWidget)
			pWidget->Show( true);

		// ?EE?Á¤º¸ ·¹ÀÌ?Ealign (·¹ÀÌºú}lignÀº ¹ö±×°¡ ÀÖ¾ûØ­ xml¿¡¼­ ÁöÁ¤ÇÒ?E¾ø´Ù-_-;)
		MWidget* pFrame = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_CharacterInfo");
		if (pFrame)
		{
			int numChild = pFrame->GetChildCount();
			MWidget* pChild = NULL;
			for (int i=0; i<numChild; ++i)
			{
				pChild = pFrame->GetChild(i);
				if (pChild &&
					strcmp(pChild->GetClassName(), MINT_LABEL) == 0 &&
					strstr(pChild->m_szIDLName, "CombatDT_PlayerInfo_"))
				{
					if (strstr(pChild->m_szIDLName, "Left"))
						((MLabel*)pChild)->SetAlignment(MAM_RIGHT | MAM_VCENTER);
					else if (strstr(pChild->m_szIDLName, "Right"))
						((MLabel*)pChild)->SetAlignment(MAM_LEFT | MAM_VCENTER);
					else
						((MLabel*)pChild)->SetAlignment(MAM_HCENTER | MAM_VCENTER);
				}
			}
		}
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDT_MatchLevel");
        if (pLabel)
			pLabel->SetAlignment(MAM_HCENTER | MAM_VCENTER);

		GetWidgetCharViewLeft()->SetEnableRotateZoom(false, false);
		GetWidgetCharViewRight()->SetEnableRotateZoom(false, false);
		GetWidgetCharViewResult()->SetEnableRotateZoom(false, false);

		// ´©Àûµ¥¹Ì?EÀû?Eµà¾óÅä³Ê¸ÕÆ®ÀÏ½Ã OK½ÂÀÌ Èûµé¶§ ´©Àûµ¥¹ÌÁö·Î ÆÇÁ¤½Â Ã³¸®?E
		ZGetGame()->m_pMyCharacter->EnableAccumulationDamage(true);
	}

	// °ÔÀÓ ³ª°¡?E¹öÆ°ÀÇ È®ÀÎ ¸Þ½ÃÁö¸¦ °ÔÀÓ?E?µû?Eº¯°æÇÑ´Ù
	MButton* pExitConfirmButton = (MButton*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageExit");
	if (pExitConfirmButton) {
		char szConfirmMsg[256];
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			ZTransMsg(szConfirmMsg, MSG_GAME_DUELTOURNAMENT_MATCH_EXITSTAGE_CONFIRM);	// TP Æä³ÎÆ¼ °æ°úÕ®
			pExitConfirmButton->SetAlterableConfirmMessage(szConfirmMsg);
		}
		else
		{
			// ±×¿Ü °ÔÀÓ?E??EØ¼­´Â µðÆúÆ® ¸Þ½Ã?E
			pExitConfirmButton->RestoreIDLConfirmMessage();
		}
	}
	if (ZGetGame())
	{
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
		{
			m_pSpyIcon = new MBitmapR2;
			m_pSpyIcon->Create("icon_spy.tga", RGetDevice(), "interface/default/icon_spy.tga");

			OnSpyCreate();
		}
	}
#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->Set3DSoundUpdate( true );
#endif

	m_bOnFinish = false;
	m_bShowResult = false;
	m_bIsShowUI = true;

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Option");
	if ( pWidget)
		pWidget->Show( false);

	return true;
}


void ZCombatInterface::OnDestroy()
{
	if(m_nClanIDBlue) {
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanIDBlue);
		m_nClanIDBlue = 0;
	}
	if(m_nClanIDRed) {
		ZGetEmblemInterface()->DeleteClanInfo(m_nClanIDRed);
		m_nClanIDRed = 0;
	}
	
	// Custom: Infected
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
	{
		SAFE_DELETE(m_pInfectedOverlay);
		SAFE_DELETE(m_pInfectedWidescreenOverlay);
	}
	// Custom: CTF
	if (IsGameRuleCTF(ZGetGame()->GetMatch()->GetMatchType()))
	{
		ZGetScreenEffectManager()->DestroyQuestRes();
	}
	// Custom: SpyMode
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
	{
		SAFE_DELETE(m_pSpyIcon);
	}
	for (int i = 0; i < MMATCH_GAMETYPE_MAX; ++i)
	{
		SAFE_DELETE(m_pGameModeBitmap[i]);
	}

	if (m_pQuestScreen){ delete m_pQuestScreen; m_pQuestScreen=NULL; }

	m_Observer.Destroy();

	m_ResultItems.Destroy();
	SAFE_DELETE(m_pResultPanel);
	SAFE_DELETE(m_pResultPanel_Team);
	SAFE_DELETE(m_pResultLeft);
	SAFE_DELETE(m_pResultRight);

#ifdef _RADAR
	if (strstr(ZGetGameClient()->GetChannelName(), "Ladder"))
	{
		SAFE_DELETE(m_pRadar);
	}
#endif

	EnableInputChat(false);

	m_Chat.Destroy();
	m_AdminMsg.Destroy();

	/*
	if (m_pScoreBoard)
	{
		m_pScoreBoard->OnDestroy();
		delete m_pScoreBoard;
		m_pScoreBoard = NULL;
	}
	*/
	m_CrossHair.Destroy();

	if (m_pTargetLabel)
	{
		delete m_pTargetLabel;
		m_pTargetLabel = NULL;
	}
	ShowInfo(false);


	//if(ZGetGame())
	//{
	//	if(ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
	//	{
	if (m_pSpyIcon)
	{
		delete m_pSpyIcon;
		m_pSpyIcon = NULL;
	}

	OnSpyDestroy();
	//	}
	//}


#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->Set3DSoundUpdate( false );
#endif

	MPicture *pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "GameResult_Background");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	if ( m_pResultBgImg != NULL)
	{
		delete m_pResultBgImg;
		m_pResultBgImg = NULL;
	}


	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatTDMInfo");
	if ( pWidget)
		pWidget->Show( false);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatDTInfo");
	if ( pWidget)
		pWidget->Show( false);

	ZCharacterView* pCharView = GetWidgetCharViewLeft();
	if (pCharView)
		pCharView->SetCharacter(MUID(0,0));
	
	pCharView = GetWidgetCharViewRight();
	if (pCharView)
		pCharView->SetCharacter(MUID(0,0));

	pCharView = GetWidgetCharViewResult();
	if (pCharView)
		pCharView->SetCharacter(MUID(0,0));

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatDT_CharacterInfo");
	if (pWidget)
		pWidget->Show(false);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatChallQuestInfo");
	if (pWidget)
		pWidget->Show(false);

	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Blitzkrieg");
	if (pWidget)
		pWidget->Show(false);

	// Custom: Fixed calling destructor of ZWeaponScreenEffect
	if( m_pWeaponScreenEffect )
		m_pWeaponScreenEffect->Destroy();

	SAFE_DELETE(m_pWeaponScreenEffect);

	ZScoreBoardItem::Release();
}

void TextRelative(MDrawContext* pDC,float x,float y,const char *szText,bool bCenter)
{
	if( RGetIsWidthScreen() )
		x = (x*800+80)/960.f;

	int screenx=x*MGetWorkspaceWidth();
	if(bCenter)
	{
		MFont *pFont=pDC->GetFont();
		screenx-=pFont->GetWidth(szText)/2;
	}

	pDC->Text(screenx,y*MGetWorkspaceHeight(),szText);
}

void BitmapRelative(MDrawContext* pDC, float x, float y, float w, float h, MBitmap* pBitmap, bool bCenter=false)
{
	pDC->SetBitmap( pBitmap);

	if( RGetIsWidthScreen() )
		x = (x*800+80)/960.f;

	int screenx=x*MGetWorkspaceWidth();
	if(bCenter)
	{
		MFont *pFont=pDC->GetFont();
		screenx-=w/2;
	}

	pDC->Draw( screenx, y*MGetWorkspaceHeight(), w, h);
}

void MatchOrderRelative(MDrawContext* pDC, float x, float y, float fHalfGrid, int nMatchCount, int nCouple, bool bBlink)
{
	// UI°¡ ³ª¿À?E±×¸²À¸·Î ?E¼ÇÒ²¨À?... ±×·¡¼­ ¸ÅÁ÷³Ñ?E»ç?E
	float screenx=x*MGetWorkspaceWidth();
	float screeny=y*MGetWorkspaceHeight();

	// ?EøÇ?¹Ú½º ±×·ÁÁÖ?E
	float fRectX = screenx-2;
	float fRectY = screeny-4;
	float fRectWidth = 0.138f*MGetWorkspaceWidth();
	float fRectHeight = 0.04f*MGetWorkspaceHeight();
	pDC->Rectangle(fRectX, fRectY, fRectWidth, fRectHeight );

	if(bBlink)
		return;
	// ?EøÇ?¼±?E±×·ÁÁÖ?E
	float fLineWidth = 0.032f*MGetWorkspaceWidth();
	float fLineHeight = 0.038f*MGetWorkspaceHeight();

	MDUELTOURNAMENTTYPE eDTType = ZApplication::GetGameInterface()->GetDuelTournamentType();
	switch(eDTType)
	{
	case MDUELTOURNAMENTTYPE_FINAL:				//< °á½Â?EÎÅ?
		{
			fLineWidth = 0.145f*MGetWorkspaceWidth();
			if(nCouple != 1)
				pDC->HLine(fRectX+fRectWidth, fRectY+fRectHeight/2, fLineWidth);
			return;
		}
		break;
	case MDUELTOURNAMENTTYPE_SEMIFINAL:			//< 4°­?EÎÅ?
		{
			fLineWidth = 0.082f*MGetWorkspaceWidth();
			fLineHeight = 0.049f*MGetWorkspaceHeight();
		}
		break;
	case MDUELTOURNAMENTTYPE_QUATERFINAL:		//< 8°­?EÎÅ?
		{
			fLineWidth = 0.032f*MGetWorkspaceWidth();
			fLineHeight = 0.038f*MGetWorkspaceHeight();
		}
		break;
	}

	fLineHeight *= nMatchCount+0.9f;
	float fHalf = fHalfGrid*MGetWorkspaceHeight();
	if(nCouple)
	{
		fLineHeight = -fLineHeight; // ?EøÇ?¼¼·Î¼±À» À§·Î ±×¸±?E¾Æ·¡·Î ±×¸±?EÁ¤ÇØÁØ´Ù.
		fHalf = -fHalf;
	}
	// ¹Ú½º ?EE°¡·Î¼±
	pDC->HLine(fRectX+fRectWidth, fRectY+fRectHeight/2, fLineWidth);
	// ¹Ú½º ?EE¾Æ·¡ ¼¼·Î¼±
	pDC->VLine(fRectX+fRectWidth+fLineWidth, fRectY+fRectHeight/2, fLineHeight);
	// ¹Ú½º ÇÑ½Ö °¡¿û?E¼±
	if(nMatchCount != eDTType+1) // Final °¡¿ûÑ¥ ¼±Àº ¾È±×¸°´Ù.
		pDC->HLine(fRectX+fRectWidth+fLineWidth, fRectY+fRectHeight/2 + fHalf, fLineWidth);
}

void ZCombatInterface::DrawNPCName(MDrawContext* pDC)
{
	for(ZObjectManager::iterator itor = ZGetObjectManager()->begin();
		itor != ZGetObjectManager()->end(); ++itor)
	{
		rvector pos, screen_pos;
		ZObject* pObject= (*itor).second;
		if (!pObject->IsVisible()) continue;
		if (pObject->IsDie()) continue;
		if(!pObject->IsNPC()) continue;

		ZActor *pActor = (ZActor*)pObject;
//		if(!pActor->CheckFlag(AF_MY_CONTROL)) continue;


		pos = pObject->GetPosition();
		RVisualMesh* pVMesh = pObject->m_pVMesh;
		RealSpace2::rboundingbox box;

		if (pVMesh == NULL) continue;
		
		box.vmax = pos + rvector(50.f, 50.f, 190.f);
		box.vmin = pos + rvector(-50.f, -50.f, 0.f);

		if (isInViewFrustum(&box, RGetViewFrustum()))
		{
			screen_pos = RGetTransformCoord(pObject->GetPosition()+rvector(0,0,100.f));

			MFont *pFont=NULL;
			pFont = pActor->CheckFlag(AF_MY_CONTROL) ? MFontManager::Get("FONTa12_O1Blr") : MFontManager::Get("FONTa12_O1Red");
			pDC->SetColor(MCOLOR(0xFF00FF00));
			pDC->SetBitmap(NULL);
			pDC->SetFont(pFont);

			int x = screen_pos.x - pDC->GetFont()->GetWidth(pActor->m_szOwner) / 2;
			pDC->Text(x, screen_pos.y - 12, pActor->m_szOwner);

			// ¸¶Áö¸· Á¤º¸¸¦ ¹ÞÀº½Ã°£ÀÌ ÀÌ»óÇÏ?E?EûÝØ´?
			float fElapsedTime = ZGetGame()->GetTime() - pActor->m_fLastBasicInfo;
			if(!pActor->CheckFlag(AF_MY_CONTROL) && fElapsedTime>.2f) {
				int y= screen_pos.y;
				y+=pFont->GetHeight();
				char temp[256];
				sprintf(temp,"%2.2f",fElapsedTime);
				x = screen_pos.x - pDC->GetFont()->GetWidth(temp) / 2;
				pDC->Text(x, y - 12, temp);
			}
		}
	}
}

void ZCombatInterface::DrawTDMScore(MDrawContext* pDC)
{
	int nBlueKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE);
	int nRedKills = ZGetGame()->GetMatch()->GetTeamKills(MMT_RED);
	int nTargetKills = ZGetGameClient()->GetMatchStageSetting()->GetRoundMax();


	ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Blue");
	if ( pBmNumLabel)
		pBmNumLabel->SetNumber( nBlueKills);

	pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Red");
	if ( pBmNumLabel)
		pBmNumLabel->SetNumber( nRedKills);

	pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Max");
		pBmNumLabel->SetNumber( nTargetKills);


	MWidget* pRed = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_RedWin");
	MWidget* pBlue = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_BlueWin");
	if ( pRed && pBlue)
	{
		int nTime[] = { 1, 1400, 1400, 900, 900, 200 };
		int nDiff = min( abs( nBlueKills - nRedKills) , 5);
		int nCurrTime = timeGetTime() % nTime[ nDiff];

		if (IsGameRuleCTF(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		{
			nCurrTime = timeGetTime() % 300;
			if (nCurrTime > 100)
			{
				pRed->Show( false);
				pBlue->Show( false);

				return;
			}

			ZRuleTeamCTF* pRule = (ZRuleTeamCTF*)ZGetGame()->GetMatch()->GetRule();

			if (!pRule) return;

			pRed->Show( pRule->GetBlueCarrier().IsValid());
			pBlue->Show( pRule->GetRedCarrier().IsValid());

			return;
		}

		if (nDiff == 0)
		{
			pRed->Show( false);
			pBlue->Show( false);

			return;
		}
		if ( (nDiff == 1) || (nDiff == 2) || ( nDiff >= 5))
		{
			if ( nCurrTime > 100)
			{
				pRed->Show( false);
				pBlue->Show( false);

				return;
			}
		}
		else if ( (nDiff == 3) || (nDiff == 4))
		{
			if ( ((nCurrTime > 100) && (nCurrTime < 200)) || (nCurrTime > 300))
			{
				pRed->Show( false);
				pBlue->Show( false);

				return;
			}
		}


		if ( nRedKills > nBlueKills)
		{
			pRed->Show( true);
			pBlue->Show( false);
		}
		else if ( nRedKills < nBlueKills)
		{
			pRed->Show( false);
			pBlue->Show( true);
		}
		else
		{
			pRed->Show( false);
			pBlue->Show( false);
		}
	}
}
void ZCombatInterface::EventAlive(MDrawContext* pDC)
{
	ZCharacter* pCharacter = GetTargetCharacter();

	if (pCharacter == NULL || pCharacter->IsObserverTarget())
		return;

	char szAlive[64]; // alive
	pDC->SetFont(GetGameFontHPAP()); // new font
	pDC->SetColor(MCOLOR(255, 255, 255)); // color

	if (strstr(ZGetGameClient()->GetChannelName(), "Event") && ZGetMyInfo()->IsAdminGrade()) // channel
	{
		int nPlayers = 0;
		for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin(); itor != ZGetGame()->m_CharacterManager.end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;
			if (ZGetGame()->GetMatch()->IsTeamPlay())
			{
				if (!pCharacter->IsDie() && pCharacter->GetTeamID() != ZGetGame()->m_pMyCharacter->GetTeamID())
				{
					++nPlayers;
				}
			}
			else if (!pCharacter->IsDie() && pCharacter != ZGetGame()->m_pMyCharacter)
			{
				++nPlayers;
			}
		}
		sprintf(szAlive, "(%d) Players", nPlayers);
		TextRelative(pDC, 10.f / 800.f, 100.f / 600.f, szAlive);
	}
}
/*void ZCombatInterface::DrawHPAPBars(MDrawContext* pDC)
{
	bool bClanWars = ZGetGameClient()->IsCWChannel();
	bool bPlayerWars = ZGetGameClient()->IsPWChannel();

	MFont* pFont = MFontManager::Get("FONTa10_O2Wht");
	pDC->SetFont(pFont);

	if (bClanWars && bPlayerWars)
	{
		ZCharacter* pTargetCharacter = GetTargetCharacter();
		if (pTargetCharacter == NULL) return;

		float texty = 0.75000f;
		float x = 0.024f;
		float ysub = 0.05000f;

		for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
			itor != ZGetGame()->m_CharacterManager.end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;
			if (!pCharacter->IsVisible()) continue;
			if (pCharacter->IsDie()) continue;
			if (pCharacter->GetTeamID() != pTargetCharacter->GetTeamID()) continue;
			if (pCharacter == pTargetCharacter) continue;

			// Orby: HP/AP Bars.
			MCOLOR tmpColor = pDC->GetColor();
			pDC->SetColor(MCOLOR(240, 248, 255));
			pDC->Text(x * MGetWorkspaceWidth(), texty * MGetWorkspaceHeight() - 24, pCharacter->GetUserNameA());

			pDC->SetColor(MCOLOR(80, 80, 80));
			pDC->FillRectangle((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, 0.08 * MGetWorkspaceWidth(), 7);

			float nHP = 0.08 * pCharacter->GetHP() / pCharacter->GetMaxHP();
			pDC->SetColor(MCOLOR(0, 140, 230));
			pDC->FillRectangle((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, nHP * MGetWorkspaceWidth(), 7);

			pDC->SetColor(MCOLOR(80, 80, 80));
			pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, 0.08 * MGetWorkspaceWidth(), 7);

			float nAP = 0.08 * pCharacter->GetAP() / pCharacter->GetMaxAP();
			pDC->SetColor(MCOLOR(68, 193, 62));
			pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, nAP * MGetWorkspaceWidth(), 7);
			pDC->SetColor(tmpColor);
			texty -= ysub;
		}
	}
}*/
void ZCombatInterface::UpdateNetworkAlive(MDrawContext* pDC)
{
	DWORD dw;
	BOOL bIsNetworkAlive = IsNetworkAlive(&dw);
	if (!bIsNetworkAlive)
	{
		m_dAbuseHandicapTick = timeGetTime() + 2000;
		m_bNetworkAlive = false;
	}
	if (bIsNetworkAlive && timeGetTime() > m_dAbuseHandicapTick)
	{
		m_bNetworkAlive = true;
	}
	return;
}
void ZCombatInterface::OnDraw(MDrawContext* pDC)
{
//#ifdef LOCALE_KOREA

	if(timeGetTime() - m_dLastTimeTick > 500)
	{
		UpdateNetworkAlive(pDC);
		m_dLastTimeTick = timeGetTime();
	}
//#endif // LOCALE_KOREA



	if ( m_bShowResult)
		return;

	if (IsShowUI() && !m_bSkipUIDrawByRule)
	{
		if( ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED 
			&& (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY || ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH))
		{
			static int nFadeOpacity = 150;
			static bool bUp = true;
			if ((!m_Observer.IsVisible() && ZGetGame()->m_pMyCharacter->m_bInfected) || (m_Observer.IsVisible() && m_Observer.GetTargetCharacter()->m_bInfected))
			{
				// Resolution change fix
				MBitmap* pBitmapSelection = (RGetIsWidthScreen() || RGetIsLongScreen()) ? m_pInfectedWidescreenOverlay : m_pInfectedOverlay;

				if (pBitmapSelection)
				{
					DWORD elapsed = timeGetTime() - g_dwScoreboardLastModTime;

					pDC->SetEffect(MDE_ADD); // the magic trick to ignore black for alpha
					pDC->SetOpacity(nFadeOpacity); // 75 percent of 0xFF (255)
					pDC->SetBitmap(pBitmapSelection);
					pDC->Draw(0, 0, RGetScreenWidth(), RGetScreenHeight());
					pDC->SetOpacity(255); // Reset opacity
					pDC->SetBitmap(NULL); // Reset bitmap
					pDC->SetEffect(MDE_NORMAL);

					if (bUp)
					{
						if (nFadeOpacity == 255)
							bUp = false;

						if (elapsed > 300)
						{
							g_dwScoreboardLastModTime = timeGetTime();
							nFadeOpacity++;
						}
					}
					else
					{
						if (nFadeOpacity == 150)
							bUp = true;

						if (elapsed > 300)
						{
							g_dwScoreboardLastModTime = timeGetTime();
							nFadeOpacity--;
						}
					}
				}
			}
		}
	}

	bool bDrawAllPlayerName = false;

	//Name ESP Command Admin
	if (ZGetGame()->m_pMyCharacter->GetStatus().Ref().isESP == 1)
		bDrawAllPlayerName = true;

	if(ZGetGame()->m_pMyCharacter->IsAdminHide() && MEvent::GetAltState() && ZGetCamera()->GetLookMode() != ZCAMERA_MINIMAP)
		bDrawAllPlayerName = true;
		
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ESP) || (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE) && ZGetGame()->GetRolledDice() == 3))
		bDrawAllPlayerName = true;
	
	// Custom: Draw all players in quest.
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) || ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		bDrawAllPlayerName = false;

	//if ((ZGetCamera()->GetLookMode() == ZCAMERA_FREELOOK && ZGetGame()->IsReplay()) || (bDrawAllPlayerName))
	if ((ZGetCamera()->GetLookMode() == ZCAMERA_DEFAULT && ZGetGame()->IsReplay()) || (bDrawAllPlayerName))
	{		                                                    
		DrawAllPlayerName(pDC); // Custom: Off ESP Replay
	}
	else
	{
		if (!ZGetGameInterface()->IsMiniMapEnable())
		{
			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
				DrawSpyName(pDC);
			else
				DrawFriendName(pDC);

			DrawEnemyName(pDC);
		}
	}

	GetVoteInterface()->DrawVoteTargetlist(pDC);
	GetVoteInterface()->DrawVoteMessage(pDC);
	ZGetScreenEffectManager()->Draw();

	if (IsShowUI())
	{
		m_Chat.OnDraw(pDC);

		if (!m_bSkipUIDrawByRule)
		{
			if (!m_Observer.IsVisible())
				m_pWeaponScreenEffect->Draw(pDC);

			if (!ZGetGame()->m_pMyCharacter->IsObserverTarget() &&
				!ZGetGame()->GetMatch()->IsQuestDrived() &&
				!ZGetGame()->GetMatch()->IsRuleSpy() &&
				!ZGetGame()->GetMatch()->IsQuestChallengue() &&
				!ZGetGame()->m_pMyCharacter->IsDie() &&
				!ZGetGameClient()->IsDuelTournamentGame())
			{
				ZCharacter* pCharacter = ZGetGame()->m_pMyCharacter;
				if (pCharacter && !pCharacter->IsDie())
				{
					MFont* pFont = GetGameFont();
					pDC->SetFont(pFont);
					pDC->SetColor(MCOLOR(0xFFFFFFFF));
					pDC->SetFont(MFontManager::Get("FONTa6_O2Wht"));
					char szMsg[220];
					{
						// Custom: Extra info Mode PRO Player
						if (ZIsActionKeyPressed(ZACTION_SCORE) == false())
						{
							if (ZGetConfiguration()->GetEtc()->bExtraStreak)
							{
								sprintf(szMsg, "%d%%", ZGetMyInfo()->GetLevelPercent());
								pDC->SetColor(MCOLOR(0xFFFFFFFF));
								TextRelative(pDC, 245.f / 800.f, 71.f / 600.f, szMsg);

								sprintf(szMsg, "Total Damage: Taken: %d - Given: %d", pCharacter->GetStatus().Ref().nTakenDamage, pCharacter->GetStatus().Ref().nGivenDamage);
								pDC->SetColor(MCOLOR(0xFFFFFFFF));
								TextRelative(pDC, 10.f / 800.f, 90.f / 600.f, szMsg);

								sprintf(szMsg, "Damage: Taken: %d - Given: %d", pCharacter->GetStatus().Ref().nRoundTakenDamage, pCharacter->GetStatus().Ref().nRoundGivenDamage);
								pDC->SetColor(MCOLOR(0xFFFFFFFF));
								TextRelative(pDC, 10.f / 800.f, 100.f / 600.f, szMsg);
#ifdef _DRAWHPAP	
								DrawMyHPAPPont(pDC);
#endif
							}
						}
					}
				}
			}

#ifdef _COUNTRYFLAG
			/*if (ZGetGameClient()->IsPWChannel() && ZGetGameClient()->IsCWChannel())
			{
				GEOID myGEO = GetUserGeoID(GEOCLASS_REGION);
				ZCharacter* pCharacter = ZGetGame()->m_pMyCharacter;
				ZPostSendCountryCode(pCharacter->GetProperty()->GetName(), myGEO);
			}*/
#endif

			EventAlive(pDC); // Custom: Live Player Room
			// Custom: Draw my HP/AP on screen over bars
			if( !ZGetGame()->IsReplay() && !m_Observer.IsVisible() && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT )
			{
				ZCharacter* pmyCharacter = ZGetGame()->m_pMyCharacter;
				if( pmyCharacter != NULL )
				{
					// Draw it nicely over different resolutions..
					float fRx = (float)MGetWorkspaceWidth();
					float fRy = (float)MGetWorkspaceHeight();

					// Coordinates. (made in 1024x768)
					float fX = (155.0f / 1024.0f) * fRx;
					float fY = (28.0f / 768.0f) * fRy;
					float fY_AP = (35.0f / 768.0f) * fRy;

					char szMsg[128] = { 0, };

					// Modify X coords for widescreen 16:9 / 16:10 aspect ratios
					if( RGetIsWidthScreen() )
						fX += 80.0f;
					//else if( RGetIsLongScreen() )
					//	fX += 80.0f; // debug
						//fX += 90.0f;

					// Set a static color to the text so that it doesn't change when you aim someone
					/*MFont *pFont = MFontManager::Get( "FONTa10_O2Wht");

					if ( pFont == NULL )
						pDC->SetFont( GetFont() );
					else
						pDC->SetFont( pFont );

					pDC->SetColor( MCOLOR(0xFFFFFFFF) );
					pDC->SetOpacity(220);
					sprintf(szMsg, "%.0f / %.0f", pmyCharacter->GetHP(), pmyCharacter->GetMaxHP());
					pDC->Text((int)fX, (int)fY, szMsg);
					sprintf(szMsg, "%.0f / %.0f", pmyCharacter->GetAP(), pmyCharacter->GetMaxAP());
					pDC->Text((int)fX, (int)(fY + fY_AP), szMsg);*/
				}
			}

			// Custom: Disable debug drawing of HP/AP
			/*
#ifdef _DEBUG
			// TodoH(?E - Áö¿öÁ®¾ß µË´Ï´Ù. ÃßÈÄ¿¡.
			ZCharacter* pCharacter = GetTargetCharacter();
			if( pCharacter != NULL ) {
				//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® ZCharacterBuff *pCharBuff = pCharacter->GetCharacterBuff();

				int nX = 300;
				int nY = 50;
				char szMsg[128] = { 0, };

				sprintf(szMsg, "HP : %f / %f", pCharacter->GetHP(), pCharacter->GetMaxHP());
				pDC->Text(nX, nY, szMsg);

				sprintf(szMsg, "AP : %f / %f", pCharacter->GetAP(), pCharacter->GetMaxAP());
				pDC->Text(nX, nY + 30, szMsg);
			}			
#endif
			*/


#ifdef _RADAR
			if (strstr(ZGetGameClient()->GetChannelName(), "Ladder"))
			{
			  if (ZIsActionKeyPressed(ZACTION_SCORE) == false || ZGetGame()->GetMatch()->IsTeamPlay())
			{
				m_pRadar->OnDraw(pDC);
			}
			if (ZGetGame()->GetMatch()->IsTeamPlay())
			{
				char buffer[256];
				sprintf(buffer, "RADAR");
				TextRelative(pDC, 705.f / 800.f,          //Left Right 
					350.f / 600.f, buffer); //Up Down
			}
			}
#endif      // Custom: HideExtra Default Press TAB
			if (ZIsActionKeyPressed(ZACTION_SCORE) == false())
			{
				ZGetScreenEffectManager()->DrawMyWeaponImage();
				ZGetScreenEffectManager()->DrawMyBuffImage();
				DrawMyWeaponPont(pDC);
				if (m_pQuestScreen) m_pQuestScreen->OnDraw(pDC);
				ZGameClient* pGameClient = ZGetGameClient();
				ZGetScreenEffectManager()->DrawMyHPPanal(pDC);
				DrawSoloSpawnTimeMessage(pDC);
				DrawMyNamePont(pDC);;
				DrawPont(pDC);
			}


			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DROPMAGIC && ZIsActionKeyPressed(ZACTION_SCORE) == false)
			{
				ZRuleDropGunGame* pRule = (ZRuleDropGunGame*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
				pRule->OnDrawDropGunGame(pDC);
			}
			// Å©·Î½º?EE
			if(ZGetGameInterface()->GetCamera()->GetLookMode()==ZCAMERA_DEFAULT)
				m_CrossHair.Draw(pDC);

			// Custom: FPS mod
			if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FPS))
			{
				if( !g_bShownSprintMsg && (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_COUNTDOWN || ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY) )
				{
					g_bShownSprintMsg = true;
					g_dwSprintMsgShowTime = GetTickCount();
				}

				if( g_bShownSprintMsg && GetTickCount() - g_dwSprintMsgShowTime < 7000 )
				{
					char szMsg[ 128];
					sprintf( szMsg, "< [FPS] Hold SHIFT to sprint! >" );

					pDC->SetFont( MFontManager::Get("FONTa10_O2Wht"));
					pDC->SetColor(MCOLOR(0xFFFFFFFF));

					TextRelative(pDC,400.f/800.f,400.f/600.f, szMsg, true);
				}
			}

			// Custom: Show RTD Roll.
			if( ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed( MMOD_ROLLTHEDICE ))
			{
				if( g_bShowRTDMsg && GetTickCount() - g_dwRTDMsgShowTime < 5000 )
				{
					pDC->SetFont( MFontManager::Get("FONTa10_O2Wht"));
					pDC->SetColor(MCOLOR(ZCOLOR_CHAT_BROADCAST));
					TextRelative(pDC,400.f/800.f,300.f/600.f, g_szRTDMsg, true);
				}
			}

			if( ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF 
				&& ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY )
			{
				ZBmNumLabel* pBmNumLabel = NULL;
				MPOINT point;
				
				ZRuleTeamCTF* pRule = (ZRuleTeamCTF*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
				bool bRedFlagTaken = pRule->GetBlueCarrier().IsValid(),
					 bBlueFlagTaken = pRule->GetRedCarrier().IsValid();

				bool bMyFlagTaken = ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED ? bRedFlagTaken : bBlueFlagTaken;
				bool bOtherFlagTaken = ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED ? bBlueFlagTaken : bRedFlagTaken;

				pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Max");
				if (pBmNumLabel)
				{
					point = pBmNumLabel->GetPosition();

					if ( RGetIsWidthScreen() || RGetIsLongScreen() )
					{
						point.x += 28;
						point.y += 50;
					}
					else
					{
						point.x += 14;
						point.y += 50;
					}

					pDC->SetFont( MFontManager::Get("FONTa10_O2Wht") );
					pDC->SetColor(MCOLOR(0xFFE8A317));

					if (bMyFlagTaken)
					{
						// high risk alert colour
						pDC->SetColor(MCOLOR(0xFFFF0000));
						pDC->Text(point, "Team Flag");
					}
					else if (bOtherFlagTaken)
					{
						if (ZGetGame()->m_pMyCharacter->IsTagger())
						{
							pDC->SetColor(MCOLOR(0xFF00FF00));
							pDC->Text(point, "Your base!");
						}
						else
						{
							pDC->SetColor(MCOLOR(0xFF00FF00));
							pDC->Text(point, "Defend!");
						}
					}
					else
					{
						point.x -= 6;
						pDC->Text(point, "Enemy Flag");
					}

					if( g_bShowCTFMsg && GetTickCount() - g_dwCTFMsgShowTime < 5000 )
					{
						point = pBmNumLabel->GetPosition();

						if ( RGetIsWidthScreen() || RGetIsLongScreen() )
						{
							point.x -= 90;
							point.y += 145;
						}
						else
						{
							point.x -= 100;
							point.y += 145;
						}

						pDC->SetColor(MCOLOR(0xFF00FFDC));
						pDC->Text(point, g_szCTFMsg);
					}
				}

				if (bRedFlagTaken)
				{
					pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Red");
					if ( pBmNumLabel)
					{
						pDC->SetFont( MFontManager::Get("FONTa10_O2Wht") );
						pDC->SetColor(MCOLOR(0xFFFFFF00));
						point = pBmNumLabel->GetPosition();

						if ( RGetIsWidthScreen() || RGetIsLongScreen() )
						{
							point.x += 10;
							point.y += 50;
						}
						else
						{
							point.x -= 25;
							point.y += 50;
						}
						pDC->Text(point, "<FLAG TAKEN>");
					}
				}

				if (bBlueFlagTaken)
				{
					pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Blue");
					if ( pBmNumLabel)
					{
						pDC->SetFont( MFontManager::Get("FONTa10_O2Wht") );
						pDC->SetColor(MCOLOR(0xFFFFFF00));
						point = pBmNumLabel->GetPosition();

						if ( RGetIsWidthScreen() || RGetIsLongScreen() )
						{
							point.x -= 10;
							point.y += 50;
						}
						else
						{
							point.x += 5;
							point.y += 50;
						}

						pDC->Text(point, "<FLAG TAKEN>");
					}
				}
			}

			if( ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED )
			{
				ZBmNumLabel* pBmNumLabel = NULL;
				MPOINT point;

				pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "TDM_Score_Max");
				if (pBmNumLabel)
				{
					point = pBmNumLabel->GetPosition();

					if ( RGetIsWidthScreen() || RGetIsLongScreen() )
					{
						point.x += 28;
						point.y += 50;
					}
					else
					{
						point.x += 14;
						point.y += 50;
					}

					if( g_bShowCTFMsg && GetTickCount() - g_dwCTFMsgShowTime < 5000 )
					{
						point = pBmNumLabel->GetPosition();

						if ( RGetIsWidthScreen() || RGetIsLongScreen() )
						{
							point.x -= 90;
							point.y += 145;
						}
						else
						{
							point.x -= 100;
							point.y += 145;
						}

						pDC->SetFont( MFontManager::Get("FONTa10_O2Wht"));
						pDC->SetColor(MCOLOR(0xFF00FFDC));
						pDC->Text(point, g_szCTFMsg);
					}
				}
			}
		}

		DrawBuffStatus(pDC);

		// ½ºÄÚ?E
		DrawScore(pDC);

#ifdef _DUELTOURNAMENT
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->OnDraw(pDC);
		}
#endif
	}

	//±×¸®´Â ¼ø¼­ ¶§¹®¿¡
	if(ZGetGame()) {
		ZGetGame()->m_HelpScreen.DrawHelpScreen();
	}

	// µ¿¿µ?EÄ¸ÃÄ...2008.10.20
	if (ZGetGameInterface()->GetBandiCapturer() != NULL)
		ZGetGameInterface()->GetBandiCapturer()->DrawCapture(pDC);

	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		DrawChallengeQuest(pDC);

	if (ZGetGame() && ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
		OnSpyDraw(pDC);

}
void ZCombatInterface::DrawSpyName(MDrawContext* pDC)
{
	ZCharacter* pTargetCharacter = GetTargetCharacter();
	if (!pTargetCharacter) return;

	bool bCountDown = ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_COUNTDOWN;
	bool bOpen = ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY && m_bSpyLocationOpened;

	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
		itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		rvector pos, screen_pos;
		ZCharacter* pCharacter = (*itor).second;
		if (!pCharacter->IsVisible()) continue;
		if (pCharacter->IsDie()) continue;
		if (pCharacter == pTargetCharacter) continue;

		if ((!bCountDown) && (!bOpen || pTargetCharacter->GetTeamID() == MMT_RED))
			if (pCharacter->GetTeamID() != pTargetCharacter->GetTeamID())
				continue;

		pos = pCharacter->GetPosition();
		RVisualMesh* pVMesh = pCharacter->m_pVMesh;
		RealSpace2::rboundingbox box;

		if (pVMesh == NULL) continue;

		box.vmax = pos + rvector(50.f, 50.f, 190.f);
		box.vmin = pos + rvector(-50.f, -50.f, 0.f);

		if (isInViewFrustum(&box, RGetViewFrustum()))
		{
			// ¹Ì´Ï¸ÊÀÌ¸Ez °ªÀ» 0¿¡ ¸ÂÃá´Ù
			if (ZGetCamera()->GetLookMode() == ZCAMERA_MINIMAP) {
				rvector pos = pCharacter->GetPosition();	//mmemory proxy
				pos.z = 0;
				screen_pos = RGetTransformCoord(pos);
			}
			else
				screen_pos = RGetTransformCoord(pCharacter->GetVisualMesh()->GetHeadPosition() + rvector(0, 0, 30.f));

			MFont* pFont = NULL;

			if (pCharacter->IsAdminName()) {
				pFont = MFontManager::Get("FONTa12_O1Org");
				pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
			}
			else {
				if (bCountDown)
					pFont = MFontManager::Get("FONTa12_O1Blr");
				else if (pTargetCharacter->GetTeamID() != pCharacter->GetTeamID())
					pFont = MFontManager::Get("FONTa12_O1Red");
				else
					pFont = MFontManager::Get("FONTa12_O1Blr");

				pDC->SetColor(MCOLOR(0xFF00FF00));
			}

			pDC->SetBitmap(NULL);

			/////// Outline Font //////////
//				MFont *pFont=MFontManager::Get("FONTa12_O1Blr");
			if (pFont == NULL) _ASSERT(0);
			pDC->SetFont(pFont);
			///////////////////////////////

			int x = screen_pos.x - pDC->GetFont()->GetWidth(pCharacter->GetUserName()) / 2;

			pDC->Text(x, screen_pos.y - 12, pCharacter->GetUserName());

			if (bOpen && pTargetCharacter->GetTeamID() == MMT_BLUE && pCharacter->GetTeamID() == MMT_RED)
			{
				const int nIconWidth = 48, nIconHeight = 48;

				pDC->SetBitmap((MBitmap*)m_pSpyIcon);
				pDC->Draw(screen_pos.x - nIconWidth / 2, screen_pos.y - nIconHeight / 2 - 34, nIconWidth, nIconHeight);
			}
		}
	}
}
void ZCombatInterface::DrawMyNamePont(MDrawContext* pDC)
{
	if(ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
		return;

	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL) return;
	if (m_Observer.IsVisible())
		return;

	MFont *pFont=GetGameFontHPAP(); // Custom: Font HPAP Bars

	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));

	char buffer[256];
	sprintf(buffer, "%d  %s", pCharacter->GetProperty()->nLevel, pCharacter->GetProperty()->GetName());
	if ( (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL) || ( !pCharacter->IsObserverTarget()))
	{
		float fCenterVert = 0.018f - (float)pFont->GetHeight()/(float)RGetScreenHeight()/2;
		TextRelative(pDC,100.f/800.f,fCenterVert,buffer);
	}
}
void ZCombatInterface::DrawMyWeaponPont(MDrawContext* pDC)
{
	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL) return;
	if (m_Observer.IsVisible()) return;

	MFont *pFont=GetGameFont();

	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));

	char buffer[256];

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GUNGAME)
	{
		ZRuleGunGame* pRule = (ZRuleGunGame*)ZGetGame()->GetMatch()->GetRule();

		if (pRule)
		{
			if (pCharacter->m_nGunGameWeaponLevel == pRule->m_nWeaponMaxLevel)
			{
				pDC->SetFont(pFont);
				pDC->SetColor(MCOLOR(0xFFFF0000));
			}

			sprintf_s(buffer, "[Level %d / %d]", pCharacter->m_nGunGameWeaponLevel, pRule->m_nWeaponMaxLevel);
			TextRelative(pDC,660.f/800.f,480.f/600.f,buffer);
		}
	}
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DROPMAGIC)
	{
		sprintf(buffer, "[Kills %d / %d]", pCharacter->GetKils(), ZGetGame()->GetMatch()->GetRoundCount());
		TextRelative(pDC, 660.f / 800.f, 490.f / 600.f, buffer);
	}
	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));

	// ¹«?EÀÌ¸§
	TextRelative(pDC,660.f/800.f,510.f/600.f,m_szItemName);

	// Åº¾Ë?E
	MMatchCharItemParts nParts = pCharacter->GetItems()->GetSelectedWeaponParts();
	if (nParts != MMCIP_MELEE && nParts < MMCIP_END) 
	{
		// meleeÀÏ¶§´Â Åº¾Ë?EÇ¥½Ã¸¦ ÇÏ?E¾Ê´Â´Ù.
		sprintf(buffer,"%d / %d", m_nBulletCurrMagazine, m_nBulletSpare);
		TextRelative(pDC, 720.f/800.f, 585.f/600.f, buffer);
	}
}
void ZCombatInterface::DrawMyHPAPPont(MDrawContext* pDC)
{
	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL) return;
	if (m_Observer.IsVisible())
		return;

	MFont* pFont = GetGameFontHPAP(); // Custom: HPAP Normal

	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));

	char hp_buffer[256];
	char ap_buffer[256];
	//sprintf(hp_buffer, "	%d (%d)", (int)pCharacter->GetHP(), (int)pCharacter->GetMaxHP());
	//sprintf(ap_buffer, "	%d (%d)", (int)pCharacter->GetAP(), (int)pCharacter->GetMaxAP());
	sprintf(hp_buffer, "HP %d", (int)pCharacter->GetHP(), (int)pCharacter->GetMaxHP());
	sprintf(ap_buffer, "AP %d", (int)pCharacter->GetAP(), (int)pCharacter->GetMaxAP());

	if ((ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL) || (!pCharacter->IsObserverTarget()))
	{
		TextRelative(pDC, 245.f / 800.f, 25.f / 600.f, hp_buffer);
		TextRelative(pDC, 245.f / 800.f, 47.f / 600.f, ap_buffer);
	}
}
void ZCombatInterface::DrawPont(MDrawContext* pDC)
{
	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL) return;

	if (m_Observer.IsVisible())
		return;

	// µà?E¸ðµåÀÏ¶§(¿ÉÁ®?E¸ð?E¾Æ´Ô)
	if ( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		char	charName[ 3][ 32];
		charName[0][0] = charName[1][0] = charName[2][0] = 0;
		float fRx = (float)MGetWorkspaceWidth()  / 800.0f;
		float fRy = (float)MGetWorkspaceHeight() / 600.0f;

		MFont *pFont = MFontManager::Get( "FONTa10_O2Wht");
		if ( pFont == NULL)
			_ASSERT(0);
		pDC->SetFont( pFont);
		pDC->SetColor( MCOLOR(0xFFFFFFFF));

		bool bIsChallengerDie = false;
		int nMyChar = -1;

		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		if ( pDuel)
		{
			for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
			{
				ZCharacter* pCharacter = (ZCharacter*) (*itor).second;

				// Player
				if ( pCharacter->GetUID() == pDuel->QInfo.m_uidChampion)
				{
					if ( ZGetMyUID() == pDuel->QInfo.m_uidChampion)
					{
						// Draw victory
						ZGetCombatInterface()->DrawVictory( pDC, 210, 86, pDuel->QInfo.m_nVictory);
					}
					else
					{
						sprintf( charName[ 0], "%s%d  %s", ZMsg( MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());

						if ( (ZGetMyUID() == pDuel->QInfo.m_uidChampion) || (ZGetMyUID() == pDuel->QInfo.m_uidChallenger))
						{
							// Draw victory
							int nTextWidth = pFont->GetWidth( charName[ 0]);
							int nWidth = ZGetCombatInterface()->DrawVictory( pDC, 162, 300, pDuel->QInfo.m_nVictory, true);
							ZGetCombatInterface()->DrawVictory( pDC, 43+nTextWidth+nWidth, 157, pDuel->QInfo.m_nVictory);
						}
					}
				}

				else if ( pCharacter->GetUID() == pDuel->QInfo.m_uidChallenger)
				{
					if ( ZGetMyUID() != pDuel->QInfo.m_uidChallenger)
						sprintf( charName[ 0], "%s%d  %s", ZMsg( MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());

					bIsChallengerDie = pCharacter->IsDie();
				}

				// Waiting 1
				else if (pCharacter->GetUID() == pDuel->QInfo.m_WaitQueue[0])
					sprintf( charName[ 1], "%s%d  %s", ZMsg( MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());

				// Waiting 2
				else if (pCharacter->GetUID() == pDuel->QInfo.m_WaitQueue[1])
					sprintf( charName[ 2], "%s%d  %s", ZMsg( MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel, pCharacter->GetUserName());
			}
		}

		MBitmap* pBitmap = MBitmapManager::Get( "duel-mode.tga");
		if ( pBitmap)
		{
			pDC->SetBitmap( pBitmap);

			int nIcon = 50.0f*fRx;
			pDC->Draw( 8.0f*fRx, 153.0f*fRy, nIcon, nIcon);
		}

		pBitmap = MBitmapManager::Get( "icon_play.tga");
		if ( pBitmap && ( charName[1][0] != 0))
		{
			pDC->SetBitmap( pBitmap);

			int nIcon = 22.0f*fRx;
			pDC->Draw( 60.0f*fRx, 175.0f*fRy, nIcon, nIcon);
			pDC->Draw( 53.0f*fRx, 175.0f*fRy, nIcon, nIcon);
		}

		//MCOLOR color;

		int nTime = timeGetTime() % 200;
		if ( nTime < 100)
			pDC->SetColor( MCOLOR( 0xFFFFFF00));
		else
			pDC->SetColor( MCOLOR( 0xFFA0A0A0));

		if ( bIsChallengerDie)
			pDC->SetColor( MCOLOR( 0xFF808080));

		int nPosY = 160.0f*fRy;
		pDC->Text( 60.0f*fRx, nPosY, charName[ 0]);

		pDC->SetColor( MCOLOR(0xFF808080));
		nPosY += 20;
		pDC->Text( 80.0f*fRx, nPosY, charName[ 1]);
		nPosY += 15;
		//pDC->Text( 80.0f*fRx, nPosY, charName[ 2]);
	}
}

void ZCombatInterface::DrawScore(MDrawContext* pDC)
{
	m_bDrawScoreBoard = false;
	if( ZIsActionKeyPressed(ZACTION_SCORE) == true ) {
		if (m_Chat.IsShow() == false)
			m_bDrawScoreBoard = true;
	}
	else if( ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PREPARE ) {
		int cur_round = ZGetGame()->GetMatch()->GetCurrRound();
		//		int max_round = ZGetGame()->GetMatch()->GetRoundCount();

		if(cur_round == 0) {
			m_bDrawScoreBoard = true;
		}
	}

	// Custom: CTF
	if ( ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()) )
	{
		DrawTDMScore(pDC);
	}

	if ( m_bDrawScoreBoard ) {
		if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
			DrawScoreBoard(pDC);
		else
			DrawDuelTournamentScoreBoard(pDC);
	}
}

void ZCombatInterface::DrawBuffStatus(MDrawContext* pDC)
{
	//¹öÇÁÁ¤º¸ÀÓ½ÃÁÖ¼® 
/*	if(ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		ZCharacter* pCharacter = GetTargetCharacter();
		if (pCharacter == NULL) return;
		if (m_Observer.IsVisible()) return;

		ZCharacterBuff *pCharBuff = pCharacter->GetCharacterBuff();
		if( pCharBuff == NULL ) return;

		MFont *pFont = GetGameFont();

		pDC->SetFont(pFont);		
		pDC->SetColor(MCOLOR(0xFFFFFFFF));

		int nMinutes, nSeconds;
		char szMsg[128] = { 0, };
		for(int i = 0; i < MAX_CHARACTER_SHORT_BUFF_COUNT; i++){
			ZShortBuff* pShortBuff = pCharBuff->GetShortBuff(i);
			nMinutes = pShortBuff->GetBuffPeriodRemainder(timeGetTime()) / 1000 / 60;
			nSeconds = pShortBuff->GetBuffPeriodRemainder(timeGetTime()) / 1000 - (60 * nMinutes);
			sprintf(szMsg, "%d:%d", nMinutes, nSeconds);

			if( nMinutes != 0 || nSeconds != 0 ) {
				TextRelative(pDC, (100.f + (i * 50)) / 800.f, 90.f / 600.f, szMsg);
			}			
		}
	}
	else 
	{
	}
*/
}
void ZCombatInterface::DrawFinish()
{
//	DrawResultBoard(pDC);
	// Finish ÈÄ¿¡ ÀÏÁ¤ ½Ã°£ÀÌ °æ°úÇÏ?E°á?EÈ­?Eº¸¿©ÁÜ
	if ( !m_bShowResult && IsFinish())
	{
		// ¹è?EÀ½¾Ç º¼·ýÀ» ¼­¼­?E³·?E
		float fVolume;
		DWORD dwClock = timeGetTime() - m_nReserveFinishTime;
		if ( dwClock > 4000)
			fVolume = 0.0f;
		else
			fVolume = (float)(4000 - dwClock) / 4000.0f * m_fOrgMusicVolume;

		ZApplication::GetSoundEngine()->SetMusicVolume( fVolume);


		if ( timeGetTime() >= m_nReservedOutTime)
		{
			MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Option");
			if ( pWidget)
				pWidget->Show( false);
			pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatMenuFrame");
			if ( pWidget)
				pWidget->Show( false);
			MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatChatInput");
			if ( pLabel)
				pLabel->SetText("");
			ZGetCombatInterface()->EnableInputChat( false);



			// ¸¸?EÄù½ºÆ® ½ÇÆÐ?E½ºÅ×ÀÌÁö·Î ¹Ù·Î ³Ñ¾ûÌ£´Ù.
			if ( ZGetGameTypeManager()->IsQuestOnly( ZGetGame()->GetMatch()->GetMatchType()))
			{
				if ( !ZGetQuest()->IsQuestComplete())
				{
					ZChangeGameState( GUNZ_STAGE);
					m_bShowResult = true;

					return;
				}
			}

			// ¹è?EÈ­¸é¿¡ ÇÊ¿äÇÑ Á¤º¸¸¦ ¾÷µ¥ÀÌÆ® ÇÑ´Ù
			GetResultInfo();

			pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget( "GameResult");
			if ( pWidget)
				pWidget->Show( true, true);


			// »ç¿ûÑå¸¦ Ãâ·ÂÇÑ´Ù
			ZApplication::GetSoundEngine()->SetMusicVolume( m_fOrgMusicVolume);
#ifdef _BIRDSOUND
			ZApplication::GetSoundEngine()->OpenMusic(BGMID_FIN);
			ZApplication::GetSoundEngine()->PlayMusic(false);
#else
			ZApplication::GetSoundEngine()->OpenMusic(BGMID_FIN, ZApplication::GetFileSystem());
			ZApplication::GetSoundEngine()->PlayMusic(false);
#endif
			m_nReservedOutTime = timeGetTime() + 15000;
			m_bShowResult = true;

#ifdef LOCALE_NHNUSA
			// Custom: Disable NHN related
			//GetNHNUSAReport().ReportCompleteGameResult();
#endif

		}
	}
}


int ZCombatInterface::DrawVictory( MDrawContext* pDC, int x, int y, int nWinCount, bool bGetWidth)
{
//	nWinCount = 99;										// for test

	// Get total width
	if ( bGetWidth)
	{
		int nWidth = 0;

		int nNum = nWinCount % 5;
		if ( nNum)
			nWidth += 17.0f + 17.0f * 0.63f * (nNum - 1);

		if ( (nWinCount % 10) >= 5)
			nWidth += 19.0f * 0.2f + 19.0f * 1.1f;
		else
			nWidth += 19.0f * 0.5f;

		nNum = nWinCount / 10;
		if ( nNum)
			nWidth += 22.0f + 22.0f * 0.5f * (nNum - 1);

		return nWidth;
	}


    // Get image
	MBitmap* pBitmap = MBitmapManager::Get( "killstone.tga");
	if ( !pBitmap)
		return 0;

	pDC->SetBitmap( pBitmap);

	// Get screen
	float fRx = (float)MGetWorkspaceWidth()  / 800.0f;
	float fRy = (float)MGetWorkspaceHeight() / 600.0f;


	// Get Image Number
	int nImage = ( (timeGetTime() / 100) % 20);
	if ( nImage > 10)
		nImage = 0;
	nImage *= 32;
	nImage = ( (timeGetTime() / 100) % 20);
	if ( nImage > 10)
		nImage = 0;
	nImage *= 32;

	// Draw
	int nPosX = x * fRx;
	int nPosY = y * fRy;
	int nSize = 17.0f * fRx;								// 1 ´ÜÀ§
	for ( int i = 0;  i < (nWinCount % 5);  i++)
	{
		pDC->Draw( nPosX, nPosY, nSize, nSize, nImage, 0, 32, 32);
		nPosX -= nSize * 0.63f;
	}

	nSize = 19.0f * fRx;
	nPosY = ( y - 2) * fRy;
	if ( (nWinCount % 10) >= 5)								// 5 ´ÜÀ§
	{
		nPosX -= nSize * 0.2f;
		pDC->Draw( nPosX, nPosY, nSize, nSize, nImage, 64, 32, 32);
		nPosX -= nSize * 1.1f;
	}
	else
		nPosX -= nSize * 0.5f;

	nSize = 22.0f * fRx;									// 10 ´ÜÀ§
	nPosY = ( y - 5) * fRy;
	for ( int i = 0;  i < (nWinCount / 10);  i++)
	{
		pDC->Draw( nPosX, nPosY, nSize, nSize, nImage, 32, 32, 32);
		nPosX -= nSize * 0.5f;
	}

	// ¿¬½Â ¼ýÀÚ Ç¥½Ã
/*	if ( nWinCount >= 10)
	{
		pFont = MFontManager::Get( "FONTa9b");
		pDC->SetFont( pFont);
		pDC->SetColor( MCOLOR(0xFFFFFFFF));
		char szVictory[ 16];
		sprintf( szVictory, "%d", nWinCount);
		TextRelative( pDC, 0.195f, 0.01f, szVictory, true);
	}
*/
	return 0;
}


// TODO : ÀÌ°Ô ÇÊ?E¾ø´Â?E
// ±×¸®´Â ¼ø¼­¶§¹®¿¡ ¸¸?EÆã¼Ç
void ZCombatInterface::OnDrawCustom(MDrawContext* pDC)
{
	// °á?EÈ­?Eº¸ÀÎ ÀÌÈÄ¿¡ ÀÏÁ¤ ½Ã°£ ÈÄ ÀÚµ¿ Á¾·áÇÑ´Ù
	if ( m_bShowResult)
	{
		// ¼ýÀÚ¸¦ Ä«¿ûáÍÇÑ´Ù.
		if ( ZGetGameTypeManager()->IsQuestOnly( ZGetGame()->GetMatch()->GetMatchType()))
		{
			int nNumCount = ( timeGetTime() - (m_nReservedOutTime - 15000)) * 3.6418424f;		// 3.6418424f´Â gain °ªÀÌ´Ù.
			ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetPlusXP");
			if ( pBmNumLabel)
			{
				if ( nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber( nNumCount, false);
				else
					pBmNumLabel->SetNumber( ZGetQuest()->GetRewardXP(), false);
			}
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetMinusXP");
			if ( pBmNumLabel)
				pBmNumLabel->SetNumber( 0, false);
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetTotalXP");
			if ( pBmNumLabel)
			{
				if ( nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber( nNumCount, false);
				else
					pBmNumLabel->SetNumber( ZGetQuest()->GetRewardXP(), false);
			}
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetBounty");
			if ( pBmNumLabel)
			{
				if ( nNumCount < ZGetQuest()->GetRewardBP())
					pBmNumLabel->SetNumber( nNumCount, false);
				else
					pBmNumLabel->SetNumber( ZGetQuest()->GetRewardBP(), false);
			}
		}
		else if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
		{

				ZRuleQuestChallenge* pCQRule = NULL;
			if (ZGetGame()->GetMatch()->GetRule())
				pCQRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
			char szText[256];
			if (pCQRule)
			{
				ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_XP");
				if (pBmNumLabel)
				{
					sprintf(szText, "%d", pCQRule->GetRewardXP());
					pBmNumLabel->SetText(szText);
					pBmNumLabel->SetAlignmentMode(MAM_RIGHT);
				}

				pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_BP");
				if (pBmNumLabel)
				{
					sprintf(szText, "%d", pCQRule->GetRewardBP());
					pBmNumLabel->SetText(szText);
					pBmNumLabel->SetAlignmentMode(MAM_RIGHT);

				}

				pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_ElapsedTime");
				if (pBmNumLabel)
				{
					sprintf(szText, "%d,%d", pCQRule->GetCurrTime() / 60, pCQRule->GetCurrTime() % 60);
					pBmNumLabel->SetText(szText);

				}
				pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_Cleard");
				if (pBmNumLabel)
				{
					if (pCQRule->GetNPC() > 0)
					{
						sprintf(szText, "%d/%d", pCQRule->GetCurrSector(), pCQRule->GetRoundMax());
						pBmNumLabel->SetText(szText);
					}
					else
					{
						sprintf(szText, "%d/%d", pCQRule->GetCurrSector() + 1, pCQRule->GetRoundMax());
						pBmNumLabel->SetText(szText);
					}
				}

				MLabel* pLabel = NULL;

			int i = 0;
			for (i = 0; i < 12; i++)
			{
				if (i > 13)
					break;
				char szProperText[260];
				sprintf(szProperText, "CQ_Result_RewardText%d", i);
				pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szProperText);
				if (pLabel)
				{
						std::string resString = "";
						pLabel->SetText("");
				}
			}


			std::vector<int>::iterator it;
			i = 0;
			for (it = pCQRule->m_OurCQRewards.begin(); it != pCQRule->m_OurCQRewards.end(); it++)
			{
				i++;
				if (i > 13)
					break;

				char szProperText[260];
				sprintf(szProperText, "CQ_Result_RewardText%d", i);
				pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szProperText);
				if (pLabel)
				{
					MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc((*it));
					if (pDesc)
					{
						std::string resString = "";
						resString.append(pDesc->m_pMItemName->Ref().m_szItemName);
						pLabel->SetText(resString.c_str());
					}
				}
			}

				MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_RewardImage");
				if (pPicture)
				{
					pPicture->SetBitmap(NULL);
					pPicture->Show(false);
				}
			}

		}
		else if ( ZGetGameTypeManager()->IsSurvivalOnly( ZGetGame()->GetMatch()->GetMatchType()))
		{
			int nNumCount = ( timeGetTime() - (m_nReservedOutTime - 15000)) * 3.6418424f;		// 3.6418424f´Â gain °ªÀÌ´Ù.

			ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetReachedRound");
			if ( pBmNumLabel)
				pBmNumLabel->SetNumber( static_cast< ZSurvival* >(ZGetQuest())->GetReachedRound(), false);
			
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetPoint");
			if ( pBmNumLabel)
			{
				if ( nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber( nNumCount, false);
				else
					pBmNumLabel->SetNumber( static_cast< ZSurvival* >(ZGetQuest())->GetPoint(), false);
			}
			
			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetXP");
			if ( pBmNumLabel)
			{
				if ( nNumCount < ZGetQuest()->GetRewardXP())
					pBmNumLabel->SetNumber( nNumCount, false);
				else
					pBmNumLabel->SetNumber( ZGetQuest()->GetRewardXP(), false);
			}

			pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetBounty");
			if ( pBmNumLabel)
			{
				if ( nNumCount < ZGetQuest()->GetRewardBP())
					pBmNumLabel->SetNumber( nNumCount, false);
				else
					pBmNumLabel->SetNumber( ZGetQuest()->GetRewardBP(), false);
			}
		}

		// ³²Àº ½Ã°£À» Ç¥½ÃÇÑ´Ù
		MLabel* pLabel = (MLabel*)ZGetGameInterface()->GetIDLResource()->FindWidget( "GameResult_RemaindTime");
		if ( pLabel)
		{
			char szRemaindTime[ 100];
			sprintf( szRemaindTime, "%d", ( m_nReservedOutTime - timeGetTime()) / 1000);
			char szText[ 100];
			ZTransMsg( szText, MSG_GAME_EXIT_N_MIN_AFTER, 1, szRemaindTime);

			pLabel->SetAlignment( MAM_HCENTER);
			pLabel->SetText( szText);
		}

		return ;
	}


	if (m_Observer.IsVisible())
	{
		if( !ZGetGameInterface()->IsMiniMapEnable())
		{
			if ( !ZGetGame()->IsReplay() || ZGetGame()->IsShowReplayInfo())
				ZGetScreenEffectManager()->DrawSpectator();
		}

#ifdef _KILLCAM
		m_Observer.OnDraw(pDC); // Custom: Reference KillCam
#endif // !_KILLCAM

		// Custom: Snipers
		if( (!ZGetGame()->IsReplay() && GetTargetCharacter() != ZGetGame()->m_pMyCharacter) || ZGetGame()->IsReplay() )
		{
			if( GetTargetCharacter()->m_dwStatusBitPackingValue.Ref().m_bSniping )
			{
				if( m_pWeaponScreenEffect && !m_pWeaponScreenEffect->IsScopeEnabled() )
					m_pWeaponScreenEffect->OnGadget( MWT_SNIFER );
			}
			else
			{
				if( m_pWeaponScreenEffect && m_pWeaponScreenEffect->IsScopeEnabled() )
					m_pWeaponScreenEffect->OnGadgetOff();
			}
		}

		if (m_pWeaponScreenEffect)
			m_pWeaponScreenEffect->Draw(pDC);
	}

	if(m_bDrawLeaveBattle)
		DrawLeaveBattleTimeMessage(pDC);
}

//void ZCombatInterface::DrawSpawnEffect(MDrawContext* pDC)
//{
//	if (ZGetGame()->m_pMyCharacter->IsAdminHide()) return;
//
//	ZMyCharacter* pMyChar = NULL;
//	pMyChar = ZGetGame()->m_pMyCharacter;
//
//	ZMatch* pMatch = ZGetGame()->GetMatch();
//	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL) {
//		if (pMatch->GetRoundState() == MMATCH_ROUNDSTATE_PLAY) {
//			int nCountTime = pMatch->GetRemainedSpawnTime();
//			if ((nCountTime > 0) && (nCountTime <= 3))
//			{
//				mlog("\n\n\n TEMPORIZADOR %d \n\n\n", nCountTime);
//				pMyChar->m_pVMesh->SetVisibility(0.4f);
//			}else {
//				mlog("\n\n\n FIM TEMPORIZADOR %d \n\n\n", nCountTime);
//				pMyChar->m_pVMesh->SetVisibility(1.0f);
//			}
//		}
//	}
//	
//}

void ZCombatInterface::DrawSoloSpawnTimeMessage(MDrawContext* pDC)
{
	if(ZGetGame()->m_pMyCharacter->IsAdminHide()) return;

	ZMatch* pMatch = ZGetGame()->GetMatch();
	if (pMatch->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		if (!pMatch->IsWaitForRoundEnd())
		{
			if (ZGetGame()->m_pMyCharacter && ZGetGame()->m_pMyCharacter->IsDie())
			{
				char szMsg[128] = "";
				int nRemainTime = pMatch->GetRemainedSpawnTime();
				if ((nRemainTime > 0) && (nRemainTime <= 5))
				{
					char temp[4];
					sprintf(temp, "%d", nRemainTime);
					ZTransMsg(szMsg, MSG_GAME_WAIT_N_MIN, 1, temp);
				}
				else if ((nRemainTime == 0) && (!ZGetGame()->GetSpawnRequested()))
				{
						sprintf(szMsg, ZMsg(MSG_GAME_CLICK_FIRE));
				}

				MFont* pFont = GetGameFont();
				pDC->SetFont(pFont);
				pDC->SetColor(MCOLOR(0xFFFFFFFF));
				TextRelative(pDC, 400.f / 800.f, 400.f / 600.f, szMsg, true);
			}
		}
	}
}

void ZCombatInterface::DrawLeaveBattleTimeMessage(MDrawContext* pDC)
{
	char szMsg[128] = "";
//	sprintf(szMsg, "%d ÃÊÈÄ¿¡ °ÔÀÓ¿¡¼­ ³ª°©´Ï´Ù", m_nDrawLeaveBattleSeconds);

	char temp[ 4 ];
	sprintf( temp, "%d", m_nDrawLeaveBattleSeconds );
	ZTransMsg( szMsg, MSG_GAME_EXIT_N_MIN_AFTER, 1, temp );

	MFont *pFont=GetGameFont();
	pDC->SetFont(pFont);
	pDC->SetColor(MCOLOR(0xFFFFFFFF));
	TextRelative(pDC,400.f/800.f,350.f/600.f, szMsg, true);
}


bool ZCombatInterface::IsDone()
{
	return false;
}

bool ZCombatInterface::OnEvent(MEvent* pEvent, MListener* pListener)
{
	return false;
}

void ZCombatInterface::Resize(int w, int h)
{
	SetSize(w, h);

}

void ZCombatInterface::ShowMenu(bool bVisible)
{
	if (m_bMenuVisible == bVisible) return;

	
	m_bMenuVisible = bVisible;
}

void ZCombatInterface::EnableInputChat(bool bInput, bool bTeamChat)
{
	// Ã¤ÆÃÃ¢ ¾Èº¸±âÀÎ »óÅÂÀÏ ¶§¿¡´Â ÀÔ·Âµµ ¾ÈµÈ´Ù.
//	if ((!ZGetConfiguration()->GetViewGameChat()) && (bInput)) return;

	m_Chat.EnableInput(bInput, bTeamChat);
}

void ZCombatInterface::OutputChatMsg(const char* szMsg)
{
	m_Chat.OutputChatMsg(szMsg);
}

void ZCombatInterface::OutputChatMsg(MCOLOR color, const char* szMsg)
{
	m_Chat.OutputChatMsg(color, szMsg);
}


void ZCombatInterface::SetItemName(const char* szName)
{
	if (!strcmp(m_szItemName, szName)) return;

	strcpy(m_szItemName, szName);
}


void ZCombatInterface::ShowInfo(bool bVisible)
{
	MWidget* pWidget;
	char szTemp[256];
	for (int i = 0; i < 9; i++)
	{
		sprintf(szTemp, "%s%d", ZIITEM_COMBAT_INFO, i);
		pWidget = m_pIDLResource->FindWidget(szTemp);
		if (pWidget!=NULL)
		{
			pWidget->Show(bVisible);
		}
	}
	pWidget = m_pIDLResource->FindWidget(ZIITEM_COMBAT_CHATFRAME);
	if (pWidget!=NULL)
	{
		pWidget->Show(bVisible);
	}
}

void ZCombatInterface::Update(float fElapsed)
{
	// Finish ÈÄ¿¡ ÀÏÁ¤ ½Ã°£ÀÌ °æ°úÇÏ?E°á?EÈ­?Eº¸¿©ÁÜ
	DrawFinish();

	// °á?EÈ­?Eº¸´Â ÁßÀÌ?EÀÏÁ¤ ½Ã°£ ÈÄ ÀÚµ¿ Á¾·áÇÑ´Ù
	if ( m_bShowResult)
	{
		if ( timeGetTime() > m_nReservedOutTime)
		{
			//if (ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsDuelTournamentGame() || ZGetGameClient()->IsPWChannel())
			if (ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsDuelTournamentGame() || ZGetGameClient()->IsPWChannel())
				ZChangeGameState(GUNZ_LOBBY);
			else
				ZChangeGameState(GUNZ_STAGE);
		}
	}

	m_fElapsed = fElapsed;

	if (m_bReserveFinish) {
		if ((timeGetTime() - m_nReserveFinishTime) > 1000) {
			OnFinish();
			m_bReserveFinish = false;
		}
	}

	ZCharacter* pCharacter = GetTargetCharacter();
	if (pCharacter == NULL)				return;
	if (!pCharacter->GetInitialized())	return;
	if(ZGetScreenEffectManager()==NULL) return;
	if(pCharacter->GetProperty()==NULL) return;

	float fGauge = 100.f;
	float fCur,fMax;
/*
	bool bPre = false;

	if(g_pGame&&g_pGame->GetMatch()) {
//	if(g_pGame&&g_pGame->GetMatch()->GetRoundState()==MMATCH_ROUNDSTATE_PLAY) {
//	if(g_pGame&&(g_pGame->GetReadyState()==ZGAME_READYSTATE_RUN)) {

	if(bPre) 
*/

	if( ZGetGame() && ZGetGame()->GetMatch() )
	{
#ifdef _SRVRPNG
		unsigned long int nNowTime = timeGetTime();
		if ((nNowTime - ZGetGame()->st_nLastTime[2]) > 4000) // Custom: Time Update Latency Default: 8000
		{
			ZGetGame()->st_nLastTime[2] = nNowTime;
			ZPostServerPing(timeGetTime(), 2);
		}
#endif
		fMax = (float)pCharacter->GetMaxHP();
		fCur = (float)pCharacter->GetHP();		

		if( fCur!=0.f && fMax!=0.f )	fGauge = fCur/fMax;
		else							fGauge = 0.f;

		if( ZGetGame()->GetMatch()->GetCurrRound()==0 && 
			ZGetGame()->GetMatch()->GetRoundState()==MMATCH_ROUNDSTATE_PREPARE)
			fGauge = 100.f;

		ZGetScreenEffectManager()->SetGauge_HP(fGauge);

		fMax = (float)pCharacter->GetMaxAP();
		fCur = (float)pCharacter->GetAP();		

		if( fCur!=0.f && fMax!=0.f )	fGauge = fCur/fMax;
		else							fGauge = 0.f;
		

		ZGetScreenEffectManager()->SetGauge_AP(fGauge);
	} 
	else 
	{
		ZGetScreenEffectManager()->SetGauge_HP(fGauge);
		ZGetScreenEffectManager()->SetGauge_AP(fGauge);
	}

//	ZGetScreenEffectManager()->SetGauge_EXP((float)pCharacter->GetStatus()->fStamina/100.f);//ÀÓ½Ã·Î stamina ·Î Å×½ºÆ®
//	ZGetScreenEffectManager()->SetGauge_EXP(100.f);//ÀÓ½Ã·Î stamina ·Î Å×½ºÆ®

	MMatchWeaponType wtype = MWT_NONE;

	ZItem* pSItem = pCharacter->GetItems()->GetSelectedWeapon();

	MMatchItemDesc* pSelectedItemDesc = NULL; 

	if( pSItem ) {
		pSelectedItemDesc = pSItem->GetDesc();

		m_nBulletSpare = pSItem->GetBulletSpare();
		m_nBulletCurrMagazine = pSItem->GetBulletCurrMagazine();
	}

	if( pSelectedItemDesc ) {
		wtype = pSelectedItemDesc->m_nWeaponType.Ref();
	}

	ZGetScreenEffectManager()->SetWeapon( wtype ,pSelectedItemDesc );


	if ((pSelectedItemDesc) && (m_pLastItemDesc != pSelectedItemDesc)) {
		SetItemName( pSelectedItemDesc->m_pMItemName->Ref().m_szItemName );
	}

	UpdateCombo(pCharacter);

	m_Chat.Update();
	m_AdminMsg.Update();


	/*
	if (m_pScoreBoard->IsVisible())
	{
		m_pScoreBoard->Update();
	}
	*/

	if (pCharacter->GetItems()->GetSelectedWeaponParts() == MMCIP_MELEE )
	{
		ShowCrossHair(false);
	}
	// Custom: Check if selected weapon type is sniper
	else if( pCharacter->GetItems()->GetSelectedWeaponParts() == MMCIP_PRIMARY || pCharacter->GetItems()->GetSelectedWeaponParts() == MMCIP_SECONDARY )
	{
		MMatchItemDesc* pItemDesc = pCharacter->GetSelectItemDesc();

		if( pItemDesc )
			ShowCrossHair(pItemDesc->m_nWeaponType.Ref() == MWT_SNIFER ? false : true);
		else if( !pItemDesc )
			_ASSERT(0);
	}
	else
	{
		ShowCrossHair(true);
	}

	GameCheckPickCharacter();

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
		OnSpyUpdate(fElapsed);

}

bool GetUserInfoUID(MUID uid,MCOLOR& _color,char* sp_name,MMatchUserGradeID& gid);

void ZCombatInterface::SetPickTarget(bool bPick, ZCharacter* pCharacter)
{
	bool bFriend = false;
	if (bPick)
	{
		if (pCharacter == NULL) return;

		if (ZGetGame()->GetMatch()->IsTeamPlay())
		{
			ZCharacter *pTargetCharacter=GetTargetCharacter();
			if(pTargetCharacter && pTargetCharacter->GetTeamID()==pCharacter->GetTeamID())
			{
				bFriend = true;
			}
		}

		if (bFriend == false) 
		{
			m_CrossHair.SetState(ZCS_PICKENEMY);
			m_pTargetLabel->SetTextColor(0xffff0000);
		}
#ifdef _VIPGRADES
		if (pCharacter->IsVIP1Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_VIP1_NAME);
		if (pCharacter->IsVIP2Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_VIP2_NAME);
		if (pCharacter->IsVIP3Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_VIP3_NAME);
		if (pCharacter->IsVIP4Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_VIP4_NAME);
		if (pCharacter->IsVIP5Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_VIP5_NAME);
		if (pCharacter->IsVIP6Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_VIP6_NAME);
		if (pCharacter->IsVIP7Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_VIP7_NAME);
#endif

#ifdef _EVENTGRD
		if (pCharacter->IsEvent1Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_EVENT1_NAME);
		if (pCharacter->IsEvent2Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_EVENT2_NAME);
		if (pCharacter->IsEvent3Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_EVENT3_NAME);
		if (pCharacter->IsEvent4Name())
			m_pTargetLabel->SetTextColor(ZCOLOR_EVENT4_NAME);
#endif

		if(pCharacter->IsAdminName() || pCharacter->GetUserGrade() == MMUG_EVENTTEAM)
		{
			// Custom: Set custom colours
			MCOLOR gmColor;
			char szEmpty[4];
			memset( szEmpty, 0, sizeof( szEmpty ) );

			if (GetUserGradeIDColor(pCharacter->GetUserGrade(), gmColor, szEmpty))
				m_pTargetLabel->SetTextColor(gmColor);
			else
				m_pTargetLabel->SetTextColor(ZCOLOR_ADMIN_NAME);
			//m_pTargetLabel->SetTextColor(ZCOLOR_ADMIN_NAME);
		}

		if (!bFriend == true && !pCharacter->IsDie()) 
		{			
			strcpy(m_szTargetName, pCharacter->GetUserName());
#ifdef _DEBUG
			sprintf(m_szTargetName, "%s : %d", pCharacter->GetUserName(), pCharacter->GetHP());
#endif
			m_pTargetLabel->SetText(m_szTargetName);
		}

		int nCrosshairHeight = m_CrossHair.GetHeight();

		int nLen = m_pTargetLabel->GetRect().w;
		m_pTargetLabel->SetPosition(((MGetWorkspaceWidth()-m_pTargetLabel->GetRect().w)/2) ,(MGetWorkspaceHeight()/2) - nCrosshairHeight );
		m_pTargetLabel->SetAlignment(MAM_HCENTER);
	}
	else
	{
		m_CrossHair.SetState(ZCS_NORMAL);
		memset(m_szTargetName, 0, sizeof(m_szTargetName));
		m_pTargetLabel->Show(false);
	}

	m_bPickTarget = bPick;
}

void ZCombatInterface::SetItemImageIndex(int nIndex)
{
	char szTemp[256];
	sprintf(szTemp, "item%02d.png", nIndex);
	BEGIN_WIDGETLIST("CombatItemPic", ZApplication::GetGameInterface()->GetIDLResource(),
		MPicture*, pPicture);

	pPicture->SetBitmap(MBitmapManager::Get(szTemp));

	END_WIDGETLIST();
}
/*
void ZCombatInterface::SetMagazine(int nMagazine)
{
	if (m_nMagazine == nMagazine) return;

	char szTemp[256];
	sprintf(szTemp, "%02d", nMagazine);
	BEGIN_WIDGETLIST("CombatMagazine", ZApplication::GetGameInterface()->GetIDLResource(),
		MWidget*, pWidget);

	pWidget->SetText(szTemp);

	END_WIDGETLIST();

	m_nMagazine = nMagazine;
}
*/

void ZCombatInterface::UpdateCombo(ZCharacter* pCharacter)
{
	if(pCharacter==NULL) return;

	static int nComboX = -999, nComboY = -999;
	static int nLastCombo = 0;

	int nCurCombo = pCharacter->GetStatus().Ref().nCombo;

	if (nCurCombo != nLastCombo)
	{
		nLastCombo = nCurCombo;
		ZGetScreenEffectManager()->SetCombo(nLastCombo);
	}
	else if (nCurCombo != 0)
	{

	}
}
// Custom: ChallengeQuest On Screen Interface
void ZCombatInterface::DrawChallengeQuest(MDrawContext* pDC)
{

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		ZRuleQuestChallenge* pCQRule = NULL;
		if (ZGetGame()->GetMatch()->GetRule())
			pCQRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
		char szText[256];
		if (pCQRule)
		{
			MWidget* pWidget = (MWidget*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatChallQuestInfo");
			if (pWidget)
			{
				int nMargin[BMNUM_NUMOFCHARSET] = { 64, 64, 64,64, 64, 64, 64, 64,64, 64, 32, 32,32};
				pWidget->Show(true);
				ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_ElapsedTime");
				if (pBmNumLabel)
				{
					pBmNumLabel->Show(true);
					sprintf(szText, "%2d,%02d", pCQRule->GetCurrTime() / 60, pCQRule->GetCurrTime() % 60);
					pBmNumLabel->SetText(szText);
					pBmNumLabel->SetCharMargin(nMargin);
					pBmNumLabel->SetAlignmentMode(MAM_HCENTER);
				}

				pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_RoundProgress");
				if (pBmNumLabel)
				{

					pBmNumLabel->Show(true);
					sprintf(szText, "%d/%d", pCQRule->GetCurrSector() + 1, pCQRule->GetRoundMax());
					pBmNumLabel->SetText(szText);
					pBmNumLabel->SetAlignmentMode(MAM_LEFT);
				}
				MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Round");
				if (pPicture)
				{
					pPicture->Show(true);
				}
				pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_FadeBG");
				if (pPicture)
				{
					pPicture->Show(false);
					if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH)
					{
						if (pCQRule->GetCurrSector() < pCQRule->GetRoundMax())
						{
							pPicture->Show(true);
						}
					}

				}

				MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_ObserverDescription");
				if (pLabel)
				{
					pLabel->Show(false);
					if (GetObserverMode())
					{
						pLabel->Show(false);
						pLabel->SetText(ZMsg(MSG_QUEST_CHALLENGE_REVIVAL));
					}
					else if (!GetObserverMode() && ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH &&
						pCQRule->GetCurrSector()< pCQRule->GetRoundMax())
					{
						pLabel->Show(true);
						pLabel->SetText(ZMsg(MSG_QUEST_CHALLENGE_STAGECHANGE));
					}
				}
			}
		}
	}
}

void ZCombatInterface::DrawFriendName(MDrawContext* pDC)
{
	if (ZGetGame()->m_pMyCharacter == NULL) return;

	// Custom: Hide friend names when scoping
	if (ZGetGame()->m_pMyCharacter->m_statusFlags.Ref().m_bSniferMode)
		return;

	if (ZGetGame()->GetMatch()->IsTeamPlay())
	{
		ZCharacter* pTargetCharacter = GetTargetCharacter();
		if (pTargetCharacter == NULL) return;
		
		float texty = 0.75000f;
		float x = 0.024f;
		float ysub = 0.05000f;

		for(ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
			itor != ZGetGame()->m_CharacterManager.end(); ++itor)
		{
			rvector pos, screen_pos;
			ZCharacter* pCharacter = (ZCharacter*) (*itor).second;
			if (!pCharacter->IsVisible()) continue;
			if (pCharacter->IsDie()) continue;
			if (pCharacter->GetTeamID() != pTargetCharacter->GetTeamID()) continue;
			if (pCharacter == pTargetCharacter) continue;
			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY) continue;
			// Custom: CTF
			if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_CTF && pCharacter->GetTeamID() != pTargetCharacter->GetTeamID())
				continue;
			//if (pCharacter->GetTeamID() != pTargetCharacter->GetTeamID()) continue;
			if (pCharacter==pTargetCharacter) continue;

			if( ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF )
			{
				if (!pCharacter->IsTagger() && pCharacter->GetTeamID() != pTargetCharacter->GetTeamID())
					continue;
			}

			// Custom: bFriend check
			bool bFriend = pCharacter->GetTeamID() == pTargetCharacter->GetTeamID() ? true : false;

			if (Z_ETC_HPAP_SCOREBOARD) // Custom: HPAPBar Left ScoreBoard Add By Bloder
			{
				MCOLOR tmpColor = pDC->GetColor();
				pDC->SetColor(MCOLOR(240, 248, 255));
				pDC->SetFont(NULL);
				pDC->Text(x * MGetWorkspaceWidth(), texty * MGetWorkspaceHeight() - 24, pCharacter->GetUserNameA());

				pDC->SetColor(MCOLOR(0x4000C9FF));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, 0.08 * MGetWorkspaceWidth(), 7);

				float nValue = 0.08 * pCharacter->GetHP() / pCharacter->GetMaxHP();
				pDC->SetColor(MCOLOR(0x9000C9FF));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, nValue * MGetWorkspaceWidth(), 7);

				pDC->SetColor(MCOLOR(0x4000C9FF));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, 0.08 * MGetWorkspaceWidth(), 7);

				nValue = 0.08 * pCharacter->GetAP() / pCharacter->GetMaxAP();
				pDC->SetColor(MCOLOR(0x9000FF59));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, nValue * MGetWorkspaceWidth(), 7);
				pDC->SetColor(tmpColor);
				texty -= ysub;
			}

			pos = pCharacter->GetPosition();
			RVisualMesh* pVMesh = pCharacter->m_pVMesh;
			RealSpace2::rboundingbox box;

			if (pVMesh == NULL) continue;
			
//			box.vmax = pVMesh->m_vBMax + pos;
//			box.vmin = pVMesh->m_vBMin + pos;
			
			box.vmax = pos + rvector(50.f, 50.f, 190.f);
			box.vmin = pos + rvector(-50.f, -50.f, 0.f);

			if (isInViewFrustum(&box, RGetViewFrustum()))
			{
				/*
#define CHARACTER_HEIGHT	185.0f
				pos.z = pos.z + CHARACTER_HEIGHT;
				screen_pos = RGetTransformCoord(pos);
				*/
				screen_pos = RGetTransformCoord(pCharacter->GetVisualMesh()->GetHeadPosition()+rvector(0,0,30.f));

				MFont *pFont=NULL;

				if(pCharacter->IsAdminName() || pCharacter->GetUserGrade() == MMUG_EVENTTEAM) {
					//pFont = bFriend == true ? MFontManager::Get("FONTa12_O1Org") : MFontManager::Get("FONTa12_O1Red");

					pFont = MFontManager::Get("FONTa12_O1Org");
					if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF)
					{
						if (!bFriend && pCharacter->IsTagger())
							pFont = MFontManager::Get("FONTa12_O1Red");
						else if (bFriend && pCharacter->IsTagger())
							pFont = MFontManager::Get("FONTa12_O1Grn");
					}
#ifdef _VIPGRADES
					else if (pCharacter->IsVIP1Name()) {
						pFont = MFontManager::Get("FONTb11b");
						pDC->SetColor(MCOLOR(ZCOLOR_VIP1_NAME));
					}

					else if (pCharacter->IsVIP2Name()) {
						pFont = MFontManager::Get("FONTb11b");
						pDC->SetColor(MCOLOR(ZCOLOR_VIP2_NAME));
					}

					else if (pCharacter->IsVIP3Name()) {
						pFont = MFontManager::Get("FONTb11b");
						pDC->SetColor(MCOLOR(ZCOLOR_VIP3_NAME));
					}
					else if (pCharacter->IsVIP4Name()) {
						pFont = MFontManager::Get("FONTb11b");
						pDC->SetColor(MCOLOR(ZCOLOR_VIP4_NAME));
					}

					else if (pCharacter->IsVIP5Name()) {
						pFont = MFontManager::Get("FONTb11b");
						pDC->SetColor(MCOLOR(ZCOLOR_VIP5_NAME));
					}

					else if (pCharacter->IsVIP6Name()) {
						pFont = MFontManager::Get("FONTb11b");
						pDC->SetColor(MCOLOR(ZCOLOR_VIP6_NAME));
					}
					else if (pCharacter->IsVIP7Name()) {
						pFont = MFontManager::Get("FONTb11b");
						pDC->SetColor(MCOLOR(ZCOLOR_VIP7_NAME));
					}
#endif


#ifdef _EVENTGRD
					else if (pCharacter->IsEvent1Name()) {
						pFont = MFontManager::Get("FONTa10_O2Wht");
						pDC->SetColor(MCOLOR(ZCOLOR_EVENT1_NAME));
					}

					else if (pCharacter->IsEvent2Name()) {
						pFont = MFontManager::Get("FONTa10_O2Wht");
						pDC->SetColor(MCOLOR(ZCOLOR_EVENT2_NAME));
					}

					else if (pCharacter->IsEvent3Name()) {
						pFont = MFontManager::Get("FONTa10_O2Wht");
						pDC->SetColor(MCOLOR(ZCOLOR_EVENT3_NAME));
					}
					else if (pCharacter->IsEvent4Name()) {
						pFont = MFontManager::Get("FONTa10_O2Wht");
						pDC->SetColor(MCOLOR(ZCOLOR_EVENT4_NAME));
					}
#endif
					// Custom: Set custom colours
					MCOLOR gmColor;
					char szEmpty[4];
					memset( szEmpty, 0, sizeof( szEmpty ) );

					if (GetUserGradeIDColor(pCharacter->GetUserGrade(), gmColor, szEmpty))
						pDC->SetColor(gmColor);
					else
						pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
					//pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));

					if (!bFriend)
						pDC->SetColor(MCOLOR(0xFFFF0000));//pDC->SetColor(MCOLOR(0xFF00FF00));
				}
				else {
					// Custom: Added bFriend check
					// Font defaults to green.
					// Custom: Set font to red

					pFont = MFontManager::Get("FONTa12_O1Blr");
					if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF)
					{
						if (!bFriend && pCharacter->IsTagger())
							pFont = MFontManager::Get("FONTa12_O1Red");
						else if (bFriend && pCharacter->IsTagger())
							pFont = MFontManager::Get("FONTa12_O1Grn");
					}

					//pFont = bFriend == true ? MFontManager::Get("FONTa12_O1Blr") : MFontManager::Get("Fonta12_O1Red");
					pDC->SetColor(MCOLOR(0xFFFF0000));//pDC->SetColor(MCOLOR(0xFF00FF00));
				}

				pDC->SetBitmap(NULL);

				/////// Outline Font //////////
//				MFont *pFont=MFontManager::Get("FONTa12_O1Blr");
				if (pFont == NULL) _ASSERT(0);
				pDC->SetFont(pFont);
				///////////////////////////////

				int x = screen_pos.x - pDC->GetFont()->GetWidth(pCharacter->GetUserName()) / 2;

				pDC->Text(x, screen_pos.y - 12, pCharacter->GetUserName());
			}
		}
	}
}

void ZCombatInterface::DrawEnemyName(MDrawContext* pDC)
{
	MPOINT Cp = GetCrosshairPoint();

	ZPICKINFO pickinfo;

	rvector pos,dir;
	if(!RGetScreenLine(Cp.x,Cp.y,&pos,&dir))
		return;
	
	ZCharacter *pTargetCharacter=GetTargetCharacter();

	if(ZGetGame()->Pick(pTargetCharacter,pos,dir,&pickinfo))
	{
		if (pickinfo.pObject) {
			if (!IsPlayerObject(pickinfo.pObject)) return;
			if (pickinfo.pObject->IsDie()) return;

			ZCharacter* pPickedCharacter = (ZCharacter*)pickinfo.pObject;

			bool bFriend = false;
			if (ZGetGame()->GetMatch()->IsTeamPlay()) {
				if (pTargetCharacter && pPickedCharacter->GetTeamID() == pTargetCharacter->GetTeamID())
					bFriend = true;
			}

			if (bFriend == false) {

				/////// Outline Font //////////

				MFont *pFont = NULL;//MFontManager::Get("FONTa12_O1Red");

				if(pPickedCharacter->IsAdminName() || pPickedCharacter->GetUserGrade() == MMUG_EVENTTEAM) {
					// Custom: Set custom colours
					MCOLOR gmColor;
					char szEmpty[4];
					memset( szEmpty, 0, sizeof( szEmpty ) );

					if (GetUserGradeIDColor(pPickedCharacter->GetUserGrade(), gmColor, szEmpty))
						pDC->SetColor(gmColor);
					else
						pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
					//pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
					pFont = MFontManager::Get("FONTa12_O1Org");
				}
#ifdef _VIPGRADES
				else if (pPickedCharacter->IsVIP1Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_VIP1_NAME));
					pFont = MFontManager::Get("FONTb11b");
				}

				else if (pPickedCharacter->IsVIP2Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_VIP2_NAME));
					pFont = MFontManager::Get("FONTb11b");
				}

				else if (pPickedCharacter->IsVIP3Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_VIP3_NAME));
					pFont = MFontManager::Get("FONTb11b");
				}
				else if (pPickedCharacter->IsVIP4Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_VIP4_NAME));
					pFont = MFontManager::Get("FONTb11b");
				}

				else if (pPickedCharacter->IsVIP5Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_VIP5_NAME));
					pFont = MFontManager::Get("FONTb11b");
				}

				else if (pPickedCharacter->IsVIP6Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_VIP6_NAME));
					pFont = MFontManager::Get("FONTb11b");
				}
				else if (pPickedCharacter->IsVIP7Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_VIP7_NAME));
					pFont = MFontManager::Get("FONTb11b");
				}
#endif


#ifdef _EVENTGRD
				else if (pPickedCharacter->IsEvent1Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_EVENT1_NAME));
					pFont = MFontManager::Get("FONTa10_O2Wht");
				}

				else if (pPickedCharacter->IsEvent2Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_EVENT2_NAME));
					pFont = MFontManager::Get("FONTa10_O2Wht");
				}

				else if (pPickedCharacter->IsEvent3Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_EVENT3_NAME));
					pFont = MFontManager::Get("FONTa10_O2Wht");
				}
				else if (pPickedCharacter->IsEvent4Name()) {
					pDC->SetColor(MCOLOR(ZCOLOR_EVENT4_NAME));
					pFont = MFontManager::Get("FONTa10_O2Wht");
				}
#endif

				else 
				{
					pFont = MFontManager::Get("FONTa12_O1Red");
				}
				if (pFont == NULL) _ASSERT(0);
				pDC->SetFont(pFont);

				int x = Cp.x - pDC->GetFont()->GetWidth(pPickedCharacter->GetUserName()) / 2;
				pDC->Text(x, Cp.y - pDC->GetFont()->GetHeight()-10, pPickedCharacter->GetUserName());

				if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY &&
					ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY &&
					pPickedCharacter->GetTeamID() == MMT_RED &&
					!m_bSpyLocationOpened)
				{
						const int nIconWidth = 48, nIconHeight = 48;
						rvector screen_pos = RGetTransformCoord(pPickedCharacter->GetVisualMesh()->GetHeadPosition() + rvector(0, 0, 30.f));

						pDC->SetBitmap((MBitmap*)m_pSpyIcon);
						pDC->Draw(screen_pos.x - nIconWidth / 2, screen_pos.y - nIconHeight / 2 - 34, nIconWidth, nIconHeight);
					}
				}
			}			
		}
	}


void ZCombatInterface::DrawAllPlayerName(MDrawContext* pDC)
{
	for(ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin();
		itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		rvector pos, screen_pos;
		ZCharacter* pCharacter = (ZCharacter*) (*itor).second;
		if (!pCharacter->IsVisible()) continue;
		if (pCharacter->IsDie()) continue;

		pos = pCharacter->GetPosition();
		RVisualMesh* pVMesh = pCharacter->m_pVMesh;
		RealSpace2::rboundingbox box;

		if (pVMesh == NULL) continue;
		
		box.vmax = pos + rvector(50.f, 50.f, 190.f);
		box.vmin = pos + rvector(-50.f, -50.f, 0.f);

		if (isInViewFrustum(&box, RGetViewFrustum()))
		{
			// ¹Ì´Ï¸ÊÀÌ?Ez °ªÀ» 0¿¡ ¸ÂÃá´Ù
			if(ZGetCamera()->GetLookMode()==ZCAMERA_MINIMAP) {
				rvector pos = pCharacter->GetPosition();	//mmemory proxy
				pos.z=0;
				screen_pos = RGetTransformCoord(pos);
			}else
				screen_pos = RGetTransformCoord(pCharacter->GetVisualMesh()->GetHeadPosition()+rvector(0,0,30.f));

			MFont *pFont=NULL;

			if(pCharacter->IsAdminName() || pCharacter->GetUserGrade() == MMUG_EVENTTEAM) {
				pFont = MFontManager::Get("FONTa12_O1Org");

				// Custom: Set custom colours
				MCOLOR gmColor;
				char szEmpty[4];
				memset( szEmpty, 0, sizeof( szEmpty ) );

				if (GetUserGradeIDColor(pCharacter->GetUserGrade(), gmColor, szEmpty))
					pDC->SetColor(gmColor);
				else
					pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
				//pDC->SetColor(MCOLOR(ZCOLOR_ADMIN_NAME));
			}
#ifdef _VIPGRADES
			else if (pCharacter->IsVIP1Name()) {
				pFont = MFontManager::Get("FONTb11b");
				pDC->SetColor(MCOLOR(ZCOLOR_VIP1_NAME));
			}

			else if (pCharacter->IsVIP2Name()) {
				pFont = MFontManager::Get("FONTb11b");
				pDC->SetColor(MCOLOR(ZCOLOR_VIP2_NAME));
			}

			else if (pCharacter->IsVIP3Name()) {
				pFont = MFontManager::Get("FONTb11b");
				pDC->SetColor(MCOLOR(ZCOLOR_VIP3_NAME));
			}
			else if (pCharacter->IsVIP4Name()) {
				pFont = MFontManager::Get("FONTb11b");
				pDC->SetColor(MCOLOR(ZCOLOR_VIP4_NAME));
			}

			else if (pCharacter->IsVIP5Name()) {
				pFont = MFontManager::Get("FONTa12_O1Org");
				pDC->SetColor(MCOLOR(ZCOLOR_VIP5_NAME));
			}

			else if (pCharacter->IsVIP6Name()) {
				pFont = MFontManager::Get("FONTb11b");
				pDC->SetColor(MCOLOR(ZCOLOR_VIP6_NAME));
			}
			else if (pCharacter->IsVIP7Name()) {
				pFont = MFontManager::Get("FONTb11b");
				pDC->SetColor(MCOLOR(ZCOLOR_VIP7_NAME));
			}
#endif

#ifdef _EVENTGRD
			else if (pCharacter->IsEvent1Name()) {
				pFont = MFontManager::Get("FONTa10_O2Wht");
				pDC->SetColor(MCOLOR(ZCOLOR_EVENT1_NAME));
			}

			else if (pCharacter->IsEvent2Name()) {
				pFont = MFontManager::Get("FONTa10_O2Wht");
				pDC->SetColor(MCOLOR(ZCOLOR_EVENT2_NAME));
			}

			else if (pCharacter->IsEvent3Name()) {
				pFont = MFontManager::Get("FONTa10_O2Wht");
				pDC->SetColor(MCOLOR(ZCOLOR_EVENT3_NAME));
			}
			else if (pCharacter->IsEvent4Name()) {
				pFont = MFontManager::Get("FONTa10_O2Wht");
				pDC->SetColor(MCOLOR(ZCOLOR_EVENT4_NAME));
			}
#endif

			else 
			{
				if (pCharacter->GetTeamID() == MMT_RED)
					pFont = MFontManager::Get("FONTa12_O1Red");
				else if (pCharacter->GetTeamID() == MMT_BLUE)
					pFont = MFontManager::Get("FONTa12_O1Blr");
				else
					pFont = MFontManager::Get("FONTa12_O1Blr");

				pDC->SetColor(MCOLOR(0xFF00FF00));
			}

			pDC->SetBitmap(NULL);

			/////// Outline Font //////////
//				MFont *pFont=MFontManager::Get("FONTa12_O1Blr");
			if (pFont == NULL) _ASSERT(0);
			pDC->SetFont(pFont);
			///////////////////////////////

			int x = screen_pos.x - pDC->GetFont()->GetWidth(pCharacter->GetUserName()) / 2;

			pDC->Text(x, screen_pos.y - 12, pCharacter->GetUserName());
		}
	}
}
MFont* ZCombatInterface::GetGameFontHPAP()
{
	MFont* pFont = MFontManager::Get("FONTa10_O2Wht");
	return pFont;
}
MFont *ZCombatInterface::GetGameFont()
{
#ifdef _FONTNEW
	MFont *pFont=MFontManager::Get("FONTa6_O2Wht");
#else
	MFont* pFont = MFontManager::Get("FONTa10_O2Wht");
#endif

	return pFont;
}
// µà¾ó¼øÀ§ / ÆÀ / »ý?E/ ¼ºÀûÀÌ ¼ÒÆ®ÀÇ ±âÁØÀÌ´Ù
bool CompareZScoreBoardItem(ZScoreBoardItem* a,ZScoreBoardItem* b) {
	if(a->nDuelQueueIdx < b->nDuelQueueIdx) return true;
	if(a->nDuelQueueIdx > b->nDuelQueueIdx) return false;

	if(a->nTeam < b->nTeam) return true;
	if(a->nTeam > b->nTeam) return false;

	/*
	if(!a->bDeath && b->bDeath) return true;
	if(a->bDeath && !b->bDeath) return false;
	*/

	if(!ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		if (a->nExp > b->nExp) return true;
		if (a->nExp < b->nExp) return false;
	}

	if(a->nKills > b->nKills) return true;
	if(a->nKills < b->nKills) return false;

	return false;
}
void ZCombatInterface::DrawDuelTournamentScoreBoard(MDrawContext* pDC)	
{
	MBitmap* pBitmap;
	int nBar = (timeGetTime() / 125 % 2);
	int Animation = 0; 
	for (Animation = 0; Animation < nBar; Animation++);
	if (Animation == 0)
	{
		char Ani0[20];
		sprintf(Ani0, "scoreboard1.png");
		MBitmap* pBitmap1;
		pBitmap1 = MBitmapManager::Get(Ani0);
		if (pBitmap1 != 0)
		{
			pDC->SetBitmap(pBitmap1);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(Ani0));
		}
	}
	if (Animation == 1)
	{
		char Ani1[20];
		sprintf(Ani1, "scoreboard2.png");
		MBitmap* pBitmap2;
		pBitmap2 = MBitmapManager::Get(Ani1);
		if (pBitmap2 != 0)
		{
			pDC->SetBitmap(pBitmap2);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(Ani1));
		}
	}
	//ZGetScreenEffectManager()->DrawScoreBoard();
	MFont *pFont=GetGameFont();
	pDC->SetFont(pFont);
	pFont=pDC->GetFont();	
	pDC->SetColor(MCOLOR(TEXT_COLOR_TITLE));
	char szText[256];
	// Name Stage
	//sprintf(szText, "[%03d] %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
	//TextRelative(pDC, 0.18f, 0.22f, szText);

	float x = 0.150f;
	float y = 0.250f;
	float linespace2= 0.071f / 3.f;

	char MapName[256];
	y -= linespace2;
	strcpy(MapName, ZGetGameClient()->GetMatchStageSetting()->GetMapName()); // MAP NAME
	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		sprintf(MapName, "%s (%s %d)", MapName, ZMsg(MSG_CHARINFO_LEVELMARKER), ZGetQuest()->GetGameInfo()->GetQuestLevel());
	}
	TextRelative(pDC, 0.23f, 0.214f, MapName);
	// TypeGame String
	//sprintf(szText, "%s", ZGetGameTypeManager()->GetGameTypeStr(ZGetGame()->GetMatch()->GetMatchType()));
	//TextRelative(pDC,x,y,szText);

	x = 0.71f;
	y = 0.284f;

	// ³²Àº ½Ã°£ Ç¥½Ã( Å¬·£?EÁ¦¿Ü)
	DrawPlayTime(pDC, x, y);	// ÇÃ·¹ÀÌ ½Ã°£
	y -= linespace2;

	// ?EøÇ?±×¸®?E
	((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->ShowMatchOrder(pDC, false, m_fElapsed);
}
void ZCombatInterface::DrawPlayTime(MDrawContext* pDC, float xPos, float yPos)
{
	char szText[256];
	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
		DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
		if (dwLimitTime != -1)//99999)
		{
			// dwLimitTime *= 60000;
			if (dwTime <= dwLimitTime)
			{
				dwTime = (dwLimitTime - dwTime) / 1000;
				sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
			}
			else
				sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
	}
	else
		sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

	TextRelative(pDC, xPos, yPos, szText);
}
// ¿ø·¡¶ó?EDrawPlayTime()¿¡¼­ ºÐ¸®ÇØ³»¾ß ÇÏ´Â ³»?EÌÁö¸?°Çµå¸®?E¹«¼·°Ô »ý?E ³²Àº½Ã°£ ¾ò´Â ÇÔ¼ö¸¦ »õ·Î ¸¸µé¾úÀ¸´Ï ½Ã°£À» ¾ò?E½ÍÀ¸?E¾ÕÀ¸·Î ÀÌ°ÍÀ» »ç?E
int ZCombatInterface::GetPlayTime()
{
	// ÃÊ´ÜÀ§·Î ³²Àº ¶ó¿ûÑE½Ã°£À» ¸®ÅÏ, ¹«È¿°ªÀÏ ¶§ -1 ¸®ÅÏÇÔ
	if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY)
		return -1;

	DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
	if (dwLimitTime == -1/*99999*/ || dwLimitTime == 0) // ¼­¹ö¿¡¼­´Â ¹«ÇÑ´EÏ¶§ nLimitTimeÀ» 0À¸·Î º¸³»ÁØ´Ù.
		return -2; // ¹«ÇÑ´E½Ã°£À» Ç¥½ÃÇÏ±EÀ§ÇØ -2·Î ¹ÝÈ¯

	// dwLimitTime *= 60000;
	DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
	if (dwTime > dwLimitTime)
		return -1;

	return (dwLimitTime - dwTime) / 1000;
}
typedef list<ZScoreBoardItem*> ZSCOREBOARDITEMLIST;
void ZCombatInterface::DrawScoreBoard(MDrawContext* pDC)
{
	MMATCH_GAMETYPE nGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
	MCHANNEL_TYPE nChannelType = ZGetGameClient()->GetChannelType();
	int nStageType = ZGetGameClient()->GetMatchStageSetting()->GetStageType();
	bool bClanGame = ZGetGameClient()->IsCWChannel();
	bool bPlayerWars = ZGetGameClient()->IsPWChannel();
	bool bSpyOpenCountDown = (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY &&
		(ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_COUNTDOWN || ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FREE));

	ZSCOREBOARDITEMLIST items;
	//Orby: Animacion ScoreBoard.
	MBitmap* pBitmap; // Custom: Define Bitmap
	int nBar = (timeGetTime() / 125 % 2);
	int Animation = 0; //wtf Xd
	for (Animation = 0; Animation < nBar; Animation++);
	if (Animation == 0)
	{
		char Ani0[20];
		sprintf(Ani0, "scoreboard1.png");
		MBitmap* pBitmap1;
		pBitmap1 = MBitmapManager::Get(Ani0);
		if (pBitmap1 != 0)
		{
			pDC->SetBitmap(pBitmap1);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(Ani0));
		}
	}
	if (Animation == 1)
	{
		char Ani1[20];
		sprintf(Ani1, "scoreboard2.png");
		MBitmap* pBitmap2;
		pBitmap2 = MBitmapManager::Get(Ani1);
		if (pBitmap2 != 0)
		{
			pDC->SetBitmap(pBitmap2);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(Ani1));
		}
	}

	ZGetScreenEffectManager()->DrawScoreBoard();

	MFont* pFont = GetGameFont();
	pDC->SetFont(pFont);
	pFont = pDC->GetFont();
	pDC->SetColor(MCOLOR(TEXT_COLOR_TITLE));

	float linespace3 = (0.578f - (float)pFont->GetHeight() * 1.1f / (float)RGetScreenHeight()) / (float)18.f;
	int GMTSizeX = 05.5f * linespace3 * (float)MGetWorkspaceHeight();
	int GMTSizeY = 1.35f * linespace3 * (float)MGetWorkspaceHeight();
	float xpos = 0.18f;
	float ypos = 0.292f;

	if (bClanGame)
	{
		switch (nStageType)
		{
		case (MST_LADDER):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("ClanWar.png"));
			break;
		case (MST_PLAYERWARS):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Ladder.png"));
			break;
		default:
			break;
		}
	}
	else
	{
		switch (nGameType)
		{
		case (MMATCH_GAMETYPE_DEATHMATCH_SOLO):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("DeathMatch.png"));
			break;
		case (MMATCH_GAMETYPE_DEATHMATCH_TEAM):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("TDM.png"));
			break;
		case (MMATCH_GAMETYPE_GLADIATOR_SOLO):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Gladiator.png"));
			break;
		case (MMATCH_GAMETYPE_BERSERKER):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("DMBerserker.png"));
			break;
		case (MMATCH_GAMETYPE_GLADIATOR_TEAM):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("TeamGladiator.png"));
			break;
		case (MMATCH_GAMETYPE_ASSASSINATE):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Assassination.png"));
			break;
		case (MMATCH_GAMETYPE_TRAINING):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Training.png"));
			break;
		case (MMATCH_GAMETYPE_SURVIVAL):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Survival.png"));
			break;
		case (MMATCH_GAMETYPE_QUEST):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Quest.png"));
			break;
		case (MMATCH_GAMETYPE_QUEST_CHALLENGE):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Challenge.png"));
			break;
		case (MMATCH_GAMETYPE_DEATHMATCH_TEAM2):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("TDMExtream.png"));
			break;
		case (MMATCH_GAMETYPE_DUEL):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("TheDuelMatch.png"));
			break;
		case (MMATCH_GAMETYPE_CTF):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("CTFMod.png"));
			break;
		case (MMATCH_GAMETYPE_SPY):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("SpyMod.png"));
			break;
		case (MMATCH_GAMETYPE_TEAM_TRAINING):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("TeamTraining.png"));
			break;
		case (MMATCH_GAMETYPE_INFECTED):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("Infected.png"));
			break;
		case (MMATCH_GAMETYPE_GUNGAME):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("GunGame.png"));
			break;
		case (MMATCH_GAMETYPE_DROPMAGIC):
			BitmapRelative(pDC, xpos, ypos, GMTSizeX, GMTSizeY, MBitmapManager::Get("MagicBox.png"));
			break;

		default:
			break;
		}
	}


	char szText[256];
	int nRed = 0, nBlue = 0;
	if (bClanGame || bPlayerWars)
	{
		for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
			itor != ZGetCharacterManager()->end(); ++itor)
		{
			ZCharacter* pCharacter = (*itor).second;

			if (pCharacter->GetTeamID() == MMT_BLUE) nBlue++;
			if (pCharacter->GetTeamID() == MMT_RED) nRed++;
		}
		if (bPlayerWars)
			sprintf(szText, "Gunz Ladder (Competitive) - (%d) VS (%d)", nRed, nBlue);
		else
		{
			switch (ZGetGame()->GetMatch()->GetMatchType())
			{
			case MMATCH_GAMETYPE_ASSASSINATE:
				sprintf(szText, "Clan War Assassinate %d:%d (%s vs %s)", nRed, nBlue, m_szRedClanName, m_szBlueClanName);
				break;
			case MMATCH_GAMETYPE_GLADIATOR_TEAM:
				sprintf(szText, "Clan War Gladiator %d:%d (%s vs %s)", nRed, nBlue, m_szRedClanName, m_szBlueClanName);
				break;
			default:
				sprintf(szText, "Clan War %d:%d (%s vs %s)", nRed, nBlue, m_szRedClanName, m_szBlueClanName);
				break;
			}
		}

		char nvsn[32];
		sprintf(nvsn, "%d:%d", nRed, nBlue);

		if (nChannelType == MCHANNEL_TYPE_CLAN)
			ZTransMsg(szText, MSG_GAME_SCORESCREEN_STAGENAME, 3, nvsn, m_szRedClanName, m_szBlueClanName);
		else if (nChannelType == MCHANNEL_TYPE_PLAYERWARS)
			ZTransMsg(szText, MSG_GAME_SCORESCREEN_STAGENAME, 3, nvsn, m_szRedClanName, m_szBlueClanName);
	}
	if (bClanGame)
	{
		sprintf(szText, "[%03d] Clan War (Mode Competitive)", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		TextRelative(pDC, 0.18f, 0.22f, szText);
	}
	else if (bPlayerWars)
	{
		sprintf(szText, "[%03d] Gunz Ladder (Mode Competitive)", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		TextRelative(pDC, 0.18f, 0.22f, szText);
	}
	else
	{
		sprintf(szText, "[%03d] %s", ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		TextRelative(pDC, 0.18f, 0.22f, szText);
	}


	float x = 0.18f; //Map Name
	float y = 0.284f;
	float linespace2 = 0.071f / 3.f;


	if (ZGetGame()->GetMatch()->IsTeamPlay())
	{
		if (bPlayerWars)
			sprintf(szText, "%d (Ladder Red)  VS  %d (Ladder Blue)", ZGetGame()->GetMatch()->GetTeamScore(MMT_RED), ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE));
		else if (bClanGame)
		{

			{

				sprintf(szText, "%d (%s)  VS  %d (%s)",
					ZGetGame()->GetMatch()->GetTeamScore(MMT_RED), m_szRedClanName,
					ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE), m_szBlueClanName);
			}
		}
		else
		{
			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) // ÆÀÀEÏ¶§ ¹«ÇÑµ¥½º¸ÅÄ¡¸¸ ¿¹¿Ü°¡ ¸¹ÀÌ ¹ß»ýÇÕ´Ï´Ù =_=;
				sprintf(szText, "%s : %d(Red) vs %d(Blue)", ZGetGameTypeManager()->GetGameTypeStr(ZGetGame()->GetMatch()->GetMatchType()),
					ZGetGame()->GetMatch()->GetTeamKills(MMT_RED),
					ZGetGame()->GetMatch()->GetTeamKills(MMT_BLUE));
		}
	}
	else
		// TypeGame String
		//sprintf(szText, "%s", ZGetGameTypeManager()->GetGameTypeStr(ZGetGame()->GetMatch()->GetMatchType()));
		//TextRelative(pDC, x, y, szText);
		y -= linespace2;

	// Custom: CQ fixes
	// Custom: MapName
	char MapName[256];
	y -= linespace2;
	strcpy(MapName, ZGetGameClient()->GetMatchStageSetting()->GetMapName()); // MAP NAME
	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()) || ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		sprintf(MapName, "%s (%s %d)", MapName, ZMsg(MSG_CHARINFO_LEVELMARKER), ZGetQuest()->GetGameInfo()->GetQuestLevel());
	}
	TextRelative(pDC, 0.18f, 0.254f, MapName);

	// Custom: Time spent in game
	{
		x = 0.70f;
		y = 0.234f;
		if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
		{
			ZRuleQuestChallenge* pCQRule = NULL;
			if (ZGetGame()->GetMatch()->GetRule())
				pCQRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
			if (pCQRule)
			{
				TextRelative(pDC, x, y, pCQRule->GetCurrTimeString());
			}
		}
	}

	x = 0.70f;
	y = 0.284f;


	// º¸?E¾ÆÀÌÅÛ
	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))		// Äù½ºÆ® ¸ðµåÀÏ°æ?E
	{
		sprintf(szText, "%s : %d", ZMsg(MSG_WORD_GETITEMQTY), ZGetQuest()->GetGameInfo()->GetNumOfObtainQuestItem());
		TextRelative(pDC, x, y, szText);
		y -= linespace2;
	}

	// Custom: CQ fixes
	// NPC ?E
	//Removed QuestChallenge from here, you can't use the quest npc counter code for zactorwithfsm npc's.
	if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType())) 	// Äù½ºÆ® ¸ðµåÀÏ °æ?E
	{
		int nNPCKilled = ZGetQuest()->GetGameInfo()->GetNPCCount() - ZGetQuest()->GetGameInfo()->GetNPCKilled();
		if (nNPCKilled < 0)
			nNPCKilled = 0;

		MUID uidBoss = ZGetQuest()->GetGameInfo()->GetBoss();

		if (uidBoss != MUID(0, 0))
			sprintf(szText, "%s : -", ZMsg(MSG_WORD_REMAINNPC));
		else
			sprintf(szText, "%s : %d", ZMsg(MSG_WORD_REMAINNPC), nNPCKilled);
		TextRelative(pDC, x, y, szText);
		y -= linespace2;
	}
	//this is where the npc count is grabbed for cq
	else if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		ZRuleQuestChallenge* pCQRule = NULL;
		if (ZGetGame()->GetMatch()->GetRule())
		{
			pCQRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
		}
		if (pCQRule)
		{
			sprintf(szText, "%s : %d", ZMsg(MSG_WORD_REMAINNPC), pCQRule->GetNPC());
			TextRelative(pDC, x, y, szText);
			y -= linespace2;
		}
	}
	else if (ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType())) 	// ¼­¹ÙÀÌ?E¸ðµåÀÏ °æ?E
	{
		int nNPCKilled = ZGetQuest()->GetGameInfo()->GetNPCCount() - ZGetQuest()->GetGameInfo()->GetNPCKilled();
		if (nNPCKilled < 0)
			nNPCKilled = 0;

		MUID uidBoss = ZGetQuest()->GetGameInfo()->GetBoss();

		// ¼­¹ÙÀÌ¹ú¿£ ÀÚÄÚ°¡ ¾øÀ¸¹Ç·Î º¸½º°¡ ³ª¿À´õ¶óµµ ÀÜ¿© npc ¼ö´Â Ç¥½Ã °¡´ÉÇÏ´Ù
		sprintf(szText, "%s : %d", ZMsg(MSG_WORD_REMAINNPC), nNPCKilled);
		TextRelative(pDC, x, y, szText);
		y -= linespace2;
	}

	// ÁøÇàµµ Ç¥½Ã
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
	{
		if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)				// ÇÃ·¹ÀÌ ÁßÀÌ¶ó?E
		{
			DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
			DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
			if (dwLimitTime != -1)//99999)
			{
				//dwLimitTime *= 60000;
				if (dwTime <= dwLimitTime)
				{
					dwTime = (dwLimitTime - dwTime) / 1000;
					sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
				}
				else
					sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
			}
			else
				sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

		TextRelative(pDC, x, y, szText);
		y -= linespace2;

		sprintf(szText, "%s : %d", ZMsg(MSG_WORD_ENDKILL), ZGetGame()->GetMatch()->GetRoundCount());
	}

	// ¶ó¿ûÑå¸¦ ±â´Ù·Á¾ß ÇÏ´Â Á¾·ù¶ó?E¶ó¿û?EÇ¥½Ã ¾Æ´Ï?E½Ã°£ Ç¥½Ã
	else if (ZGetGame()->GetMatch()->IsWaitForRoundEnd() && !bClanGame && !bPlayerWars)
	{
		if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)				// ÇÃ·¹ÀÌ ÁßÀÌ¶ó?E
		{
			DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
			DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
			if (dwLimitTime != -1)//99999)
			{
				//dwLimitTime *= 60000;
				if (dwTime <= dwLimitTime)
				{
					dwTime = (dwLimitTime - dwTime) / 1000;
					sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
				}
				else
					sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
			}
			else
				sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

		TextRelative(pDC, x, y, szText);
		y -= linespace2;

		sprintf(szText, "%s : %d / %d %s", ZMsg(MSG_WORD_RPROGRESS), ZGetGame()->GetMatch()->GetCurrRound() + 1, ZGetGame()->GetMatch()->GetRoundCount(), ZMsg(MSG_WORD_ROUND));
	}
	// Custom: CQ fixes
	else if (ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))	// Äù½ºÆ® ¸ðµåÀÏ °æ?E
	{
		sprintf(szText, "%s : %d / %d", ZMsg(MSG_WORD_RPROGRESS), ZGetQuest()->GetGameInfo()->GetCurrSectorIndex() + 1, ZGetQuest()->GetGameInfo()->GetMapSectorCount());
	}
	else if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		ZRuleQuestChallenge* pCQRule = NULL;
		if (ZGetGame()->GetMatch()->GetRule())
		{
			pCQRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
		}
		sprintf(szText, "%s : %d / %d", ZMsg(MSG_WORD_RPROGRESS), pCQRule->GetCurrSector() + 1, pCQRule->GetRoundMax());
	}
	else if (ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType())) 	// ¼­¹ÙÀÌ?E¸ðµåÀÏ °æ?E
	{
		int currSector = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex() + 1;
		int sectorCount = ZGetQuest()->GetGameInfo()->GetMapSectorCount();
		int repeatCount = ZGetQuest()->GetGameInfo()->GetRepeatCount();

		currSector += ZGetQuest()->GetGameInfo()->GetCurrRepeatIndex() * sectorCount;
		sectorCount *= repeatCount;
		sprintf(szText, "%s : %d / %d", ZMsg(MSG_WORD_RPROGRESS), currSector, sectorCount);
	}

	// ³²Àº ½Ã°£ Ç¥½Ã( Å¬·£?EÁ¦¿Ü)
	else if (!bClanGame && !bPlayerWars)
	{
		if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)				// ÇÃ·¹ÀÌ ÁßÀÌ¶ó?E
		{
			DWORD dwTime = ZGetGame()->GetMatch()->GetRemaindTime();
			DWORD dwLimitTime = ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nLimitTime;
			if (dwLimitTime != -1)//99999)
			{
				// dwLimitTime *= 60000;
				if (dwTime <= dwLimitTime)
				{
					dwTime = (dwLimitTime - dwTime) / 1000;
					sprintf(szText, "%s : %d:%02d", ZMsg(MSG_WORD_REMAINTIME), (dwTime / 60), (dwTime % 60));
				}
				else
					sprintf(szText, "%s :", ZMsg(MSG_WORD_REMAINTIME));
			}
			else
				sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));
		}
		else
			sprintf(szText, "%s : ---", ZMsg(MSG_WORD_REMAINTIME));

		TextRelative(pDC, x, y, szText);
		y -= linespace2;

		// Custom: CTF
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_CTF)
			sprintf(szText, "%s : %d", ZMsg(MSG_WORD_CAPUTRES), ZGetGame()->GetMatch()->GetRoundCount());
		else
			sprintf(szText, "%s : %d", ZMsg(MSG_WORD_ENDKILL), ZGetGame()->GetMatch()->GetRoundCount());

	}
	TextRelative(pDC, x, y, szText);

#ifdef _SRVRPNG
	if ((ZGetGameClient()->IsPWChannel() ||
		ZGetGameClient()->IsCWChannel() ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_SOLO ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GLADIATOR_SOLO ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GLADIATOR_TEAM ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_ASSASSINATE ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_TRAINING ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_BERSERKER ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM2 ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_TEAM_TRAINING ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_CTF ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GUNGAME ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DROPMAGIC ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SURVIVAL ||
		ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST))
	{
		y -= linespace2;
		int nPing = ZGetGameClient()->GetServerPing();
		sprintf(szText, "Server Ping : %d ms", nPing);
		//pDC->SetColor(255, 255, 255);
		TextRelative(pDC, x, y, szText);
	}
#endif
	if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_QUEST && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_QUEST_CHALLENGE && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_SURVIVAL && ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nPingLimitHigh != 0)
	{
		y -= linespace2;
		if (ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nPingType == true)
			sprintf(szText, "Server Ping Limiter : %d ~ %d", ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nPingLimitLow, ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nPingLimitHigh);
		else
			sprintf(szText, "Peer Ping Limiter : %d ~ %d", ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nPingLimitLow, ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->nPingLimitHigh);

		TextRelative(pDC, x, y, szText);
	}
	const float normalXPOS[] = { 0.26f, 0.47f, 0.67f, 0.78f, 0.84f, 0.93f, 0.334f, 0.311f };
	const float clanXPOS[] = { 0.44f, 0.24f, 0.67f, 0.76f, 0.82f, 0.91f, 0.514f, 0.491f };
	bool clanpos = false;
	/*if (bPlayerWars || bClanGame) clanpos = true;
	const float *ITEM_XPOS = bClanGame ? clanXPOS : normalXPOS;*/

	if (bClanGame) clanpos = true; // Custom: fix Scoreboard PW
	const float* ITEM_XPOS = clanpos ? clanXPOS : normalXPOS;

	y = 0.343f;
	const float fHeight = 0.658f;

	char szBuff[25];

	pDC->SetColor(MCOLOR(TEXT_COLOR_TITLE));
	if (bPlayerWars)
	{
		x = ITEM_XPOS[7] - .130f;	// Score
		sprintf(szBuff, "%s", ZMsg(MSG_WORD_SCORE));
		TextRelative(pDC, x, y, szBuff);
	}
	else if (bClanGame)
	{
		x = ITEM_XPOS[7] - .130f;	// Points
		sprintf(szBuff, "%s", ZMsg(MSG_WORD_POINTS));
		TextRelative(pDC, x, y, szBuff);
	}
	else
	{
		x = ITEM_XPOS[7] - .130f;	// flag
		sprintf(szBuff, "%s", ZMsg(MSG_CHARINFO_FLAG));
		TextRelative(pDC, x, y, szBuff);
	}

	x = ITEM_XPOS[0];	// level
	sprintf(szBuff, "%s", ZMsg(MSG_CHARINFO_LEVEL));
	TextRelative(pDC, x, y, szBuff);

	x = ITEM_XPOS[6];	// ÀÌ¸§
	sprintf(szBuff, "%s", ZMsg(MSG_CHARINFO_NAME));
	TextRelative(pDC, x, y, szBuff);

	x = ITEM_XPOS[1] + .02f;	// Clan
	TextRelative(pDC, x, y, ZMsg(MSG_CHARINFO_CLAN));
	if (ZGetGameTypeManager()->IsSpyGame(ZGetGame()->GetMatch()->GetMatchType())) // Custom: Spy Points Drawn
	{
		x = ITEM_XPOS[2] - .01f;	// POINTS SPY
		TextRelative(pDC, x, y, ZMsg(MSG_WORD_POINTS));
		//TextRelative(pDC, x, y, szBuff);
	}
	else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
	{
		x = ITEM_XPOS[2];	// HP/AP
		sprintf(szBuff, "%s/%s", ZMsg(MSG_CHARINFO_HP), ZMsg(MSG_CHARINFO_AP));
		TextRelative(pDC, x, y, szBuff);
	}
	else if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		char szBuff[32]; //jork
		x = ITEM_XPOS[2] - .01f;	// RANK
		TextRelative(pDC, x, y, ZMsg(MSG_WORD_RANK));
		//TextRelative(pDC, x, y, szBuff);

		x = ITEM_XPOS[3] - .01f;	// POINTS
		TextRelative(pDC, x, y, ZMsg(MSG_WORD_POINTS));
		//TextRelative(pDC, x, y, szBuff);
	}
	else
	{
		x = ITEM_XPOS[2] - .01f;	// Exp
		TextRelative(pDC, x, y, ZMsg(MSG_WORD_EXP));
	}

	if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{

	}
	else
	{
		x = ITEM_XPOS[3] - .01f;	// Kills
		TextRelative(pDC, x, y, ZMsg(MSG_WORD_KILL));
		x = ITEM_XPOS[4] - .01f;	// Deaths
		TextRelative(pDC, x, y, ZMsg(MSG_WORD_DEATH));
	}

	x = ITEM_XPOS[5] - .01f;	// Ping
	TextRelative(pDC, x, y, ZMsg(MSG_WORD_PING));

	float fTitleHeight = (float)pFont->GetHeight() * 1.1f / (float)RGetScreenHeight();
	y += fTitleHeight;

	// ±×¸±¼öÀÖ´Â ÃÖ?EÙ¼EÁÙ°£°ÝÀº 150%
//	int nMaxLineCount=int((fHeight-fTitleHeight)*(float)RGetScreenHeight()/((float)pFont->GetHeight()*1.1f));
	int nMaxLineCount = 16;

	// ÇÑÁÙ»çÀÌÀÇ °£°Ý(³ôÀÌ)
	float linespace = (fHeight - fTitleHeight) / (float)nMaxLineCount;

	// ÁÂÃøÀÇ Å¬·£¸¶Å©¹× ÀÌ¸§,Á¡?E
	if (bClanGame)
	{
		for (int i = 0; i < 2; i++)
		{
			MMatchTeam nTeam = (i == 0) ? MMT_RED : MMT_BLUE;
			char* szClanName = (i == 0) ? m_szRedClanName : m_szBlueClanName;
			int nClanID = (i == 0) ? m_nClanIDRed : m_nClanIDBlue;

			MFont* pClanFont = MFontManager::Get("FONTb11b");
			if (pClanFont == NULL) _ASSERT(0);
			pDC->SetFont(pClanFont);
			pDC->SetColor(MCOLOR(TEXT_COLOR_CLAN_NAME));

			float clancenter = .2f * (ITEM_XPOS[0] - ITEM_XPOS[1]) + ITEM_XPOS[1];
			float clanx = clancenter - .5f * ((float)pClanFont->GetWidth(szClanName) / (float)MGetWorkspaceWidth());
			float clany = y + linespace * ((nTeam == MMT_RED) ? .2f : 8.5f); // Custom: Emblem Mod

			// ¾ÆÀÌÄÜ Ãâ·Â
			MBitmap* pbmp = ZGetEmblemInterface()->GetClanEmblem(nClanID);
#ifdef TEST_CLAN_SCOREBOARD
			pbmp = MBitmapManager::Get("btntxtr_gnd_on.png");//Å×½ºÆ®?E
#endif
			if (pbmp) {
				pDC->SetBitmap(pbmp);

				const float fIconSize = .1f;
				int nIconSize = fIconSize * MGetWorkspaceWidth();

				int screenx = (clancenter - .5f * fIconSize) * MGetWorkspaceWidth();
				int screeny = (clany)*MGetWorkspaceHeight();

				pDC->Draw(screenx, screeny, nIconSize, nIconSize);

				clany += fIconSize + 1.2 * linespace;
			}

			// ÆÀÀÌ¸§ Ãâ·Â
			TextRelative(pDC, clanx, clany, szClanName);

			// Á¡?EÃâ·Â. °¡¿ûÑ¥ Á¤·Ä
			sprintf(szText, "%d", ZGetGame()->GetMatch()->GetTeamScore(nTeam));
			clanx = clancenter - .5f * ((float)pClanFont->GetWidth(szText) / (float)MGetWorkspaceWidth());
			clany += 1.f * linespace;
			TextRelative(pDC, clanx, clany, szText);

		}
	}


	// Ä³¸¯ÅÍ ¸®½ºÆ®
	ZCharacterManager::iterator itor;
	for (itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

		if (pCharacter->GetTeamID() == MMT_SPECTATOR) continue;	// ¿ÉÀú¹ö´Â –A´Ù

		if (pCharacter->IsAdminHide()) continue;

		ZScoreBoardItem* pItem = new ZScoreBoardItem;

		if (pCharacter->IsAdminName() || pCharacter->GetUserGrade() == MMUG_EVENTTEAM)
		{
			// Custom: Unmask names
			//sprintf(pItem->szLevel,"%s%d",ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			sprintf(pItem->szLevel, "--%s", ZMsg(MSG_CHARINFO_LEVELMARKER));

			// Custom: Set custom colours
			MCOLOR gmColor;
			char szEmpty[4];
			memset(szEmpty, 0, sizeof(szEmpty));

			if (GetUserGradeIDColor(pCharacter->GetUserGrade(), gmColor, szEmpty))
				pItem->SetColor(gmColor);
			else
				pItem->SetColor(ZCOLOR_ADMIN_NAME);

			//pItem->SetColor(ZCOLOR_ADMIN_NAME);
		}
#ifdef _VIPGRADES
		else if (pCharacter->IsVIP1Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_VIP1_NAME);
		}

		else if (pCharacter->IsVIP2Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_VIP2_NAME);
		}

		else if (pCharacter->IsVIP3Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_VIP3_NAME);
		}
		else if (pCharacter->IsVIP4Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_VIP4_NAME);
		}

		else if (pCharacter->IsVIP5Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_VIP5_NAME);
		}

		else if (pCharacter->IsVIP6Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_VIP6_NAME);
		}
		else if (pCharacter->IsVIP7Name()) {
			//sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			sprintf(pItem->szLevel, "--%s", ZMsg(MSG_CHARINFO_LEVELMARKER));
			pItem->SetColor(ZCOLOR_VIP7_NAME);
		}
#endif

#ifdef _EVENTGRD
		else if (pCharacter->IsEvent1Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_EVENT1_NAME);
		}

		else if (pCharacter->IsEvent2Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_EVENT2_NAME);
		}

		else if (pCharacter->IsEvent3Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_EVENT3_NAME);
		}
		else if (pCharacter->IsEvent4Name()) {
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
			pItem->SetColor(ZCOLOR_EVENT4_NAME);
		}
#endif

		else
		{
			sprintf(pItem->szLevel, "%s%d", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetProperty()->nLevel);
		}

		sprintf(pItem->szName, "%s", pCharacter->GetUserName());


#ifdef _TAGVIP
		if (pCharacter->IsAdminName())
			sprintf(pItem->szGrade, "%s", "STF");
		else if (pCharacter->IsVIP1Name())
			sprintf(pItem->szGrade, "%s", "VIP");
		else if (pCharacter->IsVIP2Name())
			sprintf(pItem->szGrade, "%s", "VIP");
		else if (pCharacter->IsVIP3Name())
			sprintf(pItem->szGrade, "%s", "VIP");
		else if (pCharacter->IsVIP4Name())
			sprintf(pItem->szGrade, "%s", "VIP");
		else if (pCharacter->IsVIP5Name())
			sprintf(pItem->szGrade, "%s", "VIP");
		else if (pCharacter->IsVIP6Name())
			sprintf(pItem->szGrade, "%s", "VIP");
		else if (pCharacter->IsVIP7Name())
			sprintf(pItem->szGrade, "%s", "SUPER");

		else if (pCharacter->IsEvent1Name())
			sprintf(pItem->szGrade, "%s", "EVN");
		else if (pCharacter->IsEvent2Name())
			sprintf(pItem->szGrade, "%s", "EVN");
		else if (pCharacter->IsEvent3Name())
			sprintf(pItem->szGrade, "%s", "EVN");
		else if (pCharacter->IsStaffName())
			sprintf(pItem->szGrade, "%s", "EVN");
		else if (pCharacter->Event())
			sprintf(pItem->szGrade, "%s", "EVN");
		else
			sprintf(pItem->szGrade, "%s", "");
#endif
		memcpy(pItem->szClan, pCharacter->GetProperty()->GetClanName(), CLAN_NAME_LENGTH);


		pItem->nClanID = pCharacter->GetClanID();
		pItem->nTeam = ZGetGame()->GetMatch()->IsTeamPlay() ? pCharacter->GetTeamID() : MMT_END;
		pItem->bDeath = pCharacter->IsDie();
		if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
			pItem->nExp = pCharacter->GetStatus().Ref().nKills * 100;
		else if (!ZGetGameTypeManager()->IsSpyGame(ZGetGame()->GetMatch()->GetMatchType()))
			pItem->nExp = pCharacter->GetStatus().Ref().nExp;
		else
			pItem->nExp = pCharacter->GetStatus().Ref().nScore;
		pItem->nKills = pCharacter->GetStatus().Ref().nKills;
		pItem->nDeaths = pCharacter->GetStatus().Ref().nDeaths;
		pItem->uidUID = pCharacter->GetUID();

		int nPing = (pCharacter->GetUID() == ZGetGameClient()->GetPlayerUID() ? 0 : MAX_PING);
		MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(pCharacter->GetUID());
		if (pPeer) {
			if (ZGetGame()->IsReplay())
				nPing = 0;
			else
				nPing = pPeer->GetPing(ZGetGame()->GetTickTime());
		}
		pItem->nPing = nPing;
		pItem->bMyChar = pCharacter->IsHero();

		if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_SPECTATOR &&
			m_Observer.IsVisible() && m_Observer.GetTargetCharacter() == pCharacter)
			pItem->bMyChar = true;



		if (pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID() && pCharacter->m_dwStatusBitPackingValue.Ref().m_bCommander)
			pItem->bCommander = true;
		else
			pItem->bCommander = false;

		// ¹ö¼­Ä¿¸ðµåÀÇ ¹ö¼­Ä¿
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_BERSERKER)
		{
			if (pCharacter->IsTagger()) pItem->bCommander = true;
		}

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_CTF && pCharacter->IsTagger())
			pItem->bCommander = true;

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED && pCharacter->m_dwStatusBitPackingValue.Ref().m_bCommander)
			pItem->bCommander = true;

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
			ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGame()->GetMatch()->GetRule();	// À§?E-_-
			pItem->nDuelQueueIdx = pDuel->GetQueueIdx(pCharacter->GetUID());
		}
		else
			pItem->nDuelQueueIdx = 0;


		// GameRoom User
		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(pCharacter->GetUID());
		if (pCache)
			pItem->bGameRoomUser = (pCache->GetPGrade() == MMPG_PREMIUM_IP) ? true : false;
		else
			pItem->bGameRoomUser = false;

		// Áö³­ÁÖ µà¾óÅä³Ê¸ÕÆ® µûÍÞ
		pItem->nDTLastWeekGrade = pCharacter->GetDTLastWeekGrade();
		pItem->nCountry = pCharacter->GetCountry();
		items.push_back(pItem);
	}


	items.sort(CompareZScoreBoardItem);
	ZSCOREBOARDITEMLIST::iterator i;

	int nCurrentTeamIndex;
	if (ZGetGame()->GetMatch()->IsTeamPlay() && !bSpyOpenCountDown)
		nCurrentTeamIndex = MMT_RED;
	else
	{
		if (items.size() > 0)
			nCurrentTeamIndex = (*items.begin())->nTeam;
	}

	int nCount = 0;

#ifdef _MIPING
	int MiPing = 0;
	int total = 0;
#endif

	for (i = items.begin(); i != items.end(); i++)
	{
		ZScoreBoardItem* pItem = *i;

		if (nCurrentTeamIndex != pItem->nTeam)
		{
			int nLineSeparate = nMaxLineCount / 2;

			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
				nLineSeparate = 4;

			nCurrentTeamIndex = pItem->nTeam;
			nCount = max(nCount, min(nLineSeparate, nMaxLineCount - ((int)items.size() - nCount)));
		}

		float itemy = y + linespace * nCount;

		nCount++;

		if (nCount > nMaxLineCount) break;

		// ¹è?E»ö±òÀ» °áÁ¤ÇÑ´Ù
		MCOLOR backgroundcolor;
		if (pItem->bMyChar) {
			backgroundcolor = BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;
			if (!bClanGame && !bPlayerWars) {
				backgroundcolor =
					(pItem->nTeam == MMT_RED) ? BACKGROUND_COLOR_MYCHAR_RED_TEAM :
					(pItem->nTeam == MMT_BLUE) ? BACKGROUND_COLOR_MYCHAR_BLUE_TEAM :
					BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;
			}
		}

		if (pItem->bCommander) backgroundcolor = BACKGROUND_COLOR_COMMANDER;

		if (pItem->bMyChar || pItem->bCommander)
		{
			int y1 = itemy * MGetWorkspaceHeight();
			int y2 = (y + linespace * nCount) * MGetWorkspaceHeight();
			bool clangame = false;
			if (bClanGame) clangame = true; // Custom: fix Scoreboard PW
			int x1 = bClanGame ? 0.345 * MGetWorkspaceWidth() : 0.180 * MGetWorkspaceWidth();
			int x2 = (0.715 + 0.26) * MGetWorkspaceWidth();

			pDC->SetColor(backgroundcolor);
			pDC->FillRectangleW(x1, y1, x2 - x1, y2 - y1);
		}

		//		backgroundy=newbackgroundy;

		/*if (pItem->bGameRoomUser)
		{
			int nIconSize = .8f * linespace * (float)MGetWorkspaceHeight();
			float icony = itemy + (linespace - (float)nIconSize / (float)MGetWorkspaceHeight())*.5f;
			BitmapRelative(pDC, ITEM_XPOS[0] - 0.043f, icony, nIconSize + 4, nIconSize, MBitmapManager::Get("icon_gameroom_s.tga"));
		}*/

		ZCharacter* pCharacter = (*itor).second;
		int nIconSize = .8f * linespace * (float)MGetWorkspaceHeight();
		float icony = itemy + (linespace - (float)nIconSize / (float)MGetWorkspaceHeight()) * .5f;
		float texty = itemy + (linespace - (float)pFont->GetHeight() / (float)MGetWorkspaceHeight()) * .5f;
#ifdef _ICONVIP
		if (pCharacter->GetUserGrade() == MMPG_PREMIUM_IP)
		{
			BitmapRelative(pDC, ITEM_XPOS[0] - 0.43f, icony, nIconSize + 4, nIconSize, MBitmapManager::Get("PC_Room.tga"));
		}
#endif
		if (bClanGame)
		{
			char szDTGradeIconFileName[64];
			GetDuelTournamentGradeIconFileName(szDTGradeIconFileName, pItem->nDTLastWeekGrade);
			MBitmap* pBmpDTGradeIcon = MBitmapManager::Get(szDTGradeIconFileName);
			BitmapRelative(pDC, ITEM_XPOS[7], icony, nIconSize, nIconSize, MBitmapManager::Get(szDTGradeIconFileName));

		}
		else if (bPlayerWars)
		{
			char szDTGradeIconFileName[64];
			GetDuelTournamentGradeIconFileName(szDTGradeIconFileName, pItem->nDTLastWeekGrade);
			MBitmap* pBmpDTGradeIcon = MBitmapManager::Get(szDTGradeIconFileName);
			BitmapRelative(pDC, ITEM_XPOS[7], icony, nIconSize, nIconSize, MBitmapManager::Get(szDTGradeIconFileName));
		}
		else
		{
			char szDTGradeIconFileName[64];
			GetDuelTournamentGradeIconFileName(szDTGradeIconFileName, pItem->nDTLastWeekGrade);
			MBitmap* pBmpDTGradeIcon = MBitmapManager::Get(szDTGradeIconFileName);
			BitmapRelative(pDC, ITEM_XPOS[7], icony, nIconSize, nIconSize, MBitmapManager::Get(szDTGradeIconFileName));

			char szCountryIconFileName[64];
			GetCountryFlagIconFileName(szCountryIconFileName, pItem->nCountry);
			MBitmap* pBmpCountryIcon = MBitmapManager::Get(szCountryIconFileName);
			BitmapRelative(pDC, ITEM_XPOS[0] - 0.066f, icony, nIconSize + 4, nIconSize, MBitmapManager::Get(szCountryIconFileName));
		}

		MCOLOR textcolor = pItem->bDeath ? TEXT_COLOR_DEATH_MATCH_DEAD : TEXT_COLOR_DEATH_MATCH;

		if (!bClanGame && !bPlayerWars)
		{
			if (pItem->nTeam == MMT_RED)		// red
				textcolor = pItem->bDeath ? TEXT_COLOR_RED_TEAM_DEAD : TEXT_COLOR_RED_TEAM;
			else
				if (pItem->nTeam == MMT_BLUE)		// blue
					textcolor = pItem->bDeath ? TEXT_COLOR_BLUE_TEAM_DEAD : TEXT_COLOR_BLUE_TEAM;
				else
					if (pItem->nTeam == MMT_SPECTATOR)
						textcolor = TEXT_COLOR_SPECTATOR;

		}

		if (pItem->bSpColor)	// Æ¯¼öÇÑ À¯Àú¶ó?E.°úÜ¯ÀÇ ÄÃ·¯¸¦ °¡Áö?EÀÖ´Ù.
		{
			if (!pItem->bDeath)
				textcolor = pItem->GetColor();
			else
				textcolor = 0xff402010;
		}

		pDC->SetColor(textcolor);
		pDC->SetFont(pFont);

		x = ITEM_XPOS[0];
		TextRelative(pDC, x, texty, pItem->szLevel);

		x = ITEM_XPOS[6];
		TextRelative(pDC, x, texty, pItem->szName);

		if (!bClanGame)
		{
			x = ITEM_XPOS[1];

			int nIconSize = .8f * linespace * (float)MGetWorkspaceHeight();
			float icony = itemy + (linespace - (float)nIconSize / (float)MGetWorkspaceHeight()) * .5f;

			if (pItem->szClan[0]) {
				MBitmap* pbmp = ZGetEmblemInterface()->GetClanEmblem(pItem->nClanID);
				if (pbmp) {
					pDC->SetBitmap(pbmp);
					int screenx = x * MGetWorkspaceWidth();
					int screeny = icony * MGetWorkspaceHeight();

					pDC->Draw(screenx, screeny, nIconSize, nIconSize);

				}
			}
			x += (float)nIconSize / (float)MGetWorkspaceWidth() + 0.005f;
			TextRelative(pDC, x, texty, pItem->szClan);
		}


		if (ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
		{
			// Custom: Draw HP/AP for quest scoreboard regardless if you are dead or alive.
			bool bDraw = m_Observer.IsVisible();

			ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.find(pItem->uidUID);
			if (itor != ZGetGame()->m_CharacterManager.end())
			{
				// Draw if roundstate is not Prepare.
				if (ZGetGame()->GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
					bDraw = true;

				ZCharacter* pQuestPlayerInfo = (ZCharacter*)(*itor).second;

				MCOLOR tmpColor = pDC->GetColor();

				x = ITEM_XPOS[2];

				pDC->SetColor(MCOLOR(0x4000C9FF));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, 0.08 * MGetWorkspaceWidth(), 7);
				float nValue = (0.08 * (pQuestPlayerInfo->GetHP() / pQuestPlayerInfo->GetMaxHP()));
				pDC->SetColor(MCOLOR(0x4000C9FF));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, nValue * MGetWorkspaceWidth(), 7);

				pDC->SetColor(MCOLOR(0x9000FF59));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, 0.08 * MGetWorkspaceWidth(), 3);
				float nValuee = (0.08 * (pQuestPlayerInfo->GetAP() / pQuestPlayerInfo->GetMaxAP()));
				pDC->SetColor(MCOLOR(0x9000FF59));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, nValuee * MGetWorkspaceWidth(), 3);
				pDC->SetColor(tmpColor);
				MMatchPeerInfo* pQuestPeerInfo = ZGetGameClient()->FindPeer(pQuestPlayerInfo->GetUID());
				//removed dependence on observer setting
				if (bDraw)
				{
					float nValue = 0.08 * pQuestPlayerInfo->GetHP() / pQuestPlayerInfo->GetMaxHP();
					pDC->SetColor(MCOLOR(0x9000a2ff));
					pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 1, nValue * MGetWorkspaceWidth(), 7);
				}
				if (bDraw)
					pDC->SetColor(MCOLOR(0x4000FF00));
				else
					pDC->SetColor(MCOLOR(0x30000000));
				pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, 0.08 * MGetWorkspaceWidth(), 3);
				if (bDraw)
				{
					float nValue = 0.08 * pQuestPlayerInfo->GetAP() / pQuestPlayerInfo->GetMaxAP();
					pDC->SetColor(MCOLOR(0x9000FF00));
					pDC->FillRectangleW((x * MGetWorkspaceWidth()), texty * MGetWorkspaceHeight() + 9, nValue * MGetWorkspaceWidth(), 3);
				}

				pDC->SetColor(tmpColor);

				x = ITEM_XPOS[3];
				int nKills = 0;
				ZModule_QuestStatus* pMod = (ZModule_QuestStatus*)pQuestPlayerInfo->GetModule(ZMID_QUESTSTATUS);
				//Check if the gametype is quest/survival
				if (pMod && ZGetGameTypeManager()->IsQuestDerived(ZGetGame()->GetMatch()->GetMatchType()))
					nKills = pMod->GetKills();
				else
					nKills = pQuestPlayerInfo->GetKils();
				sprintf(szText, "%d", nKills);
				TextRelative(pDC, x, texty, szText, true);
			}
		}
		else if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
		{
			/*ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.find(pItem->uidUID);
			if (itor != ZGetGame()->m_CharacterManager.end())*/
			{
				MCOLOR color = pDC->GetColor();
				x = ITEM_XPOS[2];
				//sprintf(szText, "%d", pItem->nRank);
				sprintf(szText, "1", pItem->nRank);
				TextRelative(pDC, x, texty, szText, true);

				pDC->SetColor(color);
			}

			MCOLOR color = pDC->GetColor();

			x = ITEM_XPOS[3];
			sprintf(szText, "%d", pItem->nKills * 43); // 13 Point for NPC
			TextRelative(pDC, x, texty, szText, true);

			pDC->SetColor(color);
		}
		else
		{
			x = ITEM_XPOS[2];
			sprintf(szText, "%d", pItem->nExp);
			//sprintf(szText, "%d", pItem->nKills * 43); 
			TextRelative(pDC, x, texty, szText, true);

			MCOLOR color = pDC->GetColor();

			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
			{
				if (!pItem->bDeath)
					pDC->SetColor(200, 0, 0);
				else
					pDC->SetColor(120, 0, 0);
			}

			x = ITEM_XPOS[3];
			sprintf(szText, "%d", pItem->nKills);
			TextRelative(pDC, x, texty, szText, true);

			pDC->SetColor(color);
		}
		if (bClanGame || bPlayerWars)
		{
			MCOLOR color = pDC->GetColor();

			x = ITEM_XPOS[0] - 0.066f;
			sprintf(szText, "%d", pItem->nKills * 2);
			TextRelative(pDC, x, texty, szText, true);
			pDC->SetColor(color);
		}
		else
		{
			//Nothing and of Nothing
		}
#ifdef _TAGVIP
		x = ITEM_XPOS[0] - 0.120f;
		TextRelative(pDC, x, texty, pItem->szGrade, true);
#endif
		if (!ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
		{
			x = ITEM_XPOS[4];
			sprintf(szText, "%d", pItem->nDeaths);
			TextRelative(pDC, x, texty, szText, true);
		}
#ifdef _MIPING
		int p = pItem->nPing;
		if (p != 999 && p > 0 && !pItem->bMyChar)
		{
			MiPing += p;
			total++;
		}
		if (pItem->uidUID == ZGetGameClient()->GetPlayerUID())
			pItem->nPing = ZGetGameClient()->MiPing;

		sprintf(szText, "%d", ((pItem->bMyChar && ZGetGameClient()->MiPing != NULL) ? ZGetGameClient()->MiPing : pItem->nPing));
#endif

#ifdef _PINGBAR
		// Ping bar.         
		x = ITEM_XPOS[5];
		int nIconSize = .8f * linespace * (float)MGetWorkspaceHeight();
		float icony = itemy + (linespace - (float)nIconSize / (float)MGetWorkspaceHeight()) * .5f;
		if (pItem->nPing == 0 || pItem->nPing == 999)
		{
			BitmapRelative(pDC, x, texty, nIconSize + 4, nIconSize, MBitmapManager::Get("AgentPing01.tga"));
		}
		else if (pItem->nPing >= 250)
		{
			BitmapRelative(pDC, x, texty, nIconSize + 4, nIconSize, MBitmapManager::Get("AgentPing03.tga"));
		}
		else if (pItem->nPing >= 1)
		{
			BitmapRelative(pDC, x, texty, nIconSize + 4, nIconSize, MBitmapManager::Get("AgentPing05.tga"));
		}
	}
#else

		x = ITEM_XPOS[5];

		char verynice[20];
		sprintf(verynice, "50.png");
		MBitmap* pBitmap4;
		pBitmap4 = MBitmapManager::Get(verynice);

		char nice[20];
		sprintf(nice, "150.png");
		MBitmap* pBitmap3;
		pBitmap3 = MBitmapManager::Get(nice);

		char medium[20];
		sprintf(medium, "250.png");
		MBitmap* pBitmap2;
		pBitmap2 = MBitmapManager::Get(medium);

		char bad[20];
		sprintf(bad, "300.png");
		MBitmap* pBitmap1;
		pBitmap1 = MBitmapManager::Get(bad);

		char fatal[20];
		sprintf(bad, "350.png");
		MBitmap* pBitmap0;
		pBitmap0 = MBitmapManager::Get(fatal);

		char dead[20];
		sprintf(dead, "999.png");
		MBitmap* pBitmap999;
		pBitmap999 = MBitmapManager::Get(dead);

		sprintf(szText, "%d", pItem->nPing);
		//MCOLOR pRealColor = pDC->GetColor();
		if (pBitmap != 0 && pItem->nPing > 0 && pItem->nPing < 50)
		{
			pDC->SetBitmap(pBitmap1);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(verynice));
		}
		else if (pBitmap != 0 && pItem->nPing > 50 && pItem->nPing < 150)
		{
			pDC->SetBitmap(pBitmap1);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(nice));
		}
		else if (pBitmap != 0 && pItem->nPing >= 150 && pItem->nPing < 250)
		{
			pDC->SetBitmap(pBitmap2);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(medium));
		}
		else if (pBitmap != 0 && pItem->nPing >= 250 && pItem->nPing < 300)
		{
			pDC->SetBitmap(pBitmap3);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(bad));
		}
		else if (pBitmap != 0 && pItem->nPing >= 300 && pItem->nPing < 350)
		{
			pDC->SetBitmap(pBitmap3);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(fatal));
		}
		else if (pBitmap != 0 && pItem->nPing >= 350 && pItem->nPing != MAX_PING)
		{
			pDC->SetBitmap(pBitmap3);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(dead));
		}
		else if (pBitmap != 0 && pItem->nPing == 999)
		{
			pDC->SetBitmap(pBitmap4);
			BitmapRelative(pDC, 0.f, 0.f, RGetScreenWidth(), RGetScreenHeight(), MBitmapManager::Get(dead));
		}
		TextRelative(pDC, x, texty, szText, true);
		//pDC->SetColor(pRealColor);
				//y+=linespace;
	}

#endif
#ifdef _MIPING
	if (total > 2)
		MiPing = (int)MiPing / total;
	ZGetGameClient()->MiPing = MiPing;
#endif
	while (!items.empty())
	{
		delete* items.begin();
		items.erase(items.begin());
	}
}
// ÆÀ / »ý?E/ ¼ºÀûÀÌ ¼ÒÆ®ÀÇ ±âÁØÀÌ´Ù
bool CompareZResultBoardItem(ZResultBoardItem* a,ZResultBoardItem* b) {
	if( a->nScore > b->nScore) return true;
	else if( a->nScore < b->nScore) return false;

	if( a->nKills > b->nKills) return true;
	else if( a->nKills < b->nKills) return false;

	if( a->nDeaths < b->nDeaths) return true;
	else if( a->nDeaths > b->nDeaths) return false;

	return false;
}

// Custom: spyresultinfo
void AddSpyResultInfo(const char* szName, int nScore,int nWinCount, int nLossCount, int bMyChar, bool bGameoomUser)
{
	char szText[256];
	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerNameList");
	if (pTextArea)
		pTextArea->AddText(szName, (bMyChar ? MCOLOR(0xFFFFF794) : MCOLOR(0xFFFFF794)));

	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerScoreList");
	if (pTextArea)
	{
		sprintf_s(szText, "%d", nScore);
		pTextArea->AddText(szText, MCOLOR(0xFFFFF794));
	}

	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerWinList");
	if (pTextArea)
	{
		sprintf_s(szText, "%d", nWinCount);
		pTextArea->AddText(szText, MCOLOR(0xFFFFF794));
	}

	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerLoseList");
	if (pTextArea)
	{
		sprintf_s(szText, "%d", nLossCount);
		pTextArea->AddText(szText, MCOLOR(0xFFFFF794));
	}
}

void AddCombatResultInfo(const char* szName, int nScore, int nKill, int nDeath, int bMyChar, bool bGameRoomUser)
{
	char szText[128];

	MTextArea* pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerNameList");
	if (pWidget)
		pWidget->AddText(szName, (bMyChar ? MCOLOR(0xFFFFF794) : MCOLOR(0xFFFFF794)));

	for (int i = 0; i < 16; i++)
	{
		char szWidget[128];
		sprintf(szWidget, "CombatResult_PlayerScore%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
		if (pLabel)
		{
			if (strcmp(pLabel->GetText(), "") == 0)
			{
				sprintf(szText, "%d", nScore);
				pLabel->SetText(szText);
				pLabel->SetAlignment(MAM_RIGHT);

				sprintf(szWidget, "CombatResult_GameRoomImg%02d", i);
				MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget(szWidget);
				if (pWidget)
					pWidget->Show(bGameRoomUser);

				break;
			}
		}
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerKillList");
	if (pWidget)
	{
		sprintf(szText, "%d", nKill);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CombatResult_PlayerDeathList");
	if (pWidget)
	{
		sprintf(szText, "%d", nDeath);
		pWidget->AddText(szText, MCOLOR(0xFFFFF794));
	}
}


void AddClanResultInfoWin( const char* szName, int nScore, int nKill, int nDeath, int bMyChar, bool bGameRoomUser)
{
	char szText[125];

	MTextArea* pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerNameList1");
	if ( pWidget)
		pWidget->AddText( szName, ( bMyChar ? MCOLOR( 0xFFFFF794) : MCOLOR( 0xFFFFF794)));

	for ( int i = 0;  i < 4;  i++)
	{
		char szWidget[ 128];
		sprintf( szWidget, "ClanResult_PlayerScore1%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pLabel)
		{
			if ( strcmp( pLabel->GetText(), "") == 0)
			{
				sprintf( szText, "%d", nScore);
				pLabel->SetText( szText);
				pLabel->SetAlignment( MAM_RIGHT);

				sprintf( szWidget, "ClanResult_GameRoomImg1%d", i);
				MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
				if ( pWidget)
					pWidget->Show( bGameRoomUser);

				break;
			}
		}
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerKillList1");
	if ( pWidget)
	{
		sprintf( szText, "%d", nKill);
		pWidget->AddText( szText, MCOLOR( 0xFFFFF794));
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerDeathList1");
	if ( pWidget)
	{
		sprintf( szText, "%d", nDeath);
		pWidget->AddText( szText, MCOLOR( 0xFFFFF794));
	}
}

void AddClanResultInfoLose( const char* szName, int nScore, int nKill, int nDeath, int bMyChar, bool bGameRoomUser)
{
	char szText[125];

	MTextArea* pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerNameList2");
	if ( pWidget)
		pWidget->AddText( szName, ( bMyChar ? MCOLOR( 0xFFFFF794) : MCOLOR( 0xFFFFF794)));

	for ( int i = 0;  i < 4;  i++)
	{
		char szWidget[ 128];
		sprintf( szWidget, "ClanResult_PlayerScore2%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pLabel)
		{
			if ( strcmp( pLabel->GetText(), "") == 0)
			{
				sprintf( szText, "%d", nScore);
				pLabel->SetText( szText);
				pLabel->SetAlignment( MAM_RIGHT);

				sprintf( szWidget, "ClanResult_GameRoomImg2%d", i);
				MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
				if ( pWidget)
					pWidget->Show( bGameRoomUser);

				break;
			}
		}
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerKillList2");
	if ( pWidget)
	{
		sprintf( szText, "%d", nKill);
		pWidget->AddText( szText, MCOLOR( 0xFFFFF794));
	}

	pWidget = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerDeathList2");
	if ( pWidget)
	{
		sprintf( szText, "%d", nDeath);
		pWidget->AddText( szText, MCOLOR( 0xFFFFF794));
	}
}

// °á?EÈ­¸éÀ» ±×¸°´Ù.
void ZCombatInterface::GetResultInfo( void)
{
	// Sort list
#ifdef _DEBUG
	// Custom: Don't show this in debug
	#ifdef TEST_SCOREBOARD
	m_ResultItems.push_back(new ZResultBoardItem("test01", "RED Clan",  MMT_RED,  ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test02", "RED Clan",  MMT_RED,  ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test03", "RED Clan",  MMT_RED,  ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test04", "RED Clan",  MMT_RED,  ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test05", "RED Clan",  MMT_RED,  ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test06", "RED Clan",  MMT_RED,  ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test07", "RED Clan",  MMT_RED,  ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test08", "BLUE Clan", MMT_BLUE, ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test09", "BLUE Clan", MMT_BLUE, ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test10", "BLUE Clan", MMT_BLUE, ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test11", "BLUE Clan", MMT_BLUE, ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test12", "BLUE Clan", MMT_BLUE, ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test13", "BLUE Clan", MMT_BLUE, ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	m_ResultItems.push_back(new ZResultBoardItem("test14", "BLUE Clan", MMT_BLUE, ((rand()%80000)-5000), (rand()%100), (rand()%100)));
	#endif
#endif
	m_ResultItems.sort( CompareZResultBoardItem);

	// Set UI
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult");
	if ( pWidget)  pWidget->Show( false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult");
	if ( pWidget)  pWidget->Show( false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult");
	if ( pWidget)  pWidget->Show( false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult");
	if ( pWidget)  pWidget->Show( false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "DuelTournamentResult");
	if ( pWidget)  pWidget->Show( false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ChallengeQuestResult");
	if (pWidget)   pWidget->Show(false);
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult");
	if (pWidget)   pWidget->Show(false);

	const int _H18 = CONVERT600(18);
	const int _H2  = CONVERT600(2);
	const int _H21 = CONVERT600(21);
	const int _W17 = CONVERT800(17);
	const int _W21 = CONVERT800(21);
	const int _W20 = CONVERT800(20);

	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_PlayerNameList");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	for ( int i = 0;  i < 4;  i++)
	{
		char szWidget[ 128];
		sprintf( szWidget, "CombatResult_PlayerScore%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pLabel)
		{
			MRECT rect;
			rect = pLabel->GetRect();
			rect.y = pTextArea->GetRect().y + _H18 * i - _H2;
			rect.h = _H21;
			pLabel->SetBounds( rect);

			pLabel->SetText( "");
			pLabel->SetAlignment( MAM_LEFT | MAM_TOP);
		}

		sprintf( szWidget, "CombatResult_GameRoomImg%02d", i);
		MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pWidget)
			pWidget->Show( false);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_PlayerKillList");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_PlayerDeathList");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerNameList");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerScoreList");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerWinList");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerLoseList");
	if (pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerNameList1");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	for ( int i = 0;  i < 4;  i++)
	{
		char szWidget[ 128];
		sprintf( szWidget, "ClanResult_PlayerScore1%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pLabel)
		{
			MRECT rect;
			rect = pLabel->GetRect();
			rect.y = pTextArea->GetRect().y + _H18 * i - _H2;
			rect.h = _H21;
			pLabel->SetBounds( rect);

			pLabel->SetText( "");
			pLabel->SetAlignment( MAM_LEFT | MAM_TOP);
		}

		sprintf( szWidget, "ClanResult_GameRoomImg1%d", i);
		MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pWidget)
			pWidget->Show( false);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerKillList1");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerDeathList1");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerNameList2");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	for ( int i = 0;  i < 4;  i++)
	{
		char szWidget[ 128];
		sprintf( szWidget, "ClanResult_PlayerScore2%d", i);
		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pLabel)
		{
			MRECT rect;
			rect = pLabel->GetRect();
			rect.y = pTextArea->GetRect().y + _H18 * i - _H2;
			rect.h = _H21;
			pLabel->SetBounds( rect);

			pLabel->SetText( "");
			pLabel->SetAlignment( MAM_LEFT | MAM_TOP);
		}

		sprintf( szWidget, "ClanResult_GameRoomImg2%d", i);
		MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szWidget);
		if ( pWidget)
			pWidget->Show( false);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerKillList2");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}
	pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerDeathList2");
	if ( pTextArea) {
		pTextArea->Clear();
		pTextArea->SetCustomLineHeight(_H18);
	}


	char szFileName[256];
	szFileName[0] = 0;

	// Set player info
	if ( ZGetGameTypeManager()->IsQuestOnly(ZGetGame()->GetMatch()->GetMatchType()))	// Äù½ºÆ®ÀÌ?E..
	{
		// ÃÊ?EUI ¼³Á¤
		strcpy( szFileName, "interface/loadable/rstbg_quest.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult");
		if ( pWidget)
			pWidget->Show( true);

		//  °æÇèÄ¡ ¹× ¹Ù¿ûâ¼ ÃÊ±âÈ­
		ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetPlusXP");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetMinusXP");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetTotalXP");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "QuestResult_GetBounty");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
	}

	else if (ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGame()->GetMatch()->GetMatchType()))
	{
		strcpy(szFileName, "interface/loadable/rstbg_challenge.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("ChallengeQuestResult");
		if (pWidget)
			pWidget->Show(true);
		ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_XP");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);

		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_BP");
		if (pBmNumLabel)
			pBmNumLabel->SetNumber(0, false);

		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("CQ_Result_Cleard");
		if (pBmNumLabel)
		{
			pBmNumLabel->SetNumber(0, false);
		}
	}

	// Custom: Spy GameResult
	else if (ZGetGameTypeManager()->IsSpyGame(ZGetGame()->GetMatch()->GetMatchType()))
	{
		strcpy(szFileName, "Interface/loadable/rstbg_deathmatch.jpg");

		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult");
		if (pWidget)
			pWidget->Show(true);

		MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_Finish");
		if (pPicture)
			pPicture->SetBitmap(MBitmapManager::Get("result_finish.tga"));

		pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_Header");
		if (pPicture)
			pPicture->SetBitmap(MBitmapManager::Get("blacknullframe.tga"));

		MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerNameListLabel");
		if (pLabel)
			pLabel->Show(true);

		pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerWinListLabel");
		if (pLabel)
			pLabel->Show(true);

		pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerScoreListLabel");
		if (pLabel)
			pLabel->Show(true);

		pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("SpyResult_PlayerLoseListLabel");
		if (pLabel)
			pLabel->Show(true);

		for (ZResultBoardList::iterator i = m_ResultItems.begin(); i != m_ResultItems.end();  i++)
		{
			if (i != m_ResultItems.end())
			{
				ZResultBoardItem *pItem = *i;

				AddSpyResultInfo(pItem->szName, pItem->nScore, pItem->nWins, pItem->nLosses, pItem->bMyChar, pItem->bGameRoomUser);
			}
		}
	}

	else if ( ZGetGameTypeManager()->IsSurvivalOnly(ZGetGame()->GetMatch()->GetMatchType()))	// ¼­¹ÙÀÌ¹úÀÌ?E..
	{
		// ÃÊ?EUI ¼³Á¤
		strcpy( szFileName, "interface/loadable/rstbg_survival.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult");
		if ( pWidget)
			pWidget->Show( true);

		//  °æÇèÄ¡ ¹× ¹Ù¿ûâ¼ ÃÊ±âÈ­
		ZBmNumLabel* pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetReachedRound");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetPoint");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetXP");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
		pBmNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "SurvivalResult_GetBounty");
		if ( pBmNumLabel)
			pBmNumLabel->SetNumber( 0, false);
	}

	else if ( ZGetGameClient()->IsCWChannel())		// Å¬·£?EÌ¸E..
	{
		// ÃÊ?EUI ¼³Á¤
		strcpy( szFileName, "interface/loadable/rstbg_clan.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult");
		if ( pWidget)
			pWidget->Show( true);

		// Get winner team
		int nWinnerTeam;
		if ( ZGetGame()->GetMatch()->GetTeamScore( MMT_RED) == ZGetGame()->GetMatch()->GetTeamScore( MMT_BLUE))  // draw 
		{
			MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_Win");
			if ( pPicture) 	pPicture->SetBitmap( MBitmapManager::Get( "result_draw.tga"));
			
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_Lose");
			if ( pPicture) 	pPicture->SetBitmap( MBitmapManager::Get( "result_draw.tga"));
		}
		else
		{
			MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_Win");
			if ( pPicture) 	pPicture->SetBitmap( MBitmapManager::Get( "result_win.tga"));

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_Lose");
			if ( pPicture) 	pPicture->SetBitmap( MBitmapManager::Get( "result_lose.tga"));
		}


		if ( ZGetGame()->GetMatch()->GetTeamScore( MMT_RED) > ZGetGame()->GetMatch()->GetTeamScore( MMT_BLUE))
			nWinnerTeam = MMT_RED;
		else
			nWinnerTeam = MMT_BLUE;

		for ( int i = 0;  i < 2;  i++) 
		{
			MMatchTeam nTeam = (i==0) ? MMT_RED : MMT_BLUE;
			char *szClanName = (i==0) ? m_szRedClanName : m_szBlueClanName;
			int nClanID = (i==0) ? m_nClanIDRed : m_nClanIDBlue;

			// Put clan mark
			MPicture* pPicture;
			if ( nWinnerTeam == nTeam)
				pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_ClanBitmap1");
			else
				pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_ClanBitmap2");
			if ( pPicture)
			{
				MBitmap* pBitmap = ZGetEmblemInterface()->GetClanEmblem2( nClanID);
				if ( pBitmap)
				{
					pPicture->SetBitmap( pBitmap);
					pPicture->Show( true);
				}
			}

			// Put label
			MLabel* pLabel;
			if ( nWinnerTeam == nTeam)
				pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerNameListLabel1");
			else
				pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "ClanResult_PlayerNameListLabel2");
			if ( pLabel)
			{
				pLabel->SetText( szClanName);
				pLabel->Show( true);
			}


			// °ÔÀÓ?EÇ¥½Ã Á¤·Ä
			int nStartX = 0;
			int nStartY = 0;
			char szName[ 256];
			sprintf( szName, "ClanResult_PlayerNameList%d", i+1);
			pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szName);
			if ( pWidget)
			{
				nStartX = pWidget->GetRect().x;
				nStartY = pWidget->GetRect().y;
			}

			for ( int j = 0;  j < 4;  j++)
			{
				char szName[ 256];
				sprintf( szName, "ClanResult_GameRoomImg%d%d", i+1, j);
				pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szName);
				if ( pWidget)
				{
					MRECT rect2;
					rect2.x = nStartX - _W17;
					rect2.y = _H18 * j + nStartY;
					rect2.w = _W21;
					rect2.h = _H18;

					pWidget->SetBounds( rect2);
				}
			}
		}


		// ÆÀ Á¤º¸ Ãß°¡
		for ( ZResultBoardList::iterator i = m_ResultItems.begin(); i != m_ResultItems.end();  i++)
		{
			ZResultBoardItem *pItem = *i;

			if ( (pItem->nTeam != MMT_RED) && (pItem->nTeam != MMT_BLUE))
				continue;

			// Put info
			if ( nWinnerTeam == pItem->nTeam)
				AddClanResultInfoWin( pItem->szName, pItem->nScore, pItem->nKills, pItem->nDeaths, pItem->bMyChar, pItem->bGameRoomUser);
			else
				AddClanResultInfoLose( pItem->szName, pItem->nScore, pItem->nKills, pItem->nDeaths, pItem->bMyChar, pItem->bGameRoomUser);
		}
	}
	else if(ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)	// µà¾óÅä³Ê¸ÕÆ®?E.
	{
		strcpy( szFileName, "Interface/loadable/rstbg_deathmatch.jpg");
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "DuelTournamentResult");
		if ( pWidget)
			pWidget->Show( true);

		// ?EÂÀ?Ä³¸¯ÅÍ ?Eº¸¿©ÁÖ?E
		ZCharacterView* pCharView = GetWidgetCharViewResult();
		if (pCharView) {
			MUID uidChampion = ((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->GetChampion();
			pCharView->SetCharacter( uidChampion);
		}
	}
	else
	{
		// ÃÊ?EUI ¼³Á¤
		if ( (ZGetLocale()->GetCountry() == MC_US) || (ZGetLocale()->GetCountry() == MC_BRAZIL) || (ZGetLocale()->GetCountry() == MC_INDIA))
		{
			// ÀÎÅÍ³»¼Å³Î ¹× ?EóÁE¹öÁ¯ ÇÑÁ¤ ¿É¼Ç
			if ( (rand() % 2))
				strcpy( szFileName, "interface/loadable/rstbg_deathmatch.jpg");
			else
				strcpy( szFileName, "interface/loadable/rstbg_clan.jpg");
		}
		else
			strcpy( szFileName, "interface/loadable/rstbg_deathmatch.jpg");

		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult");
		if ( pWidget)
			pWidget->Show( true);


		int nStartY = 0;
		pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_PlayerNameList");
		if ( pWidget)
			nStartY = pWidget->GetRect().y;


		ZResultBoardList::iterator itrList = m_ResultItems.begin();
		for ( int i = 0;  i < 16;  i++)
		{
			int nTeam = 0;

			if ( itrList != m_ResultItems.end())
			{
				ZResultBoardItem *pItem = *itrList;

				if ( (pItem->nTeam == MMT_RED) || (pItem->nTeam == MMT_BLUE) || (pItem->nTeam == 4))
					AddCombatResultInfo( pItem->szName, pItem->nScore, pItem->nKills, pItem->nDeaths, pItem->bMyChar, pItem->bGameRoomUser);

				nTeam = pItem->nTeam;
				itrList++;
			}


            for ( int j = MMT_RED;  j <= MMT_BLUE;  j++)
			{
				char szName[ 128];
				sprintf( szName, "CombatResult_%sTeamBG%02d", ((j==MMT_RED) ? "Red" : "Blue"), i);

				pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szName);
				if ( pWidget)
				{
					MRECT rect;
					rect = pWidget->GetRect();
					rect.y = _H18 * i + nStartY;
					rect.h = _H18;

					pWidget->SetBounds( rect);
		
					if ( nTeam == j)
						pWidget->Show( true);
					else
						pWidget->Show( false);

					pWidget->SetOpacity( 110);


					// Á» ¾ÈÁÁÀº À§Ä¡Áö¸¸...  -_-;
					sprintf( szName, "CombatResult_GameRoomImg%02d", i);
					pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( szName);
					if ( pWidget)
					{
						MRECT rect2;
						rect2 = pWidget->GetRect();
						rect2.x = rect.x - _W20;
						rect2.y = _H18 * i + nStartY;
						rect2.w = _W21;
						rect2.h = _H18;

						pWidget->SetBounds( rect2);
					}
				}
			}
		}


		// ÀÌ¹Ì?E¼³Á¤
		MPicture* pPicture;
		// Custom: CTF
		if ( ZGetGameTypeManager()->IsTeamExtremeGame(ZGetGame()->GetMatch()->GetMatchType()) )		// À¸¾Æ¾Æ¾Æ¾Ç
		{
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_Finish");
			if ( pPicture)
				pPicture->Show( false);

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_WinLoseDraw");
			if ( pPicture)
			{
				if ( ZGetGame()->GetMatch()->GetTeamKills( MMT_RED) == ZGetGame()->GetMatch()->GetTeamKills( MMT_BLUE))
					pPicture->SetBitmap( MBitmapManager::Get( "result_draw.tga"));
				else
				{
					if ( ZGetGame()->GetMatch()->GetTeamKills( MMT_RED) > ZGetGame()->GetMatch()->GetTeamKills( MMT_BLUE))
					{
						if ( ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED)
							pPicture->SetBitmap( MBitmapManager::Get( "result_win.tga"));
						else
							pPicture->SetBitmap( MBitmapManager::Get( "result_lose.tga"));
					}
					else
					{
						if ( ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_BLUE)
							pPicture->SetBitmap( MBitmapManager::Get( "result_win.tga"));
						else
							pPicture->SetBitmap( MBitmapManager::Get( "result_lose.tga"));
					}
				}

				pPicture->Show( true);
			}
		}
		else if (ZGetGameInterface()->m_bTeamPlay && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_SPY)		// ÆÀ?EÌ¸E..
		{
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_Finish");
			if ( pPicture)
				pPicture->Show( false);

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_WinLoseDraw");
			if ( pPicture)
			{
				if ( ZGetGame()->GetMatch()->GetTeamScore( MMT_RED) == ZGetGame()->GetMatch()->GetTeamScore( MMT_BLUE))
					pPicture->SetBitmap( MBitmapManager::Get( "result_draw.tga"));
				else
				{
					if ( ZGetGame()->GetMatch()->GetTeamScore( MMT_RED) > ZGetGame()->GetMatch()->GetTeamScore( MMT_BLUE))
					{
						if ( ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED)
							pPicture->SetBitmap( MBitmapManager::Get( "result_win.tga"));
						else
							pPicture->SetBitmap( MBitmapManager::Get( "result_lose.tga"));
					}
					else
					{
						if ( ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_BLUE)
							pPicture->SetBitmap( MBitmapManager::Get( "result_win.tga"));
						else
							pPicture->SetBitmap( MBitmapManager::Get( "result_lose.tga"));
					}
				}

				pPicture->Show( true);
			}
		}
		else										// °³ÀÎ?EÌ¸E..
		{
			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_Finish");
			if ( pPicture)
				pPicture->Show( true);

			pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "CombatResult_WinLoseDraw");
			if ( pPicture)
				pPicture->Show( false);
		}
	}


	// ¹è°æÀÌ¹Ì?E·Î?E
	m_pResultBgImg = new MBitmapR2;
	((MBitmapR2*)m_pResultBgImg)->Create( "resultbackground.png", RGetDevice(), szFileName);
	if ( m_pResultBgImg != NULL)
	{
		// ÀÐ¾ûÛÂ ºñÆ®¸Ê ÀÌ¹Ì?EÆ÷ÀÎÅÍ¸¦ ÇØ?EÀ§Á¬¿¡ ³Ñ°ÜÁà¼­ Ç¥½ÃÇÑ´Ù
		MPicture* pBgImage = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "GameResult_Background");
		if ( pBgImage)
			pBgImage->SetBitmap( m_pResultBgImg->GetSourceBitmap());
	}
}

void ZCombatInterface::DrawResultBoard(MDrawContext* pDC) // Custom: Result 1
{
	////////////////////////////////////////////////////////////////////////
	// ÀÌÇÔ¼ö´Â È£ÃâÇÏ´Â °÷µµ ¾ø´Ù. ¹ö¸®´Â ÇÔ¼öÀÎ°Í °°´Ù. ½ÇÁ¦·Î´Â GetResultInfo()¿¡¼­ ±×¸®´Â µí.
	////////////////////////////////////////////////////////////////////////

		// Å¬·£ÀüÀÏ¶§ ³» ÆÀÀ» ¿ÞÆí¿¡, »ó´ëÆÀÀ» ¿À¸¥ÂÊ¿¡ º¸¿©ÁØ´Ù

	bool bClanGame = ZGetGameClient()->IsCWChannel();
	//	bool bClanGame = true;

	if (!m_pResultPanel) return;

	m_pResultPanel->Draw(0);

	if (m_pResultPanel_Team)
		m_pResultPanel_Team->Draw(0);

	/*
	if(m_pResultPanel->GetVMesh()->isOncePlayDone())
	{
		SAFE_DELETE(m_pResultPanel);
		return;
	}
	*/

#define FADE_START_FRAME	20000

	RVisualMesh* pvm = m_pResultPanel->GetVMesh();
	if (bClanGame && pvm->isOncePlayDone())
	{
		if (!m_pResultLeft)
		{
			char* szEffectNames[] = { "clan_win", "clan_draw", "clan_lose" };

			int nRed = ZGetGame()->GetMatch()->GetTeamScore(MMT_RED);
			int nBlue = ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE);
			int nLeft, nRight;

			if (ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED) {
				nLeft = (nRed == nBlue) ? 1 : (nRed > nBlue) ? 0 : 2;
			}
			else
				nLeft = (nRed == nBlue) ? 1 : (nRed < nBlue) ? 0 : 2;

			// ¿À¸¥ÂÊÀº ¿ÞÂÊÀÇ ¹Ý´ë
			nRight = 2 - nLeft;

			m_pResultLeft = ZGetScreenEffectManager()->CreateScreenEffect(szEffectNames[nLeft],
				rvector(-240.f, -267.f, 0));
			m_pResultRight = ZGetScreenEffectManager()->CreateScreenEffect(szEffectNames[nRight],
				rvector(240.f, -267.f, 0));
		}

		m_pResultLeft->Draw(0);
		m_pResultRight->Draw(0);
	}

	int nFrame = pvm->GetFrameInfo(ani_mode_lower)->m_nFrame;

	float fOpacity = min(1.f, max(0, float(nFrame - FADE_START_FRAME)
		/ float(pvm->GetFrameInfo(ani_mode_lower)->m_pAniSet->GetMaxFrame() - FADE_START_FRAME)));

	MFont* pFont = GetGameFont();
	pDC->SetFont(pFont);
	pFont = pDC->GetFont();	// ¸¸¾à ÆùÆ®°¡ ¾øÀ¸¸é ´Ù½Ã µðÆúÆ® ÆùÆ®¸¦ ¾ò´Â´Ù

	MCOLOR opacity = MCOLOR(0, 0, 0, 255 * fOpacity);
	pDC->SetOpacity(255 * fOpacity);

	float x, y;

	char szText[256];

	x = 0.026f;
	y = 0.107f;

	const float fHeight = 0.651f;	// °ø°£ÀÇ ³ôÀÌ

	// ±×¸±¼öÀÖ´Â ÃÖ´ëÁÙ¼ö ÁÙ°£°ÝÀº 150%
//	int nMaxLineCount=int(fHeight*RGetScreenHeight()/((float)pFont->GetHeight()*1.5f));
	int nMaxLineCount = 16;

	// ÇÑÁÙ»çÀÌÀÇ °£°Ý(³ôÀÌ)
	float linespace = fHeight / (float)nMaxLineCount;

	m_ResultItems.sort(CompareZResultBoardItem);

	/*
	m_ResultItems.clear();
	g_pGame->m_pMyCharacter->SetTeamID(MMT_RED);
	m_ResultItems.push_back(new ZResultBoardItem("test1","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test2","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test3","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test4","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test5","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test6","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test7","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test8","Áö¿ÁÀÇ¹ßÂ÷±â",MMT_RED,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test1","´ë·«³¶ÆÐ",MMT_BLUE,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test2","´ë·«³¶ÆÐ",MMT_BLUE,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test3","´ë·«³¶ÆÐ",MMT_BLUE,0,0,0));
	m_ResultItems.push_back(new ZResultBoardItem("test4","´ë·«³¶ÆÐ",MMT_BLUE,0,0,0,true));
	*/

	if (bClanGame)
	{
		int nLeft = 0;
		int nRight = 0;

		y = 0.387f;

		// TODO : Å¬·£ ÀÌ¸§¹× emblem Ãâ·ÂÀº stagesetting È¤Àº matchÂÊ¿¡ Á¤º¸°¡ Ãß°¡µÇ´Â´ë·Î ¼öÁ¤
		for (ZResultBoardList::iterator i = m_ResultItems.begin(); i != m_ResultItems.end(); i++)
		{
			ZResultBoardItem* pItem = *i;

			int y1, y2;
			float itemy;

			float clancenter;
			bool bDrawClanName = false;

			MCOLOR backgroundcolor;

			if (pItem->nTeam == ZGetGame()->m_pMyCharacter->GetTeamID()) {
				x = 0.035f;
				itemy = y + linespace * nLeft;
				nLeft++;
				if (nLeft == 1)
				{
					bDrawClanName = true;
					clancenter = 0.25f;
				}
				backgroundcolor = (nLeft % 2 == 0) ? BACKGROUND_COLOR1 : BACKGROUND_COLOR2;
				y1 = itemy * MGetWorkspaceHeight();
				y2 = (y + linespace * nLeft) * MGetWorkspaceHeight();
			}
			else {
				x = 0.55f;
				itemy = y + linespace * nRight;
				nRight++;
				if (nRight == 1)
				{
					bDrawClanName = true;
					clancenter = 0.75f;
				}
				backgroundcolor = (nRight % 2 == 1) ? BACKGROUND_COLOR1 : BACKGROUND_COLOR2;
				y1 = itemy * MGetWorkspaceHeight();
				y2 = (y + linespace * nRight) * MGetWorkspaceHeight();
			}

			if (bDrawClanName)
			{
				MCOLOR textcolor = TEXT_COLOR_CLAN_NAME;
				textcolor.a = opacity.a;
				pDC->SetColor(textcolor);

				MFont* pClanFont = MFontManager::Get("FONTb11b");
				pDC->SetFont(pClanFont);

				float clanx = clancenter - .5f * (float)pClanFont->GetWidth(pItem->szClan) / (float)MGetWorkspaceWidth();
				TextRelative(pDC, clanx, 0.15, pItem->szClan);

				char szText[32];
				sprintf(szText, "%d", ZGetGame()->GetMatch()->GetTeamScore((MMatchTeam)pItem->nTeam));

				clanx = clancenter - .5f * (float)pClanFont->GetWidth(szText) / (float)MGetWorkspaceWidth();
				TextRelative(pDC, clanx, 0.2, szText);

				// Ä®·³Ç¥½Ã
				textcolor = TEXT_COLOR_TITLE;
				textcolor.a = opacity.a;
				pDC->SetColor(textcolor);
				float texty = itemy - linespace + (linespace - (float)pFont->GetHeight() / (float)RGetScreenHeight()) * .5f;
				TextRelative(pDC, x, texty, "Level Name");
				TextRelative(pDC, x + .25f, texty, "Exp", true);
				TextRelative(pDC, x + .32f, texty, "Kill", true);
				TextRelative(pDC, x + .39f, texty, "Death", true);

			}

			if (pItem->bMyChar)
				backgroundcolor = BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;
			backgroundcolor.a = opacity.a >> 1;
			pDC->SetColor(backgroundcolor);
			pDC->FillRectangleW(
				(x - .01f) * MGetWorkspaceWidth(), y1,
				.44f * MGetWorkspaceWidth(), y2 - y1);

			MCOLOR textcolor = TEXT_COLOR_DEATH_MATCH;
			textcolor.a = opacity.a;
			pDC->SetColor(textcolor);
			pDC->SetFont(pFont);

			// ±ÛÀÚ¸¦ °¡¿îµ¥ Á¤·ÄÇÏ±â À§ÇØ ..
			float texty = itemy + (linespace - (float)pFont->GetHeight() / (float)RGetScreenHeight()) * .5f;
			TextRelative(pDC, x, texty, pItem->szName);

			sprintf(szText, "%d", pItem->nScore);
			TextRelative(pDC, x + .25f, texty, szText, true);

			sprintf(szText, "%d", pItem->nKills);
			TextRelative(pDC, x + .32f, texty, szText, true);

			sprintf(szText, "%d", pItem->nDeaths);
			TextRelative(pDC, x + .39f, texty, szText, true);
		}
	}
	else
	{
		//	int backgroundy=y*MGetWorkspaceHeight();
		int nCount = 0;

		for (ZResultBoardList::iterator i = m_ResultItems.begin(); i != m_ResultItems.end(); i++)
		{
			ZResultBoardItem* pItem = *i;

			float itemy = y + linespace * nCount;

			nCount++;

			/*
			// ¸¶Áö¸·ÁÙÀÌ¸é ... À»Âï°í ³Ñ¾î°£´Ù
			if(nCount==nMaxLineCount)
			{
			pDC->SetColor(MCOLOR(255,255,255,opacity.a));
			x=0.50f;
			TextRelative(pDC,x,y,".....");
			break;
			}
			*/

			// ¹è°æ »ö±òÀ» °áÁ¤ÇÑ´Ù
			MCOLOR backgroundcolor = (nCount % 2 == 0) ? BACKGROUND_COLOR1 : BACKGROUND_COLOR2;
			if (pItem->bMyChar) backgroundcolor =
				(pItem->nTeam == MMT_RED) ? BACKGROUND_COLOR_MYCHAR_RED_TEAM :
				(pItem->nTeam == MMT_BLUE) ? BACKGROUND_COLOR_MYCHAR_BLUE_TEAM :
				BACKGROUND_COLOR_MYCHAR_DEATH_MATCH;

			backgroundcolor.a = opacity.a >> 1;
			pDC->SetColor(backgroundcolor);

			int y1 = itemy * MGetWorkspaceHeight();
			int y2 = (y + linespace * nCount) * MGetWorkspaceHeight();

			pDC->FillRectangleW(
				0.022f * MGetWorkspaceWidth(), y1,
				0.960 * MGetWorkspaceWidth(), y2 - y1);
			//		backgroundy=newbackgroundy;

			// ±ÛÀÚ »ö±òÀ» °áÁ¤ÇÑ´Ù.. 
			MCOLOR textcolor = TEXT_COLOR_DEATH_MATCH;

			if (pItem->nTeam == MMT_RED)		// red
				textcolor = TEXT_COLOR_RED_TEAM;
			else
				if (pItem->nTeam == MMT_BLUE)		// blue
					textcolor = TEXT_COLOR_BLUE_TEAM;
				else
					if (pItem->nTeam == MMT_SPECTATOR)
						textcolor = TEXT_COLOR_SPECTATOR;

			textcolor.a = opacity.a;
			pDC->SetColor(textcolor);

			// ±ÛÀÚ¸¦ °¡¿îµ¥ Á¤·ÄÇÏ±â À§ÇØ ..
			float texty = itemy + (linespace - (float)pFont->GetHeight() / (float)RGetScreenHeight()) * .5f;

			x = 0.025f;
			TextRelative(pDC, x, texty, pItem->szName);

			x = 0.43f;
			sprintf(szText, "%d", pItem->nScore);
			TextRelative(pDC, x, texty, szText, true);

			x = 0.52f;
			sprintf(szText, "%d", pItem->nKills);
			TextRelative(pDC, x, texty, szText, true);

			x = 0.61f;
			sprintf(szText, "%d", pItem->nDeaths);
			TextRelative(pDC, x, texty, szText, true);

			const float iconspace = 0.053f;

			x = 0.705f;

			pDC->SetBitmapColor(MCOLOR(255, 255, 255, 255 * fOpacity));

			IconRelative(pDC, x, texty, 0); x += iconspace;
			IconRelative(pDC, x, texty, 1); x += iconspace;
			IconRelative(pDC, x, texty, 2); x += iconspace;
			IconRelative(pDC, x, texty, 3); x += iconspace;
			IconRelative(pDC, x, texty, 4);

			pDC->SetBitmapColor(MCOLOR(255, 255, 255, 255));

			x = 0.705f + (float(pFont->GetHeight() * 1.3f) / MGetWorkspaceWidth());
			sprintf(szText, "%d", pItem->nAllKill);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nUnbelievable);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nExcellent);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nFantastic);
			TextRelative(pDC, x, texty, szText); x += iconspace;
			sprintf(szText, "%d", pItem->nHeadShot);
			TextRelative(pDC, x, texty, szText); x += iconspace;

			//		y+=linespace;
		}
	}
}

void ZCombatInterface::IconRelative(MDrawContext* pDC,float x,float y,int nIcon)
{
	MBitmap *pbmp=m_ppIcons[nIcon];
	if(!pbmp) return;

	pDC->SetBitmap(pbmp);
	int screenx=x*MGetWorkspaceWidth();
	int screeny=y*MGetWorkspaceHeight();

	int nSize=pDC->GetFont()->GetHeight();
	pDC->Draw(screenx,screeny,nSize,nSize);
}

void ZCombatInterface::Finish()
{	
	if ( IsFinish())
		return;

	ZGetFlashBangEffect()->End();

	m_fOrgMusicVolume = ZApplication::GetSoundEngine()->GetMusicVolume();
	m_nReserveFinishTime = timeGetTime();
	m_bReserveFinish = true;

	m_CrossHair.Show(false);

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->Set3DSoundUpdate( false );
#endif

}

bool ZCombatInterface::IsFinish()
{
//	if(m_pResultPanel)
//		return m_pResultPanel->GetVMesh()->isOncePlayDone();
//	return false;

	return m_bOnFinish;
}

void ZCombatInterface::OnFinish()
{
	if(m_pResultPanel) return;

	m_pResultLeft = NULL;
	m_pResultRight = NULL;

	ZGetScreenEffectManager()->AddRoundFinish();
	
//	m_pResultPanel=ZGetScreenEffectManager()->CreateScreenEffect("ef_in_result.elu");

	if (ZGetGame()->GetMatch()->IsTeamPlay() && !ZGetGameClient()->IsCWChannel() && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_SPY)
	{
		int nRed = ZGetGame()->GetMatch()->GetTeamScore(MMT_RED), nBlue = ZGetGame()->GetMatch()->GetTeamScore(MMT_BLUE);
		if(nRed==nBlue)
			m_pResultPanel_Team = ZGetScreenEffectManager()->CreateScreenEffect("teamdraw");
		else
			if(nRed>nBlue)
				m_pResultPanel_Team = ZGetScreenEffectManager()->CreateScreenEffect("teamredwin");
			else
				m_pResultPanel_Team = ZGetScreenEffectManager()->CreateScreenEffect("teambluewin");
	}

	m_ResultItems.Destroy();

	ZCharacterManager::iterator itor;
	for (itor = ZGetCharacterManager()->begin();
		itor != ZGetCharacterManager()->end(); ++itor)
	{
		ZCharacter* pCharacter = (ZCharacter*) (*itor).second;
		ZResultBoardItem *pItem=new ZResultBoardItem;

		if(pCharacter->IsAdminHide()) continue;

		if(pCharacter->IsAdminName()) 
		{
			// Custom: Unmask names
			//sprintf(pItem->szName,"%d%s %s",pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName,"--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}
#ifdef _VIPGRADES
		else if (pCharacter->IsVIP1Name()) 
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		else if (pCharacter->IsVIP2Name()) 
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		else if (pCharacter->IsVIP3Name()) 
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}
		else if (pCharacter->IsVIP4Name())
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		else if (pCharacter->IsVIP5Name())
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		else if (pCharacter->IsVIP6Name())
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}
#endif

#ifdef _EVENTGRD
		else if (pCharacter->IsEvent1Name())
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		else if (pCharacter->IsEvent2Name())
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		else if (pCharacter->IsEvent3Name())
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}
		else if (pCharacter->IsEvent4Name())
		{
			//sprintf(pItem->szName, "--%s  %s", ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
			sprintf(pItem->szName, "%d%s %s", pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}
#endif

		else 
		{
			sprintf(pItem->szName,"%d%s %s",pCharacter->GetProperty()->nLevel, ZMsg(MSG_CHARINFO_LEVELMARKER), pCharacter->GetUserName());
		}

		strcpy(pItem->szClan,pCharacter->GetProperty()->GetClanName());
		pItem->nClanID = pCharacter->GetClanID();
		pItem->nTeam = ZGetGame()->GetMatch()->IsTeamPlay() ? pCharacter->GetTeamID() : MMT_END;
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
			pItem->nScore = pCharacter->GetStatus().Ref().nScore;
		else
			pItem->nScore = pCharacter->GetStatus().Ref().nExp;
		pItem->nKills = pCharacter->GetStatus().Ref().nKills;
		pItem->nDeaths = pCharacter->GetStatus().Ref().nDeaths;
		pItem->nWins = pCharacter->GetStatus().Ref().nWinCount;
		pItem->nLosses = pCharacter->GetStatus().Ref().nLossCount;

		pItem->nAllKill= pCharacter->GetStatus().Ref().nAllKill;
		pItem->nExcellent = pCharacter->GetStatus().Ref().nExcellent;
		pItem->nFantastic = pCharacter->GetStatus().Ref().nFantastic;
		pItem->nHeadShot = pCharacter->GetStatus().Ref().nHeadShot;
		pItem->nUnbelievable = pCharacter->GetStatus().Ref().nUnbelievable;

		pItem->bMyChar = pCharacter->IsHero();
	
		MMatchObjCache* pCache = ZGetGameClient()->FindObjCache( pCharacter->GetUID());
		if ( pCache)
			pItem->bGameRoomUser = (pCache->GetPGrade() == MMPG_PREMIUM_IP) ? true : false;
		else
			pItem->bGameRoomUser = false;

		m_ResultItems.push_back(pItem);
	}

	m_Observer.Show(false);

	m_nReservedOutTime = timeGetTime() + 5000;		// 5ÃÊ ÈÄ¿¡ ÀÚµ¿ Á¾?E
	m_bOnFinish = true;
}

void ZCombatInterface::SetObserverMode(bool bEnable)
{
	if (bEnable) ZGetScreenEffectManager()->ResetSpectator();
	m_Observer.Show(bEnable);
}


ZCharacter* ZCombatInterface::GetTargetCharacter()
{
	if (m_Observer.IsVisible())
	{
		return m_Observer.GetTargetCharacter();
	}

	return ZGetGame()->m_pMyCharacter;	
}

MUID ZCombatInterface::GetTargetUID()
{
	return GetTargetCharacter()->GetUID();
}


void ZCombatInterface::GameCheckPickCharacter()
{
	MPOINT Cp = GetCrosshairPoint();

	ZPICKINFO pickinfo;

	rvector pos,dir;
	RGetScreenLine(Cp.x,Cp.y,&pos,&dir);

	ZMyCharacter* pMyChar = NULL;

	pMyChar = ZGetGame()->m_pMyCharacter;

	bool bPick = false;

	if(ZGetGame()->Pick(pMyChar,pos,dir,&pickinfo,RM_FLAG_ADDITIVE | RM_FLAG_HIDE,true)) {
		
		if(pickinfo.pObject)	{
			if (pickinfo.info.parts == eq_parts_head) bPick=true;
			bPick = true;
		}
	}

	if(pMyChar && pMyChar->m_pVMesh) 
	{
		
		rvector vRot = pMyChar->m_pVMesh->GetRotXYZ();

		RWeaponMotionType type = (RWeaponMotionType)pMyChar->m_pVMesh->GetSetectedWeaponMotionID();

		if ((type == eq_wd_katana) || (type == eq_wd_grenade) || (type == eq_ws_dagger) || (type == eq_wd_dagger)
			|| (type == eq_wd_item) || (type == eq_wd_sword) || (type == eq_wd_blade) || (type == eq_wd_spycase)) {
			bPick = false;
		}

		if(vRot.y  > -20.f &&  vRot.y < 30.f) {
			bPick = false;
		}

		if(vRot.y < -25.f)
			bPick = true;

		if( pMyChar->IsMan() ) { // ¸ðµ¨ÀÌ ³²ÀÚ?E
			if( vRot.x < -20.f) {//¿À¸¥ÂÊÀ¸·Î ÀÌµ¿Áß¿¡
				if( RCameraDirection.z < -0.2f)
					bPick = true;
			}
		}

		if( ( pMyChar->m_AniState_Lower.Ref() == ZC_STATE_LOWER_TUMBLE_RIGHT) || 
			( pMyChar->m_AniState_Lower.Ref() == ZC_STATE_LOWER_TUMBLE_LEFT) )
		{		
			if( RCameraDirection.z < -0.3f)
				bPick = true;
		}

		if (RCameraDirection.z < -0.6f)
			bPick = true;

		float fVisibility = bPick ? 0.4f : 1.0f;
		pMyChar->m_pVMesh->SetVisibility(fVisibility);

#ifdef _CHARACTERVISIBLE
		{
			if (bPick) {
				pMyChar->m_pVMesh->SetVisibility(1.0f);
			}
			else {
				pMyChar->m_pVMesh->SetVisibility(1.0f);
			}
		}
#else
		{
			if (bPick)
			{
				pMyChar->m_pVMesh->SetVisibility(0.4f);
			}
			else
			{
				pMyChar->m_pVMesh->SetVisibility(1.0f);
			}
		}
#endif
  }
}

void ZCombatInterface::OnGadget(MMatchWeaponType nWeaponType)
{
	if (m_pWeaponScreenEffect) m_pWeaponScreenEffect->OnGadget(nWeaponType);
	m_CrossHair.Show(false);
}

void ZCombatInterface::OnGadgetOff()
{
	if (m_pWeaponScreenEffect) m_pWeaponScreenEffect->OnGadgetOff();
	m_CrossHair.Show(true);
}


void ZCombatInterface::SetDrawLeaveBattle(bool bShow, int nSeconds)
{
	m_bDrawLeaveBattle = bShow;
	m_nDrawLeaveBattleSeconds = nSeconds;
}

void ZCombatInterface::OnAddCharacter(ZCharacter *pChar)
{
	bool bClanGame = ZGetGameClient()->IsCWChannel();
	if(bClanGame) {
		if (pChar->GetTeamID() == MMT_RED) {
			if(m_nClanIDRed==0) {
				m_nClanIDRed = pChar->GetClanID();
				ZGetEmblemInterface()->AddClanInfo(m_nClanIDRed);
				strcpy(m_szRedClanName,pChar->GetProperty()->GetClanName());
			}
		}
		else if (pChar->GetTeamID() == MMT_BLUE) {
			if(m_nClanIDBlue==0) {
				m_nClanIDBlue = pChar->GetClanID();
				ZGetEmblemInterface()->AddClanInfo(m_nClanIDBlue);
				strcpy(m_szBlueClanName,pChar->GetProperty()->GetClanName());
			}
		}
	}
}

void ZCombatInterface::ShowChatOutput(bool bShow)
{
	m_Chat.ShowOutput(bShow);
	ZGetConfiguration()->SetViewGameChat(bShow);
}

void ZCombatInterface::DrawAfterWidgets( MDrawContext* pDC )
{
	// µà¾óÅä³Ê¸ÕÆ® °á°úÃ¢?E?EøÇ¥¸?±×¸°´Ù.
	if(m_bShowResult)
		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
			((ZRuleDuelTournament*)ZGetGame()->GetMatch()->GetRule())->ShowMatchOrder(pDC, true, m_fElapsed);
}

void ZCombatInterface::UpdateCTFMsg( const char* szMsg )
{
	g_bShowCTFMsg = true;
	g_dwCTFMsgShowTime = GetTickCount();
	strcpy_s( g_szCTFMsg, szMsg );
}

void ZCombatInterface::UpdateRTDMsg( const char* szMsg )
{
	g_bShowRTDMsg = true;
	g_dwRTDMsgShowTime = GetTickCount();
	strcpy_s( g_szRTDMsg, szMsg );
}

void ZCombatInterface::OnInvalidate()
{
	ZCharacterView* pCharView = GetWidgetCharViewLeft();
	if (pCharView)
		pCharView->OnInvalidate();
	pCharView = GetWidgetCharViewRight();
	if (pCharView)
		pCharView->OnInvalidate();
	pCharView = GetWidgetCharViewResult();
	if (pCharView)
		pCharView->OnInvalidate();
}

void ZCombatInterface::OnRestore()
{
	ZCharacterView* pCharView = GetWidgetCharViewLeft();
	if (pCharView)
		pCharView->OnRestore();
	pCharView = GetWidgetCharViewRight();
	if (pCharView)
		pCharView->OnRestore();
	pCharView = GetWidgetCharViewResult();
	if (pCharView)
		pCharView->OnRestore();
}
////////// Spy mode ////////// 
void ZCombatInterface::OnSpyCreate()
{
	CreateSpyWaitInterface();
}

void ZCombatInterface::OnSpyDestroy()
{
	ZIDLResource* pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	MFrame* pFrame = (MFrame*)pIDLResource->FindWidget("CombatSpyInfo");
	if (pFrame)
		pFrame->Show(false);

	ZBmNumLabel* pNumLabel = (ZBmNumLabel*)pIDLResource->FindWidget("Spy_CountDownTime");
	if (pNumLabel)
		pNumLabel->Show(false);

	pNumLabel = (ZBmNumLabel*)pIDLResource->FindWidget("Spy_CountDownTime_Red");
	if (pNumLabel)
		pNumLabel->Show(false);

	SetSpyEventMsg(NULL);
	SetSpyTip(NULL);
	SetSpyNotice(NULL);
}

void ZCombatInterface::CreateSpyGameInterface()
{
	ZIDLResource* pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	MFrame* pFrame = (MFrame*)pIDLResource->FindWidget("CombatSpyInfo");
	if (pFrame)
		pFrame->Show(true);

	ZBmNumLabel* pNumLabel = (ZBmNumLabel*)pIDLResource->FindWidget("Spy_CountDownTime");
	if (pNumLabel)
		pNumLabel->Show(true);

	pNumLabel = (ZBmNumLabel*)pIDLResource->FindWidget("Spy_CountDownTime_Red");
	if (pNumLabel)
		pNumLabel->Show(false);

	SetSpyEventMsg(NULL);
	SetSpyTip(NULL);
	SetSpyNotice(NULL);

	m_bSpyLocationOpened = false;
}

void ZCombatInterface::CreateSpyWaitInterface()
{
	ZIDLResource* pIDLResource = ZApplication::GetGameInterface()->GetIDLResource();

	MFrame* pFrame = (MFrame*)pIDLResource->FindWidget("CombatSpyInfo");
	if (pFrame)
		pFrame->Show(true);

	ZBmNumLabel* pNumLabel = (ZBmNumLabel*)pIDLResource->FindWidget("Spy_CountDownTime");
	if (pNumLabel)
		pNumLabel->Show(false);

	pNumLabel = (ZBmNumLabel*)pIDLResource->FindWidget("Spy_CountDownTime_Red");
	if (pNumLabel)
		pNumLabel->Show(false);

	SetSpyEventMsg(NULL);

	char minPlayers[32];
	sprintf(minPlayers, "%d", MMatchSpyMode::GetMinPlayers());

	char tip[256];
	ZTransMsg(tip, MSG_SPY_WAITING_FOR_PLAYERS, 1, minPlayers);

	SetSpyTip(tip);

	SetSpyNotice("Spy_Notice_Wait_OtherPlayer.png");

	m_bSpyLocationOpened = false;
}

void ZCombatInterface::OnSpyUpdate(float fElapsed)
{
	DWORD dwNowTime = timeGetTime();

	int nRemainedTime = (int)(m_dwSpyTimer - dwNowTime);
	SetSpyTimeLimitValue((DWORD)((nRemainedTime < 0) ? (0) : (nRemainedTime)));

	if (ZGetGame())
	{
		if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
		{
			const MMatchSpyMapNode* pSpyMap = MMatchSpyMap::GetMap(ZGetGame()->GetMatch()->GetMapName());
			_ASSERT(pSpyMap != NULL);

			if (((nRemainedTime / 1000) <= pSpyMap->nSpyOpenTime))
			{
#define SPYTIME_FLASHING_INTERVAL	300

				bool bRedTime = ((nRemainedTime % (SPYTIME_FLASHING_INTERVAL * 2)) < SPYTIME_FLASHING_INTERVAL);

				ZBmNumLabel* pNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_CountDownTime");
				if (pNumLabel)
					pNumLabel->Show(!bRedTime);

				pNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_CountDownTime_Red");
				if (pNumLabel)
					pNumLabel->Show(bRedTime);

				if (!m_bSpyLocationOpened)
				{
					if (ZGetScreenEffectManager())
						ZGetScreenEffectManager()->AddScreenEffect("spy_location");

					m_bSpyLocationOpened = true;

					SetDefaultSpyTip(ZGetGame()->m_pMyCharacter->GetTeamID());
				}
			}
		}
	}

	if (m_bSpyNoticePlaying)
	{
		MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_Notice");
		if (pPicture)
		{
#define SPYNOTICE_PLAYER_WAITTIME	1000
#define SPYNOTICE_PLAYER_FADETIME	300
#define SPYNOTICE_PLAYER_SHOWTIME	3000

			/*
			DWORD dwPlayTime = dwNowTime - m_dwSpyNoticePlayStartedTime;

			if(dwPlayTime < SPYNOTICE_PLAYER_WAITTIME)
			{
				pPicture->SetOpacity(0);
			}
			else if(dwPlayTime < (SPYNOTICE_PLAYER_WAITTIME + SPYNOTICE_PLAYER_FADETIME))
			{
				float fOpacity = ((float)dwPlayTime / ((float)255 / (float)SPYNOTICE_PLAYER_FADETIME));
				if(fOpacity < 0.f) fOpacity = 0.f;

				pPicture->SetOpacity((unsigned char)fOpacity);
			}
			else if(dwPlayTime < (SPYNOTICE_PLAYER_WAITTIME + SPYNOTICE_PLAYER_FADETIME + SPYNOTICE_PLAYER_SHOWTIME))
			{
				pPicture->SetOpacity(255);
			}
			else if(dwPlayTime < (SPYNOTICE_PLAYER_WAITTIME + SPYNOTICE_PLAYER_FADETIME + SPYNOTICE_PLAYER_SHOWTIME + SPYNOTICE_PLAYER_FADETIME))
			{
				int fOpacity = (((float)SPYNOTICE_PLAYER_FADETIME - (float)dwPlayTime) / ((float)SPYNOTICE_PLAYER_FADETIME / (float)255));
				if(fOpacity > 255.f) fOpacity = 255.f;

				pPicture->SetOpacity((unsigned char)fOpacity);
			}
			else
			{
				pPicture->Show(false);
				pPicture->SetBitmap(NULL);
			}
			*/

			DWORD dwPlayTime = dwNowTime - m_dwSpyNoticePlayStartedTime;

			if (dwPlayTime >= (SPYNOTICE_PLAYER_WAITTIME + SPYNOTICE_PLAYER_FADETIME + SPYNOTICE_PLAYER_SHOWTIME + SPYNOTICE_PLAYER_FADETIME))
			{
				pPicture->Show(false);
				pPicture->SetBitmap(NULL);
			}
			else if (dwPlayTime >= (SPYNOTICE_PLAYER_WAITTIME + SPYNOTICE_PLAYER_FADETIME + SPYNOTICE_PLAYER_SHOWTIME))
			{
				pPicture->SetOpacity(0);
			}
			else if (dwPlayTime >= (SPYNOTICE_PLAYER_WAITTIME + SPYNOTICE_PLAYER_FADETIME))
			{
				pPicture->SetOpacity(255);
			}
		}
	}
}

void ZCombatInterface::OnSpyDraw(MDrawContext* pDC)
{
	if (!ZGetGame())
	{
		_ASSERT(0);
		return;
	}

	if (ZGetGame()->GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
	{
		int nElapsedTime = (int)ZGetGame()->GetMatch()->GetRemaindTime();

		if (nElapsedTime >= 2500 && nElapsedTime < 7500)
		{
			if (!m_Observer.IsVisible())
			{
				MFont* pFont = MFontManager::Get("FONTa12_O2Wht");
				_ASSERT(pFont != NULL);

				pDC->SetFont(pFont);
				pDC->Text(MRECT(MGetWorkspaceWidth() / 2, MGetWorkspaceHeight() / 2, 0, 0), ZMsg(MSG_SPY_IDENTITY), MAM_HCENTER | MAM_VCENTER);

				int nFontHeight = pFont->GetHeight();

				pFont = MFontManager::Get("FONTa12_O1Red");
				_ASSERT(pFont != NULL);

				char szSpyName[256] = "";

				for (ZCharacterManager::iterator it = ZGetGame()->m_CharacterManager.begin(); it != ZGetGame()->m_CharacterManager.end(); it++)
				{
					ZCharacter* pCharacter = (*it).second;

					if (pCharacter->GetTeamID() == MMT_RED)
					{
						strcat(szSpyName, pCharacter->GetUserName());
						strcat(szSpyName, " ");
					}
				}

				int nTextLen = (int)strlen(szSpyName);

				if (szSpyName[nTextLen] == ' ')
					szSpyName[nTextLen] = '\0';

				pDC->SetFont(pFont);
				pDC->Text(MRECT(MGetWorkspaceWidth() / 2, MGetWorkspaceHeight() / 2 + nFontHeight, 0, 0), szSpyName, MAM_HCENTER | MAM_VCENTER);
			}
		}
	}
}

void ZCombatInterface::SetSpyTip(const char* msg)
{
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_Tip");
	if (pLabel)
	{
		if (pLabel->GetFont())
		{
			pLabel->SetSize(pLabel->GetFont()->GetWidth(msg), 20.f / 600.f * MGetWorkspaceHeight());
			pLabel->SetPosition(MGetWorkspaceWidth() / 2 - pLabel->GetRect().w / 2, 540.f / 600.f * MGetWorkspaceHeight());
		}

		if (msg)
		{
			pLabel->SetText(msg);
			pLabel->Show(true);
		}
		else
		{
			pLabel->SetText("");
			pLabel->Show(false);
		}
	}

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_TipBG");
	if (pPicture)
	{
		if (pLabel && pLabel->GetFont())
		{
			pPicture->SetSize(pLabel->GetFont()->GetWidth(msg) * 1.04f, 20.f / 600.f * MGetWorkspaceWidth());
			pPicture->SetPosition(MGetWorkspaceWidth() / 2 - pPicture->GetRect().w / 2, 540.f / 600.f * MGetWorkspaceHeight());
		}

		if (msg)
		{
			pPicture->SetOpacity(100);
			pPicture->Show(true);
		}
		else
		{
			pPicture->SetOpacity(255);
			pPicture->Show(false);
		}
	}
}

void ZCombatInterface::SetSpyEventMsg(const char* imgName)
{
	/*
		Spy_EventMsg_ComingSoonOpenSpy.png  :   for when Counting down.
		Spy_EventMsg_Survive.png            :   for Spy side.
		Spy_EventMsg_EliminateSpy.png       :   for Tracker side.
	*/

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_EventMsg");
	if (pPicture)
	{
		if (imgName)
		{
			MBitmap* pBitmap = (MBitmap*)MBitmapManager::Get(imgName);
			if (pBitmap)
			{
				pPicture->SetBitmap(pBitmap);
				pPicture->Show(true);
			}
			else _ASSERT(0);
		}
		else
		{
			pPicture->Show(false);
			pPicture->SetBitmap(NULL);
		}
	}
}

void ZCombatInterface::SetSpyNotice(const char* imgName)
{
	/*
		Spy_Notice_Wait_OtherPlayer.png    :   Waiting for other players.
		Spy_Notice_Tracer.png              :   You are selected as tracker.
		Spy_Notice_Spy.png                 :   You are selected as spy.
	*/

	m_bSpyNoticePlaying = false;

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_Notice");
	if (pPicture)
	{
		if (imgName)
		{
			MBitmap* pBitmap = (MBitmap*)MBitmapManager::Get(imgName);
			if (pBitmap)
			{
				pPicture->SetBitmap(pBitmap);
				pPicture->SetOpacity(255);
				pPicture->Show(true);
			}
			else _ASSERT(0);
		}
		else
		{
			pPicture->Show(false);
			pPicture->SetBitmap(NULL);
		}
	}
}

void ZCombatInterface::SetDefaultSpyTip(MMatchTeam team)
{
	if (m_Observer.IsVisible())
	{
		SetSpyTip(ZMsg(MSG_SPY_PARTICIPATION_GUIDANCE));
	}
	else
	{
		if (team == MMT_RED)
		{
			if (m_bSpyLocationOpened)
				SetSpyTip(ZMsg(MSG_SPY_SPYSIDE_SPY_LOCATION_OPEN));
			else
				SetSpyTip(ZMsg(MSG_SPY_TIP_SPYSIDE));
		}
		else
		{
			if (m_bSpyLocationOpened)
				SetSpyTip(ZMsg(MSG_SPY_TRACKERSIDE_SPY_LOCATION_OPEN));
			else
				SetSpyTip(ZMsg(MSG_SPY_TIP_TRACKERSIDE));
		}
	}
}

const char* ZCombatInterface::GetSuitableSpyItemTip(int itemid)
{
	switch (itemid)
	{
	case 601001:	// flashbang.
		return ZMsg(MSG_SPY_TIP_FLASH_BANG);
	case 601002:	// smoke.
		return ZMsg(MSG_SPY_TIP_SMOKE);
	case 601003:	// frozen trap.
		return ZMsg(MSG_SPY_TIP_FROZEN_TRAP);
	case 601004:	// stun grenade.
		return ZMsg(MSG_SPY_TIP_STUN_GRENADE);
	case 601007:	// landmine.
		return ZMsg(MSG_SPY_TIP_BURN_TRAP);
	}

	return NULL;	// nothing found.
}

void ZCombatInterface::SetSpyTimeLimitValue(int m, int s, int ms)
{
	char szText[256];
	sprintf(szText, "%d:%02d:%02d", m, s, ms);

	ZBmNumLabel* pNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_CountDownTime");
	if (pNumLabel)
		pNumLabel->SetText(szText);

	pNumLabel = (ZBmNumLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_CountDownTime_Red");
	if (pNumLabel)
		pNumLabel->SetText(szText);
}

void ZCombatInterface::SetSpyTimeLimitValue(DWORD dwTime)
{
	int m = (int)(dwTime / 1000 / 60);
	int s = (int)(dwTime / 1000 % 60);
	int ms = (int)(dwTime % 1000 / 10);

	SetSpyTimeLimitValue(m, s, ms);
}

void ZCombatInterface::SetSpyTimer(DWORD dwTimeLimit)
{
	DWORD dwNowTime = timeGetTime();
	DWORD dwEndTime = dwNowTime + dwTimeLimit;

	SetSpyTimeLimitValue(dwTimeLimit);

	m_dwSpyTimer = dwEndTime;
}

void ZCombatInterface::PlaySpyNotice(const char* imgName)
{
	if (!imgName)
	{
		_ASSERT(0);
		return;
	}

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Spy_Notice");
	if (pPicture)
	{
		MBitmap* pBitmap = (MBitmap*)MBitmapManager::Get(imgName);
		if (pBitmap)
		{
			pPicture->SetBitmap(pBitmap);
			pPicture->SetOpacity(0);
			pPicture->Show(true);
		}
		else _ASSERT(0);
	}

	m_dwSpyNoticePlayStartedTime = timeGetTime();
	m_bSpyNoticePlaying = true;
}
