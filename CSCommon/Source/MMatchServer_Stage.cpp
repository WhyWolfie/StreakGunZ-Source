#include "stdafx.h"
#include "MMatrix.h"
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
#include "MVoteDiscussImpl.h"
#include "MUtil.h"
#include "MMatchGameType.h"
#include "MMatchRuleBaseQuest.h"
#include "MMatchRuleQuest.h"
#include "MMatchRuleBerserker.h"
#include "MMatchRuleDuel.h"
#include "MCrashDump.h"
#include "MNewQuestScenario.h"
#include "MMatchRuleQuestChallenge.h"
#include "MAsyncDBJob_InsertGamePlayerLog.h"
#ifdef _SPYMODE
#include "MMatchSpyMode.h"
#include "MMatchRuleSpy.h"
#endif
#include "../MagicBox/MMatchRuleDropGunGame.h"

#ifdef _UPREWARDLEVEL
#include "MMatchLevelUpReward.h"
#endif

static bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat);


MMatchStage* MMatchServer::FindStage(const MUID& uidStage)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) return NULL;

	MMatchStage* pStage = (*i).second;
	return pStage;
}

bool MMatchServer::StageAdd(MMatchChannel* pChannel, const char* pszStageName, bool bPrivate, const char* pszStagePassword, MUID* pAllocUID, bool bIsAllowNullChannel)
{
	// ≈¨∑£¿ÅE∫ pChannel¿Ã NULL¿Ã¥Ÿ.

	MUID uidStage = m_StageMap.UseUID();
	
	MMatchStage* pStage= new MMatchStage;
#ifdef _ROOMHIDE
	if (pChannel && !pChannel->AddStage(pStage, bPrivate)) {
		delete pStage;
		return false;
	}
#else
	if (pChannel && !pChannel->AddStage(pStage)) {
		delete pStage;
		return false;
}
#endif 

	MMATCH_GAMETYPE GameType = MMATCH_GAMETYPE_DEFAULT;
	bool bIsCheckTicket = false;
	DWORD dwTicketID = 0;

	if ( (NULL != pChannel) && MGetServerConfig()->IsUseTicket()) {
		bIsCheckTicket = (pChannel != 0) && pChannel->IsUseTicket() && pChannel->IsTicketChannel();
		dwTicketID = pChannel->GetTicketItemID();

		// ∆ºƒœ º≠πˆø°º≠ ªÁº≥ √§≥Œ¿∫ π´¡∂∞« ∆ºƒœ ∞ÀªÅE- ∆ºƒœ¿∫ ≈¨∑£¿ÅE∆ºƒœ∞ÅEµø¿œ«œ¥Ÿ.
		if ( pChannel->GetChannelType() == MCHANNEL_TYPE_USER) {
			bIsCheckTicket = true;
			dwTicketID = GetChannelMap()->GetClanChannelTicketInfo().m_dwTicketItemID;
		}
	}

	if (!pStage->Create( uidStage, pszStageName, bPrivate, pszStagePassword, bIsAllowNullChannel, GameType, bIsCheckTicket, dwTicketID) ) {
		if (pChannel) {
			pChannel->RemoveStage(pStage);
		}

		delete pStage;
		return false;
	}

	m_StageMap.Insert(uidStage, pStage);

	*pAllocUID = uidStage;

	return true;
}


bool MMatchServer::StageRemove(const MUID& uidStage, MMatchStageMap::iterator* pNextItor)
{
	MMatchStageMap::iterator i = m_StageMap.find(uidStage);
	if(i==m_StageMap.end()) {
		return false;
	}

	MMatchStage* pStage = (*i).second;

	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());
	if (pChannel) {
		pChannel->RemoveStage(pStage);
	}

	pStage->Destroy();
	delete pStage;

	MMatchStageMap::iterator itorTemp = m_StageMap.erase(i);
	if (pNextItor) *pNextItor = itorTemp;

	return true;
}
bool MMatchServer::StageJoin(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;

	if (pObj->GetStageUID() != MUID(0,0))
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return false;
	if (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT) return false;

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	int ret = ValidateStageJoin(uidPlayer, uidStage);
	if (ret != MOK) {
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, ret);
		return false;
	}
	pObj->OnStageJoin();

	// Cache Add
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.AddObject(pObj);
	MCommand* pCmdCacheAdd = CacheBuilder.GetResultCmd(MATCHCACHEMODE_ADD, this);
	RouteToStage(pStage->GetUID(), pCmdCacheAdd);

	// Join
	pStage->AddObject(uidPlayer, pObj);
		// ¿”Ω√ƒ⁄µÅE.. ¿ﬂ∏¯µ» ≈¨∑£ID ø¬¥Ÿ∏ÅE√º≈©«œø© ¿‚±‚¿ß«‘...20090224 by kammir
	if(pObj->GetCharInfo()->m_ClanInfo.GetClanID() >= 9000000)
		LOG(LOG_FILE, "[UpdateCharClanInfo()] %s's ClanID:%d.", pObj->GetAccountName(), pObj->GetCharInfo()->m_ClanInfo.GetClanID());

	pObj->SetStageUID(uidStage);
	pObj->SetStageState(MOSS_NONREADY);

#ifdef _FORCE_EVENT_TEAM
	if (pChannel->IsUseEvent())
	{
		if (IsAdminGrade(pObj))
			pObj->SetTeam(MMT_RED);
		else
			pObj->SetTeam(MMT_BLUE);
	}
	else
#endif

	pObj->SetTeam(pStage->GetRecommandedTeam());

	// Cast Join
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_JOIN), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	pNew->AddParameter(new MCommandParameterUInt(pStage->GetIndex()+1));
	pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
	
	if (pStage->GetState() == STAGE_STATE_STANDBY)  RouteToStage(pStage->GetUID(), pNew);
	else											RouteToListener(pObj, pNew);


	// Cache Update
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pScanObj = (MMatchObject*)GetObject(uidObj);
		if (pScanObj) {
			CacheBuilder.AddObject(pScanObj);
		} else {
			LOG(LOG_PROG, "MMatchServer::StageJoin - Invalid ObjectMUID(%u:%u) exist in Stage(%s)\n",
				uidObj.High, uidObj.Low, pStage->GetName());
			pStage->RemoveObject(uidObj);
			return false;
		}
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);


	// Cast Master(πÊ¿ÅE
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);


#ifdef _QUEST_ITEM
	// Custom: Quest/Clan Server
	//if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	//{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			mlog("MMatchServer::StageJoin - Couldn't find Setting.\n");
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				mlog("MMatchServer::StageJoin - Couldn't find Quest Rule.\n");
				return false;
			}

			pRuleQuest->OnChangeCondition();
			//pRuleQuest->OnResponseQL_ToStage( pObj->GetStageUID() );
			// µø»Øææ≤≤º≠ √≥¿Ω Ω∫≈◊¿Ã¡ÅE¡∂¿ŒΩ√¥¬ ¿Ã¿ÅE° º≥¡§¿Ã ƒ˘Ω∫∆Æ∑Œ µ«¿÷æ˚—µ 
			//  √≥¿Ω ¡∂¿Œ«— ¿Ø¿˙¥¬ ƒ˘Ω∫∆Æ ≈∏¿‘¿Œ¡ÅEæÀºˆ∞° æ¯±‚ø°,
			//	≈¨∂Û¿Ãæ∆Æ∞° Ω∫≈◊¿Ã¡ÅE≈∏¿‘¿Ã ƒ˘Ω∫∆Æ¿Œ¡ˆ∏¶ ¿ŒΩƒ«œ¥¬ Ω√¡°ø°º≠
			//  ¿Ã ¡§∫∏∏¶ ø‰√ª¿ª «œ¥¬ πÊ«‚¿∏∑Œ ºˆ¡§«‘. - 05/04/14 by √ﬂ±≥º∫.
			// pStage->GetRule()->OnResponseSacrificeSlotInfoToStage( uidPlayer );
		}
	//}
#endif


	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());


	// πÊº€ ∞ÅEË¿⁄∏ÅEπÊ¿Â±««—¿ª ¿⁄µø¿∏∑Œ ª©æ—¥¬¥Ÿ. - ø¬∞‘¿”≥› ∫Ò∫Ò∫ÅEø‰√ª
	// Custom: Disable auto change master for UGrade 252
	//if (MMUG_EVENTMASTER == pObj->GetAccountInfo()->m_nUGrade) {
	//	OnEventChangeMaster(pObj->GetUID());
	//}

	return true;
}
bool MMatchServer::StageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( !IsEnabledObject(pObj) ) return false;
	// MMatchStage* pStage = FindStage(uidStage);

	//if(pObj->GetStageUID()!=uidStage)
	//	mlog(" stage leave hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	bool bLeaverMaster = false;
	if (uidPlayer == pStage->GetMasterUID()) bLeaverMaster = true;

#ifdef _QUEST_ITEM
	// Custom: Quest/Clan Server
	//if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	//{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 != pNode )
		{
			if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
			{
				MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
				if(pRuleQuest)
				{
					pRuleQuest->PreProcessLeaveStage( uidPlayer );
				} else {
					LOG(LOG_PROG, "StageLeave:: MMatchRule to MMatchRuleBaseQuest FAILED \n");
				}
			}
		}
	//}
#endif

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LEAVE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	// pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
	RouteToStage(pStage->GetUID(), pNew);

	pStage->RemoveObject(uidPlayer);

	//MMatchObject* pObj = GetObject(uidPlayer);
	//if (pObj)
	{
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pObj);
		MCommand* pCmdCache = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REMOVE, this);
		RouteToStage(pStage->GetUID(), pCmdCache);
	}

	// cast Master
	if (bLeaverMaster) StageMaster(pStage->GetUID());

#ifdef _QUEST_ITEM
	// ¿Ø¿˙∞° Ω∫≈◊¿Ã¡ˆø°º≠ ≥™∞£»ƒø° QL¿ª ¥ŸΩ√ ∞ËªÅEÿ ¡‡æﬂ «‘.
	// Custom: Quest/Clan Server
	//if (MGetServerConfig()->GetServerMode() == MSM_TEST)
	{
		const MSTAGE_SETTING_NODE* pNode = pStage->GetStageSetting()->GetStageSetting();
		if( 0 == pNode )
		{
			LOG(LOG_PROG, "MMatchServer::StageLeave - MSTAGE_SETTING_NODE Failed.\n" );
			return false;
		}

		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType))
		{
			MMatchRuleBaseQuest* pRuleQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
			if( 0 == pRuleQuest )
			{
				LOG(LOG_PROG, "MMatchServer::StageLeave - MMatchRuleBaseQuest Failed.\n" );
				return false;
			}

			if( STAGE_STATE_STANDBY == pStage->GetState() )
				pRuleQuest->OnChangeCondition();
				//pRuleQuest->OnResponseQL_ToStage( uidStage );
		}
	}
#endif

	// Custom: Ladder Rejoin
	if (pStage->GetStageType() != MST_NORMAL)
	{
		MCommand* pNew = CreateCommand(MC_MATCH_NOTIFY_LADDER_REJOIN, uidPlayer);
		RouteToListener(pObj, pNew);
	}

	return true;
}



DWORD StageEnterBattleExceptionHandler( PEXCEPTION_POINTERS ExceptionInfo )
{
	char szStageDumpFileName[ _MAX_DIR ]= {0,};
	SYSTEMTIME SystemTime;
	GetLocalTime( &SystemTime );
	sprintf( szStageDumpFileName, "Log/StageDump_%d-%d-%d_%d-%d-%d.dmp"
		, SystemTime.wYear
		, SystemTime.wMonth
		, SystemTime.wDay
		, SystemTime.wHour
		, SystemTime.wMinute
		, SystemTime.wSecond );

	return CrashExceptionDump( ExceptionInfo, szStageDumpFileName, true );
}

void InnerEnterBattle( MMatchObject* pObj, MMatchStage* pStage )
{
	pStage->EnterBattle( pObj );
}

void InnerEnterBattleException( MMatchObject* pObj, MMatchStage* pStage )
{
		mlog( "\nexception : stage enter battle =====================\n" );


		MMatchObject* pMaster = MGetMatchServer()->GetObject( pStage->GetMasterUID() );
		if( NULL != pMaster )  
		{
			if( NULL != pMaster->GetCharInfo() )
			{
				mlog( "stage master cid : %d\n", pMaster->GetCharInfo()->m_nCID );
			}
		}
		else
		{
			mlog( "stage master hass problem.\n" );				
		}
		
		
		if( NULL != pObj->GetCharInfo() )
		{
			mlog( "cmd sender cid : %d\n", pObj->GetCharInfo()->m_nCID );
		}
		else
		{
			mlog( "cmd send char info null point.\n" );
		}

		
		MMatchStageSetting*	pStageSetting = pStage->GetStageSetting();
		if( NULL != pStageSetting )
		{
			mlog( "stage state : %d\n", pStage->GetStageSetting()->GetStageState() );

			const MSTAGE_SETTING_NODE* pExStageSettingNode = pStageSetting->GetStageSetting();
			if( NULL != pExStageSettingNode )
			{
				mlog( "stage name : %s\n", pExStageSettingNode->szMapName );
				mlog( "stage game type : %d\n", pExStageSettingNode->nGameType );
				mlog( "stage max player : %d\n", pExStageSettingNode->nMaxPlayers );
				mlog( "stage current player : %d\n", pStage->GetPlayers() );
				mlog( "stage force entry enable : %d\n", pExStageSettingNode->bForcedEntryEnabled );
				mlog( "stage rule pointer : %x\n", pStage->GetRule() );
			}
		}

		MUIDRefCache::iterator itStage, endStage;
		endStage = pStage->GetObjEnd();
		itStage = pStage->GetObjBegin();
		MMatchObject* pObj2 = NULL;
		for( ; endStage != itStage; ++itStage )
		{
			pObj2 = MGetMatchServer()->GetObject( itStage->first );
			if( NULL == pObj2 )
			{
				mlog( "!!!!stage can't find player!!!!\n" );
				continue;
			}

			mlog( "stage player name : %s\n", pObj2->GetName() );
		}

		mlog( "=====================\n\n" );
}

bool ExceptionTraceStageEnterBattle( MMatchObject* pObj, MMatchStage* pStage )
{
	if( NULL == pObj )
	{
		return false;
	}

	if( NULL == pStage )
	{
		return false;
	}

	__try
	{
		InnerEnterBattle( pObj, pStage );
	}
	__except( StageEnterBattleExceptionHandler(GetExceptionInformation()) )
	{
		InnerEnterBattleException( pObj, pStage );

		return false;
	}

	return true;
}



bool MMatchServer::StageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	
	if(pObj->GetStageUID()!=uidStage)
	{
		// Custom: Made it actually ignore the player by returning..
		mlog(" stage enter battle hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		return false;
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pObj->SetPlace(MMP_BATTLE);

	MCommand* pNew = CreateCommand(MC_MATCH_STAGE_ENTERBATTLE, MUID(0,0));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//pNew->AddParameter(new MCommandParameterUID(uidStage));

	unsigned char nParam = MCEP_NORMAL;
	if (pObj->IsForcedEntried()) nParam = MCEP_FORCED;
	pNew->AddParameter(new MCommandParameterUChar(nParam));

	void* pPeerArray = MMakeBlobArray(sizeof(MTD_PeerListNode), 1);
	MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pPeerArray, 0);
	memset(pNode, 0, sizeof(MTD_PeerListNode));
	
	pNode->uidChar	= pObj->GetUID();
	// Custom: Don't distribute IP/Port in Clan Wars
	//if (pStage->GetStageSetting() != NULL && pStage->GetStageSetting()->IsTeamWinThePoint())
	//{
	//	// unconnectable, force relay
	//	pNode->dwIP = 0;
	//	pNode->nPort = 0;
	//}
	if (pObj->GetForcedNATOption() == true)
	{
		// There is a possibility that the GMs receive the non-staff user's IP and attempts to connect..?
		pNode->dwIP = 0;
		pNode->nPort = 0;
	}
	else
	{
		pNode->dwIP = pObj->GetIP();
		pNode->nPort = pObj->GetPort();
	}
	if (pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY)
	{
		pStage->PlayerTeam(uidPlayer, MMT_BLUE);
		pNode->ExtendInfo.nTeam = (char)MMT_BLUE;
	}
	else if (pStage->GetStageSetting()->IsTeamPlay())
	{
		pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
	}
	else
	{
		pNode->ExtendInfo.nTeam = 0;
	}
	CopyCharInfoForTrans(&pNode->CharInfo, pObj->GetCharInfo(), pObj);
	//πˆ«¡¡§∫∏¿”Ω√¡÷ºÆ 	CopyCharBuffInfoForTrans(&pNode->CharBuffInfo, pObj->GetCharInfo(), pObj);

	pNode->ExtendInfo.nPlayerFlags = pObj->GetPlayerFlags();
	if (pStage->GetStageSetting()->IsTeamPlay())	pNode->ExtendInfo.nTeam = (char)pObj->GetTeam();
	else											pNode->ExtendInfo.nTeam = 0;	

	pNew->AddParameter(new MCommandParameterBlob(pPeerArray, MGetBlobArraySize(pPeerArray)));
	MEraseBlobArray(pPeerArray);

	RouteToStage(uidStage, pNew);


	pObj->GetCharInfo()->m_nBattleStartTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	pObj->GetCharInfo()->m_nBattleStartXP = pObj->GetCharInfo()->m_nXP;
	return ExceptionTraceStageEnterBattle(pObj, pStage);
}

bool MMatchServer::StageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle, bool bForcedLeave)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	if (pObj->GetPlace() != MMP_BATTLE) { return false; }

	// MMatchStage* pStage = FindStage(uidStage);

	//if(pObj->GetStageUID()!=uidStage)
	//	mlog(" stage leave battle hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);

	const MUID uidStage = pObj->GetStageUID();
	MMatchStage* pStage = FindStage(uidStage);

	if (pStage == NULL)
	{	// ≈¨∑£¿ÅE√ «—¡∑¿Ã ¥Ÿ≥™∞°∏ÅEΩ∫≈◊¿Ã¡ˆ∞° æ¯æ˚›ˆπ«∑Œ ø©±‚º≠ agent∏¶ ≤˜æ˚›ÿ¥Ÿ. 
		if (pObj->GetRelayPeer()) {
			MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
			if (pAgent) {
				MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
				pCmd->AddParameter(new MCmdParamUID(uidPlayer));
				Post(pCmd);
			}
		}

		UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
		UpdateCharItemDBCachingData(pObj);	///< Character Itemø°º≠ æ˜µ•¿Ã∆Æ∞° « ø‰«— ∞ÕµÅEæ˜µ•¿Ã∆Æ
		//CheckSpendableItemCounts(pObj);		///< «◊ªÅEUpdateCharItemDBCachingData µ⁄ø° ¿÷æ˚⁄ﬂ «’¥œ¥Ÿ.
		
		ProcessCharPlayInfo(pObj);			///< ƒ≥∏Ø≈Õ «√∑π¿Ã«— ¡§∫∏ æ˜µ•¿Ã∆Æ 
		return false;
	}
	else
	{
		// LeaveBattle∞° µ«∏Èº≠ ƒ≥∏Ø≈Õ µ•¿Ã≈Õ∏¶ æ˜µ•¿Ã∆Æ «ÿ¡ÿ¥Ÿ.
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.Reset();
		for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
			MMatchObject* pScanObj = (MMatchObject*)(*i).second;
			CacheBuilder.AddObject(pScanObj);
		}
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
		RouteToListener(pObj, pCmdCacheUpdate);
	}

	pStage->LeaveBattle(pObj);
	pObj->SetPlace(MMP_STAGE);


	// ∑π∫ßø° æ»∏¬¥¬ ¿Â∫Òæ∆¿Ã≈€ √º≈©
#define LEGAL_ITEMLEVEL_DIFF		3
	bool bIsCorrect = true;
	for (int i = 0; i < MMCIP_END; i++) {
		if (CorrectEquipmentByLevel(pObj, MMatchCharItemParts(i), LEGAL_ITEMLEVEL_DIFF)) {
			bIsCorrect = false;
		}
	}

	if (!bIsCorrect) {
		MCommand* pNewCmd = CreateCommand(MC_MATCH_RESPONSE_RESULT, MUID(0,0));
		pNewCmd->AddParameter(new MCommandParameterInt(MERR_TAKEOFF_ITEM_BY_LEVELDOWN));
		RouteToListener(pObj, pNewCmd);
	}
	
	CheckExpiredItems(pObj);		//< ±‚∞£ ∏∏∑ÅEæ∆¿Ã≈€¿Ã ¿÷¥¬¡ÅE√º≈©

	if (pObj->GetRelayPeer()) {
		MAgentObject* pAgent = GetAgent(pObj->GetAgentUID());
		if (pAgent) {
			MCommand* pCmd = CreateCommand(MC_AGENT_PEER_UNBIND, pAgent->GetCommListener());
			pCmd->AddParameter(new MCmdParamUID(uidPlayer));
			Post(pCmd);
		}
	}	

	// ƒ≥∏Ø≈Õ «√∑π¿Ã«— ¡§∫∏ æ˜µ•¿Ã∆Æ 
	ProcessCharPlayInfo(pObj);

	//=======================================================================================================================================
	
	bool bIsLeaveAllBattle = true;
	
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pAllObj = (MMatchObject*)GetObject(uidObj);
		if(NULL == pAllObj) continue;
		if(MMP_STAGE != pAllObj->GetPlace()) { 
			bIsLeaveAllBattle = false; 
			break; 
		}
	}


	if(pStage->IsRelayMap())
	{
		if(bGameFinishLeaveBattle)
		{	// ∏±∑π¿Ã∏ , πË∆≤ ¡æ∑·∑Œ Ω∫≈◊¿Ã¡ˆ∑Œ ≥™ø‘¿ª∂ß
			if(!pStage->m_bIsLastRelayMap)
			{	// ¥Ÿ¿Ω∏ ¿Ã ¿÷¥Ÿ∏ÅEπŸ∑Œ ¥Ÿ¿Ω ∏ Ω√¿€ √≥∏Æ		

				if( !bForcedLeave ) 
				{
					pObj->SetStageState(MOSS_READY);
				}

				if( bIsLeaveAllBattle ) 
				{					
					OnStageRelayStart(uidStage);
				} 

				MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
				pNew->AddParameter(new MCommandParameterUID(uidPlayer));
				pNew->AddParameter(new MCommandParameterBool(true));
				RouteToStage(uidStage, pNew);
			}
		}
		else
		{	///< ∏ﬁ¿Œ ∏ﬁ¥∫∑Œ Ω∫≈◊¿Ã¡ˆø° ≥™ø»		
			MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
			pNew->AddParameter(new MCommandParameterUID(uidPlayer));
			pNew->AddParameter(new MCommandParameterBool(false));
			RouteToStage(uidStage, pNew);			

			if(bIsLeaveAllBattle) 
			{	///< ∏µŒ Ω∫≈◊¿Ã¡ˆø° ¿÷¥Ÿ∏ÅE∏±∑π¿Ã∏  ºº∆√¿ª ¥ŸΩ√ «ÿ¡ÿ¥Ÿ.
				pStage->m_bIsLastRelayMap = true;//∏±∑π¿Ã∏ ¿ª ≥°≥Ω¥Ÿ
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);
				pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
				pStage->m_RelayMapRepeatCountRemained = pStage->GetRelayMapRepeatCount();
			}
		}
	} 
	else 
	{
		MCommand* pNew = CreateCommand(MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT, MUID(0,0));
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterBool(false));
		RouteToStage(uidStage, pNew);
	}

	//=======================================================================================================================================

	// πÊø°º≠ ≥™∞°∏ÅEnoreadyªÛ≈¬∑Œ ∫Ø∞Êµ»¥Ÿ. 
	// ∫Ø∞Êµ» ¡§∫∏∏¶ Ω∫≈◊¿Ã¡ˆ¿« ∏µÅE≈¨∂Û¿Ãæ∆Æ∑Œ ∫∏≥ª¡‹. - by SungE 2007-06-04
	StagePlayerState( uidPlayer, pStage->GetUID(), pObj->GetStageState() );	
	
	UpdateCharDBCachingData(pObj);		///< XP, BP, KillCount, DeathCount ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	UpdateCharItemDBCachingData(pObj);	///< Character Itemø°º≠ æ˜µ•¿Ã∆Æ∞° « ø‰«— ∞ÕµÅEæ˜µ•¿Ã∆Æ
	//CheckSpendableItemCounts(pObj);		///< «◊ªÅEUpdateCharItemDBCachingData µ⁄ø° ¿÷æ˚⁄ﬂ «’¥œ¥Ÿ.

	return true;
}

bool MMatchServer::StageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL)	return false;
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) return false;

	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;

//	InsertChatDBLog(uidPlayer, pszChat);

	///< »´±‚¡÷(2009.08.04)
	///< «ˆ¿ÅE«ÿ¥ÅEªÁøÅE⁄∞° ¿÷¥¬ StageøÕ ∫∏≥ªø¬ Stage¿« UID∞° ¥Ÿ∏¶ ∞ÊøÅE
	///< ¥Ÿ∏• StageµÈø°∞‘µµ Msg∏¶ ∫∏≥æ ºÅE¿÷¥¬ πÆ¡¶∞° ¿÷¿Ω («ÿ≈∑ «¡∑Œ±◊∑• ªÁøÅE√)
	if( uidStage != pObj->GetStageUID() )
	{
		//LOG(LOG_FILE,"MMatchServer::StageChat - Different Stage(S:%d, P:%d)", uidStage, pObj->GetStageUID());
		return false;
	}
#ifdef _FORCE_EVENT_TEAM
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL)
		return false;
#endif
	// Custom: Silence Room
	DWORD dwTime = timeGetTime();
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM && dwTime < pStage->GetSilenceEndTime())
	{
		char szMsg[128];
		memset(szMsg, 0, sizeof( szMsg ));
		sprintf_s( szMsg, "Room is silenced. You may chat in %d seconds.", (pStage->GetSilenceEndTime() - dwTime)/1000 );

		Announce(pObj, szMsg);
		return false;
	}

#ifdef _FORCE_EVENT_TEAM
	if (pChannel->IsUseEvent())
	{
		if (!IsAdminGrade(pObj))
			return false;
	}
	else

	{
		pObj->SetTeam(pStage->GetRecommandedTeam());
	}
#endif
	MCommand* pCmd = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_CHAT), MUID(0,0), m_This);
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterString(pszChat));
	RouteToStage(uidStage, pCmd);
	return true;
}

// Custom: in-game chat server sided.
bool MMatchServer::GameChat(const MUID& uidPlayer, const MUID& uidStage, char* szMsg, int nTeam)
{
	MMatchObject* pObj = (MMatchObject*)GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_CHAT_LIMITED) return false;
	if (nTeam > 1 && nTeam != pObj->GetTeam()) return false;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL)	return false;
	if (uidStage != pObj->GetStageUID()) return false;

	// Custom: Silence Room
	DWORD dwTime = timeGetTime();
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM && dwTime < pStage->GetSilenceEndTime())
	{
		char szMsg[128];
		memset(szMsg, 0, sizeof(szMsg));
		sprintf_s(szMsg, "Room is silenced. You may chat in %d seconds.", (pStage->GetSilenceEndTime() - dwTime) / 1000);

		Announce(pObj, szMsg);
		return false;
	}

	// Send to everyone in the chat.
	if ((nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
	{
		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_CHAT, MUID(0, 0));
		pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
		pCmd->AddParameter(new MCommandParameterUID(uidStage));
		pCmd->AddParameter(new MCommandParameterString(szMsg));
		pCmd->AddParameter(new MCommandParameterInt(nTeam));
		RouteToBattle(pObj->GetStageUID(), pCmd);
	}
	// Send to own team only.
	else if (nTeam == pObj->GetTeam())
	{
		for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); itor != pStage->GetObjEnd(); itor++)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;

			if (!IsEnabledObject(pPlayer)) continue;
			if (!pPlayer->GetEnterBattle()) continue;
			if (pPlayer->GetTeam() != nTeam && !pPlayer->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) continue;

			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_CHAT, MUID(0, 0));
			pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
			pCmd->AddParameter(new MCommandParameterUID(uidStage));
			pCmd->AddParameter(new MCommandParameterString(szMsg));
			pCmd->AddParameter(new MCommandParameterInt(nTeam));
			RouteToListener(pPlayer, pCmd);
		}
	}

	return true;
}

bool MMatchServer::StageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	pStage->PlayerTeam(uidPlayer, nTeam);

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_TEAM, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUInt(nTeam));

	RouteToStageWaitRoom(uidStage, pCmd);
	return true;
}

bool MMatchServer::StagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return false;
	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return false;

	pStage->PlayerState(uidPlayer, nStageState);
	
	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_PLAYER_STATE, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidPlayer));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterInt(nStageState));
	RouteToStage(uidStage, pCmd);
	return true;
}

bool MMatchServer::StageMaster(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return false;

	MUID uidMaster = pStage->GetMasterUID();

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToStage(uidStage, pCmd);

	return true;
}

void MMatchServer::StageLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);
		
	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()))
	{
		if (pStage->m_pRule)
		{
			pStage->m_pRule->OnQuestStageLaunch();
		}
	}


	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_LAUNCH, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidStage));
	pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
	RouteToStage(uidStage, pCmd);
}

void MMatchServer::StageRelayLaunch(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	ReserveAgent(pStage);

	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MUID uidObj = (MUID)(*i).first;
		MMatchObject* pObj = (MMatchObject*)GetObject(uidObj);
		if (pObj) {
			if( pObj->GetStageState() == MOSS_READY) {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(false));
				RouteToListener(pObj, pCmd);
			} else {
				MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RELAY_LAUNCH, MUID(0,0));
				pCmd->AddParameter(new MCmdParamUID(uidStage));
				pCmd->AddParameter(new MCmdParamStr(const_cast<char*>(pStage->GetMapName())));
				pCmd->AddParameter(new MCmdParamBool(true));
				RouteToListener(pObj, pCmd);
			}
		} else {
			LOG(LOG_PROG, "WARNING(StageRelayLaunch) : Not Existing Obj(%u:%u)\n", uidObj.High, uidObj.Low);
			i=pStage->RemoveObject(uidObj);
			LogObjectCommandHistory(uidObj);
		}
	}
}

void MMatchServer::StageFinishGame(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	bool bIsRelayMapUnFinish = true;

	if(pStage->IsRelayMap())
	{ // ∏±∑π¿Ã ∏ ¿œ∂ßø°¥¬ πË∆≤¿ª ¥ŸΩ√ Ω√¿€«ÿ¡ÿ¥Ÿ. 
		if((int)pStage->m_vecRelayMapsRemained.size() <= 0)
		{	// ≥≤¿∫ ∏ ¿Ã æ¯¿ª∂ß
			int nRepeatCount = (int)pStage->m_RelayMapRepeatCountRemained - 1;
			if(nRepeatCount < 0)
			{
				bIsRelayMapUnFinish = false;

				pStage->m_bIsLastRelayMap = true;//∏±∑π¿Ã∏ ¿ª ≥°≥Ω¥Ÿ				
				nRepeatCount = 0;
				pStage->GetStageSetting()->SetMapName(MMATCH_MAPNAME_RELAYMAP);	//"RelayMap" ºº∆√
			}
			pStage->m_RelayMapRepeatCountRemained = (RELAY_MAP_REPEAT_COUNT)nRepeatCount;
			pStage->SetRelayMapCurrList(pStage->GetRelayMapList());
		}

		if(!pStage->m_bIsLastRelayMap) {
			// √≥¿Ω Ω√¿€Ω√, Flag∏¶ OnΩ√ƒ—¡ÿ¥Ÿ. 
			if( pStage->IsStartRelayMap() == false ) {
				pStage->SetIsStartRelayMap(true);
			}			

			if((int)pStage->m_vecRelayMapsRemained.size() > 0) { // ¥Ÿ¿Ω∏ ¿Ã ¿÷¥Ÿ∏ÅE
				int nRelayMapIndex = 0;

				if(pStage->GetRelayMapType() == RELAY_MAP_TURN) {	//< ≥≤¿∫ ∞Õ¡ﬂø°º≠ √π π¯¬∞∫Œ≈Õ Ω√¿€(∞°µ∂º∫)
					nRelayMapIndex = 0; 
				} else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM) {
					nRelayMapIndex = rand() % (int)pStage->m_vecRelayMapsRemained.size();
				}

				if(nRelayMapIndex >= MAX_RELAYMAP_LIST_COUNT) { //< ∏  ±∏º∫¿Ã 20∞≥
					mlog("StageFinishGame RelayMap Fail RelayMapList MIsCorrect MaxCount[%d] \n", (int)nRelayMapIndex);
					return;
				}

				char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
				if (!szMapName)
				{
					mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
					return;
				}

				pStage->GetStageSetting()->SetMapName(szMapName);

				// Ω««‡«— ∏±∑π¿Ã∏ ¿∫ ªË¡¶«ÿ¡ÿ¥Ÿ.
				vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
				for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// «ÿ¥ÅE¿Œµ¶Ω∫±˚›ÅE¿Ãµø
				pStage->m_vecRelayMapsRemained.erase(itor);
			} 
			else {
				mlog("MMatchServer::StageFinishGame::IsRelayMap() - m_vecRelayMapsRemained.size() == 0\n");
			}
		} else {
			pStage->SetIsStartRelayMap(false);
			bIsRelayMapUnFinish = false; // ∏±∑π¿Ã∏  ¡¯«‡¿Ã ≥°≥µ¿Ω
		}
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_FINISH_GAME, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(uidStage));
	pCmd->AddParameter(new MCommandParameterBool(bIsRelayMapUnFinish));
	RouteToStage(uidStage, pCmd);

	return;
}

MCommand* MMatchServer::CreateCmdResponseStageSetting(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return NULL;

	MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_STAGESETTING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	MMatchStageSetting* pSetting = pStage->GetStageSetting();

	// Param 1 : Stage Settings
	void* pStageSettingArray = MMakeBlobArray(sizeof(MSTAGE_SETTING_NODE), 1);
	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageSettingArray, 0);
	CopyMemory(pNode, pSetting->GetStageSetting(), sizeof(MSTAGE_SETTING_NODE));
	pCmd->AddParameter(new MCommandParameterBlob(pStageSettingArray, MGetBlobArraySize(pStageSettingArray)));
	MEraseBlobArray(pStageSettingArray);

	// Param 2 : Char Settings
	int nCharCount = (int)pStage->GetObjCount();
	void* pCharArray = MMakeBlobArray(sizeof(MSTAGE_CHAR_SETTING_NODE), nCharCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) {
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (MSTAGE_CHAR_SETTING_NODE*)MGetBlobArrayElement(pCharArray, nIndex++);
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		pCharNode->uidChar = pObj->GetUID();
		pCharNode->nTeam = pObj->GetTeam();
		pCharNode->nState = pObj->GetStageState();
	}
	pCmd->AddParameter(new MCommandParameterBlob(pCharArray, MGetBlobArraySize(pCharArray)));
	MEraseBlobArray(pCharArray);

	// Param 3 : Stage State
	pCmd->AddParameter(new MCommandParameterInt((int)pStage->GetState()));

	// Param 4 : Stage Master
	pCmd->AddParameter(new MCommandParameterUID(pStage->GetMasterUID()));

	return pCmd;
}



void MMatchServer::OnStageCreate(const MUID& uidChar, char* pszStageName, bool bPrivate, char* pszStagePassword)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN)
		&& (pChannel->GetChannelType() == MCHANNEL_TYPE_DUELTOURNAMENT || pChannel->GetChannelType() == MCHANNEL_TYPE_PLAYERWARS)) {
		return;
	}

	// Custom: Event channels
	if( pChannel->IsUseEvent() && (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) )
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}

	if (strlen(pszStageName) <= 0)
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	
	MUID uidStage;

	if (!StageAdd(pChannel, pszStageName, bPrivate, pszStagePassword, &uidStage))
	{
		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_CREATE, MERR_CANNOT_CREATE_STAGE);
		return;
	}
	StageJoin(uidChar, uidStage);

	MMatchStage* pStage = FindStage(uidStage);
	if (pStage)
		pStage->SetFirstMasterName(pObj->GetCharInfo()->m_szName);
}


//void MMatchServer::OnStageJoin(const MUID& uidChar, const MUID& uidStage)
//{
//	MMatchObject* pObj = GetObject(uidChar);
//	if (pObj == NULL) return;
//
//	MMatchStage* pStage = NULL;
//
//	if (uidStage == MUID(0,0)) {
//		return;
//	} else {
//		pStage = FindStage(uidStage);
//	}
//
//	if (pStage == NULL) {
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
//		return;
//	}
//
//	if ((IsAdminGrade(pObj) == false) && pStage->IsPrivate())
//	{
//		RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
//		return;
//	}
//
//	StageJoin(uidChar, pStage->GetUID());
//}

void MMatchServer::OnPrivateStageJoin(const MUID& uidPlayer, const MUID& uidStage, char* pszPassword)
{
	if (strlen(pszPassword) > STAGEPASSWD_LENGTH) return;

	MMatchStage* pStage = NULL;

	if (uidStage == MUID(0,0)) 
	{
		return;
	} 
	else 
	{
		pStage = FindStage(uidStage);
	}

	if (pStage == NULL) 
	{
		MMatchObject* pObj = GetObject(uidPlayer);
		if (pObj != NULL)
		{
			RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_STAGE_NOT_EXIST);
		}

		return;
	}

	// øµ¿⁄≥™ ∞≥πﬂ¿⁄∏ÅEπ´Ω√..

	bool bSkipPassword = false;

	MMatchObject* pObj = GetObject(uidPlayer);

	if ((pObj == NULL) || (pObj->GetCharInfo() == NULL)) 
		return;

	MMatchUserGradeID ugid = pObj->GetAccountInfo()->m_nUGrade;

	// Custom: Allowed UGrade 252 to join private rooms
	if (ugid == MMUG_EVENTMASTER || ugid == MMUG_DEVELOPER || ugid == MMUG_ADMIN || ugid == MMUG_EVENTTEAM)
		bSkipPassword = true;

	// ∫Òπ–πÊ¿Ã æ∆¥œ∞≈≥™ ∆–Ω∫øˆµÂ∞° ∆≤∏Æ∏ÅE∆–Ω∫øˆµÂ∞° ∆≤∑»¥Ÿ∞ÅE¿¿¥‰«—¥Ÿ.
	if(bSkipPassword==false) {
		// Custom: Added check for non-GMs joining ladder/DT matches
		if ((!pStage->IsPrivate()) || (strcmp(pStage->GetPassword(), pszPassword)) || (pStage->GetStageType() == MST_LADDER) || (pStage->GetStageType() == MST_PLAYERWARS) || (pStage->GetStageSetting() && pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT))
		{
			MMatchObject* pObj = GetObject(uidPlayer);
			if (pObj != NULL)
			{
				RouteResponseToListener(pObj, MC_MATCH_RESPONSE_STAGE_JOIN, MERR_CANNOT_JOIN_STAGE_BY_PASSWORD);
			}

			return;
		}
	}

	StageJoin(uidPlayer, pStage->GetUID());
}

void MMatchServer::OnStageFollow(const MUID& uidPlayer, const char* pszTargetName)
{
	MMatchObject* pPlayerObj = GetObject(uidPlayer);
	if (pPlayerObj == NULL) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;

	// ¿⁄±ÅE¿⁄Ω≈¿ª µ˚∂ÅE∞°∑¡∞ÅE«ﬂ¿ª∞ÊøÅE∞ÀªÅE
	if (pPlayerObj->GetUID() == pTargetObj->GetUID()) return;

	// Ω∫≈◊¿Ã∆Æ∞° ¿ﬂ∏¯µ«æÅE¿÷¥¬¡ÅE∞ÀªÅE
	if (!pPlayerObj->CheckEnableAction(MMatchObject::MMOA_STAGE_FOLLOW)) return;


	// º≠∑Œ ¥Ÿ∏• √§≥Œ¿Œ¡ÅE∞ÀªÅE
	if (pPlayerObj->GetChannelUID() != pTargetObj->GetChannelUID()) {

#ifdef _VOTESETTING
		RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		return;
	}

	// Custom: Made this message appear only when the user is not an admin/event team member
	if (!IsAdminGrade(pPlayerObj) && pPlayerObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM && (IsAdminGrade(pTargetObj) == true)) {
		NotifyMessage(pPlayerObj->GetUID(), MATCHNOTIFY_GENERAL_USER_NOTFOUND);
		return;
	}

	MMatchStage* pStage = FindStage(pTargetObj->GetStageUID());
	if (pStage == NULL) return;

	// Custom: Allow admins to spectate ladder matches
	// ≈¨∑£¿ÅE‘¿”¿∫ µ˚∂Û∞• ºÅEæ¯¿Ω
	//if (pStage->GetStageType() != MST_NORMAL) return;

	if (pStage->GetStageType() != MST_NORMAL)
	{
		if (!IsAdminGrade(pPlayerObj) && pPlayerObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM)
			return;

		// Hide player
		if (!pPlayerObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
			OnAdminHide(uidPlayer);

		StageJoin(uidPlayer, pTargetObj->GetStageUID());

		// Fix for being in game room
		OnRequestForcedEntry(pStage->GetUID(), uidPlayer);
		return;
	}

	if (pStage->IsPrivate()==false) {
		if ((pStage->GetStageSetting()->GetForcedEntry()==false) && pStage->GetState() != STAGE_STATE_STANDBY) {
			// Deny Join

#ifdef _VOTESETTING
			RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW );
#endif
		} else {
			StageJoin(uidPlayer, pTargetObj->GetStageUID());
		}
	}
	else {
		// µ˚∂Û∞°∑¡¥¬ πÊ¿Ã ∫Òπ–π¯»£∏¶ « ø‰∑Œ «“∞ÊøÅE¬ µ˚∂Û∞•ºÅEæ¯¿Ω.
		//RouteResponseToListener( pPlayerObj, MC_MATCH_RESPONSE_STAGE_FOLLOW, MERR_CANNOT_FOLLOW_BY_PASSWORD );

		// «ÿ¥ÁπÊ¿Ã ∫Òπ–πÊ¿Ã∏ÅE∫Òπ–π¯»£∏¶ ø‰±∏«—¥Ÿ.
		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_REQUIRE_PASSWORD), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		pNew->AddParameter(new MCommandParameterString((char*)pStage->GetName()));
		RouteToListener(pPlayerObj, pNew);
	}
}

void MMatchServer::OnStageLeave(const MUID& uidPlayer)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject( uidPlayer );
	if( !IsEnabledObject(pObj) ) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if( !IsEnabledObject(GetObject(uidPlayer)) )
	{
		return;
	}

	StageLeave(uidPlayer);// , uidStage);
}

void MMatchServer::OnStageRequestPlayerList(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	// MMatchStage* pStage = FindStage(uidStage);
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// πÊ¿ŒøÅE∏Ò∑œ
	MMatchObjectCacheBuilder CacheBuilder;
	CacheBuilder.Reset();
	for (MUIDRefCache::iterator i=pStage->GetObjBegin(); i!=pStage->GetObjEnd(); i++) {
		MMatchObject* pScanObj = (MMatchObject*)(*i).second;
		CacheBuilder.AddObject(pScanObj);
	}
    MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_UPDATE, this);
	RouteToListener(pObj, pCmdCacheUpdate);

	// Cast Master(πÊ¿ÅE
	MUID uidMaster = pStage->GetMasterUID();
	MCommand* pMasterCmd = CreateCommand(MC_MATCH_STAGE_MASTER, MUID(0,0));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidStage));
	pMasterCmd->AddParameter(new MCommandParameterUID(uidMaster));
	RouteToListener(pObj, pMasterCmd);

	// Cast Character Setting
	StageTeam(uidPlayer, uidStage, pObj->GetTeam());
	StagePlayerState(uidPlayer, uidStage, pObj->GetStageState());
}

void MMatchServer::OnStageEnterBattle(const MUID& uidPlayer, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	StageEnterBattle(uidPlayer, uidStage);
}

void MMatchServer::OnStageLeaveBattle(const MUID& uidPlayer, bool bGameFinishLeaveBattle)//, const MUID& uidStage)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	// Custom: if vote is going on, instantly kick the user.
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (pStage->CheckUserWasVoted(uidPlayer)) {
		pStage->KickBanPlayer(pObj->GetCharInfo()->m_szName, true);
		return;
	}
	
	StageLeaveBattle(uidPlayer, bGameFinishLeaveBattle, false);//, uidStage);
}


#include "CMLexicalAnalyzer.h"
// ∞≠≈ÅE¿”Ω√ƒ⁄µÅE
bool StageKick(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/kick") == 0) {
				if (lex.GetCount() >= 2) {
					char* pszTarget = lex.GetByStr(1);
					if (pszTarget) {
						for (MUIDRefCache::iterator itor = pStage->GetObjBegin(); 
							itor != pStage->GetObjEnd(); ++itor)
						{
							MMatchObject* pTarget = (MMatchObject*)((*itor).second);
							if (stricmp(pszTarget, pTarget->GetName()) == 0) {
								if (pTarget->GetPlace() != MMP_BATTLE) {
									pServer->StageLeave(pTarget->GetUID());//, uidStage);
									bResult = true;
								}
								// Custom: Kick player from stage while target is in battle, but the game must either be in Prepare/Finish/Exit
								else if ( pTarget->GetPlace() == MMP_BATTLE && pStage->GetRule() != NULL && pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY && pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_COUNTDOWN )
								{
									pServer->StageLeaveBattle(pTarget->GetUID(), true, true);
									pServer->StageLeave(pTarget->GetUID());
									bResult = true;
								}
								break;
							}
						}
					}
				}
			}	// Kick
		}
	}

	lex.Destroy();
	return bResult;
}

// πÊ¿Â»Æ¿Œ ¿”Ω√ƒ⁄µÅE
bool StageShowInfo(MMatchServer* pServer, const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	MMatchObject* pChar = pServer->GetObject(uidPlayer);
	if (pChar == NULL)	return false;
	MMatchStage* pStage = pServer->FindStage(uidStage);
	if (pStage == NULL) return false;
	if (uidPlayer != pStage->GetMasterUID()) return false;

	bool bResult = false;
	CMLexicalAnalyzer lex;
	lex.Create(pszChat);

	if (lex.GetCount() >= 1) {
		char* pszCmd = lex.GetByStr(0);
		if (pszCmd) {
			if (stricmp(pszCmd, "/showinfo") == 0) {
				char szMsg[256]="";
				sprintf(szMsg, "FirstMaster : (%s)", pStage->GetFirstMasterName());
				pServer->Announce(pChar, szMsg);
				bResult = true;
			}	// ShowInfo
		}
	}

	lex.Destroy();
	return bResult;
}
void MMatchServer::OnStageChat(const MUID& uidPlayer, const MUID& uidStage, char* pszChat)
{
	// RAONHAJE : ∞≠≈ÅE¿”Ω√ƒ⁄µÅE
	if (pszChat[0] == '/') {
		if (StageKick(this, uidPlayer, uidStage, pszChat))
			return;
		if (StageShowInfo(this, uidPlayer, uidStage, pszChat))
			return;
	}

	StageChat(uidPlayer, uidStage, pszChat);
}

void MMatchServer::OnGameChat(const MUID& uidPlayer, const MUID& uidStage, char* szChat, int nTeam)
{
	GameChat(uidPlayer, uidStage, szChat, nTeam);
}

void MMatchServer::OnStageStart(const MUID& uidPlayer, const MUID& uidStage, int nCountdown)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;

	if (pStage->StartGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		StageRelayMapBattleStart(uidPlayer, uidStage);

		MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_START), MUID(0,0), m_This);
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(uidStage));
		pNew->AddParameter(new MCommandParameterInt(min(nCountdown,3)));
		RouteToStage(uidStage, pNew);

		// µ∫Òø° ∑Œ±◊∏¶ ≥≤±‰¥Ÿ.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStageRelayStart(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	
	if (pStage->StartRelayGame(MGetServerConfig()->IsUseResourceCRC32CacheCheck()) == true) {
		// µ∫Òø° ∑Œ±◊∏¶ ≥≤±‰¥Ÿ.
		SaveGameLog(uidStage);
	}
}

void MMatchServer::OnStartStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(true);
}

void MMatchServer::OnStopStageList(const MUID& uidComm)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->SetStageListTransfer(false);
}

void MMatchServer::OnStagePlayerState(const MUID& uidPlayer, const MUID& uidStage, MMatchObjectStageState nStageState)
{
	StagePlayerState(uidPlayer, uidStage, nStageState);
}


//void MMatchServer::OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
//{
//	MMatchStage* pStage = FindStage(uidStage);
//	if (pStage == NULL) return;
//
//	MMatchObject* pChar = GetObject(uidPlayer);
//	if (pChar == NULL) return;
//	if (pStage->GetStageType() == MST_LADDER && !IsAdminGrade(pChar) && !pChar->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER && !pChar->GetAccountInfo()->m_nUGrade == MMUG_ADMIN || !IsAdminGrade(pChar) && !pChar->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER && !pChar->GetAccountInfo()->m_nUGrade == MMUG_ADMIN || pStage->GetStageType() == MST_PLAYERWARS && !IsAdminGrade(pChar) && !pChar->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER && !pChar->GetAccountInfo()->m_nUGrade == MMUG_MANAGER) return;
//	StageTeam(uidPlayer, uidStage, nTeam);
//}
void MMatchServer::OnStageTeam(const MUID& uidPlayer, const MUID& uidStage, MMatchTeam nTeam)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;

	StageTeam(uidPlayer, uidStage, nTeam);
}
void MMatchServer::OnStageMap(const MUID& uidStage, char* pszMapName)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ¥ÅE‚ªÛ≈¬ø°º≠∏∏ πŸ≤‹ºÅE¿÷¥Ÿ
	if (strlen(pszMapName) < 2) return;

	pStage->SetMapName( pszMapName );
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pszMapName) == 0);
	
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_MAP), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterString(pStage->GetMapName()));

	if ( MGetGameTypeMgr()->IsQuestDerived( pStage->GetStageSetting()->GetGameType()))
	{
		MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule() );
		pQuest->RefreshStageGameInfo();
	}

    RouteToStage(uidStage, pNew);
}

void MMatchServer::StageRelayMapBattleStart(const MUID& uidPlayer, const MUID& uidStage)
{// ∏±∑π¿Ã∏  º±≈√«œ∞ÅE∞‘¿” Ω√¿€ πˆ∆∞ ¥©∏£∏ÅE¥Ÿ¿Ω¿ª ºˆ«‡«—¥Ÿ
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetMasterUID() != uidPlayer) return;
	if(!pStage->IsRelayMap()) return;
	
	// ∞‘¿” √≥¿ΩΩ√¿€Ω√ √ ±‚»≠ «ÿ¡÷±ÅE
	pStage->InitCurrRelayMap();

	if (pStage->m_vecRelayMapsRemained.empty()) return;

	if((int)pStage->m_vecRelayMapsRemained.size() > MAX_RELAYMAP_LIST_COUNT)
	{// ∏  ±∏º∫¿Ã 20∞≥ √ ∞˙«œ∏ÅEø°∑Ø
		mlog("RelayMapBattleStart Fail RelayMapList MIsCorrect OverCount[%d] \n", (int)pStage->m_vecRelayMapsRemained.size());
		return;
	}

	if (pStage->m_vecRelayMapsRemained.size() != pStage->GetRelayMapListCount())
	{
		mlog("m_vecRelayMapsRemained[%d] != GetRelayMapListCount[%d]\n", (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	// √≥¿Ω Ω««‡«“ ∏ ¿ª ¡§«—¥Ÿ
	int nRelayMapIndex = 0;
	if(pStage->GetRelayMapType() == RELAY_MAP_TURN )
		nRelayMapIndex = 0; // ≥≤¿∫∞Õ¡ﬂø°º≠ √≥¿Ωπ¯¬∞ ∫Œ≈Õ Ω√¿€(∞°µ∂º∫)
	else if(pStage->GetRelayMapType() == RELAY_MAP_RANDOM)
		nRelayMapIndex = rand() % int(pStage->m_vecRelayMapsRemained.size());

	if(MMATCH_MAP_RELAYMAP == pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID)
	{
		mlog("RelayMapBattleStart Fail Type[%d], RoundCount[Curr:%d][%d], ListCount[Curr:%d][%d] \n",  
			pStage->GetRelayMapType(), pStage->m_RelayMapRepeatCountRemained, pStage->GetRelayMapRepeatCount(), (int)pStage->m_vecRelayMapsRemained.size(), pStage->GetRelayMapListCount());
		return;
	}

	char* szMapName = (char*)MGetMapDescMgr()->GetMapName(pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
	if (!szMapName)
	{
		mlog("RelayMapBattleStart Fail MapID[%d] \n", (int)pStage->m_vecRelayMapsRemained[nRelayMapIndex].nMapID);
		return;
	}

	pStage->GetStageSetting()->SetMapName(szMapName);

	// Ω««‡«— ∏±∑π¿Ã∏ ¿∫ ªË¡¶«ÿ¡ÿ¥Ÿ.
	vector<RelayMap>::iterator itor = pStage->m_vecRelayMapsRemained.begin();
	for(int i=0 ; nRelayMapIndex > i ; ++itor, ++i);// «ÿ¥ÅE¿Œµ¶Ω∫±˚›ÅE¿Ãµø
	pStage->m_vecRelayMapsRemained.erase(itor);
}

void MMatchServer::OnStageRelayMapElementUpdate(const MUID& uidStage, int nType, int nRepeatCount)
{
	MMatchStage* pStage = FindStage(uidStage);

	if (pStage == NULL) return;
	if (!pStage->IsRelayMap()) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;

	pStage->SetRelayMapType((RELAY_MAP_TYPE)nType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRepeatCount);

	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	RouteToStage(uidStage, pNew);
}

void MMatchServer::OnStageRelayMapListUpdate(const MUID& uidStage, int nRelayMapType, int nRelayMapRepeatCount, void* pRelayMapListBlob)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ¥ÅE‚ªÛ≈¬ø°º≠∏∏ πŸ≤‹ºÅE¿÷¥Ÿ

	// ∏±∑π¿Ã∏  ¡§∫∏∏¶ º≠πˆ¬  Ω∫≈◊¿Ã¡ˆ∏¶ ∞ªΩ≈
	RelayMap relayMapList[MAX_RELAYMAP_LIST_COUNT];
	for (int i = 0; i < MAX_RELAYMAP_LIST_COUNT; i++)
		relayMapList[i].nMapID = -1;
	int nRelayMapListCount = MGetBlobArrayCount(pRelayMapListBlob);
	if(nRelayMapListCount > MAX_RELAYMAP_LIST_COUNT)
		nRelayMapListCount = MAX_RELAYMAP_LIST_COUNT;
	for (int i = 0; i < nRelayMapListCount; i++)
	{
		MTD_RelayMap* pRelayMap = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		if(!MGetMapDescMgr()->MIsCorrectMap(pRelayMap->nMapID))
		{
			mlog("OnStageRelayMapListUpdate Fail MIsCorrectMap ID[%d] \n", (int)pRelayMap->nMapID);
			break;
		}
		relayMapList[i].nMapID = pRelayMap->nMapID;
	}

	pStage->SetRelayMapType((RELAY_MAP_TYPE)nRelayMapType);
	pStage->SetRelayMapRepeatCount((RELAY_MAP_REPEAT_COUNT)nRelayMapRepeatCount);
	pStage->SetRelayMapList(relayMapList);
	pStage->InitCurrRelayMap();


	// ∫˙”∞ ∏∏µÈ±ÅE ∏ ∏ÆΩ∫∆Æ ºº∆√
	pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}

	// πÊ¿Â¿Ã ∫∏≥Ω ∏±∑π¿Ã∏  ¡§∫∏∏¶ πÊø¯µÈø°∞‘ ∫∏≥ø
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	RouteToStage(uidStage, pNew);
}
void MMatchServer::OnStageRelayMapListInfo(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if(pStage == NULL) return;
	if(!pStage->IsRelayMap()) return;
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;
	// ¥ÅE‚ªÛ≈¬¿œ∂ß πÊ¿Â¿∫ √≥∏Æ æ»«ÿ¡‹(∏±∑π¿Ã∏  ¿€º∫¡ﬂ¿œºˆµµ ¿÷¿Ω)
	if(pStage->GetState() == STAGE_STATE_STANDBY && pStage->GetMasterUID() == uidChar) return;	

	// ∫˙”∞ ∏∏µÈ±ÅE ∏ ∏ÆΩ∫∆Æ ºº∆√
	void* pRelayMapListBlob = MMakeBlobArray(sizeof(MTD_RelayMap), pStage->GetRelayMapListCount());
	RelayMap RelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(RelayMapList, pStage->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	for (int i = 0; i < pStage->GetRelayMapListCount(); i++)
	{
		MTD_RelayMap* pRelayMapList = (MTD_RelayMap*)MGetBlobArrayElement(pRelayMapListBlob, i);
		pRelayMapList->nMapID = RelayMapList[i].nMapID;
	}
	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE), MUID(0,0), m_This);
	pNew->AddParameter(new MCommandParameterUID(uidStage));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapType()));
	pNew->AddParameter(new MCommandParameterInt((int)pStage->GetRelayMapRepeatCount()));
	pNew->AddParameter(new MCommandParameterBlob(pRelayMapListBlob, MGetBlobArraySize(pRelayMapListBlob)));
	MEraseBlobArray(pRelayMapListBlob);

	RouteToListener(pObj, pNew); // πÊ¿Â¿Ã ∏±∑π¿Ã∏  º≥¡§¡ﬂø° æ˜µ•¿Ã∆Æµ» º≥¡§¿∏∑Œ ∫Ø∞ÅEµ…ºˆ∞° ¿÷¿Ω
}

void MMatchServer::OnStageSetting(const MUID& uidPlayer, const MUID& uidStage, void* pStageBlob, int nStageCount)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	if (pStage->GetState() != STAGE_STATE_STANDBY) return;	// ¥ÅE‚ªÛ≈¬ø°º≠∏∏ πŸ≤‹ºÅE¿÷¥Ÿ
	if (nStageCount <= 0) return;

	// validate
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) {
		mlog(" stage setting invalid object (%d, %d) ignore\n", uidPlayer.High, uidPlayer.Low);
		return;
	}

	if( pObj->GetStageUID()!=uidStage ||  nStageCount!=1 ||
		MGetBlobArraySize(pStageBlob) != (sizeof(MSTAGE_SETTING_NODE)+sizeof(int)*2) )
	{
		mlog(" stage setting hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		return;
	}

	// πÊ¿Â¿Ã∞≈≥™ ø˚€µ¿⁄∞° æ∆¥—µ• ºº∆√¿ª πŸ≤Ÿ∏ÅE±◊≥… ∏Æ≈œ
	if (pStage->GetMasterUID() != uidPlayer)
	{
		MMatchObject* pObjMaster = GetObject(uidPlayer);
		if (!IsAdminGrade(pObjMaster)) return;
	}


	MSTAGE_SETTING_NODE* pNode = (MSTAGE_SETTING_NODE*)MGetBlobArrayElement(pStageBlob, 0);

	// let's refactor
	if( (pNode->nGameType < MMATCH_GAMETYPE_DEATHMATCH_SOLO) || (pNode->nGameType >= MMATCH_GAMETYPE_MAX)) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );

		// µ∫Òø° ≥≤±‚¿⁄.
//		pObj->SetInvalidStageSettingDisconnectWaitInfo();
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );

		return;
	}

	// º≠πŸ¿Ãπ˙¿Ã ∫Ò»∞º∫ ºº∆√¿Œµ• º≠πŸ¿ÃπÅEø‰√ªΩ√
	if( MGetServerConfig()->IsEnabledSurvivalMode()==false && pNode->nGameType==MMATCH_GAMETYPE_SURVIVAL) {
		mlog(" stage setting game mode hack %s (%d, %d) ignore\n", pObj->GetName(), uidPlayer.High, uidPlayer.Low);
		LogObjectCommandHistory( uidPlayer );
		pObj->DisconnectHacker( MMHT_INVALIDSTAGESETTING );
		return;
	}

	// ±‚∫ª¿˚¿∏∑Œ √÷¥ÅE¿Œø¯¿Ã STAGE_BASIC_MAX_PLAYERCOUNT¿Ã ≥—¿∏∏ÅESTAGE_BASIC_MAX_PLAYERCOUNT∑Œ ∏¬√Á¡‹.
	// ≥≤¿∫ ¿€æ˜¿ª ¡¯«‡«œ∏È¿∫ ∞¢ ∞‘¿”ø° ∏¬¥¬ ¿Œø¯¿∏∑Œ º¬∆√¿ª «‘. - by SungE 2007-05-14
	if( STAGE_MAX_PLAYERCOUNT < pNode->nMaxPlayers )
		pNode->nMaxPlayers = STAGE_MAX_PLAYERCOUNT;

	// ¿Ã ¿ÃªÛ¿« ∂Ûø˚—ÅEº¬∆√¿∫ ∫“∞°¥… «œ¥Ÿ. π´¡∂∞« ∫∏¡§«—¥Ÿ. - By SungE 2007-11-07
	if( STAGE__MAX_ROUND < pNode->nRoundMax )
		pNode->nRoundMax = STAGE__MAX_ROUND;

	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	MMatchChannel* pChannel = FindChannel(pStage->GetOwnerChannel());

	bool bCheckChannelRule = true;

	if (QuestTestServer())
	{
		if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) || pNode->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			bCheckChannelRule = false;
		}
	}

	if ((pChannel) && (bCheckChannelRule))
	{
		// ºº∆√«“ ºÅE¿÷¥¬ ∏ , ∞‘¿”≈∏¿‘¿Œ¡ÅE√º≈©
		MChannelRule* pRule = MGetChannelRuleMgr()->GetRule(pChannel->GetRuleType());
		if (pRule)
		{
			if (!pRule->CheckGameType(pNode->nGameType))
			{
				pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
			}

			bool bDuelMode = false;
			bool bCTFMode = false;

#ifdef _CUSTOMRULEMAPS
			bool bSWRMode = false;
#endif

			if ( pNode->nGameType == MMATCH_GAMETYPE_DUEL)
				bDuelMode = true;

			if ( pNode->nGameType == MMATCH_GAMETYPE_CTF)
				bCTFMode = true;

			if (pNode->nGameType == MMATCH_GAMETYPE_DROPMAGIC)
				bSWRMode = true;

#ifdef _CUSTOMRULEMAPS
			if (!pRule->CheckSWRMap(pNode->nMapIndex) && bSWRMode)
			{
				strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
				pNode->nMapIndex = 0;
			}
#endif

			if (bCTFMode)
			{
				// map index sucks (broken)
				if (!pRule->CheckCTFMap(pNode->szMapName))
				{
					strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
					pNode->nMapIndex = 0;
				}
				else
				{
					strcpy(pNode->szMapName, pSetting->GetMapName());
					pNode->nMapIndex = pSetting->GetMapIndex();
				}
			}
			else
			{
				if (!pRule->CheckMap(pNode->nMapIndex, bDuelMode))
				{
					strcpy(pNode->szMapName, MGetMapDescMgr()->GetMapName(MMATCH_MAP_MANSION));
					pNode->nMapIndex = 0;
				}
				else
				{
					strcpy(pNode->szMapName, pSetting->GetMapName());
					pNode->nMapIndex = pSetting->GetMapIndex();
				}
			}
		}
	}

	// Custom: don't start quest games which contain roomtags. Not the most elegant solution but it works.
	if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) || pNode->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE) {
		if (strstr(pStage->GetName(), "[") || strstr(pStage->GetName(), "]")) {
			MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			pCmd->AddParameter(new MCmdParamStr("Roomtags can not be used in quest games."));
			RouteToStage(pStage->GetUID(), pCmd);
			
			pNode->nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
		}
	}
	// Custom: No Teleport
	if (MGetGameTypeMgr()->IsTeamGame(pNode->nGameType) ||
		pNode->nGameType == MMATCH_GAMETYPE_DUEL || 
		pNode->nGameType == MMATCH_GAMETYPE_GUNGAME ||
		pNode->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO ||
		pNode->nGameType == MMATCH_GAMETYPE_BERSERKER ||
		pNode->nGameType == MMATCH_GAMETYPE_DROPMAGIC ||
		pNode->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO ||
		pNode->nGameType == MMATCH_GAMETYPE_BERSERKER ||
		pNode->nGameType == MMATCH_GAMETYPE_SPY)
	{
		if (strstr(pStage->GetName(), "[tele]"))
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUInt(0));
			pCmd->AddParameter(new MCmdParamStr("This TAG is only available for Training!"));
			RouteToStage(pStage->GetUID(), pCmd);

			pNode->nGameType = MMATCH_GAMETYPE_TRAINING;
		}
	}
	MMATCH_GAMETYPE nLastGameType = pSetting->GetGameType();

	if (MGetGameTypeMgr()->IsQuestDerived(pNode->nGameType) || pNode->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		pNode->nMaxPlayers = STAGE_QUEST_MAX_PLAYER;
		pNode->nLimitTime = STAGESETTING_LIMITTIME_UNLIMITED;/*
		GetDBMgr()->GetStageBestTime(pSetting->GetMapName(), pSetting);
		strcpy_s(pNode->szStageBestTime, pSetting->GetStageBestTime());*/
	}

	if (!MGetGameTypeMgr()->IsTeamGame(pNode->nGameType) && !MGetGameTypeMgr()->IsSpyGame(pNode->nGameType))
	{
		pNode->bAutoTeamBalancing = true;
	}
	else if (MGetGameTypeMgr()->IsSpyGame(pNode->nGameType))
	{
		pNode->bAutoTeamBalancing = false;
		pNode->nMaxPlayers = 12;
	}
	// ∏±∑π¿Ã∏  ºº∆√
	pStage->SetIsRelayMap(strcmp(MMATCH_MAPNAME_RELAYMAP, pNode->szMapName) == 0);
	pStage->SetIsStartRelayMap(false);

	if(!pStage->IsRelayMap())
	{	// ∏±∑π¿Ã∏ ¿Ã æ∆¥œ∏ÅE±‚∫ª¿∏∑Œ √ ±‚»≠ «ÿ¡ÿ¥Ÿ.
		pNode->bIsRelayMap = pStage->IsRelayMap();
		pNode->bIsStartRelayMap = pStage->IsStartRelayMap();
		for (int i=0; i<MAX_RELAYMAP_LIST_COUNT; ++i)
			pNode->MapList[i].nMapID = -1;
		pNode->nRelayMapListCount = 0;
		pNode->nRelayMapType = RELAY_MAP_TURN;
		pNode->nRelayMapRepeatCount = RELAY_MAP_3REPEAT;
	}

	pSetting->UpdateStageSetting(pNode);
	pStage->ChangeRule(pNode->nGameType);


	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	RouteToStage(uidStage, pCmd);


	// ∞‘¿” ∏µÂ∞° ∫Ø∞Êµ«æ˙¿ª∞ÊøÅE±‚¡∏ ∏ ¿Ã ∫Œ¿˚¿˝«œ¥Ÿ∏ÅE∏µÂø° ∏¬¥¬ ∏ ¿∏∑Œ πŸ≤„¡ÿ¥Ÿ
	if (nLastGameType != pSetting->GetGameType())
	{
		char szNewMap[ MAPNAME_LENGTH ] = {0};

		// ∆Øºˆ«— ∏µÂµÅE(∏  ∏Ò∑œ¿Ã ¿Ø∫∞≥≠ ∞ÕµÅE
		bool bLast_QuestOrSurvival = MGetGameTypeMgr()->IsQuestDerived( nLastGameType );
		bool bCurr_QuestOrSurvival = MGetGameTypeMgr()->IsQuestDerived( pSetting->GetGameType() );
		
		bool bLast_Duel = nLastGameType == MMATCH_GAMETYPE_DUEL;
		bool bCurr_Duel = pSetting->GetGameType() == MMATCH_GAMETYPE_DUEL;

		bool bLast_ChallengeQuest = nLastGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE;
		bool bCurr_ChallengeQuest = pSetting->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE;

		// ±◊ø‹ ¿œπ›¿˚¿Œ ∏µÂµÅE(µ•Ω∫∏≈ƒ° µÅE
		bool bLast_Other = !bLast_QuestOrSurvival && !bLast_Duel && !bLast_ChallengeQuest;
		bool bCurr_Other = !bCurr_QuestOrSurvival && !bCurr_Duel && !bCurr_ChallengeQuest;

		// ƒ˘Ω∫∆Æ≥™ º≠πŸ¿Ãπ˙ø° ∏¬¥¬ ∏ ¿∏∑Œ ∫Ø∞ÅE
		if ( !bLast_QuestOrSurvival && bCurr_QuestOrSurvival)
		{
			OnStageMap(uidStage, MMATCH_DEFAULT_STAGESETTING_MAPNAME);

			MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule());
			pQuest->RefreshStageGameInfo();
		}
		// µ‡æÛø° ∏¬¥¬ ∏ ¿∏∑Œ ∫Ø∞ÅE
		else if ( !bLast_Duel && bCurr_Duel)
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_HALL));
			OnStageMap(uidStage, szNewMap);
		}
		else if ( !bLast_ChallengeQuest && bCurr_ChallengeQuest)
		{
			MNewQuestScenarioManager* pScenarioMgr = MMatchRuleQuestChallenge::GetScenarioMgr();
			_ASSERT(pScenarioMgr);
			if (pScenarioMgr)
				OnStageMap(uidStage, (char*)pScenarioMgr->GetDefaultScenarioName());
		}
		// ¿ß¿« ∏µÂµÈø°º≠ ±‚≈∏ ∏µÂ∑Œ πŸ≤ÅE∞ÊøÅEMansion¿∏∑Œ ºº∆√
		else if ( !bLast_Other && bCurr_Other)
		{
			strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_MANSION));
			OnStageMap(uidStage, szNewMap);
		}

		//if (MGetGameTypeMgr()->IsQuestDerived( nLastGameType ) == false &&
		//	MGetGameTypeMgr()->IsQuestDerived( pSetting->GetGameType() ) == true)
		//{
		//	OnStageMap(uidStage, MMATCH_DEFAULT_STAGESETTING_MAPNAME);

		//	MMatchRuleBaseQuest* pQuest = reinterpret_cast< MMatchRuleBaseQuest* >( pStage->GetRule());
		//	pQuest->RefreshStageGameInfo();
		//}
		//else if ( (nLastGameType != MMATCH_GAMETYPE_DUEL) && ( pSetting->GetGameType() == MMATCH_GAMETYPE_DUEL))
		//{
		//	strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_HALL));
		//	OnStageMap(uidStage, szNewMap);
		//}
		//else if ( ((nLastGameType == MMATCH_GAMETYPE_QUEST) || (nLastGameType == MMATCH_GAMETYPE_SURVIVAL) || (nLastGameType == MMATCH_GAMETYPE_DUEL)) &&
		//	      ((pSetting->GetGameType() != MMATCH_GAMETYPE_QUEST) && (pSetting->GetGameType() != MMATCH_GAMETYPE_SURVIVAL) && ( pSetting->GetGameType() != MMATCH_GAMETYPE_DUEL)))
		//{
		//	strcpy( szNewMap, MGetMapDescMgr()->GetMapName( MMATCH_MAP_MANSION));
		//	OnStageMap(uidStage, szNewMap);
		//}
	}

}

void MMatchServer::OnRequestStageSetting(const MUID& uidComm, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	MCommand* pCmd = CreateCmdResponseStageSetting(uidStage);
	pCmd->m_Receiver = uidComm;
	Post(pCmd);

	// ∏  º±≈√¿Ã ∏±∑π¿Ã∏ ¿Ã∏ÅE√≥∏Æ«ÿ¡ÿ¥Ÿ.
	OnStageRelayMapListInfo(uidStage, uidComm);

	MMatchObject* pChar = GetObject(uidComm);

	// Custom: Don't show firstmaster at all.
	//if (pChar && (MMUG_EVENTMASTER == pChar->GetAccountInfo()->m_nUGrade)) 	{
	//{
	//	// ¿Ã∫•∆Æ ∏∂Ω∫≈Õø°∞‘ √≥¿Ω πÊ∏∏µÈæ˙¥ÅEªÁ∂˜¿ª æÀ∑¡¡ÿ¥Ÿ
	//	StageShowInfo(this, uidComm, uidStage, "/showinfo");
	//}
}

void MMatchServer::OnRequestPeerList(const MUID& uidChar, const MUID& uidStage)
{
	ResponsePeerList(uidChar, uidStage);
}

void MMatchServer::OnRequestGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	ResponseGameInfo(uidChar, uidStage);
}

void MMatchServer::ResponseGameInfo(const MUID& uidChar, const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage); if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar); if (pObj == NULL) return;
	if (pStage->GetRule() == NULL) return;

	// Custom: Larger Game Info Packet, deprecate the old one
	MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_GAME_INFO_V2, MUID(0,0));
	pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));

	void* pGameInfoArray = MMakeBlobArray(sizeof(MTD_GameInfo_V2), 1);
	MTD_GameInfo_V2* pGameItem = (MTD_GameInfo_V2*)MGetBlobArrayElement(pGameInfoArray, 0);
	memset(pGameItem, 0, sizeof(MTD_GameInfo_V2));
	
	if (pStage->GetStageSetting()->IsTeamPlay())
	{
		pGameItem->nRedTeamScore = pStage->GetTeamScore(MMT_RED);
		pGameItem->nBlueTeamScore = pStage->GetTeamScore(MMT_BLUE);

		pGameItem->nRedTeamKills = pStage->GetTeamKills(MMT_RED);
		pGameItem->nBlueTeamKills = pStage->GetTeamKills(MMT_BLUE);
	}

	// ∞‘¿”¡§∫∏
	//void* pGameInfoArray = MMakeBlobArray(sizeof(MTD_GameInfo), 1);
	//MTD_GameInfo* pGameItem = (MTD_GameInfo*)MGetBlobArrayElement(pGameInfoArray, 0);
	//memset(pGameItem, 0, sizeof(MTD_GameInfo));
	//
	//if (pStage->GetStageSetting()->IsTeamPlay())
	//{
	//	pGameItem->nRedTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_RED));
	//	pGameItem->nBlueTeamScore = static_cast<char>(pStage->GetTeamScore(MMT_BLUE));

	//	pGameItem->nRedTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_RED));
	//	pGameItem->nBlueTeamKills = static_cast<short>(pStage->GetTeamKills(MMT_BLUE));
	//}

	pNew->AddParameter(new MCommandParameterBlob(pGameInfoArray, MGetBlobArraySize(pGameInfoArray)));
	MEraseBlobArray(pGameInfoArray);

	// ∑ÅE§∫∏
	void* pRuleInfoArray = NULL;
	if (pStage->GetRule())
		pRuleInfoArray = pStage->GetRule()->CreateRuleInfoBlob();
	if (pRuleInfoArray == NULL)
		pRuleInfoArray = MMakeBlobArray(0, 0);
	pNew->AddParameter(new MCommandParameterBlob(pRuleInfoArray, MGetBlobArraySize(pRuleInfoArray)));
	MEraseBlobArray(pRuleInfoArray);

	// Battleø° µÈæ˚Ã£ ªÁ∂˜∏∏ List∏¶ ∏∏µÁ¥Ÿ.
	int nPlayerCount = pStage->GetObjInBattleCount();

	void* pPlayerItemArray = MMakeBlobArray(sizeof(MTD_GameInfoPlayerItem), nPlayerCount);
	int nIndex=0;
	for (MUIDRefCache::iterator itor=pStage->GetObjBegin(); itor!=pStage->GetObjEnd(); itor++) 
	{
		MMatchObject* pObj = (MMatchObject*)(*itor).second;
		if (pObj->GetEnterBattle() == false) continue;

		MTD_GameInfoPlayerItem* pPlayerItem = (MTD_GameInfoPlayerItem*)MGetBlobArrayElement(pPlayerItemArray, nIndex++);
		pPlayerItem->uidPlayer = pObj->GetUID();
		pPlayerItem->bAlive = pObj->CheckAlive();
		pPlayerItem->nKillCount = pObj->GetAllRoundKillCount();
		pPlayerItem->nDeathCount = pObj->GetAllRoundDeathCount();
	}
	pNew->AddParameter(new MCommandParameterBlob(pPlayerItemArray, MGetBlobArraySize(pPlayerItemArray)));
	MEraseBlobArray(pPlayerItemArray);

	RouteToListener(pObj, pNew);
}

void MMatchServer::OnMatchLoadingComplete(const MUID& uidPlayer, int nPercent)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_LOADING_COMPLETE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidPlayer));
	pCmd->AddParameter(new MCmdParamInt(nPercent));
	RouteToStage(pObj->GetStageUID(), pCmd);	
}


void MMatchServer::OnGameRoundState(const MUID& uidStage, int nState, int nRound)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	pStage->RoundStateFromClient(uidStage, nState, nRound);
}


void MMatchServer::OnDuelSetObserver(const MUID& uidChar)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	MCommand* pCmd = CreateCommand(MC_MATCH_SET_OBSERVER, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnRequestRoll(const MUID& uidRoller)
{
	MMatchObject* pObj = GetObject(uidRoller);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// Don't give rolls when hiding
	if ((IsAdminGrade(pObj) || pObj->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		return;

	char szStageNameCopy[256]; //Thanks Linear for this code. :> 
	strcpy( szStageNameCopy, pStage->GetName() );

	for( int i = 0; i < strlen( szStageNameCopy ); ++i )
		szStageNameCopy[i] = tolower(szStageNameCopy[i]);
	
	if(strstr(szStageNameCopy, "[rtd]"))
	{
		char szMsg[256];
		memset(szMsg, 0, sizeof(szMsg));

		int rolledDice = 0;
		srand(timeGetTime());
		rolledDice = rand() % 11 + 1;

		switch(rolledDice)
		{
		case 1:
			sprintf_s( szMsg, "'%s' rolled 1: Zoomed Out.", pObj->GetName());
			break;
		case 2:
			sprintf_s( szMsg, "'%s' rolled 2: Near Death.", pObj->GetName());
			break;
		case 3:
			sprintf_s( szMsg, "'%s' rolled 3: ESP.", pObj->GetName());
			break;
		case 4:
			sprintf_s( szMsg, "'%s' rolled 4: Super Tank.", pObj->GetName());
			break;
		case 5:
			sprintf_s( szMsg, "'%s' rolled 5: Shield Mode.", pObj->GetName());
			break;
		case 6:
			sprintf_s( szMsg, "'%s' rolled 6: Ninja Jump.", pObj->GetName());
			break;
		case 7:
			sprintf_s( szMsg, "'%s' rolled 7: Speed Demon.", pObj->GetName());
			break;
		case 8:
			sprintf_s( szMsg, "'%s' rolled 8: Slow Down.", pObj->GetName());
			break;
		case 9:
			sprintf_s( szMsg, "'%s' rolled 9: Bunny Mode.", pObj->GetName());
			break;
		case 10:
			sprintf_s( szMsg, "'%s' rolled 10: Stronger Legs.", pObj->GetName());
			break;
		case 11:
			sprintf_s( szMsg, "'%s' rolled 11: Wireframe.", pObj->GetName());
		}

		MCommand* pCmd = CreateCommand(MC_MATCH_RESPONSE_ROLL, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(uidRoller));
			pCmd->AddParameter(new MCmdParamInt(rolledDice));
			pCmd->AddParameter(new MCmdParamStr(szMsg));
		RouteToStage(pStage->GetUID(), pCmd);
	}
}

void MMatchServer::OnRequestSpawn(const MUID& uidChar, const MVector& pos, const MVector& dir)
{
	MMatchObject* pObj = GetObject(uidChar);
	if (pObj == NULL) return;

	// Do Not Spawn when AdminHiding
	if (IsAdminGrade(pObj) && pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide)) return;


	// ∏∂¡ˆ∏∑ ¡◊æ˙¥ÅEΩ√∞£∞ÅEªı∑Œ ∏ÆΩ∫∆˘¿ª ø‰√ª«— Ω√∞£ ªÁ¿Ãø° 2√  ¿ÃªÛ¿« Ω√∞£¿Ã ¿÷æ˙¥¬¡ÅE∞ÀªÁ«—¥Ÿ.
	DWORD dwTime = timeGetTime() - pObj->GetLastSpawnTime();	
	if ( dwTime < RESPAWN_DELAYTIME_AFTER_DYING_MIN) return;
	pObj->SetLastSpawnTime(timeGetTime());

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;
	if ( (pStage->GetRule()->GetRoundState() != MMATCH_ROUNDSTATE_PREPARE) && (!pObj->IsEnabledRespawnDeathTime(GetTickTime())) )
		 return;

	MMatchRule* pRule = pStage->GetRule();
	MMATCH_GAMETYPE gameType = pRule->GetGameType();
	if (gameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pDuel = (MMatchRuleDuel*)pRule;
		if (uidChar != pDuel->uidChampion && uidChar != pDuel->uidChallenger)
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}
#ifdef _SPYMODE
	else if (gameType == MMATCH_GAMETYPE_SPY)
	{
		MMatchRuleSpy* pSpy = (MMatchRuleSpy*)pRule;
		if (!pSpy->IsParticipant(uidChar))
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}
#endif
	else if (MGetGameTypeMgr()->IsQuestDerived((gameType)))
	{
		MMatchRuleBaseQuest* pQst = (MMatchRuleBaseQuest*)pRule;
		if (pQst && pQst->GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
		{
			OnDuelSetObserver(uidChar);
			return;
		}
	}
	pObj->ResetCustomItemUseCount();
	pObj->SetAlive(true);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_SPAWN, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(uidChar));
	pCmd->AddParameter(new MCmdParamShortVector(pos.x, pos.y, pos.z));
	pCmd->AddParameter(new MCmdParamShortVector(DirElementToShort(dir.x), DirElementToShort(dir.y), DirElementToShort(dir.z)));
	RouteToBattle(pObj->GetStageUID(), pCmd);
}

void MMatchServer::OnGameRequestTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	pSync->Update(GetGlobalClockCount());

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_RESPONSE_TIMESYNC, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(nLocalTimeStamp));
	pCmd->AddParameter(new MCmdParamUInt(GetGlobalClockCount()));
	RouteToListener(pObj, pCmd);
}

void MMatchServer::OnGameReportTimeSync(const MUID& uidComm, unsigned long nLocalTimeStamp, unsigned int nDataChecksum)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (pObj == NULL) return;

	pObj->UpdateTickLastPacketRecved();	// Last Packet Timestamp

	if (pObj->GetEnterBattle() == false)
		return;

	//// SpeedHack Test ////
	MMatchTimeSyncInfo* pSync = pObj->GetSyncInfo();
	int nSyncDiff = nLocalTimeStamp - pSync->GetLastSyncClock();
	float fError = static_cast<float>(nSyncDiff) / static_cast<float>(MATCH_CYCLE_CHECK_SPEEDHACK);
	if (fError > 2.f) {	
		pSync->AddFoulCount();
		if (pSync->GetFoulCount() >= 3) {	// 3ø¨º” Ω∫««µÂ«Ÿ ∞À√ÅE- 3¡¯æ∆øÅE

			#ifndef _DEBUG		// µπˆ±◊«“∂ß¥¬ ª©≥ıæ“¿Ω
				NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_SPEEDHACK);
				StageLeave(pObj->GetUID());//, pObj->GetStageUID());
				Disconnect(pObj->GetUID());
			#endif
			mlog("SPEEDHACK : User='%s', SyncRatio=%f (TimeDiff=%d) \n", 
				pObj->GetName(), fError, nSyncDiff);
			pSync->ResetFoulCount();
		}
	} else {
		pSync->ResetFoulCount();
	}
	pSync->Update(GetGlobalClockCount());

	//// MemoryHack Test ////
	if (nDataChecksum > 0) {	// º≠πˆ∞° Client MemoryChecksum ∏∏£π«∑Œ ¿œ¥‹ ≈¨∂Û¿Ãæ∆Æ∞° Ω≈∞˙„œ¥¬¿«πÃ∑Œ ªÁøÅE—¥Ÿ.
		NotifyMessage(pObj->GetUID(), MATCHNOTIFY_GAME_MEMORYHACK);
		StageLeave(pObj->GetUID());//, pObj->GetStageUID());
		Disconnect(pObj->GetUID());
		mlog("MEMORYHACK : User='%s', MemoryChecksum=%u \n", pObj->GetName(), nDataChecksum);
	}
}

void MMatchServer::OnUpdateFinishedRound(const MUID& uidStage, const MUID& uidChar, 
						   void* pPeerInfo, void* pKillInfo)
{

}

void MMatchServer::OnRequestForcedEntry(const MUID& uidStage, const MUID& uidChar)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;
	MMatchObject* pObj = GetObject(uidChar);	
	if (pObj == NULL) return;

	pObj->SetForcedEntry(true);

	// Custom: Allow admins to spectate ladder matches
	// probably the best temporary fix I could provide so far without updating everything
	if (pStage->GetStageType() == MST_LADDER && pStage->GetStageType() == MST_PLAYERWARS)
	{
		if (!pObj->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		{
			RouteResponseToListener(pObj, MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MERR_CANNOT_JOIN_STAGE_BY_FORCEDENTRY);
			return;
		}

		MCommand* pCmd = CreateCommand(MC_MATCH_LADDER_LAUNCH, uidChar);
		pCmd->AddParameter(new MCmdParamUID(uidStage));
		pCmd->AddParameter(new MCmdParamStr( const_cast<char*>(pStage->GetMapName()) ));
		RouteToListener(pObj, pCmd);
	}

	if (MGetGameTypeMgr()->IsQuestDerived(pStage->GetStageSetting()->GetGameType()) || MGetGameTypeMgr()->IsQuestChallengeOnly(pStage->GetStageSetting()->GetGameType()))
	{
		if (pStage->m_pRule)
		{
			MUID uidChar = pObj->GetUID();
			pStage->m_pRule->OnQuestEnterBattle(uidChar);
		}
	}

	RouteResponseToListener(pObj, MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY, MOK);
}

void MMatchServer::OnRequestSuicide(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ReserveSuicide( uidPlayer, MGetMatchServer()->GetGlobalClockCount() );

	// OnGameKill(uidPlayer, uidPlayer);

	//MCommand* pNew = CreateCommand(MC_MATCH_RESPONSE_SUICIDE, MUID(0,0));
	//int nResult = MOK;
	//pNew->AddParameter(new MCommandParameterInt(nResult));
	//pNew->AddParameter(new MCommandParameterUID(uidPlayer));
	//RouteToBattle(pObj->GetStageUID(), pNew);
}

void MMatchServer::OnRequestObtainWorldItem(const MUID& uidPlayer, const int nItemUID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->ObtainWorldItem(pObj, nItemUID);
}
// Custom Fix Spy Landmine
void MMatchServer::OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z, float fDropDelayTime)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (!pObj->IsHaveCustomItem())
		return;
	if (pObj->IncreaseCustomItemUseCount())
	{
		pStage->RequestSpawnWorldItem(pObj, nItemID, x, y, z, fDropDelayTime);
	}
}
//void MMatchServer::OnRequestSpawnWorldItem(const MUID& uidPlayer, const int nItemID, const float x, const float y, const float z, float fDropDelayTime)
//{
//	MMatchObject* pObj = GetObject(uidPlayer);
//	if (pObj == NULL) return;
//	MMatchStage* pStage = FindStage(pObj->GetStageUID());
//	if (pStage == NULL) return;
//
//	if (pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY && nItemID == MSPYMODE_WORLDITEM_LANDMINE_ID)
//	{
//		pStage->RequestSpawnWorldItem(pObj, nItemID, x, y, z, fDropDelayTime);
//		return;
//	}
//
//	if (!pObj->IsHaveCustomItem())
//		return;
//}
void MMatchServer::OnNotifyThrowTrapItem(const MUID& uidPlayer, const int nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (!pObj->IsEquipCustomItem(nItemID))
		return;

	pStage->OnNotifyThrowTrapItem(uidPlayer, nItemID);
}

void MMatchServer::OnNotifyActivatedTrapItem(const MUID& uidPlayer, const int nItemID, const MVector3& pos)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	pStage->OnNotifyActivatedTrapItem(uidPlayer, nItemID, pos);
}

float MMatchServer::GetDuelVictoryMultiflier(int nVictorty)
{
	return 1.0f;
}

float MMatchServer::GetDuelPlayersMultiflier(int nPlayerCount)
{
	return 1.0f;
}

void MMatchServer::CalcExpOnGameKill(MMatchStage* pStage, MMatchObject* pAttacker, MMatchObject* pVictim, 
					   int* poutAttackerExp, int* poutVictimExp)
{
	bool bSuicide = false;		// ¿⁄ªÅE
	if (pAttacker == pVictim) bSuicide = true;		

	MMATCH_GAMETYPE nGameType = pStage->GetStageSetting()->GetGameType();
	float fGameExpRatio = MGetGameTypeMgr()->GetInfo(nGameType)->fGameExpRatio;

	// ∞‘¿”≈∏¿‘¿Ã Training¿Ã∏ÅEπŸ∑Œ 0∏Æ≈œ
	// Custom: Game modes
	if (nGameType == MMATCH_GAMETYPE_TRAINING || nGameType == MMATCH_GAMETYPE_TEAM_TRAINING)
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	// ∞‘¿”≈∏¿‘¿Ã πˆº≠ƒø¿œ ∞ÊøÅE
	else if (nGameType == MMATCH_GAMETYPE_BERSERKER)
	{
		MMatchRuleBerserker* pRuleBerserker = (MMatchRuleBerserker*)pStage->GetRule();

		if (pRuleBerserker->GetBerserker() == pAttacker->GetUID())
		{
			if (pAttacker != pVictim)
			{
				// πˆº≠ƒø¥¬ ∞Ê«Ëƒ°∏¶ 80%∏∏ »πµÊ«—¥Ÿ.
				fGameExpRatio = fGameExpRatio * 0.8f;
			}
			else
			{
				// πˆº≠ƒø¥¬ ¿⁄ªÅE∂«¥¬ ««∞° ¡ŸæÅE¡◊¥¬∞ÊøÅEº’Ω« ∞Ê«Ëƒ°¥¬ æ¯µµ∑œ «—¥Ÿ.
				fGameExpRatio = 0.0f;
			}
		}
	}
	else if (nGameType == MMATCH_GAMETYPE_DUEL)
	{
		MMatchRuleDuel* pRuleDuel = (MMatchRuleDuel*)pStage->GetRule();
		if (pVictim->GetUID() == pRuleDuel->uidChallenger)
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory());
		}
		else
		{
			fGameExpRatio *= GetDuelVictoryMultiflier(pRuleDuel->GetVictory()) * GetDuelPlayersMultiflier(pStage->GetPlayers());

		}
//		if (pRuleDuel->GetVictory() <= 1)
//		{
//			fGameExpRatio = fGameExpRatio * GetDuelPlayersMultiflier(pStage->GetPlayers()) * GetDuelVictoryMultiflier()
//		}
	}
	else if (nGameType == MMATCH_GAMETYPE_CTF)
	{
		MMatchRuleTeamCTF* pRuleCTF = (MMatchRuleTeamCTF*)pStage->GetRule();
		if (pAttacker != pVictim)
		{
			if(pAttacker->GetTeam() != pVictim->GetTeam() && (pVictim->GetUID() == pRuleCTF->GetBlueCarrier() || pVictim->GetUID() == pRuleCTF->GetRedCarrier()))
				fGameExpRatio *= 2;
		}
		else
		{
			fGameExpRatio = 0.0f;
		}
	}
	else if (nGameType == MMATCH_GAMETYPE_INFECTED)
	{
		MMatchRuleTeamInfected* pRuleInfected = (MMatchRuleTeamInfected*)pStage->GetRule();
		if (pAttacker != pVictim)
		{
			if(pAttacker->GetTeam() != pVictim->GetTeam())
			{
				if (pVictim->GetTeam() == MMT_RED)
					fGameExpRatio *= 3;
				else if (pVictim->GetTeam() == MMT_BLUE)
					fGameExpRatio *= 1.5f;
			}
		}
		else
		{
			fGameExpRatio = 0.0f;
		}
	}

	// ∏ , ∞‘¿”≈∏¿‘ø° ¥ÅE— ∞Ê«Ëƒ° ∫Ò¿≤ ¿˚øÅE
	int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
	if ((nMapIndex >=0) && (nMapIndex < MMATCH_MAP_COUNT))
	{
		float fMapExpRatio = MGetMapDescMgr()->GetExpRatio(nMapIndex);
		fGameExpRatio = fGameExpRatio * fMapExpRatio;
	}

	int nAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	// ∞Ê«Ëƒ° ∞ËªÅE
	int nAttackerExp = (int)(MMatchFormula::GetGettingExp(nAttackerLevel, nVictimLevel) * fGameExpRatio);
	int nVictimExp = (int)(MMatchFormula::CalcPanaltyEXP(nAttackerLevel, nVictimLevel) * fGameExpRatio);


#ifndef _EXPCHANNEL
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pStage->GetStageType() == MST_LADDER && pStage->GetStageType() == MST_PLAYERWARS))
	{
		nAttackerExp = (int)((float)nAttackerExp * 1.5f);
		nVictimExp = 0;
	}
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if (pOwnerChannel && strstr(pOwnerChannel->GetName(), "Event"))
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if ((pOwnerChannel) && (!bSuicide))
	{
		if ((pOwnerChannel->GetRuleType() == MCHANNEL_RULE_MASTERY) || 
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_ELITE) ||
#ifdef _CHANNELNEW
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_NEWS) ||
#endif
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_CHAMPION))
		{
			nVictimExp=0;
		}
	}
#else
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pStage->GetStageType() == MST_LADDER && pStage->GetStageType() == MST_PLAYERWARS))
	{
		nAttackerExp = nAttackerExp * 6;
		nVictimExp = 0;
	}
	MMatchChannel* pOwnerChannel = FindChannel(pStage->GetOwnerChannel());
	if (pOwnerChannel && strstr(pOwnerChannel->GetName(), "Event"))
	{
		*poutAttackerExp = 0;
		*poutVictimExp = 0;
		return;
	}
	if ((pOwnerChannel) && (!bSuicide))
	{
		if ((pOwnerChannel->GetRuleType() == MCHANNEL_RULE_MASTERY) ||
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_ELITE) ||
#ifdef _CHANNELNEW
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_NEWS) ||
#endif
			(pOwnerChannel->GetRuleType() == MCHANNEL_RULE_CHAMPION))
		{
			nVictimExp = 0;
		}
	}
#endif
	// ¡◊¿∫ªÁ∂˜¿Ã ø˚€µ¿⁄, ∞≥πﬂ¿⁄¿œ ∞ÊøÅE∞Ê«Ëƒ° µŒπÅE
	if ((pVictim->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pVictim->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER))
	{
		nAttackerExp = nAttackerExp * 2;
	}
	// ¡◊¿ŒªÁ∂˜¿Ã ø˚€µ¿⁄, ∞≥πﬂ¿⁄¿œ ∞ÊøÅE∞Êƒ°¥ŸøÅEæ¯¿Ω
	if ((!bSuicide) &&
		((pAttacker->GetAccountInfo()->m_nUGrade == MMUG_ADMIN) || 
		(pAttacker->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER)))
	{
		nVictimExp = 0;
	}

	// ¿⁄ªÅEœ ∞ÊøÅE∞Ê«Ëƒ° º’Ω«¿Ã µŒπÅE
	if (bSuicide) 
	{
		nVictimExp = (int)(MMatchFormula::GetSuicidePanaltyEXP(nVictimLevel) * fGameExpRatio);
		nAttackerExp = 0;
	}

	// ∆¿≈≥¿Œ∞ÊøÅE∞Ê«Ëƒ° ¡¶∑Œ
	if ((pStage->GetStageSetting()->IsTeamPlay()) && (pAttacker->GetTeam() == pVictim->GetTeam()))
	{
		nAttackerExp = 0;
	}


	// ∆¿¿ÅEœ ∞ÊøÅE∞Ê«Ëƒ° πË∫–
	if (pStage->IsApplyTeamBonus())
	{
		int nTeamBonus = 0;
		if (pStage->GetRule() != NULL)
		{
			int nNewAttackerExp = nAttackerExp;
			pStage->GetRule()->CalcTeamBonus(pAttacker, pVictim, nAttackerExp, &nNewAttackerExp, &nTeamBonus);
			nAttackerExp = nNewAttackerExp;
		}

		// ∆¿ ∞Ê«Ëƒ° ¿˚∏≥
		pStage->AddTeamBonus(nTeamBonus, MMatchTeam(pAttacker->GetTeam()));
	}

	// xp ∫∏≥ Ω∫ ¿˚øÅE≥›∏∂∫ÅEPCπÅE ∞Ê«Ëƒ° π›¡ÅE
	int nAttackerExpBonus = 0;
	if (nAttackerExp != 0)
	{
		//const float ta = float(atoi("15")) / 100.0f;
		//mlog( "test float : %f\n", ta * 100.0f );

		//MMatchItemBonusType nBonusType			= GetStageBonusType(pStage->GetStageSetting());
		//const double		dAttackerExp		= static_cast< double >( nAttackerExp );
		//const double		dXPBonusRatio		= static_cast< double >( MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType) );
		//const double		dAttackerExpBouns	= dAttackerExp * dXPBonusRatio;
		//const double		dSumAttackerExp		= dAttackerExp + dAttackerExpBouns;
		//
		//
		//nAttackerExpBonus = static_cast< int >( dAttackerExpBouns + 0.00001); 

		MMatchItemBonusType nBonusType = GetStageBonusType(pStage->GetStageSetting());
		const float fAttackerExpBonusRatio = MMatchFormula::CalcXPBonusRatio(pAttacker, nBonusType);
		 //∫Œµøº“ºˆ¡° ø¿¬˜∂ßπÆø° ∞ËªÅE° øµ«‚¿ª ¡÷¡ÅEæ ¥¬ πÅEßø°º≠ ∫∏¡§¿ª «ÿ¡ÿ¥Ÿ.
		// ∏∏æÅE¿Ã∫Œ∫–ø°º≠ ¥ŸΩ√ πÆ¡¶∞° πﬂª˝«—¥Ÿ∏ÅE∫∏¡§¿Ã æ∆¥— ∫£¿ÃΩ∫∫Œ≈Õ ºˆ¡§ ¿€æ˜¿ª «ÿ ¡‡æﬂ «—¥Ÿ.
		 nAttackerExpBonus = (int)(nAttackerExp * (fAttackerExpBonusRatio + 0.00001f));
	}

	*poutAttackerExp = nAttackerExp + nAttackerExpBonus;

	*poutVictimExp = nVictimExp;
}


const int MMatchServer::CalcBPonGameKill( MMatchStage* pStage, MMatchObject* pAttacker, const int nAttackerLevel, const int nVictimLevel )
{
	if( (0 == pStage) || (0 == pAttacker) ) 
		return -1;

	const int	nAddedBP		= static_cast< int >( MMatchFormula::GetGettingBounty(nAttackerLevel, nVictimLevel) );
	const float fBPBonusRatio	= MMatchFormula::CalcBPBounsRatio( pAttacker, GetStageBonusType(pStage->GetStageSetting()) );
	const int	nBPBonus		= static_cast< int >( nAddedBP * fBPBonusRatio );

	return nAddedBP + nBPBonus;
}




void MMatchServer::ProcessPlayerXPBP(MMatchStage* pStage, MMatchObject* pPlayer, int nAddedXP, int nAddedBP)
{
	if (pStage == NULL) return;
	if (!IsEnabledObject(pPlayer)) return;

	/*
		∞Ê«Ëƒ° ∞ËªÍ
		ƒ≥∏Ø≈Õø° ∞Ê«Ëƒ° ¿˚øÎ
		∑π∫ß ∞ËªÍ
		DBƒ≥ΩÃ æ˜µ•¿Ã∆Æ
		∑π∫ßæ˜,¥ŸøÓ ∏ﬁºº¡ˆ ¿¸º€
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// ƒ≥∏Ø≈Õ XP æ˜µ•¿Ã∆Æ
	pPlayer->GetCharInfo()->IncXP(nAddedXP);

	// ∑π∫ß ∞ËªÍ
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}

	// πŸøÓ∆º √ﬂ∞°«ÿ¡ÿ¥Ÿ
	pPlayer->GetCharInfo()->IncBP(nAddedBP);


	// DB ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	if (pPlayer->GetCharInfo()->GetDBCachingData()->IsRequestUpdate()) {
		UpdateCharDBCachingData(pPlayer);		///< XP, BP, KillCount, DeathCount ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	}

	// ∏∏æ‡ ∑π∫ß¿Ã πŸ≤Ó∏È µ˚∑Œ ∑π∫ßæ˜«—¥Ÿ.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
	{
		// ∑π∫ß¿Ã πŸ≤Ó∏È πŸ∑Œ ƒ≥Ω≥ æ˜µ•¿Ã∆Æ«—¥Ÿ
		UpdateCharDBCachingData(pPlayer);

		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
		if (!m_MatchDBMgr.UpdateCharLevel(pPlayer->GetCharInfo()->m_nCID,
			nNewPlayerLevel,
			pPlayer->GetCharInfo()->m_nBP,
			pPlayer->GetCharInfo()->m_nTotalKillCount,
			pPlayer->GetCharInfo()->m_nTotalDeathCount,
			pPlayer->GetCharInfo()->m_nTotalPlayTimeSec,
			true))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pPlayer->GetCharInfo()->m_szName);
		}
	}


	// ∑π∫ßæ˜, ∑π∫ß ¥ŸøÓ ∏ﬁºº¡ˆ ∫∏≥ª±‚
	if (nNewPlayerLevel > 0)
	{
		if (nNewPlayerLevel > nPlayerLevel)
		{
#ifdef _UPREWARDLEVEL
			MMatchLevelUpReward* pLvlUp = new MMatchLevelUpReward;
			if (pLvlUp == NULL)
			{
				mlog("Level Up Reward Failed\n");
				return;
			}
			pLvlUp->GiveReward(pPlayer, nNewPlayerLevel);
#endif

			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0, 0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
#ifdef _UPREWARDLEVEL
			pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewPlayerLevel)->GetBounty()));
			pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewPlayerLevel)->GetCash()));
			pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewPlayerLevel)->GetItemID()));
#endif
			RouteToBattle(uidStage, pCmd);
		}
		else if (nNewPlayerLevel < nPlayerLevel)
		{
			MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_DOWN, MUID(0, 0));
			pCmd->AddParameter(new MCommandParameterUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCommandParameterInt(nNewPlayerLevel));
			RouteToBattle(uidStage, pCmd);
		}
	}
}

// ∆¿ ∫∏≥ Ω∫ ¿˚øÅE
void MMatchServer::ApplyObjectTeamBonus(MMatchObject* pObject, int nAddedExp)
{
	if (!IsEnabledObject(pObject)) return;
	if (nAddedExp <= 0)
	{
		_ASSERT(0);
		return;
	}

	bool bIsLevelUp = false;

	// ∫∏≥ Ω∫ ¿˚øÎ
	if (nAddedExp != 0)
	{
		int nExpBonus = (int)(nAddedExp * MMatchFormula::CalcXPBonusRatio(pObject, MIBT_TEAM));
		nAddedExp += nExpBonus;
	}


	// ƒ≥∏Ø≈Õ XP æ˜µ•¿Ã∆Æ
	pObject->GetCharInfo()->IncXP(nAddedExp);

	// ∑π∫ß ∞ËªÍ
	int nNewLevel = -1;
	int nCurrLevel = pObject->GetCharInfo()->m_nLevel;

	if (nNewLevel > nCurrLevel) bIsLevelUp = true;

	if ((pObject->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pObject->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nCurrLevel)))
	{
		nNewLevel = MMatchFormula::GetLevelFromExp(pObject->GetCharInfo()->m_nXP);
		if (nNewLevel != nCurrLevel) pObject->GetCharInfo()->m_nLevel = nNewLevel;
	}

	// DB ƒ≥Ω≥ æ˜µ•¿Ã∆Æ
	if (pObject->GetCharInfo()->GetDBCachingData()->IsRequestUpdate())
	{
		UpdateCharDBCachingData(pObject);
	}

	// ∏∏æ‡ ∑π∫ß¿Ã πŸ≤Ó∏È πŸ∑Œ ∑π∫ßæ˜«—¥Ÿ.
	if ((nNewLevel >= 0) && (nNewLevel != nCurrLevel))
	{
		// ∑π∫ß¿Ã πŸ≤Ó∏È πŸ∑Œ ƒ≥Ω≥ æ˜µ•¿Ã∆Æ«—¥Ÿ
		UpdateCharDBCachingData(pObject);
		pObject->GetCharInfo()->m_nLevel = nNewLevel;
		nCurrLevel = nNewLevel;

		if (!m_MatchDBMgr.UpdateCharLevel(pObject->GetCharInfo()->m_nCID,
			nNewLevel,
			pObject->GetCharInfo()->m_nBP,
			pObject->GetCharInfo()->m_nTotalKillCount,
			pObject->GetCharInfo()->m_nTotalDeathCount,
			pObject->GetCharInfo()->m_nTotalPlayTimeSec,
			bIsLevelUp
		))
		{
			mlog("DB UpdateCharLevel Error : %s\n", pObject->GetCharInfo()->m_szName);
		}
	}


	MUID uidStage = pObject->GetStageUID();

	unsigned long int nExpArg;
	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pObject->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nCurrLevel);
	// ªÛ¿ß 2πŸ¿Ã∆Æ¥¬ ∞Ê«Ëƒ°, «œ¿ß 2πŸ¿Ã∆Æ¥¬ ∞Ê«Ëƒ°¿« ∆€ºæ∆Æ¿Ã¥Ÿ.
	nExpArg = MakeExpTransData(nAddedExp, nPercent);


	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_TEAMBONUS, MUID(0, 0));
	pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nExpArg));
	RouteToBattle(uidStage, pCmd);


	// ∑π∫ßæ˜ ∏ﬁºº¡ˆ ∫∏≥ª±‚
	if ((nNewLevel >= 0) && (nNewLevel > nCurrLevel))
	{
#ifdef _UPREWARDLEVEL
		MMatchLevelUpReward* pLvlUp = new MMatchLevelUpReward;
		if (pLvlUp == NULL)
		{
			mlog("Level Up Reward Failed\n");
			return;
		}
		pLvlUp->GiveReward(pObject, nNewLevel);
#endif

		MCommand* pCmd = CreateCommand(MC_MATCH_GAME_LEVEL_UP, MUID(0, 0));
		pCmd->AddParameter(new MCommandParameterUID(pObject->GetUID()));
		pCmd->AddParameter(new MCommandParameterInt(nNewLevel));
#ifdef _UPREWARDLEVEL
		pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewLevel)->GetBounty()));
		pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewLevel)->GetCash()));
		pCmd->AddParameter(new MCommandParameterInt(pLvlUp->GetLevelUpRewardByLevel(nNewLevel)->GetItemID()));
#endif
		RouteToBattle(uidStage, pCmd);
	}
}

// «√∑π¿Ã ¡ﬂ ƒ≥∏Ø≈Õ ¡§∫∏ æ˜µ•¿Ã∆Æ
void MMatchServer::ProcessCharPlayInfo(MMatchObject* pPlayer)
{
	if (!IsEnabledObject(pPlayer)) return;

	/*
	ø¯«“∂ß∏∂¥Ÿ ƒ≥∏Ø≈Õ ¡§∫∏∏¶ æ˜µ•¿Ã∆Æ ¿˚øÅE
	∞Ê«Ëƒ° ∞ËªÅE
	ƒ≥∏Ø≈Õø° ∞Ê«Ëƒ° ¿˚øÅE
	∑π∫ß ∞ËªÅE
	∑π∫ßæÅE¥ŸøÅE∏ﬁºº¡ÅE¿ÅE€
	πŸø˚‚º √ﬂ∞°«ÿ¡ÿ¥Ÿ
	¡¢º”Ω√∞£, ∞‘¿” ¡¯«‡Ω√∞£, «√∑π¿Ã Ω√∞£
	*/

	MUID uidStage = pPlayer->GetStageUID();
	int nPlayerLevel = pPlayer->GetCharInfo()->m_nLevel;

	// ∑π∫ß ∞ËªÅE
	int nNewPlayerLevel = -1;
	if ((pPlayer->GetCharInfo()->m_nLevel < MAX_LEVEL) &&
		(pPlayer->GetCharInfo()->m_nXP >= MMatchFormula::GetNeedExp(nPlayerLevel)))
	{
		nNewPlayerLevel = MMatchFormula::GetLevelFromExp(pPlayer->GetCharInfo()->m_nXP);
		if (nNewPlayerLevel != pPlayer->GetCharInfo()->m_nLevel) pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;
	}
	// ∏∏æÅE∑π∫ß¿Ã πŸ≤˚‘ÅEµ˚∑Œ ∑π∫ßæ˜«—¥Ÿ.
	if ((nNewPlayerLevel >= 0) && (nNewPlayerLevel != nPlayerLevel))
		pPlayer->GetCharInfo()->m_nLevel = nNewPlayerLevel;

	// ¡¢º”Ω√∞£, ∞‘¿” ¡¯«‡Ω√∞£, «√∑π¿Ã Ω√∞£
	unsigned long int nNowTime = MMatchServer::GetInstance()->GetGlobalClockCount();
	unsigned long int nBattlePlayingTimeSec = 0;
	if(pPlayer->GetCharInfo()->m_nBattleStartTime != 0)
	{
		nBattlePlayingTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nBattleStartTime, nNowTime) / 1000;	// πË∆≤¿ª ¡¯«‡«— Ω√∞£
		
		/*
		// æ∆π´ √≥∏Æµµ «œ¡ÅEæ ¥¬µ•, ø÷ ∑Œ±◊¥¬ ≥≤±‚¥¬∞«∞°øÅE ¿œ¥‹ ¡÷ºÆ √≥∏Æ«’¥œ¥Ÿ. - carrot318
		if(nBattlePlayingTimeSec > 60*60)
		{// ¿ÃªÛ¿˚¿∏∑Œ ∞™¿Ã ºº∆√µ≈∏ÅE∑Œ±◊∏¶ ≥≤±‰¥Ÿ.
			CTime theTime = CTime::GetCurrentTime();
			CString szTime = theTime.Format( "[%c] " );

			// ∞‘¿” ∏µÅE
			char buf[64]={0,};
			MMatchStage* pStage = FindStage(uidStage);

			if( pStage != NULL )
			{
				switch((int)pStage->GetStageSetting()->GetGameType())
				{
				case MMATCH_GAMETYPE_DEATHMATCH_SOLO:	{sprintf(buf, "DEATHMATCH_SOLO");	} break;		///< ∞≥¿Œ µ•æ≤∏≈ƒ°
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM:	{sprintf(buf, "DEATHMATCH_TEAM");	} break;		///< ∆¿ µ•æ≤∏≈ƒ°
				case MMATCH_GAMETYPE_GLADIATOR_SOLO:	{sprintf(buf, "GLADIATOR_SOLO");	} break;		///< ∞≥¿Œ ±€∑°µø°¿Ã≈Õ
				case MMATCH_GAMETYPE_GLADIATOR_TEAM:	{sprintf(buf, "GLADIATOR_TEAM");	} break;		///< ∆¿ ±€∑°µø°¿Ã≈Õ
				case MMATCH_GAMETYPE_ASSASSINATE:		{sprintf(buf, "ASSASSINATE");		} break;		///< ∫∏Ω∫¿ÅE
				case MMATCH_GAMETYPE_TRAINING:			{sprintf(buf, "TRAINING");			} break;		///< ø¨Ω¿

				case MMATCH_GAMETYPE_SURVIVAL:			{sprintf(buf, "SURVIVAL");			} break;		///< º≠πŸ¿ÃπÅE
				case MMATCH_GAMETYPE_QUEST:				{sprintf(buf, "QUEST");				} break;		///< ƒ˘Ω∫∆Æ
				case MMATCH_GAMETYPE_QUEST_CHALLENGE:	{sprintf(buf, "QUEST_CHALLENGE");	} break;		///< √ß∏∞¡ÅEƒ˘Ω∫∆Æ

				case MMATCH_GAMETYPE_BERSERKER:			{sprintf(buf, "BERSERKER");			} break;		
				case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:	{sprintf(buf, "DEATHMATCH_TEAM2");	} break;		
				case MMATCH_GAMETYPE_DUEL:				{sprintf(buf, "DUEL");				} break;	
				default:								{sprintf(buf, "don't know");		} break;
				}
				mlog("%s BattlePlayT Error GameMode:%s, CID:%d, Name:%s, ServerCurrT:%u, BattleStartT:%u, PlayT:%d, PlayerConnectT:%u \n", szTime, buf, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, nNowTime, pPlayer->GetCharInfo()->m_nBattleStartTime, nBattlePlayingTimeSec, pPlayer->GetCharInfo()->m_nConnTime);
			}
		}
		*/
		//pPlayer->GetCharInfo()->m_nBattleStartTime = 0;
	}
	unsigned long int nLoginTotalTimeSec = MGetTimeDistance(pPlayer->GetCharInfo()->m_nConnTime, nNowTime) / 1000;	// ∞‘¿”¿ª ¡¯«‡«— Ω√∞£

	// ¿ÃªÛ¿˚¿∏∑Œ ∞Ê«Ëƒ°∞° »πµÊ«ﬂ¿∏∏ÅE∑Œ±◊∏¶ ≥≤∞‹¡ÿ¥Ÿ.
	// æ∆π´ √≥∏Æµµ «œ¡ÅEæ ¥¬µ•, ø÷ ∑Œ±◊¥¬ ≥≤±‚¥¬∞«∞°øÅE ¿œ¥‹ ¡÷ºÆ √≥∏Æ«’¥œ¥Ÿ. - carrot318
	/*
	long int nBattleEXPGained = pPlayer->GetCharInfo()->m_nXP - pPlayer->GetCharInfo()->m_nBattleStartXP;
	if(nBattleEXPGained < -150000 || 150000 < nBattleEXPGained)
	{
		CTime theTime = CTime::GetCurrentTime();
		CString szTime = theTime.Format( "[%c] " );
		mlog("%s BattleXPGained Error CID:%d, Name:%s, StartXP:%d, EXPGained:%d \n", szTime, pPlayer->GetCharInfo()->m_nCID, pPlayer->GetCharInfo()->m_szName, pPlayer->GetCharInfo()->m_nBattleStartXP, nBattleEXPGained);
	}
	*/

#ifdef LOCALE_NHNUSA
	if (!m_MatchDBMgr.UpdateCharPlayInfo(pPlayer->GetAccountInfo()->m_nAID
										, pPlayer->GetCharInfo()->m_nCID
										, pPlayer->GetCharInfo()->m_nXP
										, pPlayer->GetCharInfo()->m_nLevel
										, nBattlePlayingTimeSec										// πË∆≤ Ω√∞£
										, nLoginTotalTimeSec										// √—∞‘¿”¿ª ¡¯«‡«— Ω√∞£
										, pPlayer->GetCharInfo()->m_nTotalKillCount
										, pPlayer->GetCharInfo()->m_nTotalDeathCount
										, pPlayer->GetCharInfo()->m_nBP
		                                , pPlayer->GetCharInfo()->m_nLC
										, false))
	{
		mlog("DB UpdateCharPlayInfo Error : %s\n", pPlayer->GetCharInfo()->m_szName);
	}
#endif
}

void MMatchServer::PostGameDeadOnGameKill(MUID& uidStage, MMatchObject* pAttacker, MMatchObject* pVictim,
									int nAddedAttackerExp, int nSubedVictimExp)
{
	unsigned long int nAttackerArg = 0;
	unsigned long int nVictimArg =0;

	int nRealAttackerLevel = pAttacker->GetCharInfo()->m_nLevel;
	int nRealVictimLevel = pVictim->GetCharInfo()->m_nLevel;

	unsigned long int nChrExp;
	int nPercent;

	nChrExp = pAttacker->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealAttackerLevel);
	nAttackerArg = MakeExpTransData(nAddedAttackerExp, nPercent);

	nChrExp = pVictim->GetCharInfo()->m_nXP;
	nPercent = MMatchFormula::GetLevelPercent(nChrExp, nRealVictimLevel);
	nVictimArg = MakeExpTransData(nSubedVictimExp, nPercent);

	MCommand* pCmd = CreateCommand(MC_MATCH_GAME_DEAD, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUID(pAttacker->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nAttackerArg));
	pCmd->AddParameter(new MCommandParameterUID(pVictim->GetUID()));
	pCmd->AddParameter(new MCommandParameterUInt(nVictimArg));
	RouteToBattle(uidStage, pCmd);	
}

void MMatchServer::StageList(const MUID& uidPlayer, int nStageStartIndex, bool bCacheUpdate)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (pChar == NULL) return;
	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	// ≈¨∑£º≠πˆ¿Œµ• ≈¨∑£√§≥Œ¿œ ∞ÊøÅE°¥¬ πÅE∏ÆΩ∫∆Æ¥ÅE≈ ¥ÅE‚¡ﬂ ≈¨∑£ ∏ÆΩ∫∆Æ∏¶ ∫∏≥Ω¥Ÿ.
	if ((MGetServerConfig()->GetServerMode() == MSM_CLAN) && (pChannel->GetChannelType() == MCHANNEL_TYPE_CLAN))
	{
		StandbyClanList(uidPlayer, nStageStartIndex, bCacheUpdate);
		return;
	}


	MCommand* pNew = new MCommand(m_CommandManager.GetCommandDescByID(MC_MATCH_STAGE_LIST), MUID(0,0), m_This);

	int nPrevStageCount = -1, nNextStageCount = -1;
	int nNextStageIndex = pChannel->GetMaxPlayers()-1;


	// 2008.09.16 
	int nRealStageStartIndex = nStageStartIndex;
	int nStageCount = 0;
	for(int i = 0; i < pChannel->GetMaxPlayers(); i++)
	{
		// πÊ¿Ã ∫Òøˆ¿’¿∏∏ÅE√≥∏Ææ»«—¥Ÿ
		if (pChannel->IsEmptyStage(i)) continue;
		// πÊ¿Ã ¿÷¿∏∏ÅE√≥∏Æ
		if(nStageCount < nStageStartIndex) // æ’ø° ≈«ø° √≥∏Æµ» πÊµÅE< «ˆ¿ÅE≈«ø°º≠ Ω√¿€«“ πÅEIndex
			nStageCount++;
		else
		{
			nRealStageStartIndex = i;
			break;
		}
	}

	int nRealStageCount = 0;
	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;

		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		nRealStageCount++;
		if (nRealStageCount >= TRANS_STAGELIST_NODE_COUNT) 
		{
			nNextStageIndex = i;
			break;
		}
	}

	if (!bCacheUpdate)
	{
		nPrevStageCount = pChannel->GetPrevStageCount(nStageStartIndex);
		nNextStageCount = pChannel->GetNextStageCount(nNextStageIndex);
	}

	pNew->AddParameter(new MCommandParameterChar((char)nPrevStageCount));
	pNew->AddParameter(new MCommandParameterChar((char)nNextStageCount));


	void* pStageArray = MMakeBlobArray(sizeof(MTD_StageListNode), nRealStageCount);
	int nArrayIndex=0;

	for (int i = /*nStageStartIndex*/nRealStageStartIndex; i < pChannel->GetMaxPlayers(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;
		
		if( pStage->GetState() < STAGE_STATE_STANDBY || pStage->GetState() > STAGE_STATE_COUNT )
		{
			LOG(LOG_FILE, "there is unavailable stages in %s channel. No:%d \n", pChannel->GetName(), i);
			continue;
		}


		if (nArrayIndex >= nRealStageCount) break;

		MTD_StageListNode* pNode = (MTD_StageListNode*)MGetBlobArrayElement(pStageArray, nArrayIndex++);
		pNode->uidStage = pStage->GetUID();
		strcpy(pNode->szStageName, pStage->GetName());
		pNode->nNo = (unsigned char)(pStage->GetIndex() + 1);	// ªÁøÅE⁄ø°∞‘ ∫∏ø©¡÷¥¬ ¿Œµ¶Ω∫¥¬ 1∫Œ≈Õ Ω√¿€«—¥Ÿ
		pNode->nPlayers = (char)pStage->GetPlayers();
		pNode->nMaxPlayers = pStage->GetStageSetting()->GetMaxPlayers();
		pNode->nState = pStage->GetState();
		pNode->nGameType = pStage->GetStageSetting()->GetGameType();
		
		// ∏±∑π¿Ã∏ÅE∑Œ∫ÅEπÊ∏ÆΩ∫∆Æ πË≥ ∏¶ ∏±∑π¿Ã∏ ¿∏∑Œ ¿Ø¡ˆ«ÿ¡ÿ¥Ÿ.
		if(pStage->IsRelayMap()) pNode->nMapIndex = MMATCH_MAP_RELAYMAP;
		else		 			 pNode->nMapIndex = pStage->GetStageSetting()->GetMapIndex();
		
		pNode->nSettingFlag = 0;
		// ≥≠¿‘
		if (pStage->GetStageSetting()->GetForcedEntry())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_FORCEDENTRY_ENABLED;
		}
		// ∫Òπ–πÅE
		if (pStage->IsPrivate())
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_PRIVATE;
		}
		// ∑π∫ß¡¶«—
		pNode->nLimitLevel = pStage->GetStageSetting()->GetLimitLevel();
		pNode->nMasterLevel = 0;

		if (pNode->nLimitLevel != 0)
		{
			pNode->nSettingFlag |= MSTAGENODE_FLAG_LIMITLEVEL;

			;
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());
			if (pMaster)
			{
				if (pMaster->GetCharInfo())
				{
					pNode->nMasterLevel = pMaster->GetCharInfo()->m_nLevel;
				}
			}
		}
	}

	pNew->AddParameter(new MCommandParameterBlob(pStageArray, MGetBlobArraySize(pStageArray)));
	MEraseBlobArray(pStageArray);
	RouteToListener(pChar, pNew);	
}


void MMatchServer::OnStageRequestStageList(const MUID& uidPlayer, const MUID& uidChannel, const int nStageCursor)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchChannel* pChannel = FindChannel(uidChannel);
	if (pChannel == NULL) return;

	pObj->SetStageCursor(nStageCursor);
	StageList(pObj->GetUID(), nStageCursor, false);
}


void MMatchServer::OnRequestQuickJoin(const MUID& uidPlayer, void* pQuickJoinBlob)
{
	MTD_QuickJoinParam* pNode = (MTD_QuickJoinParam*)MGetBlobArrayElement(pQuickJoinBlob, 0);
	ResponseQuickJoin(uidPlayer, pNode);
}

void MMatchServer::ResponseQuickJoin(const MUID& uidPlayer, MTD_QuickJoinParam* pQuickJoinParam)
{
	if (pQuickJoinParam == NULL) return;

	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
	if (pChannel == NULL) return;

	list<MUID>	recommended_stage_list;
	MUID uidRecommendedStage = MUID(0,0);
	int nQuickJoinResult = MOK;

	for (int i = 0; i < pChannel->GetMaxStages(); i++)
	{
		if (pChannel->IsEmptyStage(i)) continue;
		MMatchStage* pStage = pChannel->GetStage(i);
		if ((pStage == NULL) || (pStage->GetState() == STAGE_STATE_CLOSE)) continue;

		int ret = ValidateStageJoin(pObj->GetUID(), pStage->GetUID());
		if (ret == MOK)
		{
			if (pStage->IsPrivate()) continue;

			int nMapIndex = pStage->GetStageSetting()->GetMapIndex();
			int nGameType = pStage->GetStageSetting()->GetGameType();

			if (!CheckBitSet(pQuickJoinParam->nMapEnum, nMapIndex)) continue;
			if (!CheckBitSet(pQuickJoinParam->nModeEnum, nGameType)) continue;

			//if (((1 << nMapIndex) & (pQuickJoinParam->nMapEnum)) == 0) continue;
			//if (((1 << nGameType) & (pQuickJoinParam->nModeEnum)) == 0) continue;

			recommended_stage_list.push_back(pStage->GetUID());
		}
	}

	if (!recommended_stage_list.empty())
	{
		int nSize = (int)recommended_stage_list.size();
		int nIndex = rand() % nSize;

		int nCnt = 0;
		for (list<MUID>::iterator itor = recommended_stage_list.begin(); itor != recommended_stage_list.end(); ++itor)
		{
			if (nIndex == nCnt)
			{
				uidRecommendedStage = (*itor);
				break;
			}
			nCnt++;
		}
	}
	else
	{
		nQuickJoinResult = MERR_CANNOT_NO_STAGE;
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_STAGE_RESPONSE_QUICKJOIN, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterInt(nQuickJoinResult));
	pCmd->AddParameter(new MCommandParameterUID(uidRecommendedStage));
	RouteToListener(pObj, pCmd);	
}

static int __cdecl _int_sortfunc(const void* a, const void* b)
{
	return *((int*)a) - *((int*)b);
}

int MMatchServer::GetLadderTeamIDFromDB(const int nTeamTableIndex, const int* pnMemberCIDArray, const int nMemberCount)
{
	if ((nMemberCount <= 0) || (nTeamTableIndex != nMemberCount))
	{
		_ASSERT(0);
		return 0;
	}

	// cid ø¿∏ß¬˜º¯¿∏∑Œ º“∆√ - dbªÛø° º“∆√µ«æÅEµÈæ˚Ã°¿÷¥Ÿ. 
	int* pnSortedCIDs = new int[nMemberCount];
	for (int i = 0; i < nMemberCount; i++)
	{
		pnSortedCIDs[i] = pnMemberCIDArray[i];
	}
	qsort(pnSortedCIDs, nMemberCount, sizeof(int), _int_sortfunc);

	int nTID = 0;
	if (pnSortedCIDs[0] != 0)
	{
		if (!m_MatchDBMgr.GetLadderTeamID(nTeamTableIndex, pnSortedCIDs, nMemberCount, &nTID))
		{
			nTID = 0;
		}
	}

	delete[] pnSortedCIDs;
	return nTID;
}

void MMatchServer::SaveLadderTeamPointToDB(const int nTeamTableIndex, const int nWinnerTeamID, const int nLoserTeamID, const bool bIsDrawGame)
{
	// ∆˜¿Œ∆Æ ∞ËªÅE- æ◊º«∏Æ±◊ ¿ÅEÅE
	int nWinnerPoint = 0, nLoserPoint = 0, nDrawPoint = 0;

	nLoserPoint = -1;
	switch (nTeamTableIndex)
	{
		case 2:	// 2¥ÅE
		{
			nWinnerPoint = 4;
			nDrawPoint = 1;
		} break;
		case 3:
		{
			nWinnerPoint = 6;
			nDrawPoint = 1;
		} break;
		case 4:
		{
			nWinnerPoint = 10;
			nDrawPoint = 2;
		} break;
	}

	if (!m_MatchDBMgr.LadderTeamWinTheGame(nTeamTableIndex, nWinnerTeamID, nLoserTeamID, bIsDrawGame, nWinnerPoint, nLoserPoint, nDrawPoint))
	{
		mlog("DB Query(SaveLadderTeamPointToDB) Failed\n");
	}
}


void MMatchServer::OnVoteCallVote(const MUID& uidPlayer, const char* pszDiscuss, const char* pszArg)
{
	char szMsg[256];
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	// Custom: Only instant kick outside of event channel.
	MMatchChannel* pChannel = FindChannel(pObj->GetChannelUID());
#ifdef _NEWGRADE
	if ((pObj->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM || pObj->GetAccountInfo()->m_nUGrade == MMUG_JORK || IsAdminGrade(pObj)) && (pChannel && pChannel->IsUseEvent())) {
#else
	if ((pObj->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM || IsAdminGrade(pObj)) && (pChannel && pChannel->IsUseEvent())) {
#endif
		MMatchStage* pStage = FindStage(pObj->GetStageUID());
		if (pStage) {
			pStage->KickBanPlayer(pszArg, false);
		}
		return;
	}
#ifdef _NEWGRADE
	else if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM && pObj->GetAccountInfo()->m_nUGrade != MMUG_JORK && pChannel->IsUseEvent())
#else
	else if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM && pChannel->IsUseEvent())
#endif
	{
		sprintf(szMsg,"%s", "Cannot vote in event channel");
		Announce(uidPlayer, szMsg);
		return;
	}
#ifdef _NEWGRADE
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM && pObj->GetAccountInfo()->m_nUGrade != MMUG_JORK)
#else
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM)
#endif
	{
		MMatchObject* pTar = GetPlayerByName(pszArg);
		if (!pTar) return;

		if (pTar->CheckPlayerFlags(MTD_PlayerFlags_AdminHide))
		{
			sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
			Announce(uidPlayer, szMsg);
			return;
		}
#ifdef _NEWGRADE
		if (IsAdminGrade(pTar) || pTar->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM || pTar->GetAccountInfo()->m_nUGrade == MMUG_JORK)
#else
		if (IsAdminGrade(pTar) || pTar->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM)
#endif
		{
			sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
			Announce(uidPlayer, szMsg);
			return;
		}
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (pObj->WasCallVote())
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);
		return;
	}

	pObj->SetVoteState( true );

	if (pStage->GetStageType() == MST_LADDER || pStage->GetStageType() == MST_PLAYERWARS)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE_LADERGAME);
		Announce(uidPlayer, szMsg);
		return;
	}

	if (pStage->GetRule() && pStage->GetRule()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_CANNOT_VOTE);
		Announce(uidPlayer, szMsg);
		return;
	}

#ifdef _VOTESETTING
	if( !pStage->GetStageSetting()->bVoteEnabled ) {
		VoteAbort( uidPlayer );
		return;
	}

	if( pStage->WasCallVote() ) {
		VoteAbort( uidPlayer );
		return;
	}
	else {
		pStage->SetVoteState( true );
	}
#endif

	if (pStage->GetVoteMgr()->GetDiscuss())
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_ALREADY_START);
		Announce(uidPlayer, szMsg);
		return;
	}

	MVoteDiscuss* pDiscuss = MVoteDiscussBuilder::Build(uidPlayer, pStage->GetUID(), pszDiscuss, pszArg);
	if (pDiscuss == NULL) return;

	if (pStage->GetVoteMgr()->CallVote(pDiscuss))
	{
		pDiscuss->Vote(uidPlayer, MVOTE_YES);

		MCommand* pCmd = CreateCommand(MC_MATCH_NOTIFY_CALLVOTE, MUID(0,0));
		pCmd->AddParameter(new MCmdParamStr(pszDiscuss));
		pCmd->AddParameter(new MCmdParamStr(pszArg));
		RouteToStage(pStage->GetUID(), pCmd);
		return;
	}
	else
	{
		sprintf(szMsg, "%s%d", MTOK_ANNOUNCE_PARAMSTR, MERR_VOTE_FAILED);
		Announce(uidPlayer, szMsg);
		return;
	}
}

void MMatchServer::OnVoteYes(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_YES);
}

void MMatchServer::OnVoteNo(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	MVoteDiscuss* pDiscuss = pStage->GetVoteMgr()->GetDiscuss();
    if (pDiscuss == NULL) return;

	pDiscuss->Vote(uidPlayer, MVOTE_NO);
}

void MMatchServer::VoteAbort( const MUID& uidPlayer )
{
#ifndef MERR_CANNOT_VOTE
#define MERR_CANNOT_VOTE 120000
#endif

	MMatchObject* pObj = GetObject( uidPlayer );
	if (!IsEnabledObject(pObj)) return;

	MCommand* pCmd = CreateCommand( MC_MATCH_VOTE_RESPONSE, MUID(0, 0) );
	if(0 == pCmd) return;

	pCmd->AddParameter( new MCommandParameterInt(MERR_CANNOT_VOTE) );
	RouteToListener( pObj, pCmd );
}

void MMatchServer::OnEventChangeMaster(const MUID& uidAdmin)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// Staff check
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;
	if (pStage->GetMasterUID() == uidAdmin) return;

	// Custom: Event Team
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM || IsAdminGrade(pObj))
	{
		pStage->SetMasterUID(uidAdmin);
		StageMaster(pStage->GetUID());
	}
}

void MMatchServer::OnEventChangePassword(const MUID& uidAdmin, const char* pszPassword)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// Custom: Event Team + StageMaster
	if (pObj->GetUID() == pStage->GetMasterUID() || IsAdminGrade(pObj) || pObj->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM)
	{
		pStage->SetPassword(pszPassword);
		pStage->SetPrivate(true);
	}
}

void MMatchServer::OnEventRequestJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ∞¸∏Æ¿⁄ ±««—¿ª ∞°¡¯ ªÁ∂˜¿Ã æ∆¥œ∏È π´Ω√
	if (!IsAdminGrade(pObj))
	{
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;
	if (IsAdminGrade(pTargetObj)) return;		// æÓµÂπŒ ¥ÎªÛ¿∏∑Œ ¬Ø∫“∞°
	if (MMUG_STAR == pTargetObj->GetAccountInfo()->m_nUGrade) return;	// ¿ÃπÃ ¬Ø

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_STAR;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, true)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(true));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}
void MMatchServer::OnAdminSendItem(const MUID& uidAdmin, const char* szPlayer, const int nItemId, const int nDays)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if (pObj == NULL || !IsAdminGrade(pObj)) return;

	if (strlen(szPlayer) < 2) {
		Announce(pObj, "Player name must be longer.");
		return;
	}

	if (nDays < 0) {
		Announce(pObj, "RentDays must be 0 or more.");
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(szPlayer);

	int nCID = 0;
	if (m_MatchDBMgr.GetCharCID(szPlayer, &nCID) == false)
	{
		Announce(pObj, "Could not find player.");
		return;
	}

	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
	if (pItemDesc == NULL)
	{
		Announce(pObj, "Could not find item.");
		return;
	}

	if (!pItemDesc->IsSpendableItem())
	{
		unsigned long int nNewCIID = 0;
		if (!m_MatchDBMgr.InsertDistributeItem(nCID, nItemId, nDays != 0, nDays * 24, &nNewCIID)) {
			Announce(pObj, "Could not send item.");
			return;
		}

		if (pTargetObj != NULL) {
			MUID uidNew = MMatchItemMap::UseUID();
			pTargetObj->GetCharInfo()->m_ItemList.CreateItem(uidNew, nNewCIID, nItemId, nDays != 0, nDays * 24 * 60, nDays * 24);

			UpdateCharItemDBCachingData(pTargetObj);

			ResponseCharacterItemList(pTargetObj->GetUID());

			Announce(pTargetObj, "You have received an item!");
		}
	}
	else {
		Announce(pObj, "Cannot send items of this type.");
		return;
	}

	Announce(pObj, "Sent item to player.");
}
void MMatchServer::OnEventRemoveJjang(const MUID& uidAdmin, const char* pszTargetName)
{
	MMatchObject* pObj = GetObject(uidAdmin);
	if( 0 == pObj )
		return;

	if (!IsEnabledObject(pObj)) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	// ∞¸∏Æ¿⁄ ±««—¿ª ∞°¡¯ ªÁ∂˜¿Ã æ∆¥œ∏È ø¨∞·¿ª ≤˜¥¬¥Ÿ.
	if (!IsAdminGrade(pObj))
	{
		return;
	}

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (pTargetObj == NULL) return;			// æÓµÂπŒ ¥ÎªÛ¿∏∑Œ ¬Ø∫“∞°

	// Custom: Check if target is actually a jjang-ed user
	if( pTargetObj->GetAccountInfo()->m_nUGrade != MMUG_STAR ) return;

	pTargetObj->GetAccountInfo()->m_nUGrade = MMUG_FREE;

	if (m_MatchDBMgr.EventJjangUpdate(pTargetObj->GetAccountInfo()->m_nAID, false)) {
		MMatchObjectCacheBuilder CacheBuilder;
		CacheBuilder.AddObject(pTargetObj);
		MCommand* pCmdCacheUpdate = CacheBuilder.GetResultCmd(MATCHCACHEMODE_REPLACE, this);
		RouteToStage(pStage->GetUID(), pCmdCacheUpdate);

		MCommand* pCmdUIUpdate = CreateCommand(MC_EVENT_UPDATE_JJANG, MUID(0,0));
		pCmdUIUpdate->AddParameter(new MCommandParameterUID(pTargetObj->GetUID()));
		pCmdUIUpdate->AddParameter(new MCommandParameterBool(false));
		RouteToStage(pStage->GetUID(), pCmdUIUpdate);
	}
}


void MMatchServer::OnStageGo(const MUID& uidPlayer, unsigned int nRoomNo)
{
	MMatchObject* pChar = GetObject(uidPlayer);
	if (!IsEnabledObject(pChar)) return;
	if (pChar->GetPlace() != MMP_LOBBY) return;

	MMatchChannel* pChannel = FindChannel(pChar->GetChannelUID());
	if (pChannel == NULL) return;

	MMatchStage* pStage = pChannel->GetStage(nRoomNo-1);
	if (pStage) {
		MCommand* pNew = CreateCommand(MC_MATCH_REQUEST_STAGE_JOIN, GetUID());
		pNew->SetSenderUID(uidPlayer);	// «√∑π¿Ãæ˚Ã° ∫∏≥Ω ∏ﬁΩ√¡ˆ¿Œ ∞Õ√≥∑≥ ¿ß¿ÅE
		pNew->AddParameter(new MCommandParameterUID(uidPlayer));
		pNew->AddParameter(new MCommandParameterUID(pStage->GetUID()));
		Post(pNew);
	}
}

void MMatchServer::OnDuelQueueInfo(const MUID& uidStage, const MTD_DuelQueueInfo& QueueInfo)
{
	MCommand* pCmd = CreateCommand(MC_MATCH_DUEL_QUEUEINFO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamBlob(&QueueInfo, sizeof(MTD_DuelQueueInfo)));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::OnQuestSendPing(const MUID& uidStage, unsigned long int t)
{
	MCommand* pCmd = CreateCommand(MC_QUEST_PING, MUID(0,0));
	pCmd->AddParameter(new MCommandParameterUInt(t));
	RouteToBattle(uidStage, pCmd);
}

void MMatchServer::SaveGameLog(const MUID& uidStage)
{
	MMatchStage* pStage = FindStage(uidStage);
	if (pStage == NULL) return;

	int nMapID = pStage->GetStageSetting()->GetMapIndex();
	int nGameType = (int)pStage->GetStageSetting()->GetGameType();

	if ((MGetMapDescMgr()->MIsCorrectMap(nMapID)) && (MGetGameTypeMgr()->IsCorrectGameType(nGameType)))
	{
		if (pStage->GetStageType() != MST_LADDER && pStage->GetStageType() != MST_PLAYERWARS)
		{
			MMatchObject* pMaster = GetObject(pStage->GetMasterUID());

			MAsyncDBJob_InsertGameLog* pJob = new MAsyncDBJob_InsertGameLog(uidStage);
			pJob->Input(pMaster == NULL ? 0 : pMaster->GetCharInfo()->m_nCID,
				MGetMapDescMgr()->GetMapName(nMapID),
				MGetGameTypeMgr()->GetInfo(MMATCH_GAMETYPE(nGameType))->szGameTypeStr);
			PostAsyncJob(pJob);
		}
	}

}

void MMatchServer::SaveGamePlayerLog(MMatchObject* pObj, unsigned int nStageID)
{
	if (!IsEnabledObject(pObj)) return;
	if (nStageID == 0) return;

	MAsyncDBJob_InsertGamePlayerLog* pJob = new MAsyncDBJob_InsertGamePlayerLog;
	pJob->Input(
		nStageID, 
		pObj->GetCharInfo()->m_nCID,
		(GetGlobalClockCount() - pObj->GetCharInfo()->m_nBattleStartTime) / 1000,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nKillCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nDeathCount,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nXP,
		pObj->GetCharInfo()->GetCharGamePlayInfo()->nBP
	);
	PostAsyncJob(pJob);
}

void MMatchServer::OnAdminFreeze(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;

	MMatchStage* m_pStage = FindStage(pObj->GetStageUID());
	if (stricmp(pszTargetName, "all") == 0)
	{
		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;
			if (!IsEnabledObject(pPlayer)) continue;
			if (pObj->GetStageUID() != pPlayer->GetStageUID()) continue;
			if (pObj->GetUID() == pPlayer->GetUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pPlayer->GetPlace() != MMP_BATTLE) continue;

			MCommand* pCmd = CreateCommand(MC_ADMIN_FREEZE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUID(pPlayer->GetUID()));
			pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
			RouteToListener(pPlayer, pCmd);
		}

		char szBuf[128];
		sprintf(szBuf, "Froze '%s'.", pszTargetName);
		Announce(pObj, szBuf);
	}
	else
	{
		MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
		if(!IsEnabledObject(pTargetObj)) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;
		if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;

		MCommand* pCmd = CreateCommand(MC_ADMIN_FREEZE, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
		RouteToListener(pTargetObj, pCmd);

		char szBuf[128];
		sprintf(szBuf, "Froze '%s'.", pszTargetName);
		Announce(pObj, szBuf);
	}
}

void MMatchServer::OnAdminSummon(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;

	MMatchStage* m_pStage = FindStage(pObj->GetStageUID());

	if (strcmp(pszTargetName, "all") == 0)
	{
		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;
			if (!IsEnabledObject(pPlayer)) continue;
			if (pObj->GetStageUID() != pPlayer->GetStageUID()) continue;
			if (pObj->GetUID() == pPlayer->GetUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pPlayer->GetPlace() != MMP_BATTLE) continue;

			MCommand* pCmd = CreateCommand(MC_ADMIN_SUMMON, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
			pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
			RouteToListener(pPlayer, pCmd);
		}
		char szBuf[128];
		sprintf( szBuf, "Summoned '%s'.", pszTargetName );
		Announce( pObj, szBuf );
	}
	else
	{	
		MMatchObject* pTargetObj = GetPlayerByName( pszTargetName );
		if (!IsEnabledObject(pTargetObj)) return;
		if (pTargetObj->GetStageUID().IsInvalid()) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;

		MCommand* pCmd = CreateCommand(MC_ADMIN_SUMMON, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
		RouteToListener(pTargetObj, pCmd);

		char szBuf[128];
		sprintf( szBuf, "Summoned '%s'.", pszTargetName );
		Announce( pObj, szBuf );
	}
}

void MMatchServer::OnAdminGoTo(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );
	if (!IsEnabledObject(pObj)) return;
	if( pObj->GetStageUID().IsInvalid() ) return;

	MMatchObject* pTargetObj = GetPlayerByName( pszTargetName );
	if (!IsEnabledObject(pTargetObj)) return;
	if (pTargetObj->GetStageUID().IsInvalid()) return;
	if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;

	// Staff check
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;
	if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;

	MCommand* pCmd = CreateCommand(MC_ADMIN_GOTO, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
	pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
	RouteToListener(pObj, pCmd);

	char szBuf[128];
	sprintf( szBuf, "Teleported to '%s'.", pszTargetName );
	Announce( pObj, szBuf );
}

void MMatchServer::OnAdminSlap(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );
	MMatchStage* m_pStage = FindStage(pObj->GetStageUID());
	if (!IsEnabledObject(pObj)) return;
	if( pObj->GetStageUID().IsInvalid() ) return;

	// Staff check
	if (!IsAdminGrade(pObj) && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;

	if (strcmp( pszTargetName, "all") == 0)
	{
		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pPlayer = (MMatchObject*)(*itor).second;

			if (!IsEnabledObject(pPlayer)) continue;
			if (pObj->GetStageUID() != pPlayer->GetStageUID()) continue;
			if (pObj->GetUID() == pPlayer->GetUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pPlayer->GetPlace() != MMP_BATTLE) continue;
			if (!pObj->CheckAlive()) continue;

			MCommand* pCmd = CreateCommand(MC_ADMIN_SLAP, MUID(0,0));
			pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
			pCmd->AddParameter(new MCmdParamStr(pPlayer->GetName()));
			RouteToListener(pPlayer, pCmd);
		}

		char szBuf[128];
		sprintf( szBuf, "Slapped everyone." );
		Announce( pObj, szBuf );
	}
	else
	{
		MMatchObject* pTargetObj = GetPlayerByName( pszTargetName );

		if (!IsEnabledObject(pTargetObj)) return;
		if (pTargetObj->GetStageUID().IsInvalid()) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;
		if (!pObj->CheckAlive()) return;

		MCommand* pCmd = CreateCommand(MC_ADMIN_SLAP, MUID(0,0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamStr(pTargetObj->GetName()));
		RouteToListener(pTargetObj, pCmd);

		char szBuf[128];
		sprintf( szBuf, "Slapped '%s'.", pszTargetName );
		Announce( pObj, szBuf );
	}
}

void MMatchServer::OnAdminSpawn(const MUID& uidComm, char* pszTargetName)
{
	MMatchObject* pObj = GetPlayerByCommUID( uidComm );

	if (!IsEnabledObject(pObj)) return;
	if( pObj->GetStageUID().IsInvalid() ) return;
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;

	if (strcmp(pszTargetName, "all") == 0)
	{
		MMatchStage* m_pStage = FindStage(pObj->GetStageUID());

		for (MUIDRefCache::iterator itor = m_pStage->GetObjBegin(); itor != m_pStage->GetObjEnd(); ++itor)
		{
			MMatchObject* pTargetObj = (MMatchObject*)(*itor).second;

			if (!IsEnabledObject(pTargetObj)) continue;
			if (pTargetObj->GetStageUID().IsInvalid()) continue;
			if (pObj->GetStageUID() != pTargetObj->GetStageUID()) continue;
			if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) continue;
			if (IsAdminGrade(pTargetObj) || pTargetObj->GetAccountInfo()->m_nUGrade == MMUG_EVENTTEAM) continue;

			pTargetObj->SetLastSpawnTime(timeGetTime());
			pTargetObj->SetAlive(true);
			pTargetObj->ResetCustomItemUseCount();

			// Custom: New admin_spawn
			MCommand* pCmd = CreateCommand(MC_ADMIN_SPAWN_RESPONSE, MUID(0, 0));
			pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
			pCmd->AddParameter(new MCmdParamUID(pTargetObj->GetUID()));
			RouteToBattle(pObj->GetStageUID(), pCmd);
		}

		char szBuf[128];
		sprintf(szBuf, "Spawned everyone.");
		Announce(pObj, szBuf);
	}
	else
	{
		MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);

		if (!IsEnabledObject(pTargetObj)) return;
		if (pTargetObj->GetStageUID().IsInvalid()) return;
		if (pObj->GetStageUID() != pTargetObj->GetStageUID()) return;
		if (pObj->GetPlace() != MMP_BATTLE || pTargetObj->GetPlace() != MMP_BATTLE) return;

		pTargetObj->SetLastSpawnTime(timeGetTime());
		pTargetObj->SetAlive(true);
		pTargetObj->ResetCustomItemUseCount();

		// Custom: New admin_spawn
		MCommand* pCmd = CreateCommand(MC_ADMIN_SPAWN_RESPONSE, MUID(0, 0));
		pCmd->AddParameter(new MCmdParamUID(pObj->GetUID()));
		pCmd->AddParameter(new MCmdParamUID(pTargetObj->GetUID()));
		RouteToBattle(pObj->GetStageUID(), pCmd);

		char szBuf[128];
		sprintf(szBuf, "Spawned '%s'.", pszTargetName);
		Announce(pObj, szBuf);
	}
}

void MMatchServer::OnAdminPKick(const MUID& uidComm, char* pszTargetName, bool bBan)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;

	MMatchObject* pTargetObj = GetPlayerByName(pszTargetName);
	if (!IsEnabledObject(pTargetObj)) return;
	if (bBan && pTargetObj->GetStageUID().IsInvalid()) return;
	if (bBan && pObj->GetStageUID() != pTargetObj->GetStageUID()) return;

	// Event Team support
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());

	MMatchServer::GetInstance()->Announce( pObj, "Action completed." );
}

void MMatchServer::OnAdminSilence(const MUID& uidComm, int nDurationMin)
{
	MMatchObject* pObj = GetPlayerByCommUID(uidComm);
	if (!IsEnabledObject(pObj)) return;
	if (pObj->GetStageUID().IsInvalid()) return;

	// Event Team support
	if (IsAdminGrade(pObj) == false && pObj->GetAccountInfo()->m_nUGrade != MMUG_EVENTTEAM) return;
	if (pObj->GetAccountInfo()->m_nUGrade == MMUG_DEVELOPER) return;

	if (nDurationMin < 0 || nDurationMin > 5)
	{
		Announce(pObj, "Invalid operation. Permitted silence duration: 1-5 minutes. (0 min to forcefully disable if enabled)");
		return;
	}

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (!pStage) return;

	char szMsg[256];
	memset(szMsg, 0, sizeof(szMsg));

	bool bSilence = true;
	DWORD dwTime = timeGetTime();

	if (nDurationMin != 0 && dwTime < pStage->GetSilenceEndTime())
	{
		sprintf_s(szMsg, "Room is already silenced. %d seconds remaining.", (pStage->GetSilenceEndTime() - dwTime) / 1000);
		Announce(pObj, szMsg);
		return;
	}

	if (nDurationMin == 0)
	{
		if (dwTime < pStage->GetSilenceEndTime())
		{
			bSilence = false;
			pStage->SetSilenceEndTime(0);
		}
		else
		{
			sprintf_s(szMsg, "Room is not silenced.");
			Announce(pObj, szMsg);
			return;
		}
	}
	else
	{
		pStage->SetSilenceEndTime(dwTime + (nDurationMin * 60 * 1000));
	}

	if (bSilence) {
		sprintf_s(szMsg, "Room has been silenced by '%s' for %d minutes.", pObj->GetName(), nDurationMin);
	} else {
		sprintf_s(szMsg, "Room has been unsilenced by '%s'.", pObj->GetName());
	}

	MCommand* pCmd = CreateCommand(MC_MATCH_ANNOUNCE, MUID(0,0));
	pCmd->AddParameter(new MCmdParamUInt(0));
	pCmd->AddParameter(new MCmdParamStr(szMsg));
	RouteToStage(pStage->GetUID(), pCmd);
}


void MMatchServer::OnDropGunGameRequestItem(const MUID& uidPlayer)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (pObj == NULL) return;

	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if (pStage->GetStageSetting())
	{
		if (pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_DROPMAGIC)
		{
			MMatchRuleDropGunGame* pRule = (MMatchRuleDropGunGame*)pStage->GetRule();
			if (pRule)
			{
				pRule->OnObtainWorldItem(pObj);
			}
		}
	}
}
void MMatchServer::OnRequestFlagCap(const MUID& uidPlayer, const int nItemID)
{
	MMatchObject* pObj = GetObject(uidPlayer);
	if (!IsEnabledObject(pObj)) return;
	MMatchStage* pStage = FindStage(pObj->GetStageUID());
	if (pStage == NULL) return;

	if(pStage->GetStageSetting())
	{
		if(pStage->GetStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF)
		{
			MMatchRuleTeamCTF* pRule = (MMatchRuleTeamCTF*)pStage->GetRule();
			if(pRule)
			{
				pRule->OnObtainWorldItem( pObj, nItemID, NULL );
			}
		}
	}
}

//blitzkrieg
void MMatchServer::OnStartBlitz(MMatchStage* pStage)
{
	MMatchStageSetting* pSetting = pStage->GetStageSetting();
	pSetting->SetRoundMax(1);//1 round only
	MCommand* pCmd = CreateCmdResponseStageSetting(pStage->GetUID());
	RouteToStage(pStage->GetUID(), pCmd);

}

//challengequest
void MMatchServer::OnStartCQ(MMatchStage* m_pStage, int nRounds)
{
	MMatchStageSetting* pSetting = m_pStage->GetStageSetting();
	pSetting->SetRoundMax(nRounds);
	MCommand* pCmd = CreateCmdResponseStageSetting(m_pStage->GetUID());
	RouteToStage(m_pStage->GetUID(), pCmd);
}

void MMatchServer::OnStartCQForSingle(MUID uidListener, MMatchStage* m_pStage, int nRounds)
{
	MMatchObject* pObj = GetObject(uidListener);
	if (!IsEnabledObject(pObj)) return;

	MMatchStageSetting* pSetting = m_pStage->GetStageSetting();
	pSetting->SetRoundMax(nRounds);
	MCommand* pCmd = CreateCmdResponseStageSetting(m_pStage->GetUID());
	RouteToListener(pObj, pCmd);
}