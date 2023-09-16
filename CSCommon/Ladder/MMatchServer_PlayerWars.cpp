#include "stdafx.h"
#include "MMatchServer.h"
#include "MAsyncDBJob_PlayerWars.h"
#include "MBlobArray.h"
#include "MCrashDump.h"
#include "MMatchConfig.h"

DWORD PlayerWarsExceptionHandler( PEXCEPTION_POINTERS ExceptionInfo, char* Place )
{
	char szStageDumpFileName[ _MAX_DIR ]= {0,};
	SYSTEMTIME SystemTime;
	GetLocalTime( &SystemTime );
	sprintf( szStageDumpFileName, "Log/StreakLadder[%s]_%d-%d-%d_%d-%d-%d.dmp"
		, Place
		, SystemTime.wYear
		, SystemTime.wMonth
		, SystemTime.wDay
		, SystemTime.wHour
		, SystemTime.wMinute
		, SystemTime.wSecond );

	return CrashExceptionDump( ExceptionInfo, szStageDumpFileName, true );
}

void MMatchServer::UpdatePlayerWarsGame(MMatchStage* pStage, MMatchObject* pObj, MMatchTeam nTeam)
{
//	__try
//	{
	if(!pStage || !pObj) return;
	pObj->GetCharInfo()->m_nCID, (nTeam, pStage->GetUID());
	pObj->OnStageJoin();
	pObj->PlayerWarsIdentifier = -1;
	pObj->LastVoteID = -1;
	pStage->PlayerWarsAdd(pObj->GetCharInfo()->m_nCID, nTeam, pObj->GetUID());
	pStage->AddObject(pObj->GetUID(), pObj);
	pObj->SetStageUID(pStage->GetUID());
	pObj->SetStageState(MOSS_READY);
	pObj->SetLadderChallenging(false);
	pStage->PlayerTeam(pObj->GetUID(), nTeam);
	pStage->PlayerState(pObj->GetUID(), MOSS_READY);
	MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_PREPARE, pObj->GetUID());
	pCmd->AddParameter(new MCmdParamUID(pStage->GetUID()));
	pCmd->AddParameter(new MCmdParamInt(nTeam));
	Post(pCmd);
	pStage->SetCLID(pObj->GetCharInfo()->m_ClanInfo.m_nClanID, nTeam);
//	}
	//__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "UpdatePlayerWarsGame") ){ return;}
}

void MMatchServer::LaunchPlayerWarsGame(PlayerJoiningDataStruct Data, PlayerWars Type, int nMap)
{
	//	__try
	//{
	bool error = false;
	MUID uidStage = MUID(0,0);
	if (StageAdd(NULL, "Streak_Ladder", true, "", &uidStage) == false) {
		error = true;
	}
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) {
		error = true;
	}
	int MaxTeamCount = ceil((float)Data.size() / 2);
	int RedTeamCount = 0, BlueTeamCount = 0;
	if(error == false)
	{
		PlayerJoiningDataStruct Teams(Data);
		for(PlayerJoiningDataStruct::iterator i = Teams.begin(); i != Teams.end();)
		{
			MMatchObject* pObj = GetObject(i->first);
			if (pObj == NULL) { error = true; break;}
			if (pObj->GetStageUID() != MUID(0,0))
				StageLeave(pObj->GetUID());
			MMatchStage* pStage = FindStage(uidStage);
			if (pStage == NULL) { error = true; break;}
			if(i->second != MUID(0, 0))
			{
				MMatchTeam nTeam = (MMatchTeam)RandomNumber(MMT_RED, MMT_BLUE);
				if(nTeam == MMT_RED && (RedTeamCount + 2) <= MaxTeamCount || nTeam == MMT_BLUE && (BlueTeamCount + 2) <= MaxTeamCount)
				{
					UpdatePlayerWarsGame(pStage, pObj, nTeam);
					MMatchObject* pf = GetObject(i->second);
					if(pf)
					{
						UpdatePlayerWarsGame(pStage, pf, nTeam);
						Teams.erase(Teams.find(i->second));
						if(nTeam == MMT_RED)
							RedTeamCount++;
						else
							BlueTeamCount++;
					}
					if(nTeam == MMT_RED)
						RedTeamCount++;
					else
						BlueTeamCount++;
					i = Teams.erase(i++);
				} else i++;
			} else i++;
		}

		for(PlayerJoiningDataStruct::iterator i = Teams.begin(); i != Teams.end();)
		{
			MMatchObject* pObj = GetObject(i->first);
			if (pObj == NULL) { error = true; break;}
			if (pObj->GetStageUID() != MUID(0,0))
				StageLeave(pObj->GetUID());
			MMatchStage* pStage = FindStage(uidStage);
			if (pStage == NULL) { error = true; break;}
			MMatchTeam nTeam = (MMatchTeam)RandomNumber(MMT_RED, MMT_BLUE);
			if(RedTeamCount == MaxTeamCount) nTeam = MMT_BLUE; else if(BlueTeamCount == MaxTeamCount) nTeam = MMT_RED;
			switch (nTeam)
			{
			case MMT_RED:
				RedTeamCount++;
				break;
			case MMT_BLUE:
				BlueTeamCount++;
				break;
			}
			UpdatePlayerWarsGame(pStage, pObj, nTeam);
			i = Teams.erase(i++);
		}
	}
	if(error == true)
	{	
		for(PlayerJoiningDataStruct::iterator i = Data.begin(); i != Data.end(); i++)
		{
			MMatchObject* pObj = GetObject(i->first);
			if (pObj == NULL) continue;
			(pObj->GetCharInfo()->m_nCID);
			GetPlayerWarsMgr()->GetWaitingGames(Type)->AddPlayer(pObj->GetUID());
		}
		pStage->ChangeState(STAGE_STATE_CLOSE);
		return;
	}
	
	ReserveAgent(pStage);
	pStage->SetStageType(MST_PLAYERWARS);
	pStage->ChangeRule(MMATCH_GAMETYPE_DEATHMATCH_TEAM);
	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	int TimeLimit = 3 * 60 * 1000; //Update Time
	if(!pSetting)
	{
		for(PlayerJoiningDataStruct::iterator i = Data.begin(); i != Data.end(); i++)
		{
			MMatchObject* pObj = GetObject(i->first);
			if (pObj == NULL) continue;
			(pObj->GetCharInfo()->m_nCID);
			GetPlayerWarsMgr()->GetWaitingGames(Type)->AddPlayer(pObj->GetUID());
		}
		pStage->ChangeState(STAGE_STATE_CLOSE);
		return;
	}
	pSetting->SetMasterUID(MUID(0,0));
	pSetting->SetMapIndex(nMap);
	pSetting->SetGameType(MMATCH_GAMETYPE_DEATHMATCH_TEAM);
	pSetting->SetLimitTime(TimeLimit);
	pSetting->SetRoundMax(99);
	//pSetting->SetAntiLead(true);
	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);
	if ( (MGetMapDescMgr()->MIsCorrectMap(nMap)) && (MGetGameTypeMgr()->IsCorrectGameType(MMATCH_GAMETYPE_DEATHMATCH_TEAM)) )
	{
		if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {		// 게임시작
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
			MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(uidStage));
			pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
			pCmd->AddParameter(new MCmdParamBool(true));
			RouteToStage(uidStage, pCmd);
		}
	}
//	}
//	__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "LaunchPlayerWarsGame") ){}
}

void MMatchServer::SendPlayerWarsCharInfoToPlayer(MUID uidPlayer)
{
	//	__try
	//{
	MMatchObject *pPWObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pPWObj) == false || !pPWObj) return;
	if(pPWObj->GetPlayerWarsInfo() == NULL)
	{
		if( pPWObj->GetCharInfo() == NULL ) return;
		MAsyncDBJob_GetPlayerWarsCharInfo *pAsyncDbJob = new MAsyncDBJob_GetPlayerWarsCharInfo;
		if( 0 == pAsyncDbJob ) return;
		pAsyncDbJob->Input(uidPlayer, pPWObj->GetCharInfo()->m_nCID);
		MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	} else PostPlayerWarsCharInfo(uidPlayer, pPWObj->GetPlayerWarsInfo());
	//}
	//__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "SendPlayerWarsCharInfoToPlayer") ){}
}


void MMatchServer::OnAsyncGetPlayerWarsCharInfo(MAsyncJob *pJobResult)
{
		__try
	{
	MAsyncDBJob_GetPlayerWarsCharInfo* pJob = (MAsyncDBJob_GetPlayerWarsCharInfo*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("MMatchServer::OnAsyncResponse_GetStreakLadderCharacterInfo - Error\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetPlayerUID());
	if( pObj == NULL ) return;
	pObj->SetPlayerWarsCharInfo(pJob->GetPWCharInfo());
						
	PostPlayerWarsCharInfo(pObj->GetUID(), pObj->GetPlayerWarsInfo());
	}
	__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "OnAsyncGetStreakLadderCharInfo") ){}
}

void MMatchServer::PostPlayerWarsCharInfo(MUID uidPlayer, PlayerWarsCharInfo *pPWCharInfo)
{
		//__try
	//{
	if( pPWCharInfo == NULL ) return;
	MCommand *pCmd = CreateCommand(MC_MATCH_PLAYERWARS_CHARINFO, uidPlayer);
	pCmd->AddParameter(new MCommandParameterInt(pPWCharInfo->Ranking));
	pCmd->AddParameter(new MCommandParameterInt(pPWCharInfo->Wins));
	pCmd->AddParameter(new MCommandParameterInt(pPWCharInfo->Losses));
	pCmd->AddParameter(new MCommandParameterInt(pPWCharInfo->Draws));
	pCmd->AddParameter(new MCommandParameterInt(pPWCharInfo->Score));
	Post(pCmd);
//	}
	//__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "PostPlayerWarsCharInfo") ){}
}

void MMatchServer::UpdatePlayerWarsQueueingCounts(MTD_PlayerWarsCounterInfo CounterInfo)
{
	//		__try
	//{
		MCommand* pNew = CreateCommand(MC_MATCH_PLAYERWARS_COUNTER, MUID(0,0));
		void* pCounterArray	= MMakeBlobArray( sizeof(MTD_PlayerWarsCounterInfo), 1 );
		MTD_PlayerWarsCounterInfo* pSendCounter = (MTD_PlayerWarsCounterInfo*)MGetBlobArrayElement( pCounterArray, 0 );
		memcpy(pSendCounter, &CounterInfo, sizeof(MTD_PlayerWarsCounterInfo));
		pNew->AddParameter(new MCommandParameterBlob(pCounterArray, MGetBlobArraySize(pCounterArray)));
		for(MMatchObjectList::iterator i=m_Objects.begin(); i!=m_Objects.end(); i++) 
		{
				MMatchObject *pObj = (MMatchObject*)((*i).second);
				if( pObj != NULL && pObj->GetPlace() == MMP_LOBBY) 
				{
					MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
					if(pChannel && pChannel->GetChannelType() == MCHANNEL_TYPE_PLAYERWARS)
					{	
						MCommand* pSendCmd = pNew->Clone();
						pSendCmd->m_Receiver = pObj->GetUID();
						Post(pSendCmd);
					}
				}
		}
		MEraseBlobArray(pCounterArray);
	//}
	//__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "UpdatePlayerWarsQueueingCounts") ){}
}

void MMatchServer::UpdatePlayerWarsCharInfo(int cid, int win, int loss, int draw, int score, int mode)
{
		//__try
	//{
	MAsyncDBJob_UpdatePlayerWarsCharInfo *pAsyncDbJob = new MAsyncDBJob_UpdatePlayerWarsCharInfo;
	if( 0 == pAsyncDbJob ) return;
	pAsyncDbJob->Input(cid, win, loss, draw, score, mode);
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	//	}__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "UpdatePlayerWarsCharInfo") ){}

}

void MMatchServer::OnAsyncResponse_GetPlayerWarsSideRanking(MAsyncJob *pJobResult)
{
		__try
	{
	MAsyncDBJob_GetPlayerWarsSideRankingInfo* pJob = (MAsyncDBJob_GetPlayerWarsSideRankingInfo*)pJobResult;

	if( MASYNC_RESULT_SUCCEED != pJob->GetResult() ) {
		mlog("MMatchServer::OnAsyncResponse_GetStreakLadderSideRanking - 8\n");
		return;
	}

	MMatchObject* pObj = GetObject(pJob->GetPlayerUID());
	if( pObj == NULL ) return;
	PostCmdPlayerWarsCharSideRankingInfo(pObj->GetUID(), pJob->GetSideRankingList());
	}
	__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "OnAsyncResponse_GetStreakLadderSideRanking") ){}
}

void MMatchServer::PostCmdPlayerWarsCharSideRankingInfo(MUID uidPlayer, list<PWRankingInfo*>* pSideRankingList)
{
	//	__try
	//{
	MMatchObject *pDTObj = GetObject(uidPlayer);
	if(IsEnabledObject(pDTObj) == false) return;

	void* pBlobRanking = MMakeBlobArray(sizeof(DTRankingInfo), (int)pSideRankingList->size() );

	int index = 0;
	list<PWRankingInfo*>::iterator iter;
	for(iter = pSideRankingList->begin(); iter != pSideRankingList->end(); ++iter){
		PWRankingInfo *pInfoDest = reinterpret_cast<PWRankingInfo *>(MGetBlobArrayElement(pBlobRanking, index++));	
		PWRankingInfo *pInfoSrc = (*iter);

		if( pInfoDest == NULL || pInfoSrc == NULL ) { MEraseBlobArray( pBlobRanking ); return; }
		memcpy(pInfoDest, pInfoSrc, sizeof(DTRankingInfo));
	}
	
	MCommand* pCmd = CreateCommand(MC_MATCH_PLAYERWARS_SIDERANK, uidPlayer);
	if( NULL == pCmd ) {
		MEraseBlobArray( pBlobRanking );
		return;
	}
	pCmd->AddParameter( new MCommandParameterBlob(pBlobRanking, MGetBlobArraySize(pBlobRanking)) );
	MEraseBlobArray( pBlobRanking );

	Post(pCmd);
	//}
//	__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "PostCmdPlayerWarsCharSideRankingInfo") ){}
}

void MMatchServer::SendPlayerWarsSideRankingToPlayer(MUID uidPlayer)
{
	//	__try
	//{
	MMatchObject *pPWObj = GetPlayerByCommUID(uidPlayer);
	if(IsEnabledObject(pPWObj) == false || !pPWObj) return;
	if( pPWObj->GetCharInfo() == NULL ) return;
	MAsyncDBJob_GetPlayerWarsSideRankingInfo *pAsyncDbJob = new MAsyncDBJob_GetPlayerWarsSideRankingInfo;
	if( 0 == pAsyncDbJob ) return;
	pAsyncDbJob->Input(uidPlayer, pPWObj->GetCharInfo()->m_nCID);
	MMatchServer::GetInstance()->PostAsyncJob( pAsyncDbJob );
	//}
	//__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "SendPlayerWarsSideRankingToPlayer") ){}
}

void MMatchServer::SendPlayerWarsRewards(PlayerWarsListDef List, bool bIsDrawGame, MMatchTeam nWinnerTeam, int WinningScore, int LosingScore, int MapId)
{
	//	__try
//	{
	char szWinnerMembers[512] = "";
	char szLoserMembers[512] = "";
	int Score = ceil((float)List.size()/2);
	for (PlayerWarsListDef::iterator itor=List.begin(); itor!=List.end(); itor++) 
	{
		PlayerWarsInfos* Info = (PlayerWarsInfos*)(*itor).second;
		if(Info)
		{
			MMatchObject *pObj = GetPlayerByCommUID(Info->Player);
			if (pObj && IsEnabledObject(pObj) && pObj->GetPlayerWarsInfo() && pObj->GetCharInfo())
			{

				if (pObj->GetTeam() == nWinnerTeam)
				{
					if(bIsDrawGame)
					{
						pObj->GetPlayerWarsInfo()->AddDraw();
						pObj->GetPlayerWarsInfo()->AddScore(ceil((float)Score/2));
					}
					else
					{
						pObj->GetPlayerWarsInfo()->AddWin();
						pObj->GetPlayerWarsInfo()->AddScore(Score);
					}
				}
				else
				{
					if(bIsDrawGame)
					{
						pObj->GetPlayerWarsInfo()->AddDraw();
						pObj->GetPlayerWarsInfo()->AddScore(ceil((float)Score/2));
					}
					else
					{
						pObj->GetPlayerWarsInfo()->AddLoss();
						pObj->GetPlayerWarsInfo()->MinusScore(Score);
					}
				}
				//UpdatePlayerWarsCharInfo((int)(*itor).first, pObj->GetPlayerWarsInfo()->Wins, pObj->GetPlayerWarsInfo()->Losses, pObj->GetPlayerWarsInfo()->Draws, pObj->GetPlayerWarsInfo()->Score, 0);
				SendPlayerWarsCharInfoToPlayer(pObj->GetUID());
			} 
			
			if (Info->Team == nWinnerTeam)
			{
				
				char cid[100] = "";
				sprintf(cid, "%d", (int)(*itor).first);
				strcat(szWinnerMembers, cid);
				strcat(szWinnerMembers, " ");
				if(bIsDrawGame)
					UpdatePlayerWarsCharInfo((int)(*itor).first, 0, 0, 1, ceil((float)Score/2), 0);
				else
					UpdatePlayerWarsCharInfo((int)(*itor).first, 1, 0, 0, Score, 0);
			}
			else
			{
				char cid[100] = "";
				sprintf(cid, "%d", (int)(*itor).first);
				strcat(szLoserMembers, cid);
				strcat(szLoserMembers, " ");
				if(bIsDrawGame)
					UpdatePlayerWarsCharInfo((int)(*itor).first, 0, 0, 1, ceil((float)Score/2), 0);
				else
					UpdatePlayerWarsCharInfo((int)(*itor).first, 0, 1, 0, 0, 0);
			}
		}
	}
	m_MatchDBMgr.InsertPWGameLog(szWinnerMembers, szLoserMembers, WinningScore, LosingScore, MapId);
	//}
	//__except( PlayerWarsExceptionHandler(GetExceptionInformation(), "SendPlayerWarsRewards") ){}
}