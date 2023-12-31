#ifndef _ZWeaponMgr_h
#define _ZWeaponMgr_h

#include <list>
using namespace std;

#include "RTypes.h"
#include "RVisualMeshMgr.h"
#include "ZWeapon.h"


#include "ZActorAction.h"
// 수류탄
// 시작위치주컖E
// 방향주컖E(가속도)
// define 소툈E챨� ( 맵과의 위치연동 , 탄성 )
// 누구 수류탄인햨E알아야 하는가? ( 나중에 점펯E주콅E위해,킬수를 올리거나.. ) ( 쏜퀋E표콅E)
// Effect
enum eCrashType{
	et_crash_none = 0,
	et_crash_wall,
	et_crash_floor
};

class ZCharacter;
class ZEffectBillboardSource;

typedef list<ZWeapon*>			z_weapon_list;
typedef z_weapon_list::iterator	z_weapon_node;

class ZWeaponMgr {
public:
	ZWeaponMgr();
	~ZWeaponMgr();

public:
	void Add(ZWeapon* pWeapon) {
		m_list.push_back(pWeapon);
	}
	void Clear();
	void AddGrenade(rvector &pos,rvector &velocity,ZObject* pC);
	void AddRocket(rvector &pos,rvector &dir,ZObject* pC);
	void AddMagic(ZSkill* pSkill, const rvector &pos, const rvector &dir,ZObject* pOwner);
	void AddFlashBang(rvector &pos,rvector &dir,ZObject* pC);
	void AddSmokeGrenade(rvector &pos,rvector &velocity,ZObject* pC);

	//void AddMedikit(rvector &pos,rvector &velocity,ZObject* pC,float delay);
	//void AddRepairkit(rvector &pos,rvector &velocity,ZObject* pC,float delay);
	void AddKit(rvector &pos, rvector &velocity, ZCharacter* pC, float Delaytime, char *szMeshName, int nLinkedWorldItemID);
	void AddTrap(rvector &pos,rvector &velocity,int nItemId,ZObject* pC);
	void AddTrapAlreadyActivated(rvector& pos, float fActivatedTime, int nItemId, ZObject* pOwner);
	void AddDynamite(rvector &pos, rvector &velocity, ZObject* pC);
	void AddStunGrenade(rvector &pos, rvector &velocity, ZObject* pC);
	//////////SPY MODE /////////////
	void AddSpyFlashBang(rvector& pos, rvector& dir, ZObject* pC);
	void AddSpySmokeGrenade(rvector& pos, rvector& velocity, ZObject* pC);
	void AddSpyTrap(rvector& pos, rvector& velocity, int nItemId, ZObject* pC);

	void AddNewQuestProjectile(const ZActorActionRangeShot* pRangeShot, const rvector &pos, const rvector &dir, ZObject* pOwner);
	void DeleteWeaponHasTarget(const MUID& uidTargetChar);	// 특정 캐릭터를 타겟으로 하컖E있는 탄을 제거한다
	void EnableRender(bool b) { m_bEnableRender = b; }

	void Update();
	void Render();

	ZMovingWeapon*	UpdateWorldItem(int nItemID,rvector& pos);//월탛E아이템컖E보여지는 동콅E맞출것탛E..
	ZWeapon*		GetWorldItem(int nItemID);

	z_weapon_list m_list;

protected:

	bool	SamePoint(rvector& p1,rvector& p2);
	float	m_fLastTime;
	bool	m_bEnableRender;

};

#endif//_ZWeaponMgr_h