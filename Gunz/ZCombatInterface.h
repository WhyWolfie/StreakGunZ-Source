#ifndef _ZCOMBATINTERFACE_H
#define _ZCOMBATINTERFACE_H

#include "ZInterface.h"
#include "MPicture.h"
#include "MEdit.h"
#include "MListBox.h"
#include "MLabel.h"
#include "MAnimation.h"
#include "ZObserver.h"
#include "ZCombatChat.h"
#include "ZCrossHair.h"
#include "ZMiniMap.h"
#include "ZVoteInterface.h"

#ifdef _RADAR
#include "ZRadar.h"
#endif

_USING_NAMESPACE_REALSPACE2

class ZCharacter;
class ZScreenEffect;
class ZWeaponScreenEffect;
class ZMiniMap;
class ZCombatQuestScreen;

struct ZResultBoardItem {
	char szName[64];
	char szClan[CLAN_NAME_LENGTH];
	int nClanID;
	int nTeam;
	int nScore;
	int nKills;
	int nDeaths;
	int	nAllKill;
	int	nExcellent;
	int	nFantastic;
	int	nHeadShot;
	int	nUnbelievable;
	bool bMyChar;
	bool bGameRoomUser;
	int nWins;
	int	nLosses;

	ZResultBoardItem() { }
	ZResultBoardItem(const char *_szName, const char *_szClan, int _nTeam, int _nScore, int _nKills, int _nDeaths, int _nWins, int _nLosses,bool _bMyChar = false, bool _bGameRoomUser = false) {
		strcpy(szName,_szName);
		strcpy(szClan,_szClan);
		nTeam = _nTeam;
		nScore = _nScore;
		nKills = _nKills;
		nDeaths = _nDeaths;
		// �ʿ��ϸ�E�̰͵鵵 ������
		nAllKill = 0;
		nExcellent = 0;
		nFantastic = 0;
		nHeadShot = 0;
		nUnbelievable = 0;
		nWins = _nWins;
		nLosses = _nLosses;
		bMyChar = _bMyChar;
		bGameRoomUser = _bGameRoomUser;
	}
};

class ZResultBoardList : public list<ZResultBoardItem*>
{
public:
	void Destroy() { 
		while(!empty())
		{
			delete *begin();
			erase(begin());
		}
	}
};

struct DuelTournamentPlayer
{
	char m_szCharName[MATCHOBJECT_NAME_LENGTH];
	MUID uidPlayer;
	int m_nTP;
	int nVictory;
	int nMatchLevel;
	int nNumber;

	float fMaxHP;
	float fMaxAP;
	float fHP;
	float fAP;
};

class ZCombatInterface : public ZInterface
{
private:
	float				m_fElapsed;
protected:
	ZWeaponScreenEffect*		m_pWeaponScreenEffect;
//	ZScoreBoard*		m_pScoreBoard;

	// ���ȭ�鿡 �ʿ��Ѱ�
	ZScreenEffect*		m_pResultPanel;
	ZScreenEffect*		m_pResultPanel_Team;
	ZResultBoardList	m_ResultItems;
	ZScreenEffect*		m_pResultLeft;
	ZScreenEffect*		m_pResultRight;

#ifdef _RADAR
	ZRadar* m_pRadar;
#endif

	int					m_nClanIDRed;			///< Ŭ����E϶�
	int					m_nClanIDBlue;			///< �� Ŭ�� ID
	char				m_szRedClanName[32];	
	char				m_szBlueClanName[32];	///< �� Ŭ���� �̸�

	ZCombatQuestScreen*	m_pQuestScreen;

	ZBandiCapturer*		m_Capture;					///< ������Eĸ��...by kammir 2008.10.02
	bool				m_bShowUI;
	bool				m_bIsFrozen;

	ZObserver			m_Observer;			///< ������E��E
	ZCrossHair			m_CrossHair;		///< ũ�ν� ǁE�E
	ZVoteInterface		m_VoteInterface;

	ZIDLResource*		m_pIDLResource;

	MLabel*				m_pTargetLabel;
	MBitmap*			m_ppIcons[ZCI_END];		/// ĪE�����ܵ�E
	MBitmapR2*			m_pResultBgImg;
	
	bool				m_bMenuVisible;
	
	bool				m_bPickTarget;
	char				m_szTargetName[256];		// crosshair target �̸�
	
	MMatchItemDesc*		m_pLastItemDesc;

	int					m_nBulletSpare;
	int					m_nBulletCurrMagazine;
	int					m_nMagazine;

	int					m_nBulletImageIndex;
	int					m_nMagazineImageIndex;

	char				m_szItemName[256];
	
	bool				m_bReserveFinish;
	unsigned long int	m_nReserveFinishTime;

	bool				m_bDrawLeaveBattle;
	int					m_nDrawLeaveBattleSeconds;

	bool				m_bOnFinish;
	bool				m_bShowResult;
	bool				m_bIsShowUI;					// ��EUI ���߱�E.. by kammir 20081020 (������ ���û���)
	bool				m_bSkipUIDrawByRule;			// ���ӷ�E� �ʿ��ϴٸ�EUI ��ο�E� ��E��E�ֵ���

	bool				m_bDrawScoreBoard;
	MBitmapR2*			m_pGameModeBitmap[MMATCH_GAMETYPE_MAX];
	int					m_nScoreBoardIndexStart;
	DWORD				m_dwLastScoreBoardScrollTime;
//	bool				m_bKickPlayerListVisible;		// ����ȭ�鿡 �÷��̾�E����Ʈ �����ش�

	float				m_fOrgMusicVolume;

	bool				m_bNetworkAlive;
	DWORD				m_dLastTimeTick;
	DWORD				m_dAbuseHandicapTick;

	MBitmapR2*			m_pInfectedOverlay;
	MBitmapR2*			m_pInfectedWidescreenOverlay;

	void SetItemImageIndex(int nIndex);

	void SetItemName(const char* szName);
	void UpdateCombo(ZCharacter* pCharacter);
	
	void OnFinish();

	void GameCheckPickCharacter();

	// ȭ�鿡 �׸��°Ͱ�E��Eõ� ��ǵ�E
	void IconRelative(MDrawContext* pDC,float x,float y,int nIcon);
	void DrawChallengeQuest(MDrawContext* pDC);
	void DrawFriendName(MDrawContext* pDC);			// ����ƁE�̸�
	void DrawEnemyName(MDrawContext* pDC);			// ��E�̸�
	void DrawAllPlayerName(MDrawContext* pDC);		// ��E�� �̸� ǥ�� (Free Spectator)

	void DrawScoreBoard(MDrawContext* pDC);

	void DrawDuelTournamentScoreBoard(MDrawContext* pDC);						// �ཱྀE��ʸ�Ʈ ��E�ǥ ȭ��E(tabŰ)
	void DrawPlayTime(MDrawContext* pDC, float xPos, float yPos);	// �÷��� �ð�
	void DrawResultBoard(MDrawContext* pDC);		// ���� ���ȭ��E
	void DrawSoloSpawnTimeMessage(MDrawContext* pDC);	// ��� ������EŸ�̸� �޽���E
//	void DrawSpawnEffect(MDrawContext* pDC);
	void DrawLeaveBattleTimeMessage(MDrawContext* pDC);	// ���ӿ��� ������ ��ٸ��� �ð�ǥ��
//	void DrawVoteMessage(MDrawContext* pDC);		// ��ǥ�� �������϶� �޽���E
//	void DrawKickPlayerList(MDrawContext* pDC);		// kick �� �÷��̾�E�����ϴ� ȭ��E
	void GetResultInfo( void);

	void DrawTDMScore(MDrawContext* pDC);

	void DrawNPCName(MDrawContext* pDC);	// ����׿�E

	void UpdateNetworkAlive(MDrawContext* pDC);
	void EventAlive(MDrawContext* pDC);
	//void DrawHPAPBars(MDrawContext* pDC);

public:
	ZCombatChat			m_Chat;
	ZCombatChat			m_AdminMsg;
	DWORD				m_nReservedOutTime;				// Finish �Ŀ� ������ ������ �ð��� ����


	ZCombatInterface(const char* szName=NULL, MWidget* pParent=NULL, MListener* pListener=NULL);
	virtual ~ZCombatInterface();

	void OnInvalidate();
	void OnRestore();

	virtual bool OnCreate();
	virtual void OnDestroy();
	virtual void OnDraw(MDrawContext* pDC);	// �׸��� ���������� ���� �׸���
	virtual void OnDrawCustom(MDrawContext* pDC);
	virtual void DrawAfterWidgets(MDrawContext* pDC);	//MWidget���� �������� �Լ�E UI ������ ��� �׸� �� ���� DC�� �߰��� �׸���E���� �Լ�E
	void		 DrawPont(MDrawContext* pDC);
	void		 DrawMyNamePont(MDrawContext* pDC);
	void		 DrawMyWeaponPont(MDrawContext* pDC);
	void         DrawMyHPAPPont(MDrawContext* pDC);
	void		 DrawScore(MDrawContext* pDC);
	void		 DrawBuffStatus(MDrawContext* pDC);
	void		 DrawFinish();
	int DrawVictory( MDrawContext* pDC, int x, int y, int nWinCount, bool bGetWidth = false);

	void UpdateCTFMsg( const char* szMsg );
	void UpdateRTDMsg( const char* szMsg );

	virtual bool IsDone();

	void OnAddCharacter(ZCharacter *pChar);

	void Resize(int w, int h);

	void OutputChatMsg(const char* szMsg);
	void OutputChatMsg(MCOLOR color, const char* szMsg);

	virtual bool OnEvent(MEvent* pEvent, MListener* pListener);

	static MFont *GetGameFont();
	static MFont* GetGameFontHPAP();
	MPOINT GetCrosshairPoint() { return MPOINT(MGetWorkspaceWidth()/2,MGetWorkspaceHeight()/2); }
	
	ZBandiCapturer*	GetBandiCapturer()			{ return m_Capture; }					///< ������Eĸ��...by kammir 2008.10.02

	void ShowMenu(bool bVisible = true);
	void ShowInfo(bool bVisible = true);
	void EnableInputChat(bool bInput=true, bool bTeamChat=false);

	void SetDrawLeaveBattle(bool bShow, int nSeconds);

	void ShowChatOutput(bool bShow);
	void SetFrozen(bool b) { m_bIsFrozen = b; }
	bool IsFrozen() { return m_bIsFrozen; }
	bool IsChat() { return m_Chat.IsChat(); }
	bool IsTeamChat() { return m_Chat.IsTeamChat(); }
	bool IsMenuVisible() { return m_bMenuVisible; }

	void Update(float fElapsed);
	void SetPickTarget(bool bPick, ZCharacter* pCharacter = NULL);

//	void ShowScoreBoard(bool bVisible = true);
//	bool IsScoreBoardVisible() { return m_pScoreBoard->IsVisible(); }

	void Finish();
	bool IsFinish();

	ZCharacter* GetTargetCharacter();
	MUID		GetTargetUID();

	int GetPlayTime();

	void SetObserverMode(bool bEnable);
	bool GetObserverMode() { return m_Observer.IsVisible(); }
	ZObserver* GetObserver() { return &m_Observer; }
	ZCrossHair* GetCrossHair() { return &m_CrossHair; }

	ZVoteInterface* GetVoteInterface()	{ return &m_VoteInterface; }

#ifdef _RADAR
	ZRadar* GetRadar() { return m_pRadar; }
#endif

	void ShowCrossHair(bool bVisible) {	m_CrossHair.Show(bVisible); 	}
	void OnGadget(MMatchWeaponType nWeaponType);
	void OnGadgetOff();

	void SetSkipUIDraw(bool b) { m_bSkipUIDrawByRule = b; }
	bool IsSkupUIDraw() { return m_bSkipUIDrawByRule; }

	bool IsShowResult( void)  { return m_bShowResult; }
	bool IsShowUI( void)  { return m_bIsShowUI; }
	void SetIsShowUI(bool bIsShowUI)  { m_bIsShowUI = bIsShowUI; }
	bool IsShowScoreBoard()   { return m_bDrawScoreBoard; }
//	void SetKickPlayerListVisible(bool bShow = true) { m_bKickPlayerListVisible = bShow; }
//	bool IsKickPlayerListVisible() { return m_bKickPlayerListVisible; }

	bool IsNetworkalive()	{ return m_bNetworkAlive; }

	const char* GetRedClanName() const { return m_szRedClanName; }
	const char* GetBlueClanName() const { return m_szBlueClanName; }

	// Custom: Snipers
	ZWeaponScreenEffect* GetWeaponScreenEffect() { return m_pWeaponScreenEffect; }
	// Spy mode Interface.
public:
	void OnSpyCreate();
	void OnSpyDestroy();
	void DrawSpyName(MDrawContext* pDC);
	void CreateSpyGameInterface();
	void CreateSpyWaitInterface();

	void OnSpyUpdate(float fElapsed);
	void OnSpyDraw(MDrawContext* pDC);

	void SetSpyTip(const char* msg);
	void SetSpyEventMsg(const char* imgName);
	void SetSpyNotice(const char* imgName);

	void SetDefaultSpyTip(MMatchTeam team);
	const char* GetSuitableSpyItemTip(int itemid);

	bool m_bSpyLocationOpened;


protected:
	MBitmapR2* m_pSpyIcon;

protected:
	void SetSpyTimeLimitValue(int m, int s, int ms);	// if you want to set time limit, use SetSpyTimer() instead.
	void SetSpyTimeLimitValue(DWORD dwTime);

public:
	void SetSpyTimer(DWORD dwTimeLimit);
protected:
	DWORD m_dwSpyTimer;	// contains end of time limit.

public:
	void PlaySpyNotice(const char* imgName);
protected:
	bool m_bSpyNoticePlaying;
	DWORD m_dwSpyNoticePlayStartedTime;
};

void TextRelative(MDrawContext* pDC,float x,float y,const char *szText,bool bCenter=false);

#endif