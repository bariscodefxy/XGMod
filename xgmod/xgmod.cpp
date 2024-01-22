#include <extdll.h>			// always
#include <meta_api.h>		// of course
#include <engine_api.h>
#include <dllapi.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdarg.h>
#include <cbase.h>
#include <player.h>
#include "xgmod.h"
#include "entity_state.h"

#ifdef _WIN32
#define VirtFuncSpawn        0
#define VirtFuncTakeDamage    12
#define VirtFuncKilled        14
#elif __linux__
#define VirtFuncSpawn        2
#define VirtFuncTakeDamage    14
#define VirtFuncKilled        16
#include <sys/mman.h>
#endif

typedef struct admin_s
{
	std::string username;
	std::string password;
} admin_t;

cvar_t xg_hud_enabled = { "xg_hud_enabled", "1.0", FCVAR_SERVER };
cvar_t xg_hud_customtext = { "xg_hud_customtext", "", FCVAR_SERVER };
cvar_t xg_hud_damage  = { "xg_hud_damage", "1.0", FCVAR_SERVER };
cvar_t xg_hud_rainbow  = { "xg_hud_rainbow", "0.0", FCVAR_SERVER };

byte hud_colors[3] = {
	255,
	255,
	255
};

admin_t admins[] = {
	{
		"bariscodefx",
		"lolw123"
	}
};
int admincount = 1;

std::string admin_logins[32];
int clogin = 0;

void *pOrigFuncTakeDamage;
int gmsgTextMsg = 0, gmsgSayText = 0;

void HudMessage(edict_t *pent, const hudtextparms_s &textparms, const char *pMessage);


CBaseEntity	*UTIL_PlayerByIndex(int playerIndex)
{
	CBaseEntity *pPlayer = NULL;

	if (playerIndex > 0 && playerIndex <= gpGlobals->maxClients)
	{
		edict_t *pPlayerEdict = INDEXENT(playerIndex);
		if (pPlayerEdict && !pPlayerEdict->free)
		{
			pPlayer = CBaseEntity::Instance(pPlayerEdict);
		}
	}

	return pPlayer;
}

void UTIL_SayText(const char *pText, CBaseEntity *pEntity)
{
	if (!pEntity->IsNetClient())
		return;

	MESSAGE_BEGIN(MSG_ONE, gmsgSayText, NULL, pEntity->edict());
	WRITE_BYTE(pEntity->entindex());
	WRITE_STRING(pText);
	MESSAGE_END();
}

#ifdef _WIN32
int __fastcall HookTakeDamage(void *pthis, int i, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamage)
#elif __linux__
int HookTakeDamage(void *pthis, entvars_t *pevInflictor, entvars_t *pevAttacker, float flDamage, int bitsDamage)
#endif
{
#ifdef _WIN32
	int iOrigRet = reinterpret_cast<int(__fastcall *)(void *, int, entvars_t *, entvars_t *, float, int)>(pOrigFuncTakeDamage)(pthis, 0, pevInflictor, pevAttacker, flDamage, bitsDamage);
#elif __linux__
	int iOrigRet = reinterpret_cast<int(*)(void *, entvars_t *, entvars_t *, float, int)>(pOrigFuncTakeDamage)(pthis, pevInflictor, pevAttacker, flDamage, bitsDamage);
#endif

	if (CVAR_GET_FLOAT("xg_hud_damage"))
	{
		if (pevInflictor && pevAttacker && bitsDamage && flDamage)
		{
			char pMessage[512];
			snprintf(pMessage, sizeof(pMessage), "%f", flDamage);
			HudMessage(ENT(pevInflictor), { 0.4f + RANDOM_FLOAT(0.05f, 0.25f), -1.0f, 0, 0, 155, 255, 0, 0, 0, 0, 0, 0.0f, 0.0f, 1.0f, 0.0f, 4 }, pMessage);
		}
	}


	return iOrigRet;
}

void MakeHookTakeDamage(void)
{
	edict_t *pEdict = CREATE_ENTITY();

	CALL_GAME_ENTITY(PLID, "player", &pEdict->v);

	if (pEdict->pvPrivateData == NULL)
	{
		REMOVE_ENTITY(pEdict);

		return;
	}

#ifdef _WIN32
	void **vtable = *((void***)((char*)pEdict->pvPrivateData));
#elif __linux__
	void **vtable = *((void***)(((char*)pEdict->pvPrivateData) + 0x94));
#endif

	REMOVE_ENTITY(pEdict);

	if (vtable == NULL)
		return;

	int **ivtable = (int **)vtable;

	pOrigFuncTakeDamage = (void *)ivtable[VirtFuncTakeDamage];

#ifdef _WIN32
	DWORD OldFlags;

	VirtualProtect(&ivtable[VirtFuncTakeDamage], sizeof(int *), PAGE_READWRITE, &OldFlags);
#elif __linux__
	mprotect(&ivtable[VirtFuncTakeDamage], sizeof(int*), PROT_READ | PROT_WRITE);
#endif

	ivtable[VirtFuncTakeDamage] = (int *)HookTakeDamage;
}


int AddToFullPack_Post(entity_state_s* state, int e, edict_t* ent, edict_t* host, int hostflags, BOOL player, unsigned char* pSet)
{	
	if (player)
	{
		if (CVAR_GET_FLOAT("xg_hud_enabled"))
		{
			char pMessage[512];
			snprintf(pMessage, sizeof(pMessage), "XGMOD v1.1\n%s", CVAR_GET_STRING("xg_hud_customtext"));
			if(CVAR_GET_FLOAT("xg_hud_rainbow")) {
				hud_colors[0] = rand() % 255;
				hud_colors[1] = rand() % 255;
				hud_colors[2] = rand() % 255;
			}
			HudMessage(ENT(ent), { 0.05f, 0.05f, 0, hud_colors[0], hud_colors[1], hud_colors[2], 0, 0, 0, 0, 0, 0.0f, 0.0f, 1.0f, 0.0f, 3 }, pMessage);
		}
	}

	RETURN_META_VALUE(MRES_IGNORED, 0);
}

extern unsigned short FixedUnsigned16(float value, float scale)
{
	int output;

	output = value * scale;
	if (output < 0)
		output = 0;
	if (output > 0xFFFF)
		output = 0xFFFF;

	return (unsigned short)output;
}

extern short FixedSigned16(float value, float scale)
{
	int output;

	output = value * scale;

	if (output > 32767)
		output = 32767;

	if (output < -32768)
		output = -32768;

	return (short)output;
}

CBaseEntity	*PlayerByIndex(int playerIndex)
{
	CBaseEntity *pPlayer = NULL;

	if (playerIndex > 0 && playerIndex <= gpGlobals->maxClients)
	{
		edict_t *pPlayerEdict = INDEXENT(playerIndex);
		if (pPlayerEdict && !pPlayerEdict->free)
		{
			pPlayer = CBaseEntity::Instance(pPlayerEdict);
		}
	}

	return pPlayer;
}

void HudMessage(edict_t *pent, const hudtextparms_s &textparms, const char *pMessage)
{
	MESSAGE_BEGIN(MSG_ONE, SVC_TEMPENTITY, NULL, ENT(pent));
	WRITE_BYTE(TE_TEXTMESSAGE);
	WRITE_BYTE(textparms.channel & 0xFF);
	
	WRITE_SHORT(FixedSigned16(textparms.x, 1 << 13));
	WRITE_SHORT(FixedSigned16(textparms.y, 1 << 13));
	WRITE_BYTE(textparms.effect);

	WRITE_BYTE(textparms.r1);
	WRITE_BYTE(textparms.g1);
	WRITE_BYTE(textparms.b1);
	WRITE_BYTE(textparms.a1);

	WRITE_BYTE(textparms.r2);
	WRITE_BYTE(textparms.g2);
	WRITE_BYTE(textparms.b2);
	WRITE_BYTE(textparms.a2);

	WRITE_SHORT(FixedUnsigned16(textparms.fadeinTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.fadeoutTime, 1 << 8));
	WRITE_SHORT(FixedUnsigned16(textparms.holdTime, 1 << 8));

	if (textparms.effect == 2)
		WRITE_SHORT(FixedUnsigned16(textparms.fxTime, 1 << 8));

	if (strlen(pMessage) < 512)
	{
		WRITE_STRING(pMessage);
	}
	else
	{
		char tmp[512];
		strncpy(tmp, pMessage, 511);
		tmp[511] = 0;
		WRITE_STRING(tmp);
	}
	MESSAGE_END();
}

qboolean isUserAdmin(edict_t* pEdicts)
{
	const char* userinfo = g_engfuncs.pfnGetInfoKeyBuffer(pEdicts);
	const char* username = g_engfuncs.pfnInfoKeyValue((char*)userinfo, "name");

	for (int i = 0; i < clogin; i++)
	{
		if (!strcmp(username, admin_logins[i].c_str()))
		{
			return(TRUE);
		}
	}

	return(FALSE);
}

void XG_Login(edict_t* pEdicts)
{
	if (g_engfuncs.pfnCmd_Argc() < 2)
	{
		g_engfuncs.pfnClientPrintf(pEdicts, print_console, "Usage: xg_login <password>\n");
		return;
	}

	const char* userinfo = g_engfuncs.pfnGetInfoKeyBuffer(pEdicts);
	const char* username = g_engfuncs.pfnInfoKeyValue((char*)userinfo, "name");
	const char* password = g_engfuncs.pfnCmd_Argv(1);

	for (int i = 0; i < admincount; i++)
	{
		if (!strcmp(password, admins[i].password.c_str()) && !strcmp(username, admins[i].username.c_str()))
		{
			// login success
			admin_logins[clogin] = std::string(username);
			clogin++;
			g_engfuncs.pfnClientPrintf(pEdicts, print_console, "Successfully logined to your account!\n");
			return;
		}
	}

	g_engfuncs.pfnClientPrintf(pEdicts, print_console, "Invalid username or password.\n");
}

void XG_Ban(edict_t* pEdicts)
{
	if (g_engfuncs.pfnCmd_Argc() < 2)
	{
		g_engfuncs.pfnClientPrintf(pEdicts, print_console, "Usage: xg_ban <username>\n");
		return;
	}

	const char *username = g_engfuncs.pfnCmd_Argv(1);

	if (!isUserAdmin(pEdicts))
	{
		g_engfuncs.pfnClientPrintf(pEdicts, print_console, "You don't have enough permissions for this!\n");
		return;
	}

	g_engfuncs.pfnClientPrintf(pEdicts, print_console, "User not found.\n");
}

char* strafter(const char *str, int after)
{
	char newstr[sizeof(str)];
	for (int i = 0; i < sizeof(str) - 1; i++)
	{
		newstr[i] = str[i + after];
	}
	return newstr;
}

void pfnClientDisconnect(edict_t* pEdicts)
{
	const char* userinfo = g_engfuncs.pfnGetInfoKeyBuffer(pEdicts);
	const char* username = g_engfuncs.pfnInfoKeyValue((char*)userinfo, "name");

	for (int i = 0; i < clogin; i++)
	{
		if (!strcmp(username, admin_logins[i].c_str()))
		{
			admin_logins[i] = '\0';
		}
	}

	RETURN_META(MRES_HANDLED);
}

void pfnClientCommand(edict_t* pEdicts)
{
	if (g_engfuncs.pfnCmd_Argc() < 1)
		RETURN_META(MRES_IGNORED);

	const char *cmd = g_engfuncs.pfnCmd_Argv(0);
	const char *args = g_engfuncs.pfnCmd_Args();

	if (!strcmp(cmd, "xg_ban"))
	{
		XG_Ban(pEdicts);
		RETURN_META(MRES_SUPERCEDE);
	}
	else if (!strcmp(cmd, "xg_login"))
	{
		XG_Login(pEdicts);
		RETURN_META(MRES_SUPERCEDE);
	}

	RETURN_META(MRES_IGNORED);
}

void XG_Init(void)
{
	UTIL_LogPrintf("Initializing XG MOD\n");

	for (int i = 0; i < clogin; i++)
	{
		admin_logins[i] = '\0';
	}
	clogin = 0;
	gmsgSayText = REG_USER_MSG("SayText", -1);
	gmsgTextMsg = REG_USER_MSG("TextMsg", -1);
	g_engfuncs.pfnCVarRegister(&xg_hud_enabled);
	g_engfuncs.pfnCVarRegister(&xg_hud_customtext);
	g_engfuncs.pfnCVarRegister(&xg_hud_damage);
	g_engfuncs.pfnCVarRegister(&xg_hud_rainbow);
	//g_engfuncs.pfnCVarRegister(&xg_anti_csqq);

	MakeHookTakeDamage();

	RETURN_META(MRES_IGNORED);
}

void XG_Stop(void)
{
	UTIL_LogPrintf("Stopping XG MOD\n");
	RETURN_META(MRES_IGNORED);
}
