#pragma once

// SrvMgr version
#define SRVMGR_VER_HI  2
#define SRVMGR_VER_LO 00

// Server flags
#define SVF_CLOSED	        0x00000001
#define SVF_PVM		        0x00000002
#define SVF_MUTED	        0x00000004
#define SVF_ADVPVM	        0x00000040
#define SVF_ONEMAP	        0x00000080
#define SVF_NOHEALING       0x00000100
#define SVF_NOOBSRV	        0x00000200
#define SVF_SOFTCORE        0x00000400
#define SVF_NODROP	        0x00000800
#define SVF_FNODROP	        0x00001000
#define SVF_NOSAVING        0x00002000
#define SVF_SANDBOX         0x00004000
#define SVF_ENTERMAGE       0x00008000
#define SVF_ENTERWARRIOR    0x00010000

// GM command flags
// note this list is NOT compatible with SrvMgr 1.x flags
#define GMF_ANY			0x3F000000
#define GMF_CMD_KICK		0x3F000001	// #kick, #kickall, #kickme
#define GMF_CMD_INFO		0x3F000002	// #info, #locate
#define GMF_CMD_KILL		0x3F000004	// #kill, #killall, #killai
#define GMF_CMD_PICKUP		0x3F000008	// #pickup
#define GMF_CMD_SUMMON		0x3F000010	// #summon
#define GMF_CMD_CREATE		0x3F000020	// #create
#define GMF_CMD_MODIFY		0x3F000040	// #modify
#define GMF_CMD_SET		0x3F000080	// #set
#define GMF_CMD_MAPLIST		0x3F000100	// #resetmap, #nextmap, #suspend
#define GMF_CMD_SHUTDOWN	0x3F000200	// #shutdown
#define GMF_CMD_CHAT		0x3F000400	// #@message, #!message
// GM misc flags
#define GMF_INVISIBLE		0x3F000800	// admin invisibility
#define GMF_GODMODE		0x3F001000	// god mode
#define GMF_GODMODE_ADMIN	0x3F002000	// ignore god mode
#define GMF_AI_ALLY		0x3F004000	// auto-ally with monsters
#define GMF_PLAYERS_VISION	0x3F008000	// auto-vision FROM players
#define GMF_PLAYERS_ALLY	0x3F010000	// auto-ally with players
#define GMF_MAXDAMAGE		0x3F020000	// max. damage (32767)
#define GMF_CMD_SCREENSHOT	0x3F040000	// #screenshot

// Game modes
#define GAMEMODE_COOPERATIVE	0
#define GAMEMODE_DEATHMATCH	1
#define GAMEMODE_SOFTCORE	2
#define GAMEMODE_ARENA		3
#define GAMEMODE_SANDBOX	4

// Server capabilities
#define SVC_DETAILED_INFO	0x00000001
#define SVC_FIXED_MAPLIST	0x00000002
#define SVC_SAVE_DATABASE	0x00000008
#define SVC_SOFTCORE		0x00000004
#define SVC_ALL			0x0000000B

// Server logging flags
#define SVL_SAVES	0x00000001
#define SVL_CAMPING	0x00000002
#define SVL_BUILDINGS	0x00000004
#define SVL_DIPLOMACY	0x00000008
#define SVL_ALL		0x0000000F