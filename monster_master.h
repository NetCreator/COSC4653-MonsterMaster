////////////////////////////////////////////////////////////////
// Student names: Alejandro Ramirez and Mark Smullen
// Course: COSC4653 - Advanced Networks
// Assignment: #8 - Two Unique Servers
// Source Code File Name: monster_master.h
// Purpose: Provides the #define constants and type definitions
//          for the server and client programs.
// Program's Limitations: None known
// Development Computer: Mac Book Pro
// Operating System: Mac OS X 10.8.3
// Compiler: gcc
// Operational Status: In development...
////////////////////////////////////////////////////////////////

#ifndef MONSTER_MASTER_HEADER
#define MONSTER_MASTER_HEADER

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define SERVER_PORT_PVE 6012
#define SERVER_PORT_PVP 6017
#define PARENT_FIFO "parent.fifo"
#define FIFO_NAME_LENGTH 15
#define MAX_IP_ADDRESS_LENGTH 15
#define MAX_NUM_OF_MOVES 4
#define FALSE 0
#define TRUE  1

#define MAX_USERNAME_TEXT 16
#define MAX_PASSWORD_TEXT 16
#define MAX_MONSTER_TEXT 24
#define MAX_ABILITY_TEXT 24

typedef enum {
    STATUS_NORMAL       = 0x00,     // Default monster status -- no effect
    STATUS_POISON       = 0x01,     // Poisoned
    STATUS_BURN         = 0x02,     // Burned
    STATUS_REGEN        = 0x03,     // Regenerating
    STATUS_SLEEP        = 0x04,     // Asleep
    STATUS_PARA         = 0x05      // Paralyzed
} MonsterStatus;

typedef enum {
    // Client
    C_AUTH_LOGIN                    = 0x00,
    C_AUTH_LOGOUT                   = 0x02,
    C_AUTH_ACCOUNT_CREATE           = 0x03,
    C_BATTLE_INIT                   = 0x05,
    C_BATTLE_MOVE_MESSAGE           = 0x06,
    C_MONSTER_STATS_REQ             = 0x09,
    C_SWAP_MONSTER                  = 0x0C,
    C_SWAP_SERVER                   = 0x0D,
    
    // Server
    S_AUTH_LOGIN                    = 0x00,
    S_AUTH_LOGIN_INVALID            = 0x01,
    S_AUTH_LOGOUT_ACK               = 0x02,
    S_AUTH_ACCOUNT_CREATE_ACK       = 0x03,
    S_AUTH_ACCOUNT_CREATE_INVALID   = 0x04,
    S_BATTLE_ACK                    = 0x05,
    S_BATTLE_MOVE_MESSAGE           = 0x06,
    S_BATTLE_MOVE_INVALID           = 0x07,
    S_BATTLE_TERMINATE              = 0x08,
    S_MONSTER_STATS_ACK             = 0x09,
    S_BATTLE_WAIT                   = 0x0A,
    S_BATTLE_TIMEOUT                = 0x0B,
    S_SWAP_MONSTER_ACK              = 0x0C,
    S_SWAP_SERVER_ACK               = 0x0D
} PacketType;

typedef enum {

    STATE_LOGGED_IN                 = 0x01,
    STATE_IN_BATTLE                 = 0x02,
    STATE_PVP                       = 0x04
} StateType;

typedef struct
{
    int str_s; // strength
    int int_s; // intelligence
    int def_s; // defense
    int wis_s; // wisdom
    int agi_s; // agility
    int hp_max;
    int mp_max;
    int hp;
    int mp;
} statsType;

typedef struct
{
    char abilityName[MAX_ABILITY_TEXT];
    int type;   // 0/physical = str is damage modifier, 1/special = int is damage modifier
    int target; // 0 is user, 1 is opponent
    int damage; // x > 0 = damage; x < 0 = heals *Allows for self-heals
    int cost;   // MP cost
} abilityType;

typedef struct
{
    char monsterName[MAX_MONSTER_TEXT];
    int level;
    int exp;
    MonsterStatus status; // status such as: poisoned, regen, burning, etc.
    statsType stats;
    abilityType ability[MAX_NUM_OF_MOVES];
} monsterType;

typedef struct
{
    char username[MAX_USERNAME_TEXT];
    char passhash[MAX_PASSWORD_TEXT];
    monsterType monster;
} userType;

// Function Prototypes

#endif
