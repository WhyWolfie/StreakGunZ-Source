#include "stdafx.h"
#include "MMatchServer.h"
#include "MSharedCommandTable.h"
#include "MErrorTable.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "MMatchObject.h"
#include "MMatchItem.h"
#include "MAgentObject.h"
#include "MMatchNotify.h"
#include "Msg.h"
#include "MMatchObjCache.h"
#include "MMatchStage.h"
#include "MMatchTransDataType.h"
#include "MMatchFormula.h"
#include "MMatchConfig.h"
#include "MCommandCommunicator.h"
#include "MMatchShop.h"
#include "MMatchTransDataType.h"
#include "MDebug.h"
#include "MMatchAuth.h"
#include "MMatchStatus.h"
#include "MAsyncDBJob.h"
#include "MLadderMgr.h"
#include "MTeamGameStrategy.h"




/////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////
int MMatchServer::ValidateChallengeLadderGame(MMatchObject** ppMemberObject, int nMemberCount)
{
	MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	if (pTeamGameStrategy)
	{
		int nRet = pTeamGameStrategy->ValidateChallenge(ppMemberObject, nMemberCount);
		return nRet;
	};

	return MOK;
}

///////////////////////////////////////////////////////////////////////////
// LadderStage
bool MMatchServer::LadderJoin(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return false;

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	// Custom: Clan war glitch fix
	if (!pObj->GetCharInfo() || (pObj->GetCharInfo() && pObj->GetCharInfo()->m_bIsLadderMatching == false))
		return false;

	pObj->GetCharInfo()->m_bIsLadderMatching = false;
	pObj->OnStageJoin();

	// Join
	pStage->AddObject(uidPlayer, pObj);
	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_READY);
	pObj->SetLadderChallenging(false);

	// Custom: Ladder Rejoin
	if (pStage->CheckLadderList(pObj->GetCharInfo()->m_nCID))
	{
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pObj);
		MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_ADD, this);
		RouteToStage(pStage->GetUID(), pCmdCacheAdd);
	}

	pObj->SetLadderStageUID(uidStage);
	pObj->SetLadderTeam(nTeam);

	pStage->PlayerTeam(uidPlayer, nTeam);
	pStage->PlayerState(uidPlayer, MOSS_READY);
	

	MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_PREPARE, uidPlayer);
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamInt(nTeam));
	Post(pCmd);
	pStage->SetCLID(pObj->GetCharInfo()->m_ClanInfo.m_nClanID, nTeam);
	return true;
}


void MMatchServer::LadderGameLaunch(MLadderGroup* pGroupA, MLadderGroup* pGroupB,int MapID)
{
	if ((MGetServerConfig()->GetServerMode() != MSM_LADDER) && 
		(MGetServerConfig()->GetServerMode() != MSM_CLAN)) return;

	MUID uidStage = MUID(0,0);
	if (StageAdd(NULL, "LADDER_GAME", true, "", &uidStage) == false) {
		// Group ��ü
		GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
		GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		return;
	}
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) {
		// Group ��ü
		GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
		GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		return;
	}

	// A �׷� ����
	for (list<MUID>::iterator i=pGroupA->GetPlayerListBegin(); i!= pGroupA->GetPlayerListEnd(); i++)
	{
		MUID uidPlayer = (*i);
		MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
		if (pObj)
		{
			pObj->PlayerWarsIdentifier = -1;
			pObj->LastVoteID = -1;
			LadderJoin(uidPlayer, uidStage, MMT_RED);
		}
	}
	// B �׷� ����
	for (list<MUID>::iterator i=pGroupB->GetPlayerListBegin(); i!= pGroupB->GetPlayerListEnd(); i++)
	{
		MUID uidPlayer = (*i);
		MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
		if (pObj)
		{
			pObj->PlayerWarsIdentifier = -1;
			pObj->LastVoteID = -1;
			LadderJoin(uidPlayer, uidStage, MMT_BLUE);
		}
	}

	// Agent �غ�
//	ReserveAgent(pStage);

	//////////////////////////////////////////////////////////////////////////////
	//int nRandomMap = 0; // Custom: Clean RandomMap for VoteMap
	// Ŭ������ Stage�� �������� CLID���� �����ؾ��Ѵ�.
	MBaseTeamGameStrategy* pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	/*if (pTeamGameStrategy)
	{
		nRandomMap = pTeamGameStrategy->GetRandomMap((int)pGroupA->GetPlayerCount());
	};*/


	MMATCH_GAMETYPE nGameType = (MMATCH_GAMETYPE)pGroupA->GetGameType();
	int TimeLimit = 3 * 60 * 1000; //Update Time
	switch (nGameType)
	{
	case Gladiator:
		nGameType = MMATCH_GAMETYPE_GLADIATOR_TEAM;
		TimeLimit = 5 * 60 * 1000;
		break;
	case Assassinate:
		nGameType = MMATCH_GAMETYPE_ASSASSINATE;
		break;
	default:
		nGameType = MMATCH_GAMETYPE_DEATHMATCH_TEAM;
		break;
	}

	// Game ����
	pStage->SetStageType(MST_LADDER);
	pStage->ChangeRule(nGameType);

	// Ŭ������ Stage�� �������� CLID���� �����ؾ��Ѵ�.
	if (pTeamGameStrategy)
	{
		MMatchLadderTeamInfo a_RedLadderTeamInfo, a_BlueLadderTeamInfo;
		pTeamGameStrategy->SetStageLadderInfo(&a_RedLadderTeamInfo, &a_BlueLadderTeamInfo, pGroupA, pGroupB);

		pStage->SetLadderTeam(&a_RedLadderTeamInfo, &a_BlueLadderTeamInfo);
	};

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	pSetting->SetMasterUID(MUID(0,0));
	pSetting->SetMapIndex(MapID); //VoteMap
	pSetting->SetGameType(nGameType);
	// Custom: 5vs5 clan wars (Fix)
	pSetting->SetMaxPlayers(10);

	pSetting->SetLimitTime(TimeLimit);
	pSetting->SetRoundMax(99);		// �ִ� 99������� ������ �� �ִ�.
	

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);	// Stage Setting ����


	// ��� �α׸� �����.
	//VoteMap
	if ( (MGetMapDescMgr()->MIsCorrectMap(MapID)) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)) )
	{
		if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {		// ���ӽ���
			// Send Launch Command
			ReserveAgent(pStage);

			/////////////////////////////////////////////////////////////////////////////////////////////
			// Ŭ������ ObjectCache�� ���� �������� �ʱ� ������ Stage�� �ϼ��Ǹ� �׶� ������ �ش�.
			// �� ������ Ŭ���̾�Ʈ�鳢�� Peer�� ������ ���� �� ������ ������ ��û�� ������
			//  Ŭ���̾�Ʈ�� ����Ʈ ���������� �ٸ� �� �ֱ� ������ �̶� ������ ���ش�.
			// - by SungE.
			MMatchObjectCacheBuilder CacheBuilder;
			CacheBuilder.Reset();
			for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
				MUID uidObj = (MUID)(*i).first;
				MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
				if (pScanObj) {
					CacheBuilder.AddObject(pScanObj);
				}
			}
			MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
			RouteToStage(pStage->GetUID(), pCmdCacheAdd);
			/////////////////////////////////////////////////////////////////////////////////////////////
			MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(uidStage));
			pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
			pCmd->AddParameter(new MCmdParamBool(false));
			RouteToStage(uidStage, pCmd);

			// Ladder Log �����.
		} else {
			// Group ��ü
			GetLadderMgr()->CancelChallenge(pGroupA->GetID(), "");
			GetLadderMgr()->CancelChallenge(pGroupB->GetID(), "");
		}
	}
}


bool MMatchServer::IsLadderRequestUserInRequestClanMember( const MUID& uidRequestMember
														  , const MTD_LadderTeamMemberNode* pRequestMemberNode )
{
	// - by SungE 2007-10-11 
	// ��Ŀ�� MemberNamesBlob�� ������ �� �ֱ� ������ ��û�� �ڽ��� �̸��� �����ϴ� 0��° �ε��� ��带 
	//  Ŀ�ǵ� ��û�ڿ� ������ �˻��� ��� �Ѵ�. Ŭ���̾�Ʈ�� ������ ������ �������� �ʱ� ���� �߰� �˻��ڵ常 �ۼ�.
	// �� �ڵ�� Ŭ������û�� ���õ� Ŭ���̾�Ʈ �ڵ尡 ������� �ʴ´ٴ� �������ǿ��� �ۼ��� ����.
	// 0�� �ε����� ��尡 ��û���̴�.

	if( NULL == pRequestMemberNode )
		return false;

	MMatchObject* pRequestMemberObj = GetPlayerByName( pRequestMemberNode->szName );
	if( NULL == pRequestMemberObj )
		return false;

	// ��û���� ĳ���Ϳ� MemberNameNode�� 0��° �ε��� ������ UID�� ������ �˻��Ѵ�.

	if( uidRequestMember != pRequestMemberObj->GetUID() )
		return false; ///< ���� ������ ������ ������ ��û���� �Ǵ�.s

	return true;
}

void MMatchServer::OnLadderRequestChallenge(const MUID& uidRequestMember, void* pMemberNamesBlob, unsigned long int nOptions, unsigned long int nGameType)
{
	if ((MGetServerConfig()->GetServerMode() != MSM_LADDER) && 
		(MGetServerConfig()->GetServerMode() != MSM_CLAN)) return;

	MMatchObject* pLeaderObject = GetPlayerByCommUID(uidRequestMember);
	if (! IsEnabledObject(pLeaderObject)) return;

	if (nGameType <= Normal && nGameType != 0 || nGameType == EndGames) { LOG(LOG_PROG, "Invalid ClanWar GameType: %d\n", nGameType); return; }
	/*MMatchChannel* pChannel = FindChannel(pLeaderObject->GetChannelUID());

	if(!memcmp(pChannel->GetName(), pLeaderObject->GetCharInfo()->m_ClanInfo.m_szClanName, sizeof(pLeaderObject->GetCharInfo()->m_ClanInfo.m_szClanName)) == 0)
	{
		RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, MERR_WRONG_CLAN_CHANNEL);
		return;
	}*/


	int nBlobCount = MGetBlobArrayCount(pMemberNamesBlob);
	int nMemberCount = nBlobCount;
	if (nMemberCount <= 0) return;

	if( !IsLadderRequestUserInRequestClanMember(uidRequestMember
		, (MTD_LadderTeamMemberNode*)MGetBlobArrayElement(pMemberNamesBlob, 0)) )
		return;
	
	MMatchObject* pMemberObjects[MAX_CLANBATTLE_TEAM_MEMBER];
	for (int i = 0; i < nMemberCount; i++)
	{
		MTD_LadderTeamMemberNode* pNode = (MTD_LadderTeamMemberNode*)MGetBlobArrayElement(pMemberNamesBlob, i);
		if (pNode == NULL) break;
		if ((strlen(pNode->szName) <= 0) || (strlen(pNode->szName) >= MATCHOBJECT_NAME_LENGTH)) return;

		pMemberObjects[i] = GetPlayerByName(pNode->szName);

		// �Ѹ��̶� �������� ������ �ȵȴ�
		if (! IsEnabledObject(pMemberObjects[i]))
		{
			// �޼��� �����ְ� ��.
			RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, MERR_LADDER_CANNOT_CHALLENGE);
			return;
		}
	}


	int nRet = ValidateChallengeLadderGame(pMemberObjects, nMemberCount);
	if (nRet != MOK)
	{
		RouteResponseToListener(pLeaderObject, MC_MATCH_LADDER_RESPONSE_CHALLENGE, nRet);
		return;
	}

	int nTeamID = 0;

	MBaseTeamGameStrategy* pTeamGameStrategy = NULL;

	pTeamGameStrategy = MBaseTeamGameStrategy::GetInstance(MGetServerConfig()->GetServerMode());
	if (pTeamGameStrategy)
	{
        nTeamID = pTeamGameStrategy->GetNewGroupID(pLeaderObject, pMemberObjects, nMemberCount);
	}
	if (nTeamID == 0) return;

	// ������ Challenge�Ѵ�.
	// Ensure All Player Not in LadderGroup
	MLadderGroup* pGroup = GetLadderMgr()->CreateLadderGroup();
	pGroup->SetID(nTeamID);

	// balancedMatching ����
	if (nOptions == 1)
	{
		pGroup->SetBalancedMatching(true);
	}
	else
	{
		pGroup->SetBalancedMatching(false);	
	}
	pGroup->SetGameType(nGameType);
	if (pTeamGameStrategy)
	{
		pTeamGameStrategy->SetLadderGroup(pGroup, pMemberObjects, nMemberCount);
	}

	for (int i=0; i<nMemberCount; i++) {
		pGroup->AddPlayer( pMemberObjects[i] );

		if( pMemberObjects[i]->GetCharInfo() )
			pMemberObjects[i]->GetCharInfo()->m_bIsLadderMatching = true;
	}

	GetLadderMgr()->Challenge(pGroup);
}

void MMatchServer::OnLadderRequestCancelChallenge(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetLadderGroupID() == 0) return;

	GetLadderMgr()->CancelChallenge(pObj->GetLadderGroupID(), pObj->GetCharInfo()->m_szName);
}

void MMatchServer::OnRequestProposal(const MUID& uidProposer, const int nProposalMode, const int nRequestID, 
		                const int nReplierCount, void* pReplierNamesBlob)
{
	MMatchObject* pProposerObject = GetObject(uidProposer);
	if (! IsEnabledObject(pProposerObject)) return;


	if ((nReplierCount > MAX_REPLIER) || (nReplierCount < 0))
	{
		_ASSERT(0);	// 16���̻� ������ �� ����
		return;
	}


	if (!MGetServerConfig()->IsEnabledCreateLadderGame())
	{
		// �޼��� �����ְ� ��.
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_LADDER_NOT_SERVICE_TIME));
		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(pProposerObject, pNewCmd);
		return;
	}


	int nBlobCount = MGetBlobArrayCount(pReplierNamesBlob);
	if (nBlobCount != nReplierCount) return;

	MMatchObject* ppReplierObjects[MAX_REPLIER];

	for (int i = 0; i < nReplierCount; i++)
	{
		MTD_ReplierNode* pNode = (MTD_ReplierNode*)MGetBlobArrayElement(pReplierNamesBlob, i);
		if (pNode == NULL) return;
		if ((strlen(pNode->szName) <= 0) || (strlen(pNode->szName) >= MATCHOBJECT_NAME_LENGTH)) return;

		ppReplierObjects[i] = GetPlayerByName(pNode->szName);

		// �亯�ڰ� �Ѹ��̶� �������� ������ �ȵȴ�
		if (!IsEnabledObject(ppReplierObjects[i]))
		{
			// �޼��� �����ְ� ��.
			MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
			pNewCmd->AddParameter(new MCommandParameterInt(MERR_NO_TARGET));
			pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
			pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
			RouteToListener(pProposerObject, pNewCmd);

			return;
		}
	}

	int nRet = MERR_UNKNOWN;
	// ��Ȳ�� �°� validate �Ѵ�.

	switch (nProposalMode)
	{
	case MPROPOSAL_LADDER_INVITE:
		{
			MLadderGameStrategy* pLadderGameStrategy = MLadderGameStrategy::GetInstance();
			nRet = pLadderGameStrategy->ValidateRequestInviteProposal(pProposerObject, ppReplierObjects, nReplierCount);
		}
		break;
	case MPROPOSAL_CLAN_INVITE:
		{
			MClanGameStrategy* pClanGameStrategy = MClanGameStrategy::GetInstance();
			nRet = pClanGameStrategy->ValidateRequestInviteProposal(pProposerObject, ppReplierObjects, nReplierCount);
		}
		break;
	};

	if (nRet != MOK)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(nRet));
		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(pProposerObject, pNewCmd);
		return;
	}


	int nMemberCount = nReplierCount+1;		// �����ڱ��� 
	void* pBlobMembersNameArray = MMakeBlobArray(sizeof(MTD_ReplierNode), nMemberCount);

	MTD_ReplierNode* pProposerNode = (MTD_ReplierNode*)MGetBlobArrayElement(pBlobMembersNameArray, 0);
	strcpy(pProposerNode->szName, pProposerObject->GetCharInfo()->m_szName);

	for (int k = 0; k < nReplierCount; k++)
	{
		MTD_ReplierNode* pMemberNode = (MTD_ReplierNode*)MGetBlobArrayElement(pBlobMembersNameArray, k+1);
		strcpy(pMemberNode->szName, ppReplierObjects[k]->GetCharInfo()->m_szName);
	}

	// �亯�ڿ��� ���Ǹ� �����.
	for (int i = 0; i < nReplierCount; i++)
	{
		MCommand* pNewCmd = CreateCommand(MC_MATCH_ASK_AGREEMENT, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterUID(uidProposer));
//		pNewCmd->AddParameter(new MCommandParameterString(pProposerObject->GetCharInfo()->m_szName));
		pNewCmd->AddParameter(new MCommandParameterBlob(pBlobMembersNameArray, MGetBlobArraySize(pBlobMembersNameArray)));

		pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
		pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
		RouteToListener(ppReplierObjects[i], pNewCmd);


	}
	MEraseBlobArray(pBlobMembersNameArray);


	// �����ڿ��� ���� ������
	MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_PROPOSAL, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterInt(nRet));
	pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
	pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
	RouteToListener(pProposerObject, pNewCmd);

}

void MMatchServer::OnReplyAgreement(MUID& uidProposer, MUID& uidReplier, const char* szReplierName, 
		                const int nProposalMode, const int nRequestID, const bool bAgreement)
{
	MMatchObject* pProposerObject = GetObject(uidProposer);
	if (! IsEnabledObject(pProposerObject)) return;

	
	MCommand* pNewCmd = CreateCommand(MC_MATCH_REPLY_AGREEMENT, MUID(0,0));
	pNewCmd->AddParameter(new MCommandParameterUID(uidProposer));
	pNewCmd->AddParameter(new MCommandParameterUID(uidReplier));
	pNewCmd->AddParameter(new MCommandParameterString(szReplierName));
	pNewCmd->AddParameter(new MCommandParameterInt(nProposalMode));
	pNewCmd->AddParameter(new MCommandParameterInt(nRequestID));
	pNewCmd->AddParameter(new MCommandParameterBool(bAgreement));

	RouteToListener(pProposerObject, pNewCmd);	
}

void MMatchServer::OnLadderRejoin(const MUID& uidComm, bool bAcknowledged)
{
	MMatchObject* pObj = GetObject(uidComm);
	if (!IsEnabledObject(pObj))
		return;

	if (!pObj->GetCharInfo()->m_ClanInfo.IsJoined())
		return;

	if (pObj->GetStageUID().IsValid())
		return;

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (!pChannel || pChannel && pChannel->GetChannelType() != MCHANNEL_TYPE_CLAN)
		return;

	bool bJustFound = false;
	if (pObj->GetLadderStageUID() == MUID(0,0) || pObj->GetLadderTeam() == MMT_ALL)
	{
		// did the user relogin?
		bool bFound = false;

		MMatchClan* pClan = FindClan(pObj->GetCharInfo()->m_ClanInfo.m_nClanID);

		if (!pClan)
			return;

		// Don't bother checking
		if (pClan->GetMemberCount() <= 1)
			return;

		for (MUIDRefCache::iterator i=pClan->GetMemberBegin(); i!=pClan->GetMemberEnd(); i++)
		{
			MMatchObject* pClanObj = (MMatchObject*)(*i).second;
			if (!IsEnabledObject(pClanObj))
				continue;

			if (pClanObj->GetUID() == pObj->GetUID() || pClanObj->GetStageUID() == MUID(0,0))
				continue;

			MMatchStage* pClanStage = FindStage(pClanObj->GetStageUID());
			if (!pClanStage)
				continue;

			// check if it's ladder
			if (pClanStage->GetStageType() == MST_LADDER)
			{
				// is this the stage we left?
				if (!pClanStage->CheckLadderList(pObj->GetCharInfo()->m_nCID))
					continue;

				if (pClanStage->GetRule() && pClanStage->GetRule()->GetRoundState() == MMATCH_ROUNDSTATE_EXIT
					|| pClanStage->GetState() == STAGE_STATE_CLOSE)
					continue;

				// check if the game is balanced before reconnecting
				int red = 0;
				int blue = 0;
				pClanStage->GetTeamMemberCount(&red, &blue, NULL, true);

				if (pClanObj->GetTeam() == MMT_RED && red < blue || pClanObj->GetTeam() == MMT_BLUE && blue < red)
				{
					bFound = true;

					if (pClanStage->GetRule()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH)
					{
						if (bAcknowledged)
							Announce(pObj, "Please try again.");
						return;
					}

					pObj->SetLadderStageUID(pClanObj->GetStageUID());
					pObj->SetLadderTeam(pClanObj->GetTeam());
					bJustFound = true;
					break;
				}
			}
		}

		if (!bFound)
		{
			if (bAcknowledged)
				Announce(pObj, "Cannot rejoin clan war match. No existing match found.");
			return;
		}
	}

	MMatchStage* pStage = FindStage(pObj->GetLadderStageUID());
	if (!pStage)
	{
		pObj->SetLadderStageUID(MUID(0,0));
		pObj->SetLadderTeam(MMT_ALL);

		if (bAcknowledged)
			Announce(pObj, "Unable to locate existing clan war match.");

		return;
	}

	if (!bJustFound)
	{
		if (pStage->GetState() == STAGE_STATE_CLOSE)
		{
			if (bAcknowledged)
				Announce(pObj, "Cannot rejoin clan war match. The match has ended.");

			return;
		}

		if (pStage->GetRule() && pStage->GetRule()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH)
		{
			if (bAcknowledged)
				Announce(pObj, "Please try again.");

			return;
		}

		if (pStage->GetRule() && pStage->GetRule()->GetRoundState() == MMATCH_ROUNDSTATE_EXIT)
		{
			if (bAcknowledged)
				Announce(pObj, "Cannot rejoin clan war match. The match is ending.");

			return;
		}

		// check if the game is balanced before reconnecting
		int red = 0;
		int blue = 0;
		pStage->GetTeamMemberCount(&red, &blue, NULL, true);

		if (pObj->GetTeam() == MMT_RED && red >= blue || pObj->GetTeam() == MMT_BLUE && blue >= red)
		{
			if (bAcknowledged)
				Announce(pObj, "Cannot rejoin clan war match. The game is already balanced.");

			return;
		}
	}

	if (bAcknowledged)
	{
		MMatchItem* pPrimary = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMCIP_PRIMARY);
		MMatchItem* pSecondary = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMCIP_SECONDARY);
		MMatchItem* pCustom1 = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMCIP_CUSTOM1);
		MMatchItem* pCustom2 = pObj->GetCharInfo()->m_EquipedItem.GetItem(MMCIP_CUSTOM2);

		if (pPrimary && pPrimary->GetDesc())
		{
			if (pPrimary->GetDesc()->m_nWeaponType.Ref() == MWT_SNIFER)
			{
				Announce( pObj, "Cannot clan war with Sniper(s) equipped." );
				return;
			}
		}

		if (pSecondary && pSecondary->GetDesc())
		{
			if (pSecondary->GetDesc()->m_nWeaponType.Ref() == MWT_SNIFER)
			{
				Announce( pObj, "Cannot clan war with Sniper(s) equipped." );
				return;
			}
		}

		if (pCustom1 && pCustom1->GetDesc())
		{
			// Ban landmines from cw
			if (pCustom1->GetDesc()->m_nID >= 527828 && pCustom1->GetDesc()->m_nID <= 600371)
			{
				Announce( pObj, "Cannot clan war with Landmines equipped." );
				return;
			}
		}

		if (pCustom2 && pCustom2->GetDesc())
		{
			// Ban landmines from cw
			if (pCustom2->GetDesc()->m_nID >= 527828 && pCustom2->GetDesc()->m_nID <= 600371)
			{
				Announce( pObj, "Cannot clan war with Landmines equipped." );
				return;
			}
		}

		pObj->GetCharInfo()->m_bIsLadderMatching = true;
		LadderJoin(uidComm, pObj->GetLadderStageUID(), pObj->GetLadderTeam());

		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.Reset();
		for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
			MUID uidObj = (MUID)(*i).first;
			MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
			if (pScanObj) {
				CacheBuilder.AddObject(pScanObj);
			}
		}
		MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheAdd);

		MCommand* pCmd = CreateCmdResponseStageSetting(pObj->GetLadderStageUID());
		RouteToListener(pObj, pCmd);	// Stage Setting

		pObj->SetForcedEntry(true);
	
		pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, uidComm);
		pCmd->AddParameter(new MCmdParamUID(pObj->GetLadderStageUID()));
		pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
		//pCmd->AddParameter(new MCmdParamBool(false));
		RouteToListener(pObj, pCmd);
	}
	else
	{
		// notify the user
		MCommand* pNew = CreateCommand(MC_MATCH_NOTIFY_LADDER_REJOIN, uidComm);
		RouteToListener(pObj, pNew);
	}
}
