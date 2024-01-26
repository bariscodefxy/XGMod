#include <extdll.h>	  // always
#include <meta_api.h> // of course
#include <engine_api.h>
#include <dllapi.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <stdarg.h>
#include <cbase.h>
#include <player.h>
#include <FileSystem.h>
#include "xgmod.h"
#include "entity_state.h"

int xg_huddamage(edict_t *ent) {
    if (CVAR_GET_FLOAT("xg_hud_damage"))
    {
        if( ent->v.dmg_take != 0 ) 
        {
            char pMessage[512];
            snprintf(pMessage, sizeof(pMessage), "%d", static_cast<int>(ent->v.dmg_take));
            if ( ent->v.dmg_inflictor->v.health != 0  ) // fall damage fix
            {
                HudMessage(ent->v.dmg_inflictor, {RANDOM_FLOAT(0.4f, 0.6f), RANDOM_FLOAT(0.4f, 0.6f), 0, 0, 155, 255, 0, 0, 0, 0, 0, 0.0f, 0.0f, 1.0f, 0.0f, 4}, pMessage);
                HudMessage(ent, {0.6f, -1.0f, 0, 255, 0, 100, 0, 0, 0, 0, 0, 0.0f, 0.0f, 1.0f, 0.0f, 4}, pMessage);
            }
        }
    }
}