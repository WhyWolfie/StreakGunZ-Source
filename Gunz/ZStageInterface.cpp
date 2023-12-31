/***********************************************************************
  ZStageInterface.cpp
  
  ｿ・ ｵｵ : ｽｺﾅﾗﾀﾌﾁ・ﾀﾎﾅﾍﾆ菎ﾌｽｺｸｦ ｰ・ｮﾇﾏｴﾂ ﾅｬｷ｡ｽｺ. ﾄﾚｵ・ｰ・ｮｻ�ﾀﾇ ﾆ�ﾔｮｸｦ ﾀｧﾇﾘ
           ｺﾐｸｮﾇﾟﾀｽ(ｻ鄂ﾇ ｾﾆﾁ・ｿﾏﾀ・・ｴﾙ ｺﾐｸｮ ｸ�ﾇﾟﾀｽ. -_-;).
  ﾀﾛｼｺﾀﾏ : 11, MAR, 2004
  ﾀﾛｼｺﾀﾚ : ﾀﾓｵｿﾈｯ
************************************************************************/


#include "stdafx.h"							// Include stdafx.h
#include "ZStageInterface.h"				// Include ZStageInterface.h
#include "ZStageSetting.h"					// Include ZStageSetting.h
#include "ZGameInterface.h"
#include "ZPlayerListBox.h"
#include "ZCombatMenu.h"
#include "ZShopEquipListbox.h"
#include "ZMyItemList.h"
#include "ZItemSlotView.h"
#include "ZMessages.h"
#include "ZLanguageConf.h"
#include "MNewQuestScenario.h"
#include "MMatchSpyMap.h"

/* ﾇﾘｾﾟﾇﾒ ｰﾍｵ・..

 1. ZStageSetting ｰ・ﾃ ｷ酥ｾﾀｻ ｿｩｱ箙ﾎ ｿﾅｰﾜｿﾍｾﾟ ﾇﾏｴﾂｵ・..  -_-;
 2. ｹ�ﾆｰ UIﾂﾊｵｵ ｿｪｽﾃ ｿｩｱ箙ﾎ ｿﾅｰﾜｿﾍｾﾟ ﾇﾏｴﾂｵ･ ﾁｹｶ・ｲｿｿｩﾀﾖｴﾙ...  -_-;
*/
/***********************************************************************
  ZStageInterface : public
  
  desc : ｻ�ｼｺﾀﾚ
************************************************************************/
ZStageInterface::ZStageInterface( void)
{
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_pTopBgImg = NULL;
	m_pStageFrameImg = NULL;
	m_pItemListFrameImg = NULL;
	m_nListFramePos = 0;
	m_nStateSacrificeItemBox = 0;
	m_pRelayMapListFrameImg = NULL;
	m_nRelayMapListFramePos = 0;
	m_bRelayMapRegisterComplete = true;
	m_bEnableWidgetByRelayMap = true;
	m_pChallengeQuestScenario = new MNewQuestScenarioManager;

	m_pSpyBanMapListFrameImg = NULL;
	m_nSpyBanMapListFramePos = 0;

}


/***********************************************************************
  ~ZStageInterface : public
  
  desc : ｼﾒｸ・ﾚ
************************************************************************/
ZStageInterface::~ZStageInterface( void)
{
	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}

	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}

	delete m_pChallengeQuestScenario;
}


/***********************************************************************
  Create : public
  
  desc : ﾃﾊｱ篳ｭ
************************************************************************/
void ZStageInterface::OnCreate( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// ﾃﾊｱ篳ｭ ﾇﾘﾁﾖｰ・
	m_bPrevQuest = false;
	m_bDrawStartMovieOfQuest = false;
	m_nStateSacrificeItemBox = 0;		// Hide
	m_nGameType = MMATCH_GAMETYPE_DEATHMATCH_SOLO;
	m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
	m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	ReadSenarioNameXML();

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture)
		pPicture->SetOpacity( 255);
	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture)
		pPicture->SetOpacity( 255);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)pResource->FindWidget( "Stage_FrameBG");
	if ( pPicture)
	{
		m_pStageFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pStageFrameImg)->Create( "stage_frame.png", RGetDevice(), "interface/loadable/stage_frame.png");

		if ( m_pStageFrameImg != NULL)
			pPicture->SetBitmap( m_pStageFrameImg->GetSourceBitmap());
	}
	pPicture = (MPicture*)pResource->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
	{
		m_pItemListFrameImg = new MBitmapR2;
		((MBitmapR2*)m_pItemListFrameImg)->Create( "itemlistframe.png", RGetDevice(), "interface/loadable/itemlistframe.png");

		if ( m_pItemListFrameImg != NULL)
			pPicture->SetBitmap( m_pItemListFrameImg->GetSourceBitmap());
	}
	MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;
		rect = pWidget->GetRect();
		rect.x = -rect.w;
		m_nListFramePos = rect.x;
		pWidget->SetBounds( rect);
	}
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "Stage_SenarioName");
	if ( pLabel)
		pLabel->SetText( "");
	pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	if ( pWidget)
		pWidget->Show( false);
	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( pListBox)
		pListBox->RemoveAll();
	MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");
	if ( pDesc)
	{
		pDesc->SetTextColor( MCOLOR(0xFF808080));
		//pDesc->SetText( "ｾﾆﾀﾌﾅﾛﾀｻ ﾈｭｸ・ﾁﾟｾﾓｿ｡ ﾀﾖｴﾂ ｵﾎｰｳﾀﾇ ﾁｦｴﾜｿ｡ ｲ�ｾ�ﾏ�ﾀｽﾀｸｷﾎｽ・ｰﾔﾀﾓ ｷｹｺｧﾀｻ ﾁｶﾁ､ﾇﾒ ｼ・ﾀﾖｽﾀｴﾏｴﾙ.");
		char szText[256];
		sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
		pDesc->SetText(szText);
	}

	{ // ｸｱｷｹﾀﾌｸﾊ "RelayMap" ﾃﾊｱ篳ｭ
		pPicture = (MPicture*)pResource->FindWidget( "Stage_RelayMapListBG" );
		if(pPicture)
		{ // fｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ｹ隹・ﾀﾌｹﾌﾁ・
			m_pRelayMapListFrameImg = new MBitmapR2;
			((MBitmapR2*)m_pRelayMapListFrameImg)->Create( "relaymaplistframe.png", RGetDevice(), "interface/loadable/relaymaplistframe.png");

			if(m_pRelayMapListFrameImg != NULL)
				pPicture->SetBitmap(m_pRelayMapListFrameImg->GetSourceBitmap());
		}
		MWidget* pWidget = (MWidget*)pResource->FindWidget( "Stage_RelayMapListView");
		if ( pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();
			rect.x = -rect.w;
			m_nRelayMapListFramePos = rect.x;
			pWidget->SetBounds( rect);
		}

		MComboBox* pCombo = (MComboBox*)pResource->FindWidget("Stage_RelayMapType");			
		if ( pCombo)
			pCombo->CloseComboBoxList();
		pCombo = (MComboBox*)pResource->FindWidget("Stage_RelayMapRepeatCount");					
		if ( pCombo)
			pCombo->CloseComboBoxList();

		MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_RelayMapListbox");
		if ( pListBox)
			pListBox->RemoveAll();
		pListBox = (MListBox*)pResource->FindWidget( "Stage_MapListbox");
		if ( pListBox)
			pListBox->RemoveAll();

	}

	{ // SPY mode : ban map list.
		pPicture = (MPicture*)pResource->FindWidget("Stage_SpyBanMapListBG");
		if (pPicture)
		{ // fｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ｹ隹・ﾀﾌｹﾌﾁ・
			m_pSpyBanMapListFrameImg = new MBitmapR2;
			((MBitmapR2*)m_pSpyBanMapListFrameImg)->Create("relaymaplistframe.png", RGetDevice(), "interface/loadable/relaymaplistframe.png");

			if (m_pSpyBanMapListFrameImg != NULL)
				pPicture->SetBitmap(m_pSpyBanMapListFrameImg->GetSourceBitmap());
		}
		MWidget* pWidget = (MWidget*)pResource->FindWidget("Stage_SpyBanMapListView");
		if (pWidget)
		{
			MRECT rect;
			rect = pWidget->GetRect();
			rect.x = -rect.w;
			m_nSpyBanMapListFramePos = rect.x;
			pWidget->SetBounds(rect);
		}

		MListBox* pListBox = (MListBox*)pResource->FindWidget("Stage_SpyBanMapList");
		if (pListBox)
			pListBox->RemoveAll();

		pLabel = (MLabel*)pResource->FindWidget("Stage_SpyRandomMapLabel");
		if (pLabel)
			pLabel->Show(false);

		MComboBox* pComboBox = (MComboBox*)pResource->FindWidget("MapSelection");
		if (pComboBox)
			pComboBox->Show(true);
	}

	ZApplication::GetGameInterface()->ShowWidget("Stage_Flame0", false);
	ZApplication::GetGameInterface()->ShowWidget("Stage_Flame1", false);

	// ｰﾔﾀﾓ ｹ貎ﾄﾀﾎ ﾃ､ｷﾎ ｹ� ｳｪｰｬｴﾙｰ｡ ｴﾙｸ･ ｹ� ｵ鮴�ｰ｡ｸ� ｹ�ｱﾗ ｻ�ｱ箒ﾂｰﾅ ｼ�ﾁ､ 
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget("StageType");
	if (pCombo)
		pCombo->CloseComboBoxList();

	// ｸﾊｸｮｽｺﾆｮ ｿｬ ﾃ､ｷﾎ ｹ� ｳｪｰｬｴﾙｰ｡ ｴﾙｸ･ ｹ� ｵ鮴�ｰ｡ｸ� ｹ貘蠡ﾌ ｾﾆｴﾑｵ･ｵｵ ｸﾊ ｹﾙｲ翆�ｴﾂ ｹ�ｱﾗ ｼ�ﾁ､
	pCombo = (MComboBox*)pResource->FindWidget("MapSelection");
	if (pCombo)
		pCombo->CloseComboBoxList();


	// ﾃ､ｳﾎ ｸｮｽｺﾆｮ ｹﾚｽｺｴﾂ ｴﾝｾﾆｹ�ｸｲ
	pWidget = (MWidget*)pResource->FindWidget("ChannelListFrame");
	if (pWidget)
		pWidget->Show(false);


	// ﾈｭｸ� ｾ�ｵ･ ﾇﾑｹ� ﾇﾘﾁﾖｻ�~
	UpdateSacrificeItem();
	SerializeSacrificeItemListBox();

	// QL ﾃﾊｱ篳ｭ
	OnResponseQL(0);


	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights0", false);
	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights1", false);
}


/***********************************************************************
  OnDestroy : public
  
  desc : ｼｭｹ�ｳｪ ﾈ､ﾀｺ ｽﾃｽｺﾅﾛﾀﾇ ｿ菘ｻﾀｸｷﾎｺﾎﾅﾍ ｽｺﾅﾗﾀﾌﾁ・ﾈｭｸ鯊ｻ ｻ�ｷﾎ ｰｻｽﾅﾇﾏｴﾂ ﾇﾔｼ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDestroy( void)
{
	ZApplication::GetGameInterface()->ShowWidget( "Stage", false);

	MPicture* pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if ( pPicture)
		pPicture->SetBitmap( MBitmapManager::Get( "main_bg_t.png"));
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_FrameBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapListBG");
	if ( pPicture)
		pPicture->SetBitmap( NULL);

	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapListBG");
	if (pPicture)
		pPicture->SetBitmap(NULL);

	if ( m_pTopBgImg != NULL)
	{
		delete m_pTopBgImg;
		m_pTopBgImg = NULL;
	}
	if ( m_pStageFrameImg != NULL)
	{
		delete m_pStageFrameImg;
		m_pStageFrameImg = NULL;
	}
	if ( m_pItemListFrameImg != NULL)
	{
		delete m_pItemListFrameImg;
		m_pItemListFrameImg = NULL;
	}
	if (m_pRelayMapListFrameImg != NULL)
	{
		delete m_pRelayMapListFrameImg;
		m_pRelayMapListFrameImg = NULL;
	}
	if (m_pSpyBanMapListFrameImg != NULL)
	{
		delete m_pSpyBanMapListFrameImg;
		m_pSpyBanMapListFrameImg = NULL;
	}
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

//	m_SenarioDesc.clear();
}

void ZStageInterface::OnStageCharListSettup()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	for ( MStageCharSettingList::iterator itor = ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.begin();
		itor != ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.end();  ++itor) 
	{
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == pCharNode->uidChar)
				bMaster = true;

			pItem->UpdatePlayer( pCharNode->uidChar,(MMatchObjectStageState)pCharNode->nState,bMaster,MMatchTeam(pCharNode->nTeam));
		}
	}
}


/***********************************************************************
  OnStageInterfaceSettup : public
  
  desc : ｼｭｹ�ｳｪ ﾈ､ﾀｺ ｽﾃｽｺﾅﾛﾀﾇ ｿ菘ｻﾀｸｷﾎｺﾎﾅﾍ ｽｺﾅﾗﾀﾌﾁ・ﾈｭｸ鯊ｻ ｻ�ｷﾎ ｰｻｽﾅﾇﾏｴﾂ ﾇﾔｼ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnStageInterfaceSettup( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZStageSetting::InitStageSettingGameType();
/*
	// ｸﾊ ﾁｾｷ�ｸｦ ｼｳﾁ､ﾇﾑｴﾙ.
	MComboBox* pCB = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pCB)
	{
		int nSelected = pCB->GetSelIndex();

		InitMaps( pCB);

		if ( nSelected >= pCB->GetCount())
			nSelected = pCB->GetCount() - 1;

		pCB->SetSelIndex( nSelected);
	}
*/

	// CharListViewﾀﾇ Add, Remove, Updateｴﾂ ZGameClient::OnObjectCache ｿ｡ｼｭ ｰ・ｮﾇﾑｴﾙ.
	MSTAGE_CHAR_SETTING_NODE* pMyCharNode = NULL;
	bool bMyReady = false;		// Ready ｻ�ﾅﾂﾀﾎﾁ・ｾﾆｴﾑﾁ・..
	for ( MStageCharSettingList::iterator itor = ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.begin();
		itor != ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.end();  ++itor) 
	{
		MSTAGE_CHAR_SETTING_NODE* pCharNode = (*itor);

		// ｳｪ ﾀﾚｽﾅﾀﾏ ｰ豼・
		if ( pCharNode->uidChar == ZGetGameClient()->GetPlayerUID()) 
		{
			pMyCharNode = pCharNode;
			if (pMyCharNode->nState == MOSS_READY)
				bMyReady = true;
			else
				bMyReady = false;
		}

		ZPlayerListBox* pItem = (ZPlayerListBox*)pResource->FindWidget( "StagePlayerList_");
		if ( pItem)
		{
			bool bMaster = false;

			if ( ZGetGameClient()->GetMatchStageSetting()->GetMasterUID() == pCharNode->uidChar)
				bMaster = true;
			
			pItem->UpdatePlayer( pCharNode->uidChar,(MMatchObjectStageState)pCharNode->nState,bMaster,MMatchTeam(pCharNode->nTeam));
		}
	}

	// ｽｺﾅﾗﾀﾌﾁ�ﾀﾇ ｹ�ﾆｰ ｻ�ﾅﾂ(ｰﾔﾀﾓｽﾃﾀﾛ, ｳｭﾀﾔ, ﾁﾘｺ�ｿﾏｷ・ｸｦ ｼｳﾁ､ﾇﾑｴﾙ.
	ChangeStageButtons( ZGetGameClient()->IsForcedEntry(), ZGetGameClient()->AmIStageMaster(), bMyReady);

	// ｽｺﾅﾗﾀﾌﾁ�ﾀﾇ...
	ChangeStageGameSetting( ZGetGameClient()->GetMatchStageSetting()->GetStageSetting());
	
	// ｳｭﾀﾔ ｸ篁�ﾀﾏ ｰ豼・｡...
	if ( !ZGetGameClient()->AmIStageMaster() && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( pMyCharNode != NULL)
			ChangeStageEnableReady( bMyReady);
	}

	// ｸｸｾ・ｳｭﾀﾔﾀｸｷﾎ ｵ鮴�ﾛﾔｴﾂｵ･ ｴﾙｸ･ ｻ邯・ｴﾙ ｳｪｰ｡ｼｭ ｳｻｰ｡ ｹ貘蠡ﾌ ｵﾇｾ�ｴﾙｸ・ｳｭﾀﾔｸ�ｵ・ﾇﾘﾁｦ
	if ( (ZGetGameClient()->AmIStageMaster() == true) && ( ZGetGameClient()->IsForcedEntry()))
	{
		if ( ZGetGameClient()->GetMatchStageSetting()->GetStageState() == STAGE_STATE_STANDBY)
		{
			ZGetGameClient()->ReleaseForcedEntry();

			// ﾀﾎﾅﾍﾆ菎ﾌｽｺｰ・ﾃ
			if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_SPY)
				ZApplication::GetGameInterface()->EnableWidget("StageSettingCaller", true); // ｹ貍ｳﾁ､ ｹ�ﾆｰ
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", true);			// ｸﾊｼｱﾅﾃ ﾄﾞｺｸｹﾚｽｺ
			ZApplication::GetGameInterface()->EnableWidget( "StageType", true);				// ｰﾔﾀﾓｹ貎ﾄ ﾄﾞｺｸｹﾚｽｺ
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);		// ﾃﾖｴ・ﾎｿ・ﾄﾞｺｸｹﾚｽｺ
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", true);		// ｰ豎篳ｽｼ・ﾄﾞｺｸｹﾚｽｺ
		}
		else	// ｸｶｽｺﾅﾍﾀﾎｵ･ ｴﾙｸ･ｻ邯�ｵ・ｰﾔﾀﾓﾁﾟﾀﾌｸ・ﾀﾎﾅﾍﾆ菎ﾌｽｺ Disable
		{
			// ﾀﾎﾅﾍﾆ菎ﾌｽｺｰ・ﾃ
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);	// ｹ貍ｳﾁ､ ｹ�ﾆｰ
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);			// ｸﾊｼｱﾅﾃ ﾄﾞｺｸｹﾚｽｺ
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);			// ｰﾔﾀﾓｹ貎ﾄ ﾄﾞｺｸｹﾚｽｺ
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);		// ﾃﾖｴ・ﾎｿ・ﾄﾞｺｸｹﾚｽｺ
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);		// ｰ豎篳ｽｼ・ﾄﾞｺｸｹﾚｽｺ
		}
	}

	/// ｸｱｷｹﾀﾌｸﾊﾀﾌｸ鮠ｭ ｴﾙﾀｽｸﾊﾀｸｷﾎ ｰ霈ﾓ ﾁ�ﾇ狠ﾟﾀﾏｶｧ ｽﾃﾀﾛ ｹ�ﾆｰﾀｻ ｸ�ｵﾎ OFF ｽﾃﾄﾑﾁﾘｴﾙ.
	if(	ZGetGameClient()->AmIStageMaster() && 
		!m_bEnableWidgetByRelayMap && 
		ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bIsRelayMap)
	{
		ZApplication::GetGameInterface()->EnableWidget( "GameStart", false);
		// ZApplication::GetGameInterface()->EnableWidget( "StageReady", false);	// ﾁﾘｺ�ｿﾏｷ盪�ﾆｰ(ｱ篳ｹｿ｡ ｵ�ｶ・ｺｯｰ豌｡ｴﾉ)

		// ﾀﾎﾅﾍﾆ菎ﾌｽｺｰ・ﾃ
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller",	false);		// ｹ貍ｳﾁ､ ｹ�ﾆｰ
		ZApplication::GetGameInterface()->EnableWidget( "MapSelection",			false);		// ｸﾊｼｱﾅﾃ ﾄﾞｺｸｹﾚｽｺ
		ZApplication::GetGameInterface()->EnableWidget( "StageType",			false);		// ｰﾔﾀﾓｹ貎ﾄ ﾄﾞｺｸｹﾚｽｺ
		ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer",		false);		// ﾃﾖｴ・ﾎｿ・ﾄﾞｺｸｹﾚｽｺ
		ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount",		false);		// ｰ豎篳ｽｼ・ﾄﾞｺｸｹﾚｽｺ

		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller",		false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType",		false);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", false);
	}

	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	// ﾈｭｸ・ｻ�ｴﾜﾀﾇ ｸﾊ ﾀﾌｹﾌﾁ・ｼｳﾁ､ﾇﾏｱ・
	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		char szMapName[256];
		pPicture = (MPicture*)pResource->FindWidget("Stage_MainBGTop");
		if (pPicture)
		{
			if (0 == strcmp(MMATCH_MAPNAME_RELAYMAP, ZGetGameClient()->GetMatchStageSetting()->GetMapName()))
			{
				sprintf(szMapName, "interface/MapGB/%s", MGetMapDescMgr()->GetMapImageName(MMATCH_DEFAULT_STAGESETTING_MAPNAME));
			}
			else
			{
				sprintf(szMapName, "interface/MapGB/%s", MGetMapDescMgr()->GetMapImageName(ZGetGameClient()->GetMatchStageSetting()->GetMapName()));
			}
			if (m_pTopBgImg != NULL)
			{
				delete m_pTopBgImg;
				m_pTopBgImg = NULL;
			}

			m_pTopBgImg = new MBitmapR2;
			((MBitmapR2*)m_pTopBgImg)->Create("TopBgImg.png", RGetDevice(), szMapName);

			if (m_pTopBgImg != NULL)
				pPicture->SetBitmap(m_pTopBgImg->GetSourceBitmap());
		}
	}
	
	// ﾁ､ｺｸﾃ｢ｿ｡ ｰﾔﾀﾓｹ戝ｦ ｼｳﾁ､ﾇﾏｱ・
	MLabel* pLabel = (MLabel*)pResource->FindWidget( "StageNameLabel");
	if ( pLabel != 0)
	{
		char szStr[ 256];
		// Custom: remove servername from titlebar
		sprintf( szStr, "%s > %03d:%s", ZMsg( MSG_WORD_STAGE), ZGetGameClient()->GetStageNumber(), ZGetGameClient()->GetStageName());
		pLabel->SetText( szStr);
	}

	// ｻ�ﾇﾏｴﾜ ｽｺﾆｮｶ�ﾀﾌﾇﾁﾀﾇ ｻ�ｻ・ｹﾙｲﾙｱ・
#define SDM_COLOR			MCOLOR(255,0,0)
#define TDM_COLOR			MCOLOR(0,255,0)
#define SGD_COLOR			MCOLOR(0,0,255)
#define TGD_COLOR			MCOLOR(255,255,0)
#define ASSASIN_COLOR		MCOLOR(255,0,255)
#define TRAINING_COLOR		MCOLOR(0,255,255)
#define QUEST_COLOR			MCOLOR(255,255,255)
#define SURVIVAL_COLOR		MCOLOR(255,255,255)

	MCOLOR color;
	switch ( ZGetGameClient()->GetMatchStageSetting()->GetGameType() )
	{	
		case MMATCH_GAMETYPE_ASSASSINATE:
			color = ASSASIN_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_SOLO:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DEATHMATCH_TEAM:
		case MMATCH_GAMETYPE_DEATHMATCH_TEAM2:
		case MMATCH_GAMETYPE_CTF:
		case MMATCH_GAMETYPE_INFECTED:
			color = TDM_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_SOLO:
		case MMATCH_GAMETYPE_GUNGAME:
			color = SGD_COLOR;
			break;

		case MMATCH_GAMETYPE_GLADIATOR_TEAM:
			color = TGD_COLOR;
			break;

		case MMATCH_GAMETYPE_TRAINING:
		case MMATCH_GAMETYPE_TEAM_TRAINING:
			color = TRAINING_COLOR;
			break;

#ifdef _QUEST
		case MMATCH_GAMETYPE_SURVIVAL:
			color = QUEST_COLOR;
			break;

		case MMATCH_GAMETYPE_QUEST:
			color = SURVIVAL_COLOR;
			break;
#endif
		case MMATCH_GAMETYPE_QUEST_CHALLENGE:
			color = QUEST_COLOR; //todok ﾄ�ｽｺﾆｮﾃｧｸｰﾁ・ｻ�ｱ・
			break;

		case MMATCH_GAMETYPE_BERSERKER:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_DUEL:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_DUELTOURNAMENT:
			color = SDM_COLOR;
			break;
		case MMATCH_GAMETYPE_DROPMAGIC:
			color = SDM_COLOR;
			break;

		case MMATCH_GAMETYPE_SPY:
			color = TDM_COLOR;
			break;

		default:
			_ASSERT(0);
			color = MCOLOR(255,255,255,255);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripBottom");
	if(pPicture != NULL && !pPicture->IsAnim())
	{		
        pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 2, 700.0f);
	}
	pPicture = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_StripTop");
	if(pPicture != NULL && !pPicture->IsAnim())	
	{
		pPicture->SetBitmapColor( color );
		if(!(pPicture->GetBitmapColor().GetARGB() == pPicture->GetReservedBitmapColor().GetARGB()))
			pPicture->SetAnimation( 3, 700.0f);		
	}
}

void ZStageInterface::SetStageRelayMapImage()
{
	MPicture* pPicture = 0;
	MBitmap* pBitmap = 0;
	char szMapName[256];
	pPicture = (MPicture*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_MainBGTop");
	if(!pPicture) return;
	MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelayMapListBox == NULL) return;
	if( 0 < pRelayMapListBox->GetCount())
	{
		sprintf( szMapName, "interface/MapGB/%s", MGetMapDescMgr()->GetMapImageName( pRelayMapListBox->GetString(pRelayMapListBox->GetStartItem())));
		if ( m_pTopBgImg != NULL)
		{
			delete m_pTopBgImg;
			m_pTopBgImg = NULL;
		}

		m_pTopBgImg = new MBitmapR2;
		((MBitmapR2*)m_pTopBgImg)->Create( "TopBgImg.png", RGetDevice(), szMapName);

		if ( m_pTopBgImg != NULL)
			pPicture->SetBitmap( m_pTopBgImg->GetSourceBitmap());
	}
}

void ZStageInterface::SetEnableWidgetByRelayMap(bool b)
{
	if(!ZGetGameClient()->GetMatchStageSetting()->GetStageSetting()->bIsRelayMap)
	{	// ｸｱｷｹﾀﾌｸﾊﾀﾌ ｾﾆｴﾏｸ・ｸｱｷｹﾀﾌｸﾊｿ・ﾀｧﾁｬ ｺ�ﾈｰｼｺﾈｭ ﾃｳｸｮｸｦ ﾇﾒ ﾇﾊｿ・ｾ�ｴﾙ.
		m_bEnableWidgetByRelayMap = true;
		return;
	}

	m_bEnableWidgetByRelayMap = b;
}

/***********************************************************************
  ChangeStageGameSetting : public
  
  desc : ﾀﾌｰﾍｵｵ ｰﾔﾀﾓ ｰ・ﾃ ﾀﾎﾅﾍﾆ菎ﾌｽｺｸｦ ｼ�ﾁ､ﾇﾏｴﾂｰﾅ ｰｰﾀｺｵ･... ｿﾖﾀﾌｷｸｰﾔ ｸｹﾀﾌ ｳｪｴｲｳ�ﾀｺｰﾅﾁ・ -_-;
         ﾁﾖｷﾎ ﾈｭｸ鯊ﾇ ﾀ・ｼﾀ�ﾀﾎ UIｸｦ ｼｳﾁ､ﾇﾑｴﾙ.
  arg  : pSetting = ｽｺﾅﾗﾀﾌﾁ・ｼｳﾁ､ ﾁ､ｺｸ
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageGameSetting( const MSTAGE_SETTING_NODE* pSetting)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_nGameType = pSetting->nGameType;

	// Set map name
	// Custom: set map name to random (since it shows a list)
//	if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_SPY)
		SetMapName(pSetting->szMapName);
//	else
//		SetMapName("random map");

	// Is team game?
	ZApplication::GetGameInterface()->m_bTeamPlay = ZGetGameTypeManager()->IsTeamGame( pSetting->nGameType);


	// ｰ・・ﾇ譱・ｿｩｺﾎ ﾈｮﾀﾎ
	MComboBox* pCombo = (MComboBox*)pResource->FindWidget( "StageObserver");
	MButton* pObserverBtn = (MButton*)pResource->FindWidget( "StageObserverBtn");
	MLabel* pObserverLabel = (MLabel*)pResource->FindWidget( "StageObserverLabel");
	if ( pCombo && pObserverBtn && pObserverLabel)
	{
		if ( pCombo->GetSelIndex() == 1)
		{
			pObserverBtn->SetCheck( false);
			pObserverBtn->Enable( false);
			pObserverLabel->Enable( false);
		}
		else
		{
			pObserverBtn->Enable( true);
			pObserverLabel->Enable( true);
		}
	}

	// ﾃｻﾆﾀ, ﾈｫﾆﾀ ｻ�ﾅﾂ ｼｳﾁ､
	ZApplication::GetGameInterface()->UpdateBlueRedTeam();

	// ｰﾔﾀﾓ ｹ貎ﾄｿ｡ ｵ�ｶ�ｼｭ UIｸｦ ｺｯｰ貮ﾑｴﾙ
	MAnimation* pAniMapImg = (MAnimation*)pResource->FindWidget( "Stage_MapNameBG");
	bool bQuestUI = false;
	bool bChallengeQuestUI = false;
	bool bSpyUI = false;
	if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||			// ｵ･ｾｲｸﾅﾄ｡ ｰｳﾀﾎﾀ・ﾌｰﾅｳｪ...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||				// ﾄｮﾀ・ｰｳﾀﾎﾀ・ﾌｰﾅｳｪ...
		 (pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||					// ｹ�ｼｭﾄｿｸ�ｵ蠡ﾌｰﾅｳｪ...
		 (pSetting->nGameType == MMATCH_GAMETYPE_DROPMAGIC) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||					// ﾆｮｷｹﾀﾌｴﾗﾀﾌｰﾅｳｪ...
		 (pSetting->nGameType == MMATCH_GAMETYPE_DUEL))							// ｵ狎�ｸ�ｵ・ﾀﾌｸ・..
	{
		// ｸﾊ ﾀﾌｸｧ ｹ隹・ﾀﾌｹﾌﾁ・ｺｯﾈｯ
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		// ﾄ�ｽｺﾆｮ UI ｰｨﾃ・
		bQuestUI = false;
		bChallengeQuestUI = false;
	}
	else if ( (pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM) ||		// ｵ･ｾｲｸﾅﾄ｡ ﾆﾀﾀ・ﾌｰﾅｳｪ...
		(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||			// ｹｫﾇﾑｵ･ｽｺｸﾅﾄ｡ ﾆﾀﾀ・ﾌｰﾅｳｪ...
		 (pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_TEAM) ||				// ﾄｮﾀ・ﾆﾀﾀ・ﾌｰﾅｳｪ...
		 (pSetting->nGameType == MMATCH_GAMETYPE_ASSASSINATE) ||				// ｾﾏｻ・・ﾀﾌｸ・..
		 (pSetting->nGameType == MMATCH_GAMETYPE_TEAM_TRAINING) ||
		 (pSetting->nGameType == MMATCH_GAMETYPE_CTF))
	{
		// ｸﾊ ﾀﾌｸｧ ｹ隹・ﾀﾌｹﾌﾁ・ｺｯﾈｯ
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 1);

		// ﾄ�ｽｺﾆｮ UI ｰｨﾃ・
		bQuestUI = false;
		bChallengeQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL)					// ﾃｧｸｰﾁ�ﾄ�ｽｺﾆｮ ｸ�ｵ・/todok ｵ�ｷﾎ ﾇﾘｾﾟﾇﾒｱ・
	{
		// ｸﾊ ﾀﾌｸｧ ｹ隹・ﾀﾌｹﾌﾁ・ｺｯﾈｯ
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 0);

		// ﾄ�ｽｺﾆｮ UI ｰｨﾃ・
		bQuestUI = false;
		bChallengeQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST)						// ﾄ�ｽｺﾆｮ ｸ�ｵ蠡ﾌｸ・..
	{
		// ｸﾊ ﾀﾌｸｧ ｹ隹・ﾀﾌｹﾌﾁ・ｺｯﾈｯ
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 2);

		// ﾄ�ｽｺﾆｮ UI ｺｸﾀﾓ
		bQuestUI = true;
		bChallengeQuestUI = false;
	}

	else if (pSetting->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE)					// ｼｭｹﾙﾀﾌｹ・ｸ�ｵ蠡ﾌｸ・..
	{
		// ｸﾊ ﾀﾌｸｧ ｹ隹・ﾀﾌｹﾌﾁ・ｺｯﾈｯ
		if (pAniMapImg)
			pAniMapImg->SetCurrentFrame(3);

		// ﾄ�ｽｺﾆｮ UI ｰｨﾃ・
		bChallengeQuestUI = true;
	}

	else if ( pSetting->nGameType == MMATCH_GAMETYPE_INFECTED)					// Custom: Infected game mode stage
	{
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame( 4);

		bQuestUI = false;
		bChallengeQuestUI = false;
	}
	else if ( pSetting->nGameType == MMATCH_GAMETYPE_GUNGAME)					// Custom: GunGame
	{
		if ( pAniMapImg)
			pAniMapImg->SetCurrentFrame(0);

		bQuestUI = false;
		bChallengeQuestUI = false;
	}

	else if (pSetting->nGameType == MMATCH_GAMETYPE_SPY)
	{
		if (pAniMapImg)
			pAniMapImg->SetCurrentFrame(6);	// Check your interface templates.xml for more details about this number.

		bQuestUI = false;
		bChallengeQuestUI = false;
		bSpyUI = true;
	}


	// ｸﾊｼｱﾅﾃｽﾃ ｸｱｷｹﾀﾌｸﾊﾀﾌｸ・ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ｹﾚｽｺｸｦ ｿｭｾ�ﾝﾘｴﾙ.
	 if(pSetting->bIsRelayMap)
		OpenRelayMapBox();
	else
		HideRelayMapBox();

	 ///// SPY MODE /////
	 if (bSpyUI)
		 OpenSpyBanMapBox();
	 else
		 HideSpyBanMapBox();

	 MLabel* pSpyRandomMapLabel = (MLabel*)pResource->FindWidget("Stage_SpyRandomMapLabel");
	 if (pSpyRandomMapLabel)
		 pSpyRandomMapLabel->Show(bSpyUI);

pCombo = (MComboBox*)pResource->FindWidget("MapSelection");
if (pCombo)
pCombo->Show(!bSpyUI);
////////////////////
ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemImage0", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemImage1", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_QuestLevel", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_QuestLevelBG", bQuestUI);

// Custom: CQ Add Time Limit
ZApplication::GetGameInterface()->ShowWidget("Stage_ChallengeTime", bChallengeQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_ChallengeTimeBG", bChallengeQuestUI);

ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemButton0", bQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_SacrificeItemButton1", bQuestUI);

if (m_bPrevQuest != bQuestUI)
{
	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights0", bQuestUI);
	ZApplication::GetGameInterface()->ShowWidget("Stage_Lights1", bQuestUI);

	m_SacrificeItem[SACRIFICEITEM_SLOT0].RemoveItem();
	m_SacrificeItem[SACRIFICEITEM_SLOT1].RemoveItem();

	UpdateSacrificeItem();

	if (bQuestUI)
	{
		//			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
		ZPostRequestSacrificeSlotInfo(ZGetGameClient()->GetPlayerUID());
		ZPostRequestQL(ZGetGameClient()->GetPlayerUID());
		OpenSacrificeItemBox();
	}
	else
	{
		MLabel* pLabel = (MLabel*)pResource->FindWidget("Stage_SenarioName");
		if (pLabel)
			pLabel->SetText("");
		ZApplication::GetGameInterface()->ShowWidget("Stage_SenarioNameImg", false);
		//			if (ZGetGameClient()->AmIStageMaster())
		//				ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", true);

		HideSacrificeItemBox();
	}

	m_bPrevQuest = !m_bPrevQuest;
}

if ((pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || (pSetting->nGameType == MMATCH_GAMETYPE_QUEST) || (pSetting->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE) || (pSetting->nGameType == MMATCH_GAMETYPE_SPY))
ZApplication::GetGameInterface()->EnableWidget("StageSettingCaller", false);


ZApplication::GetGameInterface()->ShowWidget("Stage_SenarioDescription", bChallengeQuestUI);
ZApplication::GetGameInterface()->ShowWidget("Stage_SenarioTitle", bChallengeQuestUI);
//ZApplication::GetGameInterface()->ShowWidget("Stage_GoodTime", bChallengeQuestUI);
// Custom: CQ Add Time Limit
ZApplication::GetGameInterface()->ShowWidget("Stage_ChallengeTime", bChallengeQuestUI);
if (bChallengeQuestUI)
{
	char szText[256];
	MNewQuestScenarioManager* pNewScenarioMgr = ZApplication::GetStageInterface()->GetChallengeQuestScenario();
	if (pNewScenarioMgr)
	{
		MNewQuestScenario* pNewScenario = pNewScenarioMgr->GetScenario(pSetting->szMapName);
		if (pNewScenario)
		{
			MTextArea* pNewScenarioDesc = (MTextArea*)pResource->FindWidget("Stage_SenarioDescription");
			if (pNewScenarioDesc)
			{
				pNewScenarioDesc->SetText(pNewScenario->GetDescription());
			}

			MLabel* pNewScenarioTitle = (MLabel*)pResource->FindWidget("Stage_SenarioTitle");
			if (pNewScenarioTitle)
			{
				pNewScenarioTitle->SetText(pNewScenario->GetName());
			}
			// Custom: CQ Add Time Limit
			MLabel* pGoodTime = (MLabel*)pResource->FindWidget("Stage_ChallengeTime");
			if (pGoodTime)
			{
				sprintf(szText, "Gen. Record : 8:00", pNewScenario->GetGoodTime() / 60, pNewScenario->GetGoodTime() % 60);
				pGoodTime->SetText(szText);
			}
			MPicture* pPicture = 0;
			MBitmap* pBitmap = 0;
			char szMapName[256];
			pPicture = (MPicture*)pResource->FindWidget("Stage_MainBGTop");
			if (pPicture)
			{
				if (((pNewScenario->GetID() == 101) ||
					(pNewScenario->GetID() == 201) ||
					(pNewScenario->GetID() == 301) ||
					(pNewScenario->GetID() == 401)))
					sprintf(szMapName, "interface/MapGB/map_ChallengeQuest_Guerilla.bmp");
				else
					sprintf(szMapName, "interface/MapGB/map_ChallengeQuest_Research.bmp");
				// Custom: Blitzkrieg BETA banner map 
				 if(pNewScenario->GetID() == 403)
				  {
					 sprintf(szMapName, "interface/MapGB/map_BlitzKrieg.bmp");
                  }

					if (m_pTopBgImg != NULL)
					{
						delete m_pTopBgImg;
						m_pTopBgImg = NULL;
					}

					m_pTopBgImg = new MBitmapR2;
					((MBitmapR2*)m_pTopBgImg)->Create("TopBgImg.png", RGetDevice(), szMapName);

					if (m_pTopBgImg != NULL)
						pPicture->SetBitmap(m_pTopBgImg->GetSourceBitmap());
				}
			}
		}
	}
	// ｶ�ｿ�ﾑ・ｼｱﾅﾃ ﾄﾞｺｸｹﾚｽｺ ｺｸﾀﾌｱ・
//	bool bShowRound = true;
//	if ( ( pSetting->nGameType == MMATCH_GAMETYPE_SURVIVAL) || ( pSetting->nGameType == MMATCH_GAMETYPE_QUEST))
//		bShowRound = false;
	
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCountLabelBG", bShowRound);
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCountLabel", bShowRound);
//	ZApplication::GetGameInterface()->ShowWidget( "StageRoundCount", bShowRound);


	// ｶ�ｿ�ﾑ・or Kill
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageRoundCountLabel");
	if ( pWidget)
	{
		if ((pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DEATHMATCH_TEAM2) ||		// ﾆﾀｵ･ｽｺｸﾅﾄ｡ ﾀﾍｽｺﾆｮｸｲﾀﾏ ｶｧｿ｡ｵｵ Killｷﾎ ﾇ･ｽﾃ.... by kammir 20081117
			(pSetting->nGameType == MMATCH_GAMETYPE_GLADIATOR_SOLO) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_TRAINING) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DROPMAGIC) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_BERSERKER) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_DUEL) ||
			(pSetting->nGameType == MMATCH_GAMETYPE_GUNGAME))
			pWidget->SetText( ZMsg(MSG_WORD_KILL));

		else if(pSetting->nGameType == MMATCH_GAMETYPE_CTF)
			pWidget->SetText(ZMsg(MSG_WORD_CAPUTRES)); // derp, maiet. no more IDs for MSG_WORD_CAPTURES D:
		else
			pWidget->SetText( ZMsg(MSG_WORD_ROUND));
	}


	// ﾄﾞﾆ�ｳﾍﾆｮ ｾ�ｵ･ﾀﾌﾆｮ
	ZStageSetting::ShowStageSettingDialog( pSetting, false);


	// ｰﾔﾀﾓﾁﾟ ｸﾞｴｺ ｼ�ﾁ､ - ｸﾓﾇﾏｴﾂ ｺﾎｺﾐﾀﾎﾁ・ｾﾋｼ・ｾ�ﾀｽ...
	// ｰﾔﾀﾓﾁﾟ ｸﾞｴｺｿ｡ｼｭ ﾄ�ｽｺﾆｮｹﾗ ｼｭｹﾙﾀﾌｹ�ﾀﾏｶｧ ｴ・篁貘ｸｷﾎ ｳｪｰ｡ｱ・ｹ�ﾆｰ ｺ�ﾈｰｼｺﾈｭ ｽﾃﾅｴ
#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(pSetting->nGameType) || pSetting->nGameType == MMATCH_GAMETYPE_QUEST_CHALLENGE || ZGetGameClient()->IsCWChannel())
	{
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_STAGE_EXIT, false);
	}
	else if (ZGetGameClient()->IsPWChannel())
	{
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, true);
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_STAGE_EXIT, true);
		//ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_PROG_EXIT, false);
		//ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_CLOSE, false);
	}
	else
	{
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_BATTLE_EXIT, true);
		ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_STAGE_EXIT, true);
		//ZApplication::GetGameInterface()->GetCombatMenu()->EnableItem(ZCombatMenu::ZCMI_PROG_EXIT, true);
	}
#endif
}


/***********************************************************************
  ChangeStageButtons : public
  
  desc : ｽｺﾅﾗﾀﾌﾁ・ｳｻﾀﾇ ｹ�ﾆｰｵ・ｰﾔﾀﾓｽﾃﾀﾛ, ｳｭﾀﾔ, ﾁﾘｺ�ｿﾏｷ・ﾀﾇ ｻ�ﾅﾂｸｦ ｼｳﾁ､ﾇﾑｴﾙ.
         ｰﾔﾀﾓ ｼｳﾁ､ｰ・ｰ・ﾃｵﾈ ﾀｧﾁｬﾀﾇ UIｸｦ ｼｳﾁ､ﾇﾑｴﾙ.
  arg  : bForcedEntry = ｳｭﾀﾔ ｿｩｺﾎ(true or false)
         bMaster = ｹ貘・ｿｩｺﾎ(true or false)
		 bReady = ﾁﾘｺ・ｿﾏｷ・ｿｩｺﾎ(true or false)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageButtons( bool bForcedEntry, bool bMaster, bool bReady)
{
	if ( bForcedEntry)	// ｳｭﾀﾔ ｸ�ｵ・
	{
		ZApplication::GetGameInterface()->ShowWidget( "GameStart", false);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", false);

		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", true);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", true);

		ChangeStageEnableReady( false);
	}
	else
	{
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame", false);
		ZApplication::GetGameInterface()->ShowWidget( "ForcedEntryToGame2", false);

		ZApplication::GetGameInterface()->ShowWidget( "GameStart", bMaster);
		ZApplication::GetGameInterface()->ShowWidget( "StageReady", !bMaster);

		if ( bMaster)
		{
			// ｺｸﾅ・ｹ貘蠡ﾌｶ�ｸ・ｹ貍ｼﾆﾃﾀｻ ｹﾙｲﾜ ｼ・ﾀﾖｴﾂ ｱﾇﾇﾑﾀｻ ｰ｡ﾁ�ｴﾙ (UI enable).
			// ｱﾗｷｯｳｪ ｱﾗｷ｡ｼｭｴﾂ ｾﾈｵﾇｴﾂ ｰ豼・｡ ﾀﾖｴﾂｵ･ ｱﾗｰﾉ ﾃｼﾅｩﾇﾑｴﾙ.
			bool bMustDisableUI_despiteMaster = false;
			
			// ﾄ�ｽｺﾆｮｸ�ｵ蠢｡ｼｭ ｹ貘蠡ﾌ ｰﾔﾀﾓｽﾃﾀﾛﾀｻ ｴｩｸ｣ｰ・ﾈ�ｻ�ｾﾆﾀﾌﾅﾛ ｺﾒﾅｸｴﾂ ｾﾖｴﾏｸﾞﾀﾌｼﾇ ｵｵﾁﾟ ｹ貘蠡ﾌ ｽｺｽｺｷﾎｸｦ kickﾇﾘｼｭ
			// ｴﾙｸ･ ｻ邯�ﾀﾌ ｹ貘・ｱﾇﾇﾑﾀｻ ｾ�ﾀｺ ｰ豼・ ｳｻｰ｡ ｹ貘蠡ﾏﾁ�ｶ�ｵｵ ｴ�ﾀﾌｻ・ｹ貍ｼﾆﾃﾀｻ ｹﾙｲﾜ ｼ�ｴﾂ ｾ�ｴﾙ
			if ( m_bDrawStartMovieOfQuest)
				bMustDisableUI_despiteMaster = true;


			if (bMustDisableUI_despiteMaster)
				ChangeStageEnableReady( true);	// UI disable
			else
				ChangeStageEnableReady( false);	// UI enable
		}
		else
			ChangeStageEnableReady( bReady);
	}
}


/***********************************************************************
  ChangeStageEnableReady : public
  
  desc : ｽｺﾅﾗﾀﾌﾁ�ﾀﾇ ﾀﾎﾅﾍﾆ菎ﾌｽｺ(ｹ�ﾆｰ enableｵ・ｸｦ ﾄﾑｰﾅｳｪ ｲ�ｴﾂ ﾇﾔｼ・
  arg  : true(=interface enable) or false(=interface disable)
  ret  : none
************************************************************************/
void ZStageInterface::ChangeStageEnableReady( bool bReady)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	ZApplication::GetGameInterface()->EnableWidget( "GameStart", !bReady);

	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamBlue2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed",  !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "StageTeamRed2", !bReady);
	ZApplication::GetGameInterface()->EnableWidget( "Lobby_StageExit", !bReady);

	if ( (m_nGameType == MMATCH_GAMETYPE_SURVIVAL) || (m_nGameType == MMATCH_GAMETYPE_QUEST))
	{
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemListbox", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_PutSacrificeItem",     !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton0", !bReady);
		ZApplication::GetGameInterface()->EnableWidget( "Stage_SacrificeItemButton1", !bReady);
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
		}
//		ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
		ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
	}
	else
	{
		if ( ZGetGameClient()->AmIStageMaster())
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", !bReady);
			if (m_nGameType != MMATCH_GAMETYPE_SPY)
				ZApplication::GetGameInterface()->EnableWidget("StageSettingCaller", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType", !bReady);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", !bReady);
		}
		else
		{
			ZApplication::GetGameInterface()->EnableWidget( "MapSelection", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageType", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageMaxPlayer", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageRoundCount", false);
			ZApplication::GetGameInterface()->EnableWidget( "StageSettingCaller", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapType", false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMapRepeatCount", false);
		}
	}
    
	BEGIN_WIDGETLIST( "Stage_OptionFrame", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST( "EquipmentCaller", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	// Custom: Show shop in stage
	BEGIN_WIDGETLIST( "ShopCaller", pResource, MButton*, pButton);
	pButton->Enable( !bReady);
	END_WIDGETLIST();

	BEGIN_WIDGETLIST("MedalShopCaller", pResource, MButton*, pButton);
	pButton->Enable(!bReady);
	END_WIDGETLIST();

	ZApplication::GetGameInterface()->EnableWidget( "StagePlayerList_", !bReady);
}


/***********************************************************************
  SetMapName : public
  
  desc : ｸﾊ ｼｱﾅﾃ ﾄﾞｺｸｹﾚｽｺｿ｡ ﾇ�ﾀ・ｼｱﾅﾃｵﾈ ｸﾊ ﾀﾌｸｧﾀｻ ｼｼﾆﾃﾇﾑｴﾙ.
  arg  : szMapName = ｸﾊ ﾀﾌｸｧ
  ret  : none
************************************************************************/
void ZStageInterface::SetMapName( const char* szMapName)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( szMapName == NULL)
		return;

	MComboBox* pMapCombo = (MComboBox*)pResource->FindWidget( "MapSelection");
	if ( pMapCombo)
	{
		// ﾀﾏｴﾜ ﾀﾓｽﾃ ﾇﾏｵ蠧ﾚｵ・ｿ・｡ｿｨ~ ､ﾐ.､ﾐ)
//		if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
//			pMapCombo->SetText( "Mansion");
//		else
		pMapCombo->SetText(szMapName);
	}
}




/*
	ｿｩｱ箴ｭｺﾎﾅﾍ ｻ�ｷﾎ ﾃﾟｰ｡ｵﾈ ｳｻｿ・..

	ｿﾘｸｸﾇﾑｰﾇ ｸｮｽｺｳﾊｳｪ ｴﾙｸ･ ｰ�ｿ｡ｼｭ ﾀﾚｵｿﾀｸｷﾎ ﾈ｣ﾃ箏ﾇｵｵｷﾏ ﾇﾘｳ�ﾀｸｳｪ ｾﾆﾁ・ﾅﾗｽｺﾆｮﾀﾎ ｰ・霍ﾎ
	ｿﾏｺｮﾇﾑｰﾇ ｾﾆｴﾔ...  -_-;
*/



/***********************************************************************
  OpenSacrificeItemBox : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｼｱﾅﾃ ﾃ｢ ｿｭｱ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OpenSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( true);

	m_nStateSacrificeItemBox = 2;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  CloseSacrificeItemBox : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｼｱﾅﾃ ﾃ｢ ｴﾝｱ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::CloseSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 1;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  HideSacrificeItemBox : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｼｱﾅﾃ ﾃ｢ ｰｨﾃﾟｱ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::HideSacrificeItemBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_SacrificeItemBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	m_nStateSacrificeItemBox = 0;
	GetSacrificeItemBoxPos();
}


/***********************************************************************
  HideSacrificeItemBox : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｼｱﾅﾃ ﾃ｢ ﾀｧﾄ｡ ｱｸﾇﾏｱ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::GetSacrificeItemBoxPos( void)
{
//#ifdef _DEBUG
//	m_nListFramePos = 0;
//	return;
//#endif

	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemListView");
	if ( pWidget)
	{
		MRECT rect;

		switch ( m_nStateSacrificeItemBox)
		{
			case 0 :		// Hide
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w;
				break;

			case 1 :		// Close
				rect = pWidget->GetRect();
				m_nListFramePos = -rect.w + ( rect.w * 0.14);
				break;

			case 2 :		// Open
				m_nListFramePos = 0;
				break;
		}
	}
}


/***********************************************************************
  OnSacrificeItem0 : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ0ﾀｻ ｴｭｷｶﾀｻ ｶｧ
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem0( void)
{
}


/***********************************************************************
  OnSacrificeItem1 : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ1ｸｦ ｴｭｷｶﾀｻ ｶｧ
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnSacrificeItem1( void)
{
}


/***********************************************************************
  UpdateSacrificeItem : protected
  
  desc : ｺｯｰ豬ﾈ ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ﾀﾌｹﾌﾁ・ ﾁ､ｺｸｵ�ﾜｻ ｾ�ｵ･ﾀﾌﾆｮ ﾇﾔ.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::UpdateSacrificeItem( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// ｽｺﾅﾗﾀﾌﾁ・ｿｵｿｪｿ｡ ﾀﾖｴﾂ ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ﾀﾌｹﾌﾁ・ｼ�ﾁ､
	for ( int i = SACRIFICEITEM_SLOT0;  i <= SACRIFICEITEM_SLOT1;  i++)
	{
		char szWidgetNameItem[ 128];
		sprintf( szWidgetNameItem, "Stage_SacrificeItemImage%d", i);
		MPicture* pPicture = (MPicture*)pResource->FindWidget( szWidgetNameItem);
		if ( pPicture)
		{
			if ( m_SacrificeItem[ i].IsExist())
			{
				pPicture->SetBitmap( m_SacrificeItem[ i].GetIconBitmap());
				char szMsg[ 128];
				MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache( m_SacrificeItem[ i].GetUID());
				if ( pObjCache)
					sprintf( szMsg, "%s (%s)", m_SacrificeItem[ i].GetName(), pObjCache->GetName());
				else
					strcpy( szMsg, m_SacrificeItem[ i].GetName());
				pPicture->AttachToolTip( szMsg);
			}
			else
			{
				pPicture->SetBitmap( NULL);
				pPicture->DetachToolTip();
			}
		}
	}
}


/***********************************************************************
  SerializeSacrificeItemListBox : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｸｮｽｺﾆｮ ｹﾚｽｺｿ｡ ﾀﾚｷ盧ｦ ｹﾞｴﾂｴﾙ.
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::SerializeSacrificeItemListBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox)
		return;

	int nStartIndex  = pListBox->GetStartItem();
	int nSelectIndex = pListBox->GetSelIndex();
	pListBox->RemoveAll();

	// ｸｮｽｺﾆｮｿ｡ ﾃﾟｰ｡
	for ( MQUESTITEMNODEMAP::iterator questitem_itor = ZGetMyInfo()->GetItemList()->GetQuestItemMap().begin();
		  questitem_itor != ZGetMyInfo()->GetItemList()->GetQuestItemMap().end();
		  questitem_itor++)
	{
		ZMyQuestItemNode* pItemNode = (*questitem_itor).second;
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( pItemNode->GetItemID());
		if ( pItemDesc)
		{
			int nCount = pItemNode->m_nCount;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID()))
				nCount--;
			if ( m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist() &&
				 (m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() == ZGetGameClient()->GetPlayerUID()) &&
				 (pItemDesc->m_nItemID == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID()))
				nCount--;

			if ( pItemDesc->m_bSecrifice && (nCount > 0))		// ﾈ�ｻ� ｾﾆﾀﾌﾅﾛｸｸ ﾃﾟｰ｡
			{
				pListBox->Add( new SacrificeItemListBoxItem( pItemDesc->m_nItemID,
															 ZApplication::GetGameInterface()->GetQuestItemIcon( pItemDesc->m_nItemID, true),
															 pItemDesc->m_szQuestItemName,
															 nCount,
															 pItemDesc->m_szDesc));
			}
		}
	}

	MWidget* pWidget = pResource->FindWidget( "Stage_NoItemLabel");
	if ( pWidget)
	{
		if ( pListBox->GetCount() > 0)
			pWidget->Show( false);
		else
			pWidget->Show( true);
	}

	pListBox->SetStartItem( nStartIndex);
	pListBox->SetSelIndex( min( (pListBox->GetCount() - 1), nSelectIndex));
}


/***********************************************************************
  OnDropSacrificeItem : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｳ�ｱ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDropSacrificeItem( int nSlotNum)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pReadyBtn = (MButton*)pResource->FindWidget("StageReady");
	if(pReadyBtn) 
		if(pReadyBtn->GetCheck()) 
		{	// ﾁﾘｺ・ｿﾏｷ盪�ﾆｰﾀﾌ ｴｭｷﾁﾀﾖｴﾙｸ・
			ZApplication::GetStageInterface()->ChangeStageEnableReady(pReadyBtn->GetCheck());	// ｴﾙｸ･ UIｸｦ ｲｨﾁﾘｴﾙ.
			return;	// ﾈ�ｻ�ｾﾆﾀﾌﾅﾛﾀｻ ｽｽｷﾔｿ｡ ｿﾃｸｮﾁ・ｾﾊｴﾂｴﾙ.
		}

	MListBox* pListBox = (MListBox*)pResource->FindWidget( "Stage_SacrificeItemListbox");
	if ( !pListBox || (pListBox->GetSelIndex() < 0))
		return;

	SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->Get( pListBox->GetSelIndex());
	if ( pItemDesc)
	{
		MTextArea* pDesc = (MTextArea*)pResource->FindWidget( "Stage_ItemDesc");

		// ｽｽｷﾔﾀﾌ ｺ�ｾ�ﾜﾖﾀｸｸ・ｹｫﾁｶｰﾇ ｿﾃｸｲ
		if ( ! m_SacrificeItem[ nSlotNum].IsExist())
		{
			ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());
			char szText[256];
			sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
			pDesc->SetText(szText);
			//if ( pDesc)
			//	pDesc->Clear();
		}

		// ｽｽｷﾔﾀﾌ ｺ�ｾ�ﾜﾖﾁ・ｾﾊﾀｸｸ・..
		else
		{
			if ( (m_SacrificeItem[ nSlotNum].GetUID()    != ZGetGameClient()->GetPlayerUID()) ||
				 (m_SacrificeItem[ nSlotNum].GetItemID() != pItemDesc->GetItemID()))
				ZPostRequestDropSacrificeItem( ZGetGameClient()->GetPlayerUID(), nSlotNum, pItemDesc->GetItemID());

			char szText[256];
			sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
			pDesc->SetText(szText);
			//if ( pDesc)
			//	pDesc->Clear();
		}
	}
}


/***********************************************************************
  OnRemoveSacrificeItem : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｻｩｱ・
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnRemoveSacrificeItem( int nSlotNum)
{
	if ( !m_SacrificeItem[ nSlotNum].IsExist())
		return;

	ZPostRequestCallbackSacrificeItem( ZGetGameClient()->GetPlayerUID(),
									   nSlotNum,
									   m_SacrificeItem[ nSlotNum].GetItemID());

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	char szText[256];
	sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
	pDesc->SetText(szText);
	//if ( pDesc)
	//	pDesc->Clear();
}


/***********************************************************************
  MSacrificeItemListBoxListener
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｸｮｽｺﾆｮ ｹﾚｽｺ ｸｮｽｺｳﾊ
************************************************************************/
class MSacrificeItemListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)pWidget;

			// ｾﾆﾀﾌﾅﾛ ｼｳｸ・ｾ�ｵ･ﾀﾌﾆｮ
			SacrificeItemListBoxItem* pItemDesc = (SacrificeItemListBoxItem*)pListBox->GetSelItem();
			MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
			if ( pItemDesc && pDesc)
			{
				char szCount[ 128];
				sprintf( szCount, "%s : %d", ZMsg( MSG_WORD_QUANTITY), pItemDesc->GetItemCount());
				pDesc->SetTextColor( MCOLOR( 0xFFD0D0D0));
				pDesc->SetText( szCount);
				pDesc->AddText( "\n");
				pDesc->AddText( pItemDesc->GetItemDesc(), 0xFF808080);
			}

			return true;
		}


		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			// Put item
			if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 0].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 0);
			else if ( !ZApplication::GetStageInterface()->m_SacrificeItem[ 1].IsExist())
				ZApplication::GetStageInterface()->OnDropSacrificeItem( 1);
		
			return true;
		}

		return false;
	}
};

MSacrificeItemListBoxListener g_SacrificeItemListBoxListener;

MListener* ZGetSacrificeItemListBoxListener( void)
{
	return &g_SacrificeItemListBoxListener;
}


/***********************************************************************
  OnDropCallbackRemoveSacrificeItem
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ﾁｦｰﾅ
************************************************************************/
void OnDropCallbackRemoveSacrificeItem( void* pSelf, MWidget* pSender, MBitmap* pBitmap, const char* szString, const char* szItemString)
{
	if ( (pSender == NULL) || (strcmp(pSender->GetClassName(), MINT_ITEMSLOTVIEW)))
		return;

	ZItemSlotView* pItemSlotView = (ZItemSlotView*)pSender;
	ZApplication::GetStageInterface()->OnRemoveSacrificeItem( (strcmp( pItemSlotView->m_szItemSlotPlace, "SACRIFICE0") == 0) ? 0 : 1);
}


// ｸｱｷｹﾀﾌｸﾊ ﾀﾎﾅﾍﾆ菎ﾌｽｺ
/***********************************************************************
OpenRelayMapBox : public

desc : ｸｱｷｹﾀﾌｸﾊ ｼｱﾅﾃ ﾃ｢ ｿｭｱ・
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::OpenRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( true);

	SetRelayMapBoxPos(2);

	// ｸｮﾇﾃｷｹﾀﾌ ｹﾚｽｺｿ｡ ｸﾊｸｮｽｺﾆｮｸｦ ｻ�ｼｺ
	ZApplication::GetStageInterface()->RelayMapCreateMapList();
}

/***********************************************************************
CloseRelayMapBox : public

desc : ｸｱｷｹﾀﾌｸﾊ ｼｱﾅﾃ ﾃ｢ ｴﾝｱ・
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::CloseRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	SetRelayMapBoxPos(1);
}

/***********************************************************************
HideRelayMapBox : public

desc : ｸｱｷｹﾀﾌｸﾊ ｼｱﾅﾃ ﾃ｢ ｰｨﾃﾟｱ・
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::HideRelayMapBox( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxClose");
	if ( pButton)
		pButton->Show( false);
	pButton = (MButton*)pResource->FindWidget( "Stage_RelayMapBoxOpen");
	if ( pButton)
		pButton->Show( true);

	MWidget* pWidget = pResource->FindWidget( "Stage_CharacterInfo");
	if ( pWidget)
		pWidget->Enable( true);

	SetRelayMapBoxPos(0);
}

/***********************************************************************
GetRelayMapBoxPos : public

desc : ｸｱｷｹﾀﾌｸﾊ ｼｱﾅﾃ ﾃ｢ ﾀｧﾄ｡ ｱｸﾇﾏｱ・
arg  : none
ret  : none
************************************************************************/

void ZStageInterface::SetRelayMapBoxPos( int nBoxPos)
{
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapListView");
	if ( pWidget)
	{
		MRECT rect;

		switch ( nBoxPos)
		{
		case 0 :		// Hide
			rect = pWidget->GetRect();
			m_nRelayMapListFramePos = -rect.w;
			break;

		case 1 :		// Close
			rect = pWidget->GetRect();
			m_nRelayMapListFramePos = -rect.w + ( rect.w * 0.14);
			break;

		case 2 :		// Open
			m_nRelayMapListFramePos = 0;
			break;
		}
	}
}

/***********************************************************************
PostRelayMapType : public

desc : ｸｱｷｹﾀﾌｸﾊ ﾅｸﾀﾔ ｾ�ｵ･ﾀﾌﾆｮ ｹﾗ ｼｭｹ�ｿ｡ ﾀ・ﾛ
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::PostRelayMapElementUpdate( void)
{
	MComboBox* pCBRelayMapType = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapType" );
	if ( !pCBRelayMapType) return;
	MComboBox* pCBRelayMapTurnCount = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapRepeatCount" );
	if ( !pCBRelayMapTurnCount) return;
	ZPostStageRelayMapElementUpdate(ZGetGameClient()->GetStageUID(), pCBRelayMapType->GetSelIndex(), pCBRelayMapTurnCount->GetSelIndex());
}

/***********************************************************************
PostRelayMapListUpdate : public

desc : ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ｾ�ｵ･ﾀﾌﾆｮ(ｼｭｹ�ｿ｡ｵｵ ｸｮｽｺﾆｮｸｦ ｺｸｳｽｴﾙ.)
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::PostRelayMapInfoUpdate( void)
{
	MComboBox* pCBRelayMapType = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapType" );
	if ( !pCBRelayMapType) return;
	MComboBox* pCBRelayMapRepeatCount = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget( "Stage_RelayMapRepeatCount" );
	if ( !pCBRelayMapRepeatCount) return;

	MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelayMapListBox == NULL) return;

	if(pRelayMapListBox->GetCount() <= 0)	
	{
		ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_ONE_OR_MORE_MAP_SELECT);
		return;
	}
	
	//ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ﾀ・ﾛ
	void* pMapArray = MMakeBlobArray(sizeof(MTD_RelayMap), pRelayMapListBox->GetCount());
	int nMakeBlobCnt = 0;
	for(int i=0; i<pRelayMapListBox->GetCount(); i++)
	{
		MTD_RelayMap* pRelayMapNode = (MTD_RelayMap*)MGetBlobArrayElement(pMapArray, nMakeBlobCnt);
		for (int j = 0; j < MMATCH_MAP_COUNT; j++)
		{
			if(0 == strcmp(pRelayMapListBox->GetString(i), (char*)MGetMapDescMgr()->GetMapName(j)))
			{
				pRelayMapNode->nMapID = j;
				break;
			}
		}
		++nMakeBlobCnt;
	}

	ZPostStageRelayMapInfoUpdate(ZGetGameClient()->GetStageUID(), pCBRelayMapType->GetSelIndex(), pCBRelayMapRepeatCount->GetSelIndex(), pMapArray);
	MEraseBlobArray(pMapArray);

	ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(true);
	ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", false);
}

/***********************************************************************
PostRelayMapList : public

desc : ｸｱｷｹﾀﾌｸﾊ ｼｱﾅﾃﾀｸｷﾎ ｸﾊ ﾄﾞｺｸｹﾚｽｺ ｸｮｽｺﾆｮｸｦ ｸｸｵ鮴・ﾁﾘｴﾙ.
arg  : none
ret  : none
************************************************************************/
void ZStageInterface::RelayMapCreateMapList()
{
	MComboBox* pCombo = (MComboBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("MapSelection");
	if(pCombo == NULL) return;

	// ｸﾊ ｸｮｽｺﾆｮ ｸｸｵ鮴・ﾁﾖｱ・
	MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_MapListbox");
	if(pMapListBox == NULL) return;

	pMapListBox->RemoveAll();	// ｱ簔ｸ ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮｸｦ ｸ�ｵﾎ ﾁ�ｿ�ﾁﾘｴﾙ.
	for( int i = 0 ; i < pCombo->GetCount(); ++i )
	{
		if(strcmp(MMATCH_MAPNAME_RELAYMAP, pCombo->GetString(i)) == 0)
			continue;
		RelayMapList* pRelayMapList = new RelayMapList( pCombo->GetString(i), MBitmapManager::Get( "Mark_Arrow.png"));
		pMapListBox->Add( pRelayMapList);
	}


	// ｹ貘蠡ﾌ ｸｱｷｹﾀﾌｸﾊ ｼｼﾆﾃﾁﾟﾀﾌｸ・ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮｸｦ ｾ�ｵ･ﾀﾌﾆｮ ﾇﾏﾁ・ｾﾊｴﾂｴﾙ.
	if(!ZApplication::GetStageInterface()->GetIsRelayMapRegisterComplete())
		return;

	// ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ｸｸｵ鮴・ﾁﾖｱ・
	MListBox* pRelaMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
	if(pRelaMapListBox == NULL) return;
	RelayMap arrayRelayMapList[MAX_RELAYMAP_LIST_COUNT];
	memcpy(arrayRelayMapList, ZGetGameClient()->GetMatchStageSetting()->GetRelayMapList(), sizeof(RelayMap)*MAX_RELAYMAP_LIST_COUNT);
	
	pRelaMapListBox->RemoveAll();	// ｱ簔ｸ ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮｸｦ ｸ�ｵﾎ ﾁ�ｿ�ﾁﾘｴﾙ.
	for( int i = 0 ; i < ZGetGameClient()->GetMatchStageSetting()->GetRelayMapListCount(); ++i )
	{// ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮｿ｡ ｵ･ﾀﾌﾅﾍｸｦ ﾃﾟｰ｡ﾇﾘﾁﾘｴﾙ.
		int nMapID = arrayRelayMapList[i].nMapID;
		RelayMapList* pRelayMapList = new RelayMapList( MGetMapDescMgr()->GetMapName(MGetMapDescMgr()->GetMapID(nMapID)), MBitmapManager::Get( "Mark_X.png"));
		pRelaMapListBox->Add( pRelayMapList);
	}
}

/***********************************************************************
MMapListBoxListener

desc : ｸﾊ ｸｮｽｺﾆｮ ｹﾚｽｺ ｸｮｽｺｳﾊ
************************************************************************/
class MMapListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// ｽｺﾅﾗﾀﾌﾁ・ｸｶｽｺﾅﾍｰ｡ ｾﾆｴﾏｸ・ｸﾊ ｸｮｽｺﾆｮｸｦ ﾄﾁﾆｮｷﾑ ﾇﾒｼ�ｾ�ｴﾙ.
		if(!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			// ｸﾊｸｮｽｺﾆｮｿ｡ｼｭ ｼｱﾅﾃｵﾈ ｸﾊﾁ､ｺｸｸｦ ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮｿ｡ ﾃﾟｰ｡ﾇﾑｴﾙ.
			MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_MapListbox");
			if(pMapListBox == NULL) return false;
			MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
			if(pRelayMapListBox == NULL) return false;
			if(MAX_RELAYMAP_LIST_COUNT <= pRelayMapListBox->GetCount()) 
			{
				ZApplication::GetGameInterface()->ShowMessage( MSG_GAME_RELAYMAP_TOO_MANY_LIST_COUNT );
				return false;
			}

			RelayMapList* pMapList = (RelayMapList*)pMapListBox->GetSelItem();
			char szMapName[ MAPNAME_LENGTH ];
			strcpy(szMapName, pMapList->GetString());
			RelayMapList* pRelayMapList = new RelayMapList( szMapName, MBitmapManager::Get( "Mark_X.png"));

			pRelayMapListBox->Add( pRelayMapList);
			pRelayMapListBox->ShowItem(pRelayMapListBox->GetCount());	// ｽｺﾅｩｷﾑ ﾀｧﾄ｡ ｼｼﾆﾃ

			// ｸｱｷｹﾀﾌ ｸﾊ ｼ�ﾁ､ ｽﾃﾀﾛ
			ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(false);
			ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", true);

			return true;
		}

		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}

		return false;
	}
};
MMapListBoxListener g_MapListBoxListener;
MListener* ZGetMapListBoxListener( void)
{
	return &g_MapListBoxListener;
}

/***********************************************************************
MRelayMapListBoxListener

desc : ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ｹﾚｽｺ ｸｮｽｺｳﾊ
************************************************************************/
class MRelayMapListBoxListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// ｽｺﾅﾗﾀﾌﾁ・ｸｶｽｺﾅﾍｰ｡ ｾﾆｴﾏｸ・ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮｸｦ ﾄﾁﾆｮｷﾑ ﾇﾒｼ�ｾ�ｴﾙ.
		if(!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if ( MWidget::IsMsg( szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pRelayMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_RelayMapListbox");
			if(pRelayMapListBox == NULL) return false;
			if(pRelayMapListBox->GetCount() > 1)
			{	// ｸﾊﾀﾌ ﾃﾖｼﾒﾇﾑ ﾇﾑｰｳｶ�ｵｵ ﾀﾖｾ�ﾚﾟ ﾇﾑｴﾙ.
				pRelayMapListBox->Remove(pRelayMapListBox->GetSelIndex());

				// ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮ ｽｺﾅｩｷﾑ ﾀｧﾄ｡ ｼｼﾆﾃ
				if(pRelayMapListBox->GetCount() <= pRelayMapListBox->GetShowItemCount())
					pRelayMapListBox->ShowItem(0);
				else
					pRelayMapListBox->SetStartItem(pRelayMapListBox->GetCount());
				
				// ｸｱｷｹﾀﾌ ｸﾊ ｼ�ﾁ､ ｽﾃﾀﾛ
				ZApplication::GetStageInterface()->SetIsRelayMapRegisterComplete(false);
				ZApplication::GetGameInterface()->EnableWidget( "Stage_RelayMap_OK_Button", true);

				return true;
			}
			else
			{
				ZApplication::GetGameInterface()->ShowMessage(MSG_GAME_RELAYMAP_ONE_OR_MORE_MAP_SELECT);
				return false;
			}
		}

		// On double click
		else if ( MWidget::IsMsg( szMessage, MLB_ITEM_DBLCLK) == true)
		{
			return true;
		}
		return false;
	}
};
MRelayMapListBoxListener g_RelayMapListBoxListener;
MListener* ZGetRelayMapListBoxListener( void)
{
	return &g_RelayMapListBoxListener;
}

/***********************************************************************
  StartMovieOfQuest : public
  
  desc : ﾄ�ｽｺﾆｮ ｸ�ｵ蟾ﾎ ｽﾃﾀﾛﾇﾒｶｧ ｾﾆﾀﾌﾅﾛ ﾇﾕﾃﾄﾁ�ｴﾂ ｹｫｺ�ｸｦ ｽﾃﾀﾛﾇﾔ
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::StartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	m_dwClockOfStartMovie = timeGetTime();

	// ﾈｭｿｰ ｾﾖｴﾏｸﾞﾀﾌｼﾇ ｽﾃﾀﾛ
	MAnimation* pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame0");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}
	pAnimation = (MAnimation*)pResource->FindWidget( "Stage_Flame1");
	if ( pAnimation && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
	{
		pAnimation->SetCurrentFrame( 0);
		pAnimation->Show( true);
		pAnimation->SetRunAnimation( true);
	}

	m_bDrawStartMovieOfQuest = true;
}


/***********************************************************************
  OnDrawStartMovieOfQuest : public
  
  desc : ﾄ�ｽｺﾆｮ ｸ�ｵ蟾ﾎ ｽﾃﾀﾛﾇﾒｶｧ ｾﾆﾀﾌﾅﾛ ﾇﾕﾃﾄﾁ�ｴﾂ ｹｫｺ�ｸｦ ｱﾗｸｲ
  arg  : none
  ret  : none
************************************************************************/
void ZStageInterface::OnDrawStartMovieOfQuest( void)
{
	if ( !m_bDrawStartMovieOfQuest)
		return ;

	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	// ｰ豌・ｽﾃｰ｣ﾀｻ ｱｸﾇﾑｴﾙ.
	DWORD dwClock = timeGetTime() - m_dwClockOfStartMovie;

	// ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ﾆ菎ﾌｵ・ｾﾆｿ・
	int nOpacity = 255 - dwClock * 0.12f;
	if ( nOpacity < 0)
		nOpacity = 0;

	MPicture* pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage0");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist())
		pPicture->SetOpacity( nOpacity);

	pPicture = (MPicture*)pResource->FindWidget( "Stage_SacrificeItemImage1");
	if ( pPicture && m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
		pPicture->SetOpacity( nOpacity);

	// ﾁｾｷ・ｽﾃｰ｣ﾀﾏ ｰ豼・｡...
	if ( dwClock > 3200)
	{
		m_bDrawStartMovieOfQuest = false;

		ZMyQuestItemMap::iterator itMyQItem;

		// ｹ�ｱﾗﾇﾘｰ・.. ﾄ�ｽｺﾆｮ ｰﾔﾀﾓｽﾃﾀﾛﾀｻ ｴｩｸ｣ｴﾂ ｼ�ｰ｣ ﾇﾘｴ・ﾄ�ｽｺﾆｮ ｾﾆﾀﾌﾅﾛﾀﾌ ｰｨｼﾒｵﾈ ｻ�ﾅﾂ(ｿﾅｹﾙｸ･)ﾀﾇ ｰｹｼ�ｸｦ 
		// DBｿ｡ｼｭ ｹﾞｾﾆｿﾀｱ筝ｧｹｮｿ｡ ﾄｫｿ�箘 ｰｨｼﾒｸｦ ﾇﾏｸ・ﾀﾟｸ�ｵﾈｰｪﾀﾌ ｵﾈｴﾙ....20090318 by kammir
		// ｿｩｱ箴ｭ ｽｽｷﾔｿ｡ ﾀﾚｽﾅﾀﾇ ｾﾆﾀﾌﾅﾛﾀﾌ ｿﾃｷﾁﾁｮ ﾀﾖﾀｸｸ・ﾇﾘｴ・ｾﾆﾀﾌﾅﾛ ﾄｫｿ�箘 ｰｨｼﾒ.
		//if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetUID() )
		//{
		//	itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT0].GetItemID() );
		//	itMyQItem->second->Decrease();
		//}
		//if( ZGetGameClient()->GetUID() == m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetUID() )
		//{
		//	itMyQItem = ZGetMyInfo()->GetItemList()->GetQuestItemMap().find( m_SacrificeItem[ SACRIFICEITEM_SLOT1].GetItemID() );
		//	itMyQItem->second->Decrease();
		//}
		
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

		ZApplication::GetGameInterface()->SetState( GUNZ_GAME);
	}
}


/***********************************************************************
  IsShowStartMovieOfQuest : public
  
  desc : ﾄ�ｽｺﾆｮ ｸ�ｵ蟾ﾎ ｽﾃﾀﾛﾇﾒｶｧ ｾﾆﾀﾌﾅﾛ ﾇﾕﾃﾄﾁ�ｴﾂ ｹｫｺ�ｸｦ ｺｸｿｩﾁﾙﾁ・ｿｩｺﾎｸｦ ｰ眞､.
  arg  : none
  ret  : true(=Quest mode start movie) or false(=none)
************************************************************************/
bool ZStageInterface::IsShowStartMovieOfQuest( void)
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	if ( m_nGameType == MMATCH_GAMETYPE_QUEST)
	{
		if ( m_SacrificeItem[ SACRIFICEITEM_SLOT0].IsExist() || m_SacrificeItem[ SACRIFICEITEM_SLOT1].IsExist())
			return true;
	}

	return false;
}


/***********************************************************************
  OnResponseDropSacrificeItemOnSlot : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛﾀﾌ ｿﾃｶ�ｰｬﾀｻｶｧ
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseDropSacrificeItemOnSlot( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID, false);

		m_SacrificeItem[ nSlotIndex].SetSacrificeItemSlot( uidRequester, nItemID, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();
	}
	else if( ITEM_TYPE_NOT_SACRIFICE == nResult)
	{
		// ﾈ�ｻ� ｾﾆﾀﾌﾅﾛﾀﾌ ｾﾆｴﾔ.
		return false;
	}
	else if( NEED_MORE_QUEST_ITEM == nResult )
	{
		// ﾇ�ﾁｦ ｰ｡ﾁ�ｰ・ﾀﾖｴﾂ ｼ�ｷｮﾀｻ ﾃﾊｰ�ﾇﾘｼｭ ｿﾃｷﾁ ｳ�ﾀｸｷﾁｰ・ﾇﾟﾀｻｰ豼・
	}
	else if( MOK != nResult )
	{
		// ｽﾇﾆﾐ...
		return false;
	}
	else
	{
		// ﾁ､ﾀﾇｵﾇﾁ・ｾﾊﾀｺ error...
		ASSERT( 0 );
	}

#endif

	return true;
}


/***********************************************************************
  OnResponseCallbackSacrificeItem : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛﾀﾌ ｳｻｷﾁｰｬﾀｻｶｧ
  arg  : none
  ret  : none
************************************************************************/
bool ZStageInterface::OnResponseCallbackSacrificeItem( const int nResult, const MUID& uidRequester, const int nSlotIndex, const int nItemID )
{
#ifdef _QUEST_ITEM
	if( MOK == nResult )
	{
		m_SacrificeItem[ nSlotIndex].RemoveItem();
		SerializeSacrificeItemListBox();

		UpdateSacrificeItem();

		MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
		char szText[256];
		sprintf(szText, ZMsg( MSG_QUESTITEM_USE_DESCRIPTION ));
		pDesc->SetText(szText);
		//if ( pDesc)
		//	pDesc->Clear();
	}
	else if( ERR_SACRIFICE_ITEM_INFO == nResult )
	{
		// ﾅｬｶ�ﾀﾌｾ�ﾆｮｿ｡ｼｭ ｺｸｳｽ ﾁ､ｺｸｰ｡ ﾀﾟｸ�ｵﾈ ﾁ､ｺｸ. ｵ�ｷｯ ｿ｡ｷｯﾃｳｸｮｰ｡ ﾇﾊｿ萇ﾏｸ・ｿｩｱ箴ｭ ﾇﾘﾁﾖｸ・ｵﾊ.
	}

#endif

	return true;
}

#ifdef _QUEST_ITEM
///
// Fist : ﾃﾟｱｳｼｺ.
// Last : ﾃﾟｱｳｼｺ.
// 
// ｼｭｹ�ｷﾎｺﾎﾅﾍ QLﾀﾇ ﾁ､ｺｸｸｦ ｹﾞﾀｽ.
///
bool ZStageInterface::OnResponseQL( const int nQL )
{
	ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL);

	// ｽｺﾅﾗﾀﾌﾁ・ｿｵｿｪｿ｡ ﾀﾖｴﾂ ﾄ�ｽｺﾆｮ ｷｹｺｧ ﾇ･ｽﾃ ｼ�ﾁ､
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}

	return true;
}

bool ZStageInterface::OnStageGameInfo( const int nQL, const int nMapsetID, const unsigned int nScenarioID )
{
	if (nScenarioID != 0)
	{
		ZGetQuest()->GetGameInfo()->SetQuestLevel( nQL );
	}
	else
	{
		// ｽﾃｳｪｸｮｿﾀｰ｡ ｾ�ﾀｸｸ・ｱﾗｳﾉ 0ﾀｸｷﾎ ｺｸﾀﾌｰﾔ ﾇﾑｴﾙ.
		ZGetQuest()->GetGameInfo()->SetQuestLevel( 0 );
	}

	ZGetQuest()->GetGameInfo()->SetMapsetID( nMapsetID );
	ZGetQuest()->GetGameInfo()->SetSenarioID( nScenarioID );

	UpdateStageGameInfo(nQL, nMapsetID, nScenarioID);

	return true;
}

bool ZStageInterface::OnResponseSacrificeSlotInfo( const MUID& uidOwner1, const unsigned long int nItemID1, 
												   const MUID& uidOwner2, const unsigned long int nItemID2 )
{
	if ( (uidOwner1 != MUID(0,0)) && nItemID1)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID1);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID1, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].SetSacrificeItemSlot( uidOwner1, nItemID1, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT0].RemoveItem();

	if ( (uidOwner2 != MUID(0,0)) && nItemID2)
	{
		MQuestItemDesc* pItemDesc = GetQuestItemDescMgr().FindQItemDesc( nItemID2);
		MBitmap* pIconBitmap = ZApplication::GetGameInterface()->GetQuestItemIcon( nItemID2, false);
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].SetSacrificeItemSlot( uidOwner2, nItemID2, pIconBitmap, pItemDesc->m_szQuestItemName, pItemDesc->m_nLevel);
	}
	else
		m_SacrificeItem[ SACRIFICEITEM_SLOT1].RemoveItem();

	UpdateSacrificeItem();

	MTextArea* pDesc = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_ItemDesc");
	//if ( pDesc)
	//	pDesc->Clear();

	return true;
}


bool ZStageInterface::OnQuestStartFailed( const int nState )
{
	MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
	if ( pTextArea)
	{
		char text[256];
		sprintf(text, "%s", ZMsg(MSG_GANE_NO_QUEST_SCENARIO));
		pTextArea->AddText( text);
	}

/*
	if( MSQITRES_INV == nState )
	{
		// ﾇﾘｴ・QLｿ｡ｴ・ﾑ ﾈ�ｻ�ｾﾆﾀﾌﾅﾛ ﾁ､ｺｸ ﾅﾗﾀﾌｺ�ﾜﾌ ｾ�ﾀｽ. ﾀﾌｰ豼・ﾂ ｸﾂﾁ・ｾﾊｴﾂ ﾈ�ｻ� ｾﾆﾀﾌﾅﾛﾀﾌ ｿﾃｷﾁﾁｮ ﾀﾖﾀｻｰ豼・
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1ﾇ�ﾀ・ｳ�ｿｩﾀﾖｴﾂ ｾﾆﾀﾌﾅﾛﾀｺ ﾁｶｰﾇｿ｡ ｸﾂﾁ・ｾﾊｾﾆ ｰﾔﾀﾓﾀｻ ｽﾃﾀﾛﾇﾒ ｼ・ｾ�ｽﾀｴﾏｴﾙ.");
	}
	else if( MSQITRES_DUP == nState )
	{
		// ｾ酊ﾊ ｽｽｷﾔｿ｡ ｰｰﾀｺ ｾﾆﾀﾌﾅﾛﾀﾌ ｿﾃｷﾁﾁｮ ﾀﾖﾀｽ.
		MTextArea* pTextArea = (MTextArea*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "StageChattingOutput");
		if ( pTextArea)
			pTextArea->AddText( "^1ｰｰﾀｺ ｾﾆﾀﾌﾅﾛ 2ｰｳｰ｡ ｳ�ｿｩﾀﾖﾀｸｹﾇｷﾎ ｰﾔﾀﾓﾀｻ ｽﾃﾀﾛﾇﾒ ｼ・ｾ�ｽﾀｴﾏｴﾙ.");
	}
*/
	return true;
}


bool ZStageInterface::OnNotAllReady()
{
	return true;
}
#endif

void ZStageInterface::UpdateStageGameInfo(const int nQL, const int nMapsetID, const int nScenarioID)
{
	if (!ZGetGameTypeManager()->IsQuestOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())) return;

	// ｽｺﾅﾗﾀﾌﾁ・ｿｵｿｪｿ｡ ﾀﾖｴﾂ ﾄ�ｽｺﾆｮ ｷｹｺｧ ﾇ･ｽﾃ ｼ�ﾁ､
	MLabel* pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_QuestLevel");
	if ( pLabel)
	{
		char szText[125];
		sprintf( szText, "%s %s : %d", ZMsg( MSG_WORD_QUEST), ZMsg( MSG_CHARINFO_LEVEL), nQL);
		pLabel->SetText( szText);
	}


#define		MAPSET_NORMAL		MCOLOR(0xFFFFFFFF)
#define		MAPSET_SPECIAL		MCOLOR(0xFFFFFF40)			// Green
//#define		MAPSET_SPECIAL		MCOLOR(0xFFFF2020)		// Red

	// ｿｩｱ箴ｭ ｽﾃｳｪｸｮｿﾀ ﾀﾌｸｧﾀｻ ｺｸｿｩﾁﾘｴﾙ.
	pLabel = (MLabel*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioName");
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_SenarioNameImg");
	MPicture* pPictureL = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights0");
	MPicture* pPictureR = (MPicture*)ZApplication::GetGameInterface()->GetIDLResource()->FindWidget( "Stage_Lights1");
	if ( pLabel)
	{
		if (nScenarioID == 0)
		{
			// ｽﾃｳｪｸｮｿﾀｰ｡ ｾ�ｴﾂ ｰ豼・
			pLabel->SetText( "");
			if ( pWidget)
				pWidget->Show( false);
			if ( pPictureL) {
				pPictureL->Show( false);
			}
			if ( pPictureR) {
				pPictureR->Show( false);
			}
		}
		else
		{
			pLabel->SetAlignment( MAM_HCENTER | MAM_VCENTER);

			LIST_SCENARIONAME::iterator itr =  m_SenarioDesc.find( nScenarioID);
			if ( itr != m_SenarioDesc.end())
			{
				pLabel->SetText( (*itr).second.m_szName);
				pLabel->SetTextColor(MCOLOR(0xFFFFFF00));
				if ( pWidget)
					pWidget->Show( true);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_SPECIAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_SPECIAL);
				}
			}
			else
			{
				// ﾆｯｺｰｽﾃｳｪｸｮｿﾀｰ｡ ｾ�ﾀｻｰ豼・ﾂ ﾁ､ｱﾔｽﾃｳｪｸｮｿﾀﾀﾌｴﾙ.
				pLabel->SetText("");
				pLabel->SetTextColor(MCOLOR(0xFFFFFFFF));
				if ( pWidget)
					pWidget->Show( false);

				if ( pPictureL) {
					pPictureL->Show( true);
					pPictureL->SetBitmapColor(MAPSET_NORMAL);
				}
				if ( pPictureR) {
					pPictureR->Show( true);
					pPictureR->SetBitmapColor(MAPSET_NORMAL);
				}

			}
		}
	}
}

/***********************************************************************
  SetSacrificeItemSlot : public
  
  desc : ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ ｽｽｷﾔｿ｡ ｾﾆﾀﾌﾅﾛ ﾁ､ｺｸｸｦ ﾀﾔｷﾂ
  arg  : none
  ret  : none
************************************************************************/
void SacrificeItemSlotDesc::SetSacrificeItemSlot( const MUID& uidUserID, const unsigned long int nItemID, MBitmap* pBitmap, const char* szItemName, const int nQL)
{
	m_uidUserID = uidUserID;
	m_nItemID = nItemID;
	m_pIconBitmap = pBitmap;
	strcpy( m_szItemName, szItemName);
	m_nQL = nQL;
	m_bExist = true;
}


/***********************************************************************
  ReadSenarioNameXML : protected
  
  desc : ﾄ�ｽｺﾆｮ ﾈ�ｻ� ｾﾆﾀﾌﾅﾛ XMLﾀｻ ﾀﾐｴﾂｴﾙ
  arg  : none
  ret  : true(=success) or false(=fail)
************************************************************************/
bool ZStageInterface::ReadSenarioNameXML( void)
{
	if ( (int)m_SenarioDesc.size())
		return true;


	// XML ﾆﾄﾀﾏﾀｻ ｿｬｴﾙ
	MXmlDocument xmlQuestItemDesc;
	xmlQuestItemDesc.Create();

	char			*buffer;
	MZFile			mzFile;

	string strFileScenario("System/scenario.xml");
#ifndef _DEBUG
	strFileScenario += ""; // MEF NULL
#endif
	if( !mzFile.Open(strFileScenario.c_str(), ZApplication::GetFileSystem())) 
	{
		mlog("Error while read : %s", strFileScenario.c_str());
		xmlQuestItemDesc.Destroy();
		return false;
	} 

	buffer = new char[ mzFile.GetLength() + 1];
	buffer[ mzFile.GetLength()] = 0;

	mzFile.Read( buffer, mzFile.GetLength());

	if( !xmlQuestItemDesc.LoadFromMemory( buffer))
	{
		delete[] buffer;
		xmlQuestItemDesc.Destroy();
		return false;
	}
	delete[] buffer;
	mzFile.Close();

	int nStdScenarioCount = 1000;

	// ｵ･ﾀﾌﾅﾍｸｦ ﾀﾐｾ�ﾛﾂｴﾙ
	MXmlElement rootElement = xmlQuestItemDesc.GetDocumentElement();
	for ( int i = 0;  i < rootElement.GetChildNodeCount();  i++)
	{
		MXmlElement chrElement = rootElement.GetChildNode( i);

		char szTagName[ 256];
		chrElement.GetTagName( szTagName);

		if ( szTagName[ 0] == '#')
			continue;


		char szAttrName[64];
		char szAttrValue[256];
		int nItemID = 0;
		MSenarioList SenarioMapList;

		bool bFind = false;


		// ﾁ､ｱﾔ ｽﾃｳｪｸｮｿﾀ
		if ( !stricmp( szTagName, "STANDARD_SCENARIO"))			// ﾅﾂｱﾗ ｽﾃﾀﾛ
		{
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				SenarioMapList.m_ScenarioType = ST_STANDARD;	// Type

				if ( !stricmp( szAttrName, "title"))			// Title
					strcpy( SenarioMapList.m_szName, szAttrValue);

				else if ( !stricmp( szAttrName, "mapset"))		// Map set
					strcpy( SenarioMapList.m_szMapSet, szAttrValue);
			}

			nItemID = nStdScenarioCount++;					// ID

			bFind = true;
		}

		// ﾆｯｼ・ｽﾃｳｪｸｮｿﾀ
		else if ( !stricmp( szTagName, "SPECIAL_SCENARIO"))			// ﾅﾂｱﾗ ｽﾃﾀﾛ
		{
			// Set Tag
			for ( int k = 0;  k < chrElement.GetAttributeCount();  k++)
			{
				chrElement.GetAttribute( k, szAttrName, szAttrValue);

				SenarioMapList.m_ScenarioType = ST_SPECIAL;		// Type

				if ( !stricmp( szAttrName, "id"))				// ID
					nItemID = atoi( szAttrValue);

				else if ( !stricmp( szAttrName, "title"))		// Title
					strcpy( SenarioMapList.m_szName, szAttrValue);

				else if ( !stricmp( szAttrName, "mapset"))		// Map set
					strcpy( SenarioMapList.m_szMapSet, szAttrValue);
			}

			bFind = true;
		}


		if ( bFind)
			m_SenarioDesc.insert( LIST_SCENARIONAME::value_type( nItemID, SenarioMapList));
	}

	xmlQuestItemDesc.Destroy();


	return true;
}

bool ZStageInterface::OnStopVote()
{
	ZGetGameClient()->SetVoteInProgress( false );
	ZGetGameClient()->SetCanVote( false );

#ifdef _DEBUG
	string str = ZMsg(MSG_VOTE_VOTE_STOP);
#endif

	ZChatOutput(ZMsg(MSG_VOTE_VOTE_STOP), ZChat::CMT_SYSTEM, ZChat::CL_CURRENT);
	return true;
}


void ZStageInterface::OnStartFail( const int nType, const MUID& uidParam )
{
	if( ALL_PLAYER_NOT_READY == nType )
	{
		// ｸ�ｵ・ﾀｯﾀ�ｰ｡ ｷｹｵ�ｸｦ ﾇﾏﾁ・ｾﾊｾﾒﾀｽ.
		ZGetGameInterface()->PlayVoiceSound( VOICE_PLAYER_NOT_READY, 1500);
	}
	else if( QUEST_START_FAILED_BY_SACRIFICE_SLOT == nType )
	{
		OnQuestStartFailed( uidParam.Low );

		ZGetGameInterface()->PlayVoiceSound( VOICE_QUEST_START_FAIL, 2800);
	}
	else if( INVALID_TACKET_USER == nType )
	{
		// ﾀﾚｱ・ﾀﾚｽﾅﾀﾌｸ・-_-;;
		//if( uidParam == ZGetMyUID() )
		//{
			char szMsg[ 128 ];

			sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), ZGetGameClient()->GetObjName(uidParam).c_str() );

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
		//}

		// ﾀﾔﾀ螻ﾇ ｴｩｰ｡ ｾ�ｴﾂｰﾅｾﾟ? -_-+ - by SungE
		//const MMatchPeerInfo* pPeer = ZGetGameClient()->GetPeers()->Find( uidParam );
		//if( 0 != pPeer )
		//{
		//	char szMsg[ 128 ];

		//	sprintf( szMsg, MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), pPeer->CharInfo.szName );

		//	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
		//	ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
		//}
	}
	else if( INVALID_MAP == nType )
	{
		char szMsg[ 128 ];

			sprintf( szMsg, "INVALID MAP!" );
				//MGetStringResManager()->GetErrorStr(MERR_CANNOT_START_NEED_TICKET), 
				//ZApplication::GetInstance()->GetGameClient()->GetObjName(uidParam).c_str() );

			ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
			ZChatOutput(szMsg, ZChat::CMT_BROADCAST);
	}

	// Stage UI Enable
	ChangeStageEnableReady( false);
}

////////////////////////////////// Spy Mode. //////////////////////////////////
class MSpyBanMapListListener : public MListener
{
public:
	virtual bool OnCommand(MWidget* pWidget, const char* szMessage)
	{
		// ｽｺﾅﾗﾀﾌﾁ・ｸｶｽｺﾅﾍｰ｡ ｾﾆｴﾏｸ・ｸﾊ ｸｮｽｺﾆｮｸｦ ﾄﾁﾆｮｷﾑ ﾇﾒｼ�ｾ�ｴﾙ.
		if (!ZGetGameClient()->AmIStageMaster())
			return false;
		// On select
		if (MWidget::IsMsg(szMessage, MLB_ITEM_SEL) == true)
		{
			MListBox* pListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapList");
			if (!pListBox) return false;

			SpyBanMapList* pSpyBanMapList = (SpyBanMapList*)pListBox->GetSelItem();
			if (!MMatchSpyMap::IsExistingMap(pSpyBanMapList->GetMapID())) return false;

			ZPostActivateSpyMap(pSpyBanMapList->GetMapID(), ZGetGameClient()->GetMatchStageSetting()->IsIncludedSpyMap(pSpyBanMapList->GetMapID()));
			return true;
		}

		return false;
	}
};
MSpyBanMapListListener g_SpyBanMapListListener;
MListener* ZGetSpyBanMapListListener(void)
{
	return &g_SpyBanMapListListener;
}

///////////////////////////////////////////////////////

void ZStageInterface::OpenSpyBanMapBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxOpen");
	if (pButton)
		pButton->Show(false);
	pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxClose");
	if (pButton)
		pButton->Show(true);

	SetSpyBanMapBoxPos(2);

	CreateSpyBanMapList();
}

void ZStageInterface::CloseSpyBanMapBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxClose");
	if (pButton)
		pButton->Show(false);
	pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxOpen");
	if (pButton)
		pButton->Show(true);

	MWidget* pWidget = pResource->FindWidget("Stage_CharacterInfo");
	if (pWidget)
		pWidget->Enable(true);

	SetSpyBanMapBoxPos(1);
}

void ZStageInterface::HideSpyBanMapBox()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();

	MButton* pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxClose");
	if (pButton)
		pButton->Show(false);
	pButton = (MButton*)pResource->FindWidget("Stage_SpyBanMapBoxOpen");
	if (pButton)
		pButton->Show(true);

	MWidget* pWidget = pResource->FindWidget("Stage_CharacterInfo");
	if (pWidget)
		pWidget->Enable(true);

	SetSpyBanMapBoxPos(0);
}

void ZStageInterface::SetSpyBanMapBoxPos(int nBoxPos)
{
	MWidget* pWidget = ZApplication::GetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapListView");
	if (pWidget)
	{
		MRECT rect;

		switch (nBoxPos)
		{
		case 0:		// Hide
			rect = pWidget->GetRect();
			m_nSpyBanMapListFramePos = -rect.w;
			break;

		case 1:		// Close
			rect = pWidget->GetRect();
			m_nSpyBanMapListFramePos = -rect.w + (rect.w * 0.14);
			break;

		case 2:		// Open
			m_nSpyBanMapListFramePos = 0;
			break;
		}
	}
}

void ZStageInterface::CreateSpyBanMapList()
{
	// ｸﾊ ｸｮｽｺﾆｮ ｸｸｵ鮴・ﾁﾖｱ・
	MListBox* pMapListBox = (MListBox*)ZGetGameInterface()->GetIDLResource()->FindWidget("Stage_SpyBanMapList");
	if (pMapListBox == NULL) return;

	pMapListBox->RemoveAll();	// ｱ簔ｸ ｸｱｷｹﾀﾌｸﾊ ｸｮｽｺﾆｮｸｦ ｸ�ｵﾎ ﾁ�ｿ�ﾁﾘｴﾙ.
	int nCurrPlayers = (int)ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.size();

	for (int i = MMATCH_SPY_MAP_MANSION; i < MMATCH_SPY_MAP_MAX; i++)
	{
		char szListName[256];
		sprintf(szListName, "(%d~%d%s) %s", g_SpyMapNode[i].nMinPlayers, g_SpyMapNode[i].nMaxPlayers, ZMsg(MSG_SPY_WORD_PLAYERS), g_SpyMapNode[i].szName);

		SpyBanMapList* pSpyBanMapList = new SpyBanMapList(szListName,
			MBitmapManager::Get(
				ZGetGameClient()->GetMatchStageSetting()->IsIncludedSpyMap(g_SpyMapNode[i].nID) ?
				"Mark_Arrow.png" : "Mark_X.png"),
			g_SpyMapNode[i].nID);

		if (nCurrPlayers < g_SpyMapNode[i].nMinPlayers || nCurrPlayers > g_SpyMapNode[i].nMaxPlayers)
		{
			pSpyBanMapList->SetColor(MCOLOR(0xFFFF0000));	// RED color.
		}

		pMapListBox->Add(pSpyBanMapList);
	}
}

bool ZStageInterface::GetPlayableSpyMapList(int players, vector<int>& out)
{
	int nCurrPlayers = (int)ZGetGameClient()->GetMatchStageSetting()->m_CharSettingList.size();

	for (int i = MMATCH_SPY_MAP_MANSION; i < MMATCH_SPY_MAP_MAX; i++)
	{
		if (ZGetGameClient()->GetMatchStageSetting()->IsIncludedSpyMap(g_SpyMapNode[i].nID))
		{
#if !defined(_ARTIC_DEBUG)
			if (nCurrPlayers >= g_SpyMapNode[i].nMinPlayers && nCurrPlayers <= g_SpyMapNode[i].nMaxPlayers)
#endif
			{
				out.push_back(g_SpyMapNode[i].nID);
			}
		}
	}

	return !out.empty();
}

void ZStageInterface::OnSpyStageActivateMap(int nMapID, bool bExclude)
{
	MMatchStageSetting* pSetting = ZGetGameClient()->GetMatchStageSetting();

	if (bExclude)
		pSetting->ExcludeSpyMap(nMapID);
	else
		pSetting->IncludeSpyMap(nMapID);

	CreateSpyBanMapList();	// Refresh.
}

void ZStageInterface::OnSpyStageBanMapList(void* pMapListBlob)
{
	MMatchStageSetting* pSetting = ZGetGameClient()->GetMatchStageSetting();
	pSetting->UnbanAllSpyMap();

	int nCount = MGetBlobArrayCount(pMapListBlob);

	for (int i = 0; i < nCount; i++)
		pSetting->ExcludeSpyMap(*(int*)MGetBlobArrayElement(pMapListBlob, i));

	CreateSpyBanMapList();
}
///////////////////////////////////////////////////////////////////////////////

