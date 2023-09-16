#pragma once
/////////////////////
//  Toggle system  //
//  By Jorklenis2  //
/////////////////////

// Enabled Settings
#define _SPYMODE 1
#define _SPYLADDER 1
#define _LIMITMAP 1
#define _2PCLAN	1
#define _SYSTEMUPDATETIME (30 * 60 * 1000)
//#define _UPDATECHAR (1 * 60 * 1000)
#define _SRVRPNG 1
#define _MULTILANGUAGE 1
#define _COUNTRYFLAG 1
#define _KILLSTREAK 1
#define _STAFFCHAT 1
#define _AVATAR_ENABLE 1
#define _QUEST 1
#define _QUEST_ITEM	1
#define _MONSTER_BIBLE 1
#define _DUELTOURNAMENT	1
#define _CW_VOTE 1
#define _LADDERGAME 1
#define _LADDERGAMETEST 1
#define _VIPGRADES 1
#define _EVENTGRD 1
#define _NEWGRADE 1
#define _SHOPFIXCANT 1
#define _REPORT 1
#define _AGENTPORT 1
#define _GRADECHANGE 1
#define _EXPCHANNEL 1
#define _CHANNELNEW 1
#define _UDPCUSTOM 1
#define _CUSTOMRULEMAPS 1
#define _DATAINFOHPAP 1
#define _VOICE_CHAT 1
//#define _RECOMMANDEDTEAM 1
//#define _REGISTERLOGIN 1

#define _AGENT_IP     "127.0.0.1"

#define _MIPING	1
//#define _ANTILEECH 1
//#define _SERVER_FALLBACK 1


//#define _RANDOMNOTICE 1
//#define _RANDOMNOTICETIME (30 * 60 * 1000)
//#define _RANDOMNOTICEMSG1 "In case your medals are not reflected, it is recommended that you close your session and start again."
//#define _RANDOMNOTICEMSG2 "En caso de que no es reflejado tus medallas es recomendable que cierres sesion y vuelvas a iniciar."
//#define _RANDOMNOTICEMSG3 "For every victories you get in Streak Ladder you get 10, the more you accumulate you can spend in the medal shop."
//#define _RANDOMNOTICEMSG4 "Por cada victorias que obtengas en Streak Ladder te llevas 10, entre mas acumules podras gastar en la tienda de medallas."
//#define _RANDOMNOTICEMSG5 "New Anti-Lead System Add"
//#define _RANDOMNOTICEMSG6 "Classification System has been Updated!"
//#define _RANDOMNOTICEMSG7 "Streak Gunz has a totally stable and comfortable server and client to play."
//#define _RANDOMNOTICEMSG8 "Streak Gunz cuenta con un servidor y cliente totalmente estable y comodo para jugar."
//#define _RANDOMNOTICEMSG9 "좪l sistema de clasificaci? ha sido actualizado!"
//#define _RANDOMNOTICEMSG10 "Every weekend items are added to accounts and the Kill System for Medals."
//#define _RANDOMNOTICEMSG11 "Todo los fin de semana se agrega elementos a las cuentas y Sistema de Kill por Medallas."

//#define _FORCE_EVENT_TEAM 1
//#define _ROOMHIDE 1

//#define _EVENTCMD 1
//#define _EXPLOITCLIENT 1
//#define _LVL200 1
//#define _ANTISQL 1
//#define _RANK 1
//#define _UPREWARDLEVEL 1

#if defined(_DEBUG) || defined(_RELEASE) || defined(LOCALE_KOREA) || defined(LOCALE_NHNUSA)// || defined(LOCALE_JAPAN) || defined(LOCALE_BRAZIL)
#endif

//#if defined(_DEBUG) || defined(_RELEASE) || defined(LOCALE_KOREA)
#if 0
#	define _SELL_CASHITEM	// 캐쉬 아이템 되팔기 기능
#endif