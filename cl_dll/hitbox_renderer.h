#pragma once

struct hitbox_t {
	int id;
	double dieTime;
	Vector basepoint;
	Vector xEdge, yEdge, zEdge;
};

#define MAX_HITBOXES 192

extern hitbox_t g_Hitboxes[MAX_HITBOXES];
extern int g_iHitbox;

extern void DrawHitbox(int id, Vector base, Vector xEdge, Vector yEdge, Vector zEdge);
extern void RenderHitboxes();