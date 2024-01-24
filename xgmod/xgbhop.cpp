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

int xg_bhop(edict_t *ent) {
    if (CVAR_GET_FLOAT("xg_bhop_enabled"))
    {
        ent->v.fuser2 = 0.0f; // disable slow down after jumping

        if (ent->v.button & 2)
        {
            int flags = ent->v.flags;

            if (flags & FL_WATERJUMP)
            {
                RETURN_META_VALUE(MRES_IGNORED, 0);
                return 0;
            }
            if (ent->v.waterlevel >= 2)
            {
                RETURN_META_VALUE(MRES_IGNORED, 0);
                return 0;
            }
            if (!(flags & FL_ONGROUND))
            {
                RETURN_META_VALUE(MRES_IGNORED, 0);
                return 0;
            }

            float yaw = ent->v.v_angle[1] * (M_PI / 180.0);
            float pitch = -ent->v.v_angle[1] * (M_PI / 180.0);


            if(CVAR_GET_FLOAT("xg_bhop_boost_enabled")) {
                int multipler = CVAR_GET_FLOAT("xg_bhop_boost_multipler");
                if(ent->v.v_angle[1] > 90 || ent->v.v_angle[1] < -90) {
                    multipler *= -1;
                }

                // x-component
                ent->v.velocity[0] = cos(yaw) * cos(pitch) * multipler; 
                // z-component
                ent->v.velocity[1] = sin(yaw) * cos(pitch) * multipler;
            }

            // y-component
            ent->v.velocity[2] = 250.0f;
            
            ent->v.gaitsequence = 6;
        }
    }
}