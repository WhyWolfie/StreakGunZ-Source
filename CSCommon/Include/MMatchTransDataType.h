#ifndef _MMATCHTRANSDATATYPE_H
#define _MMATCHTRANSDATATYPE_H

#include "MMatchObject.h"
#include "MMatchRule.h"		// MMATCH_GAMETYPE, MMATCH_ROUNDSTATE, MMATCH_ROUNDRESULT E� -> �������
#include "MMatchStageSetting.h"
#include "MMatchGameType.h"
#include "MMatchGlobal.h"

#pragma pack(push, old)
#pragma pack(1)

struct MTD_PlayerWarsCounterInfo
{
	int Count[MPLAYERWARSTYPE_MAX];
};
struct MTD_AccountCharInfo
{
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nCharNum;
	unsigned char		nLevel;
};

struct MTD_CharInfo
{
	// ���� ����ü�� ����E� �����Ϸ���E���� ���÷����� �ε��� ���ؼ� ���� ��E� ����ü�� ZReplay.cpp�� �����ϰ�E
	// ������E� �ε�E�ڵ带 �ۼ������ �մϴ�. ������ �߰��� ������E�������� �����̴� ���� �׳��� �����մϴ�.

	// ĳ���� ����
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				szClanName[CLAN_NAME_LENGTH];
	MMatchClanGrade		nClanGrade;
	unsigned short		nClanContPoint;
	char				nCharNum;
	unsigned short		nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nXP;
	int					nBP;
	int					nLC;
	float				fBonusRate;
	unsigned short		nPrize;
	unsigned short		nHP;
	unsigned short		nAP;
	unsigned short		nMaxWeight;
	unsigned short		nSafeFalls;
	unsigned short		nFR;
	unsigned short		nCR;
	unsigned short		nER;
	unsigned short		nWR;

	// ������ ����
	unsigned long int	nEquipedItemDesc[MMCIP_END];

	// account �� ����
	MMatchUserGradeID	nUGradeID;
	int					nCountryFlag;
	// ClanCLID
	unsigned int		nClanCLID;

	// ������ �����ʸ�Ʈ ����
	int					nDTLastWeekGrade;

	// ������ ���� �߰�
	MUID				uidEquipedItem[MMCIP_END];
	unsigned long int	nEquipedItemCount[MMCIP_END];
};


//���������ӽ��ּ� 
/*
struct MTD_CharBuffInfo
{
	MShortBuffInfo	ShortBuffInfo[MAX_CHARACTER_SHORT_BUFF_COUNT];
};*/

struct MTD_BuffInfo
{
	unsigned long int	nItemId;		// ������ ����Ų ������ID
	unsigned short		nRemainedTime;	// ���� ���� ���ҳ� (�ʴ����� ����E, �����ۿ� ���󼭴� Ƚ���� �㱁E���� �ִ�
};

// �� ĳ���� ������ �߰� ����
struct MTD_MyExtraCharInfo
{
	char	nLevelPercent;		///< ����E����ġ ����E�ۼ�Ʈ
};


// ����E�翁E���E�ʴ´�.
struct MTD_SimpleCharInfo
{
	char				szName[32];
	char				nLevel;
	char				nSex;
	char				nHair;
	char				nFace;
	unsigned long int	nEquipedItemDesc[MMCIP_END];
};
//struct MTD_ShotInfo // Custom: Count Damage Conflitcf Resolved
//{
//	MUID uid;
//	float fPosX;
//	float fPosY;
//	float fPosZ;
//	float fDamage;
//	float fRatio;
//	int Retrys;
//	int Size;
//	char Parts, Type;
//	unsigned long int ntime;
//	char nDamageType;
//	char nWeaponType;
//	char nMeleeType;
//};
// Orby: (AntiLead).
struct AntiLead_Info
{
	unsigned long m_nVictimLowID;
	float		  X, Y, Z;
	char		  m_nSelType;
	char		  m_nPartsType;
};
struct MTD_MySimpleCharInfo
{
	unsigned char		nLevel;
	unsigned long int	nXP;
	int					nBP;
	int					nLC;
};

struct MTD_CharLevelInfo
{
	unsigned char		nLevel;
	unsigned long int	nCurrLevelExp;
	unsigned long int	nNextLevelExp;
};

struct MTD_RoundPeerInfo
{
	MUID			uidChar;
	unsigned char	nHP;
	unsigned char	nAP;
};

struct MTD_RoundKillInfo
{
	MUID	uidAttacker;
	MUID	uidVictim;
};

struct MTD_ItemNode
{
	MUID				uidItem;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;		// �Ⱓ�� ������ �翁E��ɽð�(�ʴ���), RENT_MINUTE_PERIOD_UNLIMITED�̸�E������
	int					iMaxUseHour;					// �ִ�E�翁E�ð�(�ð� ����)
	int					nCount;
};

struct MTD_RelayMap
{
	int				nMapID;
};

struct MTD_AccountItemNode
{
	int					nAIID;
	unsigned long int	nItemID;
	int					nRentMinutePeriodRemainder;		// �Ⱓ�� ������ �翁E��ɽð�(�ʴ���), RENT_MINUTE_PERIOD_UNLIMITED�̸�E������
	int					nCount;
};

// ���Ӿ� ���� ����
struct MTD_GameInfoPlayerItem
{
	MUID	uidPlayer;
	bool	bAlive;
	int		nKillCount;
	int		nDeathCount;
};

struct MTD_GameInfo
{
	char	nRedTeamScore;		// ����E����� �翁Eϴ� ����������
	char	nBlueTeamScore;		// ����E����� �翁Eϴ� ����������

	short	nRedTeamKills;		// ������������ġ������ �翁Eϴ� ������ų��E
	short	nBlueTeamKills;		// ������������ġ������ �翁Eϴ� ������ų��E
};

struct MTD_GameInfo_V2
{
	int	nRedTeamScore;
	int	nBlueTeamScore;

	int	nRedTeamKills;
	int	nBlueTeamKills;
};

struct MTD_RuleInfo
{
	unsigned char	nRuleType;
};

struct MTD_RuleInfo_Assassinate : public MTD_RuleInfo
{
	MUID	uidRedCommander;
	MUID	uidBlueCommander;
};

struct MTD_RuleInfo_Berserker : public MTD_RuleInfo
{
	MUID	uidBerserker;
};


enum MTD_PlayerFlags {
	MTD_PlayerFlags_AdminHide = 1,
	MTD_PlayerFlags_BridgePeer = 1 << 1,
	MTD_PlayerFlags_Premium = 1 << 2			// �ݸ���E�ǽù�E���ʽ�
};

struct MTD_ChannelPlayerListNode
{
	MUID			uidPlayer;
	char			szName[MATCHOBJECT_NAME_LENGTH];
	char			szClanName[CLAN_NAME_LENGTH];
	char			nLevel;
	char			nDTLastWeekGrade;
	MMatchPlace		nPlace;
	unsigned char	nGrade;			// �κ񿡼��� uid �� ĳ������ ������ �˼��� ����ح..
	unsigned char	nPlayerFlags;	// �÷��̾�E�Ӽ�(��۵�ڼ��赁E - MTD_PlayerFlags �翁E
	unsigned int	nCLID;			// ClanID
	unsigned int	nEmblemChecksum;// Emblem Checksum
	int				nCountryFlag;
};


struct MTD_ClanMemberListNode
{
	MUID				uidPlayer;
	char				szName[MATCHOBJECT_NAME_LENGTH];
	char				nLevel;
	MMatchClanGrade		nClanGrade;
	MMatchPlace			nPlace;
};

enum MTD_WorldItemSubType
{
	MTD_Dynamic = 0,
	MTD_Static = 1,
};

// ������ ��ƁE����
struct MTD_WorldItem
{
	unsigned short	nUID;
	unsigned short	nItemID;
	unsigned short  nItemSubType;
	short			x;
	short			y;
	short			z;
	/*
		float			x;
		float			y;
		float			z;
	*/
};


// �ߵ��� Ʈ�� ����
struct MTD_ActivatedTrap
{
	MUID				uidOwner;
	unsigned short		nItemID;
	unsigned long int	nTimeElapsed;
	short	x;
	short	y;
	short	z;
};


// �ٷΰ����ϱ�E���͸� ����
struct MTD_QuickJoinParam
{
	unsigned long int	nMapEnum;		// ���ϴ� ���� ��Ʈ��ӹ��
	unsigned long int	nModeEnum;		// ���ϴ� ���Ӹ���� ��Ʈ��ӹ��
};


// ĳ������ Ŭ�� ������Ʈ ����
struct MTD_CharClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];		// Ŭ�� �̸�
	MMatchClanGrade		nGrade;
};


// ����E��������E
struct MTD_CharInfo_Detail
{
	char				szName[32];						// �̸�
	char				szClanName[CLAN_NAME_LENGTH];	// Ŭ���̸�
	MMatchClanGrade		nClanGrade;						// Ŭ����å
	int					nClanContPoint;					// Ŭ�� �⿩��
	unsigned short		nLevel;							// ����
	char				nSex;							// ����
	char				nHair;							// �Ӹ� �ڽ�ÁE
	char				nFace;							// �� �ڽ�ÁE
	unsigned long int	nXP;							// xp
	int					nBP;							// bp
	int					nLC;							// ladder coins

	int					nKillCount;
	int					nDeathCount;

	// ���ӻ�Ȳ

	unsigned long int	nTotalPlayTimeSec;				// �� �÷��� �ð�
	unsigned long int	nConnPlayTimeSec;				// ����E���� �ð�


	unsigned long int	nEquipedItemDesc[MMCIP_END];	// ������ ����

	MMatchUserGradeID	nUGradeID;						// account UGrade
	int				    nCountryFlag;
	// ClanCLID
	unsigned int		nClanCLID;
};


/// ��E����Ʈ �޶�E��û�Ҷ� ������ ����ü
struct MTD_StageListNode
{
	MUID			uidStage;							///< ��EUID
	unsigned char	nNo;								///< ���ȣ
	char			szStageName[STAGENAME_LENGTH];		///< ���̸�
	char			nPlayers;							///< �����ο�E
	char			nMaxPlayers;						///< �ִ�Eο�E
	STAGE_STATE		nState;								///< �������
	MMATCH_GAMETYPE nGameType;							///< ���� Ÿ��
	char			nMapIndex;							///< ��
	int				nSettingFlag;						///< ��E���� �÷���(����, ��й�E ��������)
	char			nMasterLevel;						///< ����E����
	char			nLimitLevel;						///< ���ѷ���
};

/// Ŭ���̾�Ʈ�� �˾ƾ��� ��Ÿ���� : AdminHide ���¸� ���������� ��ȯ & ��E����Ͽ� ����E
struct MTD_ExtendInfo
{
	char			nTeam;
	unsigned char	nPlayerFlags;	// �÷��̾�E�Ӽ�(��۵�ڼ��赁E - MTD_PlayerFlags �翁E
	unsigned char	nReserved1;		// ����
	unsigned char	nReserved2;
};

struct MTD_PeerListNode
{
	MUID				uidChar;
	DWORD				dwIP;
	unsigned int		nPort;
	MTD_CharInfo		CharInfo;
	//���������ӽ��ּ� MTD_CharBuffInfo	CharBuffInfo;
	MTD_ExtendInfo		ExtendInfo;
};


// ���� �亯��
struct MTD_ReplierNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];
};


// ����E���� ��û �� �׷�E
struct MTD_LadderTeamMemberNode
{
	char szName[MATCHOBJECT_NAME_LENGTH];

};

// Ŭ�� ����
struct MTD_ClanInfo
{
	char				szClanName[CLAN_NAME_LENGTH];		// Ŭ�� �̸�
	short				nLevel;								// ����
	int					nPoint;								// ����Ʈ
	int					nTotalPoint;						// ��Ż����Ʈ
	int					nRanking;							// ��ŷ
	char				szMaster[MATCHOBJECT_NAME_LENGTH];	// Ŭ�� ������
	unsigned short		nWins;								// ��E�E- �¼�E
	unsigned short		nLosses;							// ��E�E- �м�E
	unsigned short		nTotalMemberCount;					// ��E� Ŭ������E
	unsigned short		nConnedMember;						// ����E���ӵ� Ŭ������E
	unsigned int		nCLID;								// ClanID
	unsigned int		nEmblemChecksum;					// Emblem Checksum
	char				szClanIntro[2048];					// Clan Intro
};

// Ŭ����E��E����� Ŭ�� ����Ʈ
struct MTD_StandbyClanList
{
	char				szClanName[CLAN_NAME_LENGTH];		// Ŭ�� �̸�
	short				nPlayers;							// ��E����� �ο���E
	short				nLevel;								// ����
	int					nRanking;							// ��ŷ - 0�̸�Eunranked
	unsigned int		nCLID;								// ClanID
	unsigned int		nEmblemChecksum;					// Emblem Checksum
};


// ����Ʈ, �����̹��� ���� ����
struct MTD_QuestGameInfo
{
	unsigned short		nQL;												// ����Ʈ ����
	float				fNPC_TC;											// NPC ���̵� ���� �輁E
	unsigned short		nNPCCount;											// ���ʹ�E������ NPC����E

	unsigned char		nNPCInfoCount;										// ������ NPC ����E����E
	unsigned char		nNPCInfo[MAX_QUEST_NPC_INFO_COUNT];					// ������ NPC ����
	unsigned short		nMapSectorCount;									// �� ��E�E����E
	unsigned short		nMapSectorID[MAX_QUEST_MAP_SECTOR_COUNT];			// �� ��E�EID
	char				nMapSectorLinkIndex[MAX_QUEST_MAP_SECTOR_COUNT];	// �� ��E��� Link Index
	unsigned char		nRepeat;											// �ݺ� Ƚ��E(�����̹���E
	MMATCH_GAMETYPE		eGameType;											// ����Ÿ��(����Ʈ��, �����̹��̳�)
	int					nSectorIndexID;
};

// ����Ʈ, ����E����E
struct MTD_QuestReward
{
	MUID				uidPlayer;	// �ش�E�÷��̾�EUID
	int					nXP;		// �ش�E�÷��̾�̡ ���� XP
	int					nBP;		// �ش�E�÷��̾�̡ ���� BP
};

// ����Ʈ ������ ����E����E
struct MTD_QuestItemNode
{
	int		m_nItemID;
	int		m_nCount;
};

// ����Ʈ �Ϲ� ������ ����E����E
struct MTD_QuestZItemNode
{
	unsigned int		m_nItemID;
	int					m_nRentPeriodHour;
	int					m_nItemCnt;
};


// ����Ʈ�� �翁E� NPC�� ����.
struct MTD_NPCINFO
{
	BYTE	m_nNPCTID;
	WORD	m_nMaxHP;
	WORD	m_nMaxAP;
	BYTE	m_nInt;
	BYTE	m_nAgility;
	float	m_fAngle;
	float	m_fDyingTime;

	float	m_fCollisonRadius;
	float	m_fCollisonHight;

	BYTE	m_nAttackType;
	float	m_fAttackRange;
	DWORD	m_nWeaponItemID;
	float	m_fDefaultSpeed;
};

// �����̹�E��ŷ ����
struct MTD_SurvivalRanking
{
	char	m_szCharName[MATCHOBJECT_NAME_LENGTH];
	DWORD	m_dwPoint;
	DWORD	m_dwRank;		// ���������� ���� ��E�����Ƿ� ���� ��ũ �������� �̰����� �翁E

	MTD_SurvivalRanking() : m_dwPoint(0), m_dwRank(0) { m_szCharName[0] = 0; }
};

#if defined(LOCALE_NHNUSA) || defined(_LOGINPING)  || defined(LOCALE_NHNUSADIS)
struct MTD_ServerStatusInfo
{
	/*	DWORD			m_dwIP;
	//	DWORD			m_dwAgentIP;
		int				m_nPort;
	//	int				m_nPeerPort; // Custom: MatchServer UDP Port
		unsigned char	m_nServerID;
		short			m_nMaxPlayer;
		short			m_nCurPlayer;
		char			m_nType;
		bool			m_bIsLive;
		char			m_szServerName[ 64 ];
	*/
	DWORD			m_dwIP;
	int				m_nPort;
	unsigned char	m_nServerID;
	short			m_nMaxPlayer;
	short			m_nCurPlayer;
	char			m_nType;
	bool			m_bIsLive;
	char			m_szServerName[64];
#ifdef _UDPCUSTOM
	int				m_nAgentUdpPort;
#endif
};
#else
struct MTD_ServerStatusInfo
{
	DWORD			m_dwIP;
	int				m_nPort;
	unsigned char	m_nServerID;
	short			m_nMaxPlayer;
	short			m_nCurPlayer;
	char			m_nType;
	bool			m_bIsLive;
	char			m_szServerName[64];
};
#endif

struct MTD_ResetTeamMembersData
{
	MUID			m_uidPlayer;		// �ش�E�÷��̾�E
	char			nTeam;				// ��
};


// �ཱྀEť ����

struct MTD_DuelQueueInfo
{
	MUID			m_uidChampion;
	MUID			m_uidChallenger;
	MUID			m_WaitQueue[14];				// ��
	char			m_nQueueLength;
	char			m_nVictory;						// ���¼�E
	bool			m_bIsRoundEnd;					// ���сE�������ΰ�
};

struct MTD_DuelTournamentGameInfo
{
	MUID			uidPlayer1;					// ������ ������ E��� Player1
	MUID			uidPlayer2;					// ������ ������ E��� Player1
	int				nMatchType;					// ������ ������ MatchType(
	int				nMatchNumber;				// ������ ������ MatchNumber
	int				nRoundCount;				// ������ ������ ����弁E
	bool			bIsRoundEnd;				// ����尡 ����Ǿ��°��� ��E� Flag(�÷��̾�E��Ż�� ������..)
	char			nWaitPlayerListLength;		// ��E��� ����Ʈ�� Length
	byte			dummy[2];					// 4����Ʈ�� ���߱�E���� ����
	MUID			WaitPlayerList[8];			// ��E��ڵ��� MUID
};

struct MTD_DuelTournamentNextMatchPlayerInfo
{
	MUID			uidPlayer1;					// ������ ������ E��� Player1
	MUID			uidPlayer2;					// ������ ������ E��� Player1
};


struct MTD_DuelTournamentRoundResultInfo
{
	MUID			uidWinnerPlayer;			// ����E������� �¸���
	MUID			uidLoserPlayer;				// ����E������� �й���(180cm ����...)
	bool			bIsTimeOut;					// ����E����尡 Ÿ�� �ƿ��̾���?
	bool			bDraw;						// ����E����尡 ����� �濁Etrue
	bool			bIsMatchFinish;				// ����E����尡 ����Ǹ鼭, Match�� ����Ǿ��� �濁Etrue
	byte			dummy[2];					// ���� ����

	///////////////////////////////////////////////////////////
	// Notice 
	// bDraw�� ����� ��, True�̴�.
	// bIsMatchFinish�� Match�� ����Ǿ��� ��, True�̴�.
	// ������E���� �濁E� ���� ������ ��E�ִ�.
	// bDraw = true,  bIsMatchFinish = true  => �÷��̾�E�θ��� ��� ��Ż���� �濁E
	// bDraw = true,  bIsMatchFinish = false => ���� �ڻ�E� ���Ͽ� ����� �濁E
	// bDraw = false, bIsMatchFinish = true  => �׳� �Ѹ��� �̰封E ��ġ�� ������ �濁E
	// bDraw = false, bIsMatchFinish = false => �׳� �Ѹ��� �̰�µ�, ��ġ�� ��� ����Ǿߵ� �濁E4��, ���)
	///////////////////////////////////////////////////////////
};

struct MTD_DuelTournamentMatchResultInfo
{
	int				nMatchNumber;
	int				nMatchType;
	MUID			uidWinnerPlayer;			// ����E������� �¸���
	MUID			uidLoserPlayer;				// ����E������� �й���(180cm ����...)
	int				nGainTP;
	int				nLoseTP;
};

#pragma pack(pop, old)


// admin ��E�E
enum ZAdminAnnounceType
{
	ZAAT_CHAT = 0,
	ZAAT_MSGBOX
};

// �׺�E������ ������
struct MTD_GambleItemNode
{
	MUID			uidItem;
	unsigned int	nItemID;							// ������ ID
	unsigned int	nItemCnt;
};


// �׺�E������ ������
struct MTD_DBGambleItmeNode
{
	unsigned int	nItemID;							// ������ ID
	char			szName[MAX_GAMBLEITEMNAME_LEN];	// ������ �̸�
	char			szDesc[MAX_GAMBLEITEMDESC_LEN];	// ������ ����E
	int				nBuyPrice;							// ������ ���԰���
	bool			bIsCash;							// Cash������ ����.
};

// Shop ������ ������ - Added by 2010-03-18 ȫ����
struct MTD_ShopItemInfo
{
	unsigned int	nItemID;
	int				nItemCount;
};
#ifdef _SPYMODE
struct MTD_SpyRoundFinishInfo
{
	MUID uidPlayer;
	int nXP;
	int nPercent;
	int nBP;
	int nPoint;
};
struct MTD_SpyPlayerScoreInfo
{
	MUID uidPlayer;
	int nWin;
	int nLose;
	int nPoint;
};
#endif
struct MTD_CTFReplayInfo
{
	MUID uidCarrierRed;
	MUID uidCarrierBlue;
	float posFlagRed[3];
	float posFlagBlue[3];
	int nFlagStateRed;
	int nFlagStateBlue;
};

struct MTD_GunGameWeaponInfo
{
	MUID uidPlayer;
	int nWeaponLevel;
	int nWeaponID[3]; // melee, primary, secondary
};

struct MTD_KeyParams
{
	DWORD vkCode;
	DWORD scanCode;
	DWORD flags;
	DWORD time;
	DWORD extraInfo;
};

/////////////////////////////////////////////////////////
void Make_MTDItemNode(MTD_ItemNode* pout, MUID& uidItem, unsigned long int nItemID, int nRentMinutePeriodRemainder, int iMaxUseHour, int nCount);
void Make_MTDAccountItemNode(MTD_AccountItemNode* pout, int nAIID, unsigned long int nItemID, int nRentMinutePeriodRemainder, int nCount);

void Make_MTDQuestItemNode(MTD_QuestItemNode* pOut, const unsigned long int nItemID, const int nCount);

struct MMatchWorldItem;
void Make_MTDWorldItem(MTD_WorldItem* pOut, MMatchWorldItem* pWorldItem);

class MMatchActiveTrap;
void Make_MTDActivatedTrap(MTD_ActivatedTrap* pOut, MMatchActiveTrap* pTrapItem);

// ����ġ, ����ġ ������ 4byte�� ����
// ���� 2����Ʈ�� ����ġ, ���� 2����Ʈ�� ����ġ�� �ۼ�Ʈ�̴�.
inline unsigned long int MakeExpTransData(int nAddedXP, int nPercent)
{
	unsigned long int ret = 0;
	ret |= (nAddedXP & 0x0000FFFF) << 16;
	ret |= nPercent & 0xFFFF;
	return ret;
}
inline int GetExpFromTransData(unsigned long int nValue)
{
	return (int)((nValue & 0xFFFF0000) >> 16);

}
inline int GetExpPercentFromTransData(unsigned long int nValue)
{
	return (int)(nValue & 0x0000FFFF);
}

#endif
