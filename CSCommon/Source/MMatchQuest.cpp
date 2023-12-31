#include "stdafx.h"
#include "MMatchQuest.h"

#define FILENAME_NPC_DESC				"quest/npc.xml"
#define FILENAME_SCENARIO				"quest/scenario.xml"
#define FILENAME_SURVIVALSCENARIO		"quest/survivalScenario.xml"
#define FILENAME_QUESTMAP				"quest/questmap.xml"
#define FILENAME_SURVIVALMAP			"quest/survivalmap.xml"
#define FILENAME_NPCSET_DESC			"quest/npcset.xml"
#define FILENAME_DROPTABLE				"quest/droptable.xml"


MMatchQuest::MMatchQuest() : MBaseQuest()
{

}

MMatchQuest::~MMatchQuest()
{

}

bool MMatchQuest::OnCreate()
{
	if (!m_DropTable.ReadXml(FILENAME_DROPTABLE))
	{
		mlog("Droptable Read Failed");
		return false;
	}
	if (!m_NPCCatalogue.ReadXml(FILENAME_NPC_DESC))
	{
		mlog("Read NPC Catalogue Failed");
		return false;
	}

	ProcessNPCDropTableMatching();

	if (!m_NPCSetCatalogue.ReadXml(FILENAME_NPCSET_DESC))
	{
		mlog("Read NPCSet Catalogue Failed");
		return false;
	}

	if (!m_ScenarioCatalogue.ReadXml(FILENAME_SCENARIO))
	{
		mlog("Read Scenario Catalogue Failed");
		return false;
	}
	
	if (!m_MapCatalogue.ReadXml(FILENAME_QUESTMAP))
	{
		mlog("Read Questmap Catalogue Failed");
		return false;
	}

	if (!m_SurvivalScenarioCatalogue.ReadXml(FILENAME_SURVIVALSCENARIO))
	{
		mlog("Read Survival-Scenario Catalogue Failed");
		return false;
	}

	if (!m_SurvivalMapCatalogue.ReadXml(FILENAME_SURVIVALMAP))
	{
		mlog("Read Questmap Catalogue Failed");
		return false;
	}

#ifdef _DEBUG
	m_MapCatalogue.DebugReport();
	m_SurvivalMapCatalogue.DebugReport();
#endif

	// 최근 서바이벌 랭킹 리스트 요청
	MMatchServer::GetInstance()->OnRequestSurvivalModeGroupRanking();
	return true;
}

void MMatchQuest::OnDestroy()
{

}

