#ifndef _MTEAMGAMESTRATEGY_H
#define _MTEAMGAMESTRATEGY_H

#include "MMatchGlobal.h"
#include <vector>
using namespace std;

class MMatchObject;
class MLadderGroup;
class MMatchStage;

class MLadderGameStrategy;
class MClanGameStrategy;
struct MMatchLadderTeamInfo;

class MBaseTeamGameStrategy
{
protected:
	MBaseTeamGameStrategy() { }
	virtual ~MBaseTeamGameStrategy() { }
public:
	/// ���� �������� üũ�Ѵ�.
	virtual int ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount) = 0;

	/// �������ڰ� �ٸ���������� ������ �� �ִ��� üũ�Ѵ�.
	virtual int ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
					const int nReplierCount) = 0;
	/// ���ο� LadderGroup ID�� �����ؼ� ��ȯ�Ѵ�.
	virtual int GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount) = 0;

	/// LadderGroup�� �ʿ��� ������ �����Ѵ�. ID����..
	virtual void SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount) = 0;

	/// Stage���� �ʿ��� LadderInfo�� �����Ѵ�.
	virtual void SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
									MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup) = 0;

	/// ������ �������� ����� DB �����Ѵ�.
	virtual void SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo, int GameType) = 0;

#ifdef _CW_VOTE
	virtual int GetRandomMap(int nTeamMember) = 0;
	virtual int GetPlayerWarsRandomMap(int nTeamMember) = 0;
#endif // _CW_VOTE

	/// ������忡 ���� ������ �ڽ� Ŭ������ ��ȯ�Ѵ�. MSM_LADDER, MSM_CLAN�� ����
	static MBaseTeamGameStrategy* GetInstance(MMatchServerMode nServerMode);
};


class MLadderGameStrategy : public MBaseTeamGameStrategy
{
protected:
	MLadderGameStrategy() { }
#ifdef _CW_VOTE
	vector<int>		m_RandomMapVec[MLADDERTYPE_MAX];
#endif // _CW_VOTE
public:
	static MLadderGameStrategy* GetInstance()
	{
		static MLadderGameStrategy m_stInstance;
		return &m_stInstance;
	}
	virtual int ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount);
	virtual int ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
					const int nReplierCount);
	virtual int GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount);
	virtual void SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount) { }
	virtual void SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
									MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup);
	virtual void SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo, int GameType);
#ifdef _CW_VOTE
	virtual int GetRandomMap(int nTeamMember);
	virtual int GetPlayerWarsRandomMap(int nTeamMember);
#endif
};


class MClanGameStrategy : public MBaseTeamGameStrategy
{
protected:
	MClanGameStrategy();
	vector<int>		m_RandomMapVec[MLADDERTYPE_MAX];
public:
	static MClanGameStrategy* GetInstance()
	{
		static MClanGameStrategy m_stInstance;
		return &m_stInstance;
	}

	virtual int ValidateChallenge(MMatchObject** ppMemberObject, int nMemberCount);
	virtual int ValidateRequestInviteProposal(MMatchObject* pProposerObject, MMatchObject** ppReplierObjects,
					const int nReplierCount);
	virtual int GetNewGroupID(MMatchObject* pLeaderObject, MMatchObject** ppMemberObjects, int nMemberCount);
	virtual void SetLadderGroup(MLadderGroup* pGroup, MMatchObject** ppMemberObjects, int nMemberCount);
	virtual void SetStageLadderInfo(MMatchLadderTeamInfo* poutRedLadderInfo, MMatchLadderTeamInfo* poutBlueLadderInfo,
									MLadderGroup* pRedGroup, MLadderGroup* pBlueGroup);
	virtual void SavePointOnFinishGame(MMatchStage* pStage, MMatchTeam nWinnerTeam, bool bIsDrawGame,
		                               MMatchLadderTeamInfo* pRedLadderInfo, MMatchLadderTeamInfo* pBlueLadderInfo, int GameType);
#ifdef _CW_VOTE
	virtual int GetRandomMap(int nTeamMember);
	virtual int GetPlayerWarsRandomMap(int nTeamMember);
#endif
};







#endif