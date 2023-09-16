#ifndef MSHAREDCOMMANDTABLE_H
#define MSHAREDCOMMANDTABLE_H

class MCommandManager;


#define MCOMMAND_VERSION	59 // Custom: Updated CMD version



#define MSCT_MASTER			1
#define MSCT_CLIENT			2
#define MSCT_MATCHSERVER	4
#define MSCT_AGENT			8
#define MSCT_CHECKER		16
#define MSCT_ALL			(MSCT_MASTER+MSCT_CLIENT+MSCT_AGENT)


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ���� : Ŀ�ǵ�EID�� �����ϸ�E������ ������ ���÷��� ������ �������E���� ��E�ֽ��ϴ�.
//
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/// ��EĿ�´������Ͱ� �����ϴ� Ŀ�ǵ�E
void MAddSharedCommandTable(MCommandManager* pCommandManager, int nSharedType);


/// ����EŬ���̾�Ʈ ���� ��E
#define MATCH_CYCLE_CHECK_SPEEDHACK	20000


// �⺻ Ŀ�ǵ�E����Ʈ
#define MC_HELP						0
#define MC_VERSION					1

#define MC_DEBUG_TEST				100

#define MC_LOCAL_INFO				201		///< Local ������ ��´�.
#define MC_LOCAL_ECHO				202		///< Local Echo �׽�Ʈ
#define MC_LOCAL_LOGIN				203		///< Login ó�� Local Command



#define MC_NET_ENUM					301		///< ������ ��E�ִ� Ŀ�´������� Enum
#define MC_NET_CONNECT				302		///< Ŀ�´������Ϳ� ����E
#define MC_NET_DISCONNECT			303		///< Ŀ�´������Ϳ� ����E����
#define MC_NET_CLEAR				304		///< Ŀ�´������Ϳ� ����E���� �� ��E��ڿ�Eó��
#define MC_NET_ONCONNECT			311		///< Ŀ�´������Ϳ� ����Ǿ��� ��
#define MC_NET_ONDISCONNECT			312		///< Ŀ�´������Ϳ� ������ ������ ��
#define MC_NET_ONERROR				313		///< Ŀ�´������Ϳ� ����E����
#define MC_NET_CHECKPING			321		///< Ŀ�´������� �����ӵ��˻�E
#define MC_NET_PING					322		///< Ŀ�´������� �����ӵ��˻�E��û
#define MC_NET_PONG					323		///< Ŀ�´������� �����ӵ��˻�E����E

#define MC_HSHIELD_PING				324		///< �ٽǵ�E�޼���E��û(��ġ����E-> Ŭ���̾�Ʈ)
#define MC_HSHIELD_PONG				325		///< �ٽǵ�E�޼���E����EŬ���̾�Ʈ -> ��ġ����E

#define MC_NET_BANPLAYER_FLOODING	326		///< CommandBuilder���� Flooding ����� ����� Command

#define MC_NET_CONNECTTOZONESERVER	331		///< 127.0.0.1:6000 ����E

#define MC_NET_REQUEST_INFO			341		///< Net ������ ��û�Ѵ�.
#define MC_NET_RESPONSE_INFO		342		///< Net ������ ��´�.
#define MC_NET_REQUEST_UID			343		///< MUID�� ��û
#define MC_NET_RESPONSE_UID			344		///< MUID�� Ȯ���ؼ� �����ش�.
#define MC_NET_ECHO					351		///< Net Echo �׽�Ʈ

#define MC_CLOCK_SYNCHRONIZE		361		///< Ŭ�� ��ũ
#define	MC_SERVER_PING				362     ///Server Ping


// ��ġ����E��E� ���ɾ�E
#define MC_MATCH_NOTIFY						401		///< �˸� �޽���E
#define MC_MATCH_ANNOUNCE					402		///< ����E
#define MC_MATCH_RESPONSE_RESULT			403		///< request�� ��E� ����� �˸�(S -> C)
// ��E��� ��E�E���ɾ�E
#define MC_ADMIN_ANNOUNCE					501		///< ��E� �����ڿ��� ���
#define MC_SERVER_ANNOUNCE					503
#define MC_ADMIN_REQUEST_SERVER_INFO		505		///< ����E���� ��û
#define MC_ADMIN_RESPONSE_SERVER_INFO		506		///< ����E���� ����E
#define MC_ADMIN_SERVER_HALT				511		///< ������ ����E�Ѵ�
#define MC_ADMIN_TERMINAL					512		///< �͹̳�
#define MC_ADMIN_REQUEST_UPDATE_ACCOUNT_UGRADE	513	///< ���� ���� ����E��û
#define MC_ADMIN_RESPONSE_UPDATE_ACCOUNT_UGRADE	514	///< ���� ���� ����E����E
#define MC_ADMIN_REQUEST_KICK_PLAYER			515		///< Ÿ�� ���� ����E��û
#define MC_ADMIN_RESPONSE_KICK_PLAYER			516		///< Ÿ�� ���� ����E����E
#define MC_ADMIN_REQUEST_MUTE_PLAYER			517		///< Ÿ�� ä�� ����E��û
#define MC_ADMIN_RESPONSE_MUTE_PLAYER			518		///< Ÿ�� ä�� ����E����E
#define MC_ADMIN_REQUEST_BLOCK_PLAYER			519		///< Ÿ�� ���� ��Ӱ ��û
#define MC_ADMIN_RESPONSE_BLOCK_PLAYER			520		///< Ÿ�� ���� ��Ӱ ����E
#define MC_ADMIN_PING_TO_ALL				521		///< Garbage Connection û�� Ȯ���� ���� ��������
#define MC_ADMIN_REQUEST_SWITCH_LADDER_GAME	522		///< Ŭ����E��û ���ɿ��� ����
#define MC_ADMIN_HIDE						531		///< ��۵�� �Ⱥ��̱�E
#define MC_ADMIN_RELOAD_CLIENT_HASH			532		///< reload XTrap Hashmap
#define MC_ADMIN_RESET_ALL_HACKING_BLOCK	533		///< ��E��ŷ ��Ӱ�� ÁE��Ѵ�.
#define MC_ADMIN_RELOAD_GAMBLEITEM			534		///< �׺�E������ ���� �ٽ� �б�E
#define MC_ADMIN_DUMP_GAMBLEITEM_LOG		535		///< ���� �׺�E������ ������ Log���Ͽ� ����Ѵ�.
#define MC_ADMIN_ASSASIN					536		///< GM�� Ŀ�Ǵ��� �ȴ�.
#define MC_ADMIN_KICKALL					537		


// �̺�Ʈ ������ ���ɾ�E
#define MC_EVENT_CHANGE_MASTER				601		///< ��������� �����´�
#define MC_EVENT_CHANGE_PASSWORD			602		///< ���� ��й�ȣ�� �ٲ۴�
#define MC_EVENT_REQUEST_JJANG				611		///< ¯��ũ �ο�
#define MC_EVENT_REMOVE_JJANG				612		///< ¯��ũ ȸ��E
#define MC_EVENT_UPDATE_JJANG				613		///< ¯��ũ �˸�


// ��ġ����EŬ���̾�Ʈ
#define MC_MATCH_LOGIN							1001	///< �α���
#define MC_MATCH_RESPONSE_LOGIN					1002	///< �α��� ����E
#define MC_MATCH_LOGIN_NETMARBLE				1003	///< �ݸ���ۡ�� �α���
#define MC_MATCH_LOGIN_NETMARBLE_JP				1004	///< �Ϻ��ݸ���ۡ�� �α���
#define MC_MATCH_LOGIN_FROM_DBAGENT				1005	///< DBAgent�κ����� �α��� ����E�Ϻ��ݸ���E��E�E
#define MC_MATCH_LOGIN_FROM_DBAGENT_FAILED		1008	///< DBAgent�κ����� �α��� ����(�Ϻ��ݸ���E��E�E
#define MC_MATCH_FIND_HACKING					1009	///< ��ŷ ��ÁE
#define MC_MATCH_DISCONNMSG						1010	///< Discconst�޽���E
#define MC_MATCH_LOGIN_NHNUSA					1011	///< �̱� NHN���� �α���.
#define MC_MATCH_LOGIN_GAMEON_JP				1012	///< �Ϻ� gameon���� �α���

#define MC_MATCH_BRIDGEPEER						1006	///< MatchServer�� Peer���� �˸�
#define MC_MATCH_BRIDGEPEER_ACK					1007	///< Peer���� ����Ȯ��

#define MC_MATCH_OBJECT_CACHE					1101	///< ����E�Ʈ ĳ��

#define MC_MATCH_REQUEST_RECOMMANDED_CHANNEL	1201	///< ���� E��� ä�ο���E� ��õ�� ��û�Ѵ�.
#define MC_MATCH_RESPONSE_RECOMMANDED_CHANNEL	1202	///< ���� E��� ä�ο���E� ��õ�Ѵ�.
#define MC_MATCH_CHANNEL_REQUEST_JOIN			1205	///< ä�ο� E�(C -> S)
#define MC_MATCH_CHANNEL_REQUEST_JOIN_FROM_NAME	1206	///< ä���̸����� ä�� ���� ��û
#define MC_MATCH_CHANNEL_RESPONSE_JOIN			1207	///< ä�� ���� ����ES -> C)
#define MC_MATCH_CHANNEL_LEAVE					1208	///< ä�� ŻŁE
#define MC_MATCH_CHANNEL_LIST_START				1211	///< ä�� ��� ��E۽��� ��û
#define MC_MATCH_CHANNEL_LIST_STOP				1212	///< ä�� ��� ��E�����E��û
#define MC_MATCH_CHANNEL_LIST					1213	///< ä�� ���
#define MC_MATCH_CHANNEL_REQUEST_PLAYER_LIST	1221	///< �÷��̾�E����Ʈ ��û
#define MC_MATCH_CHANNEL_RESPONSE_PLAYER_LIST	1222	///< �÷��̾�E����Ʈ ����E
#define MC_MATCH_CHANNEL_REQUEST_CHAT			1225	///< ä�ο� ��E� ��û
#define MC_MATCH_CHANNEL_CHAT					1226	///< ä�ο��� ��E� ����E
#define MC_MATCH_CHANNEL_DUMB_CHAT				1227	///< ä�ο��� ��E� ����, ����Ԯ ��� - ȫ����(2009.08.05)

#define MC_MATCH_CHANNEL_REQUEST_RULE			1230	///< ä�α�Ģ ��û
#define MC_MATCH_CHANNEL_RESPONSE_RULE			1231	///< ä�α�Ģ ����E
#define MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST		1232	///< ä���� �κ� �ִ� ��E�÷��̾�E����Ʈ ��û
#define MC_MATCH_CHANNEL_RESPONSE_ALL_PLAYER_LIST		1233	///< ä���� �κ� �ִ� �÷��̾�E����Ʈ ��û


#define MC_MATCH_STAGE_CREATE					1301	///< ��������E����
#define MC_MATCH_RESPONSE_STAGE_CREATE			1302	///< ��������E���� ����E
#define MC_MATCH_STAGE_JOIN						1303	///< ��������EE�
#define MC_MATCH_REQUEST_STAGE_JOIN				1304	///< ��������EE� ��û
#define MC_MATCH_REQUEST_PRIVATE_STAGE_JOIN		1305	///< ��� ��������EE� ��û
#define MC_MATCH_RESPONSE_STAGE_JOIN			1306	///< ���������� ���ο��θ� �˸�
#define MC_MATCH_STAGE_LEAVE					1307	///< ��������EŻŁE
#define MC_MATCH_STAGE_REQUEST_PLAYERLIST		1308	///< ���������� ������� ��û
#define MC_MATCH_STAGE_FOLLOW					1309	///< ���� ������ ���� ��������EE�
#define MC_MATCH_RESPONSE_STAGE_FOLLOW			1310	///< ���� ������ ���� ��������EE� ����E
#define MC_MATCH_REQUEST_STAGE_LIST				1311	///< Ŭ���̾�Ʈ�� ��������E����Ʈ ��û
#define MC_MATCH_STAGE_LIST_START				1312	///< ��������E��� ��E۽��� ��û
#define MC_MATCH_STAGE_LIST_STOP				1313	///< ��������E��� ��E�����E��û
#define MC_MATCH_STAGE_LIST						1314	///< ��������E���
#define MC_MATCH_STAGE_CHAT						1321	///< ��������E��E�
#define MC_MATCH_STAGE_REQUEST_QUICKJOIN		1322	///< āE��� ��û
#define MC_MATCH_STAGE_RESPONSE_QUICKJOIN		1323	///< āE��� ����E
#define MC_MATCH_STAGE_GO						1331	///< ��������E��ȣ�� E�
#define MC_MATCH_STAGE_REQUIRE_PASSWORD			1332	///< ��й��̶�E�н����尡 �ʿ��ϴ�.(S -> C)


#define MC_MATCH_STAGE_REQUEST_ENTERBATTLE		1401	///< ��������E��E�EE��Ѵٰ�E��û
#define MC_MATCH_STAGE_ENTERBATTLE				1402	///< ��������E��E�E�
#define MC_MATCH_STAGE_LEAVEBATTLE_TO_SERVER	1403	///< ��������E��E�ŻŁEC -> S)
#define MC_MATCH_STAGE_LEAVEBATTLE_TO_CLIENT	1404	///< ��������E��E�ŻŁES -> C)

#define MC_MATCH_REQUEST_STAGESETTING			1411	///< ���������� ������ ��û
#define MC_MATCH_RESPONSE_STAGESETTING			1412	///< ���������� ������ �˸�
#define MC_MATCH_STAGESETTING					1413	///< ��������E����
#define MC_MATCH_STAGE_MAP						1414	///< ��������E�� ����
#define MC_MATCH_STAGE_REQUEST_FORCED_ENTRY		1415	///< ������������ ���ӿ� ���� ��û
#define MC_MATCH_STAGE_RESPONSE_FORCED_ENTRY	1416	///< ��������E���� ���� ����E
#define MC_MATCH_STAGE_RELAY_MAP_ELEMENT_UPDATE	1417	///< ��������E�����̸� ���(Ÿ��, ȸE ������Ʈ
#define MC_MATCH_STAGE_RELAY_MAP_INFO_UPDATE	1418	///< ��������E�����̸� ����Ʈ ������Ʈ

#define MC_MATCH_STAGE_MASTER					1421	///< ��������E����E�˸�
#define MC_MATCH_STAGE_PLAYER_STATE				1422	///< ��������E����E���� �÷��̾�E���� ����
#define MC_MATCH_STAGE_TEAM						1423	///< ��������E��
#define MC_MATCH_STAGE_START					1431	///< ��������E����(ī��⮴ٿ�E
#define MC_MATCH_STAGE_LAUNCH					1432	///< ��������E��ǁE
#define MC_MATCH_STAGE_RELAY_LAUNCH				1433	///< �����̵� ��������E��ǁE

#define MC_MATCH_LOADING_COMPLETE				1441	///< �ε��� ������
#define MC_MATCH_STAGE_FINISH_GAME				1442	///< ��������E���� ����E

#define MC_MATCH_REQUEST_GAME_INFO				1451	///< ���Ӿ� ������ �޶�E��û
#define MC_MATCH_RESPONSE_GAME_INFO				1452	///< ���Ӿ� ������ �˷��ش�
#define MC_MATCH_RESPONSE_GAME_INFO_V2			1453	// Custom: Larger Game Info Packet

#define MC_MATCH_REQUEST_PEERLIST				1461	///< E����� �������� Peer������ ��û
#define MC_MATCH_RESPONSE_PEERLIST				1462	///< E����� �������� Peer������ �˸�

#define MC_MATCH_REQUEST_PEER_RELAY				1471	///< ��ġ������ �ش�Peer�� ��E� �߰��û
#define MC_MATCH_RESPONSE_PEER_RELAY			1472	///< �ش�Peer�� ��E� �߰��㰡 ŁE�

// ���� ��E�
#define MC_MATCH_GAME_ROUNDSTATE				1501	///< ������� ���¸� ����ȭ
#define MC_MATCH_ROUND_FINISHINFO				1502	///< ���сE����� ���� ������Ʈ (C <- S)
#define MC_MATCH_GAME_KILL						1511	///< ĳ���� ��� (C -> S)
#define MC_MATCH_GAME_DEAD						1512	///< ĳ���� �׾��� (S -> C)
#define MC_MATCH_GAME_LEVEL_UP					1513	///< ������ ������E
#define MC_MATCH_GAME_LEVEL_DOWN				1514	///< ������ �����ٿ�E
#define MC_MATCH_GAME_REQUEST_SPAWN				1515	///< ĳ���� ��Ȱ��û (C -> S)
#define MC_MATCH_GAME_RESPONSE_SPAWN			1516	///< ĳ���� ��Ȱ��û (C -> S)
#define MC_MATCH_GAME_TEAMBONUS					1517	///< �� ���ʽ�(S -> C)
#define MC_MATCH_GAME_CHAT						1518	// Custom: in-game chat server sided.

#define MC_MATCH_GAME_REQUEST_TIMESYNC			1521	///< Match������ �ð�����ȭ ��û
#define MC_MATCH_GAME_RESPONSE_TIMESYNC			1522	///< �ð�����ȭ
#define MC_MATCH_GAME_REPORT_TIMESYNC			1523	///< �ð�����E�˻�E

#define MC_MATCH_REQUEST_SUICIDE				1531	///< �ڻ�E��û
#define MC_MATCH_RESPONSE_SUICIDE				1532	///< �ڻ�E����E
#define MC_MATCH_RESPONSE_SUICIDE_RESERVE		1533	///< �ڻ�E����E
#define MC_MATCH_NOTIFY_THROW_TRAPITEM			1535	///< Ʈ�������� ��ô �˸�(C -> S)
#define MC_MATCH_NOTIFY_ACTIVATED_TRAPITEM		1536	///< Ʈ�������� �ߵ� �˸�(C -> S)
#define MC_MATCH_NOTIFY_ACTIATED_TRAPITEM_LIST	1437	///< ������ �������� ����E�ߵ� ���� Ʈ�������� ��� �˷��ֱ�ES -> C)
#define MC_MATCH_REQUEST_OBTAIN_WORLDITEM		1541	///< ������ �Ե��� ��û
#define MC_MATCH_OBTAIN_WORLDITEM				1542	///< ������ ���� ��Eε�ĳ��Ʈ(S -> C)
#define MC_MATCH_SPAWN_WORLDITEM				1543	///< �������� ������
#define MC_MATCH_REQUEST_SPAWN_WORLDITEM		1544	///< �������� �������� ��û(C -> S)
#define MC_MATCH_REMOVE_WORLDITEM				1545	///< ������ ����݁ES -> C)
#define MC_MATCH_ASSIGN_COMMANDER				1551	///< �ϻ�E�E��E����� �Ӹ�E
#define MC_MATCH_RESET_TEAM_MEMBERS				1552	///< ���⹁E�ٽ� ����
#define MC_MATCH_SET_OBSERVER					1553	///< ������ ���������� ��E�


// ���ǰ�E�
#define MC_MATCH_REQUEST_PROPOSAL				1561	///< �ٸ� ����� ���� ��û
#define MC_MATCH_RESPONSE_PROPOSAL				1562	///< ���� ��û ����E
#define MC_MATCH_ASK_AGREEMENT					1563	///< �ٸ� ������� ���� ����(S -> C)
#define MC_MATCH_REPLY_AGREEMENT				1564	///< ���� ����EC -> S)

// ����E��E� (Ŭ����E
#define MC_MATCH_LADDER_REQUEST_CHALLENGE		1571	///< ����E����E��û(C -> S)
#define MC_MATCH_LADDER_RESPONSE_CHALLENGE		1572	///< ����E����E����E
#define MC_MATCH_LADDER_SEARCH_RIVAL			1574	///< ����E�˻���		
#define MC_MATCH_LADDER_REQUEST_CANCEL_CHALLENGE	1575	///< ����E����EÁEҿ�û
#define MC_MATCH_LADDER_CANCEL_CHALLENGE		1576	///< ����E����EÁE�
#define MC_MATCH_LADDER_PREPARE					1578	///< ����E�����غ�E
#define MC_MATCH_LADDER_LAUNCH					1579	///< ����E��ġ (S -> C)

#define MC_MATCH_LADDER_REJOIN					1580	// Custom: Ladder Rejoin
#define MC_MATCH_NOTIFY_LADDER_REJOIN			1581	// Custom: Ladder Rejoin

// �ǻ��ŁE��E�
#define MC_MATCH_USER_WHISPER					1601	///< �ӼӸ�
#define MC_MATCH_USER_WHERE						1602	///< ��ġȮ��
#define MC_MATCH_USER_OPTION					1605	///< �����ɼ�(�Ӹ��ź�,�ʴ�Eź�,ģ���ź� ��сE
#define MC_MATCH_CHATROOM_CREATE				1651	///< ä�÷�E����
#define MC_MATCH_CHATROOM_JOIN					1652	///< ä�÷�EE�
#define MC_MATCH_CHATROOM_LEAVE					1653	///< ä�÷�EŻŁE
#define MC_MATCH_CHATROOM_INVITE				1661	///< ä�÷�E�ʴ�E
#define MC_MATCH_CHATROOM_CHAT					1662	///< ä��
#define MC_MATCH_CHATROOM_SELECT_WRITE			1665	///< ä�÷�E����

// ĳ���� ��E�
#define MC_MATCH_REQUEST_ACCOUNT_CHARLIST		1701	///< ������ ��E� ĳ���� ����Ʈ�� ��û
#define MC_MATCH_RESPONSE_ACCOUNT_CHARLIST		1702	///< ������ ��E� ĳ���� ����Ʈ�� ����E
#define MC_MATCH_REQUEST_SELECT_CHAR			1703	///< ĳ���� ���� ��û
#define MC_MATCH_RESPONSE_SELECT_CHAR			1704	///< ĳ���� ���� ����E
#define MC_MATCH_REQUEST_MYCHARINFO				1705	///< �� ĳ���� ���� ��û
#define MC_MATCH_RESPONSE_MYCHARINFO			1706	///< �� ĳ���� ���� ����E
#define MC_MATCH_REQUEST_CREATE_CHAR			1711	///< �� ĳ���� ���� ��û
#define MC_MATCH_RESPONSE_CREATE_CHAR			1712	///< �� ĳ���� ���� ����E
#define MC_MATCH_REQUEST_DELETE_CHAR			1713	///< �� ĳ���� ���� ��û
#define MC_MATCH_RESPONSE_DELETE_CHAR			1714	///< �� ĳ���� ���� ����E
#define MC_MATCH_REQUEST_COPY_TO_TESTSERVER		1715	///< ĳ���� �׽�Ʈ������ ��E� ��û - �̱�ǁE
#define MC_MATCH_RESPONSE_COPY_TO_TESTSERVER	1716	///< ĳ���� �׽�Ʈ������ ��E� ����E- �̱�ǁE
#define MC_MATCH_REQUEST_CHARINFO_DETAIL		1717	///< �ٸ� �÷��̾�E��������E���� �ڼ��� �÷��̾�E���� ��û
#define MC_MATCH_RESPONSE_CHARINFO_DETAIL		1718	///< �ٸ� �÷��̾�E��������E���� �ڼ��� �÷��̾�E���� ����E
#define MC_MATCH_REQUEST_ACCOUNT_CHARINFO		1719	///< ������ �� ĳ���� ���� ��û
#define MC_MATCH_RESPONSE_ACCOUNT_CHARINFO		1720	///< ������ �� ĳ���� ���� ����E
#define MC_MATCH_REQUEST_CREATE_ACCOUNT			1721	/// Custom: Creating Account from in game
#define MC_MATCH_RESPONSE_CREATE_ACCOUNT		1722	/// Custom: Respond to account creation request
// ������ ��E�
#define MC_MATCH_REQUEST_SIMPLE_CHARINFO		1801	///< ĳ������ ������ ���� ��û	
#define MC_MATCH_RESPONSE_SIMPLE_CHARINFO		1802	///< ĳ������ ������ ���� ����E
#define MC_MATCH_REQUEST_MY_SIMPLE_CHARINFO		1803	///< �� ĳ���� XP, BP��E���� ��û
#define MC_MATCH_RESPONSE_MY_SIMPLE_CHARINFO	1804	///< �� ĳ���� XP, BP��E���� ����E

#define MC_MATCH_REQUEST_BUY_ITEM				1811	///< ������ ���� ��û
#define MC_MATCH_RESPONSE_BUY_ITEM				1812	///< ������ ���� ����E
#define MC_MATCH_REQUEST_SELL_ITEM				1813	///< ������ �Ǹ� ��û
#define MC_MATCH_RESPONSE_SELL_ITEM				1814	///< ������ �Ǹ� ����E
#define MC_MATCH_REQUEST_SHOP_ITEMLIST			1815	///< ������ �Ǹ��ϴ� ������ ����Ʈ ��û
#define MC_MATCH_RESPONSE_SHOP_ITEMLIST			1816	///< ������ �Ǹ��ϴ� ������ ����Ʈ ����E

#define MC_MATCH_REQUEST_CHARACTER_ITEMLIST		1821	///< �� ������ ����Ʈ �ֽÿ�
#define MC_MATCH_RESPONSE_CHARACTER_ITEMLIST	1822	///< ������ ����Ʈ ������
#define MC_MATCH_REQUEST_EQUIP_ITEM				1823	///< ������ �庁E��û
#define MC_MATCH_RESPONSE_EQUIP_ITEM			1824	///< ������ �庁E����E
#define MC_MATCH_REQUEST_TAKEOFF_ITEM			1825	///< ������ ���� ��û
#define MC_MATCH_RESPONSE_TAKEOFF_ITEM			1826	///< ������ ���� ����E
#define MC_RESPONSE_GAMBLEITEMLIST				1827	///< DB�� �׺�E������ ���� ��E�.
#define MC_MATCH_ROUTE_UPDATE_STAGE_EQUIP_LOOK	1828	///< ĳ������ Look����E������Ʈ�Ҷ� �翁EѴ�.
#define MC_MATCH_REQUEST_CHARACTER_ITEMLIST_FORCE	1829	///< �� ������ ����Ʈ �ֽÿ� : �ߺ� ��û�� ��������E�ʴ� ����E�����ؼ� �翁EҰ�)

#define MC_MATCH_REQUEST_ACCOUNT_ITEMLIST			1831	///< �� â��E������ ����Ʈ �ֽÿ�
#define MC_MATCH_RESPONSE_ACCOUNT_ITEMLIST			1832	///< â��E������ ����Ʈ ������
#define MC_MATCH_REQUEST_BRING_ACCOUNTITEM			1833	///< â��E�������� �� ĳ���ͷ� ��������E
#define MC_MATCH_RESPONSE_BRING_ACCOUNTITEM			1834	///< â��E������ ��������E����E
#define MC_MATCH_REQUEST_BRING_BACK_ACCOUNTITEM		1835	///< �� ĳ�������� â���� �űⱁE��û
#define MC_MATCH_RESPONSE_BRING_BACK_ACCOUNTITEM	1836	///< �� ĳ�������� â���� �űⱁE����E
#define MC_MATCH_EXPIRED_RENT_ITEM					1837	///< �������� �Ⱓ ����Ǿ����ϴ�.(S -> C)

#define MC_MATCH_REQUEST_GAMBLE					1841	///< �̱�E(C -> S)
#define MC_MATCH_RESPONSE_GAMBLE				1842	///< �̱�E�ᰁE(S -> C)

#define MC_MATCH_REWARD_BATTLE_TIME				1843	///< ��Ʋ �ð� ����E����ES->C)

// ������ ��E�(�Ҹ�)
#define MC_MATCH_REQUEST_USE_SPENDABLE_NORMAL_ITEM		1850	///< �Ϲ� �Ҹ� ������ �翁E��û(C -> S)
#define MC_MATCH_REQUEST_USE_SPENDABLE_BUFF_ITEM		1860	///< ���� �Ҹ� ������ �翁E��û(C -> S)
#define MC_MATCH_RESPONSE_USE_SPENDABLE_BUFF_ITEM		1861	///< ���� �Ҹ� ������ �翁E�ᰁES -> C)
#define MC_MATCH_SPENDABLE_BUFF_ITEM_STATUS				1865	///< ���� �Ҹ� ������ ����(S -> C)
#define MC_MATCH_REQUEST_USE_SPENDABLE_COMMUNITY_ITEM	1870	///< Ŀ�´�Ƽ �Ҹ� ������ �翁E��û(C -> S)

// ģ�� ��E�
#define MC_MATCH_FRIEND_ADD						1901	///< ģ�� �߰�
#define MC_MATCH_FRIEND_REMOVE					1902	///< ģ�� ����
#define MC_MATCH_FRIEND_LIST					1903	///< ģ�� ��� ��û
#define MC_MATCH_RESPONSE_FRIENDLIST			1904	///< ģ�� ��� ��E�
#define MC_MATCH_FRIEND_MSG						1905	///< ģ�� ä��


// Ŭ����E�
#define MC_MATCH_CLAN_REQUEST_CREATE_CLAN			2000	///< Ŭ�� ���� ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CREATE_CLAN			2001	///< Ŭ�� ���� ����ES -> C)
#define MC_MATCH_CLAN_ASK_SPONSOR_AGREEMENT			2002	///< �߱��ο��� Ŭ�� ���� ���� ����(S -> C)
#define MC_MATCH_CLAN_ANSWER_SPONSOR_AGREEMENT		2003	///< �߱����� ���Ǹ� ����EC -> S)
#define MC_MATCH_CLAN_REQUEST_AGREED_CREATE_CLAN	2004	///< �߱����� ������ Ŭ�� ���� ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_AGREED_CREATE_CLAN	2005	///< �߱����� ������ Ŭ�� ���� ����ES -> C)
#define MC_MATCH_CLAN_REQUEST_CLOSE_CLAN			2006	///< �����Ͱ� Ŭ�� ��E��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CLOSE_CLAN			2007	///< �������� Ŭ�� ��E����ES -> C)
#define MC_MATCH_CLAN_REQUEST_JOIN_CLAN				2008	///< ��۵�ڰ� �������� ������ ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_JOIN_CLAN			2009	///< ��۵���� ������ ������ ����ES -> C)
#define MC_MATCH_CLAN_ASK_JOIN_AGREEMENT			2010	///< �����ڿ��� ���� ���� ����(S -> C)
#define MC_MATCH_CLAN_ANSWER_JOIN_AGREEMENT			2011	///< �������� ���� ���� ����EC -> S)
#define MC_MATCH_CLAN_REQUEST_AGREED_JOIN_CLAN		2012	///< �����ڰ� ������ Ŭ�� ���� ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_AGREED_JOIN_CLAN		2013	///< �����ڰ� ������ Ŭ�� ���� ����ES -> C)
#define MC_MATCH_CLAN_REQUEST_LEAVE_CLAN			2014	///< Ŭ������ Ż�� ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_LEAVE_CLAN			2015	///< Ŭ������ Ż���� ����ES -> C)
#define MC_MATCH_CLAN_UPDATE_CHAR_CLANINFO			2016	///< ĳ������ Ŭ�������� ������Ʈ��(S -> C)
#define MC_MATCH_CLAN_MASTER_REQUEST_CHANGE_GRADE	2017	///< ����� ���� ����E��û(C -> S)
#define MC_MATCH_CLAN_MASTER_RESPONSE_CHANGE_GRADE	2018	///< ����� ���� ����E����ES -> C)
#define MC_MATCH_CLAN_ADMIN_REQUEST_EXPEL_MEMBER	2019	///< ����� ŻŁE��û(C -> S)
#define MC_MATCH_CLAN_ADMIN_RESPONSE_EXPEL_MEMBER	2020	///< ����� ŻŁE����ES -> C)
#define MC_MATCH_CLAN_REQUEST_MSG					2021	///< Ŭ�� ä�� ��û(C -> S)
#define MC_MATCH_CLAN_MSG							2022	///< Ŭ�� ä��(S -> C)
#define MC_MATCH_CLAN_REQUEST_MEMBER_LIST			2023	///< Ŭ�� �⹁E����Ʈ ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_MEMBER_LIST			2024	///< Ŭ�� �⹁E����Ʈ ����ES -> C)
#define MC_MATCH_CLAN_REQUEST_CLAN_INFO				2025	///< Ŭ�� ���� ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_CLAN_INFO			2026	///< Ŭ�� ���� ����ES -> C)
#define MC_MATCH_CLAN_STANDBY_CLAN_LIST				2027	///< Ŭ����E��E����� Ŭ�� ����Ʈ ����ES -> C)
#define MC_MATCH_CLAN_MEMBER_CONNECTED				2028	///< Ŭ������ ������(S -> C)
#define MC_MATCH_FRIEND_CONNECTED					2029	///< Friend >
#define MC_MATCH_CLAN_REQUEST_EMBLEMURL				2051	///< Ŭ����ũ URL ��û(C -> S)
#define MC_MATCH_CLAN_RESPONSE_EMBLEMURL			2052	///< Ŭ����ũ URL ����ES -> C)
#define MC_MATCH_CLAN_LOCAL_EMBLEMREADY				2055	///< Ŭ����ũ �ٿ��ε�E�Ϸ�EC -> C)
#define MC_MATCH_CLAN_ACCOUNCE_DELETE				2056	///< Ŭ�� ��E��û ����E���� ����E(S -> C)


// ��ǥ��E�
#define MC_MATCH_CALLVOTE							2100	///< ��ǥ�� �����Ѵ�. (C -> S)
#define MC_MATCH_NOTIFY_CALLVOTE					2101	///< ��ǥ������ �˸���. (S -> C)
#define MC_MATCH_NOTIFY_VOTERESULT					2102	///< ��ǥ����� �˸���. (S -> C)
#define MC_MATCH_VOTE_YES							2105	///< ��ǥ (C -> S)
#define MC_MATCH_VOTE_NO							2106	///< ��ǥ (C -> S)
#define MC_MATCH_VOTE_RESPONSE						2107	///< ��ǥ ����.(S -> C)
#define MC_MATCH_VOTE_STOP							2108	///< ��ǥ ÁE�. (S -> C)

// ��۰�E�
#define MC_MATCH_BROADCAST_CLAN_RENEW_VICTORIES			2200	///< Ŭ���� ��� �������Դϴ�.(S -> C)
#define MC_MATCH_BROADCAST_CLAN_INTERRUPT_VICTORIES		2201	///< Ŭ���� ������ �����Ͽ����ϴ�.(S -> C)
#define MC_MATCH_BROADCAST_DUEL_RENEW_VICTORIES			2202	///< xx���� xxä���� xx�濡�� x�������Դϴ�.(S -> C)
#define MC_MATCH_BROADCAST_DUEL_INTERRUPT_VICTORIES		2203	///< xx���� xx���� xx������ �����߽��ϴ�. (S -> C)


// Duel Tournament ��E�.. Added by ȫ����
#define MC_MATCH_DUELTOURNAMENT_REQUEST_JOINGAME				2300	///< �ཱྀE��ʸ�Ʈ�� E� ��û(C -> S)
#define MC_MATCH_DUELTOURNAMENT_RESPONSE_JOINGAME				2301	///< �ཱྀE��ʸ�Ʈ�� E� ��û�� ��E� ����ES -> C)
#define MC_MATCH_DUELTOURNAMENT_REQUEST_CANCELGAME				2302	///< �ཱྀE��ʸ�Ʈ�� E� ��û ÁE�(C -> S)
#define MC_MATCH_DUELTOURNAMENT_CHAR_INFO						2303	///< �ཱྀE��ʸ�Ʈ�� �ɸ��� ����(S -> C)
#define MC_MATCH_DUELTOURNAMENT_PREPARE_MATCH					2304	///< �ཱྀE��ʸ�Ʈ ���� ��E
#define MC_MATCH_DUELTOURNAMENT_LAUNCH_MATCH					2305	///< �ཱྀE��ʸ�Ʈ ����! (S - > C)
#define MC_MATCH_DUELTOURNAMENT_CANCEL_MATCH					2306	///< �ཱྀE��ʸ�Ʈ ÁE�! (S - > C)
#define MC_MATCH_DUELTOURNAMENT_CHAR_INFO_PREVIOUS				2307	///< �ཱྀE��ʸ�Ʈ ĳ���� ������ ����(S -> C)
#define MC_MATCH_DUELTOURNAMENT_NOT_SERVICE_TIME				2308	///< ����E�����ʸ�Ʈ ���� �ð��� �ƴմϴ�.(���½ð� %s%d��00�� ~ %s%d��59��) (S - > C)

#define MC_MATCH_DUELTOURNAMENT_REQUEST_SIDERANKING_INFO		2400	///< �ɸ��� ��ŷ�� ��ó ��ŷ ���� ��û(C -> S)
#define MC_MATCH_DUELTOURNAMENT_RESPONSE_SIDERANKING_INFO		2401	///< �ɸ��� ��ŷ�� ��ó ��ŷ ���� ����ES -> C)
#define MC_MATCH_DUELTOURNAMENT_GROUPRANKING_INFO				2402	///< �ཱྀE��ʸ�Ʈ�� �׷�E��ŷ ����(S -> C)

#define MC_MATCH_DUELTOURNAMENT_GAME_INFO						2500	///< �ཱྀE��ʸ�Ʈ ���� �������� ����
#define MC_MATCH_DUELTOURNAMENT_GAME_ROUND_RESULT_INFO			2501	///< �ཱྀE��ʸ�Ʈ ���� �������� ���сE���� �ᰁE����
#define MC_MATCH_DUELTOURNAMENT_GAME_MATCH_RESULT_INFO			2502	///< �ཱྀE��ʸ�Ʈ ���� �������� ��ġ ���� �ᰁE����
#define MC_MATCH_DUELTOURNAMENT_GAME_PLAYER_STATUS				2503	///< �ཱྀE��ʸ�Ʈ ���� �������� ����尡 ����� ��, �ش�E���̸��� ���¸� ������ ����(C -> S)
#define MC_MATCH_DUELTOURNAMENT_GAME_NEXT_MATCH_PLYAERINFO		2504	///< �ཱྀE��ʸ�Ʈ ���� �������� ���� ��ġ���� �÷��̾�E����

// ���� ��E��E�
// ����Ŀ ��E
#define MC_MATCH_ASSIGN_BERSERKER					3001	///< ����Ŀ �Ӹ�ES -> C)


#define MC_MATCH_DUEL_QUEUEINFO						3100	///< ��E⿭ ����E�˸�(S -> C)
#define MC_MATCH_DUEL_QUEUEINFO_V2					3101	// Custom: Duel Queue Info V2


// ��ġ ������Ʈ ��E� ���ɾ�E
#define MC_MATCH_REGISTERAGENT				5001	///< ������Ʈ ����
#define MC_MATCH_UNREGISTERAGENT			5002	///< ������Ʈ ����E
#define MC_MATCH_AGENT_REQUEST_LIVECHECK	5011	///< ������Ʈ Ȯ�ο�û
#define MC_MATCH_AGENT_RESPONSE_LIVECHECK	5012	///< ������Ʈ Ȯ������E
#define MC_AGENT_ERROR						5013	///< ������E� ��E� ����

#define MC_AGENT_CONNECT					5021	///< ��ġ������ ������Ʈ ����E����
#define MC_AGENT_DISCONNECT					5022	///< ��ġ������ ������Ʈ ����E��������E
#define MC_AGENT_LOCAL_LOGIN				5023	///< Ŭ���̾�Ʈ ����ó��
#define MC_AGENT_RESPONSE_LOGIN				5024	///< Ŭ���̾�Ʈ�� �α��� ŁE�E
//#define MC_AGENT_MATCH_VALIDATE				5031	///< ��ġ�������� ����Ȯ��
#define MC_AGENT_STAGE_RESERVE				5051	///< ������E��� ��������E����
#define MC_AGENT_STAGE_RELEASE				5052	///< ������E��� ��������E����E
#define MC_AGENT_STAGE_READY				5053	///< ������E��� ��������E�غ�Ϸ�E
#define MC_AGENT_LOCATETO_CLIENT			5061	///< Ŭ���̾�Ʈ���� Agent �˸�
#define MC_AGENT_RELAY_PEER					5062	///< ������E��� �߰�����
#define MC_AGENT_PEER_READY					5063	///< ��ġ������ �Ǿ��غ�E�˸�
#define MC_AGENT_PEER_BINDTCP				5071	///< Ŭ���̾�Ʈ Bind TCP
#define MC_AGENT_PEER_BINDUDP				5072	///< Ŭ���̾�Ʈ Bind UDP
#define MC_AGENT_PEER_UNBIND				5073	///< Ŭ���̾�Ʈ Unbind
#define MC_GUNZ_SEND_COUNTRYCODE			5074    ///< Banderas
#define MC_PEER_KILLSTREAK					5075    ///< KillerStreak
#define MC_AGENT_TUNNELING_TCP				5081	///< TCP �ͳθ�
#define MC_AGENT_TUNNELING_UDP				5082	///< UDP �ͳθ�
#define MC_AGENT_ALLOW_TUNNELING_TCP		5083	///< TCP �ͳθ� �㿁E
#define MC_AGENT_ALLOW_TUNNELING_UDP		5084	///< UDP �ͳθ� �㿁E
#define MC_AGENT_DEBUGPING					5101	///< �����E�E
#define MC_AGENT_DEBUGTEST					5102	///< �����E�E

#define MC_TEST_BIRDTEST1					60001	///< ����E�׽�Ʈ1
#define MC_TEST_PEERTEST_PING				60002	///< Target Peer Test
#define MC_TEST_PEERTEST_PONG				60003	///< Target Peer Test



/// Ŀ�ǵ�E���� �ɼ�

// MC_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST�� ���οɼ�
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NORMAL		0		
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_NONCLAN		1
#define MCP_MATCH_CHANNEL_REQUEST_ALL_PLAYER_LIST_MYCLAN		2


// ����Ʈ ��E� ���ɾ�E�����̹�E��E ����Ʈ ��E��ŁE
#define MC_QUEST_NPC_SPAWN					6000	///< NPC ��ƁES -> C)
#define MC_QUEST_ENTRUST_NPC_CONTROL		6001	///< NPC�� ������ �Ǿ�ۡ�� ����(S -> C)
#define MC_QUEST_CHECKSUM_NPCINFO			6002	///< NPC�������� üũ���� �������� ������(C -> S)
#define MC_QUEST_REQUEST_NPC_DEAD			6003	///< NPC ���� ��û(C -> S)
#define MC_QUEST_NPC_DEAD					6004	///< NPC ����(S -> C)
#define MC_QUEST_REFRESH_PLAYER_STATUS		6005	///< �׾�����E�ٽ� �¾�Ϫ��E ��E���¸� �ֻ����� ����(S -> C)
#define MC_QUEST_NPC_ALL_CLEAR				6006	///< NPC ��� ����(S -> C)
#define MC_MATCH_QUEST_REQUEST_DEAD			6007	///< �÷��̾�E�׾��ٰ�E��û(C -> S) ����Ʈ��忡���� MC_MATCH_GAME_KILL��E� ������.
#define MC_MATCH_QUEST_PLAYER_DEAD			6008	///< �÷��̾�E����(S -> C)
#define MC_QUEST_OBTAIN_QUESTITEM			6009	///< ����Ʈ ������ ȹ��ES -> C)
#define MC_QUEST_STAGE_MAPSET				6010	///< ��������E�ʼ� ����ES <-> C)
#define MC_QUEST_OBTAIN_ZITEM				6011	///< �Ϲ� ������ ȹ��ES -> C)
#define MC_QUEST_PING						6012	///< ����E>Ŭ��E�� (S -> C)
#define MC_QUEST_PONG						6013	///< Ŭ��E>����E�� (C -> S)
#define MC_QUEST_NPCLIST					6014	///< ���� ����Ʈ���� �翁E� NPC�� ����.(S->C) 
///<  Ŭ��� ������ ������ ������ENPC�� �ʱ�ȭ �Ѵ�.


#define MC_QUEST_PEER_NPC_BASICINFO			6040	///< ���������� ������Ʈ�Ǵ� NPC �⺻����
#define MC_QUEST_PEER_NPC_HPINFO			6041	///< ���������� ������Ʈ�Ǵ� NPC ü������
#define MC_QUEST_PEER_NPC_ATTACK_MELEE		6042	///< ���� ����
#define MC_QUEST_PEER_NPC_ATTACK_RANGE		6043	///< ���Ÿ� ����
#define MC_QUEST_PEER_NPC_SKILL_START		6044	///< ��ų ĳ���� ����
#define MC_QUEST_PEER_NPC_SKILL_EXECUTE		6045	///< ��ų ĳ���ü���Eȿ��E����
#define MC_QUEST_PEER_NPC_DEAD				6046	///< ���� �� ������ �Ǿ��鿡�� ������. 
#define MC_QUEST_PEER_NPC_BOSS_HPAP			6047	///< ���������� ������Ʈ�Ǵ� NPC ���� ü�� �Ƹ�

// ����Ʈ ���� ��ǁE��E�(����Ʈ ��E
#define MC_QUEST_GAME_INFO					6051	///< ���� ����. �ε��� ���ҽ� ��ܻ �˸�(S -> C)
#define MC_QUEST_COMBAT_STATE				6052	///< ����Ʈ ���ӳ� ���� ����(S -> C)
#define MC_QUEST_SECTOR_START				6053	///< �ʼ��� ���� ����(S -> C)
#define MC_QUEST_COMPLETED					6054	///< �ش�E����Ʈ Ŭ����ES -> C)
#define MC_QUEST_FAILED						6055	///< ����Ʈ ����(S -> C)

#define MC_QUEST_REQUEST_MOVETO_PORTAL		6057	///< ��Ż�� �̵���(C -> S)
#define MC_QUEST_MOVETO_PORTAL				6058	///< ��Ż�� �̵�(S -> C)
#define MC_QUEST_READYTO_NEWSECTOR			6059	///< ���ο�E���� �غ�E�Ϸ�ES <-> C)

#define MC_GAME_START_FAIL					6060	///< ����Ʈ ������ ����.(S -> C)
#define MC_QUEST_STAGE_GAME_INFO			6061	///< ��E��� ��������E���� �˸�(QL, �ó�����ID). (S -> C)
#define MC_QUEST_SECTOR_BONUS				6062	///< ���� ���ʽ�(S -> C)

// ����Ʈ ���� ��ǁE��E�(�����̹�E��E
#define MC_QUEST_ROUND_START				6100	///< ���ο�E���сE����(S -> C)
#define MC_QUEST_REQUEST_QL					6101	///< ����E����Ʈ ���������� QL������ ��û.(C -> S)
#define MC_QUEST_RESPONSE_QL				6102	///< ����E����Ʈ ���������� QL������ �˸�.(S -> C)
#define MC_QUEST_SURVIVAL_RESULT			6103	///< �����̹�E�ᰁEŁE�(S -> C)
#define MC_SURVIVAL_RANKINGLIST				6104	///< �����̹�E���� ��ŷ ��� ��E�(S -> C)
#define MC_SURVIVAL_PRIVATERANKING			6105	///< ������ �����̹�E��ŷ ���� ��E�(S -> C)


// ����Ʈ �׽�Ʈ��E
#define MC_QUEST_TEST_REQUEST_NPC_SPAWN		6901	///< NPC ��ƁE��Ź(C -> S)
#define MC_QUEST_TEST_REQUEST_CLEAR_NPC		6902	///< NPC Clear ��Ź(C -> S)
#define MC_QUEST_TEST_REQUEST_SECTOR_CLEAR	6903	///< ���� Clear(C -> S)	
#define MC_QUEST_TEST_REQUEST_FINISH		6904	///< ����Ʈ Complete(C -> S)

/// �ű�����Ʈ��E(ç����E����Ʈ)
#define MC_NEWQUEST_NPC_SPAWN				6301	///< NPC ��ƁE(S -> C)
#define MC_NEWQUEST_REQUEST_NPC_SPAWN		6304	///< NPC ��ƁE(C -> S)
#define MC_NEWQUEST_REQUEST_NPC_DEAD		6302	///< NPC ���� ��û(C -> S)
#define MC_NEWQUEST_NPC_DEAD				6303	///< NPC ����(S -> C)
#define MC_NEWQUEST_MOVE_TO_NEXT_SECTOR		6341	///< ���� ���ͷ� �̵� (S -> C)
#define MC_NEWQUEST_PEER_NPC_BASICINFO		6351	///< ���������� ������Ʈ�Ǵ� NPC �⺻����
#define MC_NEWQUEST_PEER_NPC_ACTION_EXECUTE	6404	///< NPC �׼� ����
#define MC_NEWQUEST_PEER_NPC_ATTACK_MELEE	6405	///< NPC ��������	//todok del �̰� �׼� ����ŁE��� ������Epeer���� ���� ó���ϰ� �ƴ�
#define MC_NEWQUEST_NEW_CONTROL				6406
#define MC_NEWQUEST_OBTAIN_ZITEM			6407	///< �Ϲ� ������ ȹ��ES -> C)


// MatchServer Schedule��E�.
#define MC_MATCH_SCHEDULE_ANNOUNCE_MAKE				7001	///< ������E����E����.
#define MC_MATCH_SCHEDULE_ANNOUNCE_SEND				7002	///< ������E����E��E�.
#define MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_DOWN	7003	///< Ŭ������EŬ����E��Ȱ��ȭ.
#define MC_MATCH_SCHEDULE_CLAN_SERVER_SWITCH_ON		7004	///< Ŭ������EŬ����EȰ��ȭ.
#define MC_MATCH_SCHEDULE_STOP_SERVER				7005	///< ����E����E


// x-trap
#define MC_REQUEST_XTRAP_HASHVALUE					8001	///< ����E��� ȣȯ���� ���縸(�翁E���E�ʴ´�.)
#define MC_RESPONSE_XTRAP_HASHVALUE					8002	///< ����E��� ȣȯ���� ���縸(�翁E���E�ʴ´�.)
#define MC_REQUEST_XTRAP_SEEDKEY					8003	///< ��ȣŰ ��û(S -> C)
#define MC_RESPONSE_XTRAP_SEEDKEY					8004	///< ��ȣŰ ����EC -> S)
#define MC_REQUEST_XTRAP_DETECTCRACK				8005	///< ����E��� ȣȯ���� ���縸(�翁E���E�ʴ´�.)

// gameguard
#define MC_REQUEST_GAMEGUARD_AUTH					9001
#define MC_RESPONSE_GAMEGUARD_AUTH					9002
#define MC_REQUEST_FIRST_GAMEGUARD_AUTH				9003
#define MC_RESPONSE_FIRST_GAMEGUARD_AUTH			9004

// �ڼ�E
#define MC_REQUEST_GIVE_ONESELF_UP					9101	///< �ڼ��ϴ�.


//// �Ǿ����Ǿ�E��E� ���ɾ�E/////////////////////////////////////////////////////////////
#define MC_PEER_PING				10001	///< peer��E������ network latency(ping)�� �����ϱ����� command
#define MC_PEER_PONG				10002	///< ping�� ��E� ����E
#define MC_PEER_UDPTEST				10005	///< Peer to Peer UDP ��E��� ��������E�˻��Ѵ�.
#define MC_PEER_UDPTEST_REPLY		10006	///< Peer to Peer UDP ��E��� ��������E�˻��Ѵ�.

#define MC_UDP_PING					10007	///< Client�� Agent��E������ ping�� �����ϱ����� command
#define MC_UDP_PONG					10008	///< ping�� ��E� ����E

#define MC_PEER_OPENED				10011	///< ���� Peer ŁE� ��ŁE˸�
#define MC_PEER_BASICINFO			10012	///< ���������� ������Ʈ�Ǵ� ĳ���� �⺻���� ��ǥ������ �ٰŰ� �ȴ�
#define MC_PEER_HPINFO				10013	///< ���������� ������Ʈ�Ǵ� ĳ���� ü������ ��ǥ ������ �ٰŰ� �ȴ�
#define MC_PEER_HPAPINFO			10014	///< ���������� ������Ʈ�Ǵ� ĳ���� ü��/�Ƹ� ����
#define MC_PEER_DUELTOURNAMENT_HPAPINFO		10016	///< ���������� ������Ʈ�Ǵ� �����ʸ�Ʈ ��E�Eĳ���� ü��/�Ƹ� ����

#define MC_PEER_CHANGECHARACTER		10015
#define MC_PEER_MOVE				10021   ///< �� ���߿� MC_OBJECT_MOVE�� �ٲ������ҵ�E.
#define MC_PEER_CHANGE_WEAPON		10022   
#define MC_PEER_CHANGE_PARTS		10023
#define MC_PEER_ATTACK				10031
#define MC_PEER_DAMAGE				10032
#define MC_PEER_RELOAD				10033
#define MC_PEER_SHOT				10034
#define MC_PEER_SHOT_SP				10035
#define MC_PEER_SKILL				10036
#define MC_PEER_SHOT_MELEE			10037	///< ��������
#define MC_PEER_DIE					10041
#define MC_PEER_SPAWN				10042
#define MC_PEER_DASH				10045
#define MC_PEER_SPMOTION			10046
#define MC_PEER_CHAT				10052
#define MC_PEER_CHAT_ICON			10053	///< ä�� ����/��. �Ӹ��� �������� �����ְų� �����.
#define MC_PEER_REACTION			10054	///< ����Ʈ�� ���� �˷��ش� 
#define MC_PEER_ENCHANT_DAMAGE		10055	///< ��æƮ �������� �¾Ҵ�
#define MC_PEER_BUFF_INFO			10056	///< ���� ����Ʈ�� ��E�(�������� �濁E �ߵ��� ��E� ŁE��� �����Ƿ� ���� ��������E�ʾƵ� �ȴ�)
											///< �׷��� �����ڿ��Դ� �ڽ��� �ߵ� ���� ����Ʈ�� �˷��־��� �ϹǷ� �׶� �翁EѴ�)

#define MC_PEER_VAMPIRE				10066
#define MC_PEER_SNIFER				10067

#define MC_ADMIN_FREEZE				10068
#define MC_ADMIN_SPAWN_RESPONSE		10069
#define MC_ADMIN_SUMMON				10070
#define MC_ADMIN_SLAP				10071
#define MC_ADMIN_SPAWN				10072	
#define MC_ADMIN_PKICK				10073
#define MC_ADMIN_NAT				10074
#define MC_VIP_ANNOUNCE				10077

#define MC_MATCH_FLAG_EFFECT		10080
#define MC_MATCH_FLAG_CAP			10081
#define MC_MATCH_REQUEST_FLAG_CAP	10082
#define MC_MATCH_FLAG_STATE			10083

#define MC_ADMIN_GOTO				10084
#define MC_ADMIN_SILENCE			10085

#define MC_MATCH_REQUEST_INFECT		10086
#define MC_MATCH_INFECT				10087
#define MC_MATCH_LASTSURVIVOR		10088

#define MC_MATCH_GUNGAME_WEAPONDATA 10089

#define MC_GUNZ_ANTILEAD_OLD		10093
#define MC_ADMIN_TEAM				10094
#define MC_MATCH_REQUEST_ROLL		10095
#define MC_MATCH_RESPONSE_ROLL		10096
#define MC_REQUEST_RESOURCE_CRC32				11001
#define MC_RESPONSE_RESOURCE_CRC32				12002


// ����Ʈ ������ ��E� �κ�.
#define MC_MATCH_REQUEST_CHAR_QUEST_ITEM_LIST		21000	///< ���� ����Ʈ ������ ����� ��û.(C -> S)
#define MC_MATCH_RESPONSE_CHAR_QUEST_ITEM_LIST		21001	///< ���� ����Ʈ ������ ����� ��E�.(S -> C)
#define MC_MATCH_REQUEST_BUY_QUEST_ITEM				21002	///< ����Ʈ ������ ���� ��û.(C -> S)
#define MC_MATCH_RESPONSE_BUY_QUEST_ITEM			21003	///< ����Ʈ ������ ���� �Ϸ�E(S -> C)
#define MC_MATCH_REQUEST_SELL_QUEST_ITEM			21004	///< ���� ����Ʈ ������ �Ǹ� ��û.(C -> S)
#define MC_MATCH_RESPONSE_SELL_QUEST_ITEM			21005	///< ���� ����Ʈ ������ �Ǹ� �Ϸ�E(S -> C)
#define MC_MATCH_USER_REWARD_QUEST					21006	///< ����Ʈ�� �Ϸ���� ����� ����Ʈ ������ ����.
#define MC_MATCH_REQUEST_DROP_SACRIFICE_ITEM		21007	///< ��� �������� ���Կ� ����E��û.(C -> S)
#define MC_MATCH_RESPONSE_DROP_SACRIFICE_ITEM		21008	///< ��� �������� ���Կ� ����E����E(S -> C)
#define MC_MATCH_REQUEST_CALLBACK_SACRIFICE_ITEM	21009	///< ���Կ� �÷����� ��� �������� ȸ��E��û.(C -> S)
#define MC_MATCH_RESPONSE_CALLBACK_SACRIFICE_ITEM	21010	///< ���Կ� �÷����� ��� �������� �ּ�E����E(S -> C)
#define MC_MATCH_REQUEST_SLOT_INFO					21011	///< ��� ������ ���� ������ ��û.(C -> S)
#define MC_MATCH_RESPONSE_SLOT_INFO					21012	///< ��� ������ ���� ������ �˸�.(S -> C)
#define MC_MATCH_NEW_MONSTER_INFO					21013	///< ���� ������ �߰��� ���ο�E���� ����.
#define MC_MATCH_REQUEST_MONSTER_BIBLE_INFO			21014	///< ���� ������ ����E��û.(C -> S)
#define MC_MATCH_RESPONSE_MONSTER_BIBLE_INFO		21015	///< ���� ������ ����E�˸�.(S -> C)


/// Keeper�� Keeper Manager�� ŁE��ϴµ� �ʿ��� ����. 2005.06.03 Ŀ�ǵ�E���� �ʿ�E
#define MC_KEEPER_MANAGER_CONNECT				31001	///< keeper Manager���� keeper�� ���� ��û.
#define MC_CHECK_KEEPER_MANAGER_PING			31002	///< Keeper�� Keeper Manager�� ���������� ����Ǿ�E�ִ���E�˻�E
#define MC_REQUEST_MATCHSERVER_STATUS			31004	///< MatchServer�� ������ ��û.(Keeper -> MatchServer)
#define MC_RESPONSE_MATCHSERVER_STATUS			31005	///< Keeper�� MatchServer�� ������ ��E��� ��.(MatchServer -> Keeper)
// 2005.06.01 ~
#define MC_RESPONSE_KEEPER_MANAGER_CONNECT		31006	///< Keeper manager�� ������ ���������� �̷羁E��E� �˸�.( Keeper -> Keeper Manager )
#define MC_REQUEST_CONNECTION_STATE				31007	///< ���� ������ ����.(Keeper Manager -> Keeper)
#define MC_RESPONSE_CONNECTION_STATE			31008	///< �������� ����E(Keeper -> Keeper Manager)
#define MC_REQUEST_SERVER_HEARBEAT				31009	///< ������ �����ϰ�E�ִ���E����E��û.(Keeper -> MatchServer)
#define MC_RESPONSE_SERVER_HEARHEAT				31010	///< ������ �����ϰ�E������ ����E(MatchServer -> Keeper)
#define MC_REQUEST_KEEPER_CONNECT_MATCHSERVER	31011	///< Keeper�� MatchServer�� �����ϵ��� ��û��(Keeper Manager -> Keeper -> MatchServer )
#define MC_RESPONSE_KEEPER_CONNECT_MATCHSERVER	31012	///< Keeper�� MatchServer�� �����ߴٴ� ����E(MatchServer -> Keeper -> Keeper Manager )
#define MC_REQUEST_REFRESH_SERVER				31013	///< Keeper Manager�� ����E����Ʈ ���� ������ ���ؼ� Keeper�� MatchServer�� Heartbaetäũ ��û.(Keeper Manager -> Keeepr)
#define MC_REQUEST_LAST_JOB_STATE				31014	///< ������ ����� ���� ����� ��û.(Keeper -> Keeper Manager )
#define MC_RESPONSE_LAST_JOB_STATE				31015	///< �۾�E�ᰁE��û ����E(Keeper -> Keeper Manager)
#define MC_REQUEST_CONFIG_STATE					31016	///< Keeper�� ����� ���� ���� ��û.(Keeper Manager -> Keeper)
#define MC_RESPONSE_CONFIG_STATE				31017	///< Keeper�� ����� ���� ���� ��û ����E(Keeper -> Keeper Manager)
#define MC_REQUEST_SET_ONE_CONFIG				31018	///< Config�ϳ� ���� ��û.(Keeper Manager -> Keeper)
#define MC_RESPONSE_SET_ONE_CONFIG				31019	///< Config�ϳ� ���� ��û ����E(Keeper -> Keeper Manager)
#define MC_REQUEST_KEEPERMGR_ANNOUNCE			31020	///< Keeper Manager���� �� ������ ������ �������� Keeper�� ��û��.(Keeper Manager -> Keeper)
#define MC_REQUEST_KEEPER_ANNOUNCE				31021	///< Keeper���� MatchServer�� ���������� �˸����� ��û��.(Keeper -> MatchServer)
#define MC_REQUEST_RESET_PATCH					31022	///< ����+��݁E������ ��ġ �۾����¸� �ʱ�ȭ��.
#define MC_REQUEST_DISCONNECT_SERVER			31023	///< Keeper�� MatchServer���� ����E����E
#define MC_REQUEST_REBOOT_WINDOWS				31024	///< ������E�����.(Keeper Manager -> Keeper)
#define MC_REQUEST_ANNOUNCE_STOP_SERVER			31025	///< �������װ�E�԰� ������ ����E��Ŵ ��û.
#define MC_RESPONSE_ANNOUNCE_STOP_SERVER		31026	///< �������װ�E�԰� ������ ����E��Ŵ ��û ����E
#define MC_REQUEST_SERVER_AGENT_STATE			31027	///< ���� ������ ������E��� ��ǁE���� ��û.
#define MC_RESPONSE_SERVER_AGENT_STATE			31028	///< ���� ������ ������E��� ��ǁE���� ��û ����E
#define MC_REQUEST_WRITE_CLIENT_CRC				31029	///< Ŭ���̾�Ʈ�� CRC CheckSum�� ����Eini���Ͽ� ����E��û.(KeeperManager -> Keeper)
#define MC_RESPONSE_WRITE_CLIENT_CRC			31030	///< Ŭ���̾�Ʈ�� CRC CheckSum�� ����Eini���Ͽ� ����E��û ����E(Keeper -> Keeper Manager)
#define MC_REQUEST_KEEPER_RELOAD_SERVER_CONFIG	31031	///< Keeper�� ����Ǿ��ִ� ������ ���� ������ �ٽ� �ε��ϵ��� ��û��.(KeeperManager -> Keeper)
#define MC_REQUEST_RELOAD_CONFIG				31032	///< ������ ���� ������ �ٽ� �δ��ϵ��� ��û.(Keeper -> server)
#define MC_REQUEST_KEEPER_ADD_HASHMAP			31033	
#define MC_RESPONSE_KEEPER_ADD_HASHMAP			31034
#define MC_REQUEST_ADD_HASHMAP					31035
#define MC_RESPONSE_ADD_HASHMAP					31036

// MatchServer ��ġ ��E�.
#define MC_REQUEST_DOWNLOAD_SERVER_PATCH_FILE	32024	///< Keeper�� FTP�������� Patch������ �ٿ��ε�E��Ŵ.(Keeper Manager -> Keeper )
#define MC_REQUEST_START_SERVER					32026	///< MatchServer��ǁE��û.(Keeper Manager -> Keeper)
#define MC_REQUEST_STOP_SERVER					32028	///< ����E����E��û.(Keeper -> Keeper Manager)
#define MC_REQUEST_PREPARE_SERVER_PATCH			32030	///< ��ġ �غ�E�۾�E(��ÁEǮ��E �ӽ� ���� ����.)
#define MC_REQUEST_SERVER_PATCH					32032	///< ��ġ ������ ������.(Keeper Manager -> Keeper )
// MatchAgent ��ġ ��E�.
#define MC_REQUEST_STOP_AGENT_SERVER			33034	///< Agent server����E��û.(Keeper Manager -> Keeper )
#define MC_REQUEST_START_AGENT_SERVER			33036	///< Agent server���� ��û.(Keeper Manager -> Keeper)
#define MC_REQUEST_DOWNLOAD_AGENT_PATCH_FILE	33038	///< request agent patch file download.(Keeper Manager -> Keeper)
#define MC_REQUEST_PREPARE_AGENT_PATCH			33040	///< reqeust prepare agent patch.(Keeper Manager -> Keeper)
#define MC_REQUEST_AGENT_PATCH					33042	///< request agent patch.(Keeper Manager -> Keeper)
#define MC_REQUEST_SERVER_STATUS				33043	///< server�� agent�� ������ Keeper�� ��û��.(Keeper Manager -> keeper)
#define MC_RESPONSE_SERVER_STATUS				33044	///< server�� agetn�� ���� ��û ����E(Keeper -> Keeper Manager)
// Keeper manager schedule.
#define MC_REQUEST_KEEPER_MANAGER_SCHEDULE		34001	///< request keeper manager schedule.(Keeper Manager -> Keeper -> MatchServer)
#define MC_RESPONSE_KEEPER_MANAGER_SCHEDULE		34002	///< Keeper Manager�� ������E��� ��û ����E
#define MC_REQUEST_START_SERVER_SCHEDULE		34003	///< �ٽý��� �����ٿ��� ������ �����ϴ� ������. �����Ҷ���݁E�������� ���ŵ���E����.
///


/// Locator��E�.
#define MC_REQUEST_SERVER_LIST_INFO				40001	/// ���Ӱ����� ������ ����Ʈ ���� ��û.(Client -> Locator)
#define MC_RESPONSE_SERVER_LIST_INFO			40002	/// ���Ӱ����� ����E����Ʈ ���� ����E(Locator -> Client)
#define MC_RESPONSE_BLOCK_COUNTRY_CODE_IP		40003	/// ���ӺҰ� ���� �ڵ��� IP����E(Locator -> Client)


// filter.
#define MC_RESPONSE_BLOCK_COUNTRYCODE			50001	/// ��Ӱ�� �����ڵ��� IP�� ���ӽ� ŁE���E
#define MC_LOCAL_UPDATE_USE_COUNTRY_FILTER		50002
#define MC_LOCAL_GET_DB_IP_TO_COUNTRY			50003
#define MC_LOCAL_GET_DB_BLOCK_COUNTRY_CODE		50004
#define MC_LOCAL_GET_DB_CUSTOM_IP				50005
#define MC_LOCAL_UPDATE_IP_TO_COUNTRY			50006
#define MC_LOCAL_UPDATE_BLOCK_COUTRYCODE		50007
#define MC_LOCAL_UPDATE_CUSTOM_IP				50008
#define MC_LOCAL_UPDATE_ACCEPT_INVALID_IP		50009
#define MC_ADMIN_GIVE_COINSCASH					50010
#define MC_ADMIN_GIVE_COINSMEDALS				50011
#define MC_ADMIN_SENDITEM					    50012
#define MC_STAFF_CHAT							50013
#define MC_GUNZ_ANTILEAD						50014 // Orby: (AntiLead).

// MagicBox
#define MC_DROPGUNGAME_RESPONSE_ENTERGAME		50016
#define MC_DROPGUNGAME_REQUEST_ITEM				50017
#define MC_DROPGUNGAME_RESPONSE_ITEM			50018
#define MC_DROPGUNGAME_RESPONSE_WORLDITEMS		50019


#define MC_MATCH_PLAYERWARS_VOTE				50020
#define MC_MATCH_PLAYERWARS_VOTE_UPDATE			50021
#define MC_MATCH_PLAYERWARS_RANDOM_MAPS			50022

#define MC_GUNZ_LASTDMG							50023
#define MC_GUNZ_DMGGIVEN						50024
#define MC_KILL_ALL                             50025
#define MC_ADMIN_TELEPORT                       50026
#define MC_TELEPORT_ALL							50027
#define MC_ADMIN_FREEZE_ALL						50028
#define MC_REPORT_USER							50029
#define MC_ADMIN_STAGE_JOIN						50030

////////// Spy Mode //////////
#define MC_SPY_STAGE_BAN_MAP_LIST					50032
#define MC_SPY_STAGE_ACTIVATE_MAP					50033
#define MC_SPY_STAGE_REQUEST_START					50034
#define MC_SPY_GAME_INFO							50035
#define MC_SPY_GAME_RESULT							50036
#define MC_SPY_GAME_SCORE							50037
#define MC_SPY_STAGE_REQUEST_BAN_MAP_LIST			50038
#define MC_MATCH_FREEZESPY							50039
#define MC_MATCH_UNFREEZESPY					    50040
#define MC_MATCH_SPY_ROUNDRESULT			    	50041

//////////////////ChatVoice/////////////////////////////


//////////////////LadderStreak/////////////////////////////
#define MC_MATCH_PLAYERWARS_CHARINFO			50052
#define MC_MATCH_JOIN_PLAYERWARS				50053
#define MC_MATCH_PLAYERWARS_COUNTER				50054
#define MC_MATCH_PLAYERWARS_SIDERANK			50055
#define MC_MATCH_PLAYERWARS_INVITED				50056
#define MC_MATCH_PLAYERWARS_FRIENDINVITE		50057
#define MC_MATCH_PLAYERWARS_FRIENDACCEPT		50058
#define MC_MATCH_PLAYERWARS_FRIENDLEAVE			50059

#define MC_ADMIN_WALL							50060
#define MC_UGRADEID_SEND                        50061

#define MC_ADMIN_GIVE_COINSMEDALS_RESPONSE		50065
#define MC_GUNZ_HPAP_DATA						50066
#endif

#define MC_MATCH_SEND_VOICE_CHAT               50067
#define MC_MATCH_RECEIVE_VOICE_CHAT            50068