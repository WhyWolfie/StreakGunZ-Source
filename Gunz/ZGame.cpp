#include "stdafx.h"

#include "ZGame.h"
#include <windows.h> 
//#include <zmouse.h>

#include "MZFileSystem.h"
#include "RealSpace2.h"
#include "FileInfo.h"
#include "MDebug.h"
#include "MBlobArray.h"
#include "MObject.h"
#include "ZConsole.h"
#include "MCommandLogFrame.h"
#include "ZInterface.h"
#include "ZGameInterface.h"
#include "ZApplication.h"
#include "ZCommandTable.h"
#include "ZPost.h"
#include "ZPostLocal.h"
#include "MStrEx.h"
#include "MMatchItem.h"
#include "ZEffectManager.h"
#include "ZEffectBillboard.h"
#include "Config.h"
#include "MProfiler.h"
#include "MMatchItem.h"
#include "ZScreenEffectManager.h"
#include "RParticleSystem.h"
#include "RDynamicLight.h"
#include "ZConfiguration.h"
#include "ZLoading.h"
#include "Physics.h"
#include "zeffectflashbang.h"
#include "ZInitialLoading.h"
#include "RealSoundEffect.h"
#include "RLenzFlare.h"
#include "ZWorldItem.h"
#include "ZMyInfo.h"
#include "ZNetCharacter.h"
#include "ZSecurity.h"
#include "ZStencilLight.h"
#include "ZMap.h"
#include "ZEffectStaticMesh.h"
#include "ZEffectAniMesh.h"
#include "ZGameAction.h"
#include "ZSkyBox.h"
#include "ZFile.h"
#include "ZObject.h"
#include "ZCharacter.h"
#include "ZActor.h"
#include "ZActorActionManager.h"
#include "ZMapDesc.h"
#include "MXml.h"
#include "ZGameClient.h"
#include "MUtil.h"
#include "RMeshMgr.h"
#include "RVisualMeshMgr.h"
#include "RMaterialList.h"
#include "RAnimationMgr.h"
#include "ZCamera.h"
#include "Mint4R2.h"
//#include "RParticleSystem.h"
#include "ZItemDesc.h"

//#include "MObjectCharacter.h"
#include "MMath.h"
#include "ZQuest.h"
#include "MMatchUtil.h"
#include "ZReplay.h"
#include "ZRuleBerserker.h"
#include "ZRuleDuelTournament.h"
#include "ZRuleQuestChallenge.h"
#include "ZApplication.h"
#include "ZGameConst.h"

#include "ZRuleDuel.h"
#include "ZMyCharacter.h"
#include "MMatchCRC32XORCache.h"
#include "MMatchObjCache.h"

#include "ZModule_HealOverTime.h"
#include "ZRuleDeathMatch.h"
#include "ZActorWithFSM.h"
#include "ZNavigationMesh.h"

#include "ZRuleSpy.h"
#include "RGMain.h"
#include "VoiceChat.h"
#include <random>
#include "RBspObject.h"
#include "CodePageConversion.h"

#include "ZFunctions.h" // lol 
// Custom: Disable NHN auth
//#ifdef LOCALE_NHNUSA
//#include "ZNHN_USA_Report.h"
//#endif


extern DWORD g_ShouldBanPlayer;

_USING_NAMESPACE_REALSPACE2

#define ENABLE_CHARACTER_COLLISION		// ĳ���ͳ��� �浹üũ 
#define ENABLE_ADJUST_MY_DATA			// ��ǥ�ý��� ����

/////////////////////////////////////////////////////////////////////
//ä�� ���ڿ� ��?E��?E...�ٹٲ� ���ڸ� ��?E....
void CheckMsgAboutChat(char * msg)
{
	//���⼭ ä��?E�E�Ÿ��ºκ�.....
	int lenMsg = (int)strlen(msg);
	for( int i=0; i<lenMsg; i++)
	{
		if(msg[i] == '\n' || msg[i] == '\r')
		{
			msg[i] = NULL;
			break;
		}
	}
}

#ifdef _RAINSYSTEM
struct RRainParticle : public RParticle, CMemPoolSm<RRainParticle>
{

	virtual bool Update(float fTimeElapsed);
};


bool RRainParticle::Update(float fTimeElapsed)
{
	RParticle::Update(fTimeElapsed);

	if (position.z <= -1000.0f) return false;
	return true;
}

class ZRainTownParticleSystem
{
private:
	RParticles* m_pParticlesRain[3];
	bool IsRainTownMap()
	{
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Snow", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Island", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Garden", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Battle Arena", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Factory", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Town", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Citadel", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Ruin", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Port", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Lost Shrine", 4)) return true;

		return false;
	}
public:
	void Update(float fDeltaTime)
	{
		if (!IsRainTownMap()) return;

#define RAIN_PARTICLE_COUNT_PER_SECOND		800

		int nRainParticleCountPerSec = RAIN_PARTICLE_COUNT_PER_SECOND;
		switch (ZGetConfiguration()->GetVideo()->nEffectLevel)
		{
		case Z_VIDEO_EFFECT_HIGH:	break;
		case Z_VIDEO_EFFECT_NORMAL:	nRainParticleCountPerSec = nRainParticleCountPerSec / 8; break;
		case Z_VIDEO_EFFECT_LOW:	nRainParticleCountPerSec = nRainParticleCountPerSec / 12; break;
		default: nRainParticleCountPerSec = 8; break;
		}

		int nCount = min(nRainParticleCountPerSec * fDeltaTime, 20);
		for (int i = 0; i < nCount; i++)
		{
			RParticle* pp = new RRainParticle();
			pp->ftime = 0;

			// RainsDrops
			/*pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(40.0f, 40.0f), RandomNumber(40.0f, 40.0f), RandomNumber(150.0f, 250.0f));
			pp->accel = rvector(-0, -0, -200.f);*/

			// Night Stars
			pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(0.0f, 0.0f), RandomNumber(0.0f, 0.0f), RandomNumber(0.0f, 0.0f));
			pp->accel = rvector(-0, -0, -200.f);

			int particle_index = RandomNumber(0, 2);
			if (m_pParticlesRain[particle_index]) m_pParticlesRain[particle_index]->push_back(pp);
		}
	}
	void Create()
	{
		if (!IsRainTownMap()) return;

		for (int i = 0; i < 3; i++)
		{
			m_pParticlesRain[i] = NULL;
		}

		m_pParticlesRain[0] = RGetParticleSystem()->AddParticles("sfx/raindrops.bmp", 25.0f);
		m_pParticlesRain[1] = RGetParticleSystem()->AddParticles("sfx/raindrops.bmp", 20.0f);
		m_pParticlesRain[2] = RGetParticleSystem()->AddParticles("sfx/raindrops.bmp", 15.0f);
	}
	void Destroy()
	{
		if (!IsRainTownMap()) return;

		if (m_pParticlesRain[0])
		{
			m_pParticlesRain[0]->Clear();
			m_pParticlesRain[0] = NULL;
		}
		if (m_pParticlesRain[1])
		{
			m_pParticlesRain[1]->Clear();
			m_pParticlesRain[1] = NULL;
		}
		if (m_pParticlesRain[2])
		{
			m_pParticlesRain[2]->Clear();
			m_pParticlesRain[2] = NULL;
		}
	}
};

static ZRainTownParticleSystem g_RainTownParticleSystem;
#else
struct RSnowParticle : public RParticle, CMemPoolSm<RSnowParticle>
{

	virtual bool Update(float fTimeElapsed);
};


bool RSnowParticle::Update(float fTimeElapsed)
{
	RParticle::Update(fTimeElapsed);

	if (position.z <= -1000.0f) return false;
	return true;
}

class ZSnowTownParticleSystem
{
private:
	RParticles* m_pParticles[3];
	bool IsSnowTownMap()
	{
#ifdef _XMAS
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Snow", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Island", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Garden", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Battle Arena", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Factory", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Town", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "High_Haven", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Citadel", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Ruin", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Port", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Castle", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Lost Shrine", 4)) return true;
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "Stairway", 4)) return true;
#else
		if (!strnicmp(ZGetGameClient()->GetMatchStageSetting()->GetMapName(), "snow", 4)) return true;
#endif

		return false;
	}
public:
	void Update(float fDeltaTime)
	{
		if (!IsSnowTownMap()) return;

#define SNOW_PARTICLE_COUNT_PER_SECOND		400		// ?E?��?E300��?E���� ������?E14������.

		int nSnowParticleCountPerSec = SNOW_PARTICLE_COUNT_PER_SECOND;
		switch (ZGetConfiguration()->GetVideo()->nEffectLevel)
		{
		case Z_VIDEO_EFFECT_HIGH:	break;
		case Z_VIDEO_EFFECT_NORMAL:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 4; break;
		case Z_VIDEO_EFFECT_LOW:	nSnowParticleCountPerSec = nSnowParticleCountPerSec / 8; break;
		default: nSnowParticleCountPerSec = 0; break;
		}

		int nCount = min(nSnowParticleCountPerSec * fDeltaTime, 20);	// �ѹ��� 20�� �̻��� �ȳ��������Ѵ�
		for (int i = 0; i < nCount; i++)
		{
			RParticle* pp = new RSnowParticle();
			pp->ftime = 0;
			pp->position = rvector(RandomNumber(-8000.0f, 8000.0f), RandomNumber(-8000.0f, 8000.0f), 1500.0f);
			pp->velocity = rvector(RandomNumber(-40.0f, 40.0f), RandomNumber(-40.0f, 40.0f), RandomNumber(-150.0f, -250.0f));
			pp->accel = rvector(0, 0, -5.f);

			int particle_index = RandomNumber(0, 2);
			if (m_pParticles[particle_index]) m_pParticles[particle_index]->push_back(pp);
		}
	}
	void Create()
	{
		if (!IsSnowTownMap()) return;

		for (int i = 0; i < 3; i++)
		{
			m_pParticles[i] = NULL;
		}

		// ��ƼŬ ����
		m_pParticles[0] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 25.0f);
		m_pParticles[1] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 10.0f);
		m_pParticles[2] = RGetParticleSystem()->AddParticles("sfx/water_splash.bmp", 5.0f);
	}
	void Destroy()
	{
		if (!IsSnowTownMap()) return;

		// Custom: Checked for NULL pointer in m_pParticles
		if (m_pParticles[0])
		{
			m_pParticles[0]->Clear();
			m_pParticles[0] = NULL;
		}
		if (m_pParticles[1])
		{
			m_pParticles[1]->Clear();
			m_pParticles[1] = NULL;
		}
		if (m_pParticles[2])
		{
			m_pParticles[2]->Clear();
			m_pParticles[2] = NULL;
		}
	}
};

static ZSnowTownParticleSystem g_SnowTownParticleSystem;
#endif

ZGame* g_pGame = NULL;

float	g_fFOV = DEFAULT_FOV;
float	g_fFarZ = DEFAULT_FAR_Z;


//RParticleSystem* g_ParticleSystem = 0;
extern sCharacterLight g_CharLightList[NUM_LIGHT_TYPE];

MUID tempUID(0, 0);		// ���� �׽�Ʈ?E
MUID g_MyChrUID(0, 0);

#define IsKeyDown(key) ((GetAsyncKeyState(key) & 0x8000)!=0)

void TestCreateEffect(int nEffIndex)
{
	float fDist = RandomNumber(0.0f, 100.0f);
	rvector vTar = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));
	rvector vPos = ZGetGame()->m_pMyCharacter->GetPosition();
	vPos.x += RandomNumber(0.0f, 100.0f);
	vPos.y += RandomNumber(0.0f, 100.0f);
	vPos.z += RandomNumber(0.0f, 100.0f);

	rvector vTarNormal = -RealSpace2::RCameraDirection;

	vTarNormal = rvector(RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f), RandomNumber(0.0f, 100.0f));


	ZCharacter* pCharacter = ZGetGame()->m_pMyCharacter;
	ZEffectManager* pEM = ZGetEffectManager();

	switch (nEffIndex)
	{
	case 0:
		pEM->AddReBirthEffect(vPos);
		break;
	case 1:
		pEM->AddLandingEffect(vPos, vTarNormal);
		break;
	case 2:
		pEM->AddGrenadeEffect(vPos, vTarNormal);
		break;
	case 3:
		pEM->AddRocketEffect(vPos, vTarNormal);
		break;
	case 4:
		pEM->AddRocketSmokeEffect(vPos);
		break;
	case 5:
		pEM->AddSwordDefenceEffect(vPos,-vTarNormal);
		break;
	case 6:
		pEM->AddSwordWaveEffect(vPos, 200, pCharacter);
		break;
	case 7:
		pEM->AddSwordUppercutDamageEffect(vPos, pCharacter->GetUID());
		break;
	case 8:
		pEM->AddBulletMark(vPos, vTarNormal);
		break;
	case 9:
		pEM->AddShotgunEffect(vPos, vPos, vTar, pCharacter);
		break;
	case 10:
		pEM->AddBloodEffect(vPos, vTarNormal);
		break;
	case 11:
		for (int i = 0; i < SEM_End; i++)
			pEM->AddSlashEffect(vPos, vTarNormal, i);
		break;
	case 12:
		pEM->AddSlashEffectWall(vPos, vTarNormal,0);
		break;
	case 13:
		pEM->AddLightFragment(vPos, vTarNormal);
		break;
	case 14:
		//pEM->AddDashEffect(vPos, vTarNormal, pCharacter);
		pEM->AddDashEffect(vPos, vTarNormal, pCharacter, 0);
		break;
	case 15:
		pEM->AddSmokeEffect(vPos);
		break;
	case 16:
		pEM->AddSmokeGrenadeEffect(vPos);
		break;
	case 17:
		pEM->AddGrenadeSmokeEffect(vPos, 1.0f, 0.5f, 123);
		break;
	case 18:
		pEM->AddWaterSplashEffect(vPos, vPos);
		break;
	case 19:
		pEM->AddWorldItemEatenEffect(vPos);
		break;
	case 20:
		pEM->AddCharacterIcon(pCharacter, 0);
		break;
	case 21:
		pEM->AddCommanderIcon(pCharacter, 0);
		break;
	case 22:
		pEM->AddChatIcon(pCharacter);
		break;
	case 23:
		pEM->AddLostConIcon(pCharacter);
		break;
	case 24:
		pEM->AddChargingEffect(pCharacter);
		break;
	case 25:
		pEM->AddChargedEffect(pCharacter);
		break;
	case 26:
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		pEM->AddTrackFire(vPos);
		break;
	case 27:

		ZEffectWeaponEnchant* pEWE = pEM->GetWeaponEnchant(ZC_ENCHANT_FIRE);

		if(pEWE) {
			//ǥ�� �������� īŸ��... 100 ����..
			float fSIze = 105.f / 100.f;
			rvector vScale = rvector(0.6f*fSIze,0.6f*fSIze,0.9f*fSIze);// ������ ũ�⿡ ����..
			pEWE->SetUid( pCharacter->GetUID() );
			pEWE->SetAlignType(1);
			pEWE->SetScale(vScale);
			pEWE->Draw(timeGetTime());
		}

		break;
	}
}

float CalcActualDamage(ZObject* pAttacker, ZObject* pVictim, float fDamage)
{
	if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_BERSERKER)
	{
		ZRuleBerserker* pRule = (ZRuleBerserker*)ZGetGame()->GetMatch()->GetRule();
		if ((pAttacker) && (pAttacker != pVictim) && (pAttacker->GetUID() == pRule->GetBerserkerUID()))
		{
			return fDamage * BERSERKER_DAMAGE_RATIO;			
		}
	}

	/*if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_GUNGAME) // Only hit kill
	{
		return fDamage * 2.0f;
	}*/

	return fDamage;
}

void TestCreateEffects()
{
	int nCount = 100;

	for (int i = 0; i < nCount; i++)
	{
		int nEffIndex = RandomNumber(25, 28);
		TestCreateEffect(nEffIndex);
	}
}

bool CompareZCharFloat (pair<ZCharacter*, float> i, pair<ZCharacter*, float> j) { return (i.second < j.second); }

ZGame::ZGame()
{
	m_bShowWireframe = false;
	m_pMyCharacter = NULL;

	m_bReserveObserver = false;

//	memset(m_nLastTime, 0, sizeof(DWORD) * ZLASTTIME_MAX);
	for(int i=0;i<ZLASTTIME_MAX;i++)
	{
		m_nLastTime[i] = timeGetTime();
	}

//	m_fTime = 0.f;
	m_fTime.Set_MakeCrc(0.0f);

	m_nReadyState = ZGAME_READYSTATE_INIT;
#ifdef _RAINSYSTEM
	m_pParticlesRain = NULL;
#else
	m_pParticles = NULL;
#endif
	m_render_poly_cnt = 0;
	m_nReservedObserverTime = 0;
	m_nSpawnTime = 0;
	m_t = 0;
	m_DiceRoll = 0;
	m_dwLastInputTime = timeGetTime();

	m_bRecording = false;
//	m_pReplayFile = NULL;
	m_pReplayFile = NULL;

	m_bReplaying.Set_MakeCrc(false);
	m_bShowReplayInfo = true;
	m_bSpawnRequested = false;
	m_ReplayLogTime = 0;
	ZeroMemory(m_Replay_UseItem, sizeof(m_Replay_UseItem));

	m_pGameAction = new ZGameAction;
	DmgCounter = new DamageCounterMap();

	CancelSuicide();
}

ZGame::~ZGame()
{
	m_pMyCharacter = NULL;
	delete m_pGameAction;
#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Destroy();
	RRainParticle::Release();
#else
	g_SnowTownParticleSystem.Destroy();
	RSnowParticle::Release();
#endif
}

bool ZGame::Create(MZFileSystem *pfs, ZLoadingProgress *pLoading )
{
	// Ŭ��?E����� stagestate �� ����ح CastStageBridgePeer �� ���ؼ� ���⼭ȣ?E
	// Custom: Updated string
	mlog("CastStageBridgePeer call in Zgame::Create\n");
	//mlog("CastStageBridgePeer ȣ?Ein Zgame::Create\n");
	ZGetGameClient()->CastStageBridgePeer(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	mlog("game create begin , type = %d\n",ZGetGameClient()->GetMatchStageSetting()->GetGameType());

	SetReadyState(ZGAME_READYSTATE_INIT);	// Sync ��������?EGame Loop ���Ծʵ���

#ifdef _QUEST
	{
		if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
			ZGetQuest()->OnGameCreate();
	}
#endif

//	m_ItemDescManager.Create(FILENAME_ZITEM_DESC);	// ���߿� �־���?E

	// world�� ����
	if (ZGetApplication()->GetLaunchMode()!=ZApplication::ZLAUNCH_MODE_STANDALONE_AI &&
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE) {
		for (int i = 0; i < ZGetQuest()->GetGameInfo()->GetMapSectorCount(); i++)
		{
			MQuestMapSectorInfo* pSecInfo = ZGetQuest()->GetSectorInfo(ZGetQuest()->GetGameInfo()->GetMapSectorID(i));
			if(pSecInfo == NULL)
			{
				char strBuf[256];
				sprintf(strBuf, "[MQuestMapSectorInfo] m_MapSectorVector[index]:%d, GetMapSectorID:%d\n",i, ZGetQuest()->GetGameInfo()->GetMapSectorID(i));
				ASSERT( 0 && strBuf);
			}
			ZGetWorldManager()->AddWorld(pSecInfo->szTitle);
#ifdef _DEBUG
			mlog("map(%s)\n", pSecInfo ? pSecInfo->szTitle : "null");
#endif
		}
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE) {
		//toodk quest �̰� �ӽ� �ʷε��̴�.. �� ���ҽ��� ���� ������ �� ����?E ��?E����� �̷��� ���� if���� ���� ��?E
		// �ƴ�?E���� ������ �°� ?E?���� ����?E�����ؾ߸� �Ѵ�
		//ZGetWorldManager()->AddWorld("Mansion_room2");

		if (!ZRuleQuestChallenge::LoadScenarioMap(ZGetGameClient()->GetMatchStageSetting()->GetMapName())) // �ó����� �̸��� �Ѱܼ� ���Ե� ���� �ε���Ŵ
			return false;
	}
	else{
		ZGetWorldManager()->AddWorld(ZGetGameClient()->GetMatchStageSetting()->GetMapName());
	}

	if(!ZGetWorldManager()->LoadAll(pLoading))
		return false;

	ZGetWorldManager()->SetCurrent(0);

	if(ZGetApplication()->GetLaunchMode()!=ZApplication::ZLAUNCH_MODE_STANDALONE_AI &&
		ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		if(ZGetQuest() && ZGetQuest()->GetGameInfo())
			ZGetWorldManager()->SetCurrent((ZGetQuest()->GetGameInfo()->GetCurrSectorIndex()));
	}

	//RSetCamera(rvector(-10.f,0.f,150.f),rvector(0.f,0.f,0.f),rvector(0.f,1.f,0.f));
	//RSetProjection(DEFAULT_FOV,DEFAULT_NEAR_Z,DEFAULT_FAR_Z);

	//char szMapFileName[256];
	//char szMapPath[64] = "";
	//ZGetCurrMapPath(szMapPath);

	//sprintf(szMapFileName, "%s%s/%s.rs", 
	//	szMapPath,
	//	ZGetGameClient()->GetMatchStageSetting()->GetMapName(),
	//	ZGetGameClient()->GetMatchStageSetting()->GetMapName());

	//if(!strlen(szMapFileName))
	//	return false;

	//mlog("ZGame::Create() :: ReloadAllAnimation Begin \n");
	ZGetMeshMgr()->ReloadAllAnimation();// ��?E���� ���ϸ��̼��� �ִ�?E��?E
	mlog( "Reload all animation end \n" );

	//ZGetInitialLoading()->SetPercentage( 90.f );
//	ZGetInitialLoading()->SetPercentage( 70.f );
//	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );
	
	// �����϶����� PeerList�� ��û�Ѵ�
	if (ZGetGameClient()->IsForcedEntry())
	{
		ZPostRequestPeerList(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
//		ZPostRequestGameInfo(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());
	}

	//if(!GetWorld()->GetBsp()->Open(szMapFileName))
	//{
	//	MLog("error while loading %s \n",szMapFileName);
	//	return false;
	//}

	//mlog("ZGame::Create() GetWorld()->GetBsp()->Open %s \n",szMapFileName);

	//GetWorld()->GetBsp()->OptimizeBoundingBox();

	//ZGetInitialLoading()->SetPercentage( 100.f );

	//GetMapDesc()->Open(&m_bsp);

	g_fFOV = DEFAULT_FOV;
	rvector dir = GetMapDesc()->GetWaitCamDir();
	rvector pos = GetMapDesc()->GetWaitCamPos();
	rvector up(0,0,1);
	RSetCamera(pos, pos+dir, up);



	int nModelID = -1;

	m_Match.Create();
	
	D3DMATERIAL9 mtrl;
	ZeroMemory( &mtrl, sizeof(D3DMATERIAL9) );

	mtrl.Diffuse.r = 1.0f;
	mtrl.Diffuse.g = 1.0f;
	mtrl.Diffuse.b = 1.0f;
	mtrl.Diffuse.a = 1.0f;

	mtrl.Ambient.r = 1.0f;
	mtrl.Ambient.g = 1.0f;
	mtrl.Ambient.b = 1.0f;
	mtrl.Ambient.a = 1.0f;

	RGetDevice()->SetMaterial( &mtrl );

//	m_fTime=0.f;
	m_fTime.Set_CheckCrc(0.0f);
	m_bReserveObserver = false;

	
#ifdef _BIRDSOUND
	ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE);
	ZApplication::GetSoundEngine()->PlayMusic();
#else
		ZApplication::GetSoundEngine()->OpenMusic(BGMID_BATTLE, pfs);
		ZApplication::GetSoundEngine()->PlayMusic();
#endif

	m_CharacterManager.Clear();
	m_ObjectManager.Clear();

	// mlog("ZGame::Create() m_CharacterManager.Clear done \n");

	m_pMyCharacter = (ZMyCharacter*)m_CharacterManager.Add(ZGetGameClient()->GetPlayerUID(), rvector(0.0f, 0.0f, 0.0f),true);

	// Custom: AFK system
	//m_pMyCharacter->m_nLastKeyTime = timeGetTime();

	
	{
		g_CharLightList[GUN].fLife	= 300;
		g_CharLightList[GUN].fRange = 100;
		g_CharLightList[GUN].iType	= GUN;
		g_CharLightList[GUN].vLightColor.x	= 5.0f;
		g_CharLightList[GUN].vLightColor.y	= 1.0f;
		g_CharLightList[GUN].vLightColor.z	= 1.0f;

		g_CharLightList[SHOTGUN].fLife	= 1000;
		g_CharLightList[SHOTGUN].fRange = 150;
		g_CharLightList[SHOTGUN].iType	= SHOTGUN;
		g_CharLightList[SHOTGUN].vLightColor.x	= 6.0f;
		g_CharLightList[SHOTGUN].vLightColor.y	= 1.3f;
		g_CharLightList[SHOTGUN].vLightColor.z	= 1.3f;

		g_CharLightList[CANNON].fLife	= 1300;
		g_CharLightList[CANNON].fRange	= 200;
		g_CharLightList[CANNON].iType	= CANNON;
		g_CharLightList[CANNON].vLightColor.x	= 7.0f;
		g_CharLightList[CANNON].vLightColor.y	= 1.3f;
		g_CharLightList[CANNON].vLightColor.z	= 1.3f;
	}

	ZGetFlashBangEffect()->SetBuffer();
	ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->SetEffectVolume( Z_AUDIO_EFFECT_VOLUME );
	ZGetSoundEngine()->SetMusicVolume( Z_AUDIO_BGM_VOLUME );
#endif

	// Net init
	ZApplication::ResetTimer();
	m_GameTimer.Reset();
	ZSetupDataChecker_Game(&m_DataChecker);
	
	ZGetInitialLoading()->SetPercentage( 100.f );
	ZGetInitialLoading()->Draw( MODE_DEFAULT, 0 , true );


#ifdef _BIRDSOUND

#else
	list<AmbSndInfo*> aslist = GetWorld()->GetBsp()->GetAmbSndList();
	for( list<AmbSndInfo*>::iterator iter = aslist.begin(); iter!= aslist.end(); ++iter )
	{
		AmbSndInfo* pAS = *iter;
		if( pAS->itype & AS_AABB)
			ZGetSoundEngine()->SetAmbientSoundBox(pAS->szSoundName, pAS->min, pAS->max, (pAS->itype&AS_2D)?true:false );
		else if( pAS->itype & AS_SPHERE )
			ZGetSoundEngine()->SetAmbientSoundSphere(pAS->szSoundName, pAS->center, pAS->radius, (pAS->itype&AS_2D)?true:false );
	}
#endif


	// ��?E�� ���ہE. ��?E�ٸ� ��������� �˸���.
	MEMBER_SET_CHECKCRC(m_pMyCharacter->GetStatus(), nLoadingPercent, 100);
	ZPostLoadingComplete(ZGetGameClient()->GetPlayerUID(), 100);
	
	// ���ӿ� ���̬��?E���˸�
	ZPostStageEnterBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameClient()->GetStageUID());

	char tmpbuf[128];
	_strtime( tmpbuf );

	// ���� ȭ�����..

	mlog("game created ( %s )\n",tmpbuf);

	ZGetGameInterface()->GetCamera()->SetLookMode(ZCAMERA_DEFAULT);


#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Create();
#else
	g_SnowTownParticleSystem.Create();
#endif

	return true;
}

void ZGame::Destroy()
{
	//jintriple3 ��?E���� �� �߰��Ѱ�?E?�Ѥ�
	m_DataChecker.Clear();

#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Destroy();
#else
	g_SnowTownParticleSystem.Destroy();
#endif

	SetClearColor(0);

	mlog("game destroy begin\n");

	ZGetGameClient()->AgentDisconnect();

	ZApplication::GetSoundEngine()->StopMusic();
	ZApplication::GetSoundEngine()->CloseMusic();

	mlog("Destroy sound engine()\n");

#ifdef _QUEST
	{
		ZGetQuest()->OnGameDestroy();
	}
#endif

	m_Match.Destroy();

	mlog("game destroy match destroy \n");

	if (ZGetGameClient()->IsForcedEntry())
	{
		ZGetGameClient()->ReleaseForcedEntry();

		ZGetGameInterface()->SerializeStageInterface();
	}

	ZPostStageLeaveBattle(ZGetGameClient()->GetPlayerUID(), ZGetGameInterface()->GetIsGameFinishLeaveBattle());//, ZGetGameClient()->GetStageUID());

	//SAFE_DELETE( g_ParticleSystem );

	ReleaseFlashBangEffect();

	// mlog("ZGame::Destroy SAFE_DELETE(m_pSkyBox) \n");

	RGetLenzFlare()->Clear();

	// ������ ���� ������ �޸� ����?E����...
	//ReleaseMemPool(RealSoundEffectFx);
	//ReleaseMemPool(RealSoundEffect);
	//ReleaseMemPool(RealSoundEffectPlay);

//	int e_size = m_EffectManager.m_Effects[0].size();
//	e_size += m_EffectManager.m_Effects[1].size();
//	e_size += m_EffectManager.m_Effects[2].size();
//	int w_size = m_WeaponManager.m_list.size();
//	mlog("ZGame e_size : w_size : %d %d\n",e_size,w_size);

//	ZGetEffectManager()->Clear();
	m_WeaponManager.Clear();
#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->Clear();
#endif

#ifdef _BIRDSOUND

#else
	ZGetSoundEngine()->ClearAmbientSound();
#endif

	m_ObserverCommandList.Destroy();
	m_ReplayCommandList.Destroy();
	m_DelayedCommandList.Destroy();

	ZGetEffectManager()->Clear();
	ZGetScreenEffectManager()->Clear();

	ZGetWorldManager()->Clear();

	// Custom: Delete character cache
	ZGetCharacterManager()->Clear();

	char tmpbuf[128];
	_strtime( tmpbuf );

	mlog("game destroyed ( %s )\n",tmpbuf);
}

bool ZGame::CreateMyCharacter(MTD_CharInfo* pCharInfo/*, MTD_CharBuffInfo* pCharBuffInfo*/)
{
	if (!m_pMyCharacter) return false;

	m_pMyCharacter->Create(pCharInfo/*, pCharBuffInfo*/);
	m_pMyCharacter->SetVisible(true);

	mlog("Create character : Name=%s Level=%d \n", pCharInfo->szName, pCharInfo->nLevel);
	return true;
}

bool ZGame::CheckGameReady()
{
	if (GetReadyState() == ZGAME_READYSTATE_RUN) {
		return true;
	} else if (GetReadyState() == ZGAME_READYSTATE_INIT) {
		SetReadyState(ZGAME_READYSTATE_WAITSYNC);
		// �ð� ��ũ ��û
		ZPostRequestTimeSync(GetTickTime());
		return false;
	} else if (GetReadyState() == ZGAME_READYSTATE_WAITSYNC) {
		return false;
	}
	return false;
}

void ZGame::OnGameResponseTimeSync(unsigned int nLocalTimeStamp, unsigned int nGlobalTimeSync)
{
	ZGameTimer* pTimer = GetGameTimer();
	int nCurrentTick = pTimer->GetGlobalTick();
	int nDelay = (nCurrentTick - nLocalTimeStamp)/2;
	int nOffset = (int)nGlobalTimeSync - (int)nCurrentTick + nDelay;
	
	pTimer->SetGlobalOffset(nOffset);

	SetReadyState(ZGAME_READYSTATE_RUN);
}

void ZGame::Update(float fElapsed)
{
	if (CheckGameReady() == false) {
		OnCameraUpdate(fElapsed);
		return;
	}
	if (ZGetCamera()->m_bAutoAiming)
		AutoAiming();

	GetWorld()->Update(fElapsed);

	ZGetEffectManager()->Update(fElapsed);
	ZGetScreenEffectManager()->UpdateEffects();

	m_GameTimer.UpdateTick(timeGetTime());
	m_fTime.Set_CheckCrc(m_fTime.Ref() + fElapsed);
	m_fTime.ShiftHeapPos();
	m_bReplaying.ShiftHeapPos_CheckCrc();
	m_ObjectManager.Update(fElapsed);

	if (m_pMyCharacter && !m_bReplaying.Ref())
	{
		if (ZGetGame() && ZGetGame()->m_pMyCharacter && !ZGetGame()->IsReplay())
		{
			//if (ZGetGame()->CheckGameReady() == false) continue;forworker
			ZGetGame()->PostBasicInfo();
			if (ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
				ZGetGame()->PostHPAPInfo();
			else
				ZGetGame()->PostDuelTournamentHPAPInfo(); // �����ʸ�Ʈ ������ ó�����ش�.

			ZGetGame()->PostPeerPingInfo();
			ZGetGame()->PostSyncReport();
		}
	}

	CheckMyCharDead(fElapsed);
	if (m_bReserveObserver)
	{
		OnReserveObserver();
	}

	UpdateCombo();

#ifdef _RAINSYSTEM
	g_RainTownParticleSystem.Update(fElapsed);
#else
	g_SnowTownParticleSystem.Update(fElapsed);
#endif

#ifdef _WORLD_ITEM_
	ZGetWorldItemManager()->update();
#endif
	m_Match.Update(fElapsed);

	if (m_bReplaying.Ref())
		OnReplayRun();
	// ���÷��̶����� ������ ����?Eó���� �ؾ� ��?E?
	if (ZGetGameInterface()->GetCombatInterface()->GetObserverMode() || m_bReplaying.Ref())
		OnObserverRun();

	ProcessDelayedCommand();

	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE))
	{
		if (ZGetGame()->GetRolledDice() == 1)
			ZGetGameInterface()->GetCamera()->m_fDist = CAMERA_DIST_MAX;
		else
		{
			if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FPS))
			{
				ZGetGameInterface()->GetCamera()->m_fDist = 30.f;
			}
			else
				ZGetGameInterface()->GetCamera()->m_fDist = CAMERA_DEFAULT_DISTANCE;
		}
	}

	if ((ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE) && ZGetGame()->GetRolledDice() == 9))
	{
		if (m_pMyCharacter->GetDistToFloor() <= 3.0f) m_pMyCharacter->SetVelocity(m_pMyCharacter->GetVelocity().x, m_pMyCharacter->GetVelocity().y, 800.0f);
	}

	if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE) && GetRolledDice() == 11)
		m_bShowWireframe = true;
	else
		m_bShowWireframe = false;

#ifdef _QUEST
	{
		ZGetQuest()->OnGameUpdate(fElapsed);
	}
#endif

	//////////////////////////////////////////////////////////////////////////
	//	TODO : ��Ȳ�� ��?E������Ʈ �����൵ �� ���� ������ ��?E����?E
	//////////////////////////////////////////////////////////////////////////
	//g_ParticleSystem->Update(0.05);
	RGetParticleSystem()->Update(fElapsed);

	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Update();

	OnCameraUpdate(fElapsed);
	//	m_fTime->SetWarpingAdd(GetTickCount());

	m_WeaponManager.Update();
}


void ZGame::OnCameraUpdate(float Elapsed)
{
	if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
	{
		rvector dir = GetMapDesc()->GetWaitCamDir();
		rvector pos = GetMapDesc()->GetWaitCamPos();
		rvector up(0,0,1);

		RSetCamera(pos, pos+dir, up);
	}
	else
	{
		ZGetGameInterface()->GetCamera()->Update(Elapsed);
	}
}
//jintriple3 ����� �������� ��ŷ
void ZGame::CheckMyCharDeadByCriticalLine()
{
	MUID uidAttacker = MUID(0,0);
	bool bReturnValue = m_pMyCharacter->GetPosition().z >= DIE_CRITICAL_LINE;
	if (m_pMyCharacter->GetPosition().z >= DIE_CRITICAL_LINE)	//��?E���� ���� ��?E���E��?E.
		PROTECT_DEBUG_REGISTER(bReturnValue)
			return;

	// �������� ������?E��..-_-;
	//m_pMyCharacter->SetVelocity(rvector(0,0,0));
	uidAttacker = m_pMyCharacter->GetLastThrower();

	ZObject *pAttacker = ZGetObjectManager()->GetObject(uidAttacker);
	if(pAttacker==NULL || !CanAttack(pAttacker,m_pMyCharacter))
	{
		uidAttacker = ZGetMyUID();
		pAttacker = m_pMyCharacter;
	}

	m_pMyCharacter->OnDamaged(pAttacker,m_pMyCharacter->GetPosition(),ZD_FALLING,MWT_NONE,m_pMyCharacter->GetHP());
	// Custom: Made CMT_SYSTEM
	ZChatOutput( ZMsg(MSG_GAME_FALL_NARAK), ZChat::CMT_SYSTEM );

#ifdef _DEBUG
	if (ZIsLaunchDevelop() && ZGetQuest()->GetCheet(ZQUEST_CHEET_GOD) == true)
		ZGetGameInterface()->RespawnMyCharacter();
#endif
}
//jintriple3 ����� �������� ��ŷ
void ZGame::CheckMyCharDeadUnchecked()
{
	MUID uidAttacker = MUID(0,0);
//	if ((m_pMyCharacter->IsDie() == false) && (m_pMyCharacter->GetHP() <= 0))
	bool bCheck = (m_pMyCharacter->IsDie() == true) | (m_pMyCharacter->GetHP() > 0);
	if((m_pMyCharacter->IsDie() == true) || (m_pMyCharacter->GetHP() > 0))
		PROTECT_DEBUG_REGISTER(bCheck)
			return;


	//hp <=0 && m_pMyCharacter->IsDie() == false
	if (uidAttacker == MUID(0,0) && m_pMyCharacter->GetLastAttacker() != MUID(0,0)) 
		uidAttacker = m_pMyCharacter->GetLastAttacker();

	// ���������� �Ѿ�̡?E���� finish ���¿����� �޽�?E��?E��?�����Ѵ�
	if( GetMatch()->GetRoundState() == MMATCH_ROUNDSTATE_FINISH) 
	{
		// �״� ô�� �Ѵ�
		m_pMyCharacter->ActDead();
		m_pMyCharacter->Die();
		return;
	}

	ZPostDie(uidAttacker);		// �Ǿ��鿡�� ������ �޼�?E

	// ����Ʈ ���� ���� �޼����� �ٸ���.
	if (! ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) && ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		ZPostGameKill(uidAttacker);	// ������ ������ �޼�?E
	}
	else
	{
		ZPostQuestGameKill();
	}

	// �׾����� ��Ʋ���� ����?E������ �Ǿ�����?Eī��⮸� �ٽ� �������ش�.
	if(ZApplication::GetGameInterface()->IsLeaveBattleReserved() == true)
		ZApplication::GetGameInterface()->ReserveLeaveBattle();
}

void ZGame::CheckMyCharDead(float fElapsed)
{
	//jintriple3 ����� �������� ��
	bool bReturnValue = !m_pMyCharacter || m_pMyCharacter->IsDie();
	if(!m_pMyCharacter || m_pMyCharacter->IsDie()) 
		PROTECT_DEBUG_REGISTER(bReturnValue)
			return;
	
	CheckMyCharDeadByCriticalLine();
	CheckMyCharDeadUnchecked();
    
}

void ZGame::OnPreDraw()
{
	__BP(19,"ZGame::sub1");

	RSetProjection(g_fFOV, DEFAULT_NEAR_Z, g_fFarZ);

	bool bTrilinear=RIsTrilinear();
	RGetDevice()->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(0, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	RGetDevice()->SetSamplerState(1, D3DSAMP_MIPFILTER, bTrilinear ? D3DTEXF_LINEAR : D3DTEXF_NONE);

	if(m_bShowWireframe ) {
		RGetDevice()->SetRenderState( D3DRS_FILLMODE , D3DFILL_WIREFRAME );
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		GetWorld()->SetFog(false);
	}
	else {
		RGetDevice()->SetRenderState( D3DRS_FILLMODE ,  D3DFILL_SOLID );
		GetWorld()->SetFog(true);
	}

	GetWorld()->GetBsp()->SetWireframeMode(m_bShowWireframe);

	rmatrix initmat;
	D3DXMatrixIdentity(&initmat);
	RGetDevice()->SetTransform( D3DTS_WORLD, &initmat );
	RGetDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, false );
	RGetDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
	RGetDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
	RGetDevice()->SetRenderState(D3DRS_ZWRITEENABLE, true );
//not sure if right place
	RGetDevice()->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, TRUE);
	LPDIRECT3DDEVICE9 pd3dDevice=RGetDevice();
	pd3dDevice->SetTexture(0,NULL);
	pd3dDevice->SetTexture(1,NULL);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
	pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_COLOROP,   D3DTOP_DISABLE );
	pd3dDevice->SetTextureStageState( 1, D3DTSS_ALPHAOP,   D3DTOP_DISABLE );
	pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1 );

	pd3dDevice->SetRenderState(D3DRS_CULLMODE  ,D3DCULL_CW);
	pd3dDevice->SetRenderState(D3DRS_NORMALIZENORMALS , TRUE );

	if(m_pMyCharacter)
	{
		if( ZGetConfiguration()->GetVideo()->bDynamicLight )
		{
			rvector pos = m_pMyCharacter->GetPosition();
			RGetDynamicLightManager()->SetPosition( pos );
		}
	}

	__EP(19);
}

int g_debug_render_mode = 0;

extern MDrawContextR2* g_pDC;

int IsNan(double x)
{
	union { unsigned __int64 u; double f; } ieee754;
	ieee754.f = x;

	return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) + ((unsigned)ieee754.u != 0) > 0x7ff00000;
}

int IsInf(double x)
{
	union { unsigned __int64 u; double f; } ieee754;
	ieee754.f = x;

	return ((unsigned)(ieee754.u >> 32) & 0x7fffffff) == 0x7ff00000 && ((unsigned)ieee754.u == 0);
}


void ZGame::Draw()
{
////////�׽�Ʈ ?E��?E..���߿� ��?E?.//////////////////////////////
////////////////////////////////////////////////////////////////////for test
	/*
#ifdef _DEBUG
	if(GetAsyncKeyState( VK_UP ))	//Į�� ��
	{
		fShotTime += 0.1f;
		rvector vPos = g_pGame->m_pMyCharacter->GetPosition();
	
		ZPostShotMelee( vPos, nShot);
		nShot++;
		if(nShot >4)
			nShot = 1;
	}
	if(GetAsyncKeyState( VK_LEFT ))		//����?E��
	{
		fShotTime += 0.1f;
		ZPostSkill( ZC_SKILL_SPLASHSHOT, MMCIP_MELEE);

	}
	if(GetAsyncKeyState( VK_RIGHT ))	//����ź ��
	{
		rvector vPos = g_pGame->m_pMyCharacter->GetPosition();
		rvector vDir = g_pGame->m_pMyCharacter->GetDirection();
		fShotTime += 0.1f;

		vPos.z += 120.0f;

		int type = ZC_WEAPON_SP_GRENADE;	//weapon
		int sel_type = MMCIP_MELEE; //MMCIP_PRIMARY; //MMCIP_MELEE;			//parts
		ZPostShotSp( vPos, vDir, type, sel_type);
	}
	if(GetAsyncKeyState( VK_DOWN ))	    //���� ��
	{
		rvector vPos = g_pGame->m_pMyCharacter->GetPosition();
		rvector vDir = g_pGame->m_pMyCharacter->GetDirection();

		vPos.z += 120.0f;
		fShotTime += 0.1f;

		int type = ZC_WEAPON_SP_ROCKET; //ZC_WEAPON_SP_GRENADE;	//weaponww
	Ȥ�� ���߿� �� �� ?E��ؼ� ������ �������� �־�ح �ּ�ó���ؼ� ���ܵ�..
	*/
	
	
////////////////////////////////////////////////////////////////////
	__BP(20,"ZGame::Draw");

	RRESULT isOK=RIsReadyToRender();

	if(isOK==R_NOTREADY)
	{
		__EP(20);
		return ;
	}

	OnPreDraw();		// Device ���°� ������ ���⼭ ����

//	RRenderNodeMgr::m_bRenderBuffer = true;//------test code
	/*
	else if(isOK==R_RESTORED) {

	} // restore device dependent objects
	*/

	rmatrix _mat;
	RGetDevice()->GetTransform(D3DTS_WORLD, &_mat);

	__BP(21,"ZGame::Draw::DrawWorld");
	GetWorld()->Draw();
	__EP(21);

	// �ʿ� ����� Ư?E���� ��?E�Ʈ�� �׷��� �Ұ��� �ִ� ��?E

	ZMapDesc* pMapDesc = GetMapDesc();

	if( pMapDesc ) {
		pMapDesc->DrawMapDesc();
	}

	/*
	D3DLIGHT9 light;
	light.Type			= D3DLIGHT_POINT;
	light.Ambient.r		= 0.1f;
	light.Ambient.g		= 0.1f;
	light.Ambient.b		= 0.1f;
	light.Specular.r	= 1.0f;
	light.Specular.g	= 1.0f;
	light.Specular.b	= 1.0f;
	light.Attenuation0	= 0.05f; 
	light.Attenuation1	= 0.002f; 
	light.Attenuation2	= 0.0f; 

	light.Range			= 200.f;
	light.Position		= m_pMyCharacter->GetPosition();

	light.Diffuse.r		= .9f;
	light.Diffuse.g		= .1f;
	light.Diffuse.b		= .1f;

	GetWorld()->GetBsp()->DrawLight(&light);
	//*/
	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(22,"ZGame::Draw::DrawCharacters");

		m_ObjectManager.Draw();

		__EP(22);

		m_render_poly_cnt = RealSpace2::g_poly_render_cnt;

//		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
//		RGetDevice()->SetTexture(0,NULL);
//		RGetDevice()->SetTexture(1,NULL);
//		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	}

	RGetDevice()->SetTransform(D3DTS_WORLD, &_mat);//map_mat

	ZGetWorldItemManager()->Draw(0,GetWorld()->GetWaterHeight(),GetWorld()->IsWaterMap());

	m_WeaponManager.Render();//weapon

	__BP(50,"ZGame::DrawObjects");

	GetWorld()->GetBsp()->DrawObjects();

	__EP(50);

	__BP(17,"ZGame::Draw::Reflection");

	GetWorld()->GetWaters()->Render();

	__EP(17);

	if (m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PREPARE)
	{
		__BP(23,"ZGame::Draw::DrawWeapons and effects");
#ifndef _PUBLISH
//		TestCreateEffects();
#endif

		ZGetEffectManager()->Draw(timeGetTime());

		__EP(23);
	}

#ifdef _WORLD_ITEM_
	__BP(34,"ZGame::Draw::ZGetWorldItemManager");

		ZGetWorldItemManager()->Draw(1,GetWorld()->GetWaterHeight(),GetWorld()->IsWaterMap());

	__EP(34);
#endif


//	RRenderNodeMgr::m_bRenderBuffer = false;//------test code

	/*
	if(m_bCharacterLight)
	GetWorld()->GetBsp()->DrawLight(&light);
	*/
	//	m_render_poly_cnt = RealSpace2::g_poly_render_cnt;	


	__BP(35,"ZGame::Draw::RGetParticleSystem");

	RGetParticleSystem()->Draw();

	__EP(35);

	__BP(36,"ZGame::Draw::LenzFlare");

	if( RReadyLenzFlare())
	{
		RGetLenzFlare()->Render( RCameraPosition, GetWorld()->GetBsp());
	}

	RSetProjection(DEFAULT_FOV,DEFAULT_NEAR_Z, g_fFarZ);
	RSetFog(FALSE);

	__EP(36);

	__BP(37,"ZGame::Draw::FlashBangEffect");

	if( IsActivatedFlashBangEffect() )
	{
		ShowFlashBangEffect();
	}

	__BP(505, "ZGame::Draw::RenderStencilLight");
	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->Render();
	__EP(505);

	__EP(37);

	__BP(38,"ZGame::Draw::DrawGameMessage");

	m_Match.OnDrawGameMessage();

//	m_HelpScreen.DrawHelpScreen();

	__EP(38);

	__EP(20);

//	���������� �׷����� ȭ�鿡 ����?E���� �ٲ�ѵ��...? �⺻�� ��?Eüũ��?E����...
/*
	rvector line1 = rvector(200,163,168);
	rvector line2 = rvector(900,163,168);

	rmatrix m;

	rvector pos = line1;

	rvector dir = rvector(0,0,1);
	rvector up  = rvector(0,1,0);
	rvector max = rvector( 10, 10, 10);
	rvector min = rvector(-10,-10,-10);

	MakeWorldMatrix(&m,pos,dir,up);

	draw_box(&m,max,min,0xffff0000);

	pos = line2;

	MakeWorldMatrix(&m,pos,dir,up);

	draw_box(&m,max,min,0xffff0000);

	//////////////////////////////////////////////////////////////////////

	D3DXMatrixIdentity(&m);

	RGetDevice()->SetTransform( D3DTS_WORLD, &m );

	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	RDrawLine(line1,line2,0xffff0000);

	rvector new_line1;
	rvector new_line2;

	D3DXVec3TransformCoord(&new_line1,&line1,&RViewProjection);
	D3DXVec3TransformCoord(&new_line2,&line2,&RViewProjection);

	rvector tmin = rvector(-1.f,-1.f,0.f);
	rvector tmax = rvector( 1.f, 1.f,1.f);

	D3DXMatrixIdentity(&m);
//	MakeWorldMatrix(&m,rvector(0,0,0),dir,up);
	draw_box(&m,tmax*100,tmin*100,0xff00ffff);

	D3DXMatrixIdentity(&m);
	RGetDevice()->SetTransform( D3DTS_WORLD, &m );
	RGetDevice()->SetTexture(0,NULL);
	RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
	RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );
	RDrawLine(new_line1*100,new_line2*100,0xffffffff);

	/////////////////////////////////////////////////////////////////////

	int nPick = 0;

	if(isInViewFrustum(line1,line2, RGetViewFrustum() )) {
		nPick = 1;
	}
	else 
		nPick = 0;

	char szTemp[256];
	sprintf(szTemp, "line1 = %6.3f %6.3f %6.3f  line2 = %6.3f %6.3f %6.3f Pick %d", new_line1.x,new_line1.y,new_line1.z, new_line2.x,new_line2.y,new_line2.z,nPick);
	g_pDC->Text(100,200,szTemp);
*/

	/*//bsp pick �׽�Ʈ kimyhwan
	{
		float v1z = 0; float v2z = 100;
		FILE* fp = fopen("g:\\coord.txt", "rt");
		if (fp)
		{
			char sz[256];
			fgets(sz, 256, fp);
			sscanf(sz, "%f %f", &v1z, &v2z);
			fclose(fp);
		}

		rvector v1(0, 0, v1z);
		rvector v2(0, 0, v2z);
		DWORD color = 0xff0000ff;

		//const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;
		const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE;
		RBSPPICKINFO bpi;
		bool bBspPicked = GetWorld()->GetBsp()->PickTo(v1, v2, &bpi, dwPickPassFlag);
		if (bBspPicked)
			color = 0xffff0000;

		if (bBspPicked)
		{
			if (Magnitude(v2-v1)<Magnitude(bpi.PickPos-v1))
				color = 0xffff00ff;
		}

		rmatrix m;
		D3DXMatrixIdentity(&m);
		RGetDevice()->SetTransform( D3DTS_WORLD, &m );
		RGetDevice()->SetTexture(0,NULL);
		RGetDevice()->SetRenderState( D3DRS_LIGHTING, FALSE );
		RGetDevice()->SetFVF( D3DFVF_XYZ | D3DFVF_DIFFUSE );

		RDrawLine(v1, v2, color);
	}*/
}

void ZGame::DrawDebugInfo()
{
	char szTemp[256] = "";
	int n = 20;
	g_pDC->SetColor(MCOLOR(0xFFffffff));

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacterObject* pCharacter = (*itor).second;
		sprintf(szTemp, "Pos = %6.3f %6.3f %6.3f  Dir = %6.3f %6.3f %6.3f", pCharacter->GetPosition().x,
				pCharacter->GetPosition().y, pCharacter->GetPosition().z, 
				pCharacter->m_Direction.x, pCharacter->m_Direction.y, pCharacter->m_Direction.z);
		g_pDC->Text(20,n,szTemp);
		n += 15;

/*
		sprintf(szTemp, "state = %d , %d", (int)(pCharacter->GetState()), (int)(pCharacter->GetStateSub()));
		g_pDC->Text(20, n, szTemp);
		n+= 15;
*/

		RVisualMesh* pVMesh = pCharacter->m_pVMesh;

		AniFrameInfo* pAniLow = pVMesh->GetFrameInfo(ani_mode_lower);
		AniFrameInfo* pAniUp  = pVMesh->GetFrameInfo(ani_mode_upper);

		sprintf(szTemp,"%s frame down %d / %d ",pAniLow->m_pAniSet->GetName() , pAniLow->m_nFrame , pAniLow->m_pAniSet->GetMaxFrame());
		g_pDC->Text(20,n,szTemp);
		n+= 15;

		if( pAniUp->m_pAniSet ) 
		{
			sprintf(szTemp,"%s frame up %d / %d ",pAniUp->m_pAniSet->GetName(),pAniUp->m_nFrame,pAniUp->m_pAniSet->GetMaxFrame());
			g_pDC->Text(20,n,szTemp);
			n+= 15;
		}
	}

/*
	n = 300;
	for (MMatchPeerInfoList::iterator itor = ZGetGameClient()->GetPeers()->begin();
		 itor != ZGetGameClient()->GetPeers()->end(); ++itor)
	{
		MMatchPeerInfo* pPeerInfo = (*itor);
		sprintf(szTemp, "MUID(%d, %d) , IP = %s, port = %d", pPeerInfo->uidChar.High, 
			    pPeerInfo->uidChar.Low, pPeerInfo->szIP, pPeerInfo->nPort);
		g_pDC->Text(20,n,szTemp);
		n+=15;
	}
*/
}


void ZGame::Draw(MDrawContextR2 &dc)
{
	/*	// �н�?E�E���.. for debug
	char buffer[256];
	sprintf(buffer," state: %d , pathnode: %d",m_pMyCharacter->m_State,m_pMyCharacter->m_nPathNodeID);

	dc.SetColor(MCOLOR(0xFFffffff));
	dc.Text(0,20,buffer);
	//*/
}

void ZGame::ParseReservedWord(char* pszDest, const char* pszSrc)
{
	char szSrc[256];
	char szWord[256];

	strcpy(szSrc, pszSrc);

	char szOut[256];	ZeroMemory(szOut, 256);
	int nOutOffset = 0;

	char* pszNext = szSrc;
	while( *pszNext != NULL ) {
		pszNext = MStringCutter::GetOneArg(pszNext, szWord);

		if ( (*szWord == '$') && (stricmp(szWord, "$player")==0) ) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);
		} else if ( (*szWord == '$') && (stricmp(szWord, "$target")==0) ) {
			sprintf(szWord, "%d %d", m_pMyCharacter->GetUID().High, m_pMyCharacter->GetUID().Low);	// Target����� �� Target ���� �ٲ�?E
		}

		strcpy(szOut+nOutOffset, szWord);	nOutOffset += (int)strlen(szWord);
		if (*pszNext) { 
			strcpy(szOut+nOutOffset, " ");
			nOutOffset++;
		}
	}
	strcpy(pszDest, szOut);
}

#include "ZMessages.h"

extern bool g_bProfile;

// observer ��忡���� �����̸� ��ĥ �ʿ���� Ŀ�ǵ�?E
bool IsIgnoreObserverCommand(int nID)
{
	switch(nID) {
		case MC_PEER_PING :
		case MC_PEER_PONG :
		case MC_PEER_OPENED	:
		case MC_MATCH_GAME_RESPONSE_TIMESYNC :
			return false;
	}
	return true;
}

void ZGame::OnCommand_Observer(MCommand* pCommand)
{
	if(!IsIgnoreObserverCommand(pCommand->GetID()))
	{
		OnCommand_Immidiate(pCommand);
		return;
	}


	ZObserverCommandItem *pZCommand=new ZObserverCommandItem;
	pZCommand->pCommand=pCommand->Clone();
	pZCommand->fTime=GetTime();
	m_ObserverCommandList.push_back(pZCommand);
	
#ifdef _LOG_ENABLE_OBSERVER_COMMAND_BUSH_
	if(pCommand->GetID() != 10012 && pCommand->GetID() != 10014)
	{ // [ID:10012]:BasicInfo, [ID:10014]:HPAPInfo
		char buf[256];
		sprintf(buf,"[OBSERVER_COMMAND_BUSH:%d]: %s\n", pCommand->GetID(), pCommand->GetDescription());
		OutputDebugString(buf);
	}
#endif



	if(pCommand->GetID()==MC_PEER_BASICINFO)
	{
		/*
		ZCharacter *pChar=m_CharacterManager.Find(pCommand->GetSenderUID());
		if(pChar)
		{
			mlog("%s basic info : %3.3f \n",pChar->GetProperty()->szName,pZCommand->fTime);
		}
		*/
		OnPeerBasicInfo(pCommand,true,false);
	}
}

void ZGame::ProcessDelayedCommand()
{
	// Custom: Iterator increment fix
	for(ZObserverCommandList::iterator i = m_DelayedCommandList.begin(); i != m_DelayedCommandList.end();)
	{
		ZObserverCommandItem *pItem = *i;

		// ������ �ð��� ������?E�����Ѵ�
		if(GetTime() > pItem->fTime) 
		{
			OnCommand_Immidiate(pItem->pCommand);
			i = m_DelayedCommandList.erase(i);
			delete pItem->pCommand;
			delete pItem;
		}
		else
			++i;
	}
}

void ZGame::OnReplayRun()
{
	if(m_ReplayCommandList.size()==0 && m_bReplaying.Ref()) {
		m_bReplaying.Set_CheckCrc(false);
		EndReplay();
		return;
	}

	//	static float fLastTime = 0;
	while(m_ReplayCommandList.size())
	{
		ZObserverCommandItem *pItem=*m_ReplayCommandList.begin();


		//		_ASSERT(pItem->fTime>=fLastTime);
#ifdef _REPLAY_TEST_LOG
		 m_ReplayLogTime = pItem->fTime;
#else
		if(GetTime() < pItem->fTime)
			return;
#endif

		//		mlog("curtime = %d ( %3.3f ) time = %3.3f , id %d \n",timeGetTime(),GetTime(),pItem->fTime,pItem->pCommand->GetID());

		m_ReplayCommandList.erase(m_ReplayCommandList.begin());

		bool bSkip = false;
		switch( pItem->pCommand->GetID())
		{
		case MC_REQUEST_XTRAP_HASHVALUE:
		case MC_RESPONSE_XTRAP_HASHVALUE:
		case MC_REQUEST_XTRAP_SEEDKEY:
		case MC_RESPONSE_XTRAP_SEEDKEY:
		case MC_REQUEST_XTRAP_DETECTCRACK:
		case MC_REQUEST_GAMEGUARD_AUTH :
		case MC_RESPONSE_GAMEGUARD_AUTH :
		case MC_REQUEST_FIRST_GAMEGUARD_AUTH :
		case MC_RESPONSE_FIRST_GAMEGUARD_AUTH :
			bSkip = true;
		}

		if (bSkip == false)
			OnCommand_Observer(pItem->pCommand);

#ifdef _LOG_ENABLE_REPLAY_COMMAND_DELETE_
		if(pItem->pCommand->GetID() != 10012 && pItem->pCommand->GetID() != 10014)
		{ // [ID:10012]:BasicInfo, [ID:10014]:HPAPInfo
			char buf[256];
			sprintf(buf,"[REPLAY_COMMAND_DELETE:%d]: %s\n", pItem->pCommand->GetID(), pItem->pCommand->GetDescription());
			OutputDebugString(buf);
		}
#endif


#ifdef _REPLAY_TEST_LOG
		// ����?E����?EĿ��?E�ð��� m_ObserverCommandList�� ���̡�鼭 ������?E������ ���⼭ ó��
		switch (pItem->pCommand->GetID())
		{
		case MC_MATCH_STAGE_ENTERBATTLE:
			{	
				unsigned char nParam;
				pItem->pCommand->GetParameter(&nParam,		0, MPT_UCHAR);

				MCommandParameter* pParam = pItem->pCommand->GetParameter(1);
				if(pParam->GetType()!=MPT_BLOB) break;
				void* pBlob = pParam->GetPointer();

				MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);
				mlog("[%d EnterBattleRoom Time:%3.3f]\n", pPeerNode->uidChar.Low, pItem->fTime);
			}
			break;
		case MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT:
			{			
				MUID uidChar;
				pItem->pCommand->GetParameter(&uidChar, 0, MPT_UID);
				mlog("[%d LeaveBattleRoom Time:%3.3f]\n", uidChar.Low, m_ReplayLogTime);
			}
			break;
		}
#endif

		delete pItem->pCommand;
		delete pItem;
	}
}

void ZGame::OnObserverRun()
{
	while(m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem *pItem=*m_ObserverCommandList.begin();
		if(GetTime()-pItem->fTime < ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetDelay())
			return;

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if(pItem->pCommand->GetID()==MC_PEER_BASICINFO)
			OnPeerBasicInfo(pItem->pCommand,false,true);
		else
		{
			OnCommand_Immidiate(pItem->pCommand);

#ifdef _LOG_ENABLE_OBSERVER_COMMAND_DELETE_
			char buf[256];
			sprintf(buf,"[OBSERVER_COMMAND_DELETE:%d]: %s\n", pItem->pCommand->GetID(), pItem->pCommand->GetDescription());
			OutputDebugString(buf);
#endif
		}

		delete pItem->pCommand;
		delete pItem;
	}

#ifdef _REPLAY_TEST_LOG
	for(int i=0; i<16; ++i)
	{
		if(m_Replay_UseItem[i].uid.Low == 0)
			break;
		for(int j=0; j<5; ++j)
		{
			if(m_Replay_UseItem[i].Item[j].Itemid == 0)
				break;
			MMatchItemDesc* pItemDesc = MGetMatchItemDescMgr()->GetItemDesc(m_Replay_UseItem[i].Item[j].Itemid);

			mlog("[ uid:%d Item:%s(%d) UseCount:%d ]\n", m_Replay_UseItem[i].uid.Low, pItemDesc->m_pMItemName->Ref().m_szItemName, pItemDesc->m_nID, m_Replay_UseItem[i].Item[j].ItemUseCount);
		}
	}
	mlog("[Replay Playtime: %f]\n[End Replay]\n", m_ReplayLogTime);
#endif
}

void ZGame::FlushObserverCommands()
{
	while(m_ObserverCommandList.begin() != m_ObserverCommandList.end())
	{
		ZObserverCommandItem *pItem=*m_ObserverCommandList.begin();

		m_ObserverCommandList.erase(m_ObserverCommandList.begin());

		if(pItem->pCommand->GetID()!=MC_PEER_BASICINFO)
			OnCommand_Immidiate(pItem->pCommand);

		delete pItem->pCommand;
		delete pItem;
	}
}

bool ZGame::OnCommand(MCommand* pCommand)
{
	if(m_bRecording)
	{
		ZObserverCommandItem *pItem = new ZObserverCommandItem;
		pItem->fTime = m_fTime.Ref();
		pItem->pCommand = pCommand->Clone();

		m_ReplayCommandList.push_back(pItem);

#ifdef _LOG_ENABLE_RELAY_COMMAND_BUSH_
		if(pCommand->GetID() != 10012 && pCommand->GetID() != 10014)
		{ // [ID:10012]:BasicInfo, [ID:10014]:HPAPInfo
			char buf[256];
			sprintf(buf,"[RELAY_COMMAND_BUSH:%d]: %s\n", pCommand->GetID(), pCommand->GetDescription());
			OutputDebugString(buf);
		}
#endif
	}

	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		OnCommand_Observer(pCommand);
		return true;
	}

	if(FilterDelayedCommand(pCommand))
	{
		return true;
	}

	return OnCommand_Immidiate(pCommand);
}

// ��?E�÷�

bool GetUserGradeIDColor(MMatchUserGradeID gid,MCOLOR& UserNameColor,char* sp_name)
{
	// Custom: Different UGrade colours
	if(gid == MMUG_DEVELOPER) 
	{
		UserNameColor = MCOLOR(0, 255, 127);
		//UserNameColor = MCOLOR(255,128, 64); 
		// Custom: Unmask names
		//if(sp_name) { 
		//	strcpy(sp_name,ZMsg(MSG_WORD_DEVELOPER));
		//}
		return true; 
	}
	else if(gid == MMUG_ADMIN)
	{
		UserNameColor = MCOLOR(255, 0, 0);
		//UserNameColor = MCOLOR(255,128, 64); 
		// Custom: Unmask names
		//if(sp_name) { 
		//	strcpy(sp_name,ZMsg(MSG_WORD_ADMIN));
		//}
		return true; 
	}
	else if(gid == MMUG_EVENTMASTER)
	{
		UserNameColor = MCOLOR(255, 128, 64); //MCOLOR(91, 91, 179);
		return true;
	}
	else if(gid == MMUG_EVENTTEAM)
	{
		UserNameColor = MCOLOR(0xFF800080);
		return true;
	}

	else if (gid == MMUG_MANAGER)
	{
		UserNameColor = MCOLOR(0xFFFF6F6F);
		return true;
	}
#ifdef _VIPGRADES
	else if (gid == MMUG_VIP1)
	{
		UserNameColor = MCOLOR(0xFF00FFF6); //Cyan
		//UserNameColor = MCOLOR(R, G, B); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_VIP2)
	{
		UserNameColor = MCOLOR(0xFF0072FF); //Blue
		//UserNameColor = MCOLOR(R, G, B); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_VIP3)
	{
		UserNameColor = MCOLOR(0xFFFFD800); //Yellow
		//UserNameColor = MCOLOR(R, G, B); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_VIP4)
	{
		UserNameColor = MCOLOR(0xFFFF54f9); //Yellow
		//UserNameColor = MCOLOR(R, G, B); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_VIP5)
	{
		UserNameColor = MCOLOR(0xFF404040); //Yellow
		//UserNameColor = MCOLOR(R, G, B); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_VIP6)
	{
		UserNameColor = MCOLOR(0xFFBD00FC); //Yellow
		//UserNameColor = MCOLOR(R, G, B); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_VIP7)
	{
		UserNameColor = MCOLOR(0xFFFF7300); //Super Donor
		//UserNameColor = MCOLOR(R, G, B); //RED,GREEN,BLUE
		return true;
	}
#endif

#ifdef _EVENTGRD
	else if (gid == MMUG_EVENT1)
	{
		UserNameColor = MCOLOR(0xFFb5006d); //Fiucha
		//UserNameColor = MCOLOR(181, 0, 109); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_EVENT2)
	{
		UserNameColor = MCOLOR(0xFF009db5); //Agua
		//UserNameColor = MCOLOR(0, 157, 181); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_EVENT3)
	{
		UserNameColor = MCOLOR(0xFFb57600); //Kake
		//UserNameColor = MCOLOR(181, 118, 0); //RED,GREEN,BLUE
		return true;
	}
	else if (gid == MMUG_EVENT4)
	{
	UserNameColor = MCOLOR(0xFF0d0d0d); //Black
	//UserNameColor = MCOLOR(181, 118, 0); //RED,GREEN,BLUE
	return true;
	}
#endif


	return false;
}

bool ZGame::GetUserNameColor(MUID uid,MCOLOR& UserNameColor,char* sp_name)
{
	MMatchUserGradeID gid = MMUG_FREE;

	if(m_pMyCharacter->GetUID()==uid) 
	{
		if(ZGetMyInfo()) {
			gid = ZGetMyInfo()->GetUGradeID();
//			gid = MMUG_DEVELOPER;//test

		} 
		else {
			mlog("ZGame::GetUserNameColor ZGetMyInfo==NULL \n");
		}
	}
	else 
	{
		MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);
		if(pPeer) {
			 gid = pPeer->CharInfo.nUGradeID;
		}		
	}

	return GetUserGradeIDColor( gid, UserNameColor, sp_name );
}

void ZTranslateCommand(MCommand* pCmd, string& strLog)
{
	char szBuf[256]="";

	// �ð�
	unsigned long nGlobalClock = ZGetGame()->GetTickTime();
	itoa(nGlobalClock, szBuf, 10);
	strLog = szBuf;
	strLog += ": ";

	// Command
	strLog += pCmd->m_pCommandDesc->GetName();

	// PlayerName
	string strPlayerName;
	MUID uid = pCmd->GetSenderUID();
	ZCharacter* pChar = (ZCharacter*) ZGetCharacterManager()->Find(uid);
	if (pChar)
		strPlayerName = pChar->GetProperty()->GetName();
	else
		strPlayerName = "Unknown";

	strLog += " (";
	strLog += strPlayerName;
	strLog += ") ";

	// Params
	string strParams;
	for(int i=0; i<pCmd->GetParameterCount(); i++){
		char szParam[256]="";
		pCmd->GetParameter(i)->GetString(szParam);
		strParams += szParam;
		if (i<pCmd->GetParameterCount()-1)
			strParams += ", ";
	}
	strLog += strParams;
}

void ZLogCommand(MCommand* pCmd)
{
	if (pCmd->GetID() == MC_AGENT_TUNNELING_UDP) {
		return;
	}

	string strCmd;
	ZTranslateCommand(pCmd, strCmd);

	OutputDebugString(strCmd.c_str());
	OutputDebugString("\n");
}
#ifdef _ZPEERANTIHACK
void ZGame::OnPeerAntiLead(MCommand* PCommand)
{// hahahahahaha
	if (m_pMyCharacter->GetUID() != PCommand->GetReceiverUID()) return;
	int nID = 0;
	if (!PCommand->GetParameter(&nID, 1, MPT_INT)) return;
	RecvArray::iterator it = RecievedShots.find(nID);
	if (it != RecievedShots.end())
	{
		ZPOSTHITRESPONSE(MCommandParameterInt(nID), PCommand->GetSenderUID());
		it->second = timeGetTime();
		return;
	}
	else RecievedShots.insert(RecvArray::value_type(nID, timeGetTime()));
	ZPOSTHITRESPONSE(MCommandParameterInt(nID), PCommand->GetSenderUID());
	ZCharacter* pCharacter = ZGetCharacterManager()->Find(PCommand->GetSenderUID());
	if (!pCharacter || pCharacter && pCharacter->m_dwStatusBitPackingValue.Ref().m_bLostConEffect == true || !pCharacter->GetProperty()) return;
	MCommandParameter* pParam = PCommand->GetParameter(0);
	if (pParam->GetType() != MPT_BLOB) return;
	void* pBlob = pParam->GetPointer();
	char victim[MATCHOBJECT_NAME_LENGTH], attacker[MATCHOBJECT_NAME_LENGTH];
	sprintf(victim, m_pMyCharacter->GetProperty()->GetName());
	sprintf(attacker, pCharacter->GetProperty()->GetName());
	int nVic = strlen(victim), nAttc = strlen(attacker);
	MTD_SHOTINFO* Info = (MTD_SHOTINFO*)MGetBlobArrayElement(pBlob, 0);
	if (!Info) return;
	Info->Type = Info->Type ^ (char)((nVic >= 3) ? victim[2] : 'p');
	ZItem* pItem = pCharacter->GetItems()->GetItem((MMatchCharItemParts)(Info->Type));
	if (!pItem || !pItem->GetDesc()) return;
	if (m_pMyCharacter && m_pMyCharacter->GetUID() == PCommand->GetReceiverUID())
	{
		MMatchItemDesc* MyDesc = MGetMatchItemDescMgr()->GetItemDesc(pItem->GetDesc()->m_nID);
		if (!MyDesc || pItem->GetDesc() != MyDesc) return;
		int nTotalDmg = 0;
		//for (int i = 0; i < nSize; ++i)
		//{
		if (m_pMyCharacter && ZGetGameClient()->GetPlayerUID() != PCommand->GetSenderUID() && (Info->MYID.Low ^ 'j') == ZGetGameClient()->GetPlayerUID().Low && (!GetMatch()->IsTeamPlay() || ((GetMatch()->IsTeamPlay() && GetMatch()->GetTeamKillEnabled()) || !m_pMyCharacter->IsTeam(pCharacter))))
		{
			int ShotLimit = 1;
			int nSize = Info->Size ^ (char)((nAttc >= 2) ? attacker[1] : 'p');
			if (MyDesc->m_nWeaponType.Ref() == MWT_SAWED_SHOTGUN || MyDesc->m_nWeaponType.Ref() == MWT_SHOTGUN) ShotLimit = 12;
			if (nSize > ShotLimit) return;
			Info->Parts = Info->Parts ^ (char)((nVic >= 4) ? victim[3] : 'p');
			float fRatio = pItem->GetPiercingRatio(MyDesc->m_nWeaponType.Ref(), (RMeshPartsType)(Info->Parts));
			ZDAMAGETYPE dt = ((RMeshPartsType)(Info->Parts) == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
			/*
			if(m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bLand == true && ZGetGame()->CheckWall((ZCharacter*)m_pMyCharacter, pCharacter))
			{
				if(RandomNumber(0, 2) == 1)
					Dmg = Dmg/2;
				else continue;
			}*/
			float Dmg = CalcActualDamage(pCharacter, m_pMyCharacter, (float)MyDesc->m_nDamage.Ref());
			Dmg = Dmg * nSize;
			nTotalDmg = Dmg;
			m_pMyCharacter->OnDamaged(pCharacter, rvector(Info->X, Info->Y, Info->Z), dt, MyDesc->m_nWeaponType.Ref(), Dmg, fRatio);
			static float MyLastHpWas = m_pMyCharacter->GetHP(), MyLastApWas = m_pMyCharacter->GetAP();
			static int Count = 0;
			if (m_pMyCharacter->IsDie() == false && (m_pMyCharacter->GetHP() == MyLastHpWas || MyLastApWas != 0 && m_pMyCharacter->GetAP() == MyLastApWas))
				if (Count++ >= 25)
				{
					char key = 44;
					char text[] = { "{DMX_xDE_" };
					for (int i = 0; i < strlen(text); i++)
						text[i] = text[i] ^ key;
					mlog("%s\n", text);
					ZPostDisconnect();
				}
		}
		//}
#ifdef _ZPEERANTIHACK
		ZPeerAntiHackAdd(PCommand->GetSenderUID(), Shot, MyDesc->m_nDelay.Ref(), (MMatchCharItemParts)Info->Type, MyDesc->m_nWeaponType.Ref());
#endif	
		ZPOSTDMGTAKEN(MCommandParameterInt(nTotalDmg), MCommandParameterInt(0), PCommand->GetSenderUID());
	}
}
void ZGame::ZPeerAntiHackAdd(MUID Attacker, ZAntiHackArrayInfo Type, int WepDelay, MMatchCharItemParts EquipType, MMatchWeaponType WeaponType)
{
	if (IsReplay() || ZGetGameInterface()->GetCombatInterface()->GetObserverMode() || m_pMyCharacter->GetUID() == Attacker) return;
	if (ZPeerAntiHackIsBlocked(Attacker)) return;
	ZPeerAntiHackDef::iterator i = ZPeerAntiHack.find(Attacker);
	if (i != ZPeerAntiHack.end())
	{
		int EType = 0;
		EType = (EquipType == MMCIP_PRIMARY ? 0 : 1);
		ZPeerAntiHackData* Data = i->second;
		if (Data)
		{
			unsigned long int nTime = timeGetTime();
			char type[100];
			bool Warning = false;
			switch (Type)
			{
			case Shot:
			{
				int nAddedDelay = 100;
				if (WeaponType == WeaponType == MWT_SMGx2) return; // temp fix
				if (WeaponType == MWT_MACHINEGUN || WeaponType == MWT_RIFLE || WeaponType == MWT_SMG) nAddedDelay = 70;
				int LastDelay = Data->WepDelay[EType][Type];
				if (LastDelay != 0 && ((nTime - Data->LastTime[EType][Type]) + nAddedDelay) <= LastDelay)
				{
					sprintf(type, "Shot Delay (%d/%d)", nTime - Data->LastTime[EType][Type], LastDelay);
					Warning = true;
				}
				else
				{
					Data->WepDelay[EType][Type] = WepDelay;
					Data->LastTime[EType][Type] = nTime;
				}
			}
			break;
			case Melee:
			{
				int nAddedDelay = 300;
				int LastDelay = Data->WepDelay[EType][Type];
				if (((Data->LastTime[EType][Type] - nTime) + nAddedDelay) <= LastDelay)
				{
					sprintf(type, "Melee Delay (%d/%d)", nTime - Data->LastTime[EType][Type], LastDelay);
					Warning = true;
				}
				Data->LastTime[EType][Type] = nTime;
			}
			break;
			case Flip:
			{
				sprintf(type, "Flip");
			}
			break;
			case Massive:
			{
				sprintf(type, "Massive");
			}
			break;
			}
			if (Warning)
			{
				Data->Warned[Type]++;
				if (Data->Warned[Type] >= BLOCKEDAMOUNT)
				{
					char lol[100];
					ZCharacter* pCharacter = ZGetCharacterManager()->Find(Attacker);
					if (pCharacter)
					{
						sprintf(lol, "[PEER ANTIHACK] %s Blocked For %s Hack.", pCharacter->GetCharInfo()->szName, type);
						ZChatOutput(MCOLOR(41, 199, 24), lol);
						ZPeerAntiHackBlocked.push_back(Attacker);
					}
				}
				else if (Data->Warned[Type] >= 1)
				{
					char lol[100];
					ZCharacter* pCharacter = ZGetCharacterManager()->Find(Attacker);
					if (pCharacter)
					{
						sprintf(lol, "[PEER ANTIHACK] %s (%s Hack) Warning %d/%d", pCharacter->GetCharInfo()->szName, type, Data->Warned[Type], BLOCKEDAMOUNT);
						ZChatOutput(MCOLOR(41, 199, 24), lol);
					}
				}
			}
		}
	}
	else ZPeerAntiHack.insert(ZPeerAntiHackDef::value_type(Attacker, new ZPeerAntiHackData()));
}
#endif
bool ZGame::OnCommand_Immidiate(MCommand* pCommand)
{
	/* rpg ��?E?��?E �ʿ���µ��ϴ�.
	string test;
	if(TranslateMessage(pCommand,&test))
		ZChatOutput(test.c_str());
	*/

#ifdef _DEBUG
//	ZLogCommand(pCommand);
#endif

	// ��?EZGameAction ���� ó���Ǵ� Ŀ�ǵ�?Eó���Ѵ�.
	if(m_pGameAction->OnCommand(pCommand))
	{
		return true;
	}


	if (OnRuleCommand(pCommand))
	{
		return true;
	}

	switch (pCommand->GetID())
	{
	case MC_MATCH_STAGE_ENTERBATTLE:
		{	
			unsigned char nParam;
			pCommand->GetParameter(&nParam,		0, MPT_UCHAR);

			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();

			MTD_PeerListNode* pPeerNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, 0);
			OnStageEnterBattle(MCmdEnterBattleParam(nParam), pPeerNode);
		}
		break;

	case MC_MATCH_RECEIVE_VOICE_CHAT:
	{
		MUID uid;
		if (!pCommand->GetParameter(&uid, 0, MPT_UID))
			break;

		auto it = m_CharacterManager.find(uid);

		if (it == m_CharacterManager.end())
			break;

		auto Char = it->second;

		MCommandParameter* pParam = pCommand->GetParameter(1);
		if (pParam->GetType() != MPT_BLOB) break;
		void* pBlob = pParam->GetPointer();

		GetRGMain().OnReceiveVoiceChat(Char, (unsigned char*)pBlob, pParam->GetSize() - sizeof(int));
	}
	break;

#ifdef _KILLSTREAK
	case MC_PEER_KILLSTREAK:
	{
		int nKillStreakCount;
		char szName[MAX_CHARNAME_LENGTH];
		char szMsg[512];
		char szVictim[MAX_CHARNAME_LENGTH];

		const MCOLOR StreakColor = MCOLOR(255, 87, 51);

		pCommand->GetParameter(szName, 0, MPT_STR, MAX_CHARNAME_LENGTH);
		pCommand->GetParameter(&nKillStreakCount, 1, MPT_INT);
		pCommand->GetParameter(szVictim, 2, MPT_STR, MAX_CHARNAME_LENGTH);

		if (m_pMyCharacter->GetKillStreaks() == 0)
		{
			MCOLOR StreakColor = MCOLOR(255, 0, 154);
			sprintf(szMsg, "%s has stopped %s's killing spree.", szName, szVictim);
			ZApplication::GetSoundEngine()->PlaySound("shutdown");
		}
		if (nKillStreakCount == 2)
		{
			sprintf(szMsg, "%s has slain %s for a double kill!(%i kills)", szName, szVictim, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("doublekill");
		}
		else if (nKillStreakCount == 3)
		{
			sprintf(szMsg, "%s has slain %s for a triple kill!(%i kills)", szName, szVictim, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("triplekill");
		}
		else if (nKillStreakCount == 4)
		{
			sprintf(szMsg, "%s has slain %s for a quadra kill!(%i kills)", szName, szVictim, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("quadrakill");
		}
		else if (nKillStreakCount == 5)
		{
			sprintf(szMsg, "%s has slain %s for a penta kill!(%i kills)", szName, szVictim, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("pentakill");
		}
		else if (nKillStreakCount == 6)
		{
			sprintf(szMsg, "%s is on a killing spree!(%i kills)", szName, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("killingspree");
		}
		else if (nKillStreakCount == 8)
		{
			sprintf(szMsg, "%s is on a rampage!(%i kills)", szName, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("rampage");
		}
		else if (nKillStreakCount == 10)
		{
			sprintf(szMsg, "%s is godlike!(%i kills)", szName, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("godlike");
		}
		else if (nKillStreakCount == 12)
		{
			sprintf(szMsg, "%s is dominating!(%i kills)", szName, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("dominating");
		}
		else if (nKillStreakCount > 12 && (nKillStreakCount % 2) == 0)
		{
			sprintf(szMsg, "%s is the super legendary annihilator with a few streaks of %i kills", szName, nKillStreakCount);
			ZApplication::GetSoundEngine()->PlaySound("legendary");
		}
		else
		{
			sprintf(szMsg, "");
		}
		if (szMsg != "")
			ZChatOutput(StreakColor, szMsg);
	}
	break;
#endif
	case MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT:
		{			
			MUID uidChar;
			bool bIsRelayMap;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&bIsRelayMap, 1, MPT_BOOL);

			OnStageLeaveBattle(uidChar, bIsRelayMap);//, uidStage);
		}
		break;
	case MC_MATCH_RESPONSE_PEERLIST:
		{
			MUID uidStage;
			pCommand->GetParameter(&uidStage, 0, MPT_UID);
			MCommandParameter* pParam = pCommand->GetParameter(1);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();
			int nCount = MGetBlobArrayCount(pBlob);
			OnPeerList(uidStage, pBlob, nCount);
		}
		break;
	case MC_MATCH_GAME_ROUNDSTATE:
		{
			MUID uidStage;
			int nRoundState, nRound, nArg;
			DWORD dwElapsed;

			pCommand->GetParameter(&uidStage, 0, MPT_UID);
			pCommand->GetParameter(&nRound, 1, MPT_INT);
			pCommand->GetParameter(&nRoundState, 2, MPT_INT);
			pCommand->GetParameter(&nArg, 3, MPT_INT);
			pCommand->GetParameter(&dwElapsed, 4, MPT_UINT);

			OnGameRoundState(uidStage, nRound, nRoundState, nArg);

			ZGetGame()->GetMatch()->SetRoundStartTime(dwElapsed);
		}
		break;
	case MC_MATCH_GAME_RESPONSE_TIMESYNC: 
		{
			unsigned int nLocalTS, nGlobalTS;
			pCommand->GetParameter(&nLocalTS, 0, MPT_UINT);
			pCommand->GetParameter(&nGlobalTS, 1, MPT_UINT);

			OnGameResponseTimeSync(nLocalTS, nGlobalTS);
		}
		break;
	case MC_MATCH_RESPONSE_SUICIDE:
		{
			int nResult;
			MUID	uidChar;
			pCommand->GetParameter(&nResult, 0, MPT_INT);
			pCommand->GetParameter(&uidChar, 1, MPT_UID);

			if (nResult == MOK)
			{
				OnPeerDie(uidChar, uidChar);
				CancelSuicide();
			}
		}
		break;

	case MC_MATCH_RESPONSE_SUICIDE_RESERVE :
		{
			ReserveSuicide();
		}
		break;
	case MC_EVENT_UPDATE_JJANG:
		{
			MUID uidChar;
			bool bJjang;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&bJjang, 1, MPT_BOOL);

			OnEventUpdateJjang(uidChar, bJjang);
		}
		break;

	case MC_MATCH_GAME_CHAT:
		{
			MUID uidPlayer, uidStage;
			char szMsg[CHAT_STRING_LEN];
			memset(szMsg, 0, sizeof(szMsg));
			int nTeam;

			pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
			pCommand->GetParameter(&uidStage, 1, MPT_UID);
			pCommand->GetParameter(&szMsg, 2, MPT_STR);
			pCommand->GetParameter(&nTeam, 3, MPT_INT);
			
			if (uidStage != ZGetGameClient()->GetStageUID()) break;

			CheckMsgAboutChat(szMsg);
			MCOLOR ChatColor = MCOLOR(0xFFD0D0D0);
			const MCOLOR TeamChatColor = MCOLOR(109, 207, 246);
			MCOLOR UserNameColor = MCOLOR(190, 190, 0);
			ZCharacter *pChar = (ZCharacter*)ZGetCharacterManager()->Find(uidPlayer);

			char sp_name[256];
			bool bSpUser = GetUserNameColor(uidPlayer, UserNameColor, sp_name);

			if (pChar)
			{
				if ((nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
				{
					if (!ZGetGameClient()->GetRejectNormalChat() || (strcmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
					{
						// Custom: Ignore List check (Not self)
						if (ZGetGameClient()->IsUserIgnored(pChar->GetUserName()) && stricmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()))
							break;

						ZGetSoundEngine()->PlaySound("if_error");
						char szTemp[sizeof(szMsg) + 64];

						// Custom: Check alive status
						if (!pChar->IsVisible() || pChar->IsDie())
							sprintf(szTemp, "(DEAD)%s : %s", pChar->GetProperty()->GetName(), szMsg);
						else
							sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(), szMsg);

						if (bSpUser) {
							ZChatOutput(UserNameColor, szTemp);
						}
						else {
							ZChatOutput(ChatColor, szTemp);
						}
					}
				}
				else if (nTeam > 1)
				{
					if ((!ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->IsPWChannel() && !ZGetGameClient()->GetRejectTeamChat()) ||
						((ZGetGameClient()->IsCWChannel() || ZGetGameClient()->IsPWChannel()) && !ZGetGameClient()->GetRejectClanChat()) ||
						(strcmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
					{
						// Custom: Ignore List check (Not self)
						if (ZGetGameClient()->IsUserIgnored(pChar->GetUserName()) && stricmp(pChar->GetUserName(), ZGetMyInfo()->GetCharName()))
							break;

						ZGetSoundEngine()->PlaySound("if_error");
						char szTemp[256];

						// Custom: Check alive status
						if (!pChar->IsVisible() || pChar->IsDie()) {
							if (ZGetGame()->m_pMyCharacter->IsAdminHide()){
								sprintf(szTemp, "(DEAD)(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(), szMsg);
							}
							else {
								sprintf(szTemp, "(DEAD)(Team)%s : %s", pChar->GetProperty()->GetName(), szMsg);
							}
							
						} 
						else {
							if (ZGetGame()->m_pMyCharacter->IsAdminHide()) {
								sprintf(szTemp, "(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(), szMsg);
							}
							else {
								sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(), szMsg);
							}
							
						}

						if (bSpUser) {
							ZChatOutput(UserNameColor, szTemp);
						}
						else {
							ZChatOutput(TeamChatColor, szTemp);
						}
					}
				}
			}
		}
		break;
	case MC_PEER_CHAT:
		{
			// Has been moved to MC_MATCH_GAME_CHAT
			break;
			/*int nTeam = MMT_ALL;
			char szMsg[CHAT_STRING_LEN];
			memset(szMsg, 0, sizeof(szMsg));

			pCommand->GetParameter(&nTeam, 0, MPT_INT);
			pCommand->GetParameter(szMsg, 1, MPT_STR, CHAT_STRING_LEN );
			//jintriple3 �� �ٲ� ���� ���͸� �ϴ� �κ�..
			CheckMsgAboutChat(szMsg);

			MCOLOR ChatColor = MCOLOR(0xFFD0D0D0);
			const MCOLOR TeamChatColor = MCOLOR(109,207,246);

			MUID uid=pCommand->GetSenderUID();
			ZCharacter *pChar = (ZCharacter*) ZGetCharacterManager()->Find(uid);

			MCOLOR UserNameColor = MCOLOR(190,190,0);

			char sp_name[256];
			bool bSpUser = GetUserNameColor(uid,UserNameColor,sp_name);

			if(pChar) 
			{
				int nMyTeam = ZGetGame()->m_pMyCharacter->GetTeamID();

				// �Ϲ� ä�� �� �϶�...
				if ( (nTeam == MMT_ALL) || (nTeam == MMT_SPECTATOR))
				{
					if ( !ZGetGameClient()->GetRejectNormalChat() || ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
					{
						// Custom: Ignore List check (Not self)
						if( ZGetGameClient()->IsUserIgnored( pChar->GetUserName() ) && stricmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName() ) )
							break;

						ZGetSoundEngine()->PlaySound("if_error");
						char szTemp[sizeof(szMsg)+64];

						// Custom: Disable duel chat block
						//if ( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)	// ��?E��ġ�϶�
						//{
							//if ( !m_pMyCharacter->IsDie() && pChar->IsDie())
							//{
								//ChatColor = MCOLOR(0xFF808080);
								//strcpy( szMsg, "^0. . . . .");
							//}
						//}

						// Custom: Check alive status
						if( !pChar->IsVisible() || pChar->IsDie() )
							sprintf(szTemp, "(DEAD)%s : %s", pChar->GetProperty()->GetName(),szMsg);
						else
							sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(),szMsg);

						if(bSpUser) {
							// Custom: Unmask names
							//sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(),szMsg);
							//sprintf(szTemp, "%s : %s", sp_name,szMsg);
							ZChatOutput(UserNameColor, szTemp);
						}
						else {
							//sprintf(szTemp, "%s : %s", pChar->GetProperty()->GetName(),szMsg);
							ZChatOutput(ChatColor, szTemp);
						}
					}
				}

				// �� ä�� �� �϶�...
				else if (nTeam == nMyTeam)
				{
					if ( (!ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectTeamChat()) ||
						 ( ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectClanChat()) ||
						 ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
					{
						// Custom: Ignore List check (Not self)
						if( ZGetGameClient()->IsUserIgnored( pChar->GetUserName() ) && stricmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName() ) )
							break;

						ZGetSoundEngine()->PlaySound("if_error");
						char szTemp[256];

						// Custom: Check alive status
						if( !pChar->IsVisible() || pChar->IsDie() )
							sprintf(szTemp, "(DEAD)(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);
						else
							sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);

						if(bSpUser) {
							// Custom: Unmask names
							//sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);
							//sprintf(szTemp, "(Team)%s : %s", sp_name,szMsg);
							ZChatOutput(UserNameColor, szTemp);
						}
						else {
							//sprintf(szTemp, "(Team)%s : %s", pChar->GetProperty()->GetName(),szMsg);
							ZChatOutput(TeamChatColor, szTemp);
						}
					}
				}

				// Custom: Admin Hide show all team chats
				if (ZGetGame()->m_pMyCharacter->IsAdminHide() && nTeam != MMT_ALL && nTeam != MMT_SPECTATOR)
				{
					if ( (!ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectTeamChat()) ||
						 ( ZGetGameClient()->IsCWChannel() && !ZGetGameClient()->GetRejectClanChat()) ||
						 ( strcmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName()) == 0))
					{
						// Custom: Ignore List check (Not self)
						if( ZGetGameClient()->IsUserIgnored( pChar->GetUserName() ) && stricmp( pChar->GetUserName(), ZGetMyInfo()->GetCharName() ) )
							break;

						ZGetSoundEngine()->PlaySound("if_error");
						char szTemp[256];

						// Custom: Check alive status
						if( !pChar->IsVisible() || pChar->IsDie() )
							sprintf(szTemp, "(DEAD)(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(),szMsg);
						else
							sprintf(szTemp, "(%s Team)%s : %s", nTeam == MMT_RED ? "Red" : "Blue", pChar->GetProperty()->GetName(),szMsg);

						if(bSpUser)
							ZChatOutput(UserNameColor, szTemp);
						else
							ZChatOutput(TeamChatColor, szTemp);
					}
				}
			}*/
		}
		break;

	case MC_PEER_CHAT_ICON:
	{
		bool bShow = false;
		pCommand->GetParameter(&bShow, 0, MPT_BOOL);

		MUID uid = pCommand->GetSenderUID();
		ZCharacter* pChar = (ZCharacter*)ZGetCharacterManager()->Find(uid);
		if (pChar)
		{
			//jintriple3 ��Ʈ ��ŷ �޸� ���Ͻ�...
			ZCharaterStatusBitPacking& uStatus = pChar->m_dwStatusBitPackingValue.Ref();
			if (bShow)
			{
				if (!uStatus.m_bChatEffect)
				{
					uStatus.m_bChatEffect = true;
					ZGetEffectManager()->AddChatIcon(pChar);
				}
			}
			else
				uStatus.m_bChatEffect = false;
		}
	}break;

		/*
		case MC_PEER_MOVE:
		{

		rvector pos, dir, velocity;
		pCommand->GetParameter(&pos, 0, MPT_POS);
		pCommand->GetParameter(&dir, 1, MPT_VECTOR);
		pCommand->GetParameter(&velocity, 2, MPT_VECTOR);
		int upper, lower;
		pCommand->GetParameter(&upper, 3, MPT_INT);
		pCommand->GetParameter(&lower, 4, MPT_INT);

		OnPeerMove(pCommand->GetSenderUID(), pos, dir, velocity, ZC_STATE_UPPER(upper), ZC_STATE_LOWER(lower));
		}
		break;
		*/
	case MC_MATCH_OBTAIN_WORLDITEM:
		{
			if (!IsReplay()) break;

			MUID uidPlayer;
			int nItemUID;

			pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
			pCommand->GetParameter(&nItemUID, 1, MPT_INT);

			ZGetGameClient()->OnObtainWorldItem(uidPlayer, nItemUID);
		}
		break;
	case MC_MATCH_SPAWN_WORLDITEM:
		{
			if (!IsReplay()) break;

			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (pParam->GetType()!=MPT_BLOB) break;

			void* pSpawnInfoBlob = pParam->GetPointer();

			ZGetGameClient()->OnSpawnWorldItem(pSpawnInfoBlob);
		}
		break;
	case MC_MATCH_REMOVE_WORLDITEM:
		{
			if (!IsReplay()) break;

			int nItemUID;

			pCommand->GetParameter(&nItemUID, 0, MPT_INT);

			ZGetGameClient()->OnRemoveWorldItem(nItemUID);
		}
		break;
	case MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (pParam->GetType()!=MPT_BLOB) break;

			void* pActiveTrapBlob = pParam->GetPointer();
			ZGetGameClient()->OnNotifyActivatedTrapItemList(pActiveTrapBlob);
			//todok ������ ����� ?E��� ���÷����Ҷ��� ��?E?�Ǵ°� Ȯ���غ���
		}
		break;




	case MC_PEER_BASICINFO	: OnPeerBasicInfo(pCommand);break;
	case MC_PEER_HPINFO		: OnPeerHPInfo(pCommand);break;
	case MC_PEER_HPAPINFO	: OnPeerHPAPInfo(pCommand);break;
	case MC_PEER_DUELTOURNAMENT_HPAPINFO		: OnPeerDuelTournamentHPAPInfo(pCommand);break;
	case MC_PEER_PING		: OnPeerPing(pCommand); break;
	case MC_PEER_PONG		: OnPeerPong(pCommand); break;
	case MC_PEER_OPENED		: OnPeerOpened(pCommand); break;
	case MC_PEER_DASH	: OnPeerDash(pCommand); break;
	case MC_PEER_SHOT:
		{
#ifdef _ZPEERANTIHACK
		if (ZPeerAntiHackIsBlocked(pCommand->GetSenderUID())) return true;
#endif
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) break;	// ������ �ִ�

			ZPACKEDSHOTINFO *pinfo =(ZPACKEDSHOTINFO*)pParam->GetPointer();

			rvector pos = rvector(pinfo->posx,pinfo->posy,pinfo->posz);
			rvector to = rvector(pinfo->tox,pinfo->toy,pinfo->toz);

			// TODO : �ð������� �����ؾ��Ѵ�
			OnPeerShot(pCommand->GetSenderUID(), pinfo->fTime, pos, to, (MMatchCharItemParts)pinfo->sel_type);
		}
		break;
	case MC_PEER_SHOT_MELEE:
		{
#ifdef _ZPEERANTIHACK
		if (ZPeerAntiHackIsBlocked(pCommand->GetSenderUID())) return true;
#endif
			float fShotTime;
			rvector pos, dir;

			pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
			pCommand->GetParameter(&pos, 1, MPT_POS);

			OnPeerShot(pCommand->GetSenderUID(), fShotTime, pos, pos, MMCIP_MELEE);
		}
		break;

	case MC_PEER_SHOT_SP:
		{
#ifdef _ZPEERANTIHACK
		if (ZPeerAntiHackIsBlocked(pCommand->GetSenderUID())) return true;
#endif
			float fShotTime;
			rvector pos, dir;
			int sel_type,type;

			pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
			pCommand->GetParameter(&pos, 1, MPT_POS);
			pCommand->GetParameter(&dir, 2, MPT_VECTOR);
			pCommand->GetParameter(&type, 3, MPT_INT);
			pCommand->GetParameter(&sel_type, 4, MPT_INT);



			unsigned long spType;
			rvector Position = pos;
			rvector Direction = dir;

			if (sel_type == 6)        // if type is KIT
			{
				{
					if (IsNan(Position.x) || IsNan(Position.y) || IsNan(Position.z) || IsNan(Direction.x) || IsNan(Direction.y) || IsNan(Direction.z) ||
						IsInf(Position.x) || IsInf(Position.y) || IsInf(Position.z) || IsInf(Direction.x) || IsInf(Direction.y) || IsInf(Direction.z))
					{
						break;
					}
				}
			}

			// fShotTime �� ������?E.
			//fShotTime=GetTime()-(float)GetPing(pCommand->GetSenderUID())*0.001f;

			OnPeerShotSp(pCommand->GetSenderUID(), fShotTime, pos, dir,type,(MMatchCharItemParts)sel_type);
		}
		break;

	case MC_PEER_RELOAD:
		{
			OnPeerReload(pCommand->GetSenderUID());
		}
		break;
	case MC_PEER_CHANGECHARACTER:
		{
			OnPeerChangeCharacter(pCommand->GetSenderUID());
		}
		break;

	case MC_PEER_DIE:
		{
			MUID	uid;
			pCommand->GetParameter(&uid , 0, MPT_UID);

			OnPeerDie(pCommand->GetSenderUID(), uid);

		}
		break;
	case MC_PEER_BUFF_INFO:
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if(pParam->GetType()!=MPT_BLOB) break;
			void* pBlob = pParam->GetPointer();

			OnPeerBuffInfo(pCommand->GetSenderUID(), pBlob);
		}
		break;
	case MC_MATCH_GAME_DEAD:
		{
			MUID uidAttacker, uidVictim;
			unsigned long int nAttackerArg, nVictimArg;

			pCommand->GetParameter(&uidAttacker, 0, MPT_UID);
			pCommand->GetParameter(&nAttackerArg, 1, MPT_UINT);
			pCommand->GetParameter(&uidVictim, 2, MPT_UID);
			pCommand->GetParameter(&nVictimArg, 3, MPT_UINT);

			OnPeerDead(uidAttacker, nAttackerArg, uidVictim, nVictimArg);
		}
		break;
	case MC_MATCH_GAME_TEAMBONUS:
		{
			MUID uidChar;
			unsigned long int nExpArg;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&nExpArg, 1, MPT_UINT);

			OnReceiveTeamBonus(uidChar, nExpArg);
		}
		break;
/*
	case MC_MATCH_ASSIGN_COMMANDER:
		{
			MUID uidRedCommander, uidBlueCommander;

			pCommand->GetParameter(&uidRedCommander, 0, MPT_UID);
			pCommand->GetParameter(&uidBlueCommander, 1, MPT_UID);

			OnAssignCommander(uidRedCommander, uidBlueCommander);
		}
		break;
*/
	case MC_PEER_SPAWN:
		{
			rvector pos, dir;
			pCommand->GetParameter(&pos, 0, MPT_POS);
			pCommand->GetParameter(&dir, 1, MPT_DIR);

			OnPeerSpawn(pCommand->GetSenderUID(), pos, dir);
		}
		break;
	case MC_MATCH_GAME_RESPONSE_SPAWN:
		{
			MUID uidChar;
			MShortVector s_pos, s_dir;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);
			pCommand->GetParameter(&s_pos, 1, MPT_SVECTOR);
			pCommand->GetParameter(&s_dir, 2, MPT_SVECTOR);

			rvector pos, dir;
			pos = rvector((float)s_pos.x, (float)s_pos.y, (float)s_pos.z);
			dir = rvector(ShortToDirElement(s_dir.x), ShortToDirElement(s_dir.y), ShortToDirElement(s_dir.z));

			OnPeerSpawn(uidChar, pos, dir);
		}
		break;

		// Orby: (AntiLead).
	case MC_GUNZ_ANTILEAD:
	{
		MCommandParameter* pParam = pCommand->GetParameter(0);

		if (pParam->GetType() != MPT_BLOB) break;

		void* pBlob = pParam->GetPointer();
		if (pBlob == NULL) break;

		int nSize = MGetBlobArrayCount(pBlob);
		if (nSize == NULL) break;

		ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
		if (pCharacter == NULL) break;

		for (int i = 0; i < nSize; ++i)
		{
			AntiLead_Info* pInfo = (AntiLead_Info*)MGetBlobArrayElement(pBlob, i);
			if (pInfo == NULL) break;

			if (pInfo->m_nVictimLowID != ZGetGameClient()->GetPlayerUID().Low) continue;

			ZItem* pItem = pCharacter->GetItems()->GetItem((MMatchCharItemParts)pInfo->m_nSelType);
			if (!pItem || !pItem->GetDesc()) continue;

			float fRatio = pItem->GetPiercingRatio(pItem->GetDesc()->m_nWeaponType.Ref(), (RMeshPartsType)pInfo->m_nPartsType);
			float fDamage = CalcActualDamage(pCharacter, m_pMyCharacter, (float)pItem->GetDesc()->m_nDamage.Ref());
			if (fDamage == NULL) break;
			MMatchWeaponType wt = pItem->GetDesc()->m_nWeaponType.Ref();
			ZDAMAGETYPE dt = (pInfo->m_nPartsType == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
			if (m_pMyCharacter && ZGetGameClient()->GetPlayerUID().Low != pCommand->GetSenderUID().Low) 
			{
				if (pCharacter && !ZGetGame()->GetMatch()->IsTeamPlay() || ((m_pMyCharacter->IsTeam(pCharacter) && ZGetGame()->GetMatch()->GetTeamKillEnabled()) || !m_pMyCharacter->IsTeam(pCharacter)))
				{
					m_pMyCharacter->OnDamaged(pCharacter, rvector(pInfo->X, pInfo->Y, pInfo->Z), dt, wt, fDamage, fRatio);
				}
				/*if (m_pMyCharacter && ZGetGameClient()->GetPlayerUID().Low != pCommand->GetSenderUID().Low)
				{
					UpdateHPAPBar();
				}
				else
				{
					m_pMyCharacter->GetStatus().CheckCrc();
					m_pMyCharacter->GetStatus().Ref().nHitsDealt += 1;
					m_pMyCharacter->GetStatus().MakeCrc();
				}*/
				ZPOSTDMGTAKEN(MCommandParameterInt(fDamage), MCommandParameterInt(0), pCommand->GetSenderUID());
			}
		}
	}
	break;

	case MC_GUNZ_DMGGIVEN:
	{
		int GivenDmg = 0, Type = 0, Index = 0;
		if (!pCommand->GetParameter(&GivenDmg, Index++, MPT_INT) || !pCommand->GetParameter(&Type, Index++, MPT_INT)) return true;
		if (Type == 0 && !m_pMyCharacter->IsObserverTarget() && !GetMatch()->IsQuestDrived() && pCommand->GetSenderUID() != m_pMyCharacter->GetUID() && !ZGetGameClient()->IsDuelTournamentGame())
		{
			ZGetGame()->m_pMyCharacter->GetStatus().CheckCrc();
			ZGetGame()->m_pMyCharacter->GetStatus().Ref().nGivenDamage += GivenDmg;
			ZGetGame()->m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage += GivenDmg;
			ZGetGame()->m_pMyCharacter->GetStatus().MakeCrc();
		}
	}
	break;
	case MC_GUNZ_LASTDMG:
	{
		int GivenDmg = 0, TakenDmg = 0, Index = 0;
		if (pCommand->GetParameter(&GivenDmg, Index++, MPT_INT) && pCommand->GetParameter(&TakenDmg, Index++, MPT_INT) && !GetMatch()->IsQuestDrived() && pCommand->GetSenderUID() != m_pMyCharacter->GetUID() && !ZGetGameClient()->IsDuelTournamentGame())
		{
			ZCharacter* pCharacter = (ZCharacter*)ZGetCharacterManager()->Find(pCommand->GetSenderUID());
			if (pCharacter == NULL) break;  // Orby: Packet

			if (pCharacter)
			{
				pCharacter->GetStatus().CheckCrc();
				pCharacter->GetStatus().Ref().nRoundLastGivenDamage = GivenDmg;
				pCharacter->GetStatus().Ref().nRoundLastTakenDamage = TakenDmg;
				pCharacter->GetStatus().MakeCrc();
			}
		}
	}
	break;

	case MC_MATCH_SET_OBSERVER:
		{
			MUID uidChar;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);

			OnSetObserver(uidChar);
		}
		break;
	case MC_PEER_CHANGE_WEAPON:
		{
			int nWeaponID;

			pCommand->GetParameter(&nWeaponID, 0, MPT_INT);

			OnChangeWeapon(pCommand->GetSenderUID(),MMatchCharItemParts(nWeaponID));
		}

		break;

	case MC_PEER_SPMOTION:
		{
			int nMotionType;

			pCommand->GetParameter(&nMotionType, 0, MPT_INT);

			OnPeerSpMotion(pCommand->GetSenderUID(),nMotionType);
		}
		break;

	case MC_PEER_CHANGE_PARTS:
		{
			int PartsType;
			int PartsID;

			pCommand->GetParameter(&PartsType, 0, MPT_INT);
			pCommand->GetParameter(&PartsID, 1, MPT_INT);

			OnChangeParts(pCommand->GetSenderUID(),PartsType,PartsID);
		}
		break;

	case MC_PEER_ATTACK:
		{
			int		type;
			rvector pos;

			pCommand->GetParameter(&type, 0, MPT_INT);
			pCommand->GetParameter(&pos , 1, MPT_POS);

			OnAttack( pCommand->GetSenderUID(), type, pos);
		}
		break;

	case MC_PEER_DAMAGE:
		{
			MUID	tuid;
			int		damage;

			pCommand->GetParameter(&tuid   , 0, MPT_UID);
			pCommand->GetParameter(&damage , 1, MPT_INT);

			OnDamage( pCommand->GetSenderUID(), tuid, damage);
		}
		break;


	case MC_PEER_VAMPIRE:
		{
			MUID uidAttacker;
			float damage;

			pCommand->GetParameter( &uidAttacker, 0, MPT_UID );
			pCommand->GetParameter( &damage, 1, MPT_FLOAT );

			OnVampire( pCommand->GetSenderUID(), uidAttacker, damage );
		}
		break;

	case MC_PEER_SNIFER:
		{
			bool bSniping = false;

			pCommand->GetParameter( &bSniping, 0, MPT_BOOL );

			OnPeerScope( pCommand->GetSenderUID(), bSniping );
		}
		break;

	case MC_ADMIN_SUMMON:
		{
			MUID uidPlayer;
			char szName[128];

			pCommand->GetParameter( &uidPlayer, 0, MPT_UID );
			pCommand->GetParameter( szName, 1, MPT_STR, 128 );

			OnAdminSummon( uidPlayer, szName );
		}
		break;

	case MC_ADMIN_FREEZE:
	{
		MUID uidPlayer;
		char szName[64];
		pCommand->GetParameter(&uidPlayer, 0, MPT_UID);
		pCommand->GetParameter(szName, 1, MPT_STR, sizeof(szName));
		OnAdminFreeze(uidPlayer, szName);
	}break;
	case MC_ADMIN_FREEZE_ALL:
	{
		MUID uidAdmin = MUID(0, 0);
		ZCharacter* pAdmin = NULL;

		if (ZGetGame() && ZGetGame()->m_pMyCharacter && (ZGetGame()->m_pMyCharacter->IsAdminName()))
			break;

		if (!ZGetCharacterManager())
			break;

		uidAdmin = pCommand->GetSenderUID();
		pAdmin = ZGetCharacterManager()->Find(uidAdmin);

		if (pAdmin == NULL)
			break;

		if (!IsAdminGrade(pAdmin->GetCharInfo()->nUGradeID))
			break;

		int nstate = 0;
		pCommand->GetParameter(&nstate, 0, MPT_INT);

		m_bPause = (nstate != 0);

		if (m_pMyCharacter)
			rStuckPosition = m_pMyCharacter->GetPosition();

		char szMsg[256];
		if (nstate != 0)
			sprintf(szMsg, "The room has been freezed");
		else
			sprintf(szMsg, "The room has been unfreezed");

		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szMsg);
	}
	break;
	case MC_ADMIN_GOTO:
		{
			MUID uidPlayer;
			char szName[128];

			pCommand->GetParameter( &uidPlayer, 0, MPT_UID );
			pCommand->GetParameter( szName, 1, MPT_STR, 128 );

			OnAdminGoTo( uidPlayer, szName );
		}
		break;

	case MC_ADMIN_SLAP:
		{
			MUID uidPlayer;
			char szName[128];

			pCommand->GetParameter( &uidPlayer, 0, MPT_UID );
			pCommand->GetParameter( szName, 1, MPT_STR, 128 );

			OnAdminSlap( uidPlayer, szName );
		}
		break;

	case MC_ADMIN_SPAWN_RESPONSE:
		{
			MUID uidPlayer, uidVictim;
			//char szName[128];

			pCommand->GetParameter( &uidPlayer, 0, MPT_UID );
			pCommand->GetParameter( &uidVictim, 1, MPT_UID);

			OnAdminSpawn( uidPlayer, uidVictim );
		}
		break;

	case MC_MATCH_RESPONSE_ROLL:
		{
			if (!ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_ROLLTHEDICE))
				break;

			char szMsg[256];
			MUID roller;
			int roll;
			
			pCommand->GetParameter(&roller, 0, MPT_UID);
			pCommand->GetParameter(&roll, 1, MPT_INT);
			pCommand->GetParameter(szMsg, 2, MPT_STR, sizeof(szMsg));

			if (roller == m_pMyCharacter->GetUID())
			{
				SetRolledDice(roll);
				ZGetGameInterface()->GetCombatInterface()->UpdateRTDMsg( szMsg );
				if (GetRolledDice() != roll) break;
				switch (GetRolledDice())
				{
				case 1:
					//Nothing Neccessary here.
					break;
				case 2:
					m_pMyCharacter->SetHP(20);
					m_pMyCharacter->SetAP(0);
					m_pMyCharacter->SetMaxAP(0); //Added 0 AP MAX to prevent users from fully healing back up.
					ZGetScreenEffectManager()->ReSetHpPanel();
					break;
				case 3:
					//Nothing necessary here.
					break;
				case 4:
					m_pMyCharacter->SetMaxAP(300);
					m_pMyCharacter->SetMaxHP(300);
					m_pMyCharacter->SetHP(300);
					m_pMyCharacter->SetAP(300);
					ZGetScreenEffectManager()->ReSetHpPanel();

					break;
				case 5:
					//Nothing necessary here.
					break;
				case 6:
					//Nothing necessary here.
					break;
				case 7:
					//Nothing necessary here.
					break;
				case 8:
					//Nothing necessary here.
					break;
				case 9:
					//Nothing necessary here.
					break;
				case 10:
					//I left the cases here to confuse disassemblers. Once again nothing here. :>
					break;
				case 11:
					break;
				}
			}

			//if (strlen(szMsg) > 0) Bug fixed, no longer necessary.
				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szMsg);
		}
		break;

	case MC_MATCH_RESET_TEAM_MEMBERS:
		{
			OnResetTeamMembers(pCommand);
		}
		break;

	
	case MC_REQUEST_XTRAP_HASHVALUE:				// Update sgk 0706 (��?E��? ��?E��?ȣȯ�� ���� Ŀ�ǵ常 ��?E
		{
		}
		break;

	case MC_MATCH_DISCONNMSG :
		{
			DWORD dwMsgID;
			pCommand->GetParameter( &dwMsgID, 0, MPT_UINT );

			ZApplication::GetGameInterface()->OnDisconnectMsg( dwMsgID );
		}
		break;

		/*
	case MC_PEER_SKILL:
		{
			float fTime;
			int nSkill,sel_type;

			pCommand->GetParameter(&fTime, 0, MPT_FLOAT);
			pCommand->GetParameter(&nSkill, 1, MPT_INT);
			pCommand->GetParameter(&sel_type, 2, MPT_INT);

			OnPeerSkill( pCommand->GetSenderUID(), fTime, nSkill, sel_type );
		}
		break;
		*/

	case ZC_TEST_INFO:
		{
			OutputToConsole("Sync : %u", ZGetGameClient()->GetGlobalClockCount());

			rvector v;
			//				int iQueueCount;
			v = m_pMyCharacter->GetPosition();
			//				iQueueCount = (int)m_pMyCharacter->m_PathQueue.size();
			OutputToConsole("My Pos = %.2f %.2f %.2f", v.x, v.y, v.z);

			/*
			for (ZCharacterItor itor = m_OtherCharacters.begin(); itor != m_OtherCharacters.end(); )
			{
			ZCharacter* pCharacter = (*itor).second;
			v = pCharacter->m_Position;
			iQueueCount = (int)pCharacter->m_PathQueue.size();
			OutputToConsole("Other Pos(%d) = %.2f %.2f %.2f", iQueueCount, v.x, v.y, v.z);
			++itor;
			}
			*/
		}
		break;
	case ZC_BEGIN_PROFILE:	
		g_bProfile=true;	
		break;
	case ZC_END_PROFILE:	
		g_bProfile=false;	
		break;
	case ZC_EVENT_OPTAIN_SPECIAL_WORLDITEM:
		{
			OnLocalOptainSpecialWorldItem(pCommand);
		}
		break;

#ifdef _GAMEGUARD
	case MC_REQUEST_GAMEGUARD_AUTH :
		{
			DWORD dwIndex;
			DWORD dwValue1;
			DWORD dwValue2;
			DWORD dwValue3;

			pCommand->GetParameter( &dwIndex, 0, MPT_UINT );
			pCommand->GetParameter( &dwValue1, 1, MPT_UINT );
			pCommand->GetParameter( &dwValue2, 2, MPT_UINT );
			pCommand->GetParameter( &dwValue3, 3, MPT_UINT );

			ZApplication::GetGameInterface()->OnRequestGameguardAuth( dwIndex, dwValue1, dwValue2, dwValue3 );

#ifdef _DEBUG
			mlog( "zgame recevie request gameguard auth. CmdID(%u) : %u, %u, %u, %u\n", pCommand->GetID(), dwIndex, dwValue1, dwValue2, dwValue3 );
#endif

		}
        break;
#endif

#ifdef _XTRAP
	case MC_REQUEST_XTRAP_SEEDKEY:									// add sgk 0411
		{
			MCommandParameter* pParam = pCommand->GetParameter(0);
			if (pParam->GetType() != MPT_BLOB)
			{
				break;
			}
			void* pComBuf = pParam->GetPointer();
			unsigned char *szComBuf = (unsigned char *)MGetBlobArrayElement(pComBuf, 0);
			ZApplication::GetGameInterface()->OnRequestXTrapSeedKey(szComBuf);
		}
		break;
#endif
	case MC_MATCH_RESPONSE_USE_SPENDABLE_BUFF_ITEM:
		{
			MUID uidItem;
			int nResult;

			pCommand->GetParameter(&uidItem, 0, MPT_UID);
			pCommand->GetParameter(&nResult, 0, MPT_INT);

			OnResponseUseSpendableBuffItem(uidItem, nResult);
		}
		break;

	case MC_MATCH_SPENDABLE_BUFF_ITEM_STATUS:
		{
			//���������ӽ��ּ� 
			_ASSERT(0);
			/*
			MUID uidChar;

			pCommand->GetParameter(&uidChar, 0, MPT_UID);

			MCommandParameter* pParam = pCommand->GetParameter(1);
			if (pParam->GetType() != MPT_BLOB) break;
			void* pCmdBuf = pParam->GetPointer();
            MTD_CharBuffInfo* pCharBuffInfo = (MTD_CharBuffInfo*)MGetBlobArrayElement(pCmdBuf, 0);

			OnGetSpendableBuffItemStatus(uidChar, pCharBuffInfo);
			*/
		}
		break;
	}

	// ����?E?��Ҳ Ŀ�ǵ尡 ó���Ǿ���?E�˼��ֵ��� ��ȸ�� ����
	// ����?E?��?E��Ҳ Ŀ�ǵ尡 ZGame���� ó���� �Ŀ� ���� ?E��?E���� ���� ���� ���� ���Ѱ�
	ZRule* pRule = m_Match.GetRule();
	if (pRule) {
		pRule->AfterCommandProcessed(pCommand);
	}

	// return true;
	return false;
}

rvector ZGame::GetMyCharacterFirePosition(void)
{
	rvector p = ZGetGame()->m_pMyCharacter->GetPosition();
	p.z += 160.f;
	return p;
}

// ����?E������ �� ����� ������ �и��ȴ� 
// ?E �̸� history�� ������?E������ Ÿ�ֿ̹� ����ȴ�.
void ZGame::OnPeerBasicInfo(MCommand *pCommand,bool bAddHistory,bool bUpdate)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return;

	ZPACKEDBASICINFO* ppbi= (ZPACKEDBASICINFO*)pParam->GetPointer();
	
	ZBasicInfo bi;
	bi.position = rvector(Roundf(ppbi->posx),Roundf(ppbi->posy),Roundf(ppbi->posz));
	bi.velocity = rvector(ppbi->velx,ppbi->vely,ppbi->velz);
	bi.direction = 1.f/32000.f * rvector(ppbi->dirx,ppbi->diry,ppbi->dirz);
	
	MUID uid = pCommand->GetSenderUID();

	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uid);
	if (pPeer) {
		if (pPeer->IsOpened() == false) {
			MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_PEER_OPENED, ZGetGameClient()->GetPlayerUID());
			pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
			ZGetGameClient()->Post(pCmd);

			pPeer->SetOpened(true);
		}
	}

	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	// ĳ������ ����ð��� ������Ʈ�Ѵ�
	// ĳ������ ����ð� ����ġ
	float fCurrentLocalTime = pCharacter->m_fTimeOffset + GetTime() ;

	// ĳ���Ͱ� ������ �ð��� ���� ������ �ð�?E3�� ��?E���̰� ��?E���� �˰��ִ� �ð��� ��ࣴ�.
	float fTimeError = ppbi->fTime - fCurrentLocalTime;
	if(fabs(fTimeError) > TIME_ERROR_BETWEEN_RECIEVEDTIME_MYTIME) {
		pCharacter->m_fTimeOffset = ppbi->fTime - GetTime();
		pCharacter->m_fAccumulatedTimeError = 0;
		pCharacter->m_nTimeErrorCount = 0;
	}else
	{
		// ���̰� 3�� �̳���?E�����ð� ���ߴٰ� ����(������ ��)�� �����Ѵ�
		pCharacter->m_fAccumulatedTimeError += fTimeError;
		pCharacter->m_nTimeErrorCount ++;
		if(pCharacter->m_nTimeErrorCount > 10) {
			pCharacter->m_fTimeOffset += .5f*pCharacter->m_fAccumulatedTimeError/10.f;
			pCharacter->m_fAccumulatedTimeError = 0;
			pCharacter->m_nTimeErrorCount = 0;
		}
	}

	// ����ð��� ������ ������ �����ð����� ���.
	pCharacter->m_fLastReceivedTime = GetTime();

	pCharacter->m_dwLastBasicInfoTime = timeGetTime();


	// ���߿� ������ ���� histroy �� ��?EѴ?
	if(bAddHistory)
	{
		ZBasicInfoItem *pitem=new ZBasicInfoItem;
		CopyMemory(&pitem->info, &bi, sizeof(ZBasicInfo));

		pitem->fReceivedTime=GetTime();

		pitem->fSendTime= ppbi->fTime - pCharacter->m_fTimeOffset;	// �� �������� ��ȯ
		
		pCharacter->m_BasicHistory.push_back(pitem);

		while(pCharacter->m_BasicHistory.size()>CHARACTER_HISTROY_COUNT)
		{
			delete *pCharacter->m_BasicHistory.begin();
			pCharacter->m_BasicHistory.erase(pCharacter->m_BasicHistory.begin());
		}
	}

	if(bUpdate)
	{
		// ���÷��̶��� ������?E�� ĳ���ʹ� ����̳� �ӵ�����ܻ ������Ʈ �� �ʿ䰡 ����.
		if(!IsReplay() && pCharacter->IsHero()) return;
		
		
		// �����̿��� network delay �ð��� ��?EѴ?
		/*
		float fDelay=(GetTime()-ppbi->fTime) *.5f;
		pCharacter->m_PingData.push_back(fDelay);
		while(pCharacter->m_PingData.size()>10)
			pCharacter->m_PingData.erase(pCharacter->m_PingData.begin());

		float fDelaySum=0;
		for(list<float>::iterator i=pCharacter->m_PingData.begin();i!=pCharacter->m_PingData.end();i++)
		{
			fDelaySum+=*i;
		}

		pCharacter->m_fAveragePingTime=fDelaySum/(float)pCharacter->m_PingData.size();
		*/

		// �����ӹ� �ִϸ��̼ǵ��� ó��

		// ����?E������ ������ ��ġ�� �����ϱ⶧���� ������ ��ġ�� ������ �ʿ䰡 ����.
		// ���� �Ʒ��� ����?E?�ܼ����ڵ�� ?E?.
/*
		rvector nextPos;
		pCharacter->SetPeerPosition(pci->position);
		pCharacter->SetVelocity(pci->velocity);
		pCharacter->SetAccel(pci->accel);
		pCharacter->m_TargetDir = pci->direction;
		pCharacter->m_fLastValidTime = GetTime();
*/

		((ZNetCharacter*)(pCharacter))->SetNetPosition(bi.position, bi.velocity, bi.direction);

		pCharacter->SetAnimationLower((ZC_STATE_LOWER)ppbi->lowerstate);
		pCharacter->SetAnimationUpper((ZC_STATE_UPPER)ppbi->upperstate);

		// ����ִ� ���Ⱑ �ٸ�?E�ٲ��ش�
		if(pCharacter->GetItems()->GetSelectedWeaponParts()!=ppbi->selweapon) {
			pCharacter->ChangeWeapon((MMatchCharItemParts)ppbi->selweapon);
		}
	}
}

void ZGame::OnPeerHPInfo(MCommand *pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP=0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);

	// Custom: Changed logic here
	if( !IsReplay() && pCharacter->GetUID() != ZGetMyUID() && uid != ZGetMyUID() )
		pCharacter->SetHP( fHP );
	else if( IsReplay() || ZGetGameInterface()->GetCombatInterface()->GetObserverMode() )
		pCharacter->SetHP( fHP );

	// ����?E��?E�������� ������?E���� hp ������ �����Ѵ�.
	//if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
	//	pCharacter->SetHP(fHP);
	//}
}


void ZGame::OnPeerHPAPInfo(MCommand *pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	float fHP=0.0f;
	pCommand->GetParameter(&fHP, 0, MPT_FLOAT);
	float fAP=0.0f;
	pCommand->GetParameter(&fAP, 1, MPT_FLOAT);

	// Custom: Changed logic here
	if( !IsReplay() && pCharacter->GetUID() != ZGetMyUID() && uid != ZGetMyUID() )
	{
		pCharacter->SetHP( fHP );
		pCharacter->SetAP( fAP );
	}
	// Custom: Added a check for teambar (the teamid.ref() is a saftey check, although it's probably bypassed easily).
	else if( IsReplay() || ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		pCharacter->SetHP( fHP );
		pCharacter->SetAP( fAP );
	}


	// ����?E��?E�������� ������?E���� hp ������ �����Ѵ�.
	//if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode()) {
	//	pCharacter->SetHP(fHP);
	//	pCharacter->SetAP(fAP);
	//}
}

void ZGame::OnPeerDuelTournamentHPAPInfo(MCommand *pCommand)
{
	MUID uid = pCommand->GetSenderUID();
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
	if (!pCharacter) return;

	BYTE MaxHP = 0;
	BYTE MaxAP = 0;
	BYTE HP = 0;
	BYTE AP = 0;

	pCommand->GetParameter(&MaxHP, 0, MPT_UCHAR);	
	pCommand->GetParameter(&MaxAP, 1, MPT_UCHAR);

	pCommand->GetParameter(&HP, 2, MPT_UCHAR);	
	pCommand->GetParameter(&AP, 3, MPT_UCHAR);

	// ���� peer�� HP/AP ������ ��?E���������� ������?E���ؼ��� ?E޵ȴ?
	// ��ȹ?E�����ʸ�Ʈ�� ������ ���� �÷��߿��� ?E�E��?E?HP,AP�� UI�� �׷��ֱ�� ��?E�ִ�.
	// �������� �ƴҶ�(���� �÷��̸� �Ҷ�)�� peer�� ĳ���� HP, AP�� ��������?E
	// '�� ĳ������ ������ ���� ���� �Ǵ��Ѵ�'�� ���� p2p��å?E������ �߻��� ?E��?Eĳ���Ϳ� ���� HP/AP�� set��?E��?E
	// UI ���?E��?���� HP/AP���� ��?EѴ?
	if(ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT) {
		((ZRuleDuelTournament*)m_Match.GetRule())->SetPlayerHpApForUI(uid, (float)MaxHP, (float)MaxAP, (float)HP, (float)AP);
	}

	// ����?E��?E�������� ������?E���� hp ������ �����Ѵ�.
	// Custom: Changed logic here
	if( pCharacter->GetUID() != ZGetMyUID() && uid != ZGetMyUID() )
	{
		pCharacter->SetMaxHP((float)MaxHP);
		pCharacter->SetMaxAP((float)MaxAP);
		pCharacter->SetHP((float)HP);
		pCharacter->SetAP((float)AP);
	}
	else if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		pCharacter->SetMaxHP((float)MaxHP);
		pCharacter->SetMaxAP((float)MaxAP);
		pCharacter->SetHP((float)HP);
		pCharacter->SetAP((float)AP);
	}
}

#ifdef _DEBUG
	static int g_nPingCount=0;
	static int g_nPongCount=0;
#endif

void ZGame::OnPeerPing(MCommand *pCommand)
{
	if(m_bReplaying.Ref()) return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);
	
	// PONG ���� �����Ѵ�
	MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
	MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PONG), 
								  pCommand->GetSenderUID(), ZGetGameClient()->GetUID());	
	pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
	ZGetGameClient()->Post(pCmd);
}

void ZGame::OnPeerPong(MCommand *pCommand)
{
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(pCommand->GetSenderUID());
	if (pPeer == NULL)
		return;

	unsigned int nTimeStamp;
	pCommand->GetParameter(&nTimeStamp, 0, MPT_UINT);

	int nPing = (GetTickTime() - nTimeStamp)/2;
    pPeer->UpdatePing(GetTickTime(), nPing);

/*
	if (pPeer->IsOpened() == false) {
		MCommand* pCmd = ZGetGameClient()->CreateCommand(MC_PEER_OPENED, ZGetGameClient()->GetPlayerUID());
		pCmd->AddParameter(new MCmdParamUID(pPeer->uidChar));
		ZGetGameClient()->Post(pCmd);

		pPeer->SetOpened(true);
	}
*/
	#ifdef _DEBUG
		g_nPongCount++;

		// Custom: Added test messages to fix spike issue
		//OutputToConsole( "pong -> response (%s): %d", pPeer->CharInfo.szName, nPing );
	#endif
}

void ZGame::OnPeerOpened(MCommand *pCommand)
{
	MUID uidChar;
	pCommand->GetParameter(&uidChar, 0, MPT_UID);

	//// Show Character ////////////////////////////////////////
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uidChar);
	if (pCharacter && pCharacter->IsDie()==false) {
		pCharacter->SetVisible(true);

		// ���� ĳ���Ϳ��� �ڽ��� ���⸦ �˸���...
		ZCharacter* pMyCharacter = ZGetGame()->m_pMyCharacter;
		if(pMyCharacter)
		{
			int nParts = ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
			ZGetGame()->m_pMyCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = false;
			ZPostChangeWeapon(nParts);
		}
		// �ڽ��� ���� ���¸� �˸���
		PostMyBuffInfo();
	}

#ifdef _DEBUG
	//// PeerOpened Debug log //////////////////////////////////
	char* pszName = "Unknown";
	char* pszNAT = "NoNAT";
	MMatchPeerInfo* pPeer = ZGetGameClient()->FindPeer(uidChar);
	if (pPeer) {
		pszName = pPeer->CharInfo.szName;
		if (pPeer->GetUDPTestResult() == false) pszNAT = "NAT";
	}

	char szBuf[64];
	sprintf(szBuf, "PEER_OPENED(%s) : %s(%d%d) \n", pszNAT, pszName, uidChar.High, uidChar.Low);
	OutputDebugString(szBuf);
#endif
}

void ZGame::OnChangeWeapon(MUID& uid, MMatchCharItemParts parts)
{
	ZCharacter* pCharacter = m_CharacterManager.Find(uid);
	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;
	//	_ASSERT(pCharacter != NULL);

	if (pCharacter && pCharacter != m_pMyCharacter)		// �� ĳ���ʹ� �̹� �ٲ��.
	{
		pCharacter->ChangeWeapon(parts);
	}
	if (pCharacter)
	{
		if (pCharacter == m_pMyCharacter)
		{
			if (m_Match.GetMatchType() == MMATCH_GAMETYPE_SPY && m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PLAY)
			{
				if (!ZApplication::GetGameInterface()->GetCombatInterface()) return;

				ZApplication::GetGameInterface()->GetCombatInterface()->SetDefaultSpyTip(m_pMyCharacter->GetTeamID());

				if (!ZApplication::GetGameInterface()->GetCombatInterface()->m_bSpyLocationOpened && !ZApplication::GetGameInterface()->GetCombatInterface()->GetObserverMode())
				{
					ZItem* pItem = m_pMyCharacter->GetItems()->GetItem(parts);
					if (!pItem) return;

					const char* pszSpyTip = ZApplication::GetGameInterface()->GetCombatInterface()->GetSuitableSpyItemTip((int)pItem->GetDescID());
					if (pszSpyTip)
						ZApplication::GetGameInterface()->GetCombatInterface()->SetSpyTip(pszSpyTip);
				}
			}
		}
		else
		{
			pCharacter->ChangeWeapon(parts);
		}
	}
}

void ZGame::OnChangeParts(MUID& uid,int partstype,int PartsID)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if ( pCharacter ) {
		pCharacter->OnChangeParts( (RMeshPartsType)partstype , PartsID );
	}
}

void ZGame::OnAttack(MUID& uid,int type,rvector& pos)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	// ?E?��Ǹ�..
	if ( pCharacter ) {

	}
}

void ZGame::OnDamage(MUID& uid,MUID& tuid,int damage)
{
/*
	ZCharacter* pSender = NULL;		
	ZCharacter* pTarget = NULL;		

	pSender = m_CharacterManager.Find(uid);
	pTarget = m_CharacterManager.Find(tuid);

	pTarget->OnSimpleDamaged(NULL,damage,0.5f);
*/
}

void ZGame::OnVampire( const MUID& uidVictim, const MUID& uidAttacker, float fDamage )
{
	ZCharacter* pAttacker = (ZCharacter*) m_CharacterManager.Find( uidAttacker );
	ZCharacter* pVictim = (ZCharacter*) m_CharacterManager.Find( uidVictim );

	if( !pAttacker || !pVictim )
		return;

	if( !pAttacker->GetInitialized() || !pVictim->GetInitialized() || !pAttacker->IsVisible() || !pVictim->IsVisible() )
		return;

	if( IsReplay() )
	{
		int nAddHP = (int)(fDamage * 0.33f);
		int nCurrHP = pAttacker->GetHP();
		int nMaxHP = pAttacker->GetMaxHP();

		if( nCurrHP + nAddHP > nMaxHP )
		{
			pAttacker->SetHP( nMaxHP );

			int nAddAP = (nCurrHP + nAddHP) - nMaxHP;
			int nCurrAP = pAttacker->GetAP();
			int nMaxAP = pAttacker->GetMaxAP();

			if( nCurrAP + nAddAP > nMaxAP )
				pAttacker->SetAP( nMaxAP );
			else
				pAttacker->SetAP( nCurrAP + nAddAP );
		}
		else
			pAttacker->SetHP( nCurrHP + nAddHP );

		return;
	}

	if( ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed( MMOD_VAMPIRE ) && !ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived()
		&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		if( uidAttacker == ZGetMyUID() && uidVictim != ZGetMyUID() )
		{
			MMatchPeerInfo* pVictimPeer = ZGetGameClient()->FindPeer( uidVictim );

			if( !pVictimPeer )
				return;

			if( !pVictimPeer->IsOpened() )
				return;

			int nAddHP = (int)(fDamage * 0.33f);
			int nCurrHP = m_pMyCharacter->GetHP();
			int nMaxHP = m_pMyCharacter->GetMaxHP();

			if( nCurrHP + nAddHP > nMaxHP )
			{
				m_pMyCharacter->SetHP( nMaxHP );

				int nAddAP = (nCurrHP + nAddHP) - nMaxHP;
				int nCurrAP = m_pMyCharacter->GetAP();
				int nMaxAP = m_pMyCharacter->GetMaxAP();

				if( nCurrAP + nAddAP > nMaxAP )
					m_pMyCharacter->SetAP( nMaxAP );
				else
					m_pMyCharacter->SetAP( nCurrAP + nAddAP );
			}
			else
				m_pMyCharacter->SetHP( nCurrHP + nAddHP );
		}
	}
}

void ZGame::OnPeerScope( const MUID& uidPeer, bool bScope )
{
	ZCharacter* pOwner = (ZCharacter*) m_CharacterManager.Find( uidPeer );

	if( !pOwner )
		return;
	
	// visible? what if I scoped? I'll be invisible and this will return
	if( !pOwner->GetInitialized() /*|| !pOwner->IsVisible()*/ )
		return;

	if( !IsReplay() && uidPeer == ZGetMyUID() )
		return;

	ZCharaterStatusBitPacking& uStatus = pOwner->m_dwStatusBitPackingValue.Ref();

	if( ZGetGameInterface()->GetCombatInterface()->GetObserverMode() )
	{
		if( bScope )
		{
			if( !uStatus.m_bSniping )
			{
				uStatus.m_bSniping = true;

				if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter() == pOwner)
					ZGetGameInterface()->GetCombatInterface()->OnGadget(MWT_SNIFER);
			}
		}
		else
		{
			if( uStatus.m_bSniping )
			{
				uStatus.m_bSniping = false;

				if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter() == pOwner)
					ZGetGameInterface()->GetCombatInterface()->OnGadgetOff();
			}
		}
	}
}

void ZGame::OnAdminFreeze(const MUID& uidAdmin, const char* szTargetName)
{
	ZCharacter* pCharacter = ZGetGame()->m_CharacterManager.Find(uidAdmin);
	if (!pCharacter) return;
	ZCharacter* pTarget = ZGetGame()->m_CharacterManager.Find(ZGetGame()->m_pMyCharacter->GetUID());

	char szTemp[128];
	if (!stricmp(szTargetName, pTarget->GetUserName()))
	{

		sprintf(szTemp, "%s has frozen you.", pCharacter->GetUserName());
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT);
		if (!pTarget->IsAdminName() && pTarget->GetUserGrade() != MMUG_EVENTTEAM)
		{
			if (pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen == true)
			{
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = false;
			}
			else
			{
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = true;
			}
		}
	}
	if (!stricmp(szTargetName, "all"))
	{
		sprintf(szTemp, "%s has frozen the room", pCharacter->GetUserName());
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT);
		if (!pTarget->IsAdminName() && pTarget->GetUserGrade() != MMUG_EVENTTEAM)
		{
			if (pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen == true)			{
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = false;
			}
			else
			{
				pTarget->m_dwStatusBitPackingValue.Ref().m_bFrozen = true;
			}
		}
	}
}

void ZGame::OnAdminSummon( const MUID& uidAdmin, const char* szTargetName )
{
	ZCharacter* pAdmin = (ZCharacter*) m_CharacterManager.Find( uidAdmin );

	if( !pAdmin )
		return;

	if( !pAdmin->GetInitialized() || !pAdmin->IsVisible() )
		return;

	if( !stricmp( szTargetName, m_pMyCharacter->GetUserName() ) )
	{
		char szTemp[128];
		sprintf( szTemp, "%s has summoned you.", pAdmin->GetUserName() );
		ZChatOutput( MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT );
		m_pMyCharacter->SetPosition( pAdmin->GetPosition() );

		ZGetEffectManager()->AddReBirthEffect( pAdmin->GetPosition() );
		ZGetSoundEngine()->PlaySound( "fx_respawn" );
	}
}

void ZGame::OnAdminGoTo( const MUID& uidAdmin, const char* szTargetName )
{
	ZCharacter* pTarget = NULL;

	// herp derp
	// could put it into a new variable for Muid, but meh.
	for (ZCharacterManager::iterator i = ZGetCharacterManager()->begin(); i != ZGetCharacterManager()->end(); ++i)
	{
		ZCharacter* pChar = (ZCharacter*)(*i).second;

		if (pChar && pChar->GetInitialized() && pChar->IsVisible() && !pChar->IsAdminHide() && !stricmp( pChar->GetUserName(), szTargetName ))
		{
			pTarget = pChar;
			break;
		}
	}

	if( !pTarget )
		return;

	if( !pTarget->GetInitialized() || !pTarget->IsVisible() )
		return;

	// do not output a message. it's done by the server
	m_pMyCharacter->SetPosition( pTarget->GetPosition() );

	ZGetEffectManager()->AddReBirthEffect( m_pMyCharacter->GetPosition() );
	ZGetSoundEngine()->PlaySound( "fx_respawn" );
}

void ZGame::OnAdminSlap( const MUID& uidAdmin, const char* szTargetName )
{
	ZCharacter* pAdmin = (ZCharacter*) m_CharacterManager.Find( uidAdmin );

	if( !pAdmin )
		return;

	if( !pAdmin->GetInitialized() || !pAdmin->IsVisible() )
		return;

	if( !stricmp( szTargetName, m_pMyCharacter->GetUserName() ) )
	{
		char szTemp[128];
		sprintf( szTemp, "%s has slapped you!", pAdmin->GetUserName() );
		ZChatOutput( MCOLOR(ZCOLOR_CHAT_BROADCAST), szTemp, ZChat::CL_CURRENT );
		// Custom:Improved /admin_slap. Less glitchier, properly awards the kill etc.
		//m_pMyCharacter->SetAP( 0 );
		//m_pMyCharacter->SetHP( 0 );
		ZPostGameKill(uidAdmin);
		ZPostDie(uidAdmin);
	}
}

void ZGame::OnAdminSpawn( const MUID& uidAdmin, const MUID& uidVictim )
{
	ZCharacter* pAdmin = (ZCharacter*) m_CharacterManager.Find( uidAdmin );
	ZCharacter* pVictim = (ZCharacter*) m_CharacterManager.Find( uidVictim );

	if( !pAdmin || !pVictim )
		return;

	if( !pAdmin->GetInitialized() || !pAdmin->IsVisible() || !pVictim->GetInitialized() || !pVictim->IsVisible() )
		return;

	rvector pos= pVictim->GetPosition(), dir=pVictim->GetDirection();

	//I realise this is a dirty way of doing it, and relying on GunZ to update the position
	// but if you want to keep the ranomized spawn location, this is the only logical way
	// Short of posting ZPostSpawn, anyway.
	if (uidVictim == m_pMyCharacter->GetUID())
	{
		ZMapSpawnData* pSpawnData = ZGetGame()->GetMapDesc()->GetSpawnManager()->GetSoloRandomData();
		if (pSpawnData != NULL)
		{
			pos = pSpawnData->m_Pos;
			dir = pSpawnData->m_Dir;
		}
	}

	OnPeerSpawn( pVictim->GetUID(), pos, dir );

	if (uidVictim == m_pMyCharacter->GetUID())
	{
		ReleaseObserver();
		//ZGetCombatInterface()->SetObserverMode(false);
	}
}

void ZGame::OnPeerShotSp(MUID& uid, float fShotTime, rvector& pos, rvector& dir, int type, MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;		// �� ?E��?E

	pOwnerCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
//	if (uid == ZGetGameClient()->GetUID()) pOwnerCharacter = m_pMyCharacter;

//	_ASSERT(pOwnerCharacter != NULL);
	if (pOwnerCharacter == NULL) return;
	if(!pOwnerCharacter->GetInitialized()) return;
	if(!pOwnerCharacter->IsVisible()) return;

	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if(!pItem) return;

	MMatchItemDesc* pDesc = pItem->GetDesc();
	if( pDesc == NULL ) return;


	// fShotTime �� �� ĳ������ ���� �ð��̹Ƿ� �� �ð����� ��ȯ���ش�
	fShotTime -= pOwnerCharacter->m_fTimeOffset;

	/*
	float fCurrentTime = g_pGame->GetTime();
	if( abs(fCurrentTime - fShotTime) > TIME_ERROR_BETWEEN_RECIEVEDTIME_MYTIME )
	{
#ifdef _DEBUG
		mlog("!!!!����ź ��!!!! ĳ���� ����: %s      fShotTime : %f     fCurrentTime : %f \n", 
			pOwnerCharacter->GetUserName(), (fShotTime - pOwnerCharacter->m_fTimeOffset) , fCurrentTime);
#endif
		return;
	}
	�� �κ��� �ٿ��� shot�� �� �ð��� �����Ͽ� ������ ���� �����Ͽ� ���� ���� �ڵ忴�µ� �޴� �ʿ��� �ð� �˻縦 ��?E��?E
	������ �ʿ��� �˻縦 �ؼ� shot�� �� �ð��� ��?Eĳ������ lacal time?E��?E����?E�ƿ� ��Ŷ�� ����?E�ʵ��� �ٲ��. 
	���� ��?E�ڵ尡 ��?E���� ��. ���� localtime�� ������ ��?E??E��?�ּ�ó���� ���ܵ�..
	*/
 
	//��?E���߹� ?E�E�Լ���?E������ ������� ��ź���� ���� ?E�ִ� ������ �ƴ�?E�����Ѵ�. 
	if( sel_type != MMCIP_PRIMARY && sel_type != MMCIP_SECONDARY && sel_type != MMCIP_CUSTOM1 && sel_type != MMCIP_CUSTOM2)
		return;

	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if( parts != pOwnerCharacter->GetItems()->GetSelectedWeaponParts()) { ///< ���� ��?E�ִ� ����� ����?E���Ⱑ Ʋ����?E����?E����� �ٲ��ش�..		
		OnChangeWeapon(uid,parts);
	}

	//�� ������ ���� ��?EŸ���� ��..
	MMatchWeaponType nType = pDesc->m_nWeaponType.Ref();
	//��?E�ִ� ���Ⱑ ���� Ÿ���ε� 
	if(nType == MWT_ROCKET) {
		if( type != ZC_WEAPON_SP_ROCKET){	//type�� ������ �ƴ�?E�̽� ��ġ....�����Ѵ�. 		
			return;
		}
	} else if( nType == MWT_MED_KIT || nType == MWT_REPAIR_KIT || nType == MWT_BULLET_KIT || nType == MWT_FOOD || nType == MWT_LANDMINE_SPY || nType == MWT_LANDMINE ) {
		if(type != ZC_WEAPON_SP_ITEMKIT) {
			return;
		}
	} else if( nType == MWT_FLASH_BANG) {
		if( type != ZC_WEAPON_SP_FLASHBANG) {
			return;
		}
	} else if( nType == MWT_FRAGMENTATION) {
		if( type != ZC_WEAPON_SP_GRENADE) {
			return;
		}
	} else if( nType == MWT_SMOKE_GRENADE) {
		if( type != ZC_WEAPON_SP_SMOKE) {
			return;
		}
	} else if( nType == MWT_POTION ) {
		if( type != ZC_WEAPON_SP_POTION ) {
			return;
		}
	} else if( nType == MWT_TRAP) {
		if( type != ZC_WEAPON_SP_TRAP ) {
			return;
		}
	} else if( nType == MWT_DYNAMITYE ) {
		if( type != ZC_WEAPON_SP_DYNAMITE ) {
			return;
		}
	}
	///// SPY MODE /////
 else if (nType == MWT_FLASH_BANG_SPY) {
 if (type != ZC_WEAPON_SP_FLASHBANG_SPY) {
	 return;
 }
	}
 else if (nType == MWT_SMOKE_GRENADE_SPY) {
 if (type != ZC_WEAPON_SP_SMOKE_SPY) {
	 return;
 }
	}
 else if (nType == MWT_TRAP_SPY) {
 if (type != ZC_WEAPON_SP_TRAP_SPY) {
	 return;
 }
	}
 else if (nType == MWT_STUN_GRENADE_SPY) {
 if (type != ZC_WEAPON_SPY_STUNGRENADE) {
	 return;
 }
	}
	else {
		return;
	}

	// ���������� �߻�ӵ��� �����Ѵ�.
	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem)) {
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	} else {
		return;
	}

	if (uid == ZGetMyUID()) {
		int nCurrMagazine = pItem->GetBulletCurrMagazine();
		if (!pItem->Shot()) return;

		if (!(pItem->GetBulletCurrMagazine() < nCurrMagazine)) {
			if(sel_type != MMCIP_MELEE) ZGetApplication()->Exit();
		}		
	} else {
		if (!pItem->Shot()) return;
	}

#ifdef _RADAR
	if (strstr(ZGetGameClient()->GetChannelName(), "Ladder"))
	{
		if (type == ZC_WEAPON_SP_ROCKET || ZC_WEAPON_SP_GRENADE)
			ZGetCombatInterface()->GetRadar()->OnAttack(pos, uid);
	}
#endif

	rvector velocity;
	rvector _pos;

	bool dLight = true;
	bool bSpend = false;

	switch(type)
	{
	case ZC_WEAPON_SP_GRENADE : 
		{
			//static RealSoundEffectSource* pSES	= ZGetSoundEngine()->GetSES("we_grenade_fire");
			//if( pSES != NULL )
			//{
			//	ZGetSoundEngine()->PlaySE( pSES, pos.x, pos.y, pos.z, pOwnerCharacter == m_pMyCharacter );
			//}
			bSpend = true;

			velocity	= pOwnerCharacter->GetVelocity()+pOwnerCharacter->m_TargetDir*1200.f;
			velocity.z	+= 300.f;
			m_WeaponManager.AddGrenade(pos, velocity, pOwnerCharacter);
			//m_WeaponManager.AddFlashBang( pos - rvector(10,10,10), velocity, pOwnerCharacter );
			//m_WeaponManager.AddSmokeGrenade( pos + rvector(10,10,10), velocity, pOwnerCharacter );
		}
		break;

	case ZC_WEAPON_SP_ROCKET : 
		{
			m_WeaponManager.AddRocket(pos, dir, pOwnerCharacter);
			if(Z_VIDEO_DYNAMICLIGHT) {
				ZGetStencilLight()->AddLightSource( pos, 2.0f, 100 );
			}
		}
		break;

	case ZC_WEAPON_SP_FLASHBANG:
		{
			bSpend = true; 

			velocity	= pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir*1200.f;
			velocity.z	+= 300.0f;
			m_WeaponManager.AddFlashBang(pos,velocity,pOwnerCharacter);
			dLight	= false;
		}		
		break;

	case ZC_WEAPON_SP_SMOKE:
		{
			bSpend = true;

			velocity	= pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir*1200.f;
			velocity.z	+= 300.0f;
			m_WeaponManager.AddSmokeGrenade(pos, velocity, pOwnerCharacter);
			dLight	= false;
		}		
		break;

	case ZC_WEAPON_SP_TEAR_GAS:
		{
			bSpend = true;
			dLight	= false;
		}		
		break;

	case ZC_WEAPON_SP_ITEMKIT: 
		{
			int nLinkedWorldItem = ZGetWorldItemManager()->GetLinkedWorldItemID(pItem->GetDesc());

			velocity	= dir;
			_pos = pos;

			m_WeaponManager.AddKit(_pos,velocity, pOwnerCharacter, 0.2f, pItem->GetDesc()->m_pMItemName->Ref().m_szMeshName, nLinkedWorldItem);
			dLight	= false;
		}
		break;

	case ZC_WEAPON_SP_POTION :
		{			
			ApplyPotion(pItem->GetDescID(), pOwnerCharacter, 0);
		}
		break;

	case ZC_WEAPON_SP_TRAP:
		{
			OnUseTrap(pItem->GetDescID(), pOwnerCharacter, pos);
			dLight = true;
		}
		break;	

	case ZC_WEAPON_SP_DYNAMITE:
		{
			OnUseDynamite(pItem->GetDescID(), pOwnerCharacter, pos);
			dLight = true;
		}
		break;	

	case ZC_WEAPON_SP_FLASHBANG_SPY:
		{
			bSpend = true; 

			velocity	= pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir*1200.f;
			velocity.z	+= 300.0f;
			m_WeaponManager.AddSpyFlashBang(pos,velocity,pOwnerCharacter);
			dLight	= false;
		}		
		break;

		//////////// SPY ////////////
	case ZC_WEAPON_SP_SMOKE_SPY:
		{
			bSpend = true;

			velocity	= pOwnerCharacter->GetVelocity() + pOwnerCharacter->m_TargetDir*1200.f;
			velocity.z	+= 300.0f;
			m_WeaponManager.AddSpySmokeGrenade(pos, velocity, pOwnerCharacter);
			dLight	= false;
		}		
		break;

	case ZC_WEAPON_SP_TRAP_SPY:
		{
			OnUseSpyTrap(pItem->GetDescID(), pOwnerCharacter, pos);
			dLight = true;
		}
		break;	

	case ZC_WEAPON_SPY_STUNGRENADE:
		{
			OnUseStunGrenade(pItem->GetDescID(), pOwnerCharacter, pos);
			dLight = true;
		}
		break;

	default:
		_ASSERT(0);
		break;
	}

#ifdef _REPLAY_TEST_LOG
	if(type == ZC_WEAPON_SP_POTION || type == ZC_WEAPON_SP_ITEMKIT)
	{
		for(int i=0; i<16; ++i)
		{
			if(m_Replay_UseItem[i].uid.Low == 0)
			{	// uid ��?E?����?E
				m_Replay_UseItem[i].uid = uid;
				m_Replay_UseItem[i].Item[0].Itemid = pDesc->m_nID;
				m_Replay_UseItem[i].Item[0].ItemUseCount++;
				break;
			}
			if(m_Replay_UseItem[i].uid == uid)
			{	// uid�� ����?E
				for(int j=0; j<5; ++j)
				{
					if(m_Replay_UseItem[i].Item[j].Itemid == 0)
					{	// itemid ��?E?����?E
						m_Replay_UseItem[i].uid = uid;
						m_Replay_UseItem[i].Item[j].Itemid = pDesc->m_nID;
						m_Replay_UseItem[i].Item[j].ItemUseCount++;
						break;
					}
					if(m_Replay_UseItem[i].Item[j].Itemid == pDesc->m_nID)
					{
						m_Replay_UseItem[i].Item[j].ItemUseCount++;
						break;
					}
				}
				break;
			}
		}
		//mlog("[%s(%d) : %s(%d)]\n", pOwnerCharacter->GetCharInfo()->szName, uid.Low, pDesc->m_pMItemName->Ref().m_szItemName, pDesc->m_nID);
	}
#endif

	// ���Ƿ��� �׳� �ϵ��ڵ����� ������ �Դ� ����带 ���Ը� ����ش�
	// �׳� �Ϲ� ����ó�� ó����?E�߻������� ���ֵǾ�ح �ֺ�����鿡�� �鸮�� �Ǵµ�,
	// ������ �Դ� ����尡 2d������ ?E?E��?�鸮�� ��?E?E��?
	if (type==ZC_WEAPON_SP_POTION)
	{
		if( pOwnerCharacter == ZGetGame()->m_pMyCharacter ) {
			ZGetSoundEngine()->PlaySound("fx_itemget");
		}
	}
	else
	{
		ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter==m_pMyCharacter));
	}
	
	if( dLight )
	{
		// �� �� ����Ʈ �߰�
		ZCharacter* pChar;

		if( ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL )	{

			pChar = pOwnerCharacter;

			if( pChar->m_bDynamicLight ) {

				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;

			} else {

				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[CANNON].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = CANNON;
				pChar->m_fLightLife = g_CharLightList[CANNON].fLife;
			}

			if( pOwnerCharacter->IsHero() )
			{
				RGetDynamicLightManager()->AddLight( GUNFIRE, pos );
			}
		}
	}

	if (ZGetMyUID() == pOwnerCharacter->GetUID())
	{
		ZItem* pSelItem = pOwnerCharacter->GetItems()->GetSelectedWeapon();
		if( pSelItem && pSelItem->GetDesc() &&
			pSelItem->GetDesc()->IsSpendableItem() ) 
		{
			ZMyItemNode* pItemNode = ZGetMyInfo()->GetItemList()->GetEquipedItem((MMatchCharItemParts)sel_type);
			if( pItemNode ) 
			{
				pItemNode->SetItemCount(pItemNode->GetItemCount() - 1);
				ZPostRequestUseSpendableNormalItem(pItemNode->GetUID());
			}
		}
	}
}

bool ZGame::CheckWall(ZObject* pObj1,ZObject* pObj2, bool bCoherentToPeer)
{
	//### �� �Լ��� ������?E�Ȱ��� IsWallBlocked()���� ��?E?�־��� �մϴ�. ###

	// �Ǿ�Ϣ�� ��ǥ�� ������ ����� float->short ĳ������ �Ͼ�ϭ�� (?E�Ҽ������� ����)
	// ���� Ŭ���̾�Ʈ���� ���� ������ ���� �̹��ϰ� �ٸ� ���� �ִ�. �̰��� ������ ������ ����Ű?E�ʾ�����
	// �����̹����� ���� Ȯ?E?������ �߻�: npc�� �÷��̾�Ԧ ���������Ϸ�?E�Ҷ�, npc ��Ʈ�ѷ��� ���� �����ϴ�?E����.
	// �ǰݴ��ϴ� Ŭ���̾�Ʈ�� ���� ������?E�ʴ�?E����. �̷�?E�ǰݵǴ� ������ ��ġ�� �ٲ�?E�ʴ��� ���ʹ� ���ڸ����� ���� ?E��?ġ�Ե� (�ֱ�Ի ��?E�Ұ���?E���������� �ۺ�Ԯ���� �ټ��� ����)
	// bCoherentToPeer==true �϶� �Ǿ�ۡ�� ���� ��?E���� ���� ��?E?.

	if( (pObj1==NULL) || (pObj2==NULL) )
		return false;

	if( (pObj1->GetVisualMesh()==NULL) || (pObj2->GetVisualMesh()==NULL) )
		return false;

	// ���ϸ��̼� ������ ���� ��?E���̡�� ��?E?�־�ح..
	rvector p1 = pObj1->GetPosition() + rvector(0.f,0.f,100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f,0.f,100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ������ ���� ���� ��?E�׽�Ʈ�� ���� ��?E���� 1�� �ڸ���?E������ ��?E
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if( Pick( pObj1 , p1 , dir, &pickinfo ) ) {//���̶�?E
		if(pickinfo.bBspPicked)//���� �ɸ���?E
			return true;
	}

	return false;
}
//jintriple3 ����� �������� ��ŷ.....�񱳸� ���ؼ�
bool ZGame::CheckWall(ZObject* pObj1,ZObject* pObj2, int & nDebugRegister/*��?E��?E*/, bool bCoherentToPeer)
{	//bCoherentToPeer�� ?EѰ��?���� CheckWall �ּ� ?E�E

	if( (pObj1==NULL) || (pObj2==NULL) )
	{
		nDebugRegister = -10;	//���ó� ���ڴ� �ǹ̰� ����..
		return false;
	}

	if( (pObj1->GetVisualMesh()==NULL) || (pObj2->GetVisualMesh()==NULL) )
	{
		nDebugRegister = -10;
		return false;
	}

	// ���ϸ��̼� ������ ���� ��?E���̡�� ��?E?�־�ح..
	rvector p1 = pObj1->GetPosition() + rvector(0.f,0.f,100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f,0.f,100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ������ ���� ���� ��?E�׽�Ʈ�� ���� ��?E���� 1�� �ڸ���?E������ ��?E
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if( Pick( pObj1 , p1 , dir, &pickinfo ) ) {//���̶�?E
		if(pickinfo.bBspPicked)//���� �ɸ���?E
		{
			nDebugRegister = FOR_DEBUG_REGISTER;
			return true;
		}
	}
	nDebugRegister = -10;
	return false;
}
bool ZGame::IsWallBlocked(ZObject* pObj1, ZObject* pObj2, bool bCoherentToPeer)
{
	//### �� �Լ��� �����ϸ� �Ȱ��� IsWallBlocked()���� ������ �־�� �մϴ�. ###

	// �Ǿ�� ��ǥ�� ������ ����� float->short ĳ������ �Ͼ�� (�� �Ҽ������� ����)
	// ���� Ŭ���̾�Ʈ���� ���� ������ ���� �̹��ϰ� �ٸ� ���� �ִ�. �̰��� ������ ������ ����Ű�� �ʾ�����
	// �����̹����� ���� Ȯ���� ������ �߻�: npc�� �÷��̾ ���������Ϸ��� �Ҷ�, npc ��Ʈ�ѷ��� ���� �����ϴٰ� ����.
	// �ǰݴ��ϴ� Ŭ���̾�Ʈ�� ���� �������� �ʴٰ� ����. �̷ν� �ǰݵǴ� ������ ��ġ�� �ٲ��� �ʴ��� ���ʹ� ���ڸ����� ���� ����� ġ�Ե� (�ֱ �ǿ� �Ұ���� ���������� �ۺ����� �ټ��� ����)
	// bCoherentToPeer==true �϶� �Ǿ�� ���� �Ͱ� ���� ���� �����..

	if ((pObj1 == NULL) || (pObj2 == NULL))
		return false;

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
		return false;

	// ���ϸ��̼� ������ ���� �հ� ���� ��쵵 �־..
	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ������ ���� ���� ���� �׽�Ʈ�� ���� �ϱ� ���� 1�� �ڸ����� ������ ����
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {//���̶��
		if (pickinfo.bBspPicked)//���� �ɸ����
			return true;
	}

	return false;
}
//jintriple3 ����� �������� ��ŷ.....�񱳸� ���ؼ�
bool ZGame::IsWallBlocked(ZObject* pObj1, ZObject* pObj2, int& nDebugRegister/*�ܼ� �񱳿�*/, bool bCoherentToPeer)
{	//bCoherentToPeer�� ���Ѱ��� ���� CheckWall �ּ� ����

	if ((pObj1 == NULL) || (pObj2 == NULL))
	{
		nDebugRegister = -10;	//���ó� ���ڴ� �ǹ̰� ����..
		return false;
	}

	if ((pObj1->GetVisualMesh() == NULL) || (pObj2->GetVisualMesh() == NULL))
	{
		nDebugRegister = -10;
		return false;
	}

	// ���ϸ��̼� ������ ���� �հ� ���� ��쵵 �־..
	rvector p1 = pObj1->GetPosition() + rvector(0.f, 0.f, 100.f);
	rvector p2 = pObj2->GetPosition() + rvector(0.f, 0.f, 100.f);

	if (bCoherentToPeer)
	{
		p1.x = short(p1.x);
		p1.y = short(p1.y);
		p1.z = short(p1.z);
		p2.x = short(p2.x);
		p2.y = short(p2.y);
		p2.z = short(p2.z);
		// ������ ���� ���� ���� �׽�Ʈ�� ���� �ϱ� ���� 1�� �ڸ����� ������ ����
		/*p1.x = (short(p1.x * 0.1f)) * 10.f;
		p1.y = (short(p1.y * 0.1f)) * 10.f;
		p1.z = (short(p1.z * 0.1f)) * 10.f;
		p2.x = (short(p2.x * 0.1f)) * 10.f;
		p2.y = (short(p2.y * 0.1f)) * 10.f;
		p2.z = (short(p2.z * 0.1f)) * 10.f;*/
	}

	rvector dir = p2 - p1;

	Normalize(dir);

	ZPICKINFO pickinfo;

	if (Pick(pObj1, p1, dir, &pickinfo)) {//���̶��
		if (pickinfo.bBspPicked)//���� �ɸ����
		{
			nDebugRegister = FOR_DEBUG_REGISTER;
			return true;
		}
	}
	nDebugRegister = -10;
	return false;
}
void ZGame::OnExplosionGrenade(MUID uidOwner,rvector pos,float fDamage,float fRange,float fMinDamage,float fKnockBack,MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;

	float fDist,fDamageRange;

	for(ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor) 
	{
		pTarget = (*itor).second;
		//jintriple3 ????? ???????? ??y ???? ????.....
		bool bReturnValue = !pTarget || pTarget->IsDie();
		if( !pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

		fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,80)));
		//jintriple3 ????? ???????? ?? ???? ????.....
		bReturnValue = fDist >=fRange;
		if(fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

		rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
		Normalize(dir);

		// ???? ???? ?�??.
		if(GetDistance(pos,pTarget->GetPosition()+rvector(0,0,50),pTarget->GetPosition()+rvector(0,0,130))<50)
		{
			fDamageRange = 1.f;
		}
		else
		{
#define MAX_DMG_RANGE	50.f	// ??????u ?????? ??? ???????? ?? ??�?
//#define MIN_DMG			0.4f	// ??? ?? ???????? ??????.
			fDamageRange = 1.f - (1.f-fMinDamage)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
		}

		// ????z?? ?????? ??????? ???????.
		ZActor* pATarget = MDynamicCast(ZActor, pTarget);
		ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pTarget);

		bool bPushSkip = false;

		if (pATarget)
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}
		if (pFSMActor)
		{
			bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
		}

		if(!bPushSkip)
		{
			pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);
		}
		else 
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		}

		ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find( uidOwner );
		if(pOwnerCharacter) 
		{
			CheckCombo(pOwnerCharacter, pTarget,!bPushSkip);
			CheckStylishAction(pOwnerCharacter);
		}

		float fActualDamage = fDamage * fDamageRange;
		float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//????z?? ???? ???????..
		pTarget->OnDamaged(pOwnerCharacter,pos,ZD_EXPLOSION,MWT_FRAGMENTATION,fActualDamage,fRatio);
//					pTarget->OnDamagedGrenade( uidOwner, dir, fDamage * fDamageRange, nTeamID);


		/*if(pTarget && !pTarget->IsDie())
		{
			fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,80)));
			if(fDist < fRange) 
			{
				// ?? ????????? ?????? ????????~ 
//				if( CheckWall(pos,pTarget) == false )
				{
					rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
					Normalize(dir);

					// ???? ???? ?�??.
					if(GetDistance(pos,pTarget->GetPosition()+rvector(0,0,50),pTarget->GetPosition()+rvector(0,0,130))<50)
					{
						fDamageRange = 1.f;
					}
					else
					{
#define MAX_DMG_RANGE	50.f	// ??????u ?????? ??? ???????? ?? ??�?
//#define MIN_DMG			0.4f	// ??? ?? ???????? ??????.
						fDamageRange = 1.f - (1.f-fMinDamage)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
					}

					// ????z?? ?????? ??????? ???????.
					ZActor* pATarget = MDynamicCast(ZActor,pTarget);

					bool bPushSkip = false;

					if(pATarget) 
					{
						bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
					}

					if(bPushSkip==false)
					{
						pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);
					}
					else 
					{
						ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
					}

					ZCharacterClass* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
					if(pOwnerCharacter) 
					{
						CheckCombo(pOwnerCharacter, pTarget,!bPushSkip);
						CheckStylishAction(pOwnerCharacter);
					}

					float fActualDamage = fDamage * fDamageRange;
					float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//????z?? ???? ???????..
					pTarget->OnDamaged(pOwnerCharacter,pos,ZD_EXPLOSION,MWT_FRAGMENTATION,fActualDamage,fRatio);
//					pTarget->OnDamagedGrenade( uidOwner, dir, fDamage * fDamageRange, nTeamID);
				}
			}
		}*/
	}

#define SHOCK_RANGE		1500.f			// 10??????? ?????

	ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower= (SHOCK_RANGE-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/SHOCK_RANGE;

	if(fPower>0)
		ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));

	GetWorld()->GetWaters()->CheckSpearing( pos, pos + rvector(0,0,MAX_WATER_DEEP), 500, 0.8f );

//	static RealSoundEffectSource* pSES= ZApplication::GetSoundEngine()->GetSES("explosion");
//	ZApplication::GetSoundEngine()->PlaySE(pSES,pos.x,pos.y,pos.z);
}
void ZGame::OnExplosionMagic(ZWeaponMagic *pWeapon, MUID uidOwner,rvector pos,float fMinDamage,float fKnockBack,MMatchTeam nTeamID,bool bSkipNpc)
{
	ZObject* pTarget = NULL;

	float fRange = 100.f * pWeapon->GetDesc()->fEffectArea;
	float fDist,fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor) 
	{
		pTarget = (*itor).second;
		// ?E������?�ƴ϶�?EŸ�ٸ� �˻��ϸ�ȴ�.
		// ����� �������� ��ŷ ��?E��?E
		bool bForDebugRegister = !pTarget || pTarget->IsDie() || pTarget->IsNPC();
		if( !pTarget || pTarget->IsDie() || pTarget->IsNPC() )
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;
		bForDebugRegister = !pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget()!=pTarget->GetUID();
		if(!pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget()!=pTarget->GetUID()) 
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
			continue;

		
		ZObject* pOwner = ZGetObjectManager()->GetObject(uidOwner);

		bForDebugRegister = !pOwner || pOwner->IsDie();
		if(!pOwner || pOwner->IsDie())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
				return;


		// �� ĳ���ͻ��̿� ��ֹ��� �������Ѵ�~ 
		//				if( CheckWall(pos,pTarget) == false )
		{
			fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,80)));
			if(pWeapon->GetDesc()->IsAreaTarget())	// ?E������̸�E�Ÿ��� ���� �������� ��?EѴ?
			{
				PROTECT_DEBUG_REGISTER(fDist > fRange) continue;	// ?E��?����ϵ��

				// ���� ���� �¾Ҵ�.
				if(GetDistance(pos,pTarget->GetPosition()+rvector(0,0,50),pTarget->GetPosition()+rvector(0,0,130))<50)
				{
					fDamageRange = 1.f;
				}
				else
				{
#define MAX_DMG_RANGE	50.f	// �ݰ��̸�ŭ ������ ��?E�������� �� �Դ´�

					fDamageRange = 1.f - (1.f-fMinDamage)*( max(fDist-MAX_DMG_RANGE,0) / (fRange-MAX_DMG_RANGE));
				}
			}
			else 
			{
				fDamageRange = 1.f;
			}

			// resist �� üũ�Ѵ�
			//����� �������� ��ŷ ��?E��?E����.
			float fDamage = pWeapon->GetDesc()->nModDamage;
			bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget,&fDamage);
			if( !(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage)))
				PROTECT_DEBUG_REGISTER(bForDebugRegister)
				continue;

			ZCharacter* pOwnerCharacter = (ZCharacter*) ZGetGame()->m_CharacterManager.Find( uidOwner );
			if(pOwnerCharacter) 
			{
				CheckCombo(pOwnerCharacter, pTarget,true);
				CheckStylishAction(pOwnerCharacter);
			}

			// ����ź�� ����?E�ݵ����� Ƣ��Ϫ����.
			rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
			Normalize(dir);
			pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);

			float fActualDamage = fDamage * fDamageRange;
			float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//����ź?E���� ���о���..
			
			if(m_pMyCharacter && pTarget->GetUID() == m_pMyCharacter->GetUID())
				m_pMyCharacter->OnDamaged(pOwner,pos,ZD_MAGIC,MWT_FRAGMENTATION,fActualDamage,fRatio);
			else
				pTarget->OnDamaged(pOwner,pos,ZD_MAGIC,MWT_FRAGMENTATION,fActualDamage,fRatio);

			// resist �� üũ�Ѵ�
			/*			float fDamage = pWeapon->GetDesc()->nModDamage;
			if(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage)) 
			{
			ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
			if(pOwnerCharacter) 
			{
			CheckCombo(pOwnerCharacter, pTarget,true);
			CheckStylishAction(pOwnerCharacter);
			}

			// ����ź�� ����?E�ݵ����� Ƣ��Ϫ����.
			rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
			Normalize(dir);
			pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);

			float fActualDamage = fDamage * fDamageRange;
			float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//����ź?E���� ���о���..
			pTarget->OnDamaged(pOwnerCharacter,pos,ZD_MAGIC,MWT_FRAGMENTATION,fActualDamage,fRatio);
			} 
			else 
			{
			// TODO: ���׿� ���������� ����Ʈ�� ��������.
			}*/
		}
	}

	/*
	#define SHOCK_RANGE		1500.f			// 10���ͱ�?E��鸰��

	ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	float fPower= (SHOCK_RANGE-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/SHOCK_RANGE;

	if ((fPower>0) && (pWeapon->GetDesc()->bCameraShock))
	ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	*/

	if (pWeapon->GetDesc()->bCameraShock)
	{
		ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		const float fDefaultPower = 500.0f;
		float fShockRange = pWeapon->GetDesc()->fCameraRange;
		float fDuration = pWeapon->GetDesc()->fCameraDuration;
		float fPower= (fShockRange-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/fShockRange;
		fPower *= pWeapon->GetDesc()->fCameraPower;

		if (fPower>0)
		{
			ZGetGameInterface()->GetCamera()->Shock(fPower*fDefaultPower, fDuration, rvector(0.0f, 0.0f, -1.0f));
		}
	}

	GetWorld()->GetWaters()->CheckSpearing( pos, pos + rvector(0,0,MAX_WATER_DEEP), 500, 0.8f );
}



// �������� ������ �������� �ش�
//jintriple3 ����� �������� ��ŷ ��?E��?E����
void ZGame::OnExplosionMagicThrow(ZWeaponMagic *pWeapon, MUID uidOwner,rvector pos,float fMinDamage,float fKnockBack,MMatchTeam nTeamID,bool bSkipNpc, rvector from,rvector to)
{
	ZObject* pTarget = NULL;

	float fDist,fDamageRange;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor) 
	{
		pTarget = (*itor).second;
		// ?E������?�ƴ϶�?EŸ�ٸ� �˻��ϸ�ȴ�.
		// ����� �������� ��ŷ ��?E��?E
		bool bForDebugRegister = !pTarget || pTarget->IsDie() || pTarget->IsNPC();
		if( !pTarget || pTarget->IsDie() || pTarget->IsNPC() )
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
				continue;
		bForDebugRegister = !pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget()!=pTarget->GetUID();
		if(!pWeapon->GetDesc()->IsAreaTarget() && pWeapon->GetTarget()!=pTarget->GetUID()) 
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
				continue;

		ZObject* pOwner = ZGetObjectManager()->GetObject(uidOwner);

		bForDebugRegister = !pOwner || pOwner->IsDie();
		if(!pOwner || pOwner->IsDie())
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
				return;

		fDist = GetDistance(pTarget->GetPosition() +rvector(0,0,80), from, to );

		if( fDist > pWeapon->GetDesc()->fColRadius+ 100)
			continue;
		
		if(pWeapon->GetDesc()->IsAreaTarget())	// ?E������̸�E�Ÿ��� ���� �������� ��?EѴ?
		{
			if( fDist > pWeapon->GetDesc()->fColRadius+ 80 ) // ĳ����ũ�⸦ 160 ������ ��?E���� ġ?E�������� 80 ������ ��?E
			{
				fDamageRange = 0.1f;
			}
			else
			{
				fDamageRange = 1.f -  0.9f * fDist / (pWeapon->GetDesc()->fColRadius+ 80);
			}
		}
		else 
		{  
			fDamageRange = 1.f;
		}

		// resist �� üũ�Ѵ�
		//����� �������� ��ŷ ��?E��?E����.
		float fDamage = pWeapon->GetDesc()->nModDamage;
		bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget,&fDamage);
		if( !(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage)))
			PROTECT_DEBUG_REGISTER(bForDebugRegister)
				continue;

		ZCharacter* pOwnerCharacter = (ZCharacter*) ZGetGame()->m_CharacterManager.Find( uidOwner );
		if(pOwnerCharacter) 
		{
			CheckCombo(pOwnerCharacter, pTarget,true);
			CheckStylishAction(pOwnerCharacter);
		}

		float fActualDamage = fDamage * fDamageRange;
		float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//����ź?E���� ���о���..

		if(m_pMyCharacter && pTarget->GetUID() == m_pMyCharacter->GetUID())
			m_pMyCharacter->OnDamaged(pOwner,pos,ZD_MAGIC,MWT_FRAGMENTATION,fActualDamage,fRatio);
		else
			pTarget->OnDamaged(pOwner,pos,ZD_MAGIC,MWT_FRAGMENTATION,fActualDamage,fRatio);
	}

	if (pWeapon->GetDesc()->bCameraShock)
	{
		ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
		const float fDefaultPower = 500.0f;
		float fShockRange = pWeapon->GetDesc()->fCameraRange;
		float fDuration = pWeapon->GetDesc()->fCameraDuration;
		float fPower= (fShockRange-Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos))/fShockRange;
		fPower *= pWeapon->GetDesc()->fCameraPower;
		
		if (fPower>0)
		{
			ZGetGameInterface()->GetCamera()->Shock(fPower*fDefaultPower, fDuration, rvector(0.0f, 0.0f, -1.0f));
		}
	}

	GetWorld()->GetWaters()->CheckSpearing( pos, pos + rvector(0,0,MAX_WATER_DEEP), 500, 0.8f );
}

//����� �������� ��ŷ ��?E��?E����
void ZGame::OnExplosionMagicNonSplash(ZWeaponMagic *pWeapon, MUID uidOwner, MUID uidTarget, rvector pos, float fKnockBack)
{
	ZObject* pTarget = m_CharacterManager.Find( uidTarget );
	bool bForDebugRegister = pTarget == NULL || pTarget->IsNPC();
	if (pTarget == NULL || pTarget->IsNPC()) PROTECT_DEBUG_REGISTER(bForDebugRegister) return;
	
	bForDebugRegister = !pTarget || pTarget->IsDie();
	if(!pTarget || pTarget->IsDie())
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
			return;


	ZObject* pOwner = ZGetObjectManager()->GetObject(uidOwner);

	bForDebugRegister = !pOwner || pOwner->IsDie();
	if(!pOwner || pOwner->IsDie())
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
			return;

	// resist �� üũ�Ѵ�
	float fDamage = pWeapon->GetDesc()->nModDamage;
	bForDebugRegister = pWeapon && pWeapon->GetDesc()->CheckResist(pTarget,&fDamage);
	if( !pWeapon->GetDesc()->CheckResist(pTarget, &fDamage))
		PROTECT_DEBUG_REGISTER(bForDebugRegister)
			return;

	ZCharacter* pOwnerCharacter = (ZCharacter*) ZGetGame()->m_CharacterManager.Find( uidOwner );
	if(pOwnerCharacter) 
	{
		CheckCombo(pOwnerCharacter, pTarget,true);
		CheckStylishAction(pOwnerCharacter);
	}
	// �ݵ����� Ƣ��Ϫ����.
	rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
	Normalize(dir);
	pTarget->AddVelocity(fKnockBack*7.f*-dir);

	float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//����ź?E���� ���о���..
	if(m_pMyCharacter && pTarget->GetUID() == m_pMyCharacter->GetUID())
		m_pMyCharacter->OnDamaged(pOwner,pos,ZD_MAGIC,MWT_FRAGMENTATION,fDamage,fRatio);
	else
		pTarget->OnDamaged(pOwner,pos,ZD_MAGIC,MWT_FRAGMENTATION,fDamage,fRatio);

/*	if(pTarget && !pTarget->IsDie()) {

		// resist �� üũ�Ѵ�
		float fDamage = pWeapon->GetDesc()->nModDamage;
		if(pWeapon->GetDesc()->CheckResist(pTarget,&fDamage)) 
		{
			ZCharacter* pOwnerCharacter = g_pGame->m_CharacterManager.Find( uidOwner );
			if(pOwnerCharacter) {
				CheckCombo(pOwnerCharacter, pTarget,true);
				CheckStylishAction(pOwnerCharacter);
			}

			// �ݵ����� Ƣ��Ϫ����.
			rvector dir=pos-(pTarget->GetPosition()+rvector(0,0,80));
			Normalize(dir);
			pTarget->AddVelocity(fKnockBack*7.f*-dir);

			float fRatio = ZItem::GetPiercingRatio( MWT_FRAGMENTATION , eq_parts_chest );//����ź?E���� ���о���..
			pTarget->OnDamaged(pOwnerCharacter,pos,ZD_MAGIC,MWT_FRAGMENTATION,fDamage,fRatio);
		} else {
			// TODO: ���׿� ���������� ����Ʈ�� ��������.
		}
	}*/
}

int ZGame::SelectSlashEffectMotion(ZCharacter* pCharacter)
{
	// Custom: Exploit fix (pCharacter->GetSelectItemDesc() == NULL crash)
	if(pCharacter==NULL || pCharacter->GetSelectItemDesc() == NULL) return SEM_None;

	// ���డ ���������� Ȥ�� �� �ٲ�?E�𸣴� ���д�~~

//	MWT_DAGGER
//	MWT_DUAL_DAGGER
//	MWT_KATANA
//	MWT_GREAT_SWORD
//	MWT_DOUBLE_KATANA

	ZC_STATE_LOWER lower = pCharacter->m_AniState_Lower.Ref();

	int nAdd = 0;
	int ret = 0;

	MMatchWeaponType nType = pCharacter->GetSelectItemDesc()->m_nWeaponType.Ref();

	if(pCharacter->IsMan()) {

			 if(lower == ZC_STATE_LOWER_ATTACK1) {	nAdd = 0;	}
		else if(lower == ZC_STATE_LOWER_ATTACK2) {	nAdd = 1;	}
		else if(lower == ZC_STATE_LOWER_ATTACK3) {	nAdd = 2;	}
		else if(lower == ZC_STATE_LOWER_ATTACK4) {	nAdd = 3;	}
		else if(lower == ZC_STATE_LOWER_ATTACK5) {	nAdd = 4;	}
		else if(lower == ZC_STATE_LOWER_UPPERCUT) {	return SEM_ManUppercut;	}

			 if(nType == MWT_KATANA )		return SEM_ManSlash1 + nAdd;
		else if(nType == MWT_DOUBLE_KATANA)	return SEM_ManDoubleSlash1 + nAdd;
		else if(nType == MWT_GREAT_SWORD)	return SEM_ManGreatSwordSlash1 + nAdd;
		else if(nType == MWT_SPYCASE)		return SEM_ManSlash1 + nAdd;

	}
	else {

			 if(lower == ZC_STATE_LOWER_ATTACK1) {	nAdd = 0;	}
		else if(lower == ZC_STATE_LOWER_ATTACK2) {	nAdd = 1;	}
		else if(lower == ZC_STATE_LOWER_ATTACK3) {	nAdd = 2;	}
		else if(lower == ZC_STATE_LOWER_ATTACK4) {	nAdd = 3;	}
		else if(lower == ZC_STATE_LOWER_ATTACK5 ) {	nAdd = 4;	}
		else if(lower == ZC_STATE_LOWER_UPPERCUT) {	return SEM_WomanUppercut;	}

			 if(nType == MWT_KATANA )		return SEM_WomanSlash1 + nAdd;
		else if(nType == MWT_DOUBLE_KATANA)	return SEM_WomanDoubleSlash1 + nAdd;
		else if(nType == MWT_GREAT_SWORD)	return SEM_WomanGreatSwordSlash1 + nAdd;
		else if (nType == MWT_SPYCASE)		return SEM_WomanSlash1 + nAdd;
	}

	return SEM_None;
}

// shot �� �ʹ� Ŀ�� �и�..
void ZGame::OnPeerShot_Melee(const MUID& uidOwner, float fShotTime)
{
	// ??? ??? ??
	ZObject *pAttacker = m_ObjectManager.GetObject(uidOwner);
	float time = fShotTime;
	//jintriple3 ??? ???? ? ??? ??
	bool bReturnValue = !pAttacker;
	if (!pAttacker)
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;


	// Melee ??? ?E???? ??
	ZItem *pItem = pAttacker->GetItems()->GetItem(MMCIP_MELEE);
	//jintriple3    ??? ????? ? ??E..
	bReturnValue = !pItem;
	if (!pItem)
		PROTECT_DEBUG_REGISTER(bReturnValue)
		return;

	MMatchItemDesc *pItemDesc = pItem->GetDesc();
	//jintriple3    ??Eassert? ??? ?...?? ??E.
	bReturnValue = !pItemDesc;
	if (!pItemDesc)
		PROTECT_DEBUG_REGISTER(bReturnValue)
	{
		_ASSERT(FALSE);
		return;
	}


	// ?? ??? ?E?????.
	float fRange = pItemDesc->m_nRange.Ref();
	if (fRange == 0)
		fRange = 150.f;

	// ?? ??? ??? ???.
	float fAngle = cosf(ToRadian(pItemDesc->m_nAngle.Ref() * 0.5f));

	// NPC? ??E??? ?? ?E?????.
	if (pAttacker->IsNPC())
	{
		fRange += fRange * 0.2f;            // ??? 20% ??
		fAngle -= fAngle * 0.1f;            // ??? 10% ??
	}


	// ?? ??? ?E???? ??
	fShotTime = GetTime();


	// ???E??
	//ZGetSoundEngine()->PlaySound( "blade_swing", pAttacker->GetPosition(), uidOwner==ZGetGameInterface()->GetCombatInterface()->GetTargetUID());


	// ???? ??? ??? ?E????
	rvector AttackerPos = pAttacker->GetPosition();
	rvector AttackerNorPos = AttackerPos;
	AttackerNorPos.z = 0;

	rvector AttackerDir = pAttacker->m_Direction;
	rvector AttackerNorDir = AttackerDir;
	AttackerNorDir.z = 0;
	Normalize(AttackerNorDir);


	// ??E? ???? ??? ?????
	int cm = 0;
	ZCharacter *pOwnerCharacter = (ZCharacter*)m_CharacterManager.Find(uidOwner);
	if (pOwnerCharacter)
		cm = SelectSlashEffectMotion(pOwnerCharacter);


	// ???E??
	//if ( !pAttacker->IsNPC())
	//{
	//  ZGetSoundEngine()->PlaySound( "blade_swing", pAttacker->m_Position, uidOwner==ZGetGameInterface()->GetCombatInterface()->GetTargetUID());
	//}

	// ???E??
	bool bPlayer = false;
	rvector Pos = pAttacker->GetPosition();
	if (pAttacker == m_pMyCharacter)
	{
		Pos = RCameraPosition;
		bPlayer = true;
	}

	// default
	ZApplication::GetSoundEngine()->PlaySoundElseDefault("blade_swing", "blade_swing", rvector(Pos.x, Pos.y, Pos.z), bPlayer);


	// ??? ??? ?? ???E????.
	bool bHit = false;



	// ??? ?E???? ????.
	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		// ??? ?E???? ???.
		ZObject* pVictim = (*itor).second;

		// ??? ?? ???E?? ???? ????.
		//jintriple3    ??? ???? ? ??E...
		ZModule_HPAP* pModule = (ZModule_HPAP*)pVictim->GetModule(ZMID_HPAP);
		if (pVictim->IsDie())
			PROTECT_DEBUG_REGISTER(pModule->GetHP() == 0)
			continue;

		// ??? ??? ????E?? ???? ????.
		//jintriple3    ??? ???? ? ??E...
		bReturnValue = pAttacker == pVictim;
		if (pAttacker == pVictim)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		// ?? ??? ??E????E?? ???? ????.
		//jintriple3    ??? ???? ? ??E...
		bool bRetVal = CanAttack(pAttacker, pVictim);
		if (!bRetVal)
			PROTECT_DEBUG_REGISTER(!CanAttack_DebugRegister(pAttacker, pVictim))
			continue;



		// ??? ??? ???.
		rvector VictimPos, VictimDir, VictimNorDir;
		rvector ZeroVector = rvector(0.0f, 0.0f, 0.0f);
		VictimPos = ZeroVector;
		bRetVal = pVictim->GetHistory(&VictimPos, &VictimDir, fShotTime);
		//jintriple3    ??? ???? ? ??E...
		if (!bRetVal)
			PROTECT_DEBUG_REGISTER(VictimPos == ZeroVector)
			continue;


		// NPC? ?? ??? ??? ?...
		if (!pAttacker->IsNPC())
		{
			// ???? ???? ??? ???.
			rvector swordPos = AttackerPos + (AttackerNorDir * 100.f);
			swordPos.z += pAttacker->GetCollHeight() * .5f;
			float fDist = GetDistanceLineSegment(swordPos, VictimPos, VictimPos + rvector(0, 0, pVictim->GetCollHeight()));


			// ???? ???? ??? ?? ?? ???? ??E?? ???? ????.
			//jintriple3    ??? ???? ?? ??E...
			bReturnValue = fDist > fRange;
			if (fDist > fRange)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;


			// ???? ?E???? ??? ??? ???
			rvector VictimNorDir = VictimPos - (AttackerPos - (AttackerNorDir * 50.f));
			Normalize(VictimNorDir);


			// ??? ?? ??? ?? ???E?? ???? ????.
			//jintriple3    ??? ???? ? ??E...
			float fDot = D3DXVec3Dot(&AttackerNorDir, &VictimNorDir);
			bReturnValue = fDot < 0.5f;
			if (fDot < 0.5f)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;
		}

		// ??? ?? NPC? ??? ?...  (??? ???? ??E?? ??? ??? ??? ???? ????)
		// (??????? ??? ?? ZActorAction?? ????? ??)
		else
		{
			// ???? ?E???? ??? ??E???
			rvector VictimNorPos = VictimPos;
			VictimNorPos.z = 0;

			rvector VictimNorDir = VictimPos - (AttackerPos - (AttackerNorDir * 50.f));
			VictimNorDir.z = 0;
			Normalize(VictimNorDir);


			// ???? ???? x,y ??E?? ??? ?? ??, ??? ?? ?? ???? ??E?? ???? ????.
			//jintriple3    ??? ???? ? ??E...
			float fDist = Magnitude(AttackerNorPos - VictimNorPos);
			bReturnValue = fDist > fRange;
			if (fDist > fRange)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			// ??? ?? ??? ?? ???E?? ???? ????.
			float fDot = D3DXVec3Dot(&AttackerNorDir, &VictimNorDir);
			//jintriple3    ??? ???? ? ??E...
			bReturnValue = fDot < fAngle;
			if (fDot < fAngle)
				PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

			// ??? ??E?? ?E???? ?? ?? ??? ???E????.
			//jintriple3    ??? ???? ? ??E...
			int nDebugRegister = 0;
			if (!InRanged(pAttacker, pVictim, nDebugRegister))
				PROTECT_DEBUG_REGISTER(nDebugRegister != FOR_DEBUG_REGISTER)
				continue;
		}


		// ???? ?? ??? ?? ??E???E?? ???? ????.
		int nDebugRegister = 0;
		bRetVal = CheckWall(pAttacker, pVictim, nDebugRegister, true);
		//jintriple3 ??? ???? ? ??E...
		if (bRetVal)
			PROTECT_DEBUG_REGISTER(nDebugRegister == FOR_DEBUG_REGISTER)
			continue;


		// ??E????? ??E??? ???E??E??E
		//jintriple3 ??? ???? ? ??E..
		bRetVal = pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0);
		if (pVictim->IsGuard() && (DotProduct(pVictim->m_Direction, AttackerNorDir) < 0))
		{
			PROTECT_DEBUG_REGISTER(bRetVal)
			{
				rvector pos = pVictim->GetPosition();
				pos.z += 120.f;

				// ??E??? ??
				ZGetEffectManager()->AddSwordDefenceEffect(pos + (pVictim->m_Direction * 50.f), pVictim->m_Direction);
				pVictim->OnMeleeGuardSuccess();
				return;
			}
		}

		// ?E?�E???? ??? ????E..
		rvector pos = pVictim->GetPosition();
		pos.z += 130.f;
		pos -= AttackerDir * 50.f;


		// ???? ????.
		ZGetEffectManager()->AddBloodEffect(pos, -VictimNorDir);
		ZGetEffectManager()->AddSlashEffect(pos, -VictimNorDir, cm);


		// ?????? ???? ?????.
		float fActualDamage = CalcActualDamage( pAttacker, pVictim, (float)pItemDesc->m_nDamage.Ref());
		float fRatio = pItem->GetPiercingRatio( pItemDesc->m_nWeaponType.Ref() , eq_parts_chest );
		pVictim->OnDamaged( pAttacker, pAttacker->GetPosition(), ZD_MELEE, pItemDesc->m_nWeaponType.Ref(),
				fActualDamage, fRatio, cm);

		ZActor* pATarget = MDynamicCast(ZActor, pVictim);
		ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pVictim);

		ZPOSTDMGTAKEN(MCommandParameterInt(fActualDamage), MCommandParameterInt(0), pAttacker->GetUID()); // Custom: Fix Damage Count Melee
		// ?? ??? ?? ??
		bool bPushSkip = false;
		if (pATarget)
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}
		if (pFSMActor)
		{
			bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
		}

		float fKnockbackForce = pItem->GetKnockbackForce();
		if (bPushSkip)
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
			fKnockbackForce = 1.0f;
		}

		pVictim->OnKnockback(pAttacker->m_Direction, fKnockbackForce);


		// ??? ???E??
		ZGetSoundEngine()->PlaySoundBladeDamage(pItemDesc, pos);
#ifdef _ZPEERANTIHACK
		if (pVictim == m_pMyCharacter && pAttacker->IsNPC() == false)
			ZPeerAntiHackAdd(pAttacker->GetUID(), Melee, pItem->GetDesc()->m_nDelay.Ref(), MMCIP_PRIMARY, pItem->GetDesc()->m_nWeaponType.Ref());
#endif

		// ?? ????E?? ??? ???? ??.
		if (pAttacker == m_pMyCharacter)
		{
			CheckCombo(m_pMyCharacter, pVictim, !bPushSkip);
			CheckStylishAction(m_pMyCharacter);
		}


		// ??? ???.
		bHit = true;
	}


	// ???? ???E????E..
	// test ???? ??E?????E???..
	if (!bHit)
	{

		rvector vPos = pAttacker->GetPosition();
		rvector vDir = AttackerNorDir;

		vPos.z += 130.f;

		RBSPPICKINFO bpi;

		if (GetWorld()->GetBsp()->Pick(vPos, vDir, &bpi)) {

			float fDist = Magnitude(vPos - bpi.PickPos);

			if (fDist < fRange) {

				rplane r = bpi.pInfo->plane;
				rvector vWallDir = rvector(r.a, r.b, r.c);
				Normalize(vWallDir);

				ZGetEffectManager()->AddSlashEffectWall(bpi.PickPos - (vDir*5.f), vWallDir, cm);

				rvector pos = bpi.PickPos;

				ZGetSoundEngine()->PlaySoundBladeConcrete(pItemDesc, pos);
			}
		}
	}

	return;
}

bool ZGame::InRanged( ZObject* pAttacker, ZObject* pVictim)
{
	//### �� �Լ��� ������?E�Ȱ��� �Ʒ��� InRanged()���� ��?E?�־��� �մϴ�. ###

	// �����ڿ� Ÿ�� �Ǹ����� �ٴ�?E������ ��ġ�� ���Ѵ�.
	float fBotAtk	= pAttacker->GetPosition().z;
//	float fTopAtk	= fBotAtk + pAttacker->GetCollHeight();
	float fTopAtk	= fBotAtk + pAttacker->GetCollHeight() + (pAttacker->GetCollHeight() * 0.5f);

	float fBotVct	= pVictim->GetPosition().z;
	float fTopVct	= fBotVct + pVictim->GetCollHeight();


	// Ÿ���� �� �Ʒ��� �����ں��� ���� ����?E���� ���̴�.
	if ( fBotVct > fTopAtk)
		return false;

	// Ÿ���� �� ���� �����ں��� �Ʒ��� ����?E���� ���̴�.
	else if ( fTopVct < fBotAtk)
		return false;

	// �� �ܿ��� ?E?���� ���̴�.
	return true;
}

bool ZGame::InRanged( ZObject* pAttacker, ZObject* pVictim, int &nDebugRegister/*����� �������� ��ŷ ������ ���� ��?E*/)
{
	// �����ڿ� Ÿ�� �Ǹ����� �ٴ�?E������ ��ġ�� ���Ѵ�.
	float fBotAtk	= pAttacker->GetPosition().z;
//	float fTopAtk	= fBotAtk + pAttacker->GetCollHeight();
	float fTopAtk	= fBotAtk + pAttacker->GetCollHeight() + (pAttacker->GetCollHeight() * 0.5f);

	float fBotVct	= pVictim->GetPosition().z;
	float fTopVct	= fBotVct + pVictim->GetCollHeight();


	// Ÿ���� �� �Ʒ��� �����ں��� ���� ����?E���� ���̴�.
	if ( fBotVct > fTopAtk)
	{
		nDebugRegister = -10;//���ڴ� �ǹ̰� ����..��?E�񱳸� ���� ?E..
		return false;
	}

	// Ÿ���� �� ���� �����ں��� �Ʒ��� ����?E���� ���̴�.
	else if ( fTopVct < fBotAtk)
	{
		nDebugRegister = -10;
		return false;
	}

	// �� �ܿ��� ?E?���� ���̴�.
	nDebugRegister = FOR_DEBUG_REGISTER;
	return true;
}

//jintriple3 ����� �������� ��ŷ ��?E��?E����
void ZGame::OnPeerShot_Range_Damaged(ZObject* pOwner, float fShotTime, const rvector& pos, const rvector& to, ZPICKINFO pickinfo, DWORD dwPickPassFlag, rvector& v1, rvector& v2, ZItem *pItem, rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType)
{
	MMatchItemDesc *pDesc = pItem->GetDesc();
	bool bReturnValue = !pDesc;
	if (!pDesc) PROTECT_DEBUG_REGISTER(bReturnValue) { _ASSERT(FALSE); return; }

	rvector dir = to - pos;

	bReturnValue = !(ZGetGame()->PickHistory(pOwner, fShotTime, pos, to, &pickinfo, dwPickPassFlag));
	if (!(ZGetGame()->PickHistory(pOwner, fShotTime, pos, to, &pickinfo, dwPickPassFlag)))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			v1 = pos;
			v2 = pos + dir*10000.f;
			nTargetType = ZTT_NOTHING;
			return;
		}
	}

	// ���� -bird
	//jintriple3 ����� �������� ��ŷ ��?E��?E
	bReturnValue = (!pickinfo.pObject) && (!pickinfo.bBspPicked);
	if (pickinfo.bBspPicked)
	{
		PROTECT_DEBUG_REGISTER(pickinfo.nBspPicked_DebugRegister == FOR_DEBUG_REGISTER)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			// ��ź ��?E
			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;
			return;
		}
	}
	else if ((!pickinfo.pObject) && (!pickinfo.bBspPicked))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			_ASSERT(false);
			return;
		}
	}
	//���������� ��?E��?E..

	ZObject *pObject = pickinfo.pObject;
	bool bGuard = pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&     // �ٸ��� ����������
		DotProduct(dir, pObject->GetDirection())<0;

	if (pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&        // �ٸ��� ����������
		DotProduct(dir, pObject->GetDirection())<0) //���⵵..
	{
		PROTECT_DEBUG_REGISTER(bGuard)
		{
			nTargetType = ZTT_CHARACTER_GUARD;
			// ���Ҵ�
			rvector t_pos = pObject->GetPosition();
			t_pos.z += 100.f;
			ZGetEffectManager()->AddSwordDefenceEffect(t_pos + (-dir*50.f), -dir);
			pObject->OnGuardSuccess();
			v1 = pos;
			v2 = pickinfo.info.vOut;
			return;
		}
	}

	nTargetType = ZTT_CHARACTER;

	ZActor* pATarget = MDynamicCast(ZActor, pickinfo.pObject);
	ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pickinfo.pObject);
	bool bPushSkip = false;

	if (pATarget)
	{
		bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
	}
	if (pFSMActor)
	{
		bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
	}

	float fKnockbackForce = pItem->GetKnockbackForce();

	if (bPushSkip)
	{
		//                  ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f;
		ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos);
		fKnockbackForce = 1.0f;
	}

	pickinfo.pObject->OnKnockback(pOwner->m_Direction, fKnockbackForce);

	float fActualDamage = CalcActualDamage(pOwner, pickinfo.pObject, (float)pDesc->m_nDamage.Ref());
	float fRatio = pItem->GetPiercingRatio(pDesc->m_nWeaponType.Ref(), pickinfo.info.parts);
	ZDAMAGETYPE dt = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;

	// old
	//if(pickinfo.pObject->IsNPC() || (ZGetChannelRuleMgr()->GetCurrentRule() && ZGetChannelRuleMgr()->GetCurrentRule()->GetID() == MCHANNEL_RULE_LEAD))
	//	pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio );
	//else
	//	((ZCharacter*)pickinfo.pObject)->OnDamaged_AntiLead(pOwner, pOwner->GetPosition(), ((ZCharacter*)pOwner)->GetItems()->GetSelectedWeaponParts(), pickinfo.info.parts, dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio );
	// Orby: (AntiLead).
	if (pickinfo.pObject->IsNPC() == true || (ZGetChannelRuleMgr()->GetCurrentRule() && ZGetChannelRuleMgr()->GetCurrentRule()->GetID() == MCHANNEL_RULE_LEAD))
	{
		pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
	}
	else
	{
		AntiLead_Info* pInfo = new AntiLead_Info;
		pInfo->m_nVictimLowID = pickinfo.pObject->GetUID().Low;
		pInfo->X = pos.x;
		pInfo->Y = pos.y;
		pInfo->Z = pos.z;
		pInfo->m_nSelType = pOwner->GetItems()->GetSelectedWeaponParts();
		pInfo->m_nPartsType = pickinfo.info.parts;
		vector<AntiLead_Info*> vInfo = {};
		vInfo.push_back(pInfo);
		((ZCharacter*)pickinfo.pObject)->OnDamaged_AntiLead(pOwner, vInfo);
		pickinfo.pObject->OnDamaged(pOwner, rvector(pInfo->X, pInfo->Y, pInfo->Z), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
		for (vector<AntiLead_Info*>::iterator i = vInfo.begin(); i != vInfo.end(); i++)
		{
			AntiLead_Info* p = *i;
			delete p;
		}
		vInfo.clear();
		vInfo.shrink_to_fit();
	}

	if (pOwner == m_pMyCharacter)
	{
		CheckCombo(m_pMyCharacter, pickinfo.pObject, !bPushSkip);
		CheckStylishAction(m_pMyCharacter);
	}

	v1 = pos;
	v2 = pickinfo.info.vOut;
}
void ZGame::OnPeerShot_Range(const MMatchCharItemParts sel_type, const MUID& uidOwner, float fShotTime, const rvector& pos, const rvector& to)
{
	ZObject *pOwner = m_ObjectManager.GetObject(uidOwner);
	if(!pOwner) return;

	ZItem *pItem = pOwner->GetItems()->GetItem(sel_type);
	if(!pItem) return;
	MMatchItemDesc *pDesc = pItem->GetDesc();
	if(!pDesc) { _ASSERT(FALSE); return; }

	rvector dir = to - pos;

	Normalize(dir);

	rvector v1, v2;
	rvector BulletMarkNormal;
	bool bBulletMark = false;
	ZTargetType nTargetType = ZTT_OBJECT;

	// ZCharacter* pCharacter = NULL;

	ZPICKINFO pickinfo;

	memset(&pickinfo,0,sizeof(ZPICKINFO));

	// �Ѿ��� ������ ?E��ϴ°��??E��Ѵ?
	const DWORD dwPickPassFlag=RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

	// ���� ĳ���� ���?E��?E.
	pOwner->Tremble(8.f, 50, 30);

/*
	if(pOwner->m_pVMesh)
	{
		float fMaxValue = 8.f;// ��?E���� +- ����

		RFrameTime* ft = &pOwner->m_pVMesh->m_FrameTime;
		if(ft && !ft->m_bActive)
			ft->Start(fMaxValue,50,30);// ���� , ��?Eð?, ���ͽð�...
	}
*/
	//jintriple3 ����� �������� �� ������ ���� �Լ��� ����..�ٵ� ���ڰ� ��?E����..�̤�
	OnPeerShot_Range_Damaged(pOwner, fShotTime, pos, to, pickinfo, dwPickPassFlag,v1, v2, pItem, BulletMarkNormal, bBulletMark, nTargetType);
	/*
	if(g_pGame->PickHistory(pOwner,fShotTime,pos,to,&pickinfo,dwPickPassFlag))
	{
		// ���� -bird 
		
		if(pickinfo.pObject)
		{
			ZObject *pObject = pickinfo.pObject;
			bool bGuard = pObject->IsGuard() && (pickinfo.info.parts!=eq_parts_legs) &&		// �ٸ��� ����������
							DotProduct(dir,pObject->GetDirection())<0;

			if(bGuard) 
			{
				nTargetType = ZTT_CHARACTER_GUARD;
				// ���Ҵ�
				rvector t_pos = pObject->GetPosition();
				t_pos.z += 100.f;
				ZGetEffectManager()->AddSwordDefenceEffect(t_pos+(-dir*50.f),-dir);
				pObject->OnGuardSuccess();

			}
			else 
			{
				nTargetType = ZTT_CHARACTER;

				ZActor* pATarget = MDynamicCast(ZActor,pickinfo.pObject);

				bool bPushSkip = false;

				if(pATarget) 
				{
					bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
				}

				float fKnockbackForce = pItem->GetKnockbackForce();

				if(bPushSkip) 
				{
//					ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
					rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f; 
					ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos );
					fKnockbackForce = 1.0f;
				}

				pickinfo.pObject->OnKnockback( pOwner->m_Direction, fKnockbackForce );

				float fActualDamage = CalcActualDamage(pOwner, pickinfo.pObject, (float)pDesc->m_nDamage);
				float fRatio = pItem->GetPiercingRatio( pDesc->m_nWeaponType, pickinfo.info.parts );
				ZDAMAGETYPE dt = (pickinfo.info.parts==eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
				//jintriple3 ����� �������� �����κ��� ��?Eϰ?��ȣ�ؾ� �� �κ�..
				pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType, fActualDamage, fRatio );

				if(pOwner == m_pMyCharacter) 
				{
					CheckCombo(m_pMyCharacter,pickinfo.pObject,!bPushSkip);
					CheckStylishAction(m_pMyCharacter);
				}
			}

			v1 = pos;
			v2 = pickinfo.info.vOut;

		}
		else if(pickinfo.bBspPicked)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			// ��ź ��?E
			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;

		}
		else 
		{
			_ASSERT(false);
			return;
		}
	}

	else
	{
		v1 = pos;
		v2 = pos+dir*10000.f;
		nTargetType	= ZTT_NOTHING;
	}*/
	

	bool bPlayer = false;
	//bool b3D = (pOwnerCharacter!=m_pMyCharacter);	// �ڱⰡ ���� ������ 2D�� ����Ѵ�.
	//rvector Pos = pOwnerCharacter->GetPosition();
	rvector Pos = v1;
	if(pOwner==m_pMyCharacter)
	{

		Pos = RCameraPosition;
		bPlayer = true;
	}

	ZCharacter *pTargetCharacter=ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	ZApplication::GetSoundEngine()->PlaySEFire(pDesc, Pos.x, Pos.y, Pos.z, bPlayer);
	//if(nTargetType == ZTT_OBJECT) { ZApplication::GetSoundEngine()->PlaySERicochet(v2.x, v2.y, v2.z); }
#define SOUND_CULL_DISTANCE 1500.0F
	if( D3DXVec3LengthSq(&(v2 - pTargetCharacter->GetPosition())) < (SOUND_CULL_DISTANCE * SOUND_CULL_DISTANCE) )
	{
		if(nTargetType == ZTT_OBJECT) { 
			ZGetSoundEngine()->PlaySEHitObject( v2.x, v2.y, v2.z, pickinfo.bpi ); 
		}

		if(nTargetType == ZTT_CHARACTER) { 
			ZGetSoundEngine()->PlaySEHitBody(v2.x, v2.y, v2.z); 
		}
	}

	//// ����?E����?E����Ʈ�� �׸��ʿ�� ���� - ��Ȯ�� �ø��� ���.. by bird
	//if(!pOwner->IsRendered()) return;


	// ����?E��?E100cm �� ȭ�鿡 ���̡��?Eüũ�Ѵ�
	bool bDrawFireEffects = isInViewFrustum(v1,100.f,RGetViewFrustum());

	if(!isInViewFrustum(v1,v2,RGetViewFrustum()) // ���°����� �´°��� ������ ���̴�?E.
		&& !bDrawFireEffects) return;					// ���°������� �׸��� ����?E.

	bool bDrawTargetEffects = isInViewFrustum(v2,100.f,RGetViewFrustum());



	/////////////////////// ���Ĵ� ����Ʈ �߰�

	// ��Ƣ�� ����Ʈ üũ
	GetWorld()->GetWaters()->CheckSpearing( v1, v2, 250, 0.3 );



	// TODO: NPC �� �ѱ���ġ �������̽��� Ȯ����?E��?E�߰�����.
//	ZCharacter *pOwnerCharacter = m_CharacterManager.Find(uidOwner);

	
	ZCharacterObject* pCOwnerObject = MDynamicCast(ZCharacterObject, pOwner);

	if(pCOwnerObject) 
	{

		// �ѱ� ȭ������Ʈ
		rvector pdir = v2-v1;
		Normalize(pdir);

		int size = 3;

		rvector v[6];

//		size = GetWeapondummyPos(pOwnerCharacter,v);
		if(pCOwnerObject->IsRendered())
			size = pCOwnerObject->GetWeapondummyPos(v);
		else
		{
			size = 6;
			v[0] = v[1] = v[2] = v1;
			v[3] = v[4] = v[5] = v[0];
		}


		MMatchWeaponType wtype = pDesc->m_nWeaponType.Ref();
		bool bSlugOutput = pDesc->m_bSlugOutput; // ź����?Etrue, false) 

		// Effect
		if(bBulletMark==false) BulletMarkNormal = -pdir;

		ZGetEffectManager()->AddShotEffect( v , size , v2, BulletMarkNormal, nTargetType, wtype, bSlugOutput, pCOwnerObject,bDrawFireEffects,bDrawTargetEffects);

		// �� �� ����Ʈ �߰�
		ZCharacterObject* pChar;

		if( ZGetConfiguration()->GetVideo()->bDynamicLight && pCOwnerObject != NULL )
		{
			pChar = pCOwnerObject;
		
			if( pChar->m_bDynamicLight )
			{
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
			else
			{
				pChar->m_bDynamicLight = true;
				pChar->m_vLightColor = g_CharLightList[GUN].vLightColor;
				pChar->m_vLightColor.x = 1.0f;
				pChar->m_iDLightType = GUN;
				pChar->m_fLightLife = g_CharLightList[GUN].fLife;
			}
		}
	}
	
	// ?E߿??E��?E		   p
	GetWorld()->GetFlags()->CheckSpearing( v1, v2, BULLET_SPEAR_EMBLEM_POWER );
	if(Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource( v1, 2.0f, 75 );
}


void ZGame::OnPeerShot_Shotgun(ZItem *pItem, ZCharacter* pOwnerCharacter, float fShotTime, const rvector& pos, const rvector& to)
{
	// �� ĳ���� Ȥ�� ���� �����ִ� ĳ����
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (!pTargetCharacter) return;

	//// �����?E����
	//{
	//  if(strcmp("����õ���ũ",pTargetCharacter->GetProperty()->szName)==0) {
	//      mlog("shot : %3.3f \n",fShotTime);
	//  }
	//}

	MMatchItemDesc *pDesc = pItem->GetDesc();
	if (!pDesc) { _ASSERT(false); return; }

	// Orby: teste.
	#define SHOTGUN_BULLET_COUNT    12
    #define SHOTGUN_DIFFUSE_RANGE   0.090f // Orby: Calibration on shotgun bullets.
	// Orby: teste.
	/*if (SHOTGUN_DIFFUSE_RANGE != 0.093f)
	{
		char lol[64]; // eres una parte de mi antihack dhausdhauxddd lol eso estaba ahi? no recuerdo Lo hice por ti hoy ah good thank :)
		sprintf(lol,XorStr<0xB9, 7, 0x39E1C34A>("\xEA\xCE\xC9\xD9\xDC\xD5" + 0x39E1C34A).s);
		mlog(XorStr<0x02, 3, 0xD5F26EB8>("\x27\x70" + 0xD5F26EB8).s);
		ExitProcess(NULL);
	}*/

	if (pOwnerCharacter == NULL) return;

	// ������� ���� random seed �� ������ ���������� �ʱ�ȭ ���ش�
	int *seed = (int*)&fShotTime;
	srand(*seed);

	bool bHitGuard = false, bHitBody = false, bHitGround = false, bHitEnemy = false;
	rvector GuardPos, BodyPos, GroundPos;
	bool waterSound = false;

	rvector v1, v2;
	rvector dir;

	rvector origdir = to - pos;
	Normalize(origdir);

	int nHitCount = 0;

	// Orby: (AntiLead).
	vector<AntiLead_Info*> vInfo = {};

	for (int i = 0; i<SHOTGUN_BULLET_COUNT; i++)
	{
		dir = origdir;
		{
			// ������ - �ݵ�?E?��?E�E?����
			rvector r, up(0, 0, 1), right;
			D3DXQUATERNION q;
			D3DXMATRIX mat;

			float fAngle = (rand() % (31415 * 2)) / 1000.0f;
			float fForce = RANDOMFLOAT*SHOTGUN_DIFFUSE_RANGE;

			D3DXVec3Cross(&right, &dir, &up);
			D3DXVec3Normalize(&right, &right);
			D3DXMatrixRotationAxis(&mat, &right, fForce);
			D3DXVec3TransformCoord(&r, &dir, &mat);

			D3DXQuaternionRotationAxis(&q, &dir, fAngle);
			D3DXMatrixRotationQuaternion(&mat, &q);
			D3DXVec3TransformCoord(&r, &r, &mat);

			dir = r;
		}
		rvector BulletMarkNormal;
		bool bBulletMark = false;
		ZTargetType nTargetType = ZTT_OBJECT;

		ZPICKINFO pickinfo;

		memset(&pickinfo, 0, sizeof(ZPICKINFO));

		// �Ѿ��� ������ ����ϴ°��� ����Ѵ�
		const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;

		// Orby: (AntiLead).
		AntiLead_Info *pInfo = OnPeerShotgun_Damaged(pOwnerCharacter, fShotTime, pos, dir, pickinfo, dwPickPassFlag, v1, v2, pItem, BulletMarkNormal, bBulletMark, nTargetType, bHitEnemy);

		if (pInfo)
			vInfo.push_back(pInfo);
	}

	if (bHitEnemy) 
	{
		CheckStylishAction(pOwnerCharacter);
		CheckCombo(pOwnerCharacter, NULL, true);
	}
	else
	{

	}

	// Orby: (AntiLead).
	if (vInfo.size() > 0 && !ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
	{
		if(ZGetGame())
		{
			ZCharacter* pCharacter = ZGetCharacterManager()->Find(MUID(0, vInfo[0]->m_nVictimLowID));
			if (pCharacter && pOwnerCharacter->GetUID().Low == ZGetGameClient()->GetPlayerUID().Low)
			{
				if (!GetMatch()->IsTeamPlay())
				{
					pCharacter->OnDamaged_AntiLead(pOwnerCharacter, vInfo);
				}
				else if (GetMatch()->IsTeamPlay() && ZGetGame()->GetMatch()->GetTeamKillEnabled() && pCharacter->IsTeam(pOwnerCharacter))
				{
					pCharacter->OnDamaged_AntiLead(pOwnerCharacter, vInfo);
				}
				else if (GetMatch()->IsTeamPlay() && !pCharacter->IsTeam(pOwnerCharacter))
				{
					pCharacter->OnDamaged_AntiLead(pOwnerCharacter, vInfo);
				}
			}
		}
	}

	if (vInfo.size() > 0)
	{
		for (vector<AntiLead_Info*>::iterator i = vInfo.begin(); i != vInfo.end(); i++)
		{
			AntiLead_Info* p = *i;
			delete p;
		}
		vInfo.clear();
		vInfo.shrink_to_fit();
	}

	ZApplication::GetSoundEngine()->PlaySEFire(pItem->GetDesc(), pos.x, pos.y, pos.z, (pOwnerCharacter == m_pMyCharacter));

	// ������ ������ ����Ʈ�� �׸��ʿ�� ����
	if (!pOwnerCharacter->IsRendered()) return;

	rvector v[6];

	int _size = pOwnerCharacter->GetWeapondummyPos(v);

	dir = to - pos;
	Normalize(dir);

	ZGetEffectManager()->AddShotgunEffect(const_cast<rvector&>(pos), v[1], dir, pOwnerCharacter);

	// �� �� ����Ʈ �߰�
	ZCharacter* pChar;
	if (ZGetConfiguration()->GetVideo()->bDynamicLight && pOwnerCharacter != NULL)
	{
		pChar = pOwnerCharacter;

		if (pChar->m_bDynamicLight)
		{
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
		else
		{
			pChar->m_bDynamicLight = true;
			pChar->m_vLightColor = g_CharLightList[SHOTGUN].vLightColor;
			pChar->m_vLightColor.x = 1.0f;
			pChar->m_iDLightType = SHOTGUN;
			pChar->m_fLightLife = g_CharLightList[SHOTGUN].fLife;
		}
	}
	//  m_flags.CheckSpearing( v1, v2, SHOTGUN_SPEAR_EMBLEM_POWER );
	if (Z_VIDEO_DYNAMICLIGHT)
		ZGetStencilLight()->AddLightSource(v1, 2.0f, 200);
}

//jintriple3 ����� �������� ��ŷ ���� �ڵ� ����
AntiLead_Info* ZGame::OnPeerShotgun_Damaged(
	ZObject* pOwner, float fShotTime, const rvector& pos, rvector& dir, ZPICKINFO pickinfo, DWORD dwPickPassFlag, 
	rvector& v1, rvector& v2, ZItem *pItem, 
	rvector& BulletMarkNormal, bool& bBulletMark, ZTargetType& nTargetType, bool& bHitEnemy
	)
{
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	bool bReturnValue = !pTargetCharacter;
	if(!pTargetCharacter)PROTECT_DEBUG_REGISTER(bReturnValue) return NULL;

	MMatchItemDesc *pDesc = pItem->GetDesc();
	bReturnValue = !pDesc;
	if(!pDesc)PROTECT_DEBUG_REGISTER(bReturnValue) { _ASSERT(FALSE); return NULL; }

	//rvector dir = to - pos;

	bool waterSound = false;
	//���⿡ ����ڵ尡 ���ߵ�~
	bReturnValue = !(ZGetGame()->PickHistory(pOwner, fShotTime, pos, pos + 10000.f*dir, &pickinfo, dwPickPassFlag));
	if (!(ZGetGame()->PickHistory(pOwner, fShotTime, pos, pos + 10000.f*dir, &pickinfo, dwPickPassFlag)))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			v1 = pos;
			v2 = pos + dir*10000.f;
			nTargetType = ZTT_NOTHING;
			waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);
			return NULL;
		}
	}
	//���⵵..
	bReturnValue = (!pickinfo.pObject) && (!pickinfo.bBspPicked);
	if (pickinfo.bBspPicked)
	{
		PROTECT_DEBUG_REGISTER(pickinfo.nBspPicked_DebugRegister == FOR_DEBUG_REGISTER)
		{
			nTargetType = ZTT_OBJECT;

			v1 = pos;
			v2 = pickinfo.bpi.PickPos;

			// ��ź ����
			BulletMarkNormal.x = pickinfo.bpi.pInfo->plane.a;
			BulletMarkNormal.y = pickinfo.bpi.pInfo->plane.b;
			BulletMarkNormal.z = pickinfo.bpi.pInfo->plane.c;
			Normalize(BulletMarkNormal);
			bBulletMark = true;

			// �´°� �ݰ� 20cm �� ȭ�鿡 ������ �׸���
			bool bDrawTargetEffects = isInViewFrustum(v2, 20.f, RGetViewFrustum());
			if (bDrawTargetEffects)
				ZGetEffectManager()->AddBulletMark(v2, BulletMarkNormal);
			waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);
			return NULL;
		}
	}
	else if ((!pickinfo.pObject) && (!pickinfo.bBspPicked))
	{
		PROTECT_DEBUG_REGISTER(bReturnValue)
		{
			_ASSERT(false);
			return NULL;
		}
	}

	//���������� �˻� �ܰ�...

	ZObject *pObject = pickinfo.pObject;
	bool bGuard = pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&     // �ٸ��� ����������
		DotProduct(dir, pObject->GetDirection())<0;

	if (pObject->IsGuard() && (pickinfo.info.parts != eq_parts_legs) &&
		DotProduct(dir, pObject->GetDirection())<0)
	{
		PROTECT_DEBUG_REGISTER(bGuard)
		{
			nTargetType = ZTT_CHARACTER_GUARD;
			// ���Ҵ�
			rvector t_pos = pObject->GetPosition();
			t_pos.z += 100.f;
			ZGetEffectManager()->AddSwordDefenceEffect(t_pos + (-dir*50.f), -dir);
			pObject->OnGuardSuccess();
			v1 = pos;
			v2 = pickinfo.info.vOut;
			return NULL;
		}
	}

	ZActor* pATarget = MDynamicCast(ZActor, pObject);
	ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pObject);
	nTargetType = ZTT_CHARACTER;

	bool bPushSkip = false;

	if (pATarget)
	{
		bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
	}
	if (pFSMActor)
	{
		bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
	}

	float fKnockbackForce = pItem->GetKnockbackForce() / (.5f*float(SHOTGUN_BULLET_COUNT));

	if (bPushSkip)
	{
		//                  ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		rvector vPos = pOwner->GetPosition() + (pickinfo.pObject->GetPosition() - pOwner->GetPosition()) * 0.1f;
		ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met", vPos);
		fKnockbackForce = 1.0f;
	}

	pObject->OnKnockback(dir, fKnockbackForce);

	float fActualDamage = CalcActualDamage(pOwner, pObject, (float)pDesc->m_nDamage.Ref());
	float fRatio = pItem->GetPiercingRatio(pDesc->m_nWeaponType.Ref(), pickinfo.info.parts);
	ZDAMAGETYPE dt = (pickinfo.info.parts == eq_parts_head) ? ZD_BULLET_HEADSHOT : ZD_BULLET;
	AntiLead_Info* pInfo = NULL;
	if (pickinfo.pObject->IsNPC() == true || (ZGetChannelRuleMgr()->GetCurrentRule() && ZGetChannelRuleMgr()->GetCurrentRule()->GetID() == MCHANNEL_RULE_LEAD))
	{
		pickinfo.pObject->OnDamaged(pOwner, pOwner->GetPosition(), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
	}
	else
	{
		pInfo = new AntiLead_Info;
		pInfo->m_nVictimLowID = pickinfo.pObject->GetUID().Low;
		//pInfo->X = pos.x;
		//pInfo->Y = pos.y;
		//pInfo->Z = pos.z;
		pInfo->m_nPartsType = pickinfo.info.parts;
		pInfo->m_nSelType = ((ZCharacter*)pOwner)->GetItems()->GetSelectedWeaponParts();
		//pickinfo.pObject->OnDamaged(pOwner, rvector(pInfo->X, pInfo->Y, pInfo->Z), dt, pDesc->m_nWeaponType.Ref(), fActualDamage, fRatio);
	}

	if (!m_Match.IsTeamPlay() || (pTargetCharacter->GetTeamID() != pObject->GetTeamID()))
	{
		bHitEnemy = true;
	}

	v1 = pos;
	v2 = pickinfo.info.vOut;

	waterSound = GetWorld()->GetWaters()->CheckSpearing(v1, v2, 250, 0.3, !waterSound);

	return pInfo;
}


bool ZGame::CanISeeAttacker( ZCharacter* pAtk, const rvector& vRequestPos )
{
	const rvector& vAtkPos = pAtk->GetPosition();

	long double x = pow(vAtkPos.x - vRequestPos.x, 2);
	long double y = pow(vAtkPos.y - vRequestPos.y, 2);
	long double z = pow(vAtkPos.z - vRequestPos.z, 2);

	long double Len = x + y + z;

	// base info�� �����?E�ִ� ��ġ�� ���� �߻��� ������ ��ġ���� �Ʒ� ��ġ���� �۾��� �Ѵ�.
	// ?E?��?E1������ �����̰� ��?E�ִ�?E������.
	// ��?E?E���?E?E��?E��ġ���� ������ �ɷ� �Ǵ��Ѵ�. - by SungE 2007-04-17
#define MAX_VIEW_LENGTH 800000 // ?E?���?E?2���� �Ÿ�.

	if( MAX_VIEW_LENGTH < Len )
	{
#ifdef _DEBUG
		static rvector rv( 0.0f, 0.0f, 0.0f );

		long double l = pow(vRequestPos.x - rv.x, 2) + pow(vRequestPos.y - rv.y, 2) + pow(vRequestPos.z - rv.z, 2);

		rv = vRequestPos;

		mlog( "len : %f(%f), res(%d)\n", Len, sqrt(Len), MAX_VIEW_LENGTH < Len );
#endif
		return false;
	}

	return true;
}


// shot �� shot_range, shot_melee, shot_shotgun ���� command �� ���� �и��ϴ°͵� ����� ����?E
void ZGame::OnPeerShot( const MUID& uid, float fShotTime, const rvector& pos, const rvector& to, const MMatchCharItemParts sel_type)
{
	ZCharacter* pOwnerCharacter = NULL;

	pOwnerCharacter = m_CharacterManager.Find(uid);

	if (pOwnerCharacter == NULL) return;
	if(!pOwnerCharacter->IsVisible()) return;

#ifdef LOCALE_NHNUSA
	if( !CanISeeAttacker(pOwnerCharacter, pos) ) return;
#endif

	pOwnerCharacter->OnShot();
	
	// fShotTime �� �� ĳ������ ���� �ð��̹Ƿ� �� �ð����� ��ȯ���ش�
	fShotTime-=pOwnerCharacter->m_fTimeOffset;
	
	ZItem *pItem = pOwnerCharacter->GetItems()->GetItem(sel_type);
	if(!pItem || !pItem->GetDesc()) return;

	// ���������� �߻�ӵ��� �����Ѵ�.
	if (pOwnerCharacter->CheckValidShotTime(pItem->GetDescID(), fShotTime, pItem)) 
	{
		pOwnerCharacter->UpdateValidShotTime(pItem->GetDescID(), fShotTime);
	} 
	else 
	{
		return;
	}

	//// ������ MEMORYHACK�־��� ��?E
	if (uid == ZGetMyUID()) {

		int nCurrMagazine = pItem->GetBulletCurrMagazine();

		// ������ ���⸦ ��?E
		if (!pItem->Shot()) return;

		if (!(pItem->GetBulletCurrMagazine() < nCurrMagazine))	// Shot���� �Ѿ� �پ��߸� �����̴�
			if(sel_type != MMCIP_MELEE)
				ZGetApplication()->Exit();
	} else {
		// ������ ���⸦ ��?E
		if (!pItem->Shot()) {
//			_ASSERT(FALSE);	// �������ִ�, ġ�� ?
			if(!ZGetGame()->IsReplay())	// ���÷��̶�?E�Ѿ��� ������ ��?E��?�߻�ó���� ���ش�.
				return;	// SHOT
		}
	}

	// MELEE�� ��?E
	if (sel_type == MMCIP_MELEE)
	{
		OnPeerShot_Melee(uid,fShotTime);

		return;
	}

	if ((sel_type != MMCIP_PRIMARY) && (sel_type != MMCIP_SECONDARY) &&	(sel_type != MMCIP_CUSTOM1 )) return;


	if(!pItem->GetDesc()) return;
	MMatchWeaponType wtype = pItem->GetDesc()->m_nWeaponType.Ref();

#ifdef _RADAR
	if (strstr(ZGetGameClient()->GetChannelName(), "Ladder"))
	{
		rvector _pos = pos;
		MUID _uid = uid;
		if ((wtype >= MWT_SHOTGUN) && (wtype <= MWT_RIFLE))
			ZGetCombatInterface()->GetRadar()->OnAttack(_pos, _uid);
	}
#endif

	if(wtype == MWT_SHOTGUN)
	{
		OnPeerShot_Shotgun(pItem,pOwnerCharacter,fShotTime,pos,to);
		return;
	} else {
		OnPeerShot_Range(sel_type,uid,fShotTime,pos,to);

		rvector position;
		pOwnerCharacter->GetWeaponTypePos( weapon_dummy_muzzle_flash , &position );
		if( ZGetConfiguration()->GetVideo()->bDynamicLight )
		{
			RGetDynamicLightManager()->AddLight( GUNFIRE, position );
		}
	}
}

void ZGame::OnPeerDie(MUID& uidVictim, MUID& uidAttacker)
{
	ZCharacter* pVictim = (ZCharacter*) m_CharacterManager.Find(uidVictim);
	if (pVictim == NULL) return;

	pVictim->ActDead();

	if (pVictim == m_pMyCharacter)	
	{
		pVictim->Die();		// ���⼭ ������ �״´�. �� �ڽ��� �����ε� ���⼭ �״°� ó��

		// ���ý� �Ǵ� ����Ʈ���� ����?E����?E��?E
		if (m_Match.IsWaitForRoundEnd())
		{
			if (m_CharacterManager.GetCount() > 2)
			{
				if (GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL)
					ReserveObserver();
			}
		}
#ifdef _QUEST
		else if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) || ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
		{
			if (m_CharacterManager.GetCount() >= 2)
			{
				ReserveObserver();
			}
		}
#endif

		CancelSuicide();
	}


	ZCharacter* pAttacker = (ZCharacter*) m_CharacterManager.Find(uidAttacker);
	if (pAttacker == NULL) return;	// ���� ����� ������?E�� ��?E?�׳� ����
	if(pAttacker!=pVictim)	// ��?E?�ƴ�?E����Ʈ �����ٰ� ��������
	{
		//Only use in team games, no duels or anything else
		if (ZGetGame()->GetMatch()->IsTeamPlay() && ZGetGame()->GetMatch()->GetRoundKills() == 1)
		{
			pAttacker->AddIcon(ZCI_FIRSTBLOOD);
			return;
		}

		if (ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
		{
			if (pAttacker->GetKils() + 1 == 5)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nFantastic, pAttacker->GetStatus().Ref().nFantastic+1);
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
			else if (pAttacker->GetKils() + 1 == 15)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nExcellent, pAttacker->GetStatus().Ref().nExcellent+1);
				pAttacker->AddIcon(ZCI_EXCELLENT);
			}
			else if (pAttacker->GetKils() + 1 == 30)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nUnbelievable, pAttacker->GetStatus().Ref().nUnbelievable+1);
				pAttacker->AddIcon(ZCI_UNBELIEVABLE);
			}
		}
		else
		{
			if (pAttacker->GetKils() >= 3)
			{
				MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nFantastic, pAttacker->GetStatus().Ref().nFantastic+1);
				pAttacker->AddIcon(ZCI_FANTASTIC);
			}
		}

		if(pVictim->GetLastDamageType()==ZD_BULLET_HEADSHOT)
		{
			MEMBER_SET_CHECKCRC(pAttacker->GetStatus(), nHeadShot, pAttacker->GetStatus().Ref().nHeadShot+1);
			pAttacker->AddIcon(ZCI_HEADSHOT);
		}
	}
}

// �����κ��� ���� ������� Dead�޼�?E
void ZGame::OnPeerDead(const MUID& uidAttacker, const unsigned long int nAttackerArg, 
					   const MUID& uidVictim, const unsigned long int nVictimArg)
{
	ZCharacter* pVictim = (ZCharacter*) m_CharacterManager.Find(uidVictim);
	ZCharacter* pAttacker = (ZCharacter*) m_CharacterManager.Find(uidAttacker);

	bool bSuicide = false;
	if (uidAttacker == uidVictim) bSuicide = true;

	int nAttackerExp = 0, nVictimExp = 0;

	nAttackerExp = GetExpFromTransData(nAttackerArg);
	nVictimExp = -GetExpFromTransData(nVictimArg);

	if(pAttacker)
	{
		pAttacker->GetStatus().CheckCrc();

		pAttacker->GetStatus().Ref().AddExp(nAttackerExp);
		if (!bSuicide) 
			pAttacker->GetStatus().Ref().AddKills();

		pAttacker->GetStatus().MakeCrc();
	}

	if(pVictim)
	{
		if (pVictim != m_pMyCharacter)
		{
			pVictim->Die();		// ���⼭ ������ �״´�
		}

		pVictim->GetStatus().CheckCrc();
		
		pVictim->GetStatus().Ref().AddExp(nVictimExp);
		pVictim->GetStatus().Ref().AddDeaths();
		if (pVictim->GetStatus().Ref().nLife > 0)
			pVictim->GetStatus().Ref().nLife--;

		pVictim->GetStatus().MakeCrc();
	}

	// ȭ?E����ġ ����Ʈ ǥ��
	if(bSuicide && (ZGetCharacterManager()->Find(uidAttacker)==ZGetGame()->m_pMyCharacter)) 
	{
		// ��?E
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);
	    int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);

		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
	else if(ZGetCharacterManager()->Find(uidAttacker)==m_pMyCharacter) 
	{
		// ���� attacker �϶�
		int nExpPercent;
		ZGetScreenEffectManager()->AddExpEffect(nAttackerExp);
		nExpPercent = GetExpPercentFromTransData(nAttackerArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
	else if(ZGetCharacterManager()->Find(uidVictim)==m_pMyCharacter) 
	{
		// ���� victim �϶�
		ZGetScreenEffectManager()->AddExpEffect(nVictimExp);

		int nExpPercent = GetExpPercentFromTransData(nVictimArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}

	m_Match.AddRoundKills();

	CheckKillSound(pAttacker);
	OnPeerDieMessage(pVictim, pAttacker);
}

void ZGame::CheckKillSound(ZCharacter* pAttacker)
{
	if ((!pAttacker) || (pAttacker != m_pMyCharacter)) return;

	if (m_Match.GetRoundKills() == 1)
	{
		ZApplication::GetSoundEngine()->PlayVoiceSound(VOICE_FIRST_KILL);
	}
}

void ZGame::OnReceiveTeamBonus(const MUID& uidChar, const unsigned long int nExpArg)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	int nExp = 0;

	nExp = GetExpFromTransData(nExpArg);

	if(pCharacter)
	{
		pCharacter->GetStatus().CheckCrc();
		pCharacter->GetStatus().Ref().AddExp(nExp);
		pCharacter->GetStatus().MakeCrc();
	}

	// ȭ?E����ġ ����Ʈ ǥ��
	if(pCharacter==m_pMyCharacter) 
	{
#ifdef _DEBUG
		char szTemp[128];
		sprintf(szTemp, "TeamBonus = %d\n", nExp);
		OutputDebugString(szTemp);
#endif

		// ���� attacker �϶�
		ZGetScreenEffectManager()->AddExpEffect(nExp);

		
		int nExpPercent = GetExpPercentFromTransData(nExpArg);
		ZGetMyInfo()->SetLevelPercent(nExpPercent);
		ZGetScreenEffectManager()->SetGaugeExpFromMyInfo();
	}
}

void ZGame::OnPeerDieMessage(ZCharacter* pVictim, ZCharacter* pAttacker)
{
	const char* testdeathnametable[ZD_END + 1] = { "����", "��", "Į", "�߶�", "����", "HEADSHOT", "������Į��" };
	char szMsg[256] = "";

	const char* szAnonymous = "?Unknown?";

	char szVictim[256];
	strcpy(szVictim, pVictim ? pVictim->GetUserAndClanName() : szAnonymous);

	char szAttacker[256];
	strcpy(szAttacker, pAttacker ? pAttacker->GetUserAndClanName() : szAnonymous);

	if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
	{
		char szInfectedMsg[128];
		memset(szInfectedMsg, 0, sizeof(szInfectedMsg));

		if (pVictim == pAttacker && pVictim->GetTeamID() == MMT_BLUE)
			sprintf_s(szInfectedMsg, "Human '%s' has died!", pVictim ? pVictim->GetUserName() : "Unknown");
		else if (pVictim->GetTeamID() == MMT_RED)
		{
			sprintf_s(szInfectedMsg, "Zombie '%s' has been slain!", pVictim ? pVictim->GetUserName() : "Unknown");
			ZGetSoundEngine()->PlaySound("zombie_die");
		}
		else if (pVictim->GetTeamID() == MMT_BLUE)
			sprintf_s(szInfectedMsg, "Human '%s' has been infected!", pVictim ? pVictim->GetUserName() : "Unknown");
		ZGetGameInterface()->GetCombatInterface()->UpdateCTFMsg(szInfectedMsg);
	}

	if (pAttacker == pVictim)
	{
		if (pVictim == m_pMyCharacter)
		{
			m_pMyCharacter->GetStatus().CheckCrc();
#ifdef _KILLSTREAK
			m_pMyCharacter->GetStatus().Ref().nKillStreakCount = 0;
#endif
			m_pMyCharacter->GetStatus().MakeCrc();

			if (m_pMyCharacter->GetLastDamageType() == ZD_EXPLOSION) {
				sprintf(szMsg, ZMsg(MSG_GAME_LOSE_BY_MY_BOMB));
			}
			else {
				sprintf(szMsg, ZMsg(MSG_GAME_LOSE_MYSELF));
			}

			ZChatOutput(MCOLOR(0xFFCF2020), szMsg);
		}
		else
		{
			ZTransMsg(szMsg, MSG_GAME_WHO_LOSE_SELF, 1, szAttacker);
			ZChatOutput(MCOLOR(0xFF707070), szMsg);

			// Admin Grade
			if (ZGetMyInfo()->IsAdminGrade()) {
				MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
				if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
				{
					sprintf(szMsg, "",
						(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1,
						pAttacker->GetProperty()->GetName());
					ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg(szMsg);
				}
			}
		}

	}

	else if (pAttacker == m_pMyCharacter)
	{
#ifdef _KILLSTREAK
		int killz = m_pMyCharacter->GetKillStreaks() + 1;
		m_pMyCharacter->GetStatus().CheckCrc();
		m_pMyCharacter->GetStatus().Ref().nKillStreakCount = killz;
		m_pMyCharacter->GetStatus().MakeCrc();
		ZPostKillStreak(szAttacker, m_pMyCharacter->GetKillStreaks(), szVictim);
#endif

	}

	else if (pVictim == m_pMyCharacter)
	{
		// Custom: Fix
		m_pMyCharacter->GetStatus().CheckCrc();
#ifdef _KILLSTREAK
		m_pMyCharacter->GetStatus().Ref().nKillStreakCount = 0;
#endif
		m_pMyCharacter->GetStatus().MakeCrc();

		//MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
		//if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_CLASSIC_TEAM && !ZGetGameClient()->IsCWChannel() && !m_Match.IsQuestDrived() && pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false)
		//{
		//	ZGetCombatInterface()->SetDeathObserverMode(pAttacker->GetUID());
		//}


		if (pAttacker && !ZGetGameClient()->IsCWChannel())
		{
			char szTemp[128];
			sprintf(szTemp, " (HP: %.0f/%.0f, AP: %.0f/%.0f)", pAttacker->GetHP(), pAttacker->GetMaxHP(), pAttacker->GetAP(), pAttacker->GetMaxAP());
			ZTransMsg(szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker);
			strcat(szMsg, szTemp);
		}
		else if (pAttacker && !ZGetGameClient()->IsPWChannel())
		{
			char szTemp[128];
			sprintf(szTemp, " (HP: %.0f/%.0f, AP: %.0f/%.0f)", pAttacker->GetHP(), pAttacker->GetMaxHP(), pAttacker->GetAP(), pAttacker->GetMaxAP());
			ZTransMsg(szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker);
			strcat(szMsg, szTemp);
		}
		else
		{
			ZTransMsg(szMsg, MSG_GAME_LOSE_FROM_WHO, 1, szAttacker);
			ZChatOutput(MCOLOR(0xFFCF2020), szMsg);
		} 

#ifdef _KILLCAM
		MMATCH_GAMETYPE gametype = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
		if ((ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
			(ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_TRAINING ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GLADIATOR_SOLO ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_BERSERKER ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GUNGAME ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DROPMAGIC))
		{
			ZGetCombatInterface()->SetObserverMode(true);
			ZGetCombatInterface()->GetObserver()->SetTarget(pAttacker->GetUID());
		}
	}
	else
	{
#endif
// 		sprintf(szMsg, "%s���� %s�����κ��� �¸��Ͽ����ϴ�.", szAttacker, szVictim );
		ZTransMsg( szMsg, MSG_GAME_WHO_WIN_FROM_OTHER, 2, szAttacker, szVictim );
		ZChatOutput(MCOLOR(0xFF707070), szMsg);	

		// Admin Grade
		if (ZGetMyInfo()->IsAdminGrade()) {
			MMatchObjCache* pCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
			if (pCache && pCache->CheckFlag(MTD_PlayerFlags_AdminHide))
			{
				// Custom: Changed string
				if (pAttacker)
				{
					sprintf(szMsg, "^%d%s^9 wins,  ^%d%s^9 loses", //sprintf( szMsg, "^%d%s^9 �¸�,  ^%d%s^9 ��?E,
						(pAttacker->GetTeamID() == MMT_BLUE) ? 3 : 1, pAttacker->GetProperty()->GetName(),
						(pVictim->GetTeamID() == MMT_BLUE) ? 3 : 1, pVictim->GetProperty()->GetName());
					ZGetGameInterface()->GetCombatInterface()->m_AdminMsg.OutputChatMsg(szMsg);
				}
			}
		}
	}
}


void ZGame::OnReloadComplete(ZCharacter *pCharacter)
{
	ZItem* pItem = pCharacter->GetItems()->GetSelectedWeapon();
	
	pCharacter->GetItems()->Reload();

	if(pCharacter==m_pMyCharacter)
		ZApplication::GetSoundEngine()->PlaySound("we_weapon_rdy");

	return;

/*	ZItem* pItem = pCharacter->GetItems()->GetSelectedWeapon();
	//// ������ MEMORYHACK�־��� ��?E
	if (pCharacter->GetUID() == ZGetMyUID() && pItem!=NULL) {
		MDataChecker* pChecker = ZGetGame()->GetDataChecker();
		MDataCheckNode* pCheckNodeA = pChecker->FindCheck((BYTE*)pItem->GetBulletPointer());
		MDataCheckNode* pCheckNodeB = pChecker->FindCheck((BYTE*)pItem->GetAMagazinePointer());
		if ( (pCheckNodeA && (pCheckNodeA->UpdateChecksum()==false)) ||
		 	 (pCheckNodeB && (pCheckNodeB->UpdateChecksum()==false)) ) 
		{
			pChecker->BringError();	//// MEMORYHACK ��?E Checksum ����ߴ���?E���峽��.
		} else {
			bool bResult = pCharacter->GetItems()->Reload();	//// RELOAD ////

			//// MEMORYHACK ������?E�����Ѵ�.
			pChecker->RenewCheck((BYTE*)pItem->GetBulletPointer(), sizeof(int));
			pChecker->RenewCheck((BYTE*)pItem->GetAMagazinePointer(), sizeof(int));
		}
	} else {
		bool bResult = pCharacter->GetItems()->Reload();	//// RELOAD ////
	}

	if(pCharacter==m_pMyCharacter) {
		ZApplication::GetSoundEngine()->PlaySound("we_weapon_rdy");
	}
	return;*/
}

void ZGame::OnPeerSpMotion(MUID& uid,int nMotionType)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	// Custom: Ignore List check
	if( ZGetGameClient()->IsUserIgnored( pCharacter->GetUserName() ) && (nMotionType == ZC_SPMOTION_TAUNT || nMotionType == ZC_SPMOTION_CRY || nMotionType == ZC_SPMOTION_LAUGH) )
		return;

	pCharacter->m_dwStatusBitPackingValue.Ref().m_bSpMotion = true;

	ZC_STATE_LOWER zsl = ZC_STATE_TAUNT;

	if(nMotionType == ZC_SPMOTION_TAUNT)
	{
		zsl = ZC_STATE_TAUNT;

		char szSoundName[ 50];
		if ( pCharacter->GetProperty()->nSex == MMS_MALE)
			sprintf( szSoundName, "fx2/MAL1%d", (RandomNumber(0, 300) % 3) + 1);
		else
			sprintf( szSoundName, "fx2/FEM1%d", (RandomNumber(0, 300) % 3) + 1);

		ZGetSoundEngine()->PlaySound( szSoundName, pCharacter->GetPosition());
	}
	else if(nMotionType == ZC_SPMOTION_BOW)
		zsl = ZC_STATE_BOW;
	else if(nMotionType == ZC_SPMOTION_WAVE)
		zsl = ZC_STATE_WAVE;
	else if(nMotionType == ZC_SPMOTION_LAUGH)
	{
		zsl = ZC_STATE_LAUGH;

		if ( pCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound( "fx2/MAL01", pCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound( "fx2/FEM01", pCharacter->GetPosition());
	}
	else if(nMotionType == ZC_SPMOTION_CRY)
	{
		zsl = ZC_STATE_CRY;

		if ( pCharacter->GetProperty()->nSex == MMS_MALE)
			ZGetSoundEngine()->PlaySound( "fx2/MAL02", pCharacter->GetPosition());
		else
			ZGetSoundEngine()->PlaySound( "fx2/FEM02", pCharacter->GetPosition());
	}
	else if(nMotionType == ZC_SPMOTION_DANCE)
		zsl = ZC_STATE_DANCE;

	pCharacter->m_SpMotion = zsl;

	pCharacter->SetAnimationLower( zsl );
}

void ZGame::OnPeerReload(MUID& uid)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if (pCharacter == NULL || pCharacter->IsDie() ) return;

	// �ӽ�.. ���� ��ü�ִϸ��̼��� ����?Eĵ��.. ����?E�� ����
	//	if(pCharacter->m_pVMesh->m_pAniSet[ani_mode_upper]!=NULL) return;

	// �� ĳ���ʹ� �ִϸ��̼��� �������� ������ ����?E��Ų��.
	if(pCharacter==m_pMyCharacter)
	{
		// Custom: Fast Reload Room Modifier
		if (ZGetGameClient()->GetMatchStageSetting()->IsModifierUsed(MMOD_FASTRELOAD) && !ZGetGameClient()->GetMatchStageSetting()->IsQuestDrived() && ZGetGame()->GetMatch()->GetMatchType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
			//&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_SOLO
			//&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_TEAM
			//&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DEATHMATCH_TEAM2
			//&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL )
		{
			m_pMyCharacter->Animation_Reload(); // Play the animation to allow reload shots
			OnReloadComplete(pCharacter); // Reload the clip instantly
		}
		else
			m_pMyCharacter->Animation_Reload();
	}
	else
		OnReloadComplete(pCharacter);

	// Sound Effect
	if(pCharacter->GetItems()->GetSelectedWeapon()!=NULL) {
		rvector p = pCharacter->GetPosition()+rvector(0,0,160.f);
		ZApplication::GetSoundEngine()->PlaySEReload(pCharacter->GetItems()->GetSelectedWeapon()->GetDesc(), p.x, p.y, p.z, (pCharacter==m_pMyCharacter));
	}
}

void ZGame::OnPeerChangeCharacter(MUID& uid)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);

	//	if (uid == ZGetGameClient()->GetUID()) pCharacter = m_pMyCharacter;

	if (pCharacter == NULL) return;

	pCharacter->TestToggleCharacter();
}

/*
void ZGame::OnAssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander)
{
	AssignCommander(uidRedCommander, uidBlueCommander);
}

void ZGame::AssignCommander(const MUID& uidRedCommander, const MUID& uidBlueCommander)
{
	ZCharacter* pRedChar = m_CharacterManager.Find(uidRedCommander);
	ZCharacter* pBlueChar = m_CharacterManager.Find(uidBlueCommander);

	if(pRedChar) {
		ZGetEffectManager()->AddCommanderIcon(pRedChar,0);
		pRedChar->m_bCommander = true;
	}
	if(pBlueChar) {
		ZGetEffectManager()->AddCommanderIcon(pBlueChar,1);
		pBlueChar->m_bCommander = true;
	}

#ifdef _DEBUG
	//// DEBUG LOG ////
	const char *szUnknown = "unknown";
	char szBuf[128];
	sprintf(szBuf, "RedCMDER=%s , BlueCMDER=%s \n", 
		pRedChar ? pRedChar->GetProperty()->szName : szUnknown , 
		pBlueChar ? pBlueChar->GetProperty()->szName : szUnknown );
	OutputDebugString(szBuf);
	///////////////////
#endif
}
*/
void ZGame::OnSetObserver(MUID& uid)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;

	if(pCharacter==m_pMyCharacter)
	{
		ZGetCombatInterface()->SetObserverMode(true);
	}
	pCharacter->SetVisible(false);
	pCharacter->ForceDie();
}

void ZGame::OnPeerSpawn(MUID& uid, rvector& pos, rvector& dir)
{
	m_nSpawnTime = timeGetTime();
	SetSpawnRequested(false);

	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);
	if (pCharacter == NULL) return;
	bool isRespawn	= ( pCharacter->IsDie() == true ) ? true : false;

	//	dir = rvector(-1.f,0,0);
	pCharacter->SetVisible(true);
	pCharacter->Revival();
	pCharacter->SetPosition(pos);
	pCharacter->SetDirection(dir);
	pCharacter->SetSpawnTime(GetTime());

	ZGetEffectManager()->AddReBirthEffect(pos);

	if(pCharacter==m_pMyCharacter)
	{
#ifdef _KILLCAM
		MMATCH_GAMETYPE gametype = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
		if ((ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DEATHMATCH_SOLO) ||
			(ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_TRAINING ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GLADIATOR_SOLO ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_BERSERKER ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GUNGAME ||
				ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DROPMAGIC))
			ZGetCombatInterface()->SetObserverMode(false);
#endif
		m_pMyCharacter->InitSpawn();

#ifdef _VMPROTECT
		VMProtectBeginUltra("OnSpawnValidImage");
		if (!VMProtectIsValidImageCRC())
		{
			ZPostBanMe(WHT_MEMCMP);
			if (!g_ShouldBanPlayer)
				g_ShouldBanPlayer = timeGetTime();
		}

VMProtectEnd();

#endif

		if( isRespawn )	
		{
			ZGetSoundEngine()->PlaySound("fx_respawn");
		} else {
			ZGetSoundEngine()->PlaySound("fx_whoosh02");
		}
		ZGetScreenEffectManager()->ReSetHpPanel();

	}
	pCharacter->GetStatus().CheckCrc();
	pCharacter->GetStatus().Ref().nDamageCaused = 0;
	int LastGiven = m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage, LastTaken = m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage;
	m_pMyCharacter->GetStatus().Ref().nRoundLastGivenDamage = LastGiven;
	m_pMyCharacter->GetStatus().Ref().nRoundLastTakenDamage = LastTaken;
	ZPOSTLASTDMG(MCommandParameterInt(LastGiven), MCommandParameterInt(LastTaken));
	m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage = 0;
	m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage = 0;
	pCharacter->GetStatus().MakeCrc();
#ifndef _PUBLISH
	char szLog[128];
	sprintf(szLog, "ZGame::OnPeerSpawn() - %s(%u) Spawned \n", 
		pCharacter->GetProperty()->GetName(), pCharacter->GetUID().Low);
	OutputDebugString(szLog);
#endif

	// Custom: CTF
	//if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DEATHMATCH_TEAM2)
	if ( ZGetGameTypeManager()->IsTeamExtremeGame(GetMatch()->GetMatchType()) )
		pCharacter->SetInvincibleTime( 5000);
	// Custom: Spawn protection for non-team modes
	else if ( !ZGetGameTypeManager()->IsTeamGame(GetMatch()->GetMatchType()) && GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUELTOURNAMENT && GetMatch()->GetMatchType() != MMATCH_GAMETYPE_DUEL )
		pCharacter->SetInvincibleTime(2000);

}

void ZGame::OnPeerDash(MCommand* pCommand)
{
	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return;

	MUID uid = pCommand->GetSenderUID();
	ZPACKEDDASHINFO* ppdi= (ZPACKEDDASHINFO*)pParam->GetPointer();

	rvector pos, dir;
	int sel_type;
	

	pos = rvector(Roundf(ppdi->posx),Roundf(ppdi->posy),Roundf(ppdi->posz));
	dir = 1.f/32000.f * rvector(ppdi->dirx,ppdi->diry,ppdi->dirz);
	sel_type = (int)ppdi->seltype;


	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uid);

	if (pCharacter == NULL) return;

	MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;

	if( parts != pCharacter->GetItems()->GetSelectedWeaponParts()) {
		// ���� ��?E�ִ� ����� ����?E���Ⱑ Ʋ����?E����?E����� �ٲ��ش�..
		OnChangeWeapon(uid,parts);
	}
	// Custom: this time we're using zpackeddashinfo to add the dash effect(allows players to see other colors when packet is sent)
	ZGetEffectManager()->AddDashEffect(pos, dir, pCharacter, ppdi->nDashColor);

	/*
	// ���� �ƴ� ��?E?�ִϸ��̼��� ������ ������ �����Ƿ� �ٽ� �������ش�
	if(pCharacter!=m_pMyCharacter)
	{	
		// ���������� �ٲ�?E�����Ƿ� �̹� �ִ� �������� �Ǵ��Ѵ�
		rvector charDir = pCharacter->GetDirection();
		rvector charRight;
		CrossProduct(&charRight,dir,rvector(0,0,1));

		float fDotMax = 0.f , fDot;

		int nTumbleDir = 0;
		fDot=DotProduct(dir,-charDir);
		if(fDot>fDotMax) {	nTumbleDir = 1; fDotMax = fDot; }
		fDot=DotProduct(dir,charRight);
		if(fDot>fDotMax) {	nTumbleDir = 2; fDotMax = fDot; }
		fDot=DotProduct(dir,-charRight);
		if(fDot>fDotMax) {	nTumbleDir = 3; fDotMax = fDot; }

		switch(nTumbleDir)
		{
		case 0 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_FORWARD);break;
		case 1 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_BACK);break;
		case 2 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_RIGHT);break;
		case 3 :pCharacter->SetAnimationLower(ZC_STATE_LOWER_TUMBLE_LEFT);break;
		}
	}
	*/

}


//#define CHAR_COLLISION_HEIGHT	170.f


rvector ZGame::GetFloor(rvector pos, rplane *pimpactplane, MUID myUID)
{
	rvector floor=ZGetGame()->GetWorld()->GetBsp()->GetFloor(pos+rvector(0,0,120),CHARACTER_RADIUS-1.1f,58.f,pimpactplane);

#ifdef ENABLE_CHARACTER_COLLISION
	ZObjectManager::iterator itor = m_ObjectManager.begin();
	for ( ;itor != m_ObjectManager.end(); ++itor)
	{
		ZObject* pObject = (*itor).second;
		if (pObject->IsCollideable())
//		if(!pCharacter->IsDie() && !pCharacter->m_bBlastDrop)
		{
			rvector diff=pObject->GetPosition()-pos;
			diff.z=0;

			// ���߿� radius��������� �Ȱ� Object�� ��������� �����
			if(Magnitude(diff)<CHARACTER_RADIUS && pos.z>pObject->GetPosition().z)
			{
				rvector newfloor = pObject->GetPosition()+rvector(0,0,pObject->GetCollHeight());
				if(floor.z<newfloor.z)
				{
					if(m_pMyCharacter->GetUID() == myUID)
					{// �� ĳ���� �ٴ� ��ġ�� ó�����ش�.
						///< �������� ������ ��?E��ĳ�������� �ٸ� ĳ���Ͱ� ��?E������ �ؿ� ĳ���Ͱ� ������ 
						///< ��ĳ������ ���̰� ������ �ȵ�?E��ĳ���Ͱ� ������ ���� �ö󰡴� ����)
						if(CharacterOverlapCollision(pObject, floor.z, newfloor.z) == false)
							continue;
					}

					floor=newfloor;
					if(pimpactplane)
					{
						rvector up=rvector(0,0,1);
						D3DXPlaneFromPointNormal(pimpactplane,&floor,&up);
					}
				}
			}
		}
	}
#endif

	return floor;
}

bool ZGame::CharacterOverlapCollision(ZObject* pFloorObject, float WorldFloorHeight, float ObjectFloorHeight)
{
	OVERLAP_FLOOR* pOverlapObject = m_pMyCharacter->GetOverlapFloor();

	if(pOverlapObject->FloorUID != pFloorObject->GetUID())
	{ // �����?Eĳ���Ͱ� �����?E�ٽ� �������ش�.
		pOverlapObject->FloorUID = pFloorObject->GetUID();
		pOverlapObject->vecPosition.z = ObjectFloorHeight;
		pOverlapObject->nFloorCnt = 0;
		pOverlapObject->bJumpActivity = false;
	}
	else
	{ // ��� ����ִ� ĳ���� ó��
		if(pOverlapObject->bJumpActivity)
		{ // �������� �ߵ�
			if(m_pMyCharacter->GetPosition().z - WorldFloorHeight > 20.f)
			{ // ���ϳ��̰� �ٴڿ� ����?E������ OVERLAP_FLOOR �ʱ�ȭ
				pOverlapObject->FloorUID = MUID(0,0);
				pOverlapObject->nFloorCnt = 0;
				pOverlapObject->vecPosition.x = 0;
				pOverlapObject->vecPosition.y = 0;
				pOverlapObject->vecPosition.z = 0;
				pOverlapObject->bJumpActivity = false;
			}
			return false;	// �������װ� �ߵ�����?E�����?Eĳ���͸� ���������� ������
		}

		if(ObjectFloorHeight - pOverlapObject->vecPosition.z > 150.f) 
		{
			pOverlapObject->vecPosition.z = ObjectFloorHeight;
			pOverlapObject->nFloorCnt++;
			if(pOverlapObject->nFloorCnt >= 3)
			{
				pOverlapObject->bJumpActivity = true;
				mlog("Jump bug Activity \n");
				return false;
			}
		}
	}

	return true;
}

/*
rvector ZGame::GetCeiling(rvector pos)
{
rvector ceiling=g_pGame->GetWorld()->GetBsp()->GetCeiling(pos+rvector(0,0,130),CHARACTER_RADIUS-0.1f);

#ifdef ENABLE_CHARACTER_COLLISION
for (ZCharacterManager::iterator itor = m_CharacterManager.begin();
itor != m_CharacterManager.end(); ++itor)
{
ZCharacter* pCharacter = (*itor).second;
if(pCharacter!=m_pMyCharacter && !pCharacter->IsDie() && !pCharacter->m_bBlastDrop)
{
rvector diff=pCharacter->m_Position-m_pMyCharacter->m_Position;
diff.z=0;

if(Magnitude(diff)<CHARACTER_RADIUS && pos.z+CHAR_COLLISION_HEIGHT<pCharacter->m_Position.z)
{
rvector newceiling=pCharacter->m_Position;
if(ceiling.z<newceiling.z)
ceiling=newceiling;
}
}
}
#endif

return ceiling;
}
*/

bool ZGame::Pick(ZObject *pOwnerObject,rvector &origin,rvector &dir,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar)
{
	return PickHistory(pOwnerObject,GetTime(),origin,origin+10000.f*dir,pickinfo,dwPassFlag,bMyChar);
}

bool ZGame::PickTo(ZObject *pOwnerObject,rvector &origin,rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar)
{
	return PickHistory(pOwnerObject,GetTime(),origin,to,pickinfo,dwPassFlag,bMyChar);
}

// fTime �ð��� ĳ���� ��ġ�� pick �Ѵ�.. ĳ���ʹ� �Ǹ�?E����.
bool ZGame::PickHistory(ZObject *pOwnerObject,float fTime, const rvector &origin, const rvector &to,ZPICKINFO *pickinfo,DWORD dwPassFlag,bool bMyChar)
{
	pickinfo->pObject=NULL;
	pickinfo->bBspPicked=false;
	pickinfo->nBspPicked_DebugRegister=-10;

	RPickInfo info;
	memset(&info,0,sizeof(RPickInfo));

	ZObject *pObject=NULL;

	bool bCheck = false;

	float fCharacterDist=FLT_MAX;			// ĳ����?E������ �ּҰŸ������� ã�´�
	for(ZObjectManager::iterator i=m_ObjectManager.begin();i!=m_ObjectManager.end();i++)
	{
		ZObject *pc=i->second;

		bCheck = false;

		if(bMyChar) {
			if(pc==pOwnerObject && pc->IsVisible()) {
				bCheck = true;
			}
		}
		else {
			if( pc!=pOwnerObject && pc->IsVisible() ) {
				bCheck = true;
			}
		}

		if( pc->IsDie() )//�������� �����Ѵ�?E�ؼ�~
			bCheck = false;


		if(bCheck)
		{
			rvector hitPos;
			ZOBJECTHITTEST ht = pc->HitTest(origin,to,fTime,&hitPos);
			if(ht!=ZOH_NONE) {
				float fDistToChar = Magnitude(hitPos - origin);
				if (fDistToChar < fCharacterDist) {
					pObject = pc;
					fCharacterDist = fDistToChar;
					info.vOut = hitPos;
					switch(ht) {
						case ZOH_HEAD : info.parts=eq_parts_head;break;
						case ZOH_BODY : info.parts=eq_parts_chest;break;
						case ZOH_LEGS :	info.parts=eq_parts_legs;break;
					}
				}
			}
		}
	}

	RBSPPICKINFO bpi;
	bool bBspPicked = GetWorld()->GetBsp()->PickTo(origin, to, &bpi, dwPassFlag);

	int nCase=0;

	if(pObject && bBspPicked)		// �Ѵ� �¾������� �Ÿ��� ���������� ���Ѵ�.
	{
		if(Magnitude(info.vOut-origin)>Magnitude(bpi.PickPos-origin))
			nCase=1;
		else
			nCase=2;
	}else
		if(bBspPicked)				// ���� �ϳ��� �¾���?E������ ����?E�ȴ�.
			nCase=1;
		else
			if(pObject)
				nCase=2;

	if(nCase==0) return false;

	switch(nCase)
	{
	case 1 :						// �ʿ� ������?E
		pickinfo->bBspPicked=true;
		pickinfo->nBspPicked_DebugRegister = FOR_DEBUG_REGISTER;
		pickinfo->bpi=bpi;
		break;
	case 2 :						// ����� ������?E
		pickinfo->pObject=pObject;
		pickinfo->info=info;
		break;
	}
	return true;
}

bool ZGame::ObjectColTest(ZObject* pOwner, rvector& origin, rvector& to, float fRadius, ZObject** poutTarget)
{
	// �ʿ� �´°��� üũ��?E�ʴ´�.

	for(ZObjectManager::iterator i=m_ObjectManager.begin();i!=m_ObjectManager.end();i++)
	{
		ZObject *pc=i->second;

		if( pc == pOwner )
			continue;
		
		if( !pc->IsVisible() ) 
			continue; 
			
		if( pc->IsDie() )
			continue;


		if (pc->ColTest(origin, to, fRadius, GetTime()))
		{
			*poutTarget = pc;
			return true;
		}
	}

	return false;
}

char* ZGame::GetSndNameFromBsp(const char* szSrcSndName, RMATERIAL* pMaterial)
{
	char szMaterial[256] = "";
	static char szRealSndName[256] = "";
	szRealSndName[0] = 0;

	if (pMaterial == NULL) return "";

	strcpy(szMaterial, pMaterial->Name.c_str());

	size_t nLen = strlen(szMaterial);

#define ZMETERIAL_SNDNAME_LEN 7

	if ((nLen > ZMETERIAL_SNDNAME_LEN) && 
		(!strnicmp(&szMaterial[nLen-ZMETERIAL_SNDNAME_LEN+1], "mt", 2)))
	{
		strcpy(szRealSndName, szSrcSndName);
		strcat(szRealSndName, "_");
		strcat(szRealSndName, &szMaterial[nLen-ZMETERIAL_SNDNAME_LEN+1]);
	}
	else
	{
		strcpy(szRealSndName, szSrcSndName);
	}


	return szRealSndName;
}

/*
void ZGame::AdjustGlobalTime()
{
// ī��� �ٿ��Ҷ��� �ð��� ��ũ�Ѵ�
//	if(GetMatch()->GetRoundState()!=MMATCH_ROUNDSTATE_COUNTDOWN) return;
static DWORD nLastTime = GetTickTime();
DWORD nNowTime = GetTickTime();
if((nNowTime - nLastTime) < 100) return;	// 100�и������帶�� üũ

nLastTime = nNowTime;

float fAverageTime=0.f;

int nValidCount=0;

ZCharacterManager::iterator i;
for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
{
ZCharacter *pCharacter=i->second;
if(pCharacter->m_BasicHistory.size()==0) continue;		// ��ȿ��?E�ʴ�

// ĳ���Ͱ� ���������� ������ ������ ?E?
ZBasicInfoHistory::iterator infoi=pCharacter->m_BasicHistory.end();
infoi--;
ZBasicInfoItem *pInfo=*infoi;

// ������ ������ ����?E3�� �̻���?E������ �ִ�?E����. ��ȿ��?E�ʴ�
if(GetTime()-pInfo->fReceivedTime > 3.f) continue;

float fCharacterTime=pInfo->info.fSendTime+(GetTime()-pInfo->fReceivedTime);

nValidCount++;
fAverageTime+=fCharacterTime;
}

fAverageTime/=(float)nValidCount;
fAverageTime=max(fAverageTime,0);	// 0���� �������� ����.

// �۷�?E�ð����� ���̸� �����Ѵ�.
for(i=m_CharacterManager.begin();i!=m_CharacterManager.end();i++)
{
ZCharacter *pCharacter=i->second;
if(pCharacter->m_BasicHistory.size()==0) continue;		// ��ȿ��?E�ʴ�

ZBasicInfoHistory::iterator infoi=pCharacter->m_BasicHistory.end();
infoi--;
ZBasicInfoItem *pInfo=*infoi;
float fCharacterTime=pInfo->info.fSendTime+(GetTime()-pInfo->fReceivedTime);

pCharacter->m_TimeErrors[pCharacter->m_nTimeErrorCount++]=fAverageTime-fCharacterTime;
if( TIME_ERROR_CORRECTION_PERIOD == pCharacter->m_nTimeErrorCount )
{
pCharacter->m_nTimeErrorCount=0;
float fAvrTimeError=0;
for(int j=0;j<TIME_ERROR_CORRECTION_PERIOD;j++)
fAvrTimeError+=pCharacter->m_TimeErrors[j];
fAvrTimeError/=(float)TIME_ERROR_CORRECTION_PERIOD;

pCharacter->m_fAccumulatedTimeError+=fAvrTimeError*.5f;
if(fabs(pCharacter->m_fAccumulatedTimeError)>10.f)
{
#ifndef _PUBLISH
char szTemp[256];
sprintf(szTemp, "%s���� ���ǵ��� ? %3.1f", pCharacter->GetProperty()->szName,pCharacter->m_fAccumulatedTimeError);
ZGetGameInterface()->OutputChatMsg(MCOLOR(0xFFFF0000), szTemp);
#endif

pCharacter->m_fAccumulatedTimeError=0;
}

if(pCharacter==m_pMyCharacter)
{
m_fTime+=fAvrTimeError*.5f;
}
}
}

}
*/

// ?E?

void ZGame::AutoAiming()
{
	ZCamera* pCamera = ZGetGameInterface()->GetCamera();
	rvector vMyPos = m_pMyCharacter->GetPosition();
	if (ZGetMyInfo() == NULL)
		return;
	ZCharacter* pCharacter = NULL;
	rvector pos;
	rvector dir;

	for (ZCharacterManager::iterator i = m_CharacterManager.begin(); i != m_CharacterManager.end(); i++)
	{
		pCharacter = i->second;
		if (pCharacter != m_pMyCharacter) 
		{

			if (pCharacter->IsDie() == false) 
			{
				pos = pCharacter->GetPosition();
				pos.z += 140.f;
				dir = pos - RCameraPosition;
				Normalize(dir);
				pCamera->SetDirection(dir);
			}
		}
	}
}

#define MAX_PLAYERS		64


// ��ǥ�� ���� �Ǿ����Ƿ� �� �������� ������
void ZGame::PostHPAPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_HP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		ZPostHPAPInfo(m_pMyCharacter->GetHP(), m_pMyCharacter->GetAP());
	}

#ifdef ENABLE_ADJUST_MY_DATA
	//	AdjustMyData();
#endif
}

// �����ʸ�Ʈ UI�߿��� ��?E�E�������� �����ֱ⶧����
// 0.1�ʸ��� HP, AP�� �����鿡�� ������� �Ѵ�. (PostHPAPInfo()������ 1�ʿ� �ѹ��� ó������)
void ZGame::PostDuelTournamentHPAPInfo()
{
	DWORD nNowTime = GetTickTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	if ((nNowTime - m_nLastTime[ZLASTTIME_HPINFO]) >= PEER_DUELTOURNAMENT_HPAP_TICK)
	{
		m_nLastTime[ZLASTTIME_HPINFO] = nNowTime;

		BYTE MaxHP = (BYTE) m_pMyCharacter->GetMaxHP();
		BYTE MaxAP = (BYTE) m_pMyCharacter->GetMaxAP();
		BYTE HP = (BYTE) m_pMyCharacter->GetHP();
		BYTE AP = (BYTE) m_pMyCharacter->GetAP();

		ZPostDuelTournamentHPAPInfo(MaxHP, MaxAP, HP, AP);
	}
}

void ZGame::PostBasicInfo()
{
	// ���ͳ��� ������?EŰ��ǲ ó���� ��?E�ʴ´�.(���� �̾� ��?E��?E
	if(!ZGetGameInterface()->GetCombatInterface()->IsNetworkalive())
		return ;

	DWORD nNowTime = timeGetTime();

	if (m_pMyCharacter->GetInitialized() == false) return;

	// �װ�Ϫ�� 5�ʰ� ����?Ebasicinfo�� ����?E�ʴ´�.
	if(m_pMyCharacter->IsDie() && GetTime()-m_pMyCharacter->m_timeInfo.Ref().m_fDeadTime>5.f) return;

	// ������ ���Ŀ��� ����?E�ʴ´� ( global time �� ���õ�?E�ʾ�?E������ )
	//	if(m_bForceEntry) return;

	int nMoveTick = GetCharacterBasicInfoTick();

	if ((int)(nNowTime - m_nLastTime[ZLASTTIME_BASICINFO]) >= nMoveTick)
	{
		m_nLastTime[ZLASTTIME_BASICINFO] = nNowTime;

		ZPACKEDBASICINFO pbi;
		pbi.fTime = GetTime();

		pbi.posx = m_pMyCharacter->GetPosition().x;
		pbi.posy = m_pMyCharacter->GetPosition().y;
		pbi.posz = m_pMyCharacter->GetPosition().z;
		/*pbi.posx = (int(m_pMyCharacter->GetPosition().x * 0.1f)) * 10.f;
		pbi.posy = (int(m_pMyCharacter->GetPosition().y * 0.1f)) * 10.f;
		pbi.posz = (int(m_pMyCharacter->GetPosition().z * 0.1f)) * 10.f;*/	// ������ ���� ���������� ���� ��?E���� 1�� �ڸ���?E������ ��?E

		pbi.velx = m_pMyCharacter->GetVelocity().x;
		pbi.vely = m_pMyCharacter->GetVelocity().y;
		pbi.velz = m_pMyCharacter->GetVelocity().z;

		pbi.dirx = m_pMyCharacter->m_TargetDir.x*32000;
		pbi.diry = m_pMyCharacter->m_TargetDir.y*32000;
		pbi.dirz = m_pMyCharacter->m_TargetDir.z*32000;

		pbi.upperstate = m_pMyCharacter->GetStateUpper();
		pbi.lowerstate = m_pMyCharacter->GetStateLower();
		pbi.selweapon = m_pMyCharacter->GetItems()->GetSelectedWeaponParts();


		ZPOSTCMD1(MC_PEER_BASICINFO,MCommandParameterBlob(&pbi,sizeof(ZPACKEDBASICINFO)));
	}
}


void ZGame::PostPeerPingInfo()
{
	if (!ZGetGameInterface()->GetCombatInterface()->IsShowScoreBoard()) return;

	DWORD nNowTime = GetTickTime();

	if ((nNowTime - m_nLastTime[ZLASTTIME_PEERPINGINFO]) >= PEER_PING_TICK) {
		m_nLastTime[ZLASTTIME_PEERPINGINFO] = nNowTime;

		unsigned long nTimeStamp = GetTickTime();
		MMatchPeerInfoList* pPeers = ZGetGameClient()->GetPeers();
		for (MMatchPeerInfoList::iterator itor = pPeers->begin(); itor != pPeers->end(); ++itor) {
			MMatchPeerInfo* pPeerInfo = (*itor).second;
			if (pPeerInfo->uidChar != ZGetGameClient()->GetPlayerUID()) {
				_ASSERT(pPeerInfo->uidChar != MUID(0,0));

				MCommandManager* MCmdMgr = ZGetGameClient()->GetCommandManager();
				MCommand* pCmd = new MCommand(MCmdMgr->GetCommandDescByID(MC_PEER_PING), 
					pPeerInfo->uidChar, ZGetGameClient()->GetUID());	
				pCmd->AddParameter(new MCmdParamUInt(nTimeStamp));
				ZGetGameClient()->Post(pCmd);

#ifdef _DEBUG
				g_nPingCount++;
#endif
				pPeerInfo->SetLastPingTime(nTimeStamp);
			}
		}
	}
}

void ZGame::PostSyncReport()
{
	DWORD nNowTime = GetTickTime();

#ifdef _PUBLISH
	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= MATCH_CYCLE_CHECK_SPEEDHACK) {
#else
	if ((nNowTime - m_nLastTime[ZLASTTIME_SYNC_REPORT]) >= 1000/*MATCH_CYCLE_CHECK_SPEEDHACK*/) {
#endif
		m_nLastTime[ZLASTTIME_SYNC_REPORT] = nNowTime;
		int nDataChecksum = 0;
		if (m_DataChecker.UpdateChecksum() == false) {
			nDataChecksum = m_DataChecker.GetChecksum();
			ZGetApplication()->Exit();
		}

		if (ZCheckHackProcess() == true) {
			ZPostDisconnect();
		}
		ZPOSTCMD2(MC_MATCH_GAME_REPORT_TIMESYNC, MCmdParamUInt(nNowTime), MCmdParamUInt(nDataChecksum));
	}
}

// pOwner / pTarget = ��ĳ���� / ���� ĳ����
void ZGame::CheckCombo( ZCharacter *pOwnerCharacter , ZObject *pHitObject,bool bPlaySound)
{
	// �ڱⰡ ��?E����� üũ��?E����
	if(pOwnerCharacter==pHitObject) return;

	// �� ĳ���� Ȥ�� ���� �����ִ� ĳ����
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter) return;

	if(pTargetCharacter!=pOwnerCharacter) return;	// ���� �����ִ� ĳ���Ͱ� �ƴ�?Eüũ��?E����

	if(pHitObject)	// NULL ��?E������ �ø���
	{
		if(pHitObject->IsDie()) return;		// ��ü�� ���� üũ��?E����.
	}	

	if (IsPlayerObject(pHitObject))
	{
		// ?E�?E�������� combo �� ���Ե�?E����
		if(m_Match.IsTeamPlay() && (pTargetCharacter->GetTeamID()==((ZCharacter*)(pHitObject))->GetTeamID()))
			return;

		// ����Ʈ�϶��� ?E���ܺ ������?E�ʴ´�.
		if (m_Match.IsQuestDrived() || ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE) return;
	}

	UpdateCombo(true);

	if (Z_AUDIO_HITSOUND)
	{
		//		bool bHitObjectIsNPC = ((pHitObject) && (pHitObject->IsNPC()));

		//		if (!bHitObjectIsNPC)
		//		{
#ifdef _BIRDSOUND
		ZGetSoundEngine()->PlaySound("fx_myhit", 128);
#else
		if (bPlaySound)
			if (ZGetSoundEngine()->Get3DSoundUpdate())
				ZGetSoundEngine()->PlaySound("fx_myhit");
#endif
		//		}
	}
}
void ZGame::UpdateCombo(bool bShot)
{
	// �� ĳ���� Ȥ�� ���� �����ִ� ĳ����
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(!pTargetCharacter) return;

	// test
	static DWORD nLastShotTime = timeGetTime();
	DWORD nNowTime = timeGetTime();

	pTargetCharacter->GetStatus().CheckCrc();

	if (bShot) 
	{
		// Custom: Hit Maker Coded in command work By Dustin
		if (ZGetConfiguration()->GetEtc()->bhitmaker)
		{
			if (pTargetCharacter->GetStatus().Ref().nCombo >= 0)
			{
				if (ZGetGame()->m_pMyCharacter->GetItems()->GetSelectedWeaponParts() != MMCIP_MELEE)
					ZGetScreenEffectManager()->AddHitMaker();
				ZGetScreenEffectManager()->AddHit();
	}
}
		if (pTargetCharacter->GetStatus().Ref().nCombo < 2) {
			// hit ����Ʈ
			ZGetScreenEffectManager()->AddHit();
		}
		if ((nNowTime - nLastShotTime) < 700)
		{
			pTargetCharacter->GetStatus().Ref().nCombo++;
			if (pTargetCharacter->GetStatus().Ref().nCombo > MAX_COMBO) 
				pTargetCharacter->GetStatus().Ref().nCombo = 1;
		}
		nLastShotTime = nNowTime;
	}
	else
	{
		if ((pTargetCharacter->GetStatus().Ref().nCombo > 0) && ((nNowTime - nLastShotTime) > 1000))
		{
			pTargetCharacter->GetStatus().Ref().nCombo = 0;
		}
	}

	pTargetCharacter->GetStatus().MakeCrc();
}


void ZGame::CheckStylishAction(ZCharacter* pCharacter)
{

	if (pCharacter->GetStylishShoted())
	{
		if (pCharacter == m_pMyCharacter)
		{
			ZGetScreenEffectManager()->AddCool();
		}
	}
}

#define RESERVED_OBSERVER_TIME	5000

void ZGame::OnReserveObserver()
{
	unsigned long int currentTime = timeGetTime();

	if (currentTime - m_nReservedObserverTime > RESERVED_OBSERVER_TIME)
	{

		if ((m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PLAY) ||
			(m_Match.IsWaitForRoundEnd() && ZGetGameClient()->IsForcedEntry())
			)
		{
			ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
			m_bReserveObserver = false;
		}
		else
		{
			m_bReserveObserver = false;
		}

	}
}

void ZGame::ReserveObserver()
{
	m_bReserveObserver = true;
	m_nReservedObserverTime = timeGetTime();
}

void ZGame::ReleaseObserver()
{
	if(!m_bReplaying.Ref())
	{
		m_bReserveObserver = false;
		ZGetGameInterface()->GetCombatInterface()->SetObserverMode(false);

		FlushObserverCommands();
	}
}

void ZGame::OnInvalidate()
{
	GetWorld()->OnInvalidate();
	ZGetFlashBangEffect()->OnInvalidate();
	m_CharacterManager.OnInvalidate();
}

void ZGame::OnRestore()
{
	GetWorld()->OnRestore();
	ZGetFlashBangEffect()->OnRestore();
	m_CharacterManager.OnRestore();
}

void ZGame::InitRound()
{
//	m_fTime=0;
	SetSpawnRequested(false);
	ZGetGameInterface()->GetCamera()->StopShock();

	ZGetFlashBangEffect()->End();

	ZGetEffectManager()->Clear();
	m_WeaponManager.Clear();

#ifdef _WORLD_ITEM_
	//ZGetWorldItemManager()->Reset();
#endif

	ZGetCharacterManager()->InitRound();
}

void ZGame::AddEffectRoundState(MMATCH_ROUNDSTATE nRoundState, int nArg)
{

	switch(nRoundState) 
	{

	case MMATCH_ROUNDSTATE_COUNTDOWN : 
		{
			// ����Ʈ ��� - ������ ��?E?RoundStart����Ʈ ����Ѵ�. ����� ���?E��ٸ����� ����.
			if (m_Match.IsWaitForRoundEnd() && m_Match.GetMatchType() != MMATCH_GAMETYPE_DUEL && 
				m_Match.GetMatchType() != MMATCH_GAMETYPE_QUEST_CHALLENGE)
			{
				if(m_Match.GetCurrRound()+1==m_Match.GetRoundCount())
				{
					ZGetScreenEffectManager()->AddFinalRoundStart();
				}
				else
				{
					if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
					{
						// m_nCurrRound �� 0��?E1������̴�.
						ZRuleDuelTournament* pRule = (ZRuleDuelTournament*) m_Match.GetRule();
						int nRoundCount = pRule->GetDuelTournamentPlayCount();
						ZGetScreenEffectManager()->AddRoundStart(nRoundCount);
					}
					else
					{
						// m_nCurrRound �� 0��?E1������̴�.
						ZGetScreenEffectManager()->AddRoundStart(m_Match.GetCurrRound()+1);
					}
				}
			}
			if (m_Match.GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
			{
				ZRuleQuestChallenge* pCQRule = (ZRuleQuestChallenge*)m_Match.GetRule();
				if (pCQRule->GetCurrSector() + 1 == pCQRule->GetRoundMax())
				{
					ZGetScreenEffectManager()->AddFinalRoundStart();
				}
				else
				{
					ZGetScreenEffectManager()->AddRoundStart(pCQRule->GetCurrSector() + 1);
				}
			}
		}
		break;
	case MMATCH_ROUNDSTATE_PLAY:
	{
		ZGetCombatInterface()->SetFrozen(false);
		MMATCH_GAMETYPE gameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
		if (gameType == MMATCH_GAMETYPE_SPY)
		{
			ZGetScreenEffectManager()->AddScreenEffect("spy_selection");
			if (m_pMyCharacter->GetTeamID() == MMT_RED)
			{
				if (m_pMyCharacter->GetProperty()->nSex == MMS_MALE)
					ZGetGameInterface()->PlayVoiceSound("fx2/MAL01", 1500);
				else if (m_pMyCharacter->GetProperty()->nSex == MMS_FEMALE)
					ZGetGameInterface()->PlayVoiceSound("fx2/FEM01", 1500);
			}
		}
		else if (gameType == MMATCH_GAMETYPE_CTF)
		{
			ZGetGameInterface()->PlayVoiceSound(VOICE_CTF, 1600);
			ZGetScreenEffectManager()->AddScreenEffect("ctf_splash");
		}
		else
		{
			ZGetGameInterface()->PlayVoiceSound(VOICE_LETS_ROCK, 1100);
			ZGetScreenEffectManager()->AddRock();
#ifdef _LADDERUPDATE 
			if (ZGetGameClient()->IsPWChannel())
			{
				char szText[256];
				sprintf(szText, ZMsg(MSG_WORD_LADDERCOINN_MSG_START));
				ZChatOutput(ZCOLOR_CHAT_SYSTEM, szText);
			}
#endif
		}
	}
	break;
	case MMATCH_ROUNDSTATE_FINISH:
		{
			if (m_Match.IsTeamPlay())
			{
				MMatchObjCache* pObjCache = ZGetGameClient()->FindObjCache(ZGetMyUID());
				if (pObjCache && pObjCache->CheckFlag(MTD_PlayerFlags_AdminHide) == false)
				{
					char szName[125];
					sprintf(szName, "I have taken %d damage and given %d damage.", m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage, m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage);
					{
						ZChatOutput(szName, ZChat::CMT_SYSTEM);
					}
				}
				
				int nRedTeam, nBlueTeam;
				m_Match.GetTeamAliveCount(&nRedTeam, &nBlueTeam);
				
				if(nArg == MMATCH_ROUNDRESULT_RED_ALL_OUT || nArg ==  MMATCH_ROUNDRESULT_BLUE_ALL_OUT)
				{
					ZGetScreenEffectManager()->AddWin();
				}
				else if (nArg == MMATCH_ROUNDRESULT_DRAW )
				{
					// Custom: CTF
					if ( ZGetGameTypeManager()->IsTeamExtremeGame(GetMatch()->GetMatchType()) )
					{
						MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
						MMatchTeam nEnemyTeam = (nMyTeam == MMT_BLUE ? MMT_RED : MMT_BLUE);

						int nMyScore = GetMatch()->GetTeamKills(nMyTeam);
						int nEnemyScore = GetMatch()->GetTeamKills(nEnemyTeam);

						if (nMyScore > nEnemyScore)
							ZGetScreenEffectManager()->AddWin();
						else if (nMyScore < nEnemyScore)
							ZGetScreenEffectManager()->AddLose();
						else
							ZGetScreenEffectManager()->AddDraw();
					}
					else
						ZGetScreenEffectManager()->AddDraw();
				}
				else 
				{
					if (nArg == MMATCH_ROUNDRESULT_DRAW)
					{
						ZGetGameInterface()->PlayVoiceSound( VOICE_DRAW_GAME, 1200);
					}
					else {
						MMatchTeam nMyTeam = (MMatchTeam)m_pMyCharacter->GetTeamID();
						MMatchTeam nTeamWon = (nArg == MMATCH_ROUNDRESULT_REDWON ? MMT_RED : MMT_BLUE);

						// ��?E������ ���� �ٲ�?E��?E��?��?E�E
						if (ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam)
						{
							nMyTeam = NegativeTeam(nMyTeam);
						}

						// Spectator �ϰ�?Eó��
						if (ZGetGameInterface()->GetCombatInterface()->GetObserver()->IsVisible()) {
							ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();
							if (pTarget)
								nMyTeam = (MMatchTeam)pTarget->GetTeamID();
						}

						if (nTeamWon == nMyTeam)
							ZGetScreenEffectManager()->AddWin();
						else
							ZGetScreenEffectManager()->AddLose();

						if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
						{
							// Spy Mode : no sounds for Win/Lose.
						}
						if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_ASSASSINATE || GetMatch()->GetMatchType() == MMATCH_GAMETYPE_INFECTED)
						{
							if ( nTeamWon == MMT_RED)
								ZGetGameInterface()->PlayVoiceSound( VOICE_BLUETEAM_BOSS_DOWN, 2100);
							else
								ZGetGameInterface()->PlayVoiceSound( VOICE_REDTEAM_BOSS_DOWN, 2000);
						}
						else
						{
							if ( nTeamWon == MMT_RED)
								ZGetGameInterface()->PlayVoiceSound( VOICE_RED_TEAM_WON, 1400);
							else
								ZGetGameInterface()->PlayVoiceSound( VOICE_BLUE_TEAM_WON, 1400);
						}
					}
				}

				int nTeam = 0;

				// all kill ����
				for(int j=0;j<2;j++)
				{
					bool bAllKill=true;
					ZCharacter *pAllKillPlayer=NULL;

					for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin();
						itor != ZGetCharacterManager()->end(); ++itor)
					{
						ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
						if (pCharacter == NULL) return;
						
						if(j==0) {	
							nTeam = MMT_RED;
						} 
						else if(j==1) { 
							nTeam = MMT_BLUE;	
						}	

						if(pCharacter->GetTeamID() != nTeam) 
							continue;

						if(pCharacter->IsDie())
						{
							ZCharacter *pKiller = (ZCharacter*)ZGetCharacterManager()->Find(pCharacter->GetLastAttacker());
							if(pAllKillPlayer==NULL)
							{
								if(!pKiller || pKiller->GetTeamID()==nTeam)			// ���������� �׾���?E��
								{
									bAllKill=false;
									break;
								}

								pAllKillPlayer=pKiller;
							}
							else
								if(pAllKillPlayer!=pKiller)	// �������� ���� �׿���?E��
								{
									bAllKill=false;
									break;
								}
						}else
						{
							bAllKill=false;											// ?E��ִ?���� �־�ѵ ��
							break;
						}
					}
					// spy mode : no need to show allkill.
					if (GetMatch()->GetMatchType() == MMATCH_GAMETYPE_SPY)
						bAllKill = false;

					if((bAllKill) && (pAllKillPlayer))
					{
						MEMBER_SET_CHECKCRC(pAllKillPlayer->GetStatus(), nAllKill, pAllKillPlayer->GetStatus().Ref().nAllKill+1);
						pAllKillPlayer->AddIcon(ZCI_ALLKILL);
					}
				}

	/*			if(!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) 
					&& !ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
					&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL 
					&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
				{
					char szFinishStr[512];

					if (IsReplay())
					{
						ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();

						if (pTarget && pTarget->GetInitialized())
						{
							sprintf(szFinishStr, "'%s' dealt %d damage.", pTarget->GetUserName(), pTarget->m_nDamageThisRound);
							ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
						}
					}
					else if (m_pMyCharacter != NULL && !ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
					{
						sprintf(szFinishStr, "You dealt %d damage.", m_pMyCharacter->m_nDamageThisRound);
						ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
					}

					if (m_pMyCharacter && !m_pMyCharacter->IsAdminHide())
					{
						int nTotalDamageTeam = 0;
						vector<pair<ZCharacter*, int>> vScores;

						for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
						{
							ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

							if (!pCharacter->GetInitialized()) continue;
							if (pCharacter->IsAdminHide()) continue;

							if(pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID())
							{
								nTotalDamageTeam += pCharacter->m_nDamageCaused;
								vScores.push_back(pair<ZCharacter*, int>(pCharacter, pCharacter->m_nDamageCaused));
							}
						}

						sort( vScores.begin(), vScores.end(), CompareZCharInt );

						sprintf(szFinishStr, "Your team dealt %d damage in total.", nTotalDamageTeam);
						ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);

						if (vScores.size() > 0)
						{
							pair<ZCharacter*, int> p = vScores[vScores.size() - 1];
							sprintf(szFinishStr, "Your team's top scorer: %s (%d damage)", p.first->GetUserName(), p.first->m_nDamageThisRound );
							ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
						}

						vScores.clear();
					}
				}*/
			}


			// ��?E����� ��?E
			else if ( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUEL)
			{
			char szName[125];
			sprintf(szName, "I have taken %d damage and given %d damage.", m_pMyCharacter->GetStatus().Ref().nRoundTakenDamage, m_pMyCharacter->GetStatus().Ref().nRoundGivenDamage);
			ZGetGameClient()->OutputMessage(szName, MZMOM_LOCALREPLY);
				ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
				if ( pDuel)
				{
					bool bAddWin = false;
					bool bAddLose = false;
					int nCount = 0;				// è�Ǿ�?E��?E?��� �������̿���?Eüũ��?E���ؼ�...


					// ����?E����϶�
					MUID uidTarget;
					ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
					if ( pObserver && pObserver->IsVisible())
						uidTarget = pObserver->GetTargetCharacter()->GetUID();

					// ����?E��尡 �ƴҶ�
					else
						uidTarget = m_pMyCharacter->GetUID();


					for (ZCharacterManager::iterator itor = ZGetCharacterManager()->begin(); itor != ZGetCharacterManager()->end(); ++itor)
					{
						ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

						// Is champion or challenger
						if ( (pCharacter->GetUID() == pDuel->QInfo.m_uidChampion) || (pCharacter->GetUID() == pDuel->QInfo.m_uidChallenger))
						{
							if ( uidTarget == pCharacter->GetUID())
							{
								if ( pCharacter->IsDie())
									bAddLose |= true;
								else
									bAddWin |= true;
							}
							else
							{
								if ( pCharacter->IsDie())
									bAddWin |= true;
								else
									bAddLose |= true;
							}


							// è�ǿ�?E��?E?���� ��� ���ؼ� 2�� �Ǿ��� �Ѵ�
							nCount++;
						}
					}


					// Draw
					if ( (nCount < 2) || (bAddWin == bAddLose))
					{
						ZGetScreenEffectManager()->AddDraw();
						ZGetGameInterface()->PlayVoiceSound( VOICE_DRAW_GAME, 1200);
					}
					
					// Win
					else if ( bAddWin)
					{
						ZGetScreenEffectManager()->AddWin();
						ZGetGameInterface()->PlayVoiceSound( VOICE_YOU_WON, 1000);
					}

					// Lose
					else
					{
						ZGetScreenEffectManager()->AddLose();
						ZGetGameInterface()->PlayVoiceSound( VOICE_YOU_LOSE, 1300);
					}
				}
			}
			else if( ZGetGame()->GetMatch()->GetMatchType() == MMATCH_GAMETYPE_DUELTOURNAMENT )
			{
				if(!ZGetCombatInterface()->GetObserver()->IsVisible())	// �������� �ƴ�?E
				{
					float fMaxHP = ZGetGame()->m_pMyCharacter->GetMaxHP();
					float fMaxAP = ZGetGame()->m_pMyCharacter->GetMaxAP();

					float fHP = ZGetGame()->m_pMyCharacter->GetHP();					
					float fAP = ZGetGame()->m_pMyCharacter->GetAP();

					float fAccumulationDamage = ZGetGame()->m_pMyCharacter->GetAccumulationDamage();

					//ZPostDuelTournamentGamePlayerStatus(ZGetGame()->m_pMyCharacter->GetUID(), (int)(fHP*(100/fMaxHP)), (int)(fAP*(100/fMaxAP))); // ������� ����?E
					ZPostDuelTournamentGamePlayerStatus(ZGetGame()->m_pMyCharacter->GetUID(), fAccumulationDamage, fHP, fAP);

#ifndef _PUBLISH	// ���κ��忡�� ��?E?E���E���� ���
					char szAccumulationDamagePrint[256];
					sprintf(szAccumulationDamagePrint, "����?E��?%2.1f] ������ ����", fAccumulationDamage);
					ZChatOutput(MCOLOR(255, 200, 200), szAccumulationDamagePrint);

#	ifdef _DUELTOURNAMENT_LOG_ENABLE_
					mlog(szAccumulationDamagePrint);
#	endif

#endif
					// �� ����尡 ����?E��?E?E���E�ʱ�ȭ
					ZGetGame()->m_pMyCharacter->InitAccumulationDamage();
				}
			}
		}
		break;
	};

}

void ZGame::RoundEndDamage()
{
	/*if (m_Match.IsTeamPlay())
	{
		if (!ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
			&& !ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
			&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUEL
			&& ZGetGameClient()->GetMatchStageSetting()->GetGameType() != MMATCH_GAMETYPE_DUELTOURNAMENT)
		{
			char szFinishStr[512];

			if (IsReplay())
			{
				ZCharacter* pTarget = ZGetGameInterface()->GetCombatInterface()->GetObserver()->GetTargetCharacter();

				if (pTarget && pTarget->GetInitialized())
				{
					sprintf(szFinishStr, "'%s' dealt %.1f damage.", pTarget->GetUserName(), pTarget->m_fDamageCaused);
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
				}
			}
			else if (m_pMyCharacter != NULL && !ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
			{
				sprintf(szFinishStr, "You dealt %.1f damage.", m_pMyCharacter->fPersonalDamageDealtCounter);
				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
			}

			if (m_pMyCharacter && !m_pMyCharacter->IsAdminHide())
			{
				float fTotalDamageTeam = 0.f;

				vector<pair<ZCharacter*, float> > vScores;

				for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
				{
					ZCharacter* pCharacter = (ZCharacter*)(*itor).second;

					if (!pCharacter->GetInitialized()) continue;
					if (pCharacter->IsAdminHide()) continue;

					if(pCharacter->GetTeamID() == ZGetGame()->m_pMyCharacter->GetTeamID())
					{
						fTotalDamageTeam += pCharacter->m_fDamageCaused;
						vScores.push_back(pair<ZCharacter*, float>(pCharacter, pCharacter->m_fDamageCaused));
					}
				}

				sort( vScores.begin(), vScores.end(), CompareZCharFloat );

				sprintf(szFinishStr, "Your team dealt %.1f damage in total.", fTotalDamageTeam);
				ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);

				if (vScores.size() > 0)
				{
					pair<ZCharacter*, float> p = vScores[vScores.size() - 1];
					sprintf(szFinishStr, "Your team's top scorer: %s (%.1f damage)", p.first->GetUserName(), p.second );
					ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szFinishStr);
				}

				vScores.clear();
			}
		}
	}*/
}

void ZGame::StartRecording()
{
	int nsscount=0;

	char replayfilename[_MAX_PATH];
	char replayfilenameSafe[_MAX_PATH];
	char replayfoldername[_MAX_PATH];

	TCHAR szPath[MAX_PATH];
	if(GetMyDocumentsPath(szPath)) {
		strcpy(replayfoldername,szPath);
		strcat(replayfoldername,GUNZ_FOLDER);
		CreatePath( replayfoldername );
		strcat(replayfoldername,REPLAY_FOLDER);
		CreatePath( replayfoldername );
	}

	/*do {
		sprintf(replayfilename,"%s/Gunz%03d."GUNZ_REC_FILE_EXT , replayfoldername , nsscount);
		m_nGunzReplayNumber = nsscount;
		nsscount++;
	}
	while( IsExist(replayfilename) && nsscount<1000);

	if(nsscount==1000) goto RECORDING_FAIL;*/
	// ���ϸ�ܻ �Ϸù�ȣ ��Ŀ��� ������������ ������� ?E?
	SYSTEMTIME t;
	GetLocalTime( &t );
	char szCharName[MATCHOBJECT_NAME_LENGTH];
	ValidateFilename(szCharName, ZGetMyInfo()->GetCharName(), '_');

	const char* szGameTypeAcronym = "";
	char szValidatedOppoClanName[32] = "";
	int nCurrentRound = 0;
	//const char* szMapName = GetMatch()->GetMapName() ? GetMatch()->GetMapName() : "";

	bool bClanGame = ZGetGameClient()->IsCWChannel();
	
	REPLAY_STAGE_SETTING_NODE stageSettingNode;

	if (GetMatch()) {

		// ���� ��?E���� ���
		if (bClanGame) szGameTypeAcronym = "CLAN_";
		else szGameTypeAcronym = MMatchGameTypeAcronym[ GetMatch()->GetMatchType()];

		if(GetMatch()->IsTeamPlay()) nCurrentRound = GetMatch()->GetCurrRound()+1;

		// Ŭ��?E?��?E��?EŬ��?E�˾Ƴ�
		if (bClanGame) {
			const char* szOppositeClanName = "";

			if (0 == strcmp(ZGetMyInfo()->GetClanName(), ZGetCombatInterface()->GetRedClanName()) )
				szOppositeClanName = ZGetCombatInterface()->GetBlueClanName();
			else
				szOppositeClanName = ZGetCombatInterface()->GetRedClanName();

			ValidateFilename(szValidatedOppoClanName, szOppositeClanName, '_');
		}
	}

	if(GetMatch()->IsTeamPlay())
	{
		// Custom: Making recordings more readable? New format (TeamMatches): GameType[Round#]_CharName_Year-Month-Day_Hour-Min-Second
		sprintf(replayfilename, "%s[%03d]_%s_%4d-%02d-%02d_%02d-%02d-%02d%s%s", 
		szGameTypeAcronym, nCurrentRound, szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, 
		bClanGame ? "_" : "", szValidatedOppoClanName);
	}
	else
	{
		// Custom: Making recordings more readable? New format: GameType_CharName_Year-Month-Day_Hour-Min-Second
		sprintf_s(replayfilename, "%s_%s_%4d-%02d-%02d_%02d-%02d-%02d%s%s", 
		szGameTypeAcronym, szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, 
		bClanGame ? "_" : "", szValidatedOppoClanName);
	}

	//sprintf(replayfilename, "%s_%s_%4d%02d%02d_%02d%02d%02d%s%s",
	//	szGameTypeAcronym, szCharName, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond, 
	//	bClanGame ? "_" : "", szValidatedOppoClanName);

	sprintf(replayfilenameSafe, "%s_nocharname_%4d%02d%02d_%02d%02d%02d",
		szGameTypeAcronym, t.wYear, t.wMonth, t.wDay, t.wHour, t.wMinute, t.wSecond);// ĳ����?EŬ��?E?E������ �� ?E�ִ� ���ڿ��� ������ ��?E


	char szFullPath[_MAX_PATH];

	strcpy(m_szReplayFileName, replayfilename);
	sprintf(szFullPath,"%s/%s." GUNZ_REC_FILE_EXT , replayfoldername, replayfilename);
	m_pReplayFile = zfopen(szFullPath,true);
	if(!m_pReplayFile)
	{
		strcpy(m_szReplayFileName, replayfilenameSafe);
		sprintf(szFullPath,"%s/%s." GUNZ_REC_FILE_EXT , replayfoldername, replayfilenameSafe);	// ���ϸ�ҧ���� ?E������ �̸��� �ܼ�ȭ�ؼ� ��õ�
		m_pReplayFile = zfopen(szFullPath,true);
		
		if(!m_pReplayFile) goto RECORDING_FAIL;
	}

	

	int nWritten;

	DWORD header;
	header=GUNZ_REC_FILE_ID;
	nWritten = zfwrite(&header,sizeof(header),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	header=GUNZ_REC_FILE_VERSION;
	nWritten = zfwrite(&header,sizeof(header),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	ConvertStageSettingNodeForRecord(ZGetGameClient()->GetMatchStageSetting()->GetStageSetting(), &stageSettingNode);
	strcpy(stageSettingNode.szStageName, ZGetGameClient()->GetStageName());

	nWritten = zfwrite(&stageSettingNode, sizeof(REPLAY_STAGE_SETTING_NODE),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	// ����?E�� �߰����� ������?E���ð� ��?E
	if(ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUEL)
	{
		ZRuleDuel* pDuel = (ZRuleDuel*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pDuel->QInfo,sizeof(MTD_DuelQueueInfo),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;
	}
	else if(IsGameRuleCTF(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
	{
		ZRuleTeamCTF* pTeamCTF = (ZRuleTeamCTF*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();

		MTD_CTFReplayInfo info;
		info.uidCarrierRed = pTeamCTF->GetRedCarrier();
		info.uidCarrierBlue = pTeamCTF->GetBlueCarrier();
		info.posFlagRed[0] = pTeamCTF->GetRedFlagPos().x;
		info.posFlagRed[1] = pTeamCTF->GetRedFlagPos().y;
		info.posFlagRed[2] = pTeamCTF->GetRedFlagPos().z;
		info.posFlagBlue[0] = pTeamCTF->GetBlueFlagPos().x;
		info.posFlagBlue[1] = pTeamCTF->GetBlueFlagPos().y;
		info.posFlagBlue[2] = pTeamCTF->GetBlueFlagPos().z;
		info.nFlagStateRed = pTeamCTF->GetRedFlagState();
		info.nFlagStateBlue = pTeamCTF->GetBlueFlagState();

		nWritten = zfwrite(&info, sizeof(MTD_CTFReplayInfo), 1, m_pReplayFile);

		//nWritten = zfwrite(&pTeamCTF->GetRedCarrier(),sizeof(MUID),1,m_pReplayFile);
		//nWritten = zfwrite(&pTeamCTF->GetBlueCarrier(),sizeof(MUID),1,m_pReplayFile);
		//nWritten = zfwrite(&pTeamCTF->GetRedFlagPos(),sizeof(rvector),1,m_pReplayFile);
		//nWritten = zfwrite(&pTeamCTF->GetBlueFlagPos(),sizeof(rvector),1,m_pReplayFile);
		//int nRedFlagState = (int)pTeamCTF->GetRedFlagState();
		//int nBlueFlagState = (int)pTeamCTF->GetBlueFlagState();
		//nWritten = zfwrite(&nRedFlagState,sizeof(int),1,m_pReplayFile);
		//nWritten = zfwrite(&nBlueFlagState,sizeof(int),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_INFECTED)
	{
		ZRuleTeamInfected* pTeamInfected = (ZRuleTeamInfected*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pTeamInfected->m_uidPatientZero, sizeof(MUID), 1, m_pReplayFile);
		if (nWritten==0) goto RECORDING_FAIL;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_GUNGAME)
	{
		// TODO: WRITE all characters gungameweapondata
		ZRuleGunGame* pRuleGG = (ZRuleGunGame*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite(&pRuleGG->m_nWeaponMaxLevel,sizeof(int),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;

		int nCharacterCount= (int)m_CharacterManager.size();
		nWritten = zfwrite(&nCharacterCount,sizeof(nCharacterCount),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;

		for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
		{
			ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
			
			MTD_GunGameWeaponInfo rd;
			rd.uidPlayer = pCharacter->GetUID();
			memcpy(rd.nWeaponID, pCharacter->m_nGunGameWeaponID, sizeof(rd.nWeaponID));
			rd.nWeaponLevel = pCharacter->m_nGunGameWeaponLevel;

			nWritten = zfwrite((void*)&rd,sizeof(MTD_GunGameWeaponInfo),1,m_pReplayFile);
			if(nWritten==0) goto RECORDING_FAIL;
		}
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_DUELTOURNAMENT)
	{
		int nType = (int)ZGetGameInterface()->GetDuelTournamentType();
		nWritten = zfwrite(&nType,sizeof(int),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;

		const vector<DTPlayerInfo>& vecDTPlayerInfo = ZGetGameInterface()->GetVectorDTPlayerInfo();
		
		int nCount = (int)vecDTPlayerInfo.size();
		nWritten = zfwrite(&nCount,sizeof(int),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;
		
		nWritten = zfwrite((void*)&vecDTPlayerInfo[0],sizeof(DTPlayerInfo),nCount,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;

		ZRuleDuelTournament* pRule = (ZRuleDuelTournament*)ZGetGameInterface()->GetGame()->GetMatch()->GetRule();
		nWritten = zfwrite((void*)&pRule->m_DTGameInfo,sizeof(MTD_DuelTournamentGameInfo),1,m_pReplayFile);
		if(nWritten==0) goto RECORDING_FAIL;
	}
	else if (ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_SPY)
	{
		ZRuleSpy* pSpy = (ZRuleSpy*)m_Match.GetRule();

		int nSpyItemCount = (int)pSpy->m_vtLastSpyItem.size();
		nWritten = zfwrite(&nSpyItemCount, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		nWritten = zfwrite((void*)&pSpy->m_vtLastSpyItem[0], sizeof(MMatchSpyItem), nSpyItemCount, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		int nTrackerItemCount = (int)pSpy->m_vtLastTrackerItem.size();
		nWritten = zfwrite(&nTrackerItemCount, sizeof(int), 1, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;

		nWritten = zfwrite((void*)&pSpy->m_vtLastTrackerItem[0], sizeof(MMatchSpyItem), nTrackerItemCount, m_pReplayFile);
		if (nWritten == 0) goto RECORDING_FAIL;
	}
	int nCharacterCount= (int)m_CharacterManager.size();
	nWritten = zfwrite(&nCharacterCount,sizeof(nCharacterCount),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;

	for (ZCharacterManager::iterator itor = m_CharacterManager.begin(); itor != m_CharacterManager.end(); ++itor)
	{
		ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
		if(!pCharacter->Save(m_pReplayFile)) goto RECORDING_FAIL;
	}	

	//// Custom: CQ
	//for (auto itor = ZGetObjectManager()->begin(); itor != ZGetObjectManager()->end(); ++itor)
	//{
	//	ZActorWithFSM* pObj = (ZActorWithFSM*)(*itor).second;
	//	if (!pObj->Save(m_pReplayFile)) goto RECORDING_FAIL;

	//}

	//nWritten = zfwrite(&m_fTime,sizeof(m_fTime),1,m_pReplayFile);
	float fTime = m_fTime.Ref();
	nWritten = zfwrite(&fTime,sizeof(float),1,m_pReplayFile);
	if(nWritten==0) goto RECORDING_FAIL;


	m_bRecording=true;
	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), 
		ZMsg(MSG_RECORD_STARTING));
	return;

RECORDING_FAIL:	// ����

	if(m_pReplayFile)
	{
		zfclose(m_pReplayFile);
		m_pReplayFile = NULL;
	}

	ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
}
void ZGame::StopRecording()
{
	if(!m_bRecording) return;

	bool bError = false;

	m_bRecording=false;

	ZObserverCommandList::iterator itr = m_ReplayCommandList.begin();
	for(size_t i=0;i<m_ReplayCommandList.size();i++)
	{
		ZObserverCommandItem *pItem = *itr;
		MCommand *pCommand = pItem->pCommand;

		const int BUF_SIZE = 1024;
		char CommandBuffer[BUF_SIZE];
		int nSize = pCommand->GetData(CommandBuffer, BUF_SIZE);

		int nWritten;
		nWritten = zfwrite(&pItem->fTime,sizeof(pItem->fTime),1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }
		nWritten = zfwrite(&pCommand->m_Sender,sizeof(pCommand->m_Sender),1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }
		nWritten = zfwrite(&nSize,sizeof(nSize),1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }
		nWritten = zfwrite(CommandBuffer,nSize,1,m_pReplayFile);
		if(nWritten==0) { bError=true; break; }

		itr++;
	}

	while(m_ReplayCommandList.size())
	{
		ZObserverCommandItem *pItem = *m_ReplayCommandList.begin();
		delete pItem->pCommand;
		delete pItem;
		m_ReplayCommandList.pop_front();
	}

	if(!zfclose(m_pReplayFile))
		bError = true;

	if(bError)
	{
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), ZMsg(MSG_RECORD_CANT_SAVE));
	}
	else
	{
		char szOutputFilename[256];
		sprintf(szOutputFilename,GUNZ_FOLDER REPLAY_FOLDER "/%s." GUNZ_REC_FILE_EXT , m_szReplayFileName );

		char szOutput[256];
		// ZTranslateMessage(szOutput,MSG_RECORD_SAVED,1,szOutputFilename);
		ZTransMsg(szOutput,MSG_RECORD_SAVED,1,szOutputFilename);
		ZChatOutput(MCOLOR(ZCOLOR_CHAT_SYSTEM), szOutput);
	}

}



void ZGame::ToggleRecording()
{
	if(m_bReplaying.Ref()) return;	// ����� ?E��Ұ� -_-;

	// ����Ʈ�� ?E���?E�ʴ´�
	// Custom: Add CQ to this
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType())
		|| ZGetGameTypeManager()->IsQuestChallengeOnly(ZGetGameClient()->GetMatchStageSetting()->GetGameType()))
		return;

	if(!m_bRecording)
		StartRecording();
	else
		StopRecording();
}

DWORD dwReplayStartTime;

bool ZGame::OnLoadReplay(ZReplayLoader* pLoader)
{
	//m_fTime = pLoader->GetGameTime();
	m_fTime.Set_CheckCrc(pLoader->GetGameTime());

	m_bReplaying.Set_CheckCrc(true);
	SetReadyState(ZGAME_READYSTATE_RUN);
	GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_FREE);
	ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);
	ZGetGameInterface()->GetCombatInterface()->GetObserver()->SetTarget(ZGetGame()->m_pMyCharacter->GetUID());
	g_bProfile=true;	
	dwReplayStartTime=timeGetTime();





/*
	size_t n;

	m_bReplaying=true;

	int nCharacterCount;
	zfread(&nCharacterCount,sizeof(nCharacterCount),1,file);

	ZGetCharacterManager()->Clear();
	m_ObjectManager.Clear();

	for(int i=0;i<nCharacterCount;i++)
	{
		bool bHero;
		n=zfread(&bHero,sizeof(bHero),1,file);
		if(n!=1) return false;

		MTD_CharInfo info;

		if(nVersion<2) {
			n=zfread(&info,sizeof(info)-4,1,file);
			if(n!=1) return false;
			info.nClanCLID = 0;
		}
		else {
			n=zfread(&info,sizeof(info),1,file);
			if(n!=1) return false;
		}

		ZCharacter *pChar=NULL;
		if(bHero)
		{
			m_pMyCharacter=new ZMyCharacter;
			CreateMyCharacter(&info);
			pChar=m_pMyCharacter;
			pChar->Load(file,nVersion);
		}else
		{
			pChar=new ZNetCharacter;
			pChar->Load(file,nVersion);
			pChar->Create(&info);
		}

		ZGetCharacterManager()->Add(pChar);
		mlog("%s : %d %d\n",pChar->GetProperty()->szName,pChar->GetUID().High,pChar->GetUID().Low);

		pChar->SetVisible(true);
	}

	float fGameTime;
	zfread(&fGameTime,sizeof(fGameTime),1,file);
	m_fTime=fGameTime;

	int nCommandCount=0;

	int nSize;
	float fTime;
	while( zfread(&fTime,sizeof(fTime),1,file) )
	{
		nCommandCount++;

		char CommandBuffer[1024];

		MUID uidSender;
		zfread(&uidSender,sizeof(uidSender),1,file);
		zfread(&nSize,sizeof(nSize),1,file);
		if(nSize<0 || nSize>sizeof(CommandBuffer)) {
			m_bReplaying=false;
			ShowReplayInfo( true);
			return false;
		}
		zfread(CommandBuffer,nSize,1,file);

		ZObserverCommandItem *pZCommand=new ZObserverCommandItem;
		pZCommand->pCommand=new MCommand;
		pZCommand->pCommand->SetData(CommandBuffer,ZGetGameClient()->GetCommandManager());
		pZCommand->pCommand->m_Sender=uidSender;
		pZCommand->fTime=fTime;
		m_ReplayCommandList.push_back(pZCommand);

	}

	SetReadyState(ZGAME_READYSTATE_RUN);
	GetMatch()->SetRoundState(MMATCH_ROUNDSTATE_FREE);
	ZGetGameInterface()->GetCombatInterface()->SetObserverMode(true);

	ZGetGameInterface()->GetCombatInterface()->GetObserver()->SetTarget(m_pMyCharacter->GetUID());

	g_bProfile=true;	

	dwReplayStartTime=timeGetTime();

	return true;
*/
	return true;
}

void ZGame::EndReplay()
{
	g_bProfile=false;

	DWORD dwReplayEndTime=timeGetTime();
	
	mlog("replay end. profile saved. playtime = %3.3f seconds , average fps = %3.3f \n", 
		float(dwReplayEndTime-dwReplayStartTime)/1000.f,
		1000.f*g_nFrameCount/float(dwReplayEndTime-dwReplayStartTime));


	// ���÷��̰� �� ����?E�ٽ� ó������ ��������. - (��?E
	if (ZGetGameInterface()->GetPreviousState() == GUNZ_LOGIN || !ZGetGameClient()->IsConnected())
	{
		ZChangeGameState(GUNZ_LOGIN);
	}
	else
		ZChangeGameState(GUNZ_LOBBY);
}

void ZGame::ConfigureCharacter(const MUID& uidChar, MMatchTeam nTeam, unsigned char nPlayerFlags)
{
	ZCharacterManager* pCharMgr = ZGetCharacterManager();
	ZCharacter* pChar = (ZCharacter*) pCharMgr->Find(uidChar);
	if (pChar == NULL) return;

	pChar->SetAdminHide((nPlayerFlags & MTD_PlayerFlags_AdminHide) !=0);
	pChar->SetTeamID(nTeam);
	pChar->InitStatus();
	pChar->InitRound();

	ZGetCombatInterface()->OnAddCharacter(pChar);
}

void ZGame::RefreshCharacters()
{
	for (MMatchPeerInfoList::iterator itor = ZGetGameClient()->GetPeers()->begin();
		itor != ZGetGameClient()->GetPeers()->end(); ++itor)
	{
		MMatchPeerInfo* pPeerInfo = (*itor).second;
		ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(pPeerInfo->uidChar);

		if (pCharacter == NULL) {

			pCharacter = m_CharacterManager.Add(pPeerInfo->uidChar, rvector(0.0f, 0.0f, 0.0f));
			pCharacter->Create(&pPeerInfo->CharInfo);

			if (m_Match.GetRoundState() == MMATCH_ROUNDSTATE_PREPARE)
			{
				if (m_Match.IsTeamPlay())
				{
//					pCharacter->SetVisible(true);		// RAONHAJE: PeerOpened TEST
				}
			}


			/*
			// TODO: AGENT ?E?peer�鿡�Դ� ������. �������.

			//���� ĳ���Ϳ��� �ڽ��� ���⸦ �˸���...

			ZCharacter* pMyCharacter = g_pGame->m_pMyCharacter;
			if(pMyCharacter)
			{
				//			if(pMyCharacter != pCharacter) { // �ڽ��� ���� ��?Eĳ���� �ƴ϶�?E
				int nParts = g_pGame->m_pMyCharacter->GetItems()->GetSelectedWeaponParts();
				ZPostChangeWeapon(nParts);
				//			}
			}
			*/
		}
	}
}

void ZGame::DeleteCharacter(const MUID& uid)
{
	bool bObserverDel = false;
	ZCharacter* pCharacter = (ZCharacter*) ZGetCharacterManager()->Find(uid);

	// ����?EŸ���� ��?E�ٸ� Ÿ������ �ٲ��ش�.
	ZObserver* pObserver = ZGetGameInterface()->GetCombatInterface()->GetObserver();
	if (pObserver->IsVisible())
	{
		if ((pCharacter != NULL) && (pCharacter == pObserver->GetTargetCharacter()))
		{
			bObserverDel = true;				
		}
	}

	m_CharacterManager.Delete(uid);


	if (bObserverDel)
	{
		if (pObserver) pObserver->SetFirstTarget();
	}
}


void ZGame::OnStageEnterBattle(MCmdEnterBattleParam nParam, MTD_PeerListNode* pPeerNode)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	MUID uidChar = pPeerNode->uidChar;

	if (uidChar == ZGetMyUID())		// enter�ѻ���� ���ڽ��� ��?E
	{
#ifdef _ZPEERANTIHACK
		ZPeerAntiHack.clear();
#endif
		if (ZGetGame()->CreateMyCharacter(&pPeerNode->CharInfo/*, &pPeerNode->CharBuffInfo*/) == true)
		{
			ConfigureCharacter(uidChar, (MMatchTeam)pPeerNode->ExtendInfo.nTeam, pPeerNode->ExtendInfo.nPlayerFlags);	// Player Character ����
		}
	}
	else							// enter�ѻ���� �� �ڽ��� �ƴҰ�?E
	{
		if (ZGetGameClient()->m_bAdminNAT)
			OnAddPeer(pPeerNode->uidChar, 0, 0, pPeerNode);
		else
			OnAddPeer(pPeerNode->uidChar, pPeerNode->dwIP, pPeerNode->nPort, pPeerNode);
	}


	if (nParam == MCEP_FORCED)
	{
		ZCharacter* pChar = (ZCharacter*) ZGetCharacterManager()->Find(uidChar);
		GetMatch()->OnForcedEntry(pChar);

		char temp[256] = "";
		if((pPeerNode->ExtendInfo.nPlayerFlags & MTD_PlayerFlags_AdminHide)==0) {
			ZTransMsg(temp, MSG_GAME_JOIN_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}
#ifdef _REPLAY_TEST_LOG
		mlog("[Add Character %s(%d)]\n", pChar->GetCharInfo()->szName, uidChar.Low);
#endif
	}

	ZGetGameClient()->OnStageEnterBattle(uidChar, nParam);
}

void ZGame::OnStageLeaveBattle(const MUID& uidChar, const bool bIsRelayMap)//, const MUID& uidStage)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;

	// Custom: Prevents recordings from not getting saved if you quit a room.
	if(ZGetGame()->m_bRecording && uidChar == ZGetMyUID()) ZGetGame()->ToggleRecording();

	if (uidChar != ZGetMyUID()) {

		ZCharacter* pChar = (ZCharacter*) ZGetCharacterManager()->Find(uidChar);

		if(pChar && !pChar->IsAdminHide() && !bIsRelayMap) {
			char temp[256] = "";
			ZTransMsg(temp, MSG_GAME_LEAVE_BATTLE, 1, pChar->GetUserAndClanName());
			ZChatOutput(MCOLOR(ZCOLOR_GAME_INFO), temp);
		}

		ZGetGameClient()->DeletePeer(uidChar);
		if (ZApplication::GetGameInterface()->GetState() == GUNZ_GAME) {
			DeleteCharacter(uidChar);
		}

		ZGetGameClient()->SetVoteInProgress( false );
		ZGetGameClient()->SetCanVote( false );
	}
}

void ZGame::OnAddPeer(const MUID& uidChar, DWORD dwIP, const int nPort, MTD_PeerListNode* pNode)
{
	if ((ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) || (ZGetGame() == NULL)) return;

	/*
	//// UDPTEST LOG ////////////////////////////////
	char szLog[256];
	sprintf(szLog, "[%d:%d] ADDPEER: Char(%d:%d) IP:%s, Port:%d \n", 
	GetPlayerUID().High, GetPlayerUID().Low, uidChar.High, uidChar.Low, szIP, nPort);
	mlog(szLog);
	/////////////////////////////////////////////////
	*/

	// �߰��� ����� ��?E�ڽ��� �ƴ�?E
	if (uidChar != ZGetMyUID())
	{
		if (pNode == NULL) {
			_ASSERT(0);
		}

		ZGetGameClient()->DeletePeer(uidChar);	// Delete exist info

		MMatchPeerInfo* pNewPeerInfo = new MMatchPeerInfo;

		if (uidChar == MUID(0,0))	pNewPeerInfo->uidChar = MUID(0, nPort);	// �����׽�Ʈ�� ���ؼ�
		else						pNewPeerInfo->uidChar = uidChar;

		in_addr addr;
		addr.s_addr = dwIP;
		char* pszIP = inet_ntoa(addr);
		strcpy(pNewPeerInfo->szIP, pszIP);
		
		pNewPeerInfo->dwIP  = dwIP;
		pNewPeerInfo->nPort = nPort;

		if (!IsReplay())
			memcpy(&pNewPeerInfo->CharInfo, &(pNode->CharInfo), sizeof(MTD_CharInfo));	
		else
		{
			MTD_CharInfo currInfo;
			ConvertCharInfo(&currInfo, &pNode->CharInfo, ZReplayLoader::m_nVersion);
			memcpy(&pNewPeerInfo->CharInfo,	&currInfo, sizeof(MTD_CharInfo));	
		}
		//���������ӽ��ּ� memcpy(&pNewPeerInfo->CharBuffInfo, &(pNode->CharBuffInfo), sizeof(MTD_CharBuffInfo));			
		memcpy(&pNewPeerInfo->ExtendInfo,	&(pNode->ExtendInfo),	sizeof(MTD_ExtendInfo));

		ZGetGameClient()->AddPeer(pNewPeerInfo);	

		RefreshCharacters();
	}

	ConfigureCharacter(uidChar, (MMatchTeam)pNode->ExtendInfo.nTeam, pNode->ExtendInfo.nPlayerFlags);	// Player Character ����
}

void ZGame::OnPeerList(const MUID& uidStage, void* pBlob, int nCount)
{
	if (ZGetGameClient()->GetStageUID() != uidStage) return;
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	if ((ZGetGame() == NULL) || (ZGetCharacterManager() == NULL)) return;

	for(int i=0; i<nCount; i++) {
		MTD_PeerListNode* pNode = (MTD_PeerListNode*)MGetBlobArrayElement(pBlob, i);
		OnAddPeer(pNode->uidChar, pNode->dwIP, pNode->nPort, pNode);

		ZCharacter* pChar = (ZCharacter*) ZGetCharacterManager()->Find(pNode->uidChar);
		if (pChar) {
			pChar->SetVisible(false);
		}
	}
}

void ZGame::PostMyBuffInfo()
{
	// ���� �ߵ���?E�ִ� ���� ���¸� �Ǿ��鿡�� �˷��ش�
	if (m_pMyCharacter)
	{
		void* pBlob = m_pMyCharacter->MakeBuffEffectBlob();
		if (pBlob)
		{
			ZPostBuffInfo(pBlob);
			MEraseBlobArray(pBlob);
		}
	}
}

void ZGame::OnPeerBuffInfo(const MUID& uidSender, void* pBlobBuffInfo)
{
	if (uidSender == ZGetMyUID()) return;

	ZCharacter* pSender = (ZCharacter*) ZGetCharacterManager()->Find(uidSender);
	if (!pSender) return;
	if (!pBlobBuffInfo) return;

	MTD_BuffInfo* pBuffInfo = NULL;
	int numElem = MGetBlobArrayCount(pBlobBuffInfo);

	// Custom: Exploit fix (MTD_PeerBuffInfo blob overflow)
    if (MGetBlobArraySize(pBlobBuffInfo) != (8 + (sizeof(MTD_BuffInfo) * numElem)))
	{
        return;
    }

	for (int i=0; i<numElem; ++i)
	{
		pBuffInfo = (MTD_BuffInfo*)MGetBlobArrayElement(pBlobBuffInfo, i);

		ApplyPotion(pBuffInfo->nItemId, pSender, (float)pBuffInfo->nRemainedTime);
	}
}

void ZGame::OnGameRoundState(const MUID& uidStage, int nRound, int nRoundState, int nArg)
{
	if (ZApplication::GetGameInterface()->GetState() != GUNZ_GAME) return;
	ZMatch* pMatch = GetMatch();
	if (pMatch == NULL) return;

	MMATCH_ROUNDSTATE RoundState = MMATCH_ROUNDSTATE(nRoundState);

	// ��������ϰ�?E
	if ((RoundState == MMATCH_ROUNDSTATE_FREE) && (pMatch->GetRoundState() != RoundState))
	{
		pMatch->InitCharactersPosition();
		m_pMyCharacter->SetVisible(true);
		m_pMyCharacter->Revival();
		ReleaseObserver();
	}


	pMatch->SetRound(nRound);
	pMatch->SetRoundState(RoundState, nArg);
	AddEffectRoundState(RoundState, nArg);

	if (RoundState == MMATCH_ROUNDSTATE_FINISH)
	{
#ifdef _VMPROTECT
VMProtectBeginUltra("RoundStateFinishValidImage");
		if (!VMProtectIsValidImageCRC())
		{
			ZPostBanMe(WHT_MEMCMP);
			if (!g_ShouldBanPlayer)
				g_ShouldBanPlayer = timeGetTime();
		}
VMProtectEnd();
#endif
	}
}


bool ZGame::FilterDelayedCommand(MCommand *pCommand)
{
	bool bFiltered = true;
	float fDelayTime = 0;

	MUID uid=pCommand->GetSenderUID();
	ZCharacter *pChar = (ZCharacter*) ZGetCharacterManager()->Find(uid);
	if(!pChar) return false;

	switch (pCommand->GetID())
	{
		case MC_PEER_SKILL:
			{
				int nSkill;
				pCommand->GetParameter(&nSkill, 0, MPT_INT);
				fDelayTime = .15f;
				switch(nSkill)	{
					case ZC_SKILL_UPPERCUT		: 
						if(pChar!=m_pMyCharacter) pChar->SetAnimationLower(ZC_STATE_LOWER_UPPERCUT);
						break;
					case ZC_SKILL_SPLASHSHOT	: break;
					case ZC_SKILL_DASH			: break;
				}

				////////////////////////////////////////////////////////////////////
				int sel_type;
				pCommand->GetParameter(&sel_type, 2, MPT_INT);
				MMatchCharItemParts parts = (MMatchCharItemParts)sel_type;
				if( parts != pChar->GetItems()->GetSelectedWeaponParts()) {
					// ���� ��?E�ִ� ����� ����?E���Ⱑ Ʋ����?E����?E����� �ٲ��ش�..
					OnChangeWeapon(uid,parts);
				}
			}break;

		case MC_PEER_SHOT:
			{
				MCommandParameter* pParam = pCommand->GetParameter(0);
				if(pParam->GetType()!=MPT_BLOB) break;	// ������ �ִ�
				ZPACKEDSHOTINFO *pinfo =(ZPACKEDSHOTINFO*)pParam->GetPointer();

				// Į���� �����̰� �ִ�
				if(pinfo->sel_type!=MMCIP_MELEE) return false;

				if(pChar!=m_pMyCharacter &&
					( pChar->m_pVMesh->GetSelectWeaponMotionType()==eq_wd_dagger ||
					pChar->m_pVMesh->GetSelectWeaponMotionType()==eq_ws_dagger )) { // dagger
						pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
					}

				fDelayTime = .15f;

				////////////////////////////////////////////////////////////////////
				MMatchCharItemParts parts = (MMatchCharItemParts)pinfo->sel_type;
				if( parts != pChar->GetItems()->GetSelectedWeaponParts()) {
					// ���� ��?E�ִ� ����� ����?E���Ⱑ Ʋ����?E����?E����� �ٲ��ش�..
					OnChangeWeapon(uid,parts);
				}
				///////////////////////////////////////////////////////////////////////////////
			}
			break;

		// ���� �߰��� �������� Ŀ��?E
		case MC_PEER_SHOT_MELEE:
			{
				float fShotTime;
				rvector pos;
				int nShot;

				pCommand->GetParameter(&fShotTime, 0, MPT_FLOAT);
				pCommand->GetParameter(&pos, 1, MPT_POS);
				pCommand->GetParameter(&nShot, 2, MPT_INT);

				if(pChar!=m_pMyCharacter &&
					( pChar->m_pVMesh->GetSelectWeaponMotionType()==eq_wd_dagger ||
					pChar->m_pVMesh->GetSelectWeaponMotionType()==eq_ws_dagger )) { // dagger
						pChar->SetAnimationUpper(ZC_STATE_UPPER_SHOT);
					}

				fDelayTime = .1f;
				switch(nShot) {
					case 1 : fDelayTime = .10f;break;
					case 2 : fDelayTime = .15f;break;
					case 3 : fDelayTime = .2f;break;
					case 4 : fDelayTime = .25f;break;
				}


				if ( nShot > 1)
				{
					char szFileName[ 20];
					if ( pChar->GetProperty()->nSex == MMS_MALE)
						sprintf( szFileName, "fx2/MAL_shot_%02d", nShot);
					else
						sprintf( szFileName, "fx2/FEM_shot_%02d", nShot);

					ZGetSoundEngine()->PlaySound( szFileName, pChar->GetPosition());
				}
			}
			break;

		case MC_QUEST_PEER_NPC_ATTACK_MELEE :	  
			ZGetQuest()->OnPrePeerNPCAttackMelee(pCommand);
			fDelayTime = .4f;
			break;


		// ?E����� ���͸��� �ȵ�?E�̰� delayed command�� �ƴϴ�
		default:
			bFiltered = false;
			break;
	}

	if(bFiltered)
	{
		ZObserverCommandItem *pZCommand=new ZObserverCommandItem;
		pZCommand->pCommand=pCommand->Clone();
		pZCommand->fTime=GetTime()+fDelayTime;
		m_DelayedCommandList.push_back(pZCommand);
		return true;
	}

	return false;
}

void ZGame::PostSpMotion(ZC_SPMOTION_TYPE mtype)
{
	if(m_pMyCharacter==NULL) return;
	// Custom: Added MMATCH_ROUNDSTATE_FREE to allowed emotion list
	if(m_Match.GetRoundState() != MMATCH_ROUNDSTATE_PLAY && m_Match.GetRoundState() != MMATCH_ROUNDSTATE_FREE) return;

	// Custom: Macro spam fix
	if(ZGetGameInterface()->GetCombatInterface()->GetObserverMode())
		return;

	if( (m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE1) || 
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE2) || 
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE3) || 
		(m_pMyCharacter->m_AniState_Lower.Ref() == ZC_STATE_LOWER_IDLE4) ) 
	{

		MMatchWeaponType type = MWT_NONE;

		ZItem* pSItem = m_pMyCharacter->GetItems()->GetSelectedWeapon();

		if( pSItem && pSItem->GetDesc() ) {
			type = pSItem->GetDesc()->m_nWeaponType.Ref();
		}

		if( mtype == ZC_SPMOTION_TAUNT ) // taunt �� ��?E����� ����ح...
			if( (type == MWT_MED_KIT) || 
				(type == MWT_REPAIR_KIT) || 
				(type == MWT_FOOD) ||
				(type == MWT_LANDMINE_SPY) ||
				(type == MWT_BULLET_KIT)) 
			{
				return;
			}

		ZPostSpMotion(mtype);
	}
}

void ZGame::OnEventUpdateJjang(const MUID& uidChar, bool bJjang)
{
	ZCharacter* pCharacter = (ZCharacter*) m_CharacterManager.Find(uidChar);
	if (pCharacter == NULL) return;

	if (bJjang) 
		ZGetEffectManager()->AddStarEffect(pCharacter);    

}

bool ZGame::CanAttack(ZObject *pAttacker, ZObject *pTarget)
{
	//### �� �Լ��� ������?E�Ȱ��� CanAttack_DebugRegister()���� ��?E?�־��� �մϴ�. ###
	if(!IsReplay())
		if(GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;
	if(pAttacker==NULL) return true;

	// Custom: Silly Gunz crash fix
	if (pTarget == NULL)
		return false;

	if ( GetMatch()->IsTeamPlay() ) {
		// Custom: Element crash bug fix. Not a really nice fix
		__try
		{
			if (pAttacker->GetTeamID() == pTarget->GetTeamID()) {
				if (!GetMatch()->GetTeamKillEnabled()) 
					return false;
			}
		}
		__except(EXCEPTION_EXECUTE_HANDLER) // GetExceptionCode() == EXCEPTION_ACCESS_VIOLATION 
		{
			mlog("MERROR_1\n");
			return false;
		}
	}

#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
		MMATCH_GAMETYPE_QUEST_CHALLENGE == ZGetGameClient()->GetMatchStageSetting()->GetGameType())
	{
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			return false;
		}
	}

#endif
	return true;
}

//jintriple3 ����� �������� ��ŷ ������ ���� ISAttackable()�Լ��� �ٸ� �̸����� �ϳ� ?E�������...
bool ZGame::CanAttack_DebugRegister(ZObject *pAttacker, ZObject *pTarget)
{
	if(!IsReplay())
		if(GetMatch()->GetRoundState() != MMATCH_ROUNDSTATE_PLAY) return false;
	if(pAttacker==NULL) return true;

	// Custom: Silly Gunz crash fix
	if (pTarget == NULL)
		return false;

	if ( GetMatch()->IsTeamPlay() ) {
		if (pAttacker->GetTeamID() == pTarget->GetTeamID()) {
			if (!GetMatch()->GetTeamKillEnabled()) 
				return false;
		}
	}

#ifdef _QUEST
	if (ZGetGameTypeManager()->IsQuestDerived(ZGetGameClient()->GetMatchStageSetting()->GetGameType()) ||
		MMATCH_GAMETYPE_QUEST_CHALLENGE == ZGetGameClient()->GetMatchStageSetting()->GetGameType())
	{
		if (pAttacker->GetTeamID() == pTarget->GetTeamID())
		{
			return false;
		}
	}

#endif
	return true;
}



void ZGame::ShowReplayInfo( bool bShow)
{
	MWidget* pWidget = ZGetGameInterface()->GetIDLResource()->FindWidget( "CombatChatOutput");
	if ( pWidget)
		pWidget->Show( bShow);

	m_bShowReplayInfo = bShow;
}

void ZGame::OnLocalOptainSpecialWorldItem(MCommand* pCommand)
{
	int nWorldItemID;
	pCommand->GetParameter(&nWorldItemID   , 0, MPT_INT);

	switch (nWorldItemID)
	{
	case WORLDITEM_PORTAL_ID:
		{
			MMATCH_GAMETYPE eGameType = ZGetGameClient()->GetMatchStageSetting()->GetGameType();
			if (!ZGetGameTypeManager()->IsQuestDerived(eGameType) || ZGetGameClient()->GetMatchStageSetting()->GetGameType() == MMATCH_GAMETYPE_QUEST_CHALLENGE) break;

			// ������ ��Ż�� �̵��Ѵ�?E?E?
			char nCurrSectorIndex = ZGetQuest()->GetGameInfo()->GetCurrSectorIndex();
			ZPostQuestRequestMovetoPortal(nCurrSectorIndex);
		}
		break;
	};
}


void ZGame::ReserveSuicide( void)
{
	m_bSuicide = true;
}


bool ZGame::OnRuleCommand(MCommand* pCommand)
{

	if (m_Match.GetMatchType() == MMATCH_GAMETYPE_QUEST_CHALLENGE)
	{
		return m_Match.OnCommand(pCommand);
	}

#ifdef _QUEST
	if (ZGetQuest()->OnGameCommand(pCommand)) return true;
#endif

	switch (pCommand->GetID())
	{
	case MC_MATCH_ASSIGN_COMMANDER:
	case MC_MATCH_ASSIGN_BERSERKER:
	case MC_MATCH_FLAG_EFFECT:
	case MC_MATCH_FLAG_CAP:
	case MC_MATCH_FLAG_STATE:
	case MC_MATCH_INFECT:
	case MC_MATCH_LASTSURVIVOR:
	case MC_MATCH_GUNGAME_WEAPONDATA:
	case MC_MATCH_GAME_DEAD:
	case MC_MATCH_DUEL_QUEUEINFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_INFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO:
	case MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO:

		// MagicBox
	case MC_DROPGUNGAME_RESPONSE_ENTERGAME:
	case MC_DROPGUNGAME_RESPONSE_ITEM:
	case MC_DROPGUNGAME_RESPONSE_WORLDITEMS:

	case MC_SPY_GAME_INFO:
	case MC_SPY_GAME_RESULT:
	case MC_MATCH_SPY_ROUNDRESULT:

		{
			if (m_Match.OnCommand(pCommand)) return true;
		};
	};

	return false;
}

void ZGame::OnResetTeamMembers(MCommand* pCommand)
{
	if (!m_Match.IsTeamPlay()) return;

	if (m_Match.GetMatchType() != MMATCH_GAMETYPE_INFECTED)
		ZChatOutput( MCOLOR(ZCOLOR_GAME_INFO), ZMsg(MSG_GAME_MAKE_AUTO_BALANCED_TEAM) );

	MCommandParameter* pParam = pCommand->GetParameter(0);
	if(pParam->GetType()!=MPT_BLOB) return;
	void* pBlob = pParam->GetPointer();
	int nCount = MGetBlobArrayCount(pBlob);

	ZCharacterManager* pCharMgr = ZGetCharacterManager();

	for (int i = 0; i < nCount; i++)
	{
		MTD_ResetTeamMembersData* pDataNode = (MTD_ResetTeamMembersData*)MGetBlobArrayElement(pBlob, i);

		ZCharacter* pChar = (ZCharacter*) pCharMgr->Find(pDataNode->m_uidPlayer);
		if (pChar == NULL) continue;

		if (pChar->GetTeamID() != ( (MMatchTeam)pDataNode->nTeam) )
		{
			// ��?E���ڽ��� �������� �Ǿ���?E������Ǿ������� �����.
			if (pDataNode->m_uidPlayer == ZGetMyUID())
			{
				ZGetMyInfo()->GetGameInfo()->bForcedChangeTeam = true;
			}

			pChar->SetTeamID((MMatchTeam)pDataNode->nTeam);
		}

	}
}


void ZGame::MakeResourceCRC32( const DWORD dwKey, DWORD& out_crc32, DWORD& out_xor )
{
	out_crc32 = 0;
	out_xor = 0;

#ifdef _DEBUG
	static DWORD dwOutputCount = 0;
	++dwOutputCount;
#endif

	MMatchObjCacheMap* pObjCacheMap = ZGetGameClient()->GetObjCacheMap();
	if( NULL == pObjCacheMap )
	{
		return ;
	}

	MMatchObjCacheMap::const_iterator	end			= pObjCacheMap->end();
	MMatchObjCacheMap::iterator			it			= pObjCacheMap->begin();
	MMatchObjCache*						pObjCache	= NULL;
	MMatchItemDesc*						pitemDesc	= NULL;
	MMatchCRC32XORCache					CRC32Cache;

	CRC32Cache.Reset();
	CRC32Cache.CRC32XOR( dwKey );

#ifdef _DEBUG
	mlog( "Start ResourceCRC32Cache : %u\n", CRC32Cache.GetXOR() );
#endif

	for( ; end != it; ++it )
	{
		pObjCache = it->second;

		for( int i = 0; i < MMCIP_END; ++i )
		{
			pitemDesc = MGetMatchItemDescMgr()->GetItemDesc( pObjCache->GetCostume()->nEquipedItemID[i] );
			if( NULL == pitemDesc )
			{
				continue;
			}

			pitemDesc->CacheCRC32( CRC32Cache );
			
#ifdef _DEBUG
			if( 10 > dwOutputCount )
			{
				mlog( "ItemID : %d, CRCCache : %u\n"
					, pitemDesc->m_nID
					, CRC32Cache.GetXOR() );
			}
#endif
		}
	}

#ifdef _DEBUG
	if( 10 > dwOutputCount )
	{
		mlog( "ResourceCRCSum : %u\n", CRC32Cache.GetXOR() );
	}
#endif
	
	out_crc32 = CRC32Cache.GetCRC32();
	out_xor = CRC32Cache.GetXOR();
}

void ZGame::OnResponseUseSpendableBuffItem(MUID& uidItem, int nResult)
{
	// TodoH(?E - ��?E??E?��?Eó��
}

/*
void ZGame::OnGetSpendableBuffItemStatus(MUID& uidChar, MTD_CharBuffInfo* pCharBuffInfo)
{
	if (uidChar != ZGetMyUID()) {
		_ASSERT(0);
		return;
	}

	if( pCharBuffInfo == NULL ) {
		_ASSERT(0);
		return;
	}

	ZGetMyInfo()->SetCharBuffInfo(pCharBuffInfo);

	if( m_pMyCharacter != NULL ) {
		m_pMyCharacter->SetCharacterBuff(pCharBuffInfo);
	}
}*/

void ZGame::ApplyPotion(int nItemID, ZCharacter* pCharObj, float fRemainedTime)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if( pDesc == NULL ) { _ASSERT(0);  return; }

	MMatchDamageType nDamageType = pDesc->m_nDamageType.Ref();

	if (nDamageType == MMDT_HASTE)
	{
		// ���� ������
		ZModule_Movable* pMod = (ZModule_Movable*)pCharObj->GetModule(ZMID_MOVABLE);
		if (pMod)
		{
			if (fRemainedTime == 0)
				fRemainedTime = pDesc->m_nDamageTime.Ref()*0.001f;

			pMod->SetMoveSpeedHasteRatio(pDesc->m_nItemPower.Ref()*0.01f, fRemainedTime, nItemID);
		}
		ZGetEffectManager()->AddHasteBeginEffect(pCharObj->GetPosition(), pCharObj);
	}
	else if (nDamageType == MMDT_HEAL || nDamageType == MMDT_REPAIR)
	{
		// ?E?ȸ�� ������
		if (pDesc->m_nDamageTime.Ref() == 0)
		{
			ZGetEffectManager()->AddPotionEffect( pCharObj->GetPosition(), pCharObj, pDesc->m_nEffectId );

			if (nDamageType == MMDT_HEAL)
			{
				int nAddedHP = pDesc->m_nItemPower.Ref();
				pCharObj->SetHP( min( pCharObj->GetHP() + nAddedHP, pCharObj->GetMaxHP() ) );
			}
			else if (nDamageType == MMDT_REPAIR)
			{
				int nAddedAP = pDesc->m_nItemPower.Ref();
				pCharObj->SetAP( min( pCharObj->GetAP() + nAddedAP, pCharObj->GetMaxAP() ) );
			}
			else
				_ASSERT(0);
		}
		// ������Ÿ�� ������
		else
		{
			ZModule_HealOverTime* pMod = (ZModule_HealOverTime*)pCharObj->GetModule(ZMID_HEALOVERTIME);
			if (pMod)
			{
				int nRemainedHeal = (int)fRemainedTime;
				if (nRemainedHeal == 0)
					nRemainedHeal = pDesc->m_nDamageTime.Ref();

				//damagetime�� ?EȽ���� ǥ���� ��
				pMod->BeginHeal(pDesc->m_nDamageType.Ref(), pDesc->m_nItemPower.Ref(), nRemainedHeal, pDesc->m_nEffectId, nItemID);
			}

			switch (nDamageType)
			{
			case MMDT_HEAL:
				ZGetEffectManager()->AddHealOverTimeBeginEffect(pCharObj->GetPosition(), pCharObj);
				break;
			case MMDT_REPAIR:
				ZGetEffectManager()->AddRepairOverTimeBeginEffect(pCharObj->GetPosition(), pCharObj);
				break;
			}
		}
	}
	else
		_ASSERT(0);
}

void ZGame::OnUseTrap(int nItemID, ZCharacter* pCharObj, rvector& pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if( pDesc == NULL ) { _ASSERT(0); return; }

	rvector velocity;
	velocity	= /*pCharObj->GetVelocity()+ */pCharObj->m_TargetDir * 1300.f;
	velocity.z  = velocity.z + 300.f;
	m_WeaponManager.AddTrap(pos, velocity, nItemID, pCharObj);
}

void ZGame::OnUseSpyTrap(int nItemID, ZCharacter* pCharObj, rvector& pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { _ASSERT(0); return; }

	rvector velocity;
	velocity = /*pCharObj->GetVelocity()+ */pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddSpyTrap(pos, velocity, nItemID, pCharObj);
}

void ZGame::OnUseDynamite(int nItemID, ZCharacter* pCharObj, rvector& pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if( pDesc == NULL ) { _ASSERT(0); return; }

	rvector velocity;
	velocity	= /*pCharObj->GetVelocity()+ */pCharObj->m_TargetDir * 1300.f;
	velocity.z  = velocity.z + 300.f;
	m_WeaponManager.AddDynamite(pos, velocity, pCharObj);
}

void ZGame::CheckZoneTrap(MUID uidOwner,rvector pos,MMatchItemDesc* pItemDesc, MMatchTeam nTeamID)
{
	if (!pItemDesc) return;

	float fRange = 300.f;

	ZObject* pTarget = NULL;
	// Custom: Changed ZCharacter to ZObject, fixes cq npc's damaging eachother.
	ZObject* pOwnerObject = (ZObject*) m_ObjectManager.GetObject( uidOwner );

	if(!pOwnerObject)
		return;

	float fDist;
	bool bReturnValue;

	for(ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor) 
	{
		pTarget = (*itor).second;

		// �� Ʈ����?E���� ����
#ifndef _DEBUG	// ����� ���忡 ���ؼ� �׽�Ʈ�� ���� ��?E���� �� Ʈ���� ���ظ� �޵��� �Ѵ�
		bReturnValue = pTarget->GetUID() == uidOwner;
		if (pTarget->GetUID() == uidOwner)
			PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;
#endif

		bReturnValue = CanAttack( pOwnerObject, pTarget);
		if ( !bReturnValue)
			PROTECT_DEBUG_REGISTER(!CanAttack_DebugRegister(pOwnerObject, pTarget))
			continue;

		//jintriple3 ����� �������� ��ŷ ?E?����.....
		bReturnValue = !pTarget || pTarget->IsDie();
		if( !pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

		// ĳ������ �߸�?E��� ��?Eüũ�� �Ѵ�
		fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,10)));
		//jintriple3 ����� �������� �� ?E?����.....
		bReturnValue = fDist >=fRange;
		if(fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
				continue;

		if (pos.z > pTarget->GetPosition().z + pTarget->GetCollHeight())
			continue;

		// Ʈ�� ��?E��?E�?ĳ������ �߸���̿� ���� �ִ�?EȮ��
		/*{
			const DWORD dwPickPassFlag = RM_FLAG_ADDITIVE | RM_FLAG_HIDE | RM_FLAG_PASSROCKET | RM_FLAG_PASSBULLET;
			RBSPPICKINFO bpi;
			rvector orig = pos+rvector(0,0,fRange);
			rvector to = pTarget->GetPosition()+rvector(0,0,10);
			bool bBspPicked = GetWorld()->GetBsp()->PickTo(orig, to, &bpi, dwPickPassFlag);
			if(bBspPicked)
			{
				if (Magnitude(to-orig) > Magnitude(bpi.PickPos-orig))
					continue;
			}
		}// ����Ʈ�� ���� �վ��µ� ���ظ� ���Դ°� ?E�̻��ϴ�?E�ؼ� �ּ�*/

		//ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");

		if (m_pGameAction)
		{
			int nDuration = pItemDesc->m_nDamageTime.Ref();
			bool bApplied = false;
			switch (pItemDesc->m_nDamageType.Ref())
			{
			case MMDT_FIRE:
				// ��Ʈ���� ItemPower�� ?E����?����
				bApplied = m_pGameAction->ApplyFireEnchantDamage(pTarget, pOwnerObject, pItemDesc->m_nItemPower.Ref(), nDuration);
				break;
			case MMDT_COLD:
				// ����Ʈ���� ItemPower�� �̼Ӱ��ҷ��� ���� (80��?E80%�� �ӷ��� ��)
				bApplied = m_pGameAction->ApplyColdEnchantDamage(pTarget, pItemDesc->m_nItemPower.Ref(), nDuration);
				break;
			default:
				_ASSERT(0);
			}

			if (bApplied)
			{
				//removed npc check, checkcombo already does that
			//	if (pOwnerCharacter->IsNPC()) continue;
				ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
				if (pOwnerCharacter)
				{
					CheckCombo(pOwnerCharacter, pTarget, true);	//todok ����带 �������?E;
					CheckStylishAction(pOwnerCharacter);
				}
			}
		}
	}
	// ���ӿ� ���� ���� ����� ����� �� ��Ƣ?Eȿ���� ����Ű��
	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

void ZGame::OnExplosionDynamite(MUID uidOwner, rvector pos, float fDamage, float fRange, float fKnockBack, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;

	float fDist;


	ZObject* pOwnerObject = (ZObject*) m_ObjectManager.GetObject( uidOwner );

	if(!pOwnerObject)
		return;

	for(ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor) 
	{
		pTarget = (*itor).second;

		bool bReturnValue = !pTarget || pTarget->IsDie();
		if( !pTarget || pTarget->IsDie())
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		fDist = Magnitude(pos-(pTarget->GetPosition()+rvector(0,0,80)));
		bReturnValue = fDist >= fRange;
		if(fDist >= fRange)
			PROTECT_DEBUG_REGISTER(bReturnValue)
			continue;

		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);

		// ���̳ʸ���Ʈ�� ����źó�� �ݵ����� Ƣ��Ϫ����.
		ZActor* pATarget = MDynamicCast(ZActor,pTarget);
		ZActorWithFSM* pFSMActor = MDynamicCast(ZActorWithFSM, pTarget);
		bool bPushSkip = false;

		if(pATarget) 
		{
			bPushSkip = pATarget->GetNPCInfo()->bNeverPushed;
		}
		if (pFSMActor)
		{
			bPushSkip = pFSMActor->GetActorDef()->IsNeverBlasted();
		}

		if(bPushSkip==false)
		{
			pTarget->AddVelocity(fKnockBack*7.f*(fRange-fDist)*-dir);
		}
		else 
		{
			ZGetSoundEngine()->PlaySound("fx_bullethit_mt_met");
		}
		ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, false);
			CheckStylishAction(pOwnerCharacter);
		}

		// ���̳ʸ���Ʈ�� ���� ?E?������ ��� ���� �������� ������.
		float fRatio = ZItem::GetPiercingRatio( MWT_DYNAMITYE, eq_parts_chest );
		pTarget->OnDamaged(pOwnerObject, pos, ZD_EXPLOSION, MWT_DYNAMITYE, fDamage, fRatio);
	}

#define SHOCK_RANGE		1500.f			// 10���ͱ�?E��鸰��

	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(pTargetCharacter)
	{
		float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition()+rvector(0,0,50) - pos)) / SHOCK_RANGE;

		if(fPower>0)
			ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	}
	GetWorld()->GetWaters()->CheckSpearing( pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f );
}

int ZGame::GetCharacterBasicInfoTick()
{
	return (ZGetGameClient()->GetAllowTunneling() == false) ? PEERMOVE_TICK : PEERMOVE_AGENT_TICK;
}

MUID ZGame::GetMyUid()
{
	return ZGetGameClient()->GetPlayerUID();
}

bool ZGame::PickWorld(const rvector &pos, const rvector &dir, RBSPPICKINFO *pOut, DWORD dwPassFlag)
{
	RBspObject* r_map = GetWorld()->GetBsp();
	return r_map->Pick((D3DXVECTOR3&)pos, (D3DXVECTOR3&)dir, pOut, dwPassFlag);
}


ZNavigationMesh ZGame::GetNavigationMesh()
{
	return ZNavigationMesh( GetWorld()->GetBsp()->GetNavigationMesh());
}

void ZGame::OnUseStunGrenade(int nItemID, ZCharacter* pCharObj, rvector& pos)
{
	MMatchItemDesc* pDesc = MGetMatchItemDescMgr()->GetItemDesc(nItemID);
	if (pDesc == NULL) { _ASSERT(0); return; }

	rvector velocity;
	velocity = pCharObj->m_TargetDir * 1300.f;
	velocity.z = velocity.z + 300.f;
	m_WeaponManager.AddStunGrenade(pos, velocity, pCharObj);
}

void ZGame::OnExplosionStunGrenade(MUID uidOwner, rvector pos, float fDamage, float fRange, float fKnockBack, MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;
	float fDist;

	ZObject* pOwnerObject = (ZObject*) m_ObjectManager.GetObject( uidOwner );

	if(!pOwnerObject)
		return;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			continue;
		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		bReturnValue = fDist >= fRange;
		if (fDist >= fRange)
			continue;
		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);
		ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, false);
			CheckStylishAction(pOwnerCharacter);
		}

		float fRatio = ZItem::GetPiercingRatio(MWT_STUNGRENADE, eq_parts_chest);
		pTarget->OnDamaged(pOwnerObject, pos, ZD_EXPLOSION, MWT_STUNGRENADE, fDamage, fRatio);
		pTarget->OnDamagedAnimation(pTarget, ZD_KATANA_SPLASH);
	}
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if(pTargetCharacter)
	{
		float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

		if (fPower>0)
			ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}

void ZGame::CheckZoneItemKit(MUID uidOwner, rvector pos,float fDamage,MMatchTeam nTeamID)
{
	ZObject* pTarget = NULL;
	float fDist;

	ZObject* pOwnerObject = (ZObject*)m_ObjectManager.GetObject(uidOwner);

	if (!pOwnerObject)
		return;

	for (ZObjectManager::iterator itor = m_ObjectManager.begin(); itor != m_ObjectManager.end(); ++itor)
	{
		pTarget = (*itor).second;

		bool bReturnValue = !pTarget || pTarget->IsDie();
		if (!pTarget || pTarget->IsDie())
			continue;
		fDist = Magnitude(pos - (pTarget->GetPosition() + rvector(0, 0, 80)));
		rvector dir = pos - (pTarget->GetPosition() + rvector(0, 0, 80));
		Normalize(dir);
		ZCharacter* pOwnerCharacter = ZGetGame()->m_CharacterManager.Find(uidOwner);
		if (pOwnerCharacter)
		{
			CheckCombo(pOwnerCharacter, pTarget, false);
			CheckStylishAction(pOwnerCharacter);
		}

		pTarget->OnDamaged(pOwnerObject, pos, ZD_EXPLOSION, MWT_LANDMINE, fDamage, 0);
		if (pTarget == ZGetGame()->m_pMyCharacter)
		{
			ZGetGame()->m_pMyCharacter->OnBlast(rvector(pTarget->GetDirection()));
			ZGetGame()->m_pMyCharacter->SetVelocity(rvector(0, 0, 0));
		}
	//	pTarget->OnDamagedAnimation(pTarget, ZD_KATANA_SPLASH);
	}
	ZCharacter *pTargetCharacter = ZGetGameInterface()->GetCombatInterface()->GetTargetCharacter();
	if (pTargetCharacter)
	{
		float fPower = (SHOCK_RANGE - Magnitude(pTargetCharacter->GetPosition() + rvector(0, 0, 50) - pos)) / SHOCK_RANGE;

		if (fPower>0)
			ZGetGameInterface()->GetCamera()->Shock(fPower*500.f, .5f, rvector(0.0f, 0.0f, -1.0f));
	}

	GetWorld()->GetWaters()->CheckSpearing(pos, pos + rvector(0, 0, MAX_WATER_DEEP), 500, 0.8f);
}
void ZGame::BerserkerKI()
{
	if (!ZGetGame()) return;

	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin(); itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		ZGetEffectManager()->Clear();
		ZCharacter* pCharacter = (ZCharacter*)(*itor).second;
		pCharacter->SetTagger(false);
	}

	ZCharacter* pBerserkerKIChar = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(ZGetMyUID());
	if (pBerserkerKIChar)
	{
		ZGetEffectManager()->AddBerserkerIconKI(pBerserkerKIChar);
		pBerserkerKIChar->SetTagger(true);
	}
}
void ZGame::GetJjang()
{
	if (!ZGetGame()) return;

	for (ZCharacterManager::iterator itor = ZGetGame()->m_CharacterManager.begin(); itor != ZGetGame()->m_CharacterManager.end(); ++itor)
	{
		ZGetEffectManager()->Clear();
	}
	ZCharacter* pCharacter = (ZCharacter*)ZGetGame()->m_CharacterManager.Find(ZGetMyUID());
	if (pCharacter)
	{
		ZGetEffectManager()->AddStarEffect(pCharacter);
	}

}