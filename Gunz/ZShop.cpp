#include "stdafx.h"

#include "ZShop.h"
#include "ZPost.h"
#include "ZGameClient.h"
#include "MUID.h"
#include "ZApplication.h"
#include "ZGameInterface.h"
#include "MMultiColListBox.h"
#include "MLabel.h"
#include "ZEquipmentListBox.h"
#include "ZNetRepository.h"
#include "ZShopEquipItem.h"
#include "ZShopEquipListbox.h"

// ���� �־��??
enum {
	zshop_item_filter_all = 0,

	zshop_item_filter_head,
	zshop_item_filter_chest,
	zshop_item_filter_hands,
	zshop_item_filter_legs,
	zshop_item_filter_feet,
	zshop_item_filter_extra,
	//zshop_item_filter_set,
#ifdef _AVATAR_ENABLE
	zshop_item_filter_avatar,
#endif
	zshop_item_filter_melee,
	zshop_item_filter_range,
	zshop_item_filter_custom,
	//zshop_item_filter_cartridge,
	//zshop_item_filter_community,

	zshop_item_filter_quest,
	zshop_item_filter_gamble,	

	zshop_item_filter_enchant,
	//zshop_item_filter_convenience,

	zshop_item_filter_etc,
};

ZShop::ZShop() : m_nPage(0), m_bCreated(false)
{
	m_ListFilter = zshop_item_filter_all;
	m_ListSubFilter = 0;
	m_strSearch = "";
	m_ShopMode = ZSHOP_NORMAL;
}

ZShop::~ZShop()
{
	ClearShop();
	ClearGamble();
}

bool ZShop::Create()
{
	if (m_bCreated) return false;

	// ������ Shop Item ����Ʈ �޶�� ��û
	ZPostRequestShopItemList(ZGetGameClient()->GetPlayerUID(), 0, 0);	// 0���� ��ü�� �޶�� ��û�ϴ°��̴�
	ZPostRequestCharacterItemList(ZGetGameClient()->GetPlayerUID());

	m_bCreated = true;
	return true;
}

void ZShop::Destroy()
{
	if (!m_bCreated) return;

	m_bCreated = false;
}

ZShop* ZShop::GetInstance()
{
	static ZShop m_stShop;
	return &m_stShop;
}

bool ZShop::CheckTypeWithListFilter(int type, bool bEnchantItem)
{
		 if (m_ListFilter == zshop_item_filter_all)		return true;
	else if (m_ListFilter == zshop_item_filter_head)	{ if(type == MMIST_HEAD)	return true; }
	else if (m_ListFilter == zshop_item_filter_chest)	{ if(type == MMIST_CHEST)	return true; }
	else if (m_ListFilter == zshop_item_filter_hands)	{ if(type == MMIST_HANDS)	return true; }
	else if (m_ListFilter == zshop_item_filter_legs)	{ if(type == MMIST_LEGS)	return true; }
	else if (m_ListFilter == zshop_item_filter_feet)	{ if(type == MMIST_FEET)	return true; }
	else if (m_ListFilter == zshop_item_filter_extra)	{ if(type == MMIST_EXTRA || type == MMIST_FINGER) return true; }
	//else if (m_ListFilter == zshop_item_filter_set,
#ifdef _AVATAR_ENABLE
	else if (m_ListFilter == zshop_item_filter_avatar)	{ if(type == MMIST_AVATAR)	return true; }
#endif
	else if (m_ListFilter == zshop_item_filter_melee)	{ if(type == MMIST_MELEE)	return true; }
	else if (m_ListFilter == zshop_item_filter_range)	{ if(type == MMIST_RANGE)	return true; }
	else if (m_ListFilter == zshop_item_filter_custom)	{ if(type == MMIST_CUSTOM)	if(!bEnchantItem) return true; }
	//else if (m_ListFilter == zshop_item_filter_cartridge,
	//else if (m_ListFilter == zshop_item_filter_community,

	else if (m_ListFilter == zshop_item_filter_enchant)	{ if(type == MMIST_CUSTOM) if(bEnchantItem) return true; }
	//else if (m_ListFilter == zshop_item_filter_convenience)

	else if (m_ListFilter == zshop_item_filter_etc) { if((type == MMIST_NONE)) return true; }

	return false;
}

// Custom: Sub-Filter
bool ZShop::CheckTypeWithListSubFilter(int type)
{
	if( m_ListFilter == zshop_item_filter_all || m_ListSubFilter == 0 )
		return true;

	// Hard-coded shit y0
	if( m_ListFilter == zshop_item_filter_melee )
	{
		if( m_ListSubFilter == 1 && (type == MWT_DAGGER || type == MWT_DUAL_DAGGER) )
			return true;
		else if( m_ListSubFilter == 2 && type == MWT_KATANA )
			return true;
		else if( m_ListSubFilter == 3 && type == MWT_DOUBLE_KATANA )
			return true;
		else if( m_ListSubFilter == 4 && type == MWT_GREAT_SWORD )
			return true;
	}
	else if( m_ListFilter == zshop_item_filter_range )
	{
		if( m_ListSubFilter == 1 && (type == MWT_SHOTGUN || type == MWT_SAWED_SHOTGUN) )
			return true;
		else if( m_ListSubFilter == 2 && (type == MWT_SMG || type == MWT_SMGx2) )
			return true;
		else if( m_ListSubFilter == 3 && (type == MWT_RIFLE || type == MWT_MACHINEGUN) )
			return true;
		else if( m_ListSubFilter == 4 && (type == MWT_PISTOL || type == MWT_PISTOLx2) )
			return true;
		else if( m_ListSubFilter == 5 && (type == MWT_REVOLVER || type == MWT_REVOLVERx2) )
			return true;
		else if( m_ListSubFilter == 6 && type == MWT_ROCKET )
			return true;
		else if( m_ListSubFilter == 7 && type == MWT_SNIFER )
			return true;
	}
	else if( m_ListFilter == zshop_item_filter_custom )
	{
		if( m_ListSubFilter == 1 && (type == MWT_MED_KIT || type == MWT_REPAIR_KIT || type == MWT_BULLET_KIT || type == MWT_FOOD) )
			return true;
		else if( m_ListSubFilter == 2 && (type == MWT_POTION || type == MWT_SKILL) )
			return true;
		else if( m_ListSubFilter == 3 && (type == MWT_FRAGMENTATION || type == MWT_FLASH_BANG || type == MWT_SMOKE_GRENADE || type == MWT_TRAP || type == MWT_DYNAMITYE) )
			return true;
	}

	return false;
}

void ZShop::Serialize()
{
	ZIDLResource* pResource = ZApplication::GetGameInterface()->GetIDLResource();
	MMultiColListBox* pListBox = (MMultiColListBox*)pResource->FindWidget("AllEquipmentList");
	MEdit* pEdit = (MEdit*)pResource->FindWidget("Shop_Search");

	if (!pListBox) { _ASSERT(0); return; }
	pListBox->RemoveAll();

	if (m_ListFilter==zshop_item_filter_all || m_ListFilter==zshop_item_filter_gamble)
	{
		for (int i=0;  i<(int)m_vGItemList.size(); i++)
		{
			const ZGambleItemDefine* pDesc = ZGetGambleItemDefineMgr().GetGambleItemDefine( m_vGItemList[i]->nItemID );
			if (pDesc == NULL) { _ASSERT(0); continue; }

			ZShopEquipItem_Gamble* pGItem = new ZShopEquipItem_Gamble(pDesc);
			ZShopEquipItemHandle_PurchaseGamble* pHandle = 
				new ZShopEquipItemHandle_PurchaseGamble(pGItem);

			pGItem->SetHandlePurchase(pHandle);

			ZShopEquipListItem* pListItem = new ZShopEquipListItem(pGItem);
			pListBox->Add(pListItem);
		}
	}

	for (int i=0; i<GetItemCount(); i++)
	{
		ZShopEquipItem* pWrappedItem = NULL;

		if (MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(m_vShopItem[i]->nItemID))
		{
			if (CheckTypeWithListFilter(pDesc->m_nSlot, pDesc->IsEnchantItem()) == false) continue;
			if (pDesc->m_nResSex.Ref() != -1 && pDesc->m_nResSex.Ref() != int(ZGetMyInfo()->GetSex())) continue;
			if (CheckTypeWithListSubFilter(pDesc->m_nWeaponType.Ref()) == false) continue;
			
			if (GetShopType() == ZSHOP_NORMAL)
			{
				if (pDesc->itemClass() == ITEM_MEDAL || pDesc->itemClass() == ITEM_CASH) continue;
			}
			else if (GetShopType() == ZSHOP_MEDAL)
			{
				if (pDesc->itemClass() == ITEM_CASH || pDesc->itemClass() == ITEM_BOUNTY) continue;
			}
			// Custom: search by item name.
			if (pEdit)
			{
				if (strlen(pEdit->GetText()) > 0)
				{
					if (!StrStrI(pDesc->m_pMItemName->Ref().m_szItemName, pEdit->GetText()))
						continue;
				}
			}
			ZShopEquipItem_Match* pMItem = new ZShopEquipItem_Match(pDesc);
			ZShopEquipItemHandle_PurchaseMatch* pHandle = 
				new ZShopEquipItemHandle_PurchaseMatch(pMItem);
			pWrappedItem = pMItem;
			pWrappedItem->SetHandlePurchase(pHandle);

		}
		else if (MQuestItemDesc* pDesc = GetQuestItemDescMgr().FindQItemDesc( m_vShopItem[i]->nItemID))
		{
			// do not list in this shop
			if (ZGetShop()->GetShopType() == ZSHOP_MEDAL) continue;
			if (m_ListFilter != zshop_item_filter_all && m_ListFilter != zshop_item_filter_quest) continue;
			//Don't display quest items when the player searches for an item, they can just use the quest filter.
			if (strlen(pEdit->GetText()) > 0) continue;

			MUID uidItem = MUID(0, i+1);
			ZShopEquipItem_Quest* pQItem = new ZShopEquipItem_Quest(pDesc);
			ZShopEquipItemHandle_PurchaseQuest* pHandle = 
				new ZShopEquipItemHandle_PurchaseQuest(pQItem);
			pWrappedItem = pQItem;
			pWrappedItem->SetHandlePurchase(pHandle);
		}


		// Custom: Disabled ASSERT
		if (!pWrappedItem) { /*_ASSERT(0);*/ continue; }

		ZShopEquipListItem* pListItem = new ZShopEquipListItem(pWrappedItem);
		pListBox->Add(pListItem);
	}

	pListBox = (MMultiColListBox*)pResource->FindWidget("CashEquipmentList");

	if (!pListBox) { _ASSERT(0); return; }
	pListBox->RemoveAll();

	for (int i=0; i<GetItemCount(); i++)
	{
		ZShopEquipItem* pWrappedItem = NULL;

		if (MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(m_vShopItem[i]->nItemID))
		{
			if (CheckTypeWithListFilter(pDesc->m_nSlot, pDesc->IsEnchantItem()) == false) continue;
			if (CheckTypeWithListSubFilter(pDesc->m_nWeaponType.Ref()) == false) continue;
			if (pDesc->m_nResSex.Ref() != -1 && pDesc->m_nResSex.Ref() != int(ZGetMyInfo()->GetSex())) continue;
			if (pDesc->itemClass() == ITEM_MEDAL || pDesc->itemClass() == ITEM_BOUNTY) continue;

			ZShopEquipItem_Match* pMItem = new ZShopEquipItem_Match(pDesc);
			ZShopEquipItemHandle_PurchaseMatch* pHandle = 
				new ZShopEquipItemHandle_PurchaseMatch(pMItem);
			pWrappedItem = pMItem;
			pWrappedItem->SetHandlePurchase(pHandle);
		}

		if (!pWrappedItem) { _ASSERT(0); continue; }

		ZShopEquipListItem* pListItem = new ZShopEquipListItem(pWrappedItem);
		pListBox->Add(pListItem);
	}
	/* ��Ʈ������ �߰� ����
	else if (pSetItem && pBaseItem->GetItemType()==MSIT_SITEM)
	{
		if (m_ListFilter != zshop_item_filter_all && m_ListFilter != zshop_item_filter_set_item) continue;

		bool bSexRestricted = false;
		MMatchItemDesc* pDesc;
		for (int i=0; i<MAX_SET_ITEM_COUNT; ++i)
		{
			pDesc = MGetMatchItemDescMgr()->GetItemDesc( pSetItem->GetItemID(i));
			if (!pDesc) continue;
			if (pDesc->m_nResSex.Ref() != -1) {
				if (pDesc->m_nResSex.Ref() != int(ZGetMyInfo()->GetSex())) {
					bSexRestricted = true;
					break;
				}
			}
		}
		if (bSexRestricted)
			continue;

		ZShopEquipItem_Set* pSItem = new ZShopEquipItem_Set(pSetItem);
		ZShopEquipItemHandle_PurchaseSet* pHandle = 
			new ZShopEquipItemHandle_PurchaseSet(pSItem, pBaseItem->GetShopPriceMap(), pBaseItem->GetMSID());
		pWrappedItem = pSItem;
		pWrappedItem->SetHandlePurchase(pHandle);
	}
	*/
}

unsigned long int ZShop::GetItemID(int nIndex)
{
	if ((nIndex < 0) || (nIndex >= (int)m_vShopItem.size())) return 0;
	return m_vShopItem[nIndex]->nItemID;
}

void ZShop::SetItemsAll(const vector< MTD_ShopItemInfo*>& vShopItemList)
{
	ClearShop();

	for ( int i = 0;  i < (int)vShopItemList.size();  i++)
	{
		MTD_ShopItemInfo* pNode = new MTD_ShopItemInfo;
		memcpy( pNode, vShopItemList[i], sizeof(MTD_ShopItemInfo));
		m_vShopItem.push_back(pNode);
	}
}

void ZShop::SetItemsGamble( const vector<MTD_GambleItemNode*>& vGItemList)
{
	ClearGamble();

	if( IsAdminGrade(ZGetMyInfo()->GetUGradeID()) )
		mlog( "Shop Gamble item count : %d\n", (int)vGItemList.size() );

	for ( int i = 0;  i < (int)vGItemList.size();  i++)
	{
		MTD_GambleItemNode* pNode = new MTD_GambleItemNode;
		memcpy( pNode, vGItemList[ i], sizeof( MTD_GambleItemNode));
		m_vGItemList.push_back( pNode);

		if( IsAdminGrade(ZGetMyInfo()->GetUGradeID()) )
		{
			const ZGambleItemDefine* pGItemDef = ZGetGambleItemDefineMgr().GetGambleItemDefine( pNode->nItemID );
			if( NULL != pGItemDef )
			{
				mlog( "shop gitem : %d, %s\n"
					, pGItemDef->GetGambleItemID()
					, pGItemDef->GetName().c_str() );
			}
			else
			{
				mlog ("invaled shop gitem : %d\n"
					, pNode->nItemID );
			}
		}
	}
}

void ZShop::ClearShop()
{
	for ( int i = 0;  i < (int)m_vShopItem.size();  i++)
		delete m_vShopItem[ i];

	m_vShopItem.clear();
}

void ZShop::ClearGamble()
{
	for ( int i = 0;  i < (int)m_vGItemList.size();  i++)
		delete m_vGItemList[ i];

	m_vGItemList.clear();
}
