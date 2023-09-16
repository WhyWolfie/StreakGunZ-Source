#ifndef _ZSHOP_H
#define _ZSHOP_H

#include "ZPrerequisites.h"
#include <vector>

using namespace std;

enum {
	ZSHOP_NORMAL = 0,
	ZSHOP_CASH,
	ZSHOP_MEDAL,
};

class ZShop
{
private:
protected:
	int m_nPage;			// ������ ����Ʈ ������
	bool m_bCreated;
	vector<MTD_ShopItemInfo*>		m_vShopItem;
	vector<MTD_GambleItemNode*>		m_vGItemList;

public:
	int m_ListFilter;
	int m_ListSubFilter;
	string m_strSearch;
	int m_ShopMode;

public:
	ZShop();
	virtual ~ZShop();
	bool Create();
	void Destroy();
	void ClearShop();
	void ClearGamble();
	void Serialize();

	bool CheckTypeWithListFilter(int type, bool bEnchantItem);
	bool CheckTypeWithListSubFilter(int type);

	int GetItemCount() { return (int)m_vShopItem.size(); }
	void SetItemsAll(const vector< MTD_ShopItemInfo*>& vShopItemList);
	void SetItemsGamble( const vector<MTD_GambleItemNode*>& vGItemList);
	int GetPage() { return m_nPage; }
	int GetShopType() { return m_ShopMode; }
	unsigned long int GetItemID(int nIndex);
	static ZShop* GetInstance();
};

inline ZShop* ZGetShop() { return ZShop::GetInstance(); }

#endif