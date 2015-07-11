#include "extdll.h"
#include "util.h"
#include "hitbox_renderer.h"

#ifdef WIN32
#include "windows.h"
#include "gl/GL.h"
#endif

hitbox_t g_Hitboxes[MAX_HITBOXES];
int g_iHitbox;

struct color3f_t {
	float r, g, b;
};

color3f_t g_HitboxColors[] = {
	{ 1.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f },
	{ 0.7f, 0.0f, 0.0f },
	{ 0.0f, 0.7f, 0.0f },
	{ 0.0f, 0.0f, 0.7f },
	{ 0.7f, 0.7f, 0.7f },
	{ 0.9f, 0.9f, 0.9f },
	{ 1.0f, 0.7f, 0.0f },
	{ 1.0f, 1.0f, 0.0f },
};

void DrawHitbox(int id, Vector base, Vector xEdge, Vector yEdge, Vector zEdge) {
	hitbox_t* phb = &g_Hitboxes[g_iHitbox++];
	phb->id = id;
	phb->basepoint = base;
	phb->xEdge = xEdge;
	phb->yEdge = yEdge;
	phb->zEdge = zEdge;
	phb->dieTime = gpGlobals->time + 30.0;
}

void RenderHitboxes() {
	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_POLYGON_BIT);
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < MAX_HITBOXES; i++) {
		hitbox_t* phb = &g_Hitboxes[i];
		if (phb->id <= 0 || phb->dieTime <= gpGlobals->time)
			continue;

		int colorIdx = phb->id % ARRAYSIZE(g_HitboxColors);
		glColor4f(g_HitboxColors[colorIdx].r, g_HitboxColors[colorIdx].g, g_HitboxColors[colorIdx].b, 0.5f);

		Vector rearBase = phb->basepoint + phb->yEdge;

		glBegin(GL_QUAD_STRIP);

		glVertex3fv(phb->basepoint);
		glVertex3fv(phb->basepoint + phb->xEdge);

		glVertex3fv(phb->basepoint + phb->zEdge);
		glVertex3fv(phb->basepoint + phb->zEdge + phb->xEdge);

		glVertex3fv(rearBase + phb->zEdge);
		glVertex3fv(rearBase + phb->zEdge + phb->xEdge);

		glVertex3fv(rearBase);
		glVertex3fv(rearBase + phb->xEdge);

		glVertex3fv(phb->basepoint);
		glVertex3fv(phb->basepoint + phb->xEdge);

		glEnd();
	}

	glPopAttrib();
}