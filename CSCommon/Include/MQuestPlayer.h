#ifndef _MQUEST_PLAYER_H
#define _MQUEST_PLAYER_H


#include "MMatchNPCObject.h"

#include <map>
using std::map;

struct RewardZItemInfo
{
	unsigned int		nItemID;
	int					nRentPeriodHour;
	int					nItemCnt;
};

class MQuestRewardZItemList : public list<RewardZItemInfo>
{
};

/// ����Ʈ �꿡�� ���̴� �÷��� ����
struct MQuestPlayerInfo
{
	// NPC Control ���� /////////
	MMatchObject*		pObject;					///< Object ����
	unsigned long int	nNPCControlCheckSum;		///< NPC ���� üũ��
	MMatchNPCObjectMap	NPCObjects;					///< �������� NPC
	bool				bEnableNPCControl;			///< NPC Control�� �������� ����

	/// NPC ���� ����
	/// - ���ھ �������� �켱������ ������
	int GetNPCControlScore()						
	{
		// ������ �׳� �����ϴ� NPC ����
		return (int)(NPCObjects.size());
	}


	// ����Ʈ �� ���� ///////////
	bool				bMovedtoNewSector;			///< ���� ���ͷ� �̵��ߴ��� ����

	bool				bLateJoined;


	// �����̹� �� ���� /////////



	// ���� ���� ////////////////
	int						nQL;						///< QL
	int						nDeathCount;				///< ���� ȸ��
	int						nUsedPageSacriItemCount;	///< �⺻ ��� ������ ��� ����(������)
	int						nUsedExtraSacriItemCount;	///< �߰� ��� ������ ��� ����
	int						nXP;						///< ���� XP
	int						nBP;						///< ���� BP
	int						nKilledNpcHpApAccum;		///< �÷��̾ ���� NPC AP,HP����

	MQuestItemMap			RewardQuestItemMap;			///< ���� ����Ʈ ������
	MQuestRewardZItemList	RewardZItemList;


	// Log���� ////////////////// - by �߱���.
	// char				szName[ 24 ];


	/// �ʱ�ȭ
	/// @param pObj		�÷��̾� ������Ʈ ����
	/// @param a_nQL	�÷��̾� ����Ʈ ����
	void Init(MMatchObject* pObj, int a_nQL)
	{
		pObject = pObj;
		bEnableNPCControl = true;
		nNPCControlCheckSum = 0;
		NPCObjects.clear();
		bMovedtoNewSector = true;

		nQL = a_nQL;
		nDeathCount = 0;
		nUsedPageSacriItemCount = 0;
		nUsedExtraSacriItemCount = 0;
		nXP = 0;
		nBP = 0;
		nKilledNpcHpApAccum = 0;

		RewardQuestItemMap.Clear();
		RewardZItemList.clear();
		bLateJoined = false;
	}

	/// ������
	MQuestPlayerInfo() : nXP(0), nBP(0), nKilledNpcHpApAccum(0)
	{
		
	}
};

/// ����Ʈ���� �÷��̾� ������Ʈ ������
class MQuestPlayerManager : public map<MUID, MQuestPlayerInfo*>
{
private:
	MMatchStage* m_pStage;
public:
	MQuestPlayerManager();										///< ������
	~MQuestPlayerManager();										///< �Ҹ���
	void Create(MMatchStage* pStage);							///< �ʱ�ȭ
	void Destroy();												///< ����
	void AddPlayer(MUID& uidPlayer);							//Addplayer
	void DelPlayer(MUID& uidPlayer);							///< �÷��̾� ����
	void Clear();												///< �ʱ�ȭ
	MQuestPlayerInfo* GetPlayerInfo(const MUID& uidPlayer);		///< �÷��̾� ���� ��ȯ
};

struct MQuestChallengePlayerInfo
{
	int						nQL;						///< QL
	int						nDeathCount;				///< ���� ȸ��
	int						nUsedPageSacriItemCount;	///< �⺻ ��� ������ ��� ����(������)
	int						nUsedExtraSacriItemCount;	///< �߰� ��� ������ ��� ����
	int						nXP;						///< ���� XP
	int						nBP;						///< ���� BP
	int						nKilledNpcHpApAccum;		///< �÷��̾ ���� NPC AP,HP����

	void Init(MMatchObject* pObj, int a_nQL)
	{

		nQL = a_nQL;
		nDeathCount = 0;
		nUsedPageSacriItemCount = 0;
		nUsedExtraSacriItemCount = 0;
		nXP = 0;
		nBP = 0;
		nKilledNpcHpApAccum = 0;
	}

	/// ������
	MQuestChallengePlayerInfo() : nXP(0), nBP(0), nKilledNpcHpApAccum(0)
	{

	}
};

class MQuestChallengePlayerManager : public map<MUID, MQuestChallengePlayerInfo*>
{
private:
	MMatchStage* m_pStage;
public:
	MQuestChallengePlayerManager();										///< ������
	~MQuestChallengePlayerManager();										///< �Ҹ���
	void Create(MMatchStage* pStage);							///< �ʱ�ȭ
	void Destroy();												///< ����
	void AddPlayer(MUID& uidPlayer);							//Addplayer
	void DelPlayer(MUID& uidPlayer);							///< �÷��̾� ����
	void Clear();												///< �ʱ�ȭ
	MQuestChallengePlayerInfo* GetPlayerInfo(const MUID& uidPlayer);		///< �÷��̾� ���� ��ȯ
};

#endif