#ifndef _MMATCHRULEQUESTCHALLENGE_H
#define _MMATCHRULEQUESTCHALLENGE_H

#include "MMatchRule.h"
#include "MActorDef.h"
#include "MNewQuestScenario.h"
#include "MMatchQuestGameLog.h"


class MNewQuestPlayerManager;
class MNewQuestNpcManager;

class MMatchRuleQuestChallenge : public MMatchRule
{
	static MActorDefManager ms_actorDefMgr;
	static MNewQuestScenarioManager ms_scenarioMgr;
public:
	static MNewQuestScenarioManager* GetScenarioMgr();

private:

	MNewQuestPlayerManager* m_pPlayerMgr;
	MNewQuestNpcManager* m_pNpcMgr;
	std::map<int, string> m_mapNPCBitFlags;

	MNewQuestScenario* m_pScenario;
	MMatchQuestChallengeGameLogInfoManager	m_QuestChallengeGameLogInfoMgr;
	int m_nCurrSector;		// ���� �������� ����

protected:
	virtual bool RoundCount();										///< ���� ī��Ʈ. ��� ���尡 ������ false�� ��ȯ�Ѵ�.
	virtual void OnBegin();											///< ��ü ���� ���۽� ȣ��
	virtual void OnEnd();											///< ��ü ���� ����� ȣ��
	virtual void OnRoundBegin();									///< ���� ������ �� ȣ��
	virtual void OnRoundEnd();										///< ���� ���� �� ȣ��
	virtual bool OnRun();											///< ����ƽ�� ȣ��
	virtual bool CheckPlayersAlive();
	virtual void OnLeaveBattle(MUID& uidChar);
	virtual bool OnCheckEnableBattleCondition(); // Custom: Emulating maiets roundstates for CQ
	DWORD m_dwCurrTime; // Custom: Log the game time
	//virtual void ProcessNPCSpawn() {}						///< NPC �����۾�
	//virtual bool CheckNPCSpawnEnable() { return true; }					///< NPC�� ���� �������� ����
	//virtual void RouteGameInfo() {}							///< Ŭ���̾�Ʈ�� ���� ���� �����ش�.
	//virtual void RouteStageGameInfo() {}					///< ����� ������������ �ٲ� ���� ������ �����ش�.

	////virtual void OnCompleted();					///< ����Ʈ ������ ȣ��ȴ�.
	////virtual void OnFailed();					///< ����Ʈ ���н� ȣ��ȴ�.
	//virtual void RouteCompleted() {}			///< ����Ʈ ���� �޽����� ������. - ��������� �Բ� ������.
	//virtual void RouteFailed() {}				///< ����Ʈ ���� �޽��� ������.
	//virtual void DistributeReward() {}			///< ����Ʈ ������ ������ ���

	//virtual void OnRequestMovetoPortal(const MUID& uidPlayer) {}	/// �÷��̾ ��Ż�� �̵����� ��� ȣ��ȴ�.
	//virtual void OnReadyToNewSector(const MUID& uidPlayer) {}		/// ��Ż�� �̵��ϰ� ���� �̵��� �Ϸ�Ǿ��� ��� ȣ��ȴ�.

	//virtual void OnRequestTestSectorClear() {}
	//virtual void OnRequestTestFinish() {}

protected:
	virtual bool OnCheckRoundFinish();							///< ���尡 �������� üũ

public:
	virtual void OnQuestEnterBattle(MUID& uidChar);
	virtual MMATCH_GAMETYPE GetGameType() { return MMATCH_GAMETYPE_QUEST_CHALLENGE; }
	virtual void OnObtainWorldItem(MMatchObject* pObj, int nItemID, int* pnExtraValues);
	bool OnObtainWorldItemRandom(int nItemID, int* pnExtraValues);
	void RouteObtainZItem(MUID uidRef, unsigned long int nItemID);
	DWORD GetScenarioTime() { return m_dwCurrTime; };
	void PostCQGameLog();
	void PostInsertQuestChallengeGameLogAsyncJob();
	void CollectEndQuestChallengeGameLogInfo();
	void CollectStartingQuestChallengeGameLogInfo();
public:
	MMatchRuleQuestChallenge(MMatchStage* pStage);
	virtual ~MMatchRuleQuestChallenge();

	virtual void OnCommand(MCommand* pCommand);

private:
	void RouteSpawnNpc(MUID uidNPC, MUID uidController, const char* szNpcDefName, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bIsBoss);
	void RouteSpawnNpcSummon(MUID uidNPC, MUID uidController, int num, const char* szNpcDefName, MShortVector sVec, MShortVector dir);
	void RouteSpawnNpcSingle(MUID listener, MUID uidNPC, MUID uidController, const char* szNpcDefName, int nCustomSpawnTypeIndex, int nSpawnIndex, bool bIsBoss);
	void RouteNpcDead(MUID uidNPC, MUID uidKiller);
	void RouteMoveToNextSector(int nSectorID);
	void RouteMoveToNextSectorSingle(MUID listener, int nSectorID);
	void RouteExpToPlayers();

	void ProcessNpcSpawning();
	void SpawnNpcsForSingle(MUID playerUid);
	void SpawnNpc(const char* szActorDef, int nCustomSpawnTypeIndex, int nSpawnIndex, int nDropItemId, bool bIsBoss);
	void SpawnNpcSummon(const char* szActorDef, int num, MShortVector nSpawnPos, MShortVector nSpawnDir);
	void DropItemByNpcDead(const MUID& uidKiller, int nWorldItemId, const MVector& pos);
};

#endif