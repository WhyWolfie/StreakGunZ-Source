/***********************************************************************
  ZStageInterface.h
  
  ｿ・ ｵｵ : ｽｺﾅﾗﾀﾌﾁ・ﾀﾎﾅﾍﾆ菎ﾌｽｺｸｦ ｰ・ｮﾇﾏｴﾂ ﾅｬｷ｡ｽｺ. ﾄﾚｵ・ｰ・ｮｻ�ﾀﾇ ﾆ�ﾔｮｸｦ ﾀｧﾇﾘ
           ｺﾐｸｮﾇﾟﾀｽ(ｻ鄂ﾇ ｾﾆﾁ・ｿﾏﾀ・・ｴﾙ ｺﾐｸｮ ｸ�ﾇﾟﾀｽ. -_-;).
  ﾀﾛｼｺﾀﾏ : 11, MAR, 2004
  ﾀﾛｼｺﾀﾚ : ﾀﾓｵｿﾈｯ
************************************************************************/


#ifndef _ZSTAGEINTERFACE_H
#define _ZSTAGEINTERFACE_H

#define SACRIFICEITEM_SLOT0		0
#define SACRIFICEITEM_SLOT1		1

class MNewQuestScenarioManager;


// ｾﾆﾀﾌﾅﾛ ｽｽｷﾔｿ｡ ﾀﾖｴﾂ ｾﾆﾀﾌﾅﾛ ﾁ､ｺｸ
class SacrificeItemSlotDesc
{
protected:
	MUID		m_uidUserID;
	int			m_nItemID;
	MBitmap*	m_pIconBitmap;
	char		m_szItemName[25];
	int			m_nQL;
	bool		m_bExist;
    

public:
	SacrificeItemSlotDesc()
	{
		m_nItemID = 0;
		m_pIconBitmap = NULL;
		m_szItemName[ 0] = 0;
		m_nQL = 0;
		m_bExist = false;
	}

public:
	void SetSacrificeItemSlot( const MUID& uidUserID, const unsigned long int nItemID, MBitmap* pBitmap, const char* szItemName, const int nQL);
	void RemoveItem( void)						{ m_bExist = false; }

	MUID GetUID( void)							{ return m_uidUserID; }
	MBitmap* GetIconBitmap( void)				{ return m_pIconBitmap; }
	unsigned long int GetItemID( void)			{ return m_nItemID; }
	const char* GetName( void)					{ return m_szItemName; }
	int GetQL( void)							{ return m_nQL; }
	bool IsExist( void)							{ return m_bExist; }
};


// ﾈ�ｻ� ｾﾆﾀﾌﾅﾛｹﾚｽｺ ｸｮｽｺﾆｮ ｾﾆﾀﾌﾅﾛ
class SacrificeItemListBoxItem : public MListItem
{
protected:
	unsigned long		m_nItemID;
	MBitmap*			m_pBitmap;
	char				m_szName[ 128];
	int					m_nCount;
	char				m_szDesc[ 256];

public:
	SacrificeItemListBoxItem( const unsigned long nItemID, MBitmap* pBitmap, const char* szName, int nCount, const char* szDesc)
	{
		m_nItemID = nItemID;
		m_pBitmap = pBitmap;
		m_nCount  = nCount;
		strcpy( m_szName, szName);
		strcpy( m_szDesc, szDesc);
	}
	virtual const char* GetString( void)
	{
		return m_szName;
	}
	virtual const char* GetString( int i)
	{
		if ( i == 1)
			return m_szName;

		return NULL;
	}
	virtual MBitmap* GetBitmap( int i)
	{
		if ( i == 0)
			return m_pBitmap;

		return NULL;
	}
	virtual bool GetDragItem(MBitmap** ppDragBitmap, char* szDragString, char* szDragItemString)
	{
		*ppDragBitmap = GetBitmap(0);
		strcpy( szDragString, m_szName);
		strcpy( szDragItemString, m_szName);

		return true;
	}

	unsigned long GetItemID( void)		{ return m_nItemID; }
	const char* GetItemName( void)		{ return m_szName; }
	int GetItemCount( void)				{ return m_nCount; }
	const char* GetItemDesc( void)		{ return m_szDesc; }
};

// ｸｱｷｹﾀﾌｸﾊ ﾀﾏｽﾃ ｸﾊ ｸｮｽｺﾆｮ
class RelayMapList : public MListItem
{
protected:
	char				m_szName[ 128];
	MBitmap*			m_pBitmap;

public:
	RelayMapList();
	RelayMapList( const char* szName, MBitmap* pBitmap)
	{
		strcpy( m_szName, szName);
		m_pBitmap = pBitmap;
	}

	virtual void SetString(const char *szText)
	{
		strcpy(m_szName, szText);
	}
	virtual const char* GetString( void)
	{
		return m_szName;
	}
	virtual const char* GetString( int i)
	{
		if ( i == 1)
			return m_szName;

		return NULL;
	}
	virtual MBitmap* GetBitmap( int i)
	{
		if ( i == 0)
			return m_pBitmap;

		return NULL;
	}

	const char* GetItemName( void)		{ return m_szName; }
};

class SpyBanMapList : public RelayMapList
{
protected:
	MCOLOR				m_Color;
	char				m_szMapName[128];
	int					m_nID;

public:
	SpyBanMapList(const char* szName, MBitmap* pBitmap, int nMapID) : RelayMapList(szName, pBitmap)
	{
		m_Color = MCOLOR(DEFCOLOR_MLIST_TEXT);
		m_nID = nMapID;
	}

	virtual int GetMapID()
	{
		return m_nID;
	}

	virtual void SetString(const char *szText)
	{
		strcpy(m_szName, szText);
	}
	virtual const char* GetString(void)
	{
		return m_szName;
	}
	virtual const char* GetString(int i)
	{
		if (i == 1)
			return m_szName;

		return NULL;
	}
	virtual MBitmap* GetBitmap(int i)
	{
		if (i == 0)
			return m_pBitmap;

		return NULL;
	}

	virtual const MCOLOR GetColor(void) { return GetColor(0); }
	virtual const MCOLOR GetColor(int i) {
		return m_Color;
	}

	virtual void SetColor(const MCOLOR& color) {
		m_Color = color;
	}
};


enum SCENARIOTYPE
{
	ST_STANDARD		= 0,
	ST_SPECIAL		= 1,
};

// ﾄ�ｽｺﾆｮ ｽﾃｳｪｸｮｿﾀ ﾀﾌｸｧ ｸｮｽｺﾆｮ
struct MSenarioList
{
	SCENARIOTYPE	m_ScenarioType;								// ｽﾃｳｪｸｮｿﾀ ﾅｸﾀﾔ
	char			m_szName[ 32];								// ﾄ�ｽｺﾆｮ ｽﾃｳｪｸｮｿﾀ ﾀﾌｸｧ
	char			m_szMapSet[ 32];							// ﾄ�ｽｺﾆｮ ｸﾊ ｼﾂ ﾀﾌｸｧ

	MSenarioList()
	{
		m_ScenarioType	= ST_STANDARD;
		m_szName[ 0]	= 0;
		m_szMapSet[ 0]	= 0;
	}
};

typedef map<int,MSenarioList>	LIST_SCENARIONAME;

// Class : ZStageInterface
class ZStageInterface
{
protected:	// protected varialbes
	bool			m_bDrawStartMovieOfQuest;
	DWORD			m_dwClockOfStartMovie;
	int				m_nPosOfItem0, m_nPosOfItem1;
	MMATCH_GAMETYPE		m_nGameType;
	bool			m_bPrevQuest;								// ﾀﾌﾀ・ｰﾔﾀﾓ ﾅｸﾀﾔﾀﾌ ﾄ�ｽｺﾆｮｿｴｴﾂﾁ・ｾﾆｴﾑﾁ・..
	MBitmapR2*		m_pTopBgImg;								// ｻ�ｴﾜ ｸﾊ ﾀﾌｹﾌﾁ・
	MBitmapR2*		m_pStageFrameImg;							// ﾇﾁｷｹﾀﾓ ﾀﾌｹﾌﾁ・
	MBitmapR2*		m_pItemListFrameImg;						// ｾﾆﾀﾌﾅﾛ ｸｮｽｺﾆｮ ﾇﾁｷｹﾀﾓ ﾀﾌｹﾌﾁ・
	MBitmapR2*		m_pRelayMapListFrameImg;					// ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ﾇﾁｷｹﾀﾓ ﾀﾌｹﾌﾁ・

	LIST_SCENARIONAME	 m_SenarioDesc;							// ﾄ�ｽｺﾆｮ ｽﾃｳｪｸｮｿﾀ ﾀﾌｸｧ ｸｮｽｺﾆｮ
	MNewQuestScenarioManager* m_pChallengeQuestScenario;		// ﾃｧｸｰﾁ・ﾄ�ｽｺﾆｮ ｽﾃｳｪｸｮｿﾀ ｸ�ﾘｼ
	bool			m_bRelayMapRegisterComplete;				// ｸｱｷｹﾀﾌｸﾊ ﾈｮﾀﾎ(ｵ�ﾓﾏ)ｹ�ﾆｰ ｿｩｺﾎ
	bool			m_bEnableWidgetByRelayMap;					// ｸｱｷｹﾀﾌｸﾊｰ豼・ﾀｧﾁｬﾈｰｼｺﾈｭ ｼｼﾆﾃ(ｰﾔﾀﾓｽﾃﾀﾛｹ�ﾆｰ ﾆ�ﾇﾔ)



protected:	// protected functions
	void UpdateSacrificeItem( void);
	void UpdateStageGameInfo(const int nQL, const int nMapsetID, const int nScenarioID);

public:		// public variables
	SacrificeItemSlotDesc	m_SacrificeItem[ 2];				// ｰ｢ｰ｢ﾀﾇ ｾﾆﾀﾌﾅﾛ ｽｽｷﾔｿ｡ ﾀﾖｴﾂ ｾﾆﾀﾌﾅﾛｿ｡ ｴ・ﾑ ﾁ､ｺｸｸｦ ﾀ�ﾀ・
	
	int				m_nListFramePos;							// ｾﾆﾀﾌﾅﾛ ｸｮｽｺﾆｮ ﾇﾁｷｹﾀﾓ ﾀｧﾄ｡
	int				m_nStateSacrificeItemBox;					// ｾﾆﾀﾌﾅﾛ ｸｮｽｺﾆｮ ﾇﾁｷｹﾀﾓ ｻ�ﾅﾂ

	int				m_nRelayMapListFramePos;					// ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ﾇﾁｷｹﾀﾓ ﾀｧﾄ｡

public:		// public functions
	// Initialize
	ZStageInterface( void);										// Constructor
	virtual ~ZStageInterface( void);							// Destructor
	void OnCreate( void);										// On create
	void OnDestroy( void);										// On destroy
	void OpenSacrificeItemBox( void);
	void CloseSacrificeItemBox( void);
	void HideSacrificeItemBox( void);
	void GetSacrificeItemBoxPos( void);
	LIST_SCENARIONAME* GetSenarioDesc()							{ return &m_SenarioDesc;		}
	MNewQuestScenarioManager* GetChallengeQuestScenario() { return m_pChallengeQuestScenario; }

	// ｸｱｷｹﾀﾌｸﾊ ﾀﾎﾅﾍﾆ菎ﾌｽｺ
	void OpenRelayMapBox( void);
	void CloseRelayMapBox( void);
	void HideRelayMapBox( void);
	void SetRelayMapBoxPos( int nBoxPos);
	void PostRelayMapElementUpdate( void);
	void PostRelayMapTurnCount( void);
	void PostRelayMapInfoUpdate( void);
	void RelayMapCreateMapList( void);
	bool GetIsRelayMapRegisterComplete()		{ return m_bRelayMapRegisterComplete; }
	void SetIsRelayMapRegisterComplete(bool b)	{ m_bRelayMapRegisterComplete = b; }
	bool GetEnableWidgetByRelayMap()			{ return m_bEnableWidgetByRelayMap; }
	void SetEnableWidgetByRelayMap(bool b);
	void SetStageRelayMapImage();									// ﾈｭｸ・ｻ�ｴﾜﾀﾇ ｸﾊ ﾀﾌｹﾌﾁ・ｼｳﾁ､ﾇﾏｱ・

	// Game stage interface
	void ChangeStageButtons( bool bForcedEntry, bool bMaster, bool bReady);
	void ChangeStageGameSetting( const MSTAGE_SETTING_NODE* pSetting);
	void ChangeStageEnableReady( bool bReady);
	void SetMapName(const char* szMapName);

	void OnStageInterfaceSettup( void);							// Change map combobox
	void OnStageCharListSettup( void);							// Change map combobox
	void OnSacrificeItem0( void);								// Push a button : sacrifice item 0
	void OnSacrificeItem1( void);								// Push a button : sacrifice item 1
	void OnDropSacrificeItem( int nSlotNum);					// Drop sacrifice item
	void OnRemoveSacrificeItem( int nSlotNum);					// Remove sacrifice item

	// Start Movie
	void OnDrawStartMovieOfQuest( void);
	void StartMovieOfQuest( void);
	bool IsShowStartMovieOfQuest( void);


	// Sacrifice item list box
	void SerializeSacrificeItemListBox( void);					// ｾﾆﾀﾌﾅﾛ ｹﾚｽｺ ｾ�ｵ･ﾀﾌﾆｮ
	bool ReadSenarioNameXML( void);

	void OnStartFail( const int nType, const MUID& uidParam );

#ifdef _QUEST_ITEM
	bool OnResponseDropSacrificeItemOnSlot( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID );
	bool OnResponseCallbackSacrificeItem( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID );
	bool OnResponseQL( const int nQL );
	bool OnResponseSacrificeSlotInfo( const MUID& uidOwner1, const unsigned long int nItemID1, 
									  const MUID& uidOwner2, const unsigned long int nItemID2 );
	bool OnNotAllReady();
	bool OnQuestStartFailed( const int nState );
	bool OnStageGameInfo( const int nQL, const int nMapsetID, const unsigned int nScenarioID );
#endif

	bool OnStopVote();
	// Spy Mode.
protected:
	MBitmapR2* m_pSpyBanMapListFrameImg;
public:
	void OpenSpyBanMapBox();
	void CloseSpyBanMapBox();
	void HideSpyBanMapBox();
	void SetSpyBanMapBoxPos(int nBoxPos);

	void CreateSpyBanMapList();
	bool GetPlayableSpyMapList(int players, vector<int>& out);

	void OnSpyStageActivateMap(int nMapID, bool bExclude);
	void OnSpyStageBanMapList(void* pMapListBlob);

	int m_nSpyBanMapListFramePos;
};

void OnDropCallbackRemoveSacrificeItem( void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString);

// Listner
MListener* ZGetSacrificeItemListBoxListener( void);
MListener* ZGetRelayMapListBoxListener( void);
MListener* ZGetMapListBoxListener( void);

MListener* ZGetSpyBanMapListListener(void);

#endif
