#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"hitbox_tracer.h"
#include	"gdll_rehlds_api.h"

void TraceHitboxes(CBasePlayer* who) {
	float corners[3*8];
	for (int i = 0; i < 30; i++) {
		if (!g_RehldsFuncs->GetHitboxCorners(i, corners))
			continue;

		MESSAGE_BEGIN(MSG_ONE, gmsgHitbox, NULL, who->edict());
		WRITE_BYTE(i);
		WRITE_COORD(corners[0]); WRITE_COORD(corners[1]); WRITE_COORD(corners[2]); //Base point: front-left-bottom
		WRITE_COORD(corners[3] - corners[0]); WRITE_COORD(corners[4] - corners[1]); WRITE_COORD(corners[5] - corners[2]); //Z edge
		WRITE_COORD(corners[9] - corners[0]); WRITE_COORD(corners[10] - corners[1]); WRITE_COORD(corners[11] - corners[2]); //X edge
		WRITE_COORD(corners[12] - corners[0]); WRITE_COORD(corners[13] - corners[1]); WRITE_COORD(corners[14] - corners[2]); //Y edge
		MESSAGE_END();
	}
}
