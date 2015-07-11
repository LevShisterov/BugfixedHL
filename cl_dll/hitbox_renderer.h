#pragma once

struct hitbox_t {
	bool used;
	short groupId;
	Vector basepoint;
	Vector xEdge, yEdge, zEdge;
};

struct hitinfo_t {
	bool used;
	short traceId;
	hitbox_t hitboxes[32];
	double dieTime;
	Vector serverTraceStart;
	Vector serverTraceEnd;
	int hitGroup;
};

extern hitinfo_t g_HitInfo[16];
extern int g_iHitInfo;

extern void DrawHitInfo(const hitinfo_t& hi);
extern void DrawHitBox(short traceId, int hbId, const hitbox_t& hb);
extern void RenderHitboxes();