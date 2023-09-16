#ifndef _MMATCHMAP_H
#define _MMATCHMAP_H

#include "MMatchGlobal.h"

enum MMATCH_MAP
{
	MMATCH_MAP_MANSION = 0,
	MMATCH_MAP_PRISON = 1,
	MMATCH_MAP_STATION = 2,
	MMATCH_MAP_PRISON_II = 3,
	MMATCH_MAP_BATTLE_ARENA = 4,
	MMATCH_MAP_TOWN = 5,
	MMATCH_MAP_DUNGEON = 6,
	MMATCH_MAP_RUIN = 7,
	MMATCH_MAP_ISLAND = 8,
	MMATCH_MAP_GARDEN = 9,
	MMATCH_MAP_CASTLE = 10,
	MMATCH_MAP_FACTORY = 11,
	MMATCH_MAP_PORT = 12,
	MMATCH_MAP_LOST_SHRINE = 13,
	MMATCH_MAP_STAIRWAY = 14,
	MMATCH_MAP_SNOWTOWN = 15,
	MMATCH_MAP_HALL = 16,
	MMATCH_MAP_CATACOMB = 17,
	MMATCH_MAP_JAIL = 18,
	MMATCH_MAP_SHOWERROOM = 19,
	MMATCH_MAP_HIGH_HAVEN = 20,
	MMATCH_MAP_CITADEL = 21,
	MMATCH_MAP_RELAYMAP = 22,
	MMATCH_MAP_HALLOWEEN_TOWN = 23,
	MMATCH_MAP_WEAPON_SHOP = 24,
	MMATCH_MAP_Ering = 25,
	MMATCH_MAP_de_dust2 = 26,
	MMATCH_MAP_StevensTDM = 27,
	MMATCH_MAP_SUPERFLIP = 28,
	MMATCH_MAP_Skillmap = 29,
	MMATCH_MAP_Skillmapv2 = 30,
	MMATCH_MAP_skillmapv3 = 31,
	MMATCH_MAP_skillmapv4 = 32,
	MMATCH_MAP_DusteRSkillTrailv1 = 33,
	MMATCH_MAP_Dusterskilltrail_v2 = 34,
	MMATCH_MAP_Dusterskilltrail_v3 = 35,
	MMATCH_MAP_dusterskilltrail_v4 = 36,
	MMATCH_MAP_GatoSkillTrailv1 = 37,
	MMATCH_MAP_GatoSkillTrailv2 = 38,
	MMATCH_MAP_GatoSkillTrailv3 = 39,
	MMATCH_MAP_HeroSkillV1 = 40,
	MMATCH_MAP_HeroSkillV2 = 41,
	MMATCH_MAP_HeroSkillV3 = 42,
	MMATCH_MAP_HeroSkillV4 = 43,
	MMATCH_MAP_HeroSkillV5 = 44,
	MMATCH_MAP_HighSkillV1 = 45,
	MMATCH_MAP_HighSkillV2 = 46,
	MMATCH_MAP_NewDungeon = 47,
	MMATCH_MAP_Test_A = 48,
	MMATCH_MAP_Test_B = 49,
	MMATCH_MAP_BlitzKrieg = 50,
	MMATCH_MAP_AlphaSkillTrail_V1 = 51,
	MMATCH_MAP_AtackDefense = 52,
	MMATCH_MAP_FinishLine = 53,
	MMATCH_MAP_GhostTrailsv1 = 54,
	MMATCH_MAP_Labyrinth = 55,
	MMATCH_MAP_MasterskillV1 = 56,
	MMATCH_MAP_OmegaV2 = 57,
	MMATCH_MAP_DusapposeRace = 58,
	MMATCH_MAP_MasterSkillerv3 = 59,
	MMATCH_MAP_SkillKingV3 = 60,
	MMATCH_MAP_SkillMapV16 = 61,
	MMATCH_MAP_SkillMapv19 = 62,
	MMATCH_MAP_supermario3 = 63,
	MMATCH_MAP_CreationsTrail = 64,
	MMATCH_MAP_GodSkill_Trail_V1 = 65,
	MMATCH_MAP_SkillMapGold = 66,
	MMATCH_MAP_Elementary = 67,
	MMATCH_MAP_DBZ = 68,
	MMATCH_MAP_Algebra = 69,
	MMATCH_MAP_Athena = 70,
	MMATCH_MAP_Snow_Dust_Dew = 71,

	MMATCH_MAP_BackSkillMapV1 = 72,
	MMATCH_MAP_MasterSkillV2 = 73,
	MMATCH_MAP_MatthewSkillTrailV1 = 74,
	MMATCH_MAP_SkillFlashV1 = 75,
	MMATCH_MAP_SkillmapAirstrip = 76,
	MMATCH_MAP_SkillMapv37 = 77,
	MMATCH_MAP_ZmarTzMapV2 = 78,

	MMATCH_MAP_Dasher = 79,
	MMATCH_MAP_MC_Mastery_V1 = 80,
	MMATCH_MAP_ReflectSkillZ = 81,
	MMATCH_MAP_ShowedTrailsV1 = 82,
	MMATCH_MAP_SkillGalaxyV2 = 83,
	//CQ Maps 
	MMATCH_MAP_CQ_1_GUERRILLA = 84,
	MMATCH_MAP_CQ_2_GUERRILLA = 85,
	MMATCH_MAP_CQ_3_GUERRILLA = 86,
	MMATCH_MAP_CQ_4_GUERRILLA = 87,

	MMATCH_MAP_CQ_5_RESEARCH = 88,
	MMATCH_MAP_CQ_6_RESEARCH = 89,
	MMATCH_MAP_CQ_7_RESEARCH = 90,
	MMATCH_MAP_CQ_8_RESEARCH = 91,
	//Blitzkrieg BETA TEST
	MMATCH_MAP_CQ_8_BLITZ = 92,

	// New Maps Add V2
	MMATCH_MAP_ASkillTrailv1 = 93,
	MMATCH_MAP_BlackSkillZ_V1 = 94,
	MMATCH_MAP_ChallengeMapV2 = 95,
	MMATCH_MAP_ChampionsMapV1 = 96,
	MMATCH_MAP_ExtremeSkillV1 = 97,
	MMATCH_MAP_Master_Skiller = 98,
	MMATCH_MAP_FootBall = 99,
	MMATCH_MAP_Nicos_Skillz = 100,
	MMATCH_MAP_Nicos_skillzV2 = 101,
	MMATCH_MAP_Omega = 102,
	MMATCH_MAP_SkillCityMap = 103,
	MMATCH_MAP_SkillmapCleanV1 = 104,
	MMATCH_MAP_TheCouple = 105,
	MMATCH_MAP_ZSkillV1 = 106,

	// Map News Version 2.0 Add
	MMATCH_MAP_CHRIS_TakeIt = 107,
	MMATCH_MAP_SkillEasyMap1 = 108,
	MMATCH_MAP_SkillMapDigital = 109,
	MMATCH_MAP_SkillMapFire = 110,
	MMATCH_MAP_SkillMapv21 = 111,
	MMATCH_MAP_SkillMapv46 = 112,

#ifdef _LIMITMAP
	MMATCH_MAP_MAX
#else
	MMATCH_MAP_MAX = 120
#endif
};

#define MMATCH_MAP_COUNT	MMATCH_MAP_MAX	

#define MMATCH_MAPNAME_RELAYMAP				"RelayMap"


class MMapDesc
{
private:
	const struct MapInfo
	{
		int			nMapID;							// map id
		char		szMapName[MAPNAME_LENGTH];		// ¸Ê ÀÌ¸§
		char		szMapImageName[MAPNAME_LENGTH];	// ¸Ê ÀÌ¹ÌÁö ÀÌ¸§
		char		szBannerName[MAPNAME_LENGTH];	// º£³Ê ÀÌ¸§
		float		fExpRatio;						// °æÇèÄ¡ ½Àµæ·ü
		int			nMaxPlayers;					// ÃÖ´ë ÀÎ¿ø
		bool		bOnlyDuelMap;					// µà¾ó¸Ê Àü¿ë
		bool		bIsCTFMap;

#ifdef _CUSTOMRULEMAPS
		bool		bIsSWRMap;
#endif

	};


	// data
	MapInfo	m_MapVectors[MMATCH_MAP_COUNT];
	MMapDesc();
public:
	~MMapDesc() { }
	
	static MMapDesc* GetInstance();

	bool Initialize(const char* szFileName);
	bool Initialize(MZFileSystem* pfs, const char* szFileName);
	bool MIsCorrectMap(const int nMapID);
	

 
	bool IsMapOnlyDuel( const int nMapID);
	bool IsCTFMap( const int nMapID);

#ifdef _CUSTOMRULEMAPS
	bool IsSWRMap(const int nMapID);
#endif

	int GetMapID( const int nMapID);
	const char* GetMapName(const int nMapID);
	const char* GetMapImageName(const char* szMapName);
	const char* GetBannerName(const char* szMapName);
	float GetExpRatio( const int nMapID); 
	int GetMaxPlayers( const int nMapID);

	int GetMapCount(){ return MMATCH_MAP_COUNT; }
};


inline MMapDesc* MGetMapDescMgr() 
{ 
	return MMapDesc::GetInstance();
}



#endif