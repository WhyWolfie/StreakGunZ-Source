#include "stdafx.h"
#include "ZMap.h"
#include "ZApplication.h"
#include "MComboBox.h"
#include "ZChannelRule.h"

#include "ZGameClient.h"
#include "MNewQuestScenario.h"


void ZGetCurrMapPath(char* outPath)
{
// 나중에 랜덤맵 구현할때까지 그냥 맵 하나만 사용
#ifdef _QUEST


	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		strcpy(outPath, PATH_QUEST_MAPS);
		return;
	}
#endif

	if (ZApplication::GetInstance()->GetLaunchMode() == ZApplication::ZLAUNCH_MODE_STANDALONE_QUEST)
	{
		strcpy(outPath, PATH_QUEST_MAPS);
	}
	else
	{
		strcpy(outPath, PATH_GAME_MAPS);
	}
}

bool InitMaps(MWidget *pWidget)
{
	if(!pWidget) return false;

	MComboBox* pCombo=(MComboBox*)pWidget;
	pCombo->RemoveAll();

	if(!ZGetGameClient()) return false;

	// 퀘스트 모드는 scenario.xml에 기술된 맵들만 읽어들인다.
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		LIST_SCENARIONAME* pSenarioDesc = ZApplication::GetStageInterface()->GetSenarioDesc();

		// 일반 시나리오를 읽어온다. 1000번부터 시작
		for ( int i = 1000;  ; i++)
		{
			LIST_SCENARIONAME::iterator itr = pSenarioDesc->find( i);


			// 끝이면 리턴
			if ( itr == pSenarioDesc->end())
				return true;


			// 있으면 콤보 박스에 이미 존재하는지 검사.
			MSenarioList Scenario = (*itr).second;
			bool bFind = false;

			for ( int k = 0;  k < pCombo->GetCount();  k++)
			{
				if ( !strcmp( Scenario.m_szMapSet, pCombo->GetString( k)))
				{
					bFind = true;
					break;
				}
			}

			// 콤보 박스에 없으면 콤보 박스에 추가.
			if ( !bFind)
				pCombo->Add( Scenario.m_szMapSet);

		}

		return true;
	}

	// 챌린지퀘스트는 따로 맵리스트를 어딘가에서 읽어와야 한다.
	//todok 지금은 임시 코드
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		MNewQuestScenarioManager* pScenarioMgr = ZApplication::GetStageInterface()->GetChallengeQuestScenario();
		_ASSERT(pScenarioMgr);
		if (pScenarioMgr)
		{
			vector<string> vecScenario;
			pScenarioMgr->GetScenarioNameList( vecScenario);

			int numScenario = (int)vecScenario.size();
			for (int i=0; i<numScenario; ++i)
				pCombo->Add(vecScenario[i].c_str());
		}
		return true;
	}

	MChannelRule* pRule = ZGetChannelRuleMgr()->GetCurrentRule();
	if (pRule == NULL) {
		mlog("Init maps : no Current ChannelRule \n");
		return false;
	}

	bool bRelayMapInsert = false;
	int nMapCount = MGetMapDescMgr()->GetMapCount();
	for( int i=0 ; i< nMapCount ; i++)
	{

//#ifdef _DEBUG	// 디버그시 모든맵 보여줌
//		pCombo->Add(MGetMapDescMgr()->GetMapName(i));
//		continue;
//#endif

		bool bDuelMode = false;
		bool bCTFMode = false;

		bool bSWRMode = false;

		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)
			bDuelMode = true;
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_CTF)
			bCTFMode = true;
#ifdef _CUSTOMRULEMAPS
		if (ZGetGameClient() && (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DROPMAGIC))
			bSWRMode = true;
#endif
		// 릴레이맵을 젤위에 넣어준다.
		if(!bRelayMapInsert)
		{
			for(int j=0 ; j< MGetMapDescMgr()->GetMapCount() ; j++)
			{
				if(strcmp(MMATCH_MAPNAME_RELAYMAP, MGetMapDescMgr()->GetMapName(j)) == 0)
				{
					pCombo->Add(MGetMapDescMgr()->GetMapName(j));
					bRelayMapInsert = true;
					//--nMapCount;	// 릴레이맵을 추가했으면 MapCount를 줄여준다.
					continue;
				}
			}
		}

		if(bCTFMode) 
		{
			if ( pRule->CheckCTFMap( MGetMapDescMgr()->GetMapName(i)))
			{
				if(strcmp(MMATCH_MAPNAME_RELAYMAP, MGetMapDescMgr()->GetMapName(i)) != 0)
					pCombo->Add(MGetMapDescMgr()->GetMapName(i));
			}
		}
#ifdef _CUSTOMRULEMAPS
		if (bSWRMode)
		{
			if (pRule->CheckSWRMap(MGetMapDescMgr()->GetMapName(i)))
			{
				if (strcmp(MMATCH_MAPNAME_RELAYMAP, MGetMapDescMgr()->GetMapName(i)) != 0)
					pCombo->Add(MGetMapDescMgr()->GetMapName(i));
			}
		}
#endif

#ifdef _CUSTOMRULEMAPS
		if ( pRule->CheckMap( MGetMapDescMgr()->GetMapName(i), bDuelMode) && !bCTFMode && !bSWRMode)
#else
		if (pRule->CheckMap(MGetMapDescMgr()->GetMapName(i), bDuelMode) && !bCTFMode)
#endif
			if(strcmp(MMATCH_MAPNAME_RELAYMAP, MGetMapDescMgr()->GetMapName(i)) != 0)
				pCombo->Add(MGetMapDescMgr()->GetMapName(i));
	}

	return true;
}
