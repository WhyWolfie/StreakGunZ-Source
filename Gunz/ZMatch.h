#ifndef _ZMATCH_H
#define _ZMATCH_H

#include <list>
using namespace std;

#include "ZPrerequisites.h"
#include "MUID.h"
#include "MMatchStage.h"

class ZCharacter;
class MMatchStageSetting;
class ZRule;

/// 클라이언트 게임 톩E컖E� 클래스
class ZMatch
{
private:
protected:
	ZRule*					m_pRule;
	MMatchStageSetting*		m_pStageSetting;
	MMATCH_ROUNDSTATE		m_nRoundState;
	int						m_nCurrRound;
	int						m_nTeamScore[MMT_END];
	int						m_nRoundKills;			// 현재라웝珞의 킬펯E- FirstKill체크하려컖E만탛E
	int						m_nTeamKillCount[MMT_END]; // 각 팀의 킬 펯E총합

	unsigned long			m_nNowTime;
	unsigned long			m_nStartTime;
	unsigned long int		m_nLastDeadTime;
	int						m_nSoloSpawnTime;

	DWORD					m_dwStartTime;			// 라웝珞를 시작했을 때의 시간값

	void SoloSpawn();				///< 내 캐릭터 스�갋
	void InitCharactersProperties();	///< 캐릭터 HP, 총알 탛E초기화
	void InitRound();				///< 라웝�갋처음 시작시 초기화
	void ProcessRespawn();
public:
	ZMatch();
	virtual ~ZMatch();
	bool Create();
	void Destroy();

	void Update(float fDelta);
	bool OnCommand(MCommand* pCommand);
	void OnResponseRuleInfo(MTD_RuleInfo* pInfo);
	void SetRound(int nCurrRound);						///< 현픸E라웝�갋설정
	void OnForcedEntry(ZCharacter* pCharacter);			///< 난입했을 경퓖E
	void InitCharactersPosition();						///< 라웝�갋시작할때 캐릭터 위치 설정
	void OnDrawGameMessage();
	void RespawnSolo();

	int GetRemainedSpawnTime();
	int GetRoundCount(); 
	int GetRoundReadyCount(void);
	void GetTeamAliveCount(int* pnRedTeam, int* pnBlueTeam);
	const char* GetTeamName(int nTeamID);
	void SetRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg=0);

	void SetRoundStartTime( DWORD dwElapsed ); // Custom: dwElapsed
	DWORD GetRoundStartTime() { return m_dwStartTime; }
	DWORD GetMatchTime(void);
	DWORD GetRemaindTime( void);

	inline int GetCurrRound();
	inline bool IsTeamPlay();
	inline bool IsWaitForRoundEnd();
	inline bool IsQuestDrived();
	inline MMATCH_ROUNDSTATE GetRoundState();
	inline MMATCH_GAMETYPE GetMatchType();
	inline bool GetTeamKillEnabled();
	inline const char* GetMapName();
	inline int GetTeamScore(MMatchTeam nTeam);
	inline void SetTeamScore(MMatchTeam nTeam, int nScore);
	inline int GetTeamKills(MMatchTeam nTeam);		// 팀의 총합킬펯E
	inline void AddTeamKills(MMatchTeam nTeam, int amount = 1);
	inline void SetTeamKills(MMatchTeam nTeam, int amount);
	inline int GetRoundKills();
	inline void AddRoundKills();
	inline bool IsRuleGladiator();
	inline bool IsRuleSpy();
	inline bool IsQuestChallengue();
	inline ZRule* GetRule();
};



#define DEFAULT_ONETURN_GAMETIME		300000		// 한턴 기본 큱E蒐챨� = 5분
#define DEFAULT_WAITTIME				120000		// 기본 큱E갋시간 = 2분
#define DEFAULT_READY_TIME				5000		// 준틒E시간


// inline func ////////////////////////////////////////////////////////////////////////
inline int ZMatch::GetCurrRound() 
{ 
	return m_nCurrRound; 
}
inline bool ZMatch::IsTeamPlay() 
{ 
	return m_pStageSetting->IsTeamPlay(); 
}

inline bool ZMatch::IsWaitForRoundEnd() 
{ 
	return m_pStageSetting->IsWaitforRoundEnd(); 
}

inline MMATCH_ROUNDSTATE ZMatch::GetRoundState()
{ 
	return m_nRoundState; 
}
inline MMATCH_GAMETYPE ZMatch::GetMatchType()
{ 
	return m_pStageSetting->GetStageSetting()->nGameType; 
}
inline bool ZMatch::GetTeamKillEnabled()
{ 
	return m_pStageSetting->GetStageSetting()->bTeamKillEnabled; 
}
inline const char* ZMatch::GetMapName()
{
	return m_pStageSetting->GetStageSetting()->szMapName;
}
inline int ZMatch::GetTeamScore(MMatchTeam nTeam)
{ 
	return m_nTeamScore[nTeam]; 
}

inline int ZMatch::GetTeamKills(MMatchTeam nTeam)
{ 
	return m_nTeamKillCount[nTeam]; 
}

inline void ZMatch::AddTeamKills(MMatchTeam nTeam, int amount)
{ 
	m_nTeamKillCount[nTeam]+=amount; 
}

inline void ZMatch::SetTeamKills(MMatchTeam nTeam, int amount)
{ 
	m_nTeamKillCount[nTeam]=amount; 
}

inline void ZMatch::SetTeamScore(MMatchTeam nTeam, int nScore)
{ 
	m_nTeamScore[nTeam] = nScore; 
}
inline int ZMatch::GetRoundKills()
{ 
	return m_nRoundKills; 
}
inline void ZMatch::AddRoundKills()
{ 
	m_nRoundKills++; 
}
inline bool ZMatch::IsRuleGladiator()
{
	return ((GetMatchType() == MMATCH_GAMETYPE_GLADIATOR_SOLO) || 
			(GetMatchType() == MMATCH_GAMETYPE_GLADIATOR_TEAM));
}
inline ZRule* ZMatch::GetRule() 
{ 
	return m_pRule; 
}

inline bool ZMatch::IsQuestDrived()
{
	return m_pStageSetting->IsQuestDrived();
}

inline bool ZMatch::IsRuleSpy()
{
	return (GetMatchType() == MMATCH_GAMETYPE_SPY);
}
inline bool ZMatch::IsQuestChallengue()
{
	return (GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE);
}

#endif