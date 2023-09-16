#ifndef _MBASEGAMETYPE_H
#define _MBASEGAMETYPE_H

#include <crtdbg.h>
#include <map>
#include <set>
using namespace std;


/// ���� Ÿ��
enum MMATCH_GAMETYPE {
	MMATCH_GAMETYPE_DEATHMATCH_SOLO		=0,			///< ���� ������ġ
	MMATCH_GAMETYPE_DEATHMATCH_TEAM		=1,			///< �� ������ġ
	MMATCH_GAMETYPE_GLADIATOR_SOLO		=2,			///< ���� �۷�������
	MMATCH_GAMETYPE_GLADIATOR_TEAM		=3,			///< �� �۷�������
	MMATCH_GAMETYPE_ASSASSINATE			=4,			///< ������E�ϻ�E�E
	MMATCH_GAMETYPE_TRAINING			=5,			///< ����

	MMATCH_GAMETYPE_SURVIVAL			=6,			///< �����̹�E
	MMATCH_GAMETYPE_QUEST				=7,			///< ����Ʈ

	MMATCH_GAMETYPE_BERSERKER			=8,			///< ������ġ ����Ŀ
	MMATCH_GAMETYPE_DEATHMATCH_TEAM2	=9,			///< ��������ġ �ͽ�Ʈ��
	MMATCH_GAMETYPE_DUEL				=10,		///< �ཱྀE��ġ
	MMATCH_GAMETYPE_DUELTOURNAMENT		=11,		///< �ཱྀE��ʸ�Ʈ
	MMATCH_GAMETYPE_QUEST_CHALLENGE		=12,		///< ç��������Ʈ

	// Custom: Game modes
	MMATCH_GAMETYPE_TEAM_TRAINING		=13,
	MMATCH_GAMETYPE_CTF					=14,
	MMATCH_GAMETYPE_INFECTED			=15,
	MMATCH_GAMETYPE_GUNGAME				=16,
	MMATCH_GAMETYPE_SPY				    =17,
	MMATCH_GAMETYPE_DROPMAGIC           =18,		///< Magic Box
/*
#ifndef _CLASSIC
	MMATCH_GAMETYPE_CLASSIC_SOLO,
	MMATCH_GAMETYPE_CLASSIC_TEAM,
#endif
*/
	MMATCH_GAMETYPE_MAX,
};
// ��尡 �߰��Ǹ�E���÷��� ���忁E��E���ѵ �߰��ؾ� �Ѵ� (cpp)
extern const char* MMatchGameTypeAcronym[MMATCH_GAMETYPE_MAX];

#define		MAX_RELAYMAP_LIST_COUNT			20
struct RelayMap
{
	int				nMapID;
};

enum RELAY_MAP_TYPE
{
	RELAY_MAP_TURN			= 0,
	RELAY_MAP_RANDOM,

	RELAY_MAP_MAX_TYPE_COUNT
};

enum RELAY_MAP_REPEAT_COUNT
{
	RELAY_MAP_1REPEAT			= 0,
	RELAY_MAP_2REPEAT,
	RELAY_MAP_3REPEAT,
	RELAY_MAP_4REPEAT,
	RELAY_MAP_5REPEAT,

	RELAYMAP_MAX_REPEAT_COUNT,
};

// ����Ʈ ���� Ÿ��
const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
// const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_TEAM2;


struct MMatchGameTypeInfo
{
	MMATCH_GAMETYPE		nGameTypeID;			// ID
	char				szGameTypeStr[24];		// ���� Ÿ�� �̸�
	float				fGameExpRatio;			// ����ġ ��� ���� - ����EGladiator�� 50%�̴�
	float				fTeamMyExpRatio;		// ����E��� ���ο��� ����EǴ� ����ġ
	float				fTeamBonusExpRatio;		// ����E��� ������ �����Ǵ� ����ġ
	set<int>			MapSet;					// �� ����Ÿ�Կ��� �÷��� ������ ��
	void Set(const MMATCH_GAMETYPE a_nGameTypeID, const char* a_szGameTypeStr, const float a_fGameExpRatio,
		     const float a_fTeamMyExpRatio, const float a_fTeamBonusExpRatio);
	void AddMap(int nMapID);
	void AddAllMap();
};


class MBaseGameTypeCatalogue
{
private:
	MMatchGameTypeInfo			m_GameTypeInfo[MMATCH_GAMETYPE_MAX];
public:
	MBaseGameTypeCatalogue();
	virtual ~MBaseGameTypeCatalogue();

	inline MMatchGameTypeInfo* GetInfo(MMATCH_GAMETYPE nGameType);
	inline const char* GetGameTypeStr(MMATCH_GAMETYPE nGameType);
	inline void SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName);
	inline bool IsCorrectGameType(const int nGameTypeID);	///< �ùٸ� ���� Ÿ�� ID����E����
	inline bool IsTeamGame(MMATCH_GAMETYPE nGameType);		///< �ش�E����Ÿ���� ����E���E����
	inline bool IsTeamExtremeGame(MMATCH_GAMETYPE nGameType); // Custom: IsTeamExtremeGame
	inline bool IsTeamLimitTime(MMATCH_GAMETYPE nGameType);
	inline bool IsSpyGame(MMATCH_GAMETYPE nGameType);
	inline bool IsDropMagic(MMATCH_GAMETYPE nGameType);
	inline bool IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType);		///< ���сE��������݁E��E��E�ؾ��ϴ���E����
	inline bool IsQuestOnly(MMATCH_GAMETYPE nGameType);	///< �ش�E����Ÿ���� ����Ʈ����E����
	inline bool IsSurvivalOnly(MMATCH_GAMETYPE nGameType); ///< �ش�E����Ÿ���� �����̹�����E����
	inline bool IsQuestChallengeOnly(MMATCH_GAMETYPE nGameType); // Custom: CQ fixes
	inline bool IsQuestDerived(MMATCH_GAMETYPE nGameType);	///< �ش�E����Ÿ���� ����Ʈ ��E�(NPC�� ��������E����E����
	inline bool IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType);	///< �ش�E����Ÿ�Կ��� ����������� �����Ǵ���E����
};


//////////////////////////////////////////////////////////////////////////////////
inline bool MBaseGameTypeCatalogue::IsTeamGame(MMATCH_GAMETYPE nGameType)
{
	// ����E����������EDescription�� teamgame����E���ΰ��� �ֵ��� ����.

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||
		(nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_CTF) ||
		(nGameType == MMATCH_GAMETYPE_INFECTED)||
		(nGameType == MMATCH_GAMETYPE_SPY))
	{
		return true;
	}
	return false;
}

inline bool MBaseGameTypeCatalogue::IsTeamLimitTime(MMATCH_GAMETYPE nGameType)
{
	// ����E����������EDescription�� teamgame����E���ΰ��� �ֵ��� ����.

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||
		(nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_INFECTED)||
		(nGameType == MMATCH_GAMETYPE_SPY))
	{
		return true;
	}
	return false;
}

inline bool MBaseGameTypeCatalogue::IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType)
{
	// Run until standby mode??

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_DUEL) ||
		(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||
		(nGameType == MMATCH_GAMETYPE_DUELTOURNAMENT) ||
		(nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_INFECTED)||
		(nGameType == MMATCH_GAMETYPE_SPY))
	{
		return true;
	}
	return false;
}


inline bool MBaseGameTypeCatalogue::IsQuestDerived(MMATCH_GAMETYPE nGameType)
{
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) ||(nGameType == MMATCH_GAMETYPE_QUEST) )
	{
		return true;
	}

	return false;
}

inline bool MBaseGameTypeCatalogue::IsQuestOnly(MMATCH_GAMETYPE nGameType)
{
	return nGameType == MMATCH_GAMETYPE_QUEST;
}

inline bool MBaseGameTypeCatalogue::IsSurvivalOnly(MMATCH_GAMETYPE nGameType)
{
	return nGameType == MMATCH_GAMETYPE_SURVIVAL;
}

inline bool MBaseGameTypeCatalogue::IsQuestChallengeOnly(MMATCH_GAMETYPE nGameType)
{
	return nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE;
}


inline const char* MBaseGameTypeCatalogue::GetGameTypeStr(MMATCH_GAMETYPE nGameType)
{
	return m_GameTypeInfo[nGameType].szGameTypeStr;
}

inline void MBaseGameTypeCatalogue::SetGameTypeStr(MMATCH_GAMETYPE nGameType, const char* szName)
{
	strcpy( m_GameTypeInfo[nGameType].szGameTypeStr, szName) ;
}

bool MBaseGameTypeCatalogue::IsCorrectGameType(const int nGameTypeID)
{
	if ((nGameTypeID < 0) || (nGameTypeID >= MMATCH_GAMETYPE_MAX)) return false;
	return true;
}

inline MMatchGameTypeInfo* MBaseGameTypeCatalogue::GetInfo(MMATCH_GAMETYPE nGameType)
{
	//_ASSERT((nGameType >= 0) && (nGameType < MMATCH_GAMETYPE_MAX));
	return &m_GameTypeInfo[nGameType];
}

inline bool MBaseGameTypeCatalogue::IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType)
{
	if ( (nGameType == MMATCH_GAMETYPE_SURVIVAL) || (nGameType == MMATCH_GAMETYPE_QUEST) || (nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE))
	{
		return false;
	}

	return true;

}
inline bool MBaseGameTypeCatalogue::IsTeamExtremeGame(MMATCH_GAMETYPE nGameType)
{
	// ���� ���������� Description�� teamgame���� ���ΰ��� �ֵ��� ����.

	if ((nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||
		(nGameType == MMATCH_GAMETYPE_CTF))
	{
		return true;
	}
	return false;
}
inline bool MBaseGameTypeCatalogue::IsSpyGame(MMATCH_GAMETYPE nGameType)
{
	if (nGameType == MMATCH_GAMETYPE_SPY)
		return true;

	return false;
}
inline bool MBaseGameTypeCatalogue::IsDropMagic(MMATCH_GAMETYPE nGameType)
{
	if (nGameType == MMATCH_GAMETYPE_DROPMAGIC)
		return true;

	return false;
}
#endif