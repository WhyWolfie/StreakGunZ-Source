#include "stdafx.h"
#include "ZActorWithFSM.h"
#include "MActorDef.h"
#include "ZFSMManager.h"
#include "ZFSM.h"
#include "ZFSMState.h"
#include "ZActorActionManager.h"
#include "ZActorAction.h"
//#include "ZNavigationMesh.h"
#include "ZScreenEffectManager.h"
#include "ZModule_HPAP.h"
#include "ZModule_Resistance.h"
#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"
#include "ZActorMesh.h"
#include "ZRuleQuestChallenge.h"

MImplementRTTI(ZActorWithFSM, ZActorBase);


ZActorWithFSM::ZActorWithFSM(ZGame* pGame, ZActorActionManager* pActionMgr)
: ZActorBase(pGame)
, m_pActionMgr(pActionMgr)
, m_pActorDef(NULL)
, m_pFsm(NULL)
, m_pCurrState(NULL)
, m_pCurrAction(NULL)
, m_pCurrActionStartTime(0)
, m_uidTarget(0,0)
, m_fGroggy(0)
, m_bPostedActionExecute(false)
, m_bPostedRangeShotInAction(false)
, m_bMoving(false)
, m_bSuffering(false)
, m_RealPositionBefore(0,0,0)
, m_bMyControl(false)
, m_nLastTimePostBasicInfo(0)
, m_nSpeedAccel(0.f)
, m_nTeamID(MMT_ALL)
{
	m_bIsNPC = true;
	SetVisible(true);
	m_bInitialized = true;
	m_nLastUpdate = ZTimeGetTime();

	m_pModule_HPAP				= new ZModule_HPAP;
	m_pModule_Resistance		= new ZModule_Resistance;
	m_pModule_FireDamage		= new ZModule_FireDamage;
	m_pModule_ColdDamage		= new ZModule_ColdDamage;
	m_pModule_PoisonDamage		= new ZModule_PoisonDamage;
	m_pModule_LightningDamage	= new ZModule_LightningDamage;

	AddModule(m_pModule_HPAP);
	AddModule(m_pModule_Resistance);
	AddModule(m_pModule_FireDamage);
	AddModule(m_pModule_ColdDamage);
	AddModule(m_pModule_PoisonDamage);
	AddModule(m_pModule_LightningDamage);

	// �̵� ����� ���� ������Ʈ �� ���̹Ƿ� ��Ȱ�� ���·� ������ ZModuleContainer::UpdateModules()���� �ߺ� update���� �ʴ´�
	m_pModule_Movable->Active(false);

}

ZActorWithFSM::~ZActorWithFSM()
{
	EmptyHistory();
	DestroyShadow();

	RemoveModule(m_pModule_HPAP);
	RemoveModule(m_pModule_Resistance);
	RemoveModule(m_pModule_FireDamage);
	RemoveModule(m_pModule_ColdDamage);
	RemoveModule(m_pModule_PoisonDamage);
	RemoveModule(m_pModule_LightningDamage);

	delete m_pModule_HPAP;
	delete m_pModule_Resistance;
	delete m_pModule_FireDamage;
	delete m_pModule_ColdDamage;
	delete m_pModule_PoisonDamage;
	delete m_pModule_LightningDamage;
}

void ZActorWithFSM::InitMesh(char* szMeshName)
{
	_ASSERT(m_pVMesh == NULL);

//	AllocObjectVMesh();
//	ZActorMesh::InitNpcMesh(this, szMeshName);
	//m_pVMesh->InitNpcMesh(this, szMeshName);
}

void ZActorWithFSM::OnDraw()
{
	if (m_pVMesh == NULL) return;

	Draw_SetLight(GetPosition() );

	//todok �׾��� �� ������� ǥ�� �ʿ�
//	if( IsDieAnimationDone() )
//	{
//#define TRAN_AFTER		0.5f	// �� �ð� ���ĺ���
//#define VANISH_TIME		1.f		// �� �ð����� ����������
//
//		if(m_timeDieAnimationDone==-1) m_timeDieAnimationDone = m_pGame->GetTime();
//
//		//float fOpacity = max(0.f,min(1.0f,(	VANISH_TIME-(m_pGame->GetTime()-GetDeadTime() - TRAN_AFTER))/VANISH_TIME));
//		float fOpacity = max(0.f,min(1.0f,(	VANISH_TIME-(m_pGame->GetTime()-m_timeDieAnimationDone - TRAN_AFTER))/VANISH_TIME));
//
//		m_pVMesh->SetVisibility(fOpacity);
//	}
//	else {
//		if(!m_bHero) m_pVMesh->SetVisibility(1.f);
//		m_timeDieAnimationDone = -1;
//	}

	m_pVMesh->Render();

	//Cond_Pick() �׽�Ʈ�� : �ش� �������� ������ �׸���
	/*D3DXMATRIX world;
	MakeWorldMatrix(&world, GetPosition(), GetDirection(), rvector(0,0,1));
	RGetDevice()->SetTransform(D3DTS_WORLD, &world);
	RGetDevice()->SetRenderState(D3DRS_LIGHTING, FALSE);
	if (!Cond_Pick(90, 200))
		RDrawLine(rvector(-200, 5, 0), rvector(0, 5, 0), 0xffffff);
	if (!Cond_Pick(-90, 200))
		RDrawLine(rvector(200, 5, 0), rvector(0, 5, 0), 0xffffff);
	if (!Cond_Pick(0, 200))
		RDrawLine(rvector(0, 5, -200), rvector(0, 5, 0), 0xffffff);
	if (!Cond_Pick(180, 200))
		RDrawLine(rvector(0, 5, 200), rvector(0, 5, 0), 0xffffff);
		*/
}

void ZActorWithFSM::OnUpdate( float fDelta )
{
	//todok ������ ������ �����ؾ��Ѵ�. ���� �׾ �Ⱥ��̴� �� ������Ʈ�� ��� ���ư��� ����

	if(m_pVMesh && !IsDie()) 
	{
		m_pVMesh->SetVisibility(1.f);
	}

	if (IsMyControl())
	{

		if (ZTimeGetTime() - m_nLastUpdate >= 10)
		{
			m_nLastUpdate = ZTimeGetTime();
			UpdateFsm(fDelta);
		}
		PostBasicInfo();

		UpdateVelocity(fDelta);
		UpdatePosition(fDelta);
		UpdateHeight(fDelta);
		UpdateGroggy(fDelta);
	}

	ProcessMotion(fDelta);

	ProcessShotInAction(fDelta);
}

void ZActorWithFSM::SetMyControl(bool bMyControl)
{
	m_bMyControl = bMyControl;
}

bool ZActorWithFSM::IsMyControl()
{
	return m_bMyControl;
}

bool ZActorWithFSM::IsMovingAnimation()
{
	/*if (!m_pCurrState) return false;
	IActorAction* pAction = m_pCurrState->GetAction();
	if (!pAction) return false;
	return pAction->IsMovingAnimation();*/

	if (!m_pCurrAction) return false;
	return m_pCurrAction->IsMovingAnimation();
}

bool ZActorWithFSM::IsCollideable()
{
	if (!m_Collision.IsCollideable())
		return false;

	return !IsDie();
}

bool ZActorWithFSM::IsDie() 
{ 
	if (!m_pFsm) { _ASSERT(0); return true; }

	if (m_pFsm->GetState(FSM_BISTATE_DIE) == m_pCurrState || m_pFsm->GetState(FSM_BISTATE_WAIT_DELETE) == m_pCurrState)
	{
		m_Collision.SetCollideable(false);
		return true;
	}

	return false;
}

bool ZActorWithFSM::InitWithActorDef( MActorDef* pActorDef, ZFSMManager* pFsmMgr )
{
	m_pActorDef = pActorDef;

	// init mesh
	_ASSERT(m_pVMesh == NULL);
	//AllocObjectVMesh();
	ZActorMesh::InitNpcMesh(this, m_pActorDef->GetModelName());
//	m_pVMesh->InitNpcMesh(this, m_pActorDef->GetModelName());
	if (!pActorDef->GetNoShadow())
		CreateShadow();

	m_Collision.SetHeight(pActorDef->GetCollup120() ? m_pActorDef->GetCollisionHeight() + 120 : m_pActorDef->GetCollisionHeight());
	m_Collision.SetRadius(m_pActorDef->GetCollisionRadius());
	// init hp ap
	m_pModule_HPAP->SetMaxHP(pActorDef->GetMaxHp());
	m_pModule_HPAP->SetHP(pActorDef->GetMaxHp());

	m_pModule_HPAP->SetMaxAP(pActorDef->GetMaxAp());
	m_pModule_HPAP->SetAP(pActorDef->GetMaxAp());
	
	m_pModule_HPAP->SetRealDamage(true);

	// init fsm
	m_pFsm = pFsmMgr->GetFsm( pActorDef->GetFsmName());
	if (!m_pFsm)
	{
		_ASSERT(0);
		return false;
	}

	m_pCurrState = m_pFsm->GetState( m_pFsm->GetEntryStateName());
	if (!m_pCurrState)
	{
		_ASSERT(0);
		return false;
	}

	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		m_nTeamID = MMT_BLUE;

	OnEnterState();

	return true;
}

bool ZActorWithFSM::ProcessMotion(float fDelta)
{
	// �޽� �ִϸ��̼� ������Ʈ�� ����ϴ� �Լ�!

	if (!m_pVMesh) return false;
	m_pVMesh->Frame();

	rvector vProxyPosition = GetPosition();
	rvector vProxyDirection = GetDirection();

	D3DXMATRIX world;

	rvector MeshPosition;	// �̵�����ŭ�� �� �޽� ������ ��ġ
	rvector vAnimationPositionDiff(0,0,0);

	// �������� �ִ� �ִϸ��̼��� ��ġ������ ����� �Ѵ�.
	// ���� �����Ӱ� ���� �����ӿ����� �ִϸ��̼� �� ��ġ ��ȭ���� ���ؼ� ���� ������ ��ġ�� �߰� �̵������ְ�
	// �޽��� �׸�ŭ �������� �ݴ�� �ɾ �������ؾ� �Ѵ�.
	if(IsMovingAnimation())
	{
		MakeWorldMatrix(&world,rvector(0,0,0),vProxyDirection,rvector(0,0,1));

		// ���� ��ġ�� ���� ��ǥ��� ����
		rvector footPosition = m_pVMesh->GetFootPosition();
		rvector realPosition = footPosition * world;

		MeshPosition = vProxyPosition - realPosition ;

		vAnimationPositionDiff = footPosition - m_RealPositionBefore;	// ���� �����Ӱ� ������ �� ��ġ ��ȭ��

		// �ִϸ��̼��� �������� ������ ���������� ��ȯ�Ѵ�
		vAnimationPositionDiff = vAnimationPositionDiff * world;

		m_RealPositionBefore = footPosition;
	}
	else
		MeshPosition = vProxyPosition;

	MakeWorldMatrix(&world, MeshPosition, vProxyDirection, rvector(0,0,1));
	m_pVMesh->SetWorldMatrix(world);

	if(IsMyControl() && IsMovingAnimation())
	{
		// �ִϸ��̼ǿ� ���Ե� �̵�����ŭ ���� ��ġ�� �̵�
		rvector origdiff=fDelta*GetVelocity();

		rvector diff = vAnimationPositionDiff;
		diff.z+=origdiff.z;

		if(GetDistToFloor()<0 && diff.z<0) diff.z=-GetDistToFloor();

		Move(diff);
	}

/*	//todok ����Ʈ�� �ǰ�ó��
	if(IsActiveModule(ZMID_LIGHTNINGDAMAGE)==false) {// ����Ʈ�� ���ϸ��̼� ����..

		if (m_pVMesh->isOncePlayDone())
		{
			m_Animation.Input(ZA_ANIM_DONE);
		}
	}
*/
	return true;
}

void ZActorWithFSM::UpdateVelocity(float fDelta)
{
	if (IsAir() /*&& !IsMovingAnimation()*/)
		m_pModule_Movable->UpdateGravity(fDelta);

	if (!IsSuffering() && !IsAir())
	{
		if (IsMoving())
		{
			const float fAccel = 10000.f;
			AddVelocity(m_Direction * (fAccel * fDelta));

			float fSpeed = GetSpeedAccel() > 1.f ? GetSpeedAccel() : m_pActorDef->GetSpeed();

			rvector vel = GetVelocity();
			if (Magnitude(vel) > fSpeed)
			{
				Normalize(vel);
				vel *= fSpeed;
				SetVelocity(vel);
			}
		}
		else
		{
			SetVelocity(rvector(0,0,GetVelocity().z));
		}
	}
}

void ZActorWithFSM::UpdatePosition(float fDelta)
{
	m_pModule_Movable->Update(fDelta);


	// ���� ������ ó��
	if(m_pModule_Movable->isLanding())
	{
		//SetFlag(AF_LAND, true);
		//m_Animation.Input(ZA_EVENT_REACH_GROUND);
		SetVelocity(0,0,0);

		if(GetPosition().z + 100.f < m_pModule_Movable->GetFallHeight())
		{
			float fSpeed=fabs(GetVelocity().z);

			// ���� ������ ����..
			rvector vPos = GetPosition();
			rvector vDir = rvector(0.f,0.f,-1.f);
			vPos.z += 50.f;

			RBSPPICKINFO pInfo;
			if (m_pGame->PickWorld(vPos, vDir, &pInfo))
			{
				vPos = pInfo.PickPos;

				vDir.x = pInfo.pInfo->plane.a;
				vDir.y = pInfo.pInfo->plane.b;
				vDir.z = pInfo.pInfo->plane.c;

				AddLandingEffect(vPos, vDir);
			}
		}
	}
}

void ZActorWithFSM::UpdateHeight(float fDelta)
{
	// ���� �����ų� �������� �ϴµ� �������
	rvector diff=fDelta*GetVelocity();

	bool bUp = (diff.z>0.01f);
	bool bDownward= (diff.z<0.01f);

	if(GetDistToFloor()<0 || (bDownward && m_pModule_Movable->GetLastMove().z>=0))
	{
		if(GetVelocity().z<1.f && GetDistToFloor()<1.f)
		{
			if(GetVelocity().z<0)
				SetVelocity(GetVelocity().x,GetVelocity().y,0);
		}
	}

	// ��� ���������� ������ �ö������..
	if(GetDistToFloor()<0 && !IsDie())
	{
		float fAdjust=400.f*fDelta;	// �ʴ� �̸�ŭ
		rvector diff=rvector(0,0,min(-GetDistToFloor(),fAdjust));
		Move(diff);
	}
}

void ZActorWithFSM::UpdateGroggy(float fDelta)
{
	// �׷α��� �ڿ� ȸ���� ����
	if (!m_pActorDef) { _ASSERT(0); return; }
	float fRecoverSpeed = m_pActorDef->GetGroggyRecoverySpeed();
	m_fGroggy -= fRecoverSpeed * fDelta;
	if (m_fGroggy < 0)
		m_fGroggy = 0;

	//todok del
	//mlog("��ACTOR_GROGGY : %f\n", m_fGroggy);
}

ZFSMState* ZActorWithFSM::CheckGlobalTransition()
{
	/* todok ���� ���̴� fsm ���� xml���� �������� �ϵ��� �Ѵ�.
	// todok ���� ������ ���� �뺸�ϴ� ���� func���� �����ؾ� �Ѵ�.
	// �켱 ���� �˻����.. todok ���⿡ ���� ���� �Լ��� ���� �ű���
	ZFSMState* pDieState = m_pFsm->GetState(FSM_BISTATE_DIE);
	if (m_pCurrState != pDieState)
	{
		if (Cond_GetHp() <= 0)
		{
			//todok ������ ������ �Ѵ�. ���� ���� ���Խ� �ٷ� ������ �ٵ� �̰� enter function���� �����ؾ� �ϳ�?
			//ZPostQuestRequestNPCDead(uidAttacker, GetUID(), GetPosition());
			return pDieState;
		}
	}*/
	return NULL;
}

void ZActorWithFSM::SetAnimationInAction(IActorAction* pAction)
{
	if (!pAction) return;

	const char* szAnimName = pAction->GetAnimationName();
	if (!m_pVMesh->SetAnimation(ani_mode_lower, (char*)szAnimName, false))
		_ASSERT(0);
}

void ZActorWithFSM::UpdateFsm(float fDelta)
{
	if (!IsMyControl()) return;
	if (!m_pFsm) { _ASSERT(0); return; }
	if (!m_pCurrState) { _ASSERT(0); return; }

	ValidateTarget();

	// ���� �˻�
	ZFSMState* pNextState = NULL;

	pNextState = CheckGlobalTransition();
	if (!pNextState)
		pNextState = m_pCurrState->CheckTransition(this);

	if (pNextState)
	{
		OnExitState();
		m_pCurrState = pNextState;
		OnEnterState();
	}

	// �׼� ����
	IActorAction* pAction = m_pCurrState->GetAction();
	if (pAction)
	{
		SetAnimationInAction(pAction);

		//todok del
/*		const ZActorActionMeleeShot* pMeleeShot = pAction->GetMeleeShot();
		const ZActorActionRangeShot* pRangeShot = pAction->GetRangeShot();

		// ���� Ŀ�ǵ�� state������ 1���� ������
		if (pMeleeShot && !m_bPostedActionExecute)
		{
			ZPostNewQuestPeerNpcAttackMelee(m_UID, pAction->GetName());
			m_bPostedActionExecute = true;
		}
		if (pRangeShot && !m_bPostedRangeShotInAction)
		{
			ZPostNewQuestPeerNpcAttackRange(m_UID, pAction->GetName());
			m_bPostedRangeShotInAction = true;
		}*/
	}

	// function ����
	int numFunc = m_pCurrState->GetFunctionCallCount();
	for (int i=0; i<numFunc; ++i)
	{
		const ZFSMFunctionCall* pFunc = m_pCurrState->GetFunctionCall(i);
		ExecuteFunction(fDelta, pFunc->id, pFunc->nArg[0], pFunc->nArg[1], pFunc->nArg[2], pFunc->nArg[3]);
	}
}

void ZActorWithFSM::ProcessShotInAction(float fDelta)
{
	if (!m_pCurrAction) return;

	float fCurrTime = ZGetGame()->GetTime();

	int nNumMeleeShot = m_pCurrAction->GetNumMeleeShot();
	for (int i = 0; i < nNumMeleeShot; ++i)
	{
		const ZActorActionMeleeShot* pMeleeShot = m_pCurrAction->GetMeleeShot(i);
		if (fCurrTime >= m_pCurrActionStartTime + pMeleeShot->GetDelay() && !IsProcessedShot(pMeleeShot))
		{
			pMeleeShot->ProcessShot(m_pGame, m_UID, fCurrTime);
			SetProcessedShot(pMeleeShot);
		}
	}

	int numRangeShot = m_pCurrAction->GetNumRangeShot();
	for (int i = 0; i<numRangeShot; ++i)
	{
		const ZActorActionRangeShot* pRangeShot = m_pCurrAction->GetRangeShot(i);
		if (pRangeShot)
		{
			if (fCurrTime >= m_pCurrActionStartTime + pRangeShot->GetDelay() &&
				!IsProcessedShot(pRangeShot))
			{
				rvector vDir;
				rmatrix mat;
				ZObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
				if (pChar == NULL)
					vDir = GetDirection() + pRangeShot->GetDirectionMod();
				else
					vDir = (pChar->GetPosition() - GetPosition()) + pRangeShot->GetDirectionMod();

				D3DXMatrixRotationY(&mat, ToRadian(pRangeShot->GetYAxis()));
				D3DXMatrixRotationZ(&mat, ToRadian(pRangeShot->GetZAxis()));
				if (pRangeShot->GetZAxis() != 0 || pRangeShot->GetYAxis() != 0)
				{
					vDir = vDir * mat;
				}
				else
					vDir = vDir;
				Normalize(vDir);

				rvector vPos;
				_RMeshPartsPosInfoType posPartsType = pRangeShot->GetPosPartsType();
				if (posPartsType == eq_parts_pos_info_end)
					vPos = GetPosition();
				else
					vPos = m_pVMesh->GetBipTypePosition(posPartsType);
				//	vPos *= rvector(0,pRangeShot->GetZAxis(), pRangeShot->GetYAxis());
				pRangeShot->ProcessShot(vPos, vDir, this);
				SetProcessedShot(pRangeShot);
			}
		}
	}

	int numGrenadeShot = m_pCurrAction->GetNumGrenadeShot();
	for (int i = 0; i<numGrenadeShot; ++i)
	{
		const ZActorActionGrenadeShot* pGrenadeShot = m_pCurrAction->GetGrenadeShot(i);
		if (pGrenadeShot)
		{

			if (fCurrTime >= m_pCurrActionStartTime + pGrenadeShot->GetDelay() && !IsProcessedShot(pGrenadeShot))
			{
				rvector vPos, vDir;
				rmatrix mat;
				vDir = GetDirection() + pGrenadeShot->GetDirModDirection();

				D3DXMatrixRotationY(&mat, pGrenadeShot->GetYAxis());
				if (pGrenadeShot->GetZAxis() != 0)
				{
					D3DXMatrixRotationZ(&mat, pGrenadeShot->GetZAxis());
				}
				vDir = vDir * mat;
				Normalize(vDir);

				_RMeshPartsPosInfoType posPartsType = pGrenadeShot->GetPosPartsType();
				if (posPartsType == eq_parts_pos_info_end)
					vPos = GetPosition() + pGrenadeShot->GetPosModPosition();
				else
					vPos = m_pVMesh->GetBipTypePosition(posPartsType) + pGrenadeShot->GetPosModPosition();


					pGrenadeShot->ProcessShot(vPos, vDir * pGrenadeShot->GetForce(), this);
					SetProcessedShot(pGrenadeShot);
			}
		}
	}

	int numEffect = m_pCurrAction->GetNumEffect();
	for (int i = 0; i<numEffect; ++i)
	{
		const ZActorActionEffect* pEffect = m_pCurrAction->GetEffect(i);
		if (pEffect && fCurrTime >= m_pCurrActionStartTime + pEffect->GetDelay() && !IsProcessedEffect(pEffect))
		{
			rvector vDir = GetDirection() + pEffect->GetDirectionMod();
			rmatrix mat;
			D3DXMatrixRotationZ(&mat, pEffect->GetZAxis());
			pEffect->GetZAxis() == 0 ? vDir = vDir : vDir = vDir * mat;
			Normalize(vDir);
			rvector vPos;
			_RMeshPartsPosInfoType posPartsType = pEffect->GetPosPartsType();
			if (posPartsType == eq_parts_pos_info_end)
				vPos = GetPosition() + rvector(vDir.x * pEffect->GetPosMod().y, vDir.y * pEffect->GetPosMod().y, pEffect->GetPosMod().z);
			else
				vPos = m_pVMesh->GetBipTypePosition(posPartsType) + rvector(vDir.x * pEffect->GetPosMod().y, vDir.y * pEffect->GetPosMod().y, pEffect->GetPosMod().z);


			pEffect->ProcessShot(pEffect->GetMeshName(), vPos, vDir, m_UID);
			SetProcessedEffect(pEffect);
		}
	}

	int numSounds = m_pCurrAction->GetNumSound();
	for (int i = 0; i<numSounds; ++i)
	{
		const ZActorActionSound* pSound = m_pCurrAction->GetSound(i);
		if (pSound && fCurrTime >= m_pCurrActionStartTime + pSound->GetDelay() && !IsProcessedSound(pSound))
		{
			pSound->ProcessSound(pSound->GetSoundPath(), GetPosition());
			SetProcessedSound(pSound);
		}
	}

	//Check Players in game, if less than the max playercount, spawn less npc's (not in use atm)
	if (ZGetCharacterManager()->size() < ZGetGameClient()->GetMatchStageSetting()->GetMaxPlayers())
	{
		m_bAdjustPlayerNum = true;
	}
	int nNumSummon = m_pCurrAction->GetNumSummon();
	for (int i = 0; i < nNumSummon; ++i)
	{
		const ZActorActionSummon* pSummon = m_pCurrAction->GetSummon(i);
		if (pSummon && fCurrTime >= m_pCurrActionStartTime + pSummon->GetDelay() && !IsProcessedSummon(pSummon))
		{
			rvector pos = GetPosition() + rvector(GetDirection().x * pSummon->GetRange(), GetDirection().y *
				pSummon->GetRange(), 0);
			rvector dir = GetDirection() + rvector(pSummon->GetAngle(),0,0);
			pos.z = 0;
			Normalize(dir);
			pSummon->ProcessSummon(m_uidTarget, m_UID,i,pSummon->GetSummonName(), pos,dir,pSummon->GetAdjustPlayerNum()); //change charger name later
			SetProcessedSummon(pSummon);
		}
	}
}

void ZActorWithFSM::ValidateTarget()
{
	// �������ų� ���� ��� Ÿ���� �����Ѵ�
	ZCharacter* pChar = (ZCharacter*) m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar || pChar->IsDie())
	{
		m_uidTarget.SetInvalid();
		return;
	}
}

void ZActorWithFSM::PostBasicInfo()
{
	DWORD nNowTime = timeGetTime();
	if (GetInitialized() == false) return;

	// �װ��� 5�ʰ� ������ basicinfo�� ������ �ʴ´�.
	if(IsDie() && m_pGame->GetTime() - GetDeadTime()>5.f) return;
	//int nMoveTick = (ZGetGameClient()->GetAllowTunneling() == false) ? PEERMOVE_TICK : PEERMOVE_AGENT_TICK;
	int nMoveTick = 25; //40 fps
	//nMoveTick = 1; //todok del

	if ((int)(nNowTime - m_nLastTimePostBasicInfo) >= nMoveTick)
	{
		m_nLastTimePostBasicInfo = nNowTime;

		ZACTOR_WITHFSM_BASICINFO pbi;
		pbi.uidNPC = GetUID();

		pbi.posx = GetPosition().x;
		pbi.posy = GetPosition().y;
		pbi.posz = GetPosition().z;

		pbi.velx = GetVelocity().x;
		pbi.vely = GetVelocity().y;
		pbi.velz = GetVelocity().z;

		pbi.dirx = GetDirection().x*32000.0f;
		pbi.diry = GetDirection().y*32000.0f;
		pbi.dirz = GetDirection().z*32000.0f;

		pbi.nActionIndex = -1;
		if (m_pCurrAction)
			pbi.nActionIndex = m_pActionMgr->GetIndexOfAction(m_pCurrAction);


		ZPOSTCMD1(MC_NEWQUEST_PEER_NPC_BASICINFO, MCommandParameterBlob(&pbi,sizeof(ZACTOR_WITHFSM_BASICINFO)));


		// history �� ����
#define ACTOR_HISTROY_COUNT 100
		ZBasicInfoItem *pitem=new ZBasicInfoItem;
		pitem->info.position = GetPosition();
		pitem->info.direction = GetDirection();
		pitem->info.velocity = GetVelocity();

		pitem->fSendTime = pitem->fReceivedTime = m_pGame->GetTime();
		m_BasicHistory.push_back(pitem);

		while(m_BasicHistory.size()>ACTOR_HISTROY_COUNT)
		{
			delete *m_BasicHistory.begin();
			m_BasicHistory.pop_front();
		}
	}
}

void ZActorWithFSM::OnBasicInfo(ZACTOR_WITHFSM_BASICINFO* pbi)
{
	if (!IsMyControl())
	{
		SetPosition(rvector(pbi->posx,pbi->posy,pbi->posz));
		SetVelocity(rvector(pbi->velx,pbi->vely,pbi->velz));
		SetDirection(1.f/32000.f * rvector(pbi->dirx,pbi->diry,pbi->dirz));

		IActorAction* pAction = m_pActionMgr->GetActionByIndex(pbi->nActionIndex);
		m_pCurrAction = pAction;
		SetAnimationInAction(pAction);
	}
}

void ZActorWithFSM::OnExitState()
{
	// Ż�� func ����
	int numExitFunc = m_pCurrState->GetExitFunctionCallCount();
	for (int i=0; i<numExitFunc; ++i)
	{
		const ZFSMFunctionCall* pFunc = m_pCurrState->GetExitFunctionCall(i);
		ExecuteFunction(0, pFunc->id, pFunc->nArg[0], pFunc->nArg[1], pFunc->nArg[2], pFunc->nArg[3]);
	}
	// �̹� ���¿��� �ѵ� �̵��� �÷��װ� ���� �� ������ ����
	SetMoving(false);
}

void ZActorWithFSM::OnEnterState()
{
	m_stateEnteredTimeStamper.SetEnteredTime(m_pCurrState, timeGetTime());

	// ���� func ����
	int numEnterFunc = m_pCurrState->GetEnterFunctionCallCount();
	for (int i=0; i<numEnterFunc; ++i)
	{
		const ZFSMFunctionCall* pFunc = m_pCurrState->GetEnterFunctionCall(i);
		ExecuteFunction(0, pFunc->id, pFunc->nArg[0], pFunc->nArg[1], pFunc->nArg[2], pFunc->nArg[3]);
	}

	m_bPostedActionExecute = false;
	m_bPostedRangeShotInAction = false;
	m_RealPositionBefore = rvector(0,0,0);

	// �׼� ������ �˷��� �� peer�� �� �׼ǿ� ���Ե� ������ ó���ϵ��� �Ѵ�
	int nActionIndex = -1;
	IActorAction* pAction = m_pCurrState->GetAction();
	if (pAction)
		nActionIndex = m_pActionMgr->GetIndexOfAction(pAction);
	ZPostNewQuestPeerNpcActionExecute(m_UID, nActionIndex);
}

void ZActorWithFSM::RotateToDirection(rvector vTargetDir, float fFrameDelta)
{
	float fRotSpeed = 1.0f;
	if (m_pActorDef)
		fRotSpeed = m_pActorDef->GetRotSpeed();

	rmatrix mat;
	rvector vMyDir = GetDirection();
	float fAngle=GetAngleOfVectors(vTargetDir, vMyDir);
	float fRotAngle = fFrameDelta * fRotSpeed;

	if (fAngle > 0.0f) fRotAngle = -fRotAngle;
	if (fabs(fRotAngle) > fabs(fAngle)) 
	{
		fRotAngle = -fAngle;
	}
	D3DXMatrixRotationZ(&mat, fRotAngle);

	SetDirection(vMyDir * mat);
}



void ZActorWithFSM::AddLandingEffect( rvector vPos, rvector vDir )
{
	ZGetEffectManager()->AddLandingEffect(vPos,vDir);
}

ZOBJECTHITTEST ZActorWithFSM::HitTest( const rvector& origin, const rvector& to, float fTime,  rvector *pOutPos )
{
	//todok ������ ��� �޽���ŷ�ϵ��� �ؾ� �Ѵ�. def xml �� �޽���ŷ ���θ� ������ �� �ֵ��� �ϱ�
	return HitTest_Actor(m_pActorDef->GetMeshPicking(), origin, to, fTime, pOutPos);
}
// Custom: Flip NPC CQ FlipNPC
#ifndef  _BLASTCHALLENGENPC
void ZActorWithFSM::OnBlast(rvector& dir)
{
	if (!IsMyControl()) return;
	if (!m_pActorDef) { _ASSERT(0); return; }
	if (m_pActorDef->IsNeverBlasted()) return;
	if (!m_pFsm) return;
	ZFSMState* pStateBlastedSoar = m_pFsm->GetState(FSM_BISTATE_BLASTEDSOAR);

	mlog("\n\n\n pStateBlastedSoar: %s \n\n\n", pStateBlastedSoar);

	if (!pStateBlastedSoar) { _ASSERT(0); return; }
	m_blastedDir = dir;
	OnExitState();
	m_pCurrState = pStateBlastedSoar;
	OnEnterState();

	mlog("\n\n\n m_pCurrState: %s \n\n\n", m_pCurrState);
}

void ZActorWithFSM::OnBlastDagger(rvector& dir, rvector& pos)
{
	if (!IsMyControl()) return;
	if (!m_pActorDef) { _ASSERT(0); return; }
	if (m_pActorDef->IsNeverBlasted()) return;
	if (!m_pFsm) return;
	ZFSMState* pStateBlastedThrust = m_pFsm->GetState(FSM_BISTATE_BLASTEDTHRUST);

	mlog("\n\n\n pStateBlastedThrust: %s \n\n\n", pStateBlastedThrust);

	if (!pStateBlastedThrust) { _ASSERT(0); return; }
	m_daggerBlastedDir = dir;
	m_daggerBlastedPos = pos;
	OnExitState();
	m_pCurrState = pStateBlastedThrust;
	OnEnterState();

	mlog("\n\n\n m_pCurrStateDagger: %s \n\n\n", m_pCurrState);
}
#endif
//void ZActorWithFSM::OnPeerMeleeShot(const char* szActionName)
//{
//	//todok del
//	if (!m_pActionMgr) { _ASSERT(0); return; }
//
//	IActorAction* pAction = m_pActionMgr->GetAction(szActionName);
//	if (!pAction) { _ASSERT(0); return; }
//
//	const ZActorActionMeleeShot* pMeleeShot = pAction->GetMeleeShot();
//	if (!pMeleeShot) { _ASSERT(0); return; }
//
//	pMeleeShot->ProcessShot(m_pGame, m_UID, ZGetGame()->GetTime());
//}

void ZActorWithFSM::OnPeerActionExecute(int nActionIndex)
{
	m_pCurrAction = NULL;

	if (!m_pActionMgr) { _ASSERT(0); return; }

	IActorAction* pAction = m_pActionMgr->GetActionByIndex(nActionIndex);
	if (!pAction) { return; }

	m_pCurrAction = pAction;
	m_pCurrActionStartTime = ZGetGame()->GetTime();
	m_setProcessedShotInAction.clear();
	m_setProcessedEffectInAction.clear();
	m_setProcessedSoundInAction.clear();
	m_setProcessedSummonInAction.clear(); // Custom: added summoning
}

void ZActorWithFSM::OnDamaged(ZObject* pAttacker, rvector srcPos, ZDAMAGETYPE damageType, MMatchWeaponType weaponType, float fDamage, float fPiercingRatio, int nMeleeType)
{
	if (IsMyControl())
	{
		ZCharacterObject* pCObj = MDynamicCast(ZCharacterObject, pAttacker);
		if (!pCObj)
			return;
		if (pCObj->GetTeamID() == m_nTeamID)
			return;

		if (damageType == ZD_KATANA_SPLASH)
		{
				ZC_ENCHANT etype = pCObj->GetEnchantType();
				if (etype == ZC_ENCHANT_LIGHTNING)
					OnDamaged_LightningSplash();
		}
		m_fGroggy += fDamage;

		//	ZGetEffectManager()->AddWithScale("ef_damage07.elu", this->GetVisualMesh()->GetBipTypePosition(, this->GetDirection(), m_UID, rvector(0.2,0.2,0.2));
	}
	ZObject::OnDamaged(pAttacker, srcPos, damageType, weaponType, fDamage, fPiercingRatio, nMeleeType);
}

void ZActorWithFSM::OnDamaged_LightningSplash()
{
	if (!IsMyControl()) return;
	if (!m_pFsm) { _ASSERT(0); return; }
	
	ZFSMState* pStatetLightningStun = m_pFsm->GetState(FSM_BISTATE_LIGHTNINGSTUN);
	if (!pStatetLightningStun) { _ASSERT(0); return; }

	OnExitState();
	m_pCurrState = pStatetLightningStun;
	OnEnterState();

	SetVelocity(0,0,0);
};

//////////////////////////////////////////////////////////////////////////
//		IFSMConditionSource ����
//////////////////////////////////////////////////////////////////////////

bool ZActorWithFSM::Cond_IsEndAction()
{
	if (m_pVMesh)
	{
		if (m_pVMesh->isOncePlayDone())
			return true;
	}
	return false;
}

bool ZActorWithFSM::Cond_IsLanding()
{
	return m_pModule_Movable->isLanding();
}


bool ZActorWithFSM::Cond_HasTarget()
{
	if (m_uidTarget.IsValid())
	{
		return true;
	}

	return false;
}

bool ZActorWithFSM::Cond_HasNoTarget()
{
	if (!Cond_HasTarget())
	{
		mlog("Actor has no target");
		return true;
	}
	return false;
}

bool ZActorWithFSM::Cond_TargetHeightHigher(float fHeight)
{
	if (m_uidTarget.IsValid())
	{
		ZCharacter* pChar = (ZCharacter*) m_pGame->GetCharacterMgr()->Find(m_uidTarget);
		int nDiff =  pChar->GetPosition().z - GetPosition().z;
		if(nDiff > fHeight)
			return true;
	}
	return false;
}

bool ZActorWithFSM::CanSee( ZObject* pTarget )
{
	// �þ߿� ���̴��� Ȯ��

	if(pTarget && !ZGetGame()->CheckWall(pTarget,this,true))
	{
	rvector vTargetDir = pTarget->GetPosition() - GetPosition();
	rvector vBodyDir = GetDirection();
	vBodyDir.z = vTargetDir.z = 0.0f;

	float angle = fabs(GetAngleOfVectors(vTargetDir, vBodyDir));
	if (angle <= 25) return true;
	}

	return false;
}


bool ZActorWithFSM::Cond_CanSeeTarget()
{
	if (!m_pGame) { _ASSERT(0); return false; }
	if (m_uidTarget.IsInvalid()) return false;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return false;

	return CanSee(pChar);
}

bool ZActorWithFSM::Cond_CannotSeeTarget()
{
	if (!m_pGame) { _ASSERT(0); return false; }
	if (m_uidTarget.IsInvalid()) return false;
	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!CanSee(pChar))
	{
		return true;
	}
	return false;
}

bool ZActorWithFSM::Cond_FailedBuildWayPoints()
{
	if (m_listWaypoint.size() == 0)
	{
		return true;
	}
	return false;
}


void ZActorWithFSM::Cond_GetTargetPos( rvector& out )
{
	out = rvector(0,0,0);

	if (!m_pGame) { _ASSERT(0); return; }
	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return;

	out = pChar->GetPosition();
}

bool ZActorWithFSM::Cond_AngleTargetHeight(float fActorPosLow, float fActorPosHigh )
{
	rvector out = rvector(0,0,0);
	rvector out2 = rvector(0,0,0);

	if (!m_pGame) { _ASSERT(0); return false; }
	if (m_uidTarget.IsInvalid()) return false;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return false;

	out = pChar->GetPosition() - GetPosition();
	out2 = GetPosition();

	float angle = fabs(GetAngleOfVectors(out, out2));
	if (angle <= fActorPosHigh && angle >= fActorPosLow) return true;
	return false;
}

bool ZActorWithFSM::Cond_CheckStateCooltime( ZFSMState* pState, DWORD dwCooltime )
{
	if (!m_pFsm) { _ASSERT(0); return false; }

	return m_stateEnteredTimeStamper.CheckCooltime(pState, dwCooltime);
}

DWORD ZActorWithFSM::Cond_GetStateEnteredTime()
{
	if (!m_pFsm) { _ASSERT(0); return false; }
	if (!m_pCurrState) { _ASSERT(0); return false; }

	return m_stateEnteredTimeStamper.GetEnteredTime(m_pCurrState);
}

bool ZActorWithFSM::IsActiveLightningDamageModule()
{
	return IsActiveModule(ZMID_LIGHTNINGDAMAGE);
}

bool ZActorWithFSM::Cond_Pick(float fAngleDegree, float fDist)
{
	rvector pos = GetPosition() + rvector(0,0,100);
	rvector dir = GetDirection();
	dir.z = 0;
	D3DXMATRIX mat;
	D3DXMatrixRotationZ(&mat, ToRadian(fAngleDegree));
	dir = dir*mat;
	Normalize(dir);

	ZPICKINFO pickinfo;
	memset(&pickinfo,0,sizeof(ZPICKINFO));

	const DWORD dwPickPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE;

	if (m_pGame->Pick(this,pos, dir, &pickinfo, dwPickPassFlag)) 
	{
		rvector pickpos;

		if (pickinfo.bBspPicked)
		{
			pickpos = pickinfo.bpi.PickPos;
		}
		else if (pickinfo.pObject)
		{
			pickpos = pickinfo.pObject->GetPosition();
			pickpos.z += 100;
		}

		float fDistSq = fDist * fDist;
		if (MagnitudeSq(pos - pickpos) < fDistSq)
			return true;
	}
	return false;
}

//////////////////////////////////////////////////////////////////////////
//		ZFSMFunctionServer ����
//////////////////////////////////////////////////////////////////////////

void ZActorWithFSM::Func_FindTarget()
{
	if (!m_pGame) { _ASSERT(0); return; }

	MUID uidTarget	= MUID(0,0);
	float fDist		= FLT_MAX;

	ZCharacterManager* pCharMgr = m_pGame->GetCharacterMgr();
	ZCharacterManager::iterator it;
	ZCharacter* pChar;
	for (it=pCharMgr->begin(); it!=pCharMgr->end(); ++it)
	{
		pChar = (ZCharacter*) it->second;
		if ( pChar->IsDie())
			continue;

		// �Ÿ��� ���Ѵ�.
		float dist = MagnitudeSq( pChar->GetPosition() - this->GetPosition());

		// �� ����� ���̸� �̳��� Ÿ������ ���Ѵ�.
		if ( dist < fDist)
		{
			fDist = dist;
			uidTarget = pChar->GetUID();
		}
	}

	m_uidTarget = uidTarget;
}

void ZActorWithFSM::Func_SetTargetLastAttacker()
{
	m_uidTarget = m_pModule_HPAP->GetLastAttacker();
}

void ZActorWithFSM::Func_RotateToTarget(float fFrameDelta)
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return;

	// �̹� �����ӿ� ȸ���� ��ŭ�� Ÿ���� ���� ȸ��
	rvector targetDir(pChar->GetPosition() - GetPosition());
	Normalize(targetDir);

	RotateToDirection(targetDir, fFrameDelta);
}

void ZActorWithFSM::Func_FaceToTarget()
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pChar = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pChar) return;

	// �̹� �����ӿ� ȸ���� ��ŭ�� Ÿ���� ���� ȸ��
	rvector targetDir(pChar->GetPosition() - GetPosition());
	targetDir.z = 0;
	Normalize(targetDir);

	SetDirection(targetDir);
}

void ZActorWithFSM::Func_ReduceGroggy(float f)
{
	m_fGroggy -= f;

	//todok del
	//mlog("��ACTOR_GROGGY : %f\n", m_fGroggy);
}

void ZActorWithFSM::Func_BuildWaypointsToTarget()
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pTarget = m_pGame->GetCharacterMgr()->Find(m_uidTarget);
	if (!pTarget) return;

	// Make path
	ZNavigationMesh navMesh = m_pGame->GetNavigationMesh();
	if ( navMesh.IsNull())
		return;

	// Make navigation path
	rvector tarpos = pTarget->GetPosition();
	if ( !navMesh.BuildNavigationPath( GetPosition(), tarpos))
		return;

	m_listWaypoint.clear();
	for (list<rvector>::iterator it=navMesh.GetWaypointList().begin(); it!=navMesh.GetWaypointList().end(); ++it)
		m_listWaypoint.push_back(*it);

	//AdjustWayPointWithBound(m_WayPointList, navMesh);
}

void ZActorWithFSM::Func_RunAlongTargetOrbital(float fFrameDelta, int nArg1)
{
	if (!ZGetGame())
	{
		_ASSERT(0);
		return;
	}


	if (m_uidTarget.IsInvalid())
		return;

	ZCharacterObject* pTarget = ZGetGame()->GetCharacterMgr()->Find(m_uidTarget);
	if (!pTarget)
		return;

	//Old Method
	rvector vDistance = pTarget->GetPosition() - GetPosition();
	vDistance.x += vDistance.x < 0.f ? RandomNumber(nArg1 / 6, nArg1) : -RandomNumber(nArg1 / 6, nArg1);
	vDistance.y += vDistance.y < 0.f ? RandomNumber(nArg1 / 6, nArg1) : -RandomNumber(nArg1 / 6, nArg1);
	vDistance.z = 0;

	//New Method
	//rmatrix mat;
	//MakeWorldMatrix(&mat, pTarget->GetPosition(), pTarget->GetDirection(), rvector(0, 0, 1));

	ZNavigationMesh navMesh = ZGetGame()->GetNavigationMesh();
	if (navMesh.IsNull())
		return;

	if (navMesh.BuildNavigationPath(GetPosition(), GetPosition() + vDistance))
	{
		m_listWaypoint.clear();
		for (list<rvector>::iterator it = navMesh.GetWaypointList().begin(); it != navMesh.GetWaypointList().end(); ++it)
			m_listWaypoint.push_back(*it);



		Func_RunWaypoints(fFrameDelta);
	}
}

void ZActorWithFSM::Func_RunWaypoints(float fFrameDelta)
{
	if (m_listWaypoint.empty()) return;
	if (!m_pActorDef) return;


	rvector nextWaypoint = *m_listWaypoint.begin();
	rvector myPos = GetPosition();

	nextWaypoint.z = 0;
	myPos.z = 0;

	float diffSq = MagnitudeSq(nextWaypoint - myPos);
	if (diffSq <= 20.f*20.f)
	{
		m_listWaypoint.pop_front();
		Func_RunWaypoints(fFrameDelta);	// ��������Ʈ�� ���������� ���ȣ��� ���� ��������Ʈ�� ���� ó���� �ϰ� �Ѵ�
		return;
	}

	//rvector dir = nextWaypoint - myPos;
	//Normalize(dir);
	//SetDirection(dir);

	rvector vWaypointDir = nextWaypoint - myPos;
	Normalize(vWaypointDir);

	// ���� ��������Ʈ�� ���� ���� ��ȯ
	RotateToDirection(vWaypointDir, fFrameDelta);

	// ��������Ʈ ���� �̵�
	//float fSpeed = m_pActorDef->GetSpeed();
	//rvector newPos = vWaypointDir * (fSpeed * fFrameDelta) + myPos;
	//SetPosition(newPos);
	SetMoving(true);
}

void ZActorWithFSM::Func_Disappear(float fFrameDelta, DWORD nDelay, float fRatio)
{
	if (!m_pVMesh) { _ASSERT(0); return; }
	if (!m_pCurrState) { _ASSERT(0); return; }

	DWORD elapsed = timeGetTime() - Cond_GetStateEnteredTime();

	// �����̸�ŭ ��ٷȴٰ� ����ȭ ����
	if (elapsed >= nDelay)
	{
		elapsed -= nDelay;

		float fOpacity = 1.f - fRatio * (elapsed*0.001f);

		if (fOpacity > 1.f) fOpacity = 1.f;
		if (fOpacity < 0) fOpacity = 0;

		m_pVMesh->SetVisibility(fOpacity);
	}
}

void ZActorWithFSM::Func_PostDead()
{
	if (IsMyControl())
		ZPostNewQuestRequestNPCDead(m_pModule_HPAP->GetLastAttacker(), GetUID(), GetPosition());
	else
		_ASSERT(0);
}

void ZActorWithFSM::Func_OnEnterBlastedSoar()
{
	rvector dir = GetDirection();
	dir.x = -m_blastedDir.x;
	dir.y = -m_blastedDir.y;
	SetDirection(dir);

	SetVelocity(m_blastedDir * 300.f + rvector(0,0, (float)RandomNumber( 1500, 1900)));
	//m_bBlasting = true;
	//SetFlag(AF_LAND, false); //todok land �÷��� ��������� ó������
}

void ZActorWithFSM::Func_OnEnterBlastedThrust()
{
	rvector dir = GetDirection();
	dir.x = -m_daggerBlastedDir.x;
	dir.y = -m_daggerBlastedDir.y;
	SetDirection(dir);

	Normalize(m_daggerBlastedDir);
	AddVelocity(m_daggerBlastedDir * (RUN_SPEED * 4));

	m_bSuffering = true;

	/*rvector dir = GetDirection();
	dir.x = -m_daggerBlastedDir.x;
	dir.y = -m_daggerBlastedDir.y;
	SetDirection(dir);

	SetVelocity(m_daggerBlastedDir * 300.f + rvector(0,0,100.f));

	m_vAddBlastVel = GetPosition() - m_daggerBlastedPos;
	m_vAddBlastVel.z = 0.f;

	Normalize(m_vAddBlastVel);

	m_fAddBlastVelTime = m_pGame->GetTime();

	m_bSuffering = true;
	//SetFlag(AF_LAND, false); //todok land �÷��� ��������� ó������*/
}

float accum_fBlastedThrustVelSubtract = 0;

void ZActorWithFSM::Func_OnUpdateBlastedThrust(float fFrameDelta)
{
	_ASSERT(IsSuffering());

	// �ð��� �帧�� ���� �ӵ����� x, y ������ ���ӽ�Ų��
	rvector dir=rvector(GetVelocity().x,GetVelocity().y,0);
	float fSpeed = Magnitude(dir);
	Normalize(dir);

	const float fBlastThrustBreak = 4000.f;

	fSpeed -= fBlastThrustBreak * fFrameDelta;
	if (fSpeed < 0)
		fSpeed = 0;

	SetVelocity(dir.x*fSpeed, dir.y*fSpeed, GetVelocity().z);

	//// �ð��� �帧�� ���� �ӵ����� x, y ������ ���ӽ�Ų��
	//rvector vel = GetVelocity();
	//vel.x *= 0.9;
	//vel.y *= 0.9;
	//SetVelocity(vel);

	/*float subtractRatio = 0.9f * fFrameDelta;
	accum_fBlastedThrustVelSubtract += subtractRatio;
	if (accum_fBlastedThrustVelSubtract > 0.1f)
	{
		vel.x -= vel.x * subtractRatio;
		vel.y -= vel.y * subtractRatio;

		SetVelocity(vel);

		accum_fBlastedThrustVelSubtract = 0;
	}*/
	
	//vel.x -= 500.f * fFrameDelta;
	//vel.y -= 500.f * fFrameDelta;

	//todok del
	if (IsSuffering())
	{
		mlog("* suffering vel setted :%f %f %f\n", GetVelocity().x, GetVelocity().y, GetVelocity().z);
	}

/*
#define BLAST_DAGGER_MAX_TIME 0.8f

	float fTime = max((1.f - (m_pGame->GetTime() - m_fAddBlastVelTime) / BLAST_DAGGER_MAX_TIME),0.0f);

	if( fTime < 0.4f )
		fTime = 0.f;

	float fPower = 400.f * fTime * fTime * fFrameDelta * 80.f;

	//todok �Ʒ� �ּ��� �ʿ��ҷ���? Ȯ���� �ʿ�
//	if(fPower==0.f)
//		SetFlag(AF_BLAST_DAGGER,false);//���� �ٵȰ�

	rvector vel = m_vAddBlastVel * fPower;

	SetVelocity(vel);*/
}

void ZActorWithFSM::Func_ExitSuffering()
{
	m_bSuffering = false;
}

void ZActorWithFSM::Func_OnEnterDie()
{
	SetDeadTime(m_pGame->GetTime());
}

void ZActorWithFSM::Func_FindTargetInHeight(int nHeight)
{
	if (!m_pGame) { _ASSERT(0); return; }

	MUID uidTarget	= MUID(0,0);
	float fDist		= FLT_MAX;

	ZCharacterManager* pCharMgr = m_pGame->GetCharacterMgr();
	ZCharacterManager::iterator it;
	ZCharacter* pChar;
	for (it=pCharMgr->begin(); it!=pCharMgr->end(); ++it)
	{
		pChar = (ZCharacter*) it->second;
		if ( pChar->IsDie())
			continue;

		// �Ÿ��� ���Ѵ�.
		float dist = MagnitudeSq( pChar->GetPosition() - this->GetPosition());

		// �� ����� ���̸� �̳��� Ÿ������ ���Ѵ�.
		int nDiff = pChar->GetPosition().z - this->GetPosition().z;

		if ( dist < fDist && (nDiff > -nHeight && nDiff < nHeight))
		{
			fDist = dist;
			uidTarget = pChar->GetUID();
		}
	}

	m_uidTarget = uidTarget;
}


void ZActorWithFSM::Func_SpeedAccel(int nArg1)
{
	/*
	rvector dir=rvector(GetVelocity().x,GetVelocity().y,0);
	//float fSpeed = Magnitude(dir);
	Normalize(dir);
	SetVelocity(dir.x+fFrameDelta, dir.y*fFrameDelta, GetVelocity().z);
	*/
	SetSpeedAccel(nArg1);
}

void ZActorWithFSM::Func_ClearWayPoints()
{
	if (m_listWaypoint.empty()) return;
	m_listWaypoint.clear();
}

void ZActorWithFSM::Func_TurnOrbitalDir()
{
	rvector newDir(GetDirection());
	newDir.z = 0;
	Normalize(newDir);
	newDir.x = (0.f - newDir.x);
	newDir.y = (0.f - newDir.y);
	if (newDir.x > 1.f || newDir.x < -1.f)
	{
		newDir.x = 0.f;
	}
	if (newDir.y > 1.f || newDir.y < -1.f)
	{
		newDir.y = 0.f;
	}
	//RotateToDirection(newDir)
	SetDirection(newDir);
}

bool ZActorWithFSM::Cond_SummonLess(int nArg1)
{
	ZRuleQuestChallenge* pCQRule = NULL;
	if (ZGetGame()->GetMatch()->GetRule())
		pCQRule = (ZRuleQuestChallenge*)ZGetGame()->GetMatch()->GetRule();
	//Arg1 is int number of npcs to spawn.
	if (m_bAdjustPlayerNum)
	{
		//
	}
	if(IsMyControl() && pCQRule->GetNPC() - 1 < nArg1) //ignore boss
	{
		return true;
	}
	return false;
}

//BLitzKrieg

void ZActorWithFSM::Func_FindTargetInDist(int nDist)
{
	if (!m_pGame) { _ASSERT(0); return; }

	MUID uidTarget = MUID(0, 0);
	float fDist = FLT_MAX;


	for (auto itor = ZGetObjectManager()->begin(); itor != ZGetObjectManager()->end(); ++itor)
	{
		ZCharacterObject* pObj = (ZCharacterObject*)(*itor).second;
		if (pObj->IsDie())
			continue;

		// �Ÿ��� ���Ѵ�.
		float dist = MagnitudeSq(pObj->GetPosition() - this->GetPosition());

		// �� ����� ���̸� �̳��� Ÿ������ ���Ѵ�.
		int nDiff = pObj->GetPosition().x - this->GetPosition().x;

		if (dist < fDist && (nDiff > -nDist && nDiff < nDist))
		{
			fDist = dist;
			uidTarget = pObj->GetUID();
		}
	}

	m_uidTarget = uidTarget;
}

void ZActorWithFSM::Func_RunWayPointsAlongRoute(float fFrameDelta)
{
	//MMatchBlitzKrieg* pBlitzRule = (MMatchBlitzKrieg*)ZGetGame()->GetMatch()->GetRule();
	//if (pBlitzRule)
	//{
	//	int nRoute = pBlitz->GetNpcRoute();
	//rvector nextWaypoint = *m_listWaypoint.begin();
	//rvector myPos = GetPosition();

	//nextWaypoint.z = 0;
	//myPos.z = 0;

	//float diffSq = MagnitudeSq(nextWaypoint - myPos);
	//if (diffSq <= 20.f*20.f)
	//{
	//	m_listWaypoint.pop_front();
	//	this->Func_RunWayPointsAlongRoute(fFrameDelta);	// ��������Ʈ�� ���������� ���ȣ��� ���� ��������Ʈ�� ���� ó���� �ϰ� �Ѵ�
	//	return;
	//}
	//}

	if (m_listWaypoint.empty()) return;
	if (!m_pActorDef) return;

	rvector nextWaypoint = *m_listWaypoint.begin();
	rvector myPos = GetPosition();

	nextWaypoint.z = 0;
	myPos.z = 0;

	float diffSq = MagnitudeSq(nextWaypoint - myPos);
	if (diffSq <= 20.f*20.f)
	{
		m_listWaypoint.pop_front();
		this->Func_RunWayPointsAlongRoute(fFrameDelta);	// ��������Ʈ�� ���������� ���ȣ��� ���� ��������Ʈ�� ���� ó���� �ϰ� �Ѵ�
		return;
	}

	//rvector dir = nextWaypoint - myPos;
	//Normalize(dir);
	//SetDirection(dir);

	rvector vWaypointDir = nextWaypoint - myPos;
	Normalize(vWaypointDir);

	// ���� ��������Ʈ�� ���� ���� ��ȯ
	RotateToDirection(vWaypointDir, fFrameDelta);

	// ��������Ʈ ���� �̵�
	//float fSpeed = m_pActorDef->GetSpeed();
	//rvector newPos = vWaypointDir * (fSpeed * fFrameDelta) + myPos;
	//SetPosition(newPos);
	SetMoving(true);
}

void ZActorWithFSM::Func_FaceToLatestAttacker()
{
	if (!m_pGame) { _ASSERT(0); return; }

	if (m_uidTarget.IsInvalid()) return;

	ZCharacterObject* pObj = (ZCharacterObject*)ZGetObjectManager()->GetObjectA(GetLastAttacker());
	if (!pObj) return;

	// �̹� �����ӿ� ȸ���� ��ŭ�� Ÿ���� ���� ȸ��
	rvector targetDir(pObj->GetPosition() - GetPosition());
	targetDir.z = 0;
	Normalize(targetDir);

	SetDirection(targetDir);
}

bool ZActorWithFSM::Save(ZFile* pFile)
{
	// TODO: Making recordings work in challengequest
	return false;
}