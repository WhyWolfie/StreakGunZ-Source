#ifndef _MBASEGAMETYPE_H
#define _MBASEGAMETYPE_H

#include <crtdbg.h>
#include <map>
#include <set>
using namespace std;


/// 게임 타입
enum MMATCH_GAMETYPE {
	MMATCH_GAMETYPE_DEATHMATCH_SOLO		=0,			///< 개인 데쓰매치
	MMATCH_GAMETYPE_DEATHMATCH_TEAM		=1,			///< 팀 데쓰매치
	MMATCH_GAMETYPE_GLADIATOR_SOLO		=2,			///< 개인 글래디에이터
	MMATCH_GAMETYPE_GLADIATOR_TEAM		=3,			///< 팀 글래디에이터
	MMATCH_GAMETYPE_ASSASSINATE			=4,			///< 보스픸E암퍊E갋
	MMATCH_GAMETYPE_TRAINING			=5,			///< 연습

	MMATCH_GAMETYPE_SURVIVAL			=6,			///< 서바이퉩E
	MMATCH_GAMETYPE_QUEST				=7,			///< 퀘스트

	MMATCH_GAMETYPE_BERSERKER			=8,			///< 데쓰매치 버서커
	MMATCH_GAMETYPE_DEATHMATCH_TEAM2	=9,			///< 팀데쓰매치 익스트림
	MMATCH_GAMETYPE_DUEL				=10,		///< 듀푳E매치
	MMATCH_GAMETYPE_DUELTOURNAMENT		=11,		///< 듀푳E토너먼트
	MMATCH_GAMETYPE_QUEST_CHALLENGE		=12,		///< 챌린지퀘스트

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
// 모드가 추가되툈E리플레이 저장퓖E모탛E약엉祁 추가해야 한다 (cpp)
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

// 디폴트 게임 타입
const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
// const MMATCH_GAMETYPE MMATCH_GAMETYPE_DEFAULT = MMATCH_GAMETYPE_DEATHMATCH_TEAM2;


struct MMatchGameTypeInfo
{
	MMATCH_GAMETYPE		nGameTypeID;			// ID
	char				szGameTypeStr[24];		// 게임 타입 이름
	float				fGameExpRatio;			// 경험치 배분 비율 - 현픸EGladiator만 50%이다
	float				fTeamMyExpRatio;		// 팀픸E【� 개인에게 적퓖E풔� 경험치
	float				fTeamBonusExpRatio;		// 팀픸E【� 팀에게 적립되는 경험치
	set<int>			MapSet;					// 이 게임타입에서 플레이 가능한 맵
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
	inline bool IsCorrectGameType(const int nGameTypeID);	///< 올바른 게임 타입 ID인햨E여부
	inline bool IsTeamGame(MMATCH_GAMETYPE nGameType);		///< 해큱E게임타입이 팀픸E适갋여부
	inline bool IsTeamExtremeGame(MMATCH_GAMETYPE nGameType); // Custom: IsTeamExtremeGame
	inline bool IsTeamLimitTime(MMATCH_GAMETYPE nGameType);
	inline bool IsSpyGame(MMATCH_GAMETYPE nGameType);
	inline bool IsDropMagic(MMATCH_GAMETYPE nGameType);
	inline bool IsWaitForRoundEnd(MMATCH_GAMETYPE nGameType);		///< 라웝�갋끝날때깩�갋큱E綬醍갋해야하는햨E여부
	inline bool IsQuestOnly(MMATCH_GAMETYPE nGameType);	///< 해큱E게임타입이 퀘스트인햨E여부
	inline bool IsSurvivalOnly(MMATCH_GAMETYPE nGameType); ///< 해큱E게임타입이 서바이벌인햨E여부
	inline bool IsQuestChallengeOnly(MMATCH_GAMETYPE nGameType); // Custom: CQ fixes
	inline bool IsQuestDerived(MMATCH_GAMETYPE nGameType);	///< 해큱E게임타입이 퀘스트 컖E�(NPC가 나오는햨E인햨E여부
	inline bool IsWorldItemSpawnEnable(MMATCH_GAMETYPE nGameType);	///< 해큱E게임타입에서 월드아이템이 스폰되는햨E여부
};


//////////////////////////////////////////////////////////////////////////////////
inline bool MBaseGameTypeCatalogue::IsTeamGame(MMATCH_GAMETYPE nGameType)
{
	// 좀큱E복잡해지툈EDescription에 teamgame인햨E여부값을 넣도록 하자.

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
	// 좀큱E복잡해지툈EDescription에 teamgame인햨E여부값을 넣도록 하자.

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
	// 좀더 복잡해지면 Description에 teamgame인지 여부값을 넣도록 하자.

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