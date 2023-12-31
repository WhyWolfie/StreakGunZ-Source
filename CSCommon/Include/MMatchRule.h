#ifndef _MMATCHRULE_H
#define _MMATCHRULE_H

#include "MMatchItem.h"
#include "MMatchTransDataType.h"
#include "MUID.h"
#include "MMatchGameType.h"
#include "MQuestPlayer.h"
#include "MMatchEventManager.h"

class MMatchObject;
class MMatchStage;

enum RUN_CP
{
	RUN_CP_NONE,
	RUN_CP_ONE,
	RUN_CP_TWO,
	RUN_CP_THREE,
	RUN_CP_FOUR,

	RUN_CP_MAX,
};


/// 게임 라운드 상태
enum MMATCH_ROUNDSTATE {
	MMATCH_ROUNDSTATE_PREPARE		= 0,			///< 준비 단계
	MMATCH_ROUNDSTATE_COUNTDOWN		= 1,			///< 플레이 카운트 
	MMATCH_ROUNDSTATE_PLAY			= 2,			///< 게임중
	MMATCH_ROUNDSTATE_FINISH		= 3,			///< 종료
	MMATCH_ROUNDSTATE_EXIT			= 4,			///< 대기방으로 나감
	MMATCH_ROUNDSTATE_FREE			= 5,			///< 게임진행에 필요한 요건이 충족되지 않은 프리모드 상태
	MMATCH_ROUNDSTATE_FAILED		= 6,			///< 게임클리어 실패.
	MMATCH_ROUNDSTATE_PRE_COUNTDOWN = 7,			///< 플레이 프리 카운트 다운(일단, 듀얼토너먼트에서 사용)

	MMATCH_ROUNDSTATE_END	// Not using, just a END mark
};

enum MMATCH_ROUNDRESULT {
	MMATCH_ROUNDRESULT_DRAW = 0,
	MMATCH_ROUNDRESULT_REDWON,
	MMATCH_ROUNDRESULT_BLUEWON,
	MMATCH_ROUNDRESULT_RED_ALL_OUT,   // 클랜모드에서 이기고 있는쪽이 다 나가버린경우
	MMATCH_ROUNDRESULT_BLUE_ALL_OUT,
	MMATCH_ROUNDRESULT_END
};


///////////////////////////////////////////////////////////////////////////////////////////////
/// 게임 룰 기본 베이스 클래스
class MMatchRule {
protected:
	// MMatchGameTypeInfo*	m_pGameTypeInfo;							///< 게임 타입 정보

	MMatchStage*		m_pStage;									///< 스테이지 클래스
	MMATCH_ROUNDSTATE	m_nRoundState;								///< 현재 라운드 상태
	int					m_nRoundCount;								///< 라운드 수
	int					m_nRoundArg;								///< 라운드의 추가 인자
	unsigned long		m_tmRoundStateTimer;
	int					m_nLastTimeLimitAnnounce;					// 60, 30, 10 중 하나

	MMatchEventManager m_OnBeginEventManager;						/// 게임을 시작할때 이벤트.
	MMatchEventManager m_OnGameEventManager;						/// 베틀중 이벤트.
	MMatchEventManager m_OnEndEventManager;							/// 게임이 끝날때 이벤트.

protected:
	virtual bool RoundCount() { return false; }						///< 라운드 카운트. 모든 라운드가 끝나면 false를 반환한다.
	virtual bool OnRun();											///< 게임틱시 호출
	virtual void OnBegin();											///< 전체 게임 시작시 호출
	virtual void OnEnd();											///< 전체 게임 종료시 호출
	virtual void OnRoundBegin();									///< 라운드 시작할 때 호출
	virtual void OnRoundEnd();										///< 라운드 끝날 때 호출
	virtual void OnRoundTimeOut();									///< 라운드가 타임아웃으로 종료될 떄 OnRoundEnd() 전이다.
	virtual void OnRoundCountDown();									///< 라운드가 타임아웃으로 종료될 떄 OnRoundEnd() 전이다.
	virtual bool OnCheckRoundFinish() = 0;							///< 라운드가 끝났는지 체크
	virtual bool OnCheckEnableBattleCondition() { return true; }	///< 게임 가능한지 체크
	virtual bool OnCheckBattleTimeOut(unsigned int tmTimeSpend);	///< 라운드 타임아웃인지 체크

	void SetRoundStateTimer(unsigned long tmTime)	{ m_tmRoundStateTimer = tmTime; }
	void InitRound();												///< 새로운 라운드 초기화
public:
	void SetRoundState(MMATCH_ROUNDSTATE nState);					///< 라운드 상태 변경
protected:

	void InitOnBeginEventManager();
	void InitOnGameEventManager();
	void InitOnEndEventManager();

	void CheckOnBeginEvent();
	void CheckOnGameEvent();
	void CheckOnEndEvent();

	void RunOnBeginEvent();
	void RunOnGameEvent();
	void RunOnEndEvent();
public:
	MMatchRule()					{ _ASSERT(false); }				///< 이 생성자는 절대 사용하지 않는다.
	MMatchRule(MMatchStage* pStage);								///< 생성자
	virtual ~MMatchRule()			{}								///< 소멸자
	MMatchStage* GetStage()			{ return m_pStage; }			///< 스테이지 반환

	int GetRoundCount()				{ return m_nRoundCount; }		///< 총 라운드 수 반환
	void SetRoundCount(int nRound)	{ m_nRoundCount = nRound; }		///< 총 라운드 수 설정
	int GetRoundArg()				{ return m_nRoundArg; }			///< 라운드 인자 반환
	void SetRoundArg(int nArg)		{ m_nRoundArg = nArg; }			///< 라운드 인자 설정

	MMatchEventManager& GetOnBeginEventManager()	{ return m_OnBeginEventManager; }
	MMatchEventManager& GetOnGameEventManager()		{ return m_OnGameEventManager; }
	MMatchEventManager& GetOnEndEventManager()		{ return m_OnEndEventManager; }

	MMATCH_ROUNDSTATE GetRoundState()	{ return m_nRoundState; }				///< 라운드 상태 반환
	unsigned long GetRoundStateTimer()	{ return m_tmRoundStateTimer; }
	unsigned long GetLastTimeLimitAnnounce()	{ return m_nLastTimeLimitAnnounce; }
	void SetLastTimeLimitAnnounce(int nSeconds)	{ m_nLastTimeLimitAnnounce = nSeconds; }

	virtual void* CreateRuleInfoBlob()		{ return NULL; }

	/// 팀 보너스 계산
	/// @param pAttacker		공격자
	/// @param pVictim			피해자
	/// @param nSrcExp			원래 경험치
	/// @param poutAttackerExp	공격자가 받을 경험치
	/// @param poutTeamExp		팀이 받을 경험치
	virtual void CalcTeamBonus(MMatchObject* pAttacker,
		                       MMatchObject* pVictim,
							   int nSrcExp,
							   int* poutAttackerExp,
							   int* poutTeamExp);
	/// 게임중 난입할때 호출된다.
	virtual void OnEnterBattle(MUID& uidChar) {}
	// Custom: virtual function for QuestEnterBattle
	virtual void OnQuestEnterBattle(MUID& uidChar) {}	
	virtual void OnQuestStageLaunch() {}	
	// 게임중 나갔을때 호출된다.
	virtual void OnLeaveBattle(MUID& uidChar) {}		
	/// 해당룰에서만 사용하는 커맨드는 직접 처리한다.
	virtual void OnCommand(MCommand* pCommand) {}		
	/// 월드아이템 먹었을 경우 호출된다.
	virtual void OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues) {}
	/// Kill시 호출
	virtual void OnGameKill(const MUID& uidAttacker, const MUID& uidVictim) {}
	// Custom: RUn
	virtual void HasHitCheckPoint(const MUID& uidChar, RUN_CP runCP, MMatchTeam nTeam) {}

	bool Run();														///< 게임틱
	void Begin();													///< 시작
	void End();														///< 끝

	void DebugTest();												///< 디버그 테스트

	virtual bool CheckPlayersAlive() { return true; }
	virtual void OnFailed() {}
	virtual MMATCH_GAMETYPE GetGameType() = 0;
};


///////////////////////////////////////////////////////////////////////////////////////////////








inline bool IsGameRuleDeathMatch(MMATCH_GAMETYPE nGameType)
{
	return (
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) || 
		(nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||
		(nGameType == MMATCH_GAMETYPE_TRAINING) ||
		(nGameType == MMATCH_GAMETYPE_DROPMAGIC) ||
		(nGameType == MMATCH_GAMETYPE_TEAM_TRAINING)
	);
}
inline bool IsGameRuleGladiator(MMATCH_GAMETYPE nGameType)
{
	return ((nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) || 
			(nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM));
}

inline bool IsGameRuleCTF(MMATCH_GAMETYPE nGameType)
{
	return (nGameType == MMATCH_GAMETYPE_CTF);
}

#endif