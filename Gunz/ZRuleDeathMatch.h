#ifndef _ZRULE_DEATH_MATCH_H
#define _ZRULE_DEATH_MATCH_H

#include "ZRule.h"
#include "ZWorldItem.h"
class ZRuleSoloDeathMatch : public ZRule
{
public:
	ZRuleSoloDeathMatch(ZMatch* pMatch);
	virtual ~ZRuleSoloDeathMatch();
};


class ZRuleTeamDeathMatch : public ZRule
{
public:
	ZRuleTeamDeathMatch(ZMatch* pMatch);
	virtual ~ZRuleTeamDeathMatch();
};


class ZRuleTeamDeathMatch2 : public ZRule
{
public:
	ZRuleTeamDeathMatch2(ZMatch* pMatch);
	virtual ~ZRuleTeamDeathMatch2();

	virtual bool OnCommand(MCommand* pCommand);
};

class ZRuleTeamCTF : public ZRule
{
private:
	void AssignFlagEffect(MUID& uidOwner, int nTeam);
public:
	ZRuleTeamCTF(ZMatch* pMatch);
	virtual ~ZRuleTeamCTF();

	MUID m_uidRedFlagHolder;
	MUID m_uidBlueFlagHolder;
	rvector m_BlueFlagPos;
	rvector m_RedFlagPos;
	int m_bRedFlagTaken;
	int m_bBlueFlagTaken;
	inline void SetBlueFlagState(int nState) { m_bBlueFlagTaken = nState; }
	inline void SetRedFlagState(int nState) { m_bRedFlagTaken = nState; }
	inline void SetBlueFlagPos(rvector newvector) { m_BlueFlagPos = newvector; }
	inline void SetRedFlagPos(rvector newvector) { m_RedFlagPos = newvector; }
	inline int GetBlueFlagState() { return m_bBlueFlagTaken; }
	inline int GetRedFlagState() { return m_bRedFlagTaken; }
	inline rvector GetBlueFlagPos() { return m_BlueFlagPos; }
	inline rvector GetRedFlagPos() { return m_RedFlagPos; }
	virtual bool OnCommand(MCommand* pCommand);
	inline void SetBlueCarrier(MUID state) { m_uidBlueFlagHolder = state; }
	inline void SetRedCarrier(MUID state) { m_uidRedFlagHolder = state; }
	inline MUID GetBlueCarrier() { return m_uidBlueFlagHolder; }
	inline MUID GetRedCarrier() { return m_uidRedFlagHolder; }
};

class ZRuleTeamInfected : public ZRule
{
public:
	ZRuleTeamInfected(ZMatch* pMatch);
	virtual ~ZRuleTeamInfected();

	virtual bool OnCommand(MCommand* pCommand);
	virtual void OnSetRoundState(MMATCH_ROUNDSTATE roundState);
	virtual void OnResponseRuleInfo(MTD_RuleInfo* pInfo);

	MUID m_uidPatientZero;

private:
	virtual void OnUpdate(float fDelta);
	void OnInfected(const MUID& uidPlayer, bool bFirst);
	bool m_bLastFive;
};

class ZRuleGunGame : public ZRule
{
public:
	ZRuleGunGame(ZMatch* pMatch);
	virtual ~ZRuleGunGame();

	virtual bool OnCommand(MCommand* pCommand);

public:
	int m_nWeaponMaxLevel;
	vector<MTD_GunGameWeaponInfo> m_vPlayerData;
};

class ZRuleRun : public ZRule
{
public:
	ZRuleRun(ZMatch* pMatch);
	virtual ~ZRuleRun();
	virtual void OnSetRoundState(MMATCH_ROUNDSTATE roundstate);
};


#endif