#include "stdafx.h"

#include "ZGame.h"
#include "ZWeapon.h"
#include "RBspObject.h"

#include "ZEffectBillboard.h"
#include "ZEffectSmoke.h"

#include "ZSoundEngine.h"
#include "ZApplication.h"

#include "MDebug.h"
#include "ZConfiguration.h"
#include "RealSoundEffect.h"

#include "ZEffectFlashBang.h"
#include "ZPost.h"
#include "ZStencilLight.h"
#include "ZScreenDebugger.h"

#include "ZGameConst.h"

#include "ZModule_FireDamage.h"
#include "ZModule_ColdDamage.h"
#include "ZModule_LightningDamage.h"
#include "ZModule_PoisonDamage.h"
#include "ZActorWithFSM.h"


#define BOUND_EPSILON	5
#define LANDING_VELOCITY	20
#define MAX_ROT_VELOCITY	50


#define ROCKET_VELOCITY			2700.f	// 2004ｳ・9ｿ・21ﾀﾏﾀﾚ ﾀﾌﾀ・ｺ 2500.f
#define ROCKET_SPLASH_RANGE		350.f	// ｷﾎﾄﾏ ｽｺﾇﾃｷ｡ｽﾃ ｹ・ｧ
#define ROCKET_MINIMUM_DAMAGE	.3f		// ｷﾎﾄﾏ ｹﾌｴﾏｸﾘ ｵ･ｹﾌﾁ・
#define ROCKET_KNOCKBACK_CONST	.5f		// ｷﾎﾄﾏｿ｡ ﾆｨｰﾜﾁｮ ｳｪｰ｡ｴﾂ ｻ�ｼ・

MImplementRootRTTI(ZWeapon);

ZWeapon::ZWeapon() : m_pVMesh(NULL), m_nWorldItemID(0), m_WeaponType(ZWeaponType_None), m_SLSid(0) {
	m_nItemUID = -1;
}

ZWeapon::~ZWeapon() {
	if(m_pVMesh){
		delete m_pVMesh;
		m_pVMesh = NULL;
	}

	if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
		ZGetStencilLight()->DeleteLightSource( m_SLSid );
		m_SLSid = 0;
	}
}

void ZWeapon::Create(RMesh* pMesh) {
	m_pVMesh = new RVisualMesh;
	m_pVMesh->Create(pMesh);
}

void ZWeapon::Render() {
	m_pVMesh->Frame();
	m_pVMesh->Render();
}

bool ZWeapon::Update(float fElapsedTime)
{
	return true;
}

////////////////////////////////////////////////////////////////////////////

MImplementRTTI(ZMovingWeapon, ZWeapon);

ZMovingWeapon::ZMovingWeapon() : ZWeapon() {
	m_WeaponType = ZWeaponType_MovingWeapon;
	m_PostPos = rvector(-1,-1,-1);
}

ZMovingWeapon::~ZMovingWeapon() {
}

void ZMovingWeapon::Explosion() {
}

/////////////////////////// ｷﾎﾄﾏ

MImplementRTTI(ZWeaponRocket,ZMovingWeapon);

ZWeaponRocket::ZWeaponRocket() :ZMovingWeapon() {

}

ZWeaponRocket::~ZWeaponRocket() {

}

void ZWeaponRocket::Create(RMesh* pMesh,rvector &pos, rvector &dir,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_Position=pos;
	m_Velocity=dir*ROCKET_VELOCITY;

	m_fStartTime = ZGetGame()->GetTime();
	m_fLastAddTime = ZGetGame()->GetTime();

	m_Dir=dir;
	m_Up=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage.Ref();

	if( Z_VIDEO_DYNAMICLIGHT ) {
		_ASSERT(m_SLSid==0);
		m_SLSid = ZGetStencilLight()->AddLightSource( m_Position, 2.0f );
	}
	CHECK_RETURN_CALLSTACK(Create);
	//ZGetEffectManager()->AddRocketSmokeEffect(m_Position,-RealSpace2::RCameraDirection);
}

#define ROCKET_LIFE			10.f		// 10ﾃﾊ ｵﾚｿ｡ ﾅﾍﾁ・


bool ZWeaponRocket::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;

	if(ZGetGame()->GetTime() - m_fStartTime > ROCKET_LIFE ) {
		// ｼ�ｷ�ﾅｺﾀｺ ﾅﾍﾁ�ｴﾂ ｼ�ｰ｣ｿ｡ ﾀﾌﾆ衄ｮ ﾃﾟｰ｡.. ｻ霖ｦ...
		Explosion();

		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
		}

		return false;
	}

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos;
		ZPICKINFO zpi;
		bool bPicked=ZGetGame()->Pick(ZGetObjectManager()->GetObject(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
				pickpos=zpi.bpi.PickPos;
			else
				if(zpi.pObject)
					pickpos=zpi.info.vOut;

		}
/*
		RBSPPICKINFO bpi;
		bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position,dir,&bpi,dwPickPassFlag);
*/
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			Explosion();

			if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
			{
				ZGetStencilLight()->DeleteLightSource( m_SLSid );
				m_SLSid = 0;
				ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
			}

			return false;
		}else
			m_Position+=diff;
	}

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetWorldMatrix(mat);

	float this_time = ZGetGame()->GetTime();

	if( this_time > m_fLastAddTime + 0.02f ) {

#define _ROCKET_RAND_CAP 10

		rvector add = rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		rvector pos = m_Position + 20.f*add;

		ZGetEffectManager()->AddRocketSmokeEffect(pos);
//		ZGetEffectManager()->AddSmokeEffect(NULL,pos,rvector(0,0,0),rvector(0,0,0),60.f,80.f,1.5f);
		ZGetWorld()->GetFlags()->CheckSpearing( oldPos, pos	, ROCKET_SPEAR_EMBLEM_POWER );
		m_fLastAddTime = this_time;
	}

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->SetLightSourcePosition( m_SLSid, m_Position	);

	return true;
}

void ZWeaponRocket::Render()
{
	ZWeapon::Render();

	//add

//	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);
//	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

//	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
//	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
//	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

	//alpha
/*
	RGetDevice()->SetRenderState(D3DRS_ALPHAREF, (DWORD)0x00000000);
	RGetDevice()->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_NOTEQUAL );
	RGetDevice()->SetRenderState(D3DRS_ALPHATESTENABLE,	TRUE);

	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG1 , D3DTA_TEXTURE );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAARG2 , D3DTA_TFACTOR );
	RGetDevice()->SetTextureStageState( 0, D3DTSS_ALPHAOP , D3DTOP_MODULATE );

	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
*/

}

void ZWeaponRocket::Explosion()
{
	// ﾀﾚｽﾅﾀﾇ ﾀﾌﾆ衄ｮ ｱﾗｸｮｰ・.
	// ﾁﾖｺｯ ﾀｯｴﾖｵ鯀｡ｰﾔ ｵ･ｹﾌﾁ・ﾁﾖｴﾂ ｸﾞｽﾃﾁ・ｳｯｸｮｰ・.
	// ( ｸﾞｽﾃﾁ・ｹﾞﾀｺｰ�ｿ｡ｼｭ ﾁﾖｺｯ ﾀｯｴﾖｵ・ﾃｼﾅｩﾇﾏｰ・. 
	// ﾁﾗﾀｺ ﾀｯｴﾖﾀﾌ ﾀﾖｴﾙｸ・ｹﾟｻ鄲ｯｴﾖﾀﾇ ﾅｳｼ�ｸｦ ｿﾃｷﾁﾁﾖｱ・

	rvector v = m_Position;

	//	if(v.z < 0.f)	v.z = 0.f;//ｳ�ﾀﾌ

	rvector dir = -RealSpace2::RCameraDirection;
	ZGetEffectManager()->AddRocketEffect(v,dir);

	// ｸﾞｽﾃﾁ�ｸｦ ｳｯｸｮｰﾅｳｪ~
	// ｰ｢ｰ｢ﾀﾇ ﾅｬｶ�ﾀﾌｾ�ﾆｮﾀﾇ ｹｫｱ・ﾁ､ｺｸｸｦ ｹﾏｰ・ﾃｳｸｮﾇﾏｰﾅｳｪ~

	ZGetGame()->OnExplosionGrenade(m_uidOwner,v,m_fDamage,ROCKET_SPLASH_RANGE,ROCKET_MINIMUM_DAMAGE,ROCKET_KNOCKBACK_CONST,m_nTeamID);

	ZGetSoundEngine()->PlaySound("fx_explosion01",v);

	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );

}

////////////////////////// ｸﾞｵ�ﾅｶ

MImplementRTTI(ZWeaponItemKit,ZMovingWeapon);

ZWeaponItemKit::ZWeaponItemKit() :ZMovingWeapon()
{
//	m_nWeaponType = 0;
	m_fDeathTime = 0;
	m_nWorldItemID = 0;

	m_bInit = false;
	m_bDeath = false;
	m_bSendMsg = false;
	m_bWorldItemFloorDrop = false;
}

ZWeaponItemKit::~ZWeaponItemKit() 
{

}

void ZWeaponItemKit::Create(RMesh* pMesh,rvector &pos, rvector &velocity,ZObject* pOwner)
{
	ZWeapon::Create(pMesh);

	m_Position=pos;
	rvector dir=velocity;
	Normalize(dir);
	m_Velocity=velocity;

	m_fStartTime=ZGetGame()->GetTime();

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	m_bInit = false;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage.Ref();

//	m_nSpItemID = m_nMySpItemID++;

	m_bSendMsg = false;
	m_bWorldItemFloorDrop = false;
}

rvector GetReflectionVector(rvector& v,rvector& n) {

	float dot = D3DXVec3Dot(&(-v),&n);

	return (2*dot)*n+v;
}

void ZWeaponItemKit::Render()
{
	if(m_bInit) {
		if(m_pVMesh->GetMesh()) {
		// ｹｫｱ・ﾆﾄﾃ�ｸｦ ﾄｳｸｯﾅﾍｿ｡ ｺﾙｾ�ﾜﾖﾀｻｰ豼・ﾁｶｸ�ﾜｻ ｹﾞｰ・ｾﾈｺﾙｾ�ﾜﾖﾀｻｰ豼・ﾁｶｸ�ﾜｻ ｾﾈｹﾞｴﾂ 2ｰ｡ﾁ�ｿ・ｵｷﾎｻ鄙・ﾏｱ筝ｧｹｮ		

		rmatrix mat;
		MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);
		m_pVMesh->SetWorldMatrix(mat);
//		m_pVMesh->m_pMesh->m_LitVertexModel = true;
		ZMovingWeapon::Render();
//		m_pVMesh->m_pMesh->m_LitVertexModel = false;

		}
	}
}

void ZWeaponItemKit::UpdateFirstPos()
{
	/////////////////////////////////////// ﾁ�ｱﾝﾀｺ ｰ霆・ﾘｼｭ ｺｸｳｻﾁﾘｴﾙ...

	m_bInit = true;
	return;

	if(m_bInit==false) {

		ZCharacter* pC = (ZCharacter*) ZGetCharacterManager()->Find(m_uidOwner);

		if(pC) {
			if(pC->m_pVMesh) {

				rvector vWeapon[1];

				vWeapon[0] = pC->m_pVMesh->GetCurrentWeaponPosition();

				// ｺｮｵﾚｷﾎ ｴ�ﾁ�ｴﾂ ｰ豼・｡ ｻ�ｱ篋・
				rvector nPos = pC->m_pVMesh->GetBipTypePosition(eq_parts_pos_info_Spine1);
				rvector nDir = vWeapon[0] - nPos;

				Normalize(nDir);

				RBSPPICKINFO bpi;
				// ｾﾆｷ｡ｸｦ ｺｸｸ鮠ｭ ｴ�ﾁ�ｸ・ｺｮﾀｻ ﾅ・�ﾇﾘｼｭ...
				if(ZGetWorld()->GetBsp()->Pick(nPos,nDir,&bpi))
				{
					if(D3DXPlaneDotCoord(&(bpi.pInfo->plane),&vWeapon[0])<0) {
						vWeapon[0] = bpi.PickPos - nDir;
					}
				}

				m_Position = vWeapon[0];
			}
		}

		m_bInit = true;
	}
}

void ZWeaponItemKit::UpdatePost(DWORD dwPickPassFlag, float fDropDelayTime)
{
	if(m_uidOwner == ZGetGameClient()->GetPlayerUID()) {
		ZPostRequestSpawnWorldItem(ZGetGameClient()->GetPlayerUID(), m_nWorldItemID, m_Position, fDropDelayTime);
		m_PostPos = m_Position;
		m_bSendMsg = true;
	}
}

void ZWeaponItemKit::UpdateWorldItemFloorDrop(DWORD dwPickPassFlag)
{
	RBSPPICKINFO rpi;
	bool bPicked = ZGetWorld()->GetBsp()->Pick(m_Position,rvector(0,0,-1),&rpi, dwPickPassFlag );

	if(bPicked && fabsf(Magnitude(rpi.PickPos - m_Position)) < 5.0f ) {
		if(m_bWorldItemFloorDrop == false) {
			m_bWorldItemFloorDrop = true;
			m_fDeathTime = ZGetGame()->GetTime() + 2.0f;//ｳﾗﾆｮｿ・ﾀ・ﾛ ｽﾃｰ｣ｰ�ﾓﾁ ﾃﾖｴ・2.0ﾃﾊｵﾚｿ｡ ﾀﾚｵｿ ｼﾒｸ・
		}
	}
}

void ZWeaponItemKit::UpdatePos(float fElapsedTime,DWORD dwPickPassFlag)
{
	rvector diff = m_Velocity * fElapsedTime;

	rvector dir = diff;
	Normalize(dir);

	float fDist = Magnitude(diff);

	rvector pickpos;
	rvector normal=rvector(0,0,1);

	ZPICKINFO zpi;

	bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);

	if(bPicked) {
		if(zpi.bBspPicked)	{
			pickpos=zpi.bpi.PickPos;
			rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
			normal=rvector(plane.a,plane.b,plane.c);
		}
		else if(zpi.pObject) {
			pickpos=zpi.info.vOut;
			if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
			{
				normal=pickpos-zpi.pObject->GetPosition();
				normal.z=0;
			}
			else
				normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
			Normalize(normal);
		}
	}

	if(bPicked && fabsf(Magnitude(pickpos-m_Position)) < fDist)
	{ // ｾ�ｴ�ﾀﾌｳｪ ｳｭｰ｣ﾀｧｿ｡ｼｭ ｶ･ｿ｡ ｶｳｾ�ﾝ・ｱ篌ｻ ｶｳｾ�ﾝ�ｴﾂ ｽﾃｰ｣ｺｸｴﾙ ｸｹﾀﾌ ｰﾉｸｱｰ豼・
		m_Position = pickpos + normal;
		m_Velocity = GetReflectionVector(m_Velocity,normal);
		m_Velocity *= zpi.pObject ? 0.1f: 0.2f;	// ﾄｳｸｯﾅﾍｿ｡ ﾃ豬ｹﾇﾏｸ・ｼﾓｵｵｰ｡ ｸｹﾀﾌ ﾁﾘｴﾙ
		m_Velocity *= 0.2f;

		// ｺｮｹ貮簑ﾇ ｼﾓｵｵｼｺｺﾐﾀｻ ﾁﾙﾀﾎｴﾙ.
		Normalize(normal);
		float fAbsorb=DotProduct(normal,m_Velocity);
		m_Velocity -= 0.1*fAbsorb*normal;

		float fA=RANDOMFLOAT*2*pi;
		float fB=RANDOMFLOAT*2*pi;
		m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

	} else { // ｶｳｾ�ﾝ�ｰ�ﾜﾖﾀｽ
		m_Position+=diff;
	}
}

// ｶ･ｿ｡ ｴ・ｸｸ・ｼﾒｸ黯
bool ZWeaponItemKit::Update(float fElapsedTime)
{
	if(m_bDeath) {
		return false;
	}

	if(m_bWorldItemFloorDrop) // ｿ�ｵ・ｾﾆﾀﾌﾅﾛﾀﾌ ｶ･ｿ｡ ｶｳｾ�ﾝ�ｸ・ﾃｳｸｮﾇﾏﾁ・ｾﾊｴﾂｴﾙ.
		return true;

//	if(m_bSendMsg) { // ｿﾜｺﾎｿ｡ｼｭ bDeath ｷﾎｸｸ ﾁ�ｿ�ﾐﾙ..
////		if( g_pGame->GetTime() > m_fDeathTime ) 
////			return false;
////		else 
//			return true;
//	}

	if(ZGetGame()->GetTime() - m_fStartTime < m_fDelayTime) {
		return true;
	}

	UpdateFirstPos();

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;
	
	{	// ｿ�ｵ蠕ﾆﾀﾌﾅﾛﾀﾌ ｶｳｾ�ﾝ・ｽﾃｰ｣ｰ・ﾀｧﾄ｡ｸｦ ｹﾌｸｮ ｰ霆・ﾄ ｼｭｹ�ｿ｡ ｸﾞｼｼﾁ�ｸｦ ｹﾌｸｮ ｺｸｳｻﾁﾘｴﾙ....20090213 by kammir
		rvector vTempPos = m_Position;
		rvector vTempVelocity = m_Velocity;

		if(m_bSendMsg == false)
		{ // ｸﾞｼｼﾁ�ｸｦ ｾﾈｺｸｳﾂﾀｻｶｧｿ｡ｸｸ ｽﾇﾇ・.. ﾇﾑｹ�ｸｸ ﾀ�ｿ・ﾅｰﾚｱﾝﾇﾑｴﾙ.
			float fTick = 0.02f;
			for(int i=0; i<500; i++)
			{ // 10ﾃﾊ ﾁ､ｵｵｸｦ ｹﾌｸｮ ｿｹｻ�ﾇﾏｿｩ ｾﾆﾀﾌﾅﾛﾀﾌ ｶｳｾ�ﾝ�ｴﾂｰﾉ ﾈｮﾀﾎﾇﾑｴﾙ.
				m_Velocity.z -= 500.f * fTick;
				UpdatePos( fTick , dwPickPassFlag );		// ｾﾆﾀﾌﾅﾛﾀﾌ ｰ霈ﾓ ｶｳｾ�ﾝ�ｴﾙ.
				UpdateWorldItemFloorDrop(dwPickPassFlag);	// ｾﾆﾀﾌﾅﾛﾀﾌ ｶ･ｿ｡ ｶｳｾ�ﾝｳｴﾂﾁ・ﾈｮﾀﾎ

				if(m_bWorldItemFloorDrop == true)
				{ // ｶ･ｿ｡ ｺﾎｵ�ﾇ�ﾀｸｸ・ｼｭｹ�ｿ｡ ｸﾞｼｼﾁ�ｸｦ ｳｯｸｰｴﾙ.
					UpdatePost( dwPickPassFlag, fTick*i);
					m_bWorldItemFloorDrop = false;
					break;
				}
			}
			// ｸｸｾ・10ﾃﾊｰ｡ ﾁ�ｳｪｵｵ ｹﾙｴﾚｿ｡ ｶｳｾ�ﾝ�ﾁ・ｾﾊｾﾒﾀｸｸ・ｾﾆﾀﾌﾅﾛﾀｻ ｱﾗｳﾉ ﾁﾗｿｩﾁﾘｴﾙ.
			m_bSendMsg = true;
		}

		m_Position = vTempPos;			// ｸﾞｼｼﾁ�ｸｦ ｺｸｳｻｱ簑ｧﾇﾘ ｹﾌｸｮ ｰ霆・ﾑ ｺｯｰ豬ﾈ ｰｪﾀｻ ｿ�ｷ｡ｴ・ﾎ ｵｹｷﾁﾁﾘｴﾙ.
		m_Velocity = vTempVelocity;
	}

	// ｿ�ｵ蠕ﾆﾀﾌﾅﾛﾀﾌ ｶｳｾ�ﾝ�ｴﾂｰﾉ ﾈｭｸ鯀｡ ｺｸｿｩﾁ�ｼ�ﾀﾖｰﾔ ｰ霆・ﾘﾁﾘｴﾙ.
	m_Velocity.z -= 500.f * fElapsedTime;		// ｽﾃｰ｣ｿ｡ ｵ�ｸ･ ｶ･ｿ｡ ｶｳｾ�ﾝ�ｴﾂ ｼﾓｵｵ
	UpdatePos( fElapsedTime , dwPickPassFlag ); // ｼﾓｵｵｿﾍ ｿ�ｵ蠕ﾆﾀﾌﾅﾛﾀﾇ ｳ�ﾀﾌｸｦ ｽﾃｰ｣ｴ・ｰ霆・ﾔﾀｸｷﾎｽ・ｾﾆｷ｡ｷﾎ ｶｳｾ�ﾝ�ｴﾙ.
	UpdateWorldItemFloorDrop(dwPickPassFlag);	// ｾﾆﾀﾌﾅﾛﾀﾌ ｶ･ｿ｡ ｶｳｾ�ﾝｳｴﾂﾁ・ﾈｮﾀﾎ(ｶ･ｹﾙｴﾚﾇﾏｰ・ｿ�ｵ蠕ﾆﾀﾌﾅﾛﾀﾇ ｳ�ﾀﾌｰ｡ 5ｹﾌｸｸﾀﾌｸ・ﾃｳｸｮ)

	rmatrix mat;
	rvector dir = m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	m_pVMesh->SetWorldMatrix(mat);

	return true;
}

void ZWeaponItemKit::Explosion()
{
	// ﾀﾚｽﾅﾀﾇ ﾀﾌﾆ衄ｮ ｱﾗｸｮｰ・.
	// ﾁﾖｺｯ ﾀｯｴﾖｵ鯀｡ｰﾔ ｵ･ｹﾌﾁ・ﾁﾖｴﾂ ｸﾞｽﾃﾁ・ｳｯｸｮｰ・.
	// ( ｸﾞｽﾃﾁ・ｹﾞﾀｺｰ�ｿ｡ｼｭ ﾁﾖｺｯ ﾀｯｴﾖｵ・ﾃｼﾅｩﾇﾏｰ・. 
	// ﾁﾗﾀｺ ﾀｯｴﾖﾀﾌ ﾀﾖｴﾙｸ・ｹﾟｻ鄲ｯｴﾖﾀﾇ ﾅｳｼ�ｸｦ ｿﾃｷﾁﾁﾖｱ・

	rvector v = m_Position;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	ZGetEffectManager()->AddGrenadeEffect(v, dir);

	ZGetGame()->CheckZoneItemKit(m_uidOwner, v, m_fDamage,m_nTeamID);

	ZGetSoundEngine()->PlaySound("we_grenade_explosion", v);

	ZGetWorld()->GetFlags()->SetExplosion(v, EXPLOSION_EMBLEM_POWER);
}


////////////////////////// ｼ�ｷ�ﾅｺ

MImplementRTTI(ZWeaponGrenade,ZMovingWeapon);

void ZWeaponGrenade::Create(RMesh* pMesh,rvector &pos, rvector &velocity,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_Position=pos;
	rvector dir=velocity;
	Normalize(dir);
	m_Velocity=velocity;

	m_fStartTime=ZGetGame()->GetTime();

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() )
		if( pOwner->GetItems()->GetSelectedWeapon() )
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage=pDesc->m_nDamage.Ref();

	m_nSoundCount = rand() % 2 + 2;	// 2~ 3?? ????? ????
}


#define GRENADE_LIFE			2.f		// 3ﾃﾊ ｵﾚｿ｡ ﾅﾍﾁ・

// ｼｭｷﾎｰ｣ｿ｡ ｵｿｱ篳ｭ ｵﾇｾ�ﾚﾟ ﾇﾑｴﾙ.
bool ZWeaponGrenade::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;
	if(ZGetGame()->GetTime() - m_fStartTime > GRENADE_LIFE) {
		// ｼ�ｷ�ﾅｺﾀｺ ﾅﾍﾁ�ｴﾂ ｼ�ｰ｣ｿ｡ ﾀﾌﾆ衄ｮ ﾃﾟｰ｡.. ｻ霖ｦ...
		Explosion();
		if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource( m_Position, 3.0f, 1300 );
		return false;
	}

	m_Velocity.z-=1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos,normal;

		ZPICKINFO zpi;
		bool bPicked=ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
			{
				pickpos=zpi.bpi.PickPos;
				rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal=rvector(plane.a,plane.b,plane.c);
			}
			else
			if(zpi.pObject)
			{
				pickpos=zpi.info.vOut;
				if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
				{
					normal=pickpos-zpi.pObject->GetPosition();
					normal.z=0;
				}else
					normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
				Normalize(normal);
			}
		}


		// ｾ�ﾑ�ｰ｡ｿ｡ ﾃ豬ｹﾇﾟｴﾙ
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		//*/

		/*
		RBSPPICKINFO bpi;
		bool bPicked=ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position,dir,&bpi,dwPickPassFlag);
		if(bPicked)
		{
			pickpos=bpi.PickPos;
			rplane plane=bpi.pNode->pInfo[bpi.nIndex].plane;
			normal=rvector(plane.a,plane.b,plane.c);
		}

		if(bPicked && fabsf(Magnitude(bpi.PickPos-m_Position))<fDist)
		*/
		{
			m_Position=pickpos+normal;
			m_Velocity=GetReflectionVector(m_Velocity,normal);
			m_Velocity*=zpi.pObject ? 0.4f : 0.8f;		// ﾄｳｸｯﾅﾍｿ｡ ﾃ豬ｹﾇﾏｸ・ｼﾓｵｵｰ｡ ｸｹﾀﾌ ﾁﾘｴﾙ

			// ｼﾒｸｮｸｦ ｳｽｴﾙ ﾅﾗｽｺﾆｮ
			if(zpi.bBspPicked && m_nSoundCount>0) {
				m_nSoundCount--;
				ZGetSoundEngine()->PlaySound("we_grenade_fire",m_Position);
			}

			// ｺｮｹ貮簑ﾇ ｼﾓｵｵｼｺｺﾐﾀｻ ﾁﾙﾀﾎｴﾙ.
			Normalize(normal);
			float fAbsorb=DotProduct(normal,m_Velocity);
			m_Velocity-=0.5*fAbsorb*normal;

			float fA=RANDOMFLOAT*2*pi;
			float fB=RANDOMFLOAT*2*pi;
			m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

		}else
			m_Position+=diff;
	}

	float fRotSpeed=Magnitude(m_Velocity)*0.04f;

	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed *fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	mat = rotmat*mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponGrenade::Explosion()
{
	// ﾀﾚｽﾅﾀﾇ ﾀﾌﾆ衄ｮ ｱﾗｸｮｰ・.
	// ﾁﾖｺｯ ﾀｯｴﾖｵ鯀｡ｰﾔ ｵ･ｹﾌﾁ・ﾁﾖｴﾂ ｸﾞｽﾃﾁ・ｳｯｸｮｰ・.
	// ( ｸﾞｽﾃﾁ・ｹﾞﾀｺｰ�ｿ｡ｼｭ ﾁﾖｺｯ ﾀｯｴﾖｵ・ﾃｼﾅｩﾇﾏｰ・. 
	// ﾁﾗﾀｺ ﾀｯｴﾖﾀﾌ ﾀﾖｴﾙｸ・ｹﾟｻ鄲ｯｴﾖﾀﾇ ﾅｳｼ�ｸｦ ｿﾃｷﾁﾁﾖｱ・

	rvector v = m_Position;

	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	ZGetEffectManager()->AddGrenadeEffect(v,dir);

	// ??????? ?????u?~
	// ?????? U?????T?? ???? ?????? ??? �????u?~

	ZGetGame()->OnExplosionGrenade(m_uidOwner,v,m_fDamage,400.f,.2f,1.f,m_nTeamID);
	ZGetSoundEngine()->PlaySound("we_grenade_explosion",v);

	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}

/////////////////////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponFlashBang,ZWeaponGrenade);

#define FLASHBANG_LIFE	3	//<<- TODO : ｹｫｱ簑ﾇ ｼﾓｼｺｿ｡ｼｭ ｹﾞｾﾆｿﾀｱ・
#define FLASHBANG_DISTANCE	2000		// 20ｹﾌﾅﾍ ｱ�ﾝ・ｿｵﾇ簑ｻ ﾁﾜ

void ZWeaponFlashBang::Explosion()
{
	rvector dir		= RCameraPosition - m_Position;
	float dist		= Magnitude( dir );
	D3DXVec3Normalize( &dir, &dir );
	RBSPPICKINFO pick;

	if( dist > FLASHBANG_DISTANCE )	// ｰﾅｸｮ ｹﾛｿ｡ｼｭｴﾂ ｿｵﾇ簑ｻ ﾁﾖﾁ・ｸ�ﾇﾔ
	{
		return;
	}

	if( ZGetGame()->m_pMyCharacter->IsDie() )	// ｿﾉﾀ�ｹ・ｸ�ｵ蠢｡ｼｭｴﾂ ｿｵﾇ簑ｻ ｹﾞﾁ・ｾﾊﾀｽ
	{
		return;
	}

	// Custom: don't show flashbang effect when the [EX] roomtag is used.
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_EXPLOSION))
	{
		return;
	}

	if( !ZGetGame()->GetWorld()->GetBsp()->Pick( m_Position, dir, &pick ) )
	{
		mbIsLineOfSight	= true;
	}
	else
	{
		float distMap	= D3DXVec3LengthSq( &(pick.PickPos - m_Position) );
		rvector temp = ZGetGame()->m_pMyCharacter->GetPosition() - m_Position;
		float distChar	= D3DXVec3LengthSq( &(temp) );
		if( distMap > distChar )
		{
			mbIsLineOfSight	= true;
		}
		else
		{
			mbIsLineOfSight	= false;
		}
	}

	if( !mbIsExplosion && mbIsLineOfSight )
	{
		rvector pos		= ZGetGame()->m_pMyCharacter->GetPosition();
		rvector dir		= ZGetGame()->m_pMyCharacter->m_TargetDir;
		mbIsExplosion	= true;
		CreateFlashBangEffect( m_Position, pos, dir, 10 );
	}
	ZGetSoundEngine()->PlaySound("we_flashbang_explosion",m_Position);
}

bool	ZWeaponFlashBang::Update( float fElapsedTime )
{
	rvector oldPos = m_Position;

	float lap	= ZGetGame()->GetTime() - m_fStartTime;

	if( lap >= FLASHBANG_LIFE )
	{
		Explosion();
		return false;
	}

	m_Velocity.z	-= 1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff	= m_Velocity * fElapsedTime;
		rvector dir		= diff;
		Normalize( dir );

		float fDist		= Magnitude( diff );

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked	= ZGetGame()->Pick( ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag );

		if( bPicked )
		{
			if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
			{
				Explosion();
				return false;
			}
			if( zpi.bBspPicked )
			{
				pickpos			= zpi.bpi.PickPos;
				rplane plane	= zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal			= rvector( plane.a, plane.b, plane.c );
			}
			else if( zpi.pObject )
			{
				pickpos			= zpi.info.vOut;
				if( zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f )
				{
					normal		= pickpos-zpi.pObject->GetPosition();
					normal.z	= 0;
				}
				else
				{
					normal		= pickpos - (zpi.pObject->GetPosition()+rvector(0,0,90));
				}
				Normalize(normal);
			}

			pickpos		+= normal * BOUND_EPSILON;
		}

		if( bPicked && fabsf( Magnitude(pickpos-m_Position) ) < (fDist + BOUND_EPSILON) )
		{
			m_Position	= pickpos + normal;
			m_Velocity	= GetReflectionVector( m_Velocity, normal );
			m_Velocity	*= zpi.pObject ? 0.4f : 0.8f;

			Normalize( normal );
			float fAbsorb	= DotProduct( normal, m_Velocity );
			m_Velocity		-= 0.5 * fAbsorb * normal;

			float fA	= RANDOMFLOAT * 2 * pi;
			float fB	= RANDOMFLOAT * 2 * pi;
			m_RotAxis	= rvector( sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB) );

		}
		else
		{
			m_Position	+= diff;
		}
	}

	rmatrix Mat;

	if( !mbLand )
	{
		mRotVelocity	= min( Magnitude( m_Velocity ), MAX_ROT_VELOCITY );

		if( Magnitude(m_Velocity) < LANDING_VELOCITY )
		{
			mbLand	= true;
			rvector	right;
			m_Up	= rvector( 0, 1, 0 );
			D3DXVec3Cross( &right, &m_Dir, &m_Up );
			D3DXVec3Cross( &m_Dir, &right, &m_Up );
			D3DXMatrixIdentity( &mRotMatrix );
		}
		else
		{
			rmatrix	Temp;
			D3DXMatrixRotationAxis( &Temp, &m_RotAxis, mRotVelocity * 0.001 );
			mRotMatrix	= mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp;
		D3DXMatrixRotationX( &Temp, mRotVelocity * 0.001 );
		mRotMatrix	= mRotMatrix * Temp;
		mRotVelocity	*= 0.97;
	}

	MakeWorldMatrix( &Mat, m_Position, m_Dir, m_Up );
	Mat		= mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix( Mat );

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position	, ROCKET_SPEAR_EMBLEM_POWER );

	return true;
}

//////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponSmokeGrenade, ZWeaponGrenade);

#define SMOKE_GRENADE_LIFETIME 30
#define SPYSMOKE_GRENADE_LIFETIME 10
#define SMOKE_GRENADE_EXPLOSION	3
#define SPYSMOKE_GRENADE_EXPLOSION 0.5

const float ZWeaponSmokeGrenade::mcfTrigerTimeList[NUM_SMOKE] =
{
	0, 0.5, 1, 1.7, 2.3, 2.5, 3
};

bool ZWeaponSmokeGrenade::Update( float fElapsedTime )
{
	rvector oldPos = m_Position;
	float lap	= ZGetGame()->GetTime() - m_fStartTime;

	if( lap >= SMOKE_GRENADE_LIFETIME )
	{
		return false;
	}

	// Custom: don't show flashbang effect when the [EX] roomtag is used.
	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_EXPLOSION))
	{
		return false;
	}
	
	if( miSmokeIndex < NUM_SMOKE && lap - SMOKE_GRENADE_EXPLOSION >= mcfTrigerTimeList[miSmokeIndex] )
	{

		Explosion();
		++miSmokeIndex;

	}

	m_Velocity.z	-= 1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff	= m_Velocity * fElapsedTime;
		rvector dir		= diff;
		Normalize( dir );

		float fDist		= Magnitude( diff );

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked	= ZGetGame()->Pick( ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag );
		
		if( bPicked )
		{
			if( zpi.bBspPicked )
			{
				pickpos			= zpi.bpi.PickPos;
				rplane plane	= zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal			= rvector( plane.a, plane.b, plane.c );
			}
			else if( zpi.pObject )
			{
				pickpos			= zpi.info.vOut;
				if( zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f )
				{
					normal		= pickpos-zpi.pObject->GetPosition();
					normal.z	= 0;
				}
				else
				{
					normal		= pickpos - (zpi.pObject->GetPosition()+rvector(0,0,90));
				}
				Normalize(normal);
			}

			pickpos		+= normal * BOUND_EPSILON;
		}

		if( bPicked && fabsf( Magnitude(pickpos-m_Position) ) < (fDist + BOUND_EPSILON) )
		{
			m_Position	= pickpos + normal;
			m_Velocity	= GetReflectionVector( m_Velocity, normal );
			m_Velocity	*= zpi.pObject ? 0.4f : 0.8f;

			Normalize( normal );
			float fAbsorb	= DotProduct( normal, m_Velocity );
			m_Velocity		-= 0.5 * fAbsorb * normal;

			float fA	= RANDOMFLOAT * 2 * pi;
			float fB	= RANDOMFLOAT * 2 * pi;
			m_RotAxis	= rvector( sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB) );

		}
		else
		{
			m_Position	+= diff;
		}
	}

	rmatrix Mat;

	if( !mbLand )
	{
 		mRotVelocity	= min( Magnitude( m_Velocity ), MAX_ROT_VELOCITY );

		if( Magnitude(m_Velocity) < LANDING_VELOCITY )
		{
			mbLand	= true;
			rvector	right;
			m_Up	= rvector( 0, 1, 0 );
			D3DXVec3Cross( &right, &m_Dir, &m_Up );
			D3DXVec3Cross( &m_Dir, &right, &m_Up );
			D3DXMatrixIdentity( &mRotMatrix );
		}
		else
		{
			rmatrix	Temp;
			D3DXMatrixRotationAxis( &Temp, &m_RotAxis, mRotVelocity * 0.001 );
			mRotMatrix	= mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp;
		D3DXMatrixRotationX( &Temp, mRotVelocity * 0.001 );
		mRotMatrix = mRotMatrix * Temp;
 		mRotVelocity	*= 0.97;
	}
	
	MakeWorldMatrix( &Mat, m_Position, m_Dir, m_Up );
	Mat = mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix( Mat );
	
	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponSmokeGrenade::Explosion()
{
	
	ZGetEffectManager()->AddSmokeGrenadeEffect( m_Position );
	
	mRotVelocity *= 10;

	ZGetSoundEngine()->PlaySound("we_gasgrenade_explosion",m_Position);
}

/////////////////////////// ｸｶｹ� ｹﾌｻ鄲ﾏｷ・.

MImplementRTTI(ZWeaponMagic, ZMovingWeapon);

void ZWeaponMagic::Create(RMesh* pMesh, ZSkill* pSkill, const rvector &pos, const rvector &dir, float fMagicScale,ZObject* pOwner) {

	ZWeapon::Create(pMesh);

	m_fMagicScale = fMagicScale;

	m_pVMesh->SetAnimation("play");

	m_pSkillDesc = pSkill->GetDesc();

	m_Position=pos;

	if (m_pSkillDesc) m_Velocity=dir * m_pSkillDesc->fVelocity;
	else m_Velocity=dir*ROCKET_VELOCITY;
	

	m_fStartTime = ZGetGame()->GetTime();
	m_fLastAddTime = ZGetGame()->GetTime();

	m_Dir=dir;
	m_Up=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	m_fDamage = pSkill->GetDesc()->nModDamage;

	m_uidTarget = pSkill->GetTarget();
	m_bGuide = pSkill->GetDesc()->bGuidable;

	m_bThroughNPC = pSkill->GetDesc()->bThroughNPC;
	m_bSurfaceMount = pSkill->GetDesc()->bSurfaceMount;
	
	if( Z_VIDEO_DYNAMICLIGHT && !pSkill->GetDesc()->bOffDLight) {
		_ASSERT( m_SLSid == 0);
		m_SLSid = ZGetStencilLight()->AddLightSource( m_Position, 2.0f );
	}

	//ZGetEffectManager()->AddRocketSmokeEffect(m_Position,-RealSpace2::RCameraDirection);
}

#define MAGIC_WEAPON_LIFE			10.f		// 10ﾃﾊ ｵﾚｿ｡ ﾅﾍﾁ・


bool ZWeaponMagic::Update(float fElapsedTime)
{
	// ﾀｯｵｵﾃｳｸｮ
	if(m_bGuide) {
		ZObject *pTarget = ZGetObjectManager()->GetObject(m_uidTarget);
		if (pTarget) {

			float fCurrentSpeed = Magnitude(m_Velocity);
			rvector currentDir = m_Velocity;
			Normalize(currentDir);

			rvector dir = (pTarget->GetPosition()+rvector(0,0,100)) - m_Position;

			if ( m_bSurfaceMount)
				dir.z = 0;

			Normalize(dir);

			float fCos = DotProduct(dir,currentDir);
			float fAngle = acos(fCos);
			if(fAngle>0.01f) {

		#define ANGULAR_VELOCITY	0.01f		// ｰ｢ｼﾓｵｵ : ﾃﾊｴ・ﾈｸﾀ・｡ｴﾉﾇﾑ ｰ｢ﾀﾇ ﾅｩｱ・ (ｴﾜﾀｧ:ｶ�ｵ�ｾﾈ)
				float fAngleDiff = min(1000.f*fElapsedTime*ANGULAR_VELOCITY,fAngle);

				rvector newDir = InterpolatedVector(m_Dir,dir,fAngleDiff/fAngle);
				m_Dir = newDir;

				m_Velocity = fCurrentSpeed * newDir;
				_ASSERT(!_isnan(m_Velocity.x) &&!_isnan(m_Velocity.y) && !_isnan(m_Velocity.z) );
			}
		}
	}

	rvector oldPos = m_Position;

	if(ZGetGame()->GetTime() - m_fStartTime > MAGIC_WEAPON_LIFE ) {
		// ｼ�ｷ�ﾅｺﾀｺ ﾅﾍﾁ�ｴﾂ ｼ�ｰ｣ｿ｡ ﾀﾌﾆ衄ｮ ﾃﾟｰ｡.. ｻ霖ｦ...
		Explosion( WMET_MAP, NULL , rvector(0,1,0));

		if(Z_VIDEO_DYNAMICLIGHT && m_SLSid ) {
			ZGetStencilLight()->DeleteLightSource( m_SLSid );
			m_SLSid = 0;
		}

		return false;
	}

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos;
		rvector pickdir;

		ZPICKINFO zpi;
		WeaponMagicExplosionType type = WMET_MAP;

		ZObject* pOwnerObject = ZGetObjectManager()->GetObject(m_uidOwner);
		ZObject* pPickObject = NULL;

		bool bPicked=ZGetGame()->Pick(pOwnerObject, m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked) {
				pickpos=zpi.bpi.PickPos;
				pickdir.x=zpi.bpi.pInfo->plane.a;
				pickdir.y=zpi.bpi.pInfo->plane.b;			
				pickdir.z=zpi.bpi.pInfo->plane.c;
				Normalize(pickdir);
			}	
			else
				if(zpi.pObject) {
					pPickObject = zpi.pObject;
					pickpos=zpi.info.vOut;
					type = WMET_OBJECT;
				}

		}


		if( bPicked && fabsf( Magnitude(pickpos-m_Position)) < fDist)
		{
			// ﾅ・�ﾇﾏｴﾂ ﾁｾｷ�ｶ�ｸ・..
			bool bCheck = true;
			if ( m_bThroughNPC && pPickObject && pPickObject->IsNPC())
				bCheck = false;

			if ( bCheck)
			{
				Explosion( type, pPickObject,pickdir);

				if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
				{
					ZGetStencilLight()->DeleteLightSource( m_SLSid );
					m_SLSid = 0;
					ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
				}

				return false;
			}
		}

//		else
		{
			rvector to = m_Position + diff;

			if ( ZGetGame()->ObjectColTest(pOwnerObject, m_Position, to, m_pSkillDesc->fColRadius, &pPickObject))
			{
				// ﾅ・�ﾇﾏｴﾂ ﾁｾｷ�ｶ�ｸ・..
				bool bCheck = true;
				if ( m_bThroughNPC && pPickObject && pPickObject->IsNPC())
					bCheck = false;

				if ( bCheck)
				{
					pickdir = to - m_Position;
					Normalize(pickdir);

					ExplosionThrow( WMET_OBJECT, pPickObject,pickdir);

					if(Z_VIDEO_DYNAMICLIGHT && m_SLSid )
					{
						ZGetStencilLight()->DeleteLightSource( m_SLSid );
						m_SLSid = 0;
						ZGetStencilLight()->AddLightSource( pickpos, 3.0f, 1300 );
					}
					
					return false;
				}
			}
//			else
			{
				m_Position+=diff;
			}
		}
	}

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

//	mat._12 *= m_fMagicScale;
//	mat._23 *= m_fMagicScale;
//	mat._31 *= m_fMagicScale;

	m_pVMesh->SetScale(rvector(m_fMagicScale,m_fMagicScale,m_fMagicScale));
	m_pVMesh->SetWorldMatrix(mat);

	float this_time = ZGetGame()->GetTime();

	if( this_time > m_fLastAddTime + 0.02f ) {

#define _ROCKET_RAND_CAP 10

		/*
		rvector add;

		add.x = rand()%_ROCKET_RAND_CAP;
		add.y = rand()%_ROCKET_RAND_CAP;
		add.z = rand()%_ROCKET_RAND_CAP;

		if(rand()%2) add.x=-add.x;
		if(rand()%2) add.y=-add.y;
		if(rand()%2) add.z=-add.z;

		rvector pos = m_Position+add;
		*/

		rvector add = rvector(RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f,RANDOMFLOAT-0.5f);
		rvector pos = m_Position + 20.f*add;

//		case type

		ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

		nEffectType = m_pSkillDesc->nTrailEffectType;

		if (m_pSkillDesc->bDrawTrack)
		{
				if(nEffectType==ZSTE_FIRE)		ZGetEffectManager()->AddTrackFire(pos);
			else if(nEffectType==ZSTE_COLD)		ZGetEffectManager()->AddTrackCold(pos);
			else if(nEffectType==ZSTE_MAGIC)	ZGetEffectManager()->AddTrackMagic(pos);
		}

//		ZGetEffectManager()->AddRocketSmokeEffect(pos);
//		ZGetEffectManager()->AddSmokeEffect(NULL,pos,rvector(0,0,0),rvector(0,0,0),60.f,80.f,1.5f);
		ZGetWorld()->GetFlags()->CheckSpearing( oldPos, pos	, ROCKET_SPEAR_EMBLEM_POWER );
		m_fLastAddTime = this_time;
	}

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->SetLightSourcePosition( m_SLSid, m_Position	);

	return true;
}

void ZWeaponMagic::Render()
{
	ZWeapon::Render();


	// test ﾄﾚｵ・
#ifndef _PUBLISH
	if ((ZApplication::GetGameInterface()->GetScreenDebugger()->IsVisible()) && (ZIsLaunchDevelop()))
	{
		if (m_pSkillDesc)
		{
			RDrawSphere(m_Position, m_pSkillDesc->fColRadius, 10);
		}
	}
#endif

}



void ZWeaponMagic::Explosion(WeaponMagicExplosionType type, ZObject* pVictim,rvector& vDir)
{
	rvector v = m_Position-rvector(0,0,100.f);

#define MAGIC_MINIMUM_DAMAGE	0.2f
#define MAGIC_KNOCKBACK_CONST   .3f	

	if (!m_pSkillDesc->IsAreaTarget())
	{
		if ((pVictim) && (type == WMET_OBJECT))
		{
			ZGetGame()->OnExplosionMagicNonSplash(this, m_uidOwner, pVictim->GetUID(), v, m_pSkillDesc->fModKnockback);
		}
	}
	else
	{
		ZGetGame()->OnExplosionMagic(this,m_uidOwner,v,MAGIC_MINIMUM_DAMAGE,m_pSkillDesc->fModKnockback, m_nTeamID, true);
	}

	ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

	nEffectType = m_pSkillDesc->nTrailEffectType;

	rvector pos = m_Position;

	if(type == WMET_OBJECT) {// object ｿ｡ ｺﾎｵ�ﾈ�ｴﾂ ｰ豼・..
		float fScale = m_fMagicScale;

		// ﾀﾌﾆ衄ｮｴﾂ ｽｺﾄﾉﾀﾏｸｵﾇﾏｸ・ﾁﾙﾀﾎｴﾙ.
		/*		if(nEffectType==ZSTE_FIRE)			
		{
		if (fScale > 1.0f)
		{
		fScale = 1.0f + fScale*0.2f;
		pos -= rvector(0.0f, 0.0f, fScale * 100.0f);
		}

		ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_FIRE,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_COLD)		
		{
		ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_COLD,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_MAGIC)	
		{
		ZGetEffectManager()->AddMagicEffect(m_Position,0, fScale);
		}
		*/

		// ﾀﾌﾆ衄ｮ ｻ鄙�ﾑ・ﾃﾟｰ｡
		if ( m_pSkillDesc->szExplosionSound[ 0])
		{
			rvector soundPos = pVictim->GetPosition();
			ZGetSoundEngine()->PlaySound( m_pSkillDesc->szExplosionSound, soundPos);
		}


		// ｻ�ｴ・ﾅｸｱ・｡ ﾀﾎﾃｾﾆｮ ﾀﾌﾆ衄ｮ ﾃﾟｰ｡
		switch ( m_pSkillDesc->ResistType)
		{
		case ZSR_FIRE :
			{
				ZModule_FireDamage *pMod = (ZModule_FireDamage*)pVictim->GetModule( ZMID_FIREDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}

		case ZSR_COLD :
			{
				ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pVictim->GetModule( ZMID_COLDDAMAGE);
				if ( pMod)  pMod->BeginDamage( 1.0f, 0.9f);

				break;
			}

		case ZSR_LIGHTNING :
			{
				//				ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pVictim->GetModule( ZMID_LIGHTNINGDAMAGE);
				//				if ( pMod)  pMod->BeginDamage( MUID(0,0), 0, 0.9f);

				break;
			}

		case ZSR_POISON :
			{
				ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pVictim->GetModule( ZMID_POISONDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}
		}
	}


	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}



void ZWeaponMagic::ExplosionThrow(WeaponMagicExplosionType type, ZObject* pVictim,rvector& vDir)
{
	rvector v = m_Position-rvector(0,0,100.f);

#define MAGIC_MINIMUM_DAMAGE	0.2f
#define MAGIC_KNOCKBACK_CONST   .3f	

	if (!m_pSkillDesc->IsAreaTarget())
	{
		if ((pVictim) && (type == WMET_OBJECT))
		{
			ZGetGame()->OnExplosionMagicNonSplash(this, m_uidOwner, pVictim->GetUID(), v, m_pSkillDesc->fModKnockback);
		}
	}
	else
	{
		ZGetGame()->OnExplosionMagicThrow(this,m_uidOwner,v,MAGIC_MINIMUM_DAMAGE,m_pSkillDesc->fModKnockback, m_nTeamID, true, m_Position, vDir + m_Position);
	}

	ZSKILLEFFECTTRAILTYPE nEffectType = ZSTE_NONE;

	nEffectType = m_pSkillDesc->nTrailEffectType;

	rvector pos = m_Position;

	if(type == WMET_OBJECT) {// object ｿ｡ ｺﾎｵ�ﾈ�ｴﾂ ｰ豼・..
		float fScale = m_fMagicScale;
		
		// ﾀﾌﾆ衄ｮｴﾂ ｽｺﾄﾉﾀﾏｸｵﾇﾏｸ・ﾁﾙﾀﾎｴﾙ.
/*		if(nEffectType==ZSTE_FIRE)			
		{
			if (fScale > 1.0f)
			{
				fScale = 1.0f + fScale*0.2f;
				pos -= rvector(0.0f, 0.0f, fScale * 100.0f);
			}

			ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_FIRE,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_COLD)		
		{
			ZGetEffectManager()->AddSwordEnchantEffect(ZC_ENCHANT_COLD,pos,0, fScale);
		}
		else if(nEffectType==ZSTE_MAGIC)	
		{
			ZGetEffectManager()->AddMagicEffect(m_Position,0, fScale);
		}
*/

		// ﾀﾌﾆ衄ｮ ｻ鄙�ﾑ・ﾃﾟｰ｡
		if ( m_pSkillDesc->szExplosionSound[ 0])
		{
			rvector soundPos = pVictim->GetPosition();
			ZGetSoundEngine()->PlaySound( m_pSkillDesc->szExplosionSound, soundPos);
		}


		// ｻ�ｴ・ﾅｸｱ・｡ ﾀﾎﾃｾﾆｮ ﾀﾌﾆ衄ｮ ﾃﾟｰ｡
		switch ( m_pSkillDesc->ResistType)
		{
			case ZSR_FIRE :
			{
				ZModule_FireDamage *pMod = (ZModule_FireDamage*)pVictim->GetModule( ZMID_FIREDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}

			case ZSR_COLD :
			{
				ZModule_ColdDamage *pMod = (ZModule_ColdDamage*)pVictim->GetModule( ZMID_COLDDAMAGE);
				if ( pMod)  pMod->BeginDamage( 1.0f, 0.9f);

				break;
			}

			case ZSR_LIGHTNING :
			{
//				ZModule_LightningDamage *pMod = (ZModule_LightningDamage*)pVictim->GetModule( ZMID_LIGHTNINGDAMAGE);
//				if ( pMod)  pMod->BeginDamage( MUID(0,0), 0, 0.9f);

				break;
			}

			case ZSR_POISON :
			{
				ZModule_PoisonDamage *pMod = (ZModule_PoisonDamage*)pVictim->GetModule( ZMID_POISONDAMAGE);
				if ( pMod)  pMod->BeginDamage( NULL, 0, 0.9f);

				break;
			}
		}
	}


	ZGetWorld()->GetFlags()->SetExplosion( v, EXPLOSION_EMBLEM_POWER );
}

////////////////////////// ｴﾙﾀﾌｳｪｸｶﾀﾌﾆｮ

MImplementRTTI(ZWeaponDynamite, ZMovingWeapon);

void ZWeaponDynamite::Create(RMesh* pMesh, rvector &pos, rvector &velocity, ZObject* pOwner) 
{
	ZWeapon::Create(pMesh);
	
	rvector dir = velocity;
	Normalize(dir);

	m_Velocity = velocity;
	m_Position = pos;

	m_bCollided = false;
	m_fStartTime = 0.f;

	m_Dir		= rvector(1,0,0);
	m_Up		= rvector(0,0,1);
	m_RotAxis	= rvector(0,0,1);

	m_uidOwner	= pOwner->GetUID();
	m_nTeamID	= pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if( pOwner->GetItems() ) {
		if( pOwner->GetItems()->GetSelectedWeapon() ) {
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();
		}
	}

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage = pDesc->m_nDamage.Ref();
	m_nSoundCount = (rand() % 2) + 2;
}

#define DYNAMITE_LIFE 0.7f	// ﾃｹｹ�ﾂｰ ﾃ豬ｹ ﾈﾄ 0.7ﾃﾊﾈﾄｿ｡ ﾅﾍﾁ・

bool ZWeaponDynamite::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;

	if (m_Position.z < DIE_CRITICAL_LINE)
		return false;

	// ﾃｳﾀｽﾀｸｷﾎ ｺﾎｵ�ﾈ・ｶｧｺﾎﾅﾍﾀﾇ ｽﾃｰ｣ﾀｻ ﾃｼﾅｩﾇﾏｿｩ ﾆ�ｹﾟ
	if( m_bCollided &&
		ZGetGame()->GetTime() - m_fStartTime > DYNAMITE_LIFE) {
		Explosion();
		if(Z_VIDEO_DYNAMICLIGHT)
			ZGetStencilLight()->AddLightSource( m_Position, 3.0f, 1300 );
		return false;
	}

	m_Velocity.z = m_Velocity.z - (1000.f * fElapsedTime);

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff = m_Velocity * fElapsedTime;
		rvector dir	= diff;
		Normalize(dir);

		float fDist = Magnitude(diff);

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
			{
				pickpos = zpi.bpi.PickPos;
				rplane plane = zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal = rvector(plane.a, plane.b, plane.c);
			}
			else 
			{
				if(zpi.pObject)
				{
					pickpos = zpi.info.vOut;
					if(zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f)
					{
						normal = pickpos-zpi.pObject->GetPosition();
						normal.z = 0;
					}
					else 
					{
						normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
					}

					Normalize(normal);
				}
			}
		}

		// ｾ�ﾑ�ｰ｡ｿ｡ ﾃ豬ｹﾇﾟｴﾙ
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			// ﾃｳﾀｽﾀｸｷﾎ ｾ�ﾑ�ｰ｡ ｺﾎｵ�ﾇ�ﾀｻ ｶｧｺﾎﾅﾍ ｽﾃｰ｣ﾀｻ ﾀ邀・ｽﾃﾀﾛ
			if (!m_bCollided)
			{
				m_bCollided = true;
				m_fStartTime = ZGetGame()->GetTime();
			}

			m_Position=pickpos+normal;
			m_Velocity=GetReflectionVector(m_Velocity,normal);
			m_Velocity*=zpi.pObject ? 0.4f : 0.8f;		// ﾄｳｸｯﾅﾍｿ｡ ﾃ豬ｹﾇﾏｸ・ｼﾓｵｵｰ｡ ｸｹﾀﾌ ﾁﾘｴﾙ

			// ｼﾒｸｮｸｦ ｳｽｴﾙ ﾅﾗｽｺﾆｮ
			if(zpi.bBspPicked && m_nSoundCount>0) {
				m_nSoundCount--;
				ZGetSoundEngine()->PlaySound("we_grenade_fire",m_Position);
			}

			// ｺｮｹ貮簑ﾇ ｼﾓｵｵｼｺｺﾐﾀｻ ﾁﾙﾀﾎｴﾙ.
			Normalize(normal);
			float fAbsorb=DotProduct(normal,m_Velocity);
			m_Velocity-=0.5*fAbsorb*normal;

			float fA=RANDOMFLOAT*2*pi;
			float fB=RANDOMFLOAT*2*pi;
			m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

		}
		else
			m_Position+=diff;
	}

	float fRotSpeed = Magnitude(m_Velocity) * 0.04f;

	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed *fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	mat = rotmat*mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );
	return true;
}

void ZWeaponDynamite::Explosion()
{
	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;

	ZGetEffectManager()->AddDynamiteEffect(m_Position, dir);
	ZGetSoundEngine()->PlaySound("we_grenade_explosion", m_Position);

	ZGetGame()->OnExplosionDynamite(m_uidOwner, m_Position, m_fDamage, 400.f, 1.f, m_nTeamID);
	ZGetWorld()->GetFlags()->SetExplosion( m_Position, EXPLOSION_EMBLEM_POWER );
}


MImplementRTTI(ZWeaponTrap, ZMovingWeapon);

bool ZWeaponTrap::Init(RMesh* pMesh, rvector& pos, rvector& velocity, int nItemId, ZObject* pOwner)
{
	ZMovingWeapon::Create(pMesh);

	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
	if (!pDesc) return false;

	m_fDamage=pDesc->m_nDamage.Ref();
	m_nItemId = nItemId;

	m_Position=pos;
	m_Velocity=velocity;

	m_Dir=rvector(1,0,0);
	m_Up=rvector(0,0,1);
	m_RotAxis=rvector(0,0,1);

	m_uidOwner=pOwner->GetUID();
	m_nTeamID=pOwner->GetTeamID();

	m_fThrowedTime = ZGetGame()->GetTime();

	m_bLanded = false;
	m_fLandedTime = 0;

	m_bActivated = false;
	m_fActivatedTime = 0;
	m_fNextEffectTime = 0;

	m_bFriendly = false;
	ZObject* pMyChar = ZGetCharacterManager()->Find(ZGetMyUID());
	if (!ZGetGame()->CanAttack(pOwner, pMyChar))
		m_bFriendly = true;
	else if (pOwner && pOwner->GetUID()==ZGetMyUID())
		m_bFriendly = true;

	m_nDamageType = pDesc->m_nDamageType.Ref();
	
	return true;
}

bool ZWeaponTrap::Create(RMesh* pMesh, rvector& pos, rvector& velocity, int nItemId, ZObject* pOwner)
{
	if (!Init(pMesh, pos, velocity, nItemId, pOwner)) return false;

	// ﾁ､ｸｻ ﾇﾘｴ・ｾﾆﾀﾌﾅﾛﾀｻ ﾀ蠎�ﾇﾏｰ・ﾀﾖｴﾂｰ｡
	if (!pOwner->IsNPC())
	{
		if (pOwner->GetItems())
		{
			bool bEquiped = false;
			for (int i = MMCIP_CUSTOM1; i <= MMCIP_CUSTOM2; ++i)
			{
				ZItem* pItem = pOwner->GetItems()->GetItem((MMatchCharItemParts)i);
				if (pItem->GetDesc() && pItem->GetDesc()->m_nID == nItemId)
					bEquiped = true;
			}

			if (!bEquiped) return false;
		}
		if (m_uidOwner == ZGetMyUID())
			ZPostNotifyThrowedTrap(m_nItemId);
	}
	else
	{
		ZActorWithFSM* pActor = (ZActorWithFSM*)pOwner;
		if (pActor->IsMyControl())
			ZPostNotifyThrowedTrap(m_nItemId);
	}

	return true;
}

bool ZWeaponTrap::CreateActivated( RMesh* pMesh, rvector& pos, float fActivatedTime, int nItemId, ZObject* pOwner )
{
	if (!Init(pMesh, pos, rvector(0,0,0), nItemId, pOwner))
		return false;

	m_fActivatedTime = fActivatedTime;
	m_fNextEffectTime = m_fActivatedTime;
	m_bActivated = true;

	rmatrix mat;
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);
	m_pVMesh->SetWorldMatrix(mat);

	float fElapsedTime = ZGetGame()->GetTime() - m_fActivatedTime;
	AddEffectActivating(fElapsedTime * 1000, m_bFriendly);

	return true;
}

#define TRAP_LANDING_TIME 0.7f		// ｶ･ｿ｡ ｴ・ｺ ﾈﾄ ﾀﾌ ｽﾃｰ｣ｵｿｾﾈ ｰ｡ｸｸﾈ・ｱ箒ﾙｸｰｴﾙ

bool ZWeaponTrap::Update( float fElapsedTime )
{
	if (m_bActivated)
		return UpdateOnActivated(fElapsedTime);
	
	if (m_bLanded)
	{
		if (ZGetGame()->GetTime() - m_fLandedTime > TRAP_LANDING_TIME)
		{
			Activate();
		}
		return true;
	}

	if (m_Position.z < DIE_CRITICAL_LINE)
		return false;

	if (ZGetGame()->GetTime() - m_fThrowedTime > MAX_TRAP_THROWING_LIFE)
		return false;

	rvector oldPos = m_Position;

	m_Velocity.z-=1000.f*fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff=m_Velocity*fElapsedTime;
		rvector dir=diff;
		Normalize(dir);

		float fDist=Magnitude(diff);

		rvector pickpos,normal;

		ZPICKINFO zpi;
		bool bPicked=ZGetGame()->Pick(ZGetObjectManager()->GetObject(m_uidOwner),m_Position,dir,&zpi,dwPickPassFlag);
		if(bPicked)
		{
			if(zpi.bBspPicked)
			{
				pickpos=zpi.bpi.PickPos;
				rplane plane=zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal=rvector(plane.a,plane.b,plane.c);
			}
			else if(zpi.pObject)
			{
				pickpos=zpi.info.vOut;
				if(zpi.pObject->GetPosition().z+30.f<=pickpos.z && pickpos.z<=zpi.pObject->GetPosition().z+160.f)
				{
					normal=pickpos-zpi.pObject->GetPosition();
					normal.z=0;
				}
				else
					normal=pickpos-(zpi.pObject->GetPosition()+rvector(0,0,90));
				Normalize(normal);
			}
		}


		// ｾ�ﾑ�ｰ｡ｿ｡ ﾃ豬ｹﾇﾟｴﾙ
		if(bPicked && fabsf(Magnitude(pickpos-m_Position))<fDist)
		{
			m_Position=pickpos+normal;
			m_Velocity=GetReflectionVector(m_Velocity,normal);
			m_Velocity*=zpi.pObject ? 0.4f : 0.8f;		// ﾄｳｸｯﾅﾍｿ｡ ﾃ豬ｹﾇﾏｸ・ｼﾓｵｵｰ｡ ｸｹﾀﾌ ﾁﾘｴﾙ

			if(zpi.bBspPicked) {
				ZGetSoundEngine()->PlaySound("we_grenade_fire",m_Position);
			}

			// ｺｮｹ貮簑ﾇ ｼﾓｵｵｼｺｺﾐﾀｻ ﾁﾙﾀﾎｴﾙ.
			Normalize(normal);
			float fAbsorb=DotProduct(normal,m_Velocity);
			m_Velocity-=0.5*fAbsorb*normal;

			float fA=RANDOMFLOAT*2*pi;
			float fB=RANDOMFLOAT*2*pi;
			m_RotAxis=rvector(sin(fA)*sin(fB),cos(fA)*sin(fB),cos(fB));

			// ﾁ�ｸ鯀｡ ｴ・ﾒﾀｻ ｶｧ ﾀﾏﾁ､ｽﾃｰ｣ ﾈﾄ ﾆ�ｹﾟ (ｺｮﾀﾌｳｪ ﾃｵﾀ・ﾃ豬ｹﾀｺ ﾆﾐｽｺ)
			if(DotProduct(normal, rvector(0,0,1)) > 0.8f)
			{
				if (!m_bLanded)
				{
					m_bLanded = true;
					m_fLandedTime = ZGetGame()->GetTime();
					
					// ﾆｮｷｦﾀﾌ ｼｳﾄ｡ｵﾈ ﾀｧﾄ｡ｸｦ ｾﾋ ｼ・ﾀﾖｰﾔ ｰ｡ﾀﾌｵ蟶ｦ ﾁｶｱﾗｸﾄｰﾔ ﾇ･ｽﾃﾇﾏﾀﾚ
					ZGetEffectManager()->AddTrapGuideEffect(m_Position, rvector(0,1,0), int(TRAP_LANDING_TIME*1000), m_bFriendly, 0.1f, 12.f);
				}
				return true;
			}

		}else
			m_Position+=diff;
	}

	float fRotSpeed=Magnitude(m_Velocity)*0.04f;

	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed *fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir=m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat,m_Position,m_Dir,m_Up);

	mat = rotmat*mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing( oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER );

	return true;
}

void ZWeaponTrap::Activate()
{
	m_bActivated = true;
	m_fActivatedTime = ZGetGame()->GetTime();
	m_fNextEffectTime = m_fActivatedTime;
	
	AddEffectActivating(0, m_bFriendly);

	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource( m_Position, 3.0f, 1300 );

	// ｼｭｹ�ｿ｡ ｹﾟｵｿｵﾈ ﾆｮｷｦﾀｻ ｵ�ﾓﾏﾇﾘｼｭ ﾀﾌﾈﾄ ｳｭﾀﾔﾇﾏｴﾂ ﾀｯﾀ�ｰ｡ ｿ�ｵ蠢｡ ｼｳﾄ｡ｵﾈ ﾆｮｷｦｵ鯊ｻ ｾﾋ ｼ・ﾀﾖｵｵｷﾏ ﾇﾏﾀﾚ
	if (m_uidOwner == ZGetMyUID())
		ZPostNotifyActivatedTrap(m_nItemId, m_Position);
}

void ZWeaponTrap::AddEffectActivating(int nElapsedIfActivated, bool bFriendly)
{
	// ｹﾟｵｿﾀﾌ ﾁ�ｼﾓｵﾉ ｵｿｾﾈ ｺｸｿｩﾁ・ﾀﾌﾆ衄ｮ ｼｼﾆﾃ
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nItemId);
	if (!pItemDesc) return;
	int nLifeTime = pItemDesc->m_nLifeTime.Ref();

	ZGetEffectManager()->AddTrapGuideEffect(m_Position, rvector(0,1,0), nLifeTime - nElapsedIfActivated, bFriendly);
}

bool ZWeaponTrap::UpdateOnActivated( float fElapsedTime )
{
	if (m_bActivated)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nItemId);
		if (!pItemDesc) return false;

		float fCurrTime = ZGetGame()->GetTime();
		if (fCurrTime - m_fActivatedTime > pItemDesc->m_nLifeTime.Ref()*0.001f)
			return false;

		if (fCurrTime > m_fNextEffectTime)
		{
			m_fNextEffectTime += 1.f;	// 1ﾃﾊｸｶｴﾙ ﾀﾌﾆ衄ｮｸｦ ﾃ箙ﾂﾇﾑｴﾙ
			switch (m_nDamageType)
			{
			case MMDT_FIRE:
				ZGetEffectManager()->AddTrapFireEffect(m_Position, rvector(0,1,0));
				break;
			case MMDT_COLD:
				ZGetEffectManager()->AddTrapColdEffect(m_Position, rvector(0,1,0));
				break;
			default:
				_ASSERT(0);
			}
		}

		ZGetGame()->CheckZoneTrap(m_uidOwner,m_Position,pItemDesc,m_nTeamID);
		return true;
	}
	return true;
}

//////////////////////////////////////// MODIFIED SPECIAL WEAPONS FOR SPY MODE ////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponSpyFlashBang, ZWeaponGrenade);

#define SPY_FLASHBANG_LIFE		0.4f		//<<- TODO : ｹｫｱ簑ﾇ ｼﾓｼｺｿ｡ｼｭ ｹﾞｾﾆｿﾀｱ・
#define SPY_FLASHBANG_DISTANCE	2500		// 20ｹﾌﾅﾍ ｱ�ﾝ・ｿｵﾇ簑ｻ ﾁﾜ

void ZWeaponSpyFlashBang::Explosion()
{
	rvector dir = RCameraPosition - m_Position;
	float dist = Magnitude(dir);
	D3DXVec3Normalize(&dir, &dir);
	RBSPPICKINFO pick;

	if (dist > SPY_FLASHBANG_DISTANCE)	// ｰﾅｸｮ ｹﾛｿ｡ｼｭｴﾂ ｿｵﾇ簑ｻ ﾁﾖﾁ・ｸ�ﾇﾔ
	{
		return;
	}

	if (ZGetGame()->m_pMyCharacter->IsDie())	// ｿﾉﾀ�ｹ・ｸ�ｵ蠢｡ｼｭｴﾂ ｿｵﾇ簑ｻ ｹﾞﾁ・ｾﾊﾀｽ
	{
		return;
	}

	if (!ZGetGame()->GetWorld()->GetBsp()->Pick(m_Position, dir, &pick))
	{
		mbIsLineOfSight = true;
	}
	else
	{
		float distMap = D3DXVec3LengthSq(&(pick.PickPos - m_Position));
		rvector temp = ZGetGame()->m_pMyCharacter->GetPosition() - m_Position;
		float distChar = D3DXVec3LengthSq(&(temp));
		if (distMap > distChar)
		{
			mbIsLineOfSight = true;
		}
		else
		{
			mbIsLineOfSight = false;
		}
	}

	if (!mbIsExplosion && mbIsLineOfSight)
	{
		rvector pos = ZGetGame()->m_pMyCharacter->GetPosition();
		rvector dir = ZGetGame()->m_pMyCharacter->m_TargetDir;
		mbIsExplosion = true;
		CreateFlashBangEffect(m_Position, pos, dir, ZGetGame()->m_pMyCharacter->GetTeamID() == MMT_RED ? 7.f : 15.f);
	}
	ZGetSoundEngine()->PlaySound("we_flashbang_explosion", m_Position);
}

bool	ZWeaponSpyFlashBang::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;

	float lap = ZGetGame()->GetTime() - m_fStartTime;

	if (lap >= SPY_FLASHBANG_LIFE)
	{
		Explosion();
		return false;
	}

	m_Velocity.z -= 1000.f * fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff = m_Velocity * fElapsedTime;
		rvector dir = diff;
		Normalize(dir);

		float fDist = Magnitude(diff);

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);

		if (bPicked)
		{
			if (zpi.bBspPicked)
			{
				pickpos = zpi.bpi.PickPos;
				rplane plane = zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal = rvector(plane.a, plane.b, plane.c);
			}
			else if (zpi.pObject)
			{
				pickpos = zpi.info.vOut;
				if (zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f)
				{
					normal = pickpos - zpi.pObject->GetPosition();
					normal.z = 0;
				}
				else
				{
					normal = pickpos - (zpi.pObject->GetPosition() + rvector(0, 0, 90));
				}
				Normalize(normal);
			}

			pickpos += normal * BOUND_EPSILON;
		}

		if (bPicked && fabsf(Magnitude(pickpos - m_Position)) < (fDist + BOUND_EPSILON))
		{
			m_Position = pickpos + normal;
			m_Velocity = GetReflectionVector(m_Velocity, normal);
			m_Velocity *= zpi.pObject ? 0.4f : 0.8f;

			Normalize(normal);
			float fAbsorb = DotProduct(normal, m_Velocity);
			m_Velocity -= 0.5 * fAbsorb * normal;

			float fA = RANDOMFLOAT * 2 * pi;
			float fB = RANDOMFLOAT * 2 * pi;
			m_RotAxis = rvector(sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB));

		}
		else
		{
			m_Position += diff;
		}
	}

	rmatrix Mat;

	if (!mbLand)
	{
		mRotVelocity = min(Magnitude(m_Velocity), MAX_ROT_VELOCITY);

		if (Magnitude(m_Velocity) < LANDING_VELOCITY)
		{
			mbLand = true;
			rvector	right;
			m_Up = rvector(0, 1, 0);
			D3DXVec3Cross(&right, &m_Dir, &m_Up);
			D3DXVec3Cross(&m_Dir, &right, &m_Up);
			D3DXMatrixIdentity(&mRotMatrix);
		}
		else
		{
			rmatrix	Temp;
			D3DXMatrixRotationAxis(&Temp, &m_RotAxis, mRotVelocity * 0.001);
			mRotMatrix = mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp;
		D3DXMatrixRotationX(&Temp, mRotVelocity * 0.001);
		mRotMatrix = mRotMatrix * Temp;
		mRotVelocity *= 0.97;
	}

	MakeWorldMatrix(&Mat, m_Position, m_Dir, m_Up);
	Mat = mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix(Mat);

	ZGetWorld()->GetFlags()->CheckSpearing(oldPos, m_Position, ROCKET_SPEAR_EMBLEM_POWER);

	return true;
}

//////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponSpySmokeGrenade, ZWeaponGrenade);

#define SPY_SMOKE_GRENADE_LIFETIME	40
#define SPY_SMOKE_GRENADE_EXPLOSION	0.3f

const float ZWeaponSpySmokeGrenade::mcfTrigerTimeList[SPY_NUM_SMOKE] =
{
	0.f, 0.01f, 0.03f, 0.06f, 0.1f, 0.15f, 0.21f, 0.28f, 0.36f, 0.45f, 0.55f, 0.66f
};

bool ZWeaponSpySmokeGrenade::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;
	float lap = ZGetGame()->GetTime() - m_fStartTime;

	if (lap >= SPY_SMOKE_GRENADE_LIFETIME)
	{
		return false;
	}

	if (miSmokeIndex < SPY_NUM_SMOKE && lap - SPY_SMOKE_GRENADE_EXPLOSION >= mcfTrigerTimeList[miSmokeIndex])
	{
		Explosion();
		++miSmokeIndex;
	}

	m_Velocity.z -= 1000.f * fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff = m_Velocity * fElapsedTime;
		rvector dir = diff;
		Normalize(dir);

		float fDist = Magnitude(diff);

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);

		if (bPicked)
		{
			if (zpi.bBspPicked)
			{
				pickpos = zpi.bpi.PickPos;
				rplane plane = zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal = rvector(plane.a, plane.b, plane.c);
			}
			else if (zpi.pObject)
			{
				pickpos = zpi.info.vOut;
				if (zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f)
				{
					normal = pickpos - zpi.pObject->GetPosition();
					normal.z = 0;
				}
				else
				{
					normal = pickpos - (zpi.pObject->GetPosition() + rvector(0, 0, 90));
				}
				Normalize(normal);
			}

			pickpos += normal * BOUND_EPSILON;
		}

		if (bPicked && fabsf(Magnitude(pickpos - m_Position)) < (fDist + BOUND_EPSILON))
		{
			m_Position = pickpos + normal;
			m_Velocity = GetReflectionVector(m_Velocity, normal);
			m_Velocity *= zpi.pObject ? 0.4f : 0.8f;

			Normalize(normal);
			float fAbsorb = DotProduct(normal, m_Velocity);
			m_Velocity -= 0.5 * fAbsorb * normal;

			float fA = RANDOMFLOAT * 2 * pi;
			float fB = RANDOMFLOAT * 2 * pi;
			m_RotAxis = rvector(sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB));

		}
		else
		{
			m_Position += diff;
		}
	}

	rmatrix Mat;

	if (!mbLand)
	{
		mRotVelocity = min(Magnitude(m_Velocity), MAX_ROT_VELOCITY);

		if (Magnitude(m_Velocity) < LANDING_VELOCITY)
		{
			mbLand = true;
			rvector	right;
			m_Up = rvector(0, 1, 0);
			D3DXVec3Cross(&right, &m_Dir, &m_Up);
			D3DXVec3Cross(&m_Dir, &right, &m_Up);
			D3DXMatrixIdentity(&mRotMatrix);
		}
		else
		{
			rmatrix	Temp;
			D3DXMatrixRotationAxis(&Temp, &m_RotAxis, mRotVelocity * 0.001);
			mRotMatrix = mRotMatrix * Temp;
		}
	}
	else
	{
		rmatrix Temp;
		D3DXMatrixRotationX(&Temp, mRotVelocity * 0.001);
		mRotMatrix = mRotMatrix * Temp;
		mRotVelocity *= 0.97;
	}

	MakeWorldMatrix(&Mat, m_Position, m_Dir, m_Up);
	Mat = mRotMatrix * Mat;
	m_pVMesh->SetWorldMatrix(Mat);

	ZGetWorld()->GetFlags()->CheckSpearing(oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER);

	return true;
}

void ZWeaponSpySmokeGrenade::Explosion()
{
	ZGetEffectManager()->AddSmokeGrenadeEffect(m_Position);
	mRotVelocity *= 10;

	ZGetSoundEngine()->PlaySound("we_gasgrenade_explosion", m_Position);
}

//////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponSpyTrap, ZMovingWeapon);

bool ZWeaponSpyTrap::Init(RMesh* pMesh, rvector& pos, rvector& velocity, int nItemId, ZObject* pOwner)
{
	ZMovingWeapon::Create(pMesh);

	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemId);
	if (!pDesc) return false;

	m_fDamage = pDesc->m_nDamage.Ref();
	m_nItemId = nItemId;

	m_Position = pos;
	m_Velocity = velocity;

	m_Dir = rvector(1, 0, 0);
	m_Up = rvector(0, 0, 1);
	m_RotAxis = rvector(0, 0, 1);

	m_uidOwner = pOwner->GetUID();
	m_nTeamID = pOwner->GetTeamID();

	m_fThrowedTime = ZGetGame()->GetTime();

	m_bLanded = false;
	m_fLandedTime = 0;

	m_bActivated = false;
	m_fActivatedTime = 0;
	m_fNextEffectTime = 0;

	m_bFriendly = false;
	ZObject* pMyChar = ZGetCharacterManager()->Find(ZGetMyUID());
	if (!ZGetGame()->CanAttack(pOwner, pMyChar))
		m_bFriendly = true;
	else if (pOwner && pOwner->GetUID() == ZGetMyUID())
		m_bFriendly = true;

	m_nDamageType = pDesc->m_nDamageType.Ref();

	return true;
}

bool ZWeaponSpyTrap::Create(RMesh* pMesh, rvector& pos, rvector& velocity, int nItemId, ZObject* pOwner)
{
	if (!Init(pMesh, pos, velocity, nItemId, pOwner)) return false;

	// ﾁ､ｸｻ ﾇﾘｴ・ｾﾆﾀﾌﾅﾛﾀｻ ﾀ蠎�ﾇﾏｰ・ﾀﾖｴﾂｰ｡
	if (pOwner->GetItems())
	{
		bool bEquiped = false;
		for (int i = MMCIP_CUSTOM1; i <= MMCIP_CUSTOM2; ++i)
		{
			ZItem* pItem = pOwner->GetItems()->GetItem((MMatchCharItemParts)i);
			if (pItem->GetDesc() && pItem->GetDesc()->m_nID == nItemId)
				bEquiped = true;
		}

		if (!bEquiped) return false;
	}

	// ｼｭｹ�ｿ｡ ﾆｮｷｦﾀｻ ｴ�ﾁ�ｴﾙｴﾂ ｰﾍﾀｻ ｺｸｳｽｴﾙ. ｼｭｹ�ｴﾂ ｹｺｰ｡ ｴ�ﾁｮﾁｳｴﾙｴﾂ ｰﾍｸｸ ｱ篝・ﾘｵﾎｾ�ｴﾙｰ｡ 
	// ﾀﾌﾈﾄｿ｡ ﾆｮｷｦﾀﾇ ｹﾟｵｿ ﾅ・ｸｸｦ ｴﾙｽﾃ ｹﾞﾀｻｶｧ ｾﾆ ｾﾆｱ・ｴ�ﾁｮﾁ・ｾﾖｰ｡ ｾ�ﾑ�ｿ｡ｼｭ ｹﾟｵｿｵﾆｱｸｳｪ ﾇﾏｰ・ﾃｼﾅｩﾇﾏｸ・ｵﾈｴﾙ.
	// ｾ�ﾒｲ ﾆｮｷｦﾀﾌ ｴ�ﾁｮﾁｳﾀｸｳｪ ｾﾆﾁ・ｹﾟｵｿｵﾇﾁ・ｾﾊﾀｺ ｻ�ﾅﾂﾀﾏ ｶｧ ｳｭﾀﾔﾇﾑ ﾀｯﾀ�ｿ｡ｰﾔｴﾂ ﾀﾌ ｹﾟｵｿｵﾈ ﾆｮｷｦｿ｡ ｴ・ﾘｼｭ ｼｭｹ�ｰ｡ ｺｰｵｵｷﾎ ｸﾞｽﾃﾁ�ｸｦ ｺｸｳｻﾁﾘｴﾙ
	if (m_uidOwner == ZGetMyUID())
		ZPostNotifyThrowedTrap(m_nItemId);

	return true;
}

bool ZWeaponSpyTrap::CreateActivated(RMesh* pMesh, rvector& pos, float fActivatedTime, int nItemId, ZObject* pOwner)
{
	if (!Init(pMesh, pos, rvector(0, 0, 0), nItemId, pOwner))
		return false;

	m_fActivatedTime = fActivatedTime;
	m_fNextEffectTime = m_fActivatedTime;
	m_bActivated = true;

	rmatrix mat;
	MakeWorldMatrix(&mat, m_Position, m_Dir, m_Up);
	m_pVMesh->SetWorldMatrix(mat);

	float fElapsedTime = ZGetGame()->GetTime() - m_fActivatedTime;
	AddEffectActivating(fElapsedTime * 1000, m_bFriendly);

	return true;
}

bool ZWeaponSpyTrap::Update(float fElapsedTime)
{
	if (m_bActivated)
		return UpdateOnActivated(fElapsedTime);

	if (m_bLanded)
	{
		Activate();
		return true;
	}

	if (m_Position.z < DIE_CRITICAL_LINE)
		return false;

	if (ZGetGame()->GetTime() - m_fThrowedTime > MAX_TRAP_THROWING_LIFE)
		return false;

	rvector oldPos = m_Position;

	m_Velocity.z -= 1000.f * fElapsedTime;

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff = m_Velocity * fElapsedTime;
		rvector dir = diff;
		Normalize(dir);

		float fDist = Magnitude(diff);

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);
		if (bPicked)
		{
			if (zpi.bBspPicked)
			{
				pickpos = zpi.bpi.PickPos;
				rplane plane = zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal = rvector(plane.a, plane.b, plane.c);
			}
			else if (zpi.pObject)
			{
				pickpos = zpi.info.vOut;
				if (zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f)
				{
					normal = pickpos - zpi.pObject->GetPosition();
					normal.z = 0;
				}
				else
					normal = pickpos - (zpi.pObject->GetPosition() + rvector(0, 0, 90));
				Normalize(normal);
			}
		}


		// ｾ�ﾑ�ｰ｡ｿ｡ ﾃ豬ｹﾇﾟｴﾙ
		if (bPicked && fabsf(Magnitude(pickpos - m_Position)) < fDist)
		{
			m_Position = pickpos + normal;
			m_Velocity = GetReflectionVector(m_Velocity, normal);
			m_Velocity *= zpi.pObject ? 0.4f : 0.8f;		// ﾄｳｸｯﾅﾍｿ｡ ﾃ豬ｹﾇﾏｸ・ｼﾓｵｵｰ｡ ｸｹﾀﾌ ﾁﾘｴﾙ

			if (zpi.bBspPicked) {
				ZGetSoundEngine()->PlaySound("we_grenade_fire", m_Position);
			}

			// ｺｮｹ貮簑ﾇ ｼﾓｵｵｼｺｺﾐﾀｻ ﾁﾙﾀﾎｴﾙ.
			Normalize(normal);
			float fAbsorb = DotProduct(normal, m_Velocity);
			m_Velocity -= 0.5 * fAbsorb * normal;

			float fA = RANDOMFLOAT * 2 * pi;
			float fB = RANDOMFLOAT * 2 * pi;
			m_RotAxis = rvector(sin(fA) * sin(fB), cos(fA) * sin(fB), cos(fB));

			// ﾁ�ｸ鯀｡ ｴ・ﾒﾀｻ ｶｧ ﾀﾏﾁ､ｽﾃｰ｣ ﾈﾄ ﾆ�ｹﾟ (ｺｮﾀﾌｳｪ ﾃｵﾀ・ﾃ豬ｹﾀｺ ﾆﾐｽｺ)
			if (DotProduct(normal, rvector(0, 0, 1)) > 0.8f)
			{
				if (!m_bLanded)
				{
					m_bLanded = true;
					m_fLandedTime = ZGetGame()->GetTime();

					// ﾆｮｷｦﾀﾌ ｼｳﾄ｡ｵﾈ ﾀｧﾄ｡ｸｦ ｾﾋ ｼ・ﾀﾖｰﾔ ｰ｡ﾀﾌｵ蟶ｦ ﾁｶｱﾗｸﾄｰﾔ ﾇ･ｽﾃﾇﾏﾀﾚ
					ZGetEffectManager()->AddTrapGuideEffect(m_Position, rvector(0, 1, 0), int(TRAP_LANDING_TIME * 1000), m_bFriendly, 0.1f, 12.f);
				}
				return true;
			}

		}
		else
			m_Position += diff;
	}

	float fRotSpeed = Magnitude(m_Velocity) * 0.04f;

	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed * fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;

	rmatrix mat;
	rvector dir = m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat, m_Position, m_Dir, m_Up);

	mat = rotmat * mat;

	m_pVMesh->SetWorldMatrix(mat);

	ZGetWorld()->GetFlags()->CheckSpearing(oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER);

	return true;
}

void ZWeaponSpyTrap::Activate()
{
	m_bActivated = true;
	m_fActivatedTime = ZGetGame()->GetTime();
	m_fNextEffectTime = m_fActivatedTime;

	AddEffectActivating(0, m_bFriendly);

	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource(m_Position, 3.0f, 1300);

	// ｼｭｹ�ｿ｡ ｹﾟｵｿｵﾈ ﾆｮｷｦﾀｻ ｵ�ﾓﾏﾇﾘｼｭ ﾀﾌﾈﾄ ｳｭﾀﾔﾇﾏｴﾂ ﾀｯﾀ�ｰ｡ ｿ�ｵ蠢｡ ｼｳﾄ｡ｵﾈ ﾆｮｷｦｵ鯊ｻ ｾﾋ ｼ・ﾀﾖｵｵｷﾏ ﾇﾏﾀﾚ
	if (m_uidOwner == ZGetMyUID())
		ZPostNotifyActivatedTrap(m_nItemId, m_Position);
}

void ZWeaponSpyTrap::AddEffectActivating(int nElapsedIfActivated, bool bFriendly)
{
	// ｹﾟｵｿﾀﾌ ﾁ�ｼﾓｵﾉ ｵｿｾﾈ ｺｸｿｩﾁ・ﾀﾌﾆ衄ｮ ｼｼﾆﾃ
	MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nItemId);
	if (!pItemDesc) return;
	int nLifeTime = pItemDesc->m_nLifeTime.Ref();

	ZGetEffectManager()->AddTrapGuideEffect(m_Position, rvector(0, 1, 0), nLifeTime - nElapsedIfActivated, bFriendly);
}

bool ZWeaponSpyTrap::UpdateOnActivated(float fElapsedTime)
{
	if (m_bActivated)
	{
		MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_nItemId);
		if (!pItemDesc) return false;

		float fCurrTime = ZGetGame()->GetTime();
		if (fCurrTime - m_fActivatedTime > pItemDesc->m_nLifeTime.Ref() * 0.001f)
			return false;

		if (fCurrTime > m_fNextEffectTime)
		{
			m_fNextEffectTime += 1.f;	// 1ﾃﾊｸｶｴﾙ ﾀﾌﾆ衄ｮｸｦ ﾃ箙ﾂﾇﾑｴﾙ
			switch (m_nDamageType)
			{
			case MMDT_FIRE:
				ZGetEffectManager()->AddTrapFireEffect(m_Position, rvector(0, 1, 0));
				break;
			case MMDT_COLD:
				ZGetEffectManager()->AddTrapColdEffect(m_Position, rvector(0, 1, 0));
				break;
			default:
				_ASSERT(0);
			}
		}

		ZGetGame()->CheckZoneTrap(m_uidOwner, m_Position, pItemDesc, m_nTeamID);
		return true;
	}
	return true;
}

//////////////////////////////////////////////////////////////

MImplementRTTI(ZWeaponStunGrenade, ZMovingWeapon);

void ZWeaponStunGrenade::Create(RMesh* pMesh, rvector &pos, rvector &velocity, ZObject* pOwner)
{
	ZWeapon::Create(pMesh);

	rvector dir = velocity;
	Normalize(dir);

	m_Velocity = velocity;
	m_Position = pos;

	m_bCollided = false;
	m_fStartTime = 0.f;

	m_Dir = rvector(1, 0, 0);
	m_Up = rvector(0, 0, 1);
	m_RotAxis = rvector(0, 0, 1);

	m_uidOwner = pOwner->GetUID();
	m_nTeamID = pOwner->GetTeamID();

	MMatchItemDesc* pDesc = NULL;

	if (pOwner->GetItems()) {
		if (pOwner->GetItems()->GetSelectedWeapon()) {
			pDesc = pOwner->GetItems()->GetSelectedWeapon()->GetDesc();
		}
	}

	if (pDesc == NULL) {
		_ASSERT(0);
		return;
	}

	m_fDamage = pDesc->m_nDamage.Ref();
	m_nSoundCount = (rand() % 2) + 2;
}

#define DYNAMITE_LIFE 0.7f	// ﾃｹｹ�ﾂｰ ﾃ豬ｹ ﾈﾄ 0.7ﾃﾊﾈﾄｿ｡ ﾅﾍﾁ"

bool ZWeaponStunGrenade::Update(float fElapsedTime)
{
	rvector oldPos = m_Position;

	if (m_Position.z < DIE_CRITICAL_LINE)
		return false;

	// ﾃｳﾀｽﾀｸｷﾎ ｺﾎｵ�ﾈ"ｶｧｺﾎﾅﾍﾀﾇ ｽﾃｰ｣ﾀｻ ﾃｼﾅｩﾇﾏｿｩ ﾆ�ｹﾟ
	if (m_bCollided &&
		ZGetGame()->GetTime() - m_fStartTime > DYNAMITE_LIFE)
	{
		Explosion();
		if (Z_VIDEO_DYNAMICLIGHT)
			ZGetStencilLight()->AddLightSource(m_Position, 3.0f, 1300);
		return false;
	}

	m_Velocity.z = m_Velocity.z - (1000.f * fElapsedTime);

	const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET;

	{
		rvector diff = m_Velocity * fElapsedTime;
		rvector dir = diff;
		Normalize(dir);

		float fDist = Magnitude(diff);

		rvector pickpos, normal;

		ZPICKINFO zpi;
		bool bPicked = ZGetGame()->Pick(ZGetCharacterManager()->Find(m_uidOwner), m_Position, dir, &zpi, dwPickPassFlag);
		if (bPicked)
		{
			if (zpi.bBspPicked)
			{
				pickpos = zpi.bpi.PickPos;
				rplane plane = zpi.bpi.pNode->pInfo[zpi.bpi.nIndex].plane;
				normal = rvector(plane.a, plane.b, plane.c);
			}
			else
			{
				if (zpi.pObject)
				{
					pickpos = zpi.info.vOut;
					if (zpi.pObject->GetPosition().z + 30.f <= pickpos.z && pickpos.z <= zpi.pObject->GetPosition().z + 160.f)
					{
						normal = pickpos - zpi.pObject->GetPosition();
						normal.z = 0;
					}
					else
					{
						normal = pickpos - (zpi.pObject->GetPosition() + rvector(0, 0, 90));
					}

					Normalize(normal);
				}
			}
		}
		if (bPicked && fabsf(Magnitude(pickpos - m_Position))<fDist)
		{
			if (!m_bCollided)
			{
				m_bCollided = true;
				m_fStartTime = ZGetGame()->GetTime();
			}

			m_Position = pickpos + normal;
			m_Velocity = GetReflectionVector(m_Velocity, normal);
			m_Velocity *= zpi.pObject ? 0.4f : 0.8f;
			if (zpi.bBspPicked && m_nSoundCount > 0)
			{
				m_nSoundCount--;
				ZGetSoundEngine()->PlaySound("we_grenade_fire", m_Position);
			}
			Normalize(normal);
			float fAbsorb = DotProduct(normal, m_Velocity);
			m_Velocity -= 0.5*fAbsorb*normal;

			float fA = RANDOMFLOAT * 2 * pi;
			float fB = RANDOMFLOAT * 2 * pi;
			m_RotAxis = rvector(sin(fA)*sin(fB), cos(fA)*sin(fB), cos(fB));

		}
		else
			m_Position += diff;
	}

	float fRotSpeed = Magnitude(m_Velocity) * 0.04f;
	rmatrix rotmat;
	D3DXQUATERNION q;
	D3DXQuaternionRotationAxis(&q, &m_RotAxis, fRotSpeed *fElapsedTime);
	D3DXMatrixRotationQuaternion(&rotmat, &q);
	m_Dir = m_Dir * rotmat;
	m_Up = m_Up * rotmat;
	rmatrix mat;
	rvector dir = m_Velocity;
	Normalize(dir);
	MakeWorldMatrix(&mat, m_Position, m_Dir, m_Up);
	mat = rotmat*mat;
	m_pVMesh->SetWorldMatrix(mat);
	ZGetWorld()->GetFlags()->CheckSpearing(oldPos, m_Position, GRENADE_SPEAR_EMBLEM_POWER);
	return true;
}


void ZWeaponStunGrenade::Explosion()
{
	rvector dir = -RealSpace2::RCameraDirection;
	dir.z = 0.f;
	ZGetEffectManager()->AddSmokeEffect(m_Position);
	ZGetEffectManager()->AddDynamiteEffect(m_Position, dir);
	ZGetSoundEngine()->PlaySound("we_grenade_explosion", m_Position);
	ZGetGame()->OnExplosionStunGrenade(m_uidOwner, m_Position, m_fDamage, 400.f, 1.f, m_nTeamID);
	ZGetWorld()->GetFlags()->SetExplosion(m_Position, EXPLOSION_EMBLEM_POWER);
}
