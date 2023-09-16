#ifndef _MMATCHCLAN_H
#define _MMATCHCLAN_H


#include "MMatchGlobal.h"
#include "MUID.h"
#include "MSmartRefresh.h"
#include <string>
#include <list>
using namespace std;

// Ŭ�� ����
enum MMatchClanGrade
{
	MCG_NONE		= 0,		// Ŭ������ �ƴ�
	MCG_MASTER		= 1,		// Ŭ�� ������
	MCG_ADMIN		= 2,		// Ŭ�� ��۵��

	MCG_MEMBER		= 9,		// �Ϲ� Ŭ����E
	MCG_END
};

// Ŭ�������� nSrcGrade�� nDstGrade���� ���ų� ��E������E�Ǻ��Ѵ�.
inline bool IsUpperClanGrade(MMatchClanGrade nSrcGrade, MMatchClanGrade nDstGrade)
{
	if ((nSrcGrade != MCG_NONE) && ((int)nSrcGrade <= (int)nDstGrade)) return true;
	return false;
}

///////////////////////////////////
class MMatchObject;

/// Ŭ��
class MMatchClan
{
private:
	int				m_nCLID;							///< Ŭ�� ��ܯ�� ID
	char			m_szClanName[CLAN_NAME_LENGTH];		///< �̸�
	unsigned long	m_nDBRefreshLifeTime;

	struct ClanInfoEx
	{
		int	nLevel;							///< ����
		int	nTotalPoint;					///< ��Ż����Ʈ
		int	nPoint;							///< ����Ʈ
		int	nWins;							///< ��E�E- �¼�E
		int	nLosses;						///< ��E�E- �м�E
		int nRanking;						///< ��ŷ
		int	nTotalMemberCount;				///< ���ο�E
		char szMaster[MATCHOBJECT_NAME_LENGTH];	
		char szEmblemUrl[256];					///< Ŭ����ũ URL
		int nEmblemChecksum;					///< Ŭ����ũ üũ��
	};

	ClanInfoEx		m_ClanInfoEx;
	MUIDRefCache	m_Members;							///< �÷��̾�сE
	MSmartRefresh	m_SmartRefresh;						///< �÷��̾�Eĳ��

	int				m_nSeriesOfVictories;				///< Ŭ����E���¼�E
	list<int>		m_MatchedClanList;					///< ��E�Eߴ�EŬ��

	unsigned long	m_nEmptyPeriod;

	void	Clear();
	void InitClanInfoEx(const int nLevel, const int nTotalPoint, const int nPoint, const int nRanking,
		                const int nWins, const int nLosses, const int nTotalMemberCount, const char* szMaster,
						const char* szEmblemUrl, int nEmblemChecksum);
public:
	MMatchClan();
	virtual ~MMatchClan();

	// MMatchClanMap���� �翁Eϴ� �Լ�E
	void Create(int nCLID, const char* szClanName);
	void AddObject(const MUID& uid, MMatchObject* pObj);
	void RemoveObject(const MUID& uid);

	
	void Tick(unsigned long nClock);
	void SyncPlayerList(MMatchObject* pObj, int nCategory);
	void InitClanInfoFromDB();			// db���� Ŭ�������� �ʱ�ȭ�Ѵ�.
	bool CheckLifePeriod();

	// get ������E
	int			GetCLID()						{ return m_nCLID; }
	const char* GetName()						{ return m_szClanName; }
//	int			GetLevel()						{ return m_nLevel; }
	int			GetMemberCount()				{ return (int)m_Members.size(); }
	ClanInfoEx*	GetClanInfoEx()					{ return &m_ClanInfoEx; }
	bool		IsInitedClanInfoEx()			{ return (m_ClanInfoEx.nLevel != 0); }
	int			GetSeriesOfVictories()			{ return m_nSeriesOfVictories; }
	const char*	GetEmblemURL()					{ return m_ClanInfoEx.szEmblemUrl; }
	int			GetEmblemChecksum()				{ return m_ClanInfoEx.nEmblemChecksum; }
	

	// Inc
	void IncWins(int nAddedWins, int GT) {
		switch (GT)
		{
		case 1:
			m_ClanInfoEx.nWins += nAddedWins;
			break;
			/*case 3:
				m_ClanInfoEx.nGladiatorWins += nAddedWins;
			break;
			case 4:
				m_ClanInfoEx.nAssassinWins += nAddedWins;
			break;*/
		}
		m_nSeriesOfVictories++;
	}
	void IncLosses(int nAddedLosses, int GT) {
		switch (GT)
		{
		case 1:
			m_ClanInfoEx.nLosses += nAddedLosses;
			break;
			/*case 3:
				m_ClanInfoEx.nGladiatorLosses += nAddedLosses;
			break;
			case 4:
				m_ClanInfoEx.nAssassinLosses += nAddedLosses;
			break;*/
		}
		m_nSeriesOfVictories = 0;
	}
	void IncPoint(int nAddedPoint, int GT)
	{
		switch (GT)
		{
		case 1:
			m_ClanInfoEx.nPoint += nAddedPoint;
			if (nAddedPoint > 0) m_ClanInfoEx.nTotalPoint += nAddedPoint;
			if (m_ClanInfoEx.nPoint < 0) m_ClanInfoEx.nPoint = 0;
			break;
			/*case 3:
				m_ClanInfoEx.nGladiatorPoint += nAddedPoint;
				if (nAddedPoint > 0) m_ClanInfoEx.nGladiatorTotalPoint += nAddedPoint;
				if (m_ClanInfoEx.nGladiatorPoint < 0) m_ClanInfoEx.nGladiatorPoint =0;
			break;
			case 4:
				m_ClanInfoEx.nAssassinPoint += nAddedPoint;
				if (nAddedPoint > 0) m_ClanInfoEx.nAssassinTotalPoint += nAddedPoint;
				if (m_ClanInfoEx.nAssassinPoint < 0) m_ClanInfoEx.nAssassinPoint =0;
			break;*/
		}
	}
	void InsertMatchedClanID(int nCLID);	///< ��E�Eߴ�EŬ��

	MUIDRefCache::iterator GetMemberBegin()		{ return m_Members.begin(); }
	MUIDRefCache::iterator GetMemberEnd()		{ return m_Members.end(); }
};

///////////////////////////////////

class MMatchClanMap : public map<int, MMatchClan*>
{
private:
	unsigned long	m_nLastTick;						///< ƽ
	map<std::string, MMatchClan*>	m_ClanNameMap;
	void CreateClan(int nCLID, const char* szClanName);
	void DestroyClan(int nCLID, MMatchClanMap::iterator* pNextItor);
	bool CheckTick(unsigned long nClock);
public:
	MMatchClanMap();
	virtual ~MMatchClanMap();
	void Destroy(); 
	void Tick(unsigned long nClock);

	void AddObject(const MUID& uid, MMatchObject* pObj);
	void RemoveObject(const MUID& uid, MMatchObject* pObj);
	MMatchClan* GetClan(const int nCLID);
	MMatchClan* GetClan(const char* szClanName);
};


#endif