#pragma once

//sdk_util.cpp
void UTIL_LogPrintf(const char *fmt, ...);

//xg_bhop.cpp
int xg_bhop(edict_t *ent);

//xg_huddamage.cpp
int xg_huddamage(edict_t *ent);

//xgmod.cpp
void HudMessage(edict_t *pent, const hudtextparms_s &textparms, const char *pMessage);
void pfnClientCommand(edict_t* pEdicts);
void XG_Init(void);
void XG_Stop(void);
void XG_Ban(edict_t* pEdicts);
void pfnGameInit(void);
void pfnPlayerPreThink(edict_t *pent);
void pfnClientDisconnect(edict_t *pEdicts);
int AddToFullPack_Post(entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, BOOL player, unsigned char* pSet);