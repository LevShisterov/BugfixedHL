#include	"extdll.h"
#include	"util.h"
#include	"cbase.h"
#include	"player.h"
#include	"hitbox_tracer.h"
#include	"gdll_rehlds_api.h"

short g_TraceCount = 0;

void WriteHiresFloat(float fl) {
	int i = (int)((double)fl * 512.0);
	int neg = (i < 0) ? 1 : 0;
	if (neg) {
		i = -i;
	}
	WRITE_BYTE(i & 0xFF);
	WRITE_BYTE((i >> 8) & 0xFF);
	WRITE_BYTE(((i >> 16) & 0x7F) | (neg << 7));
}

void SetupHitboxesTracing() {
	g_RehldsFuncs->SetupHitboxTracing();
}

void TraceHitboxes2(CBasePlayer* who, Vector &start, const TraceResult &tr) {
	short traceId = g_TraceCount++;

	MESSAGE_BEGIN(MSG_ALL, gmsgHitInfo, NULL);
	WRITE_SHORT(traceId);
	WriteHiresFloat(start.x); WriteHiresFloat(start.y); WriteHiresFloat(start.z);
	WriteHiresFloat(tr.vecEndPos.x); WriteHiresFloat(tr.vecEndPos.y); WriteHiresFloat(tr.vecEndPos.z);
	WRITE_BYTE(tr.iHitgroup);
	MESSAGE_END();

	for (int i = 0; i < 32; i++) {
		float corners[24];
		int groupId = 0;
		if (!g_RehldsFuncs->GetHitboxCorners(i, corners, &groupId))
			continue;

		MESSAGE_BEGIN(MSG_ALL, gmsgHitbox, NULL);
		WRITE_SHORT(traceId);
		WRITE_CHAR(i);
		WRITE_SHORT(groupId);
		WriteHiresFloat(corners[0]); WriteHiresFloat(corners[1]); WriteHiresFloat(corners[2]); //Base point: front-left-bottom
		WriteHiresFloat(corners[3] - corners[0]); WriteHiresFloat(corners[4] - corners[1]); WriteHiresFloat(corners[5] - corners[2]); //Z edge
		WriteHiresFloat(corners[9] - corners[0]); WriteHiresFloat(corners[10] - corners[1]); WriteHiresFloat(corners[11] - corners[2]); //X edge
		WriteHiresFloat(corners[12] - corners[0]); WriteHiresFloat(corners[13] - corners[1]); WriteHiresFloat(corners[14] - corners[2]); //Y edge
		MESSAGE_END();
	}

}