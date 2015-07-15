#include "hud.h"
#include "cl_util.h"

#include "hitbox_renderer.h"


#ifdef WIN32
#include "windows.h"
#include "gl/GL.h"
#endif


hitinfo_t g_HitInfo[16];
int g_iHitInfo;

player_pos_trace_point_t g_PlayerPosTraces[1024];
int g_PlayePossTracesNum;

struct color3f_t {
	float r, g, b;
};

extern globalvars_t *gpGlobals;

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

void DrawHitInfo(const hitinfo_t& hi) {
	hitinfo_t* phi = &g_HitInfo[g_iHitInfo++];
	if (g_iHitInfo >= ARRAYSIZE(g_HitInfo)) {
		g_iHitInfo = 0;
	}

	*phi = hi;
	phi->used = true;
	phi->dieTime = gpGlobals->time + 30.0;
}

void DrawHitBox(short traceId, int hbId, const hitbox_t& hb) {
	hitinfo_t* phi = NULL;
	for (int i = 0; i < ARRAYSIZE(g_HitInfo); i++) {
		if (traceId == g_HitInfo[i].traceId) {
			phi = &g_HitInfo[i];
			break;
		}
	}

	if (phi == NULL) {
		gEngfuncs.Con_Printf("Received hitbox that doesn't correspond with ani hit\n");
		return;
	}

	if (hbId > 0 && hbId < 32) {
		phi->hitboxes[hbId] = hb;
		phi->hitboxes[hbId].used = true;
	}
}

void TracePlayerPos(Vector& pos, Vector &color, unsigned int delayMsec) {
	player_pos_trace_point_t *trace = &g_PlayerPosTraces[g_PlayePossTracesNum++];
	if (g_PlayePossTracesNum >= ARRAYSIZE(g_PlayerPosTraces)) {
		g_PlayePossTracesNum = 0;
	}

	unsigned int curTime = GetTickCount();
	trace->active = true;
	trace->displayAfter = curTime + delayMsec;
	trace->displayBefore = curTime + delayMsec + 700;
	trace->p = pos;
	trace->color = color;
}

void RenderPlayerTraces() {
	unsigned int curTime = GetTickCount();
	for (int i = 0; i < ARRAYSIZE(g_PlayerPosTraces); i++) {
		player_pos_trace_point_t *trace = &g_PlayerPosTraces[i];
		if (!trace->active || curTime < trace->displayAfter || curTime > trace->displayBefore)
			continue;

		glPointSize(2.0);
		glColor3fv(trace->color);

		float size = 2.0f;
		glBegin(GL_POINTS);
		glVertex3fv(trace->p + Vector(-size, -size, -size + 40.0f));
		glVertex3fv(trace->p + Vector(-size, +size, -size + 40.0f));
		glVertex3fv(trace->p + Vector(+size, -size, -size + 40.0f));
		glVertex3fv(trace->p + Vector(+size, +size, -size + 40.0f));
		glVertex3fv(trace->p + Vector(-size, -size, +size + 40.0f));
		glVertex3fv(trace->p + Vector(-size, +size, +size + 40.0f));
		glVertex3fv(trace->p + Vector(+size, -size, +size + 40.0f));
		glVertex3fv(trace->p + Vector(+size, +size, +size + 40.0f));
		glEnd();
	}
}

void RegisterActualPlayerPositions() {
	cl_entity_s* me = gEngfuncs.GetLocalPlayer();
	for (int i = 0; i < 32; i++) {
		cl_entity_s* ent = gEngfuncs.GetEntityByIndex(i + 1);
		if (me == ent || !ent->player) {
			continue;
		}

		Vector color(0.0f, 1.0f, 0.0f);
		TracePlayerPos(ent->origin, color, 0);
	}
}

void RenderHitboxes() {
	RegisterActualPlayerPositions();

	glPushAttrib(GL_ENABLE_BIT | GL_DEPTH_BUFFER_BIT | GL_CURRENT_BIT | GL_COLOR_BUFFER_BIT | GL_ACCUM_BUFFER_BIT | GL_POLYGON_BIT);
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_CULL_FACE);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	for (int i = 0; i < ARRAYSIZE(g_HitInfo); i++) {
		hitinfo_t* phi = &g_HitInfo[i];
		if (!phi->used || phi->dieTime <= gpGlobals->time)
			continue;

		//draw hitboxes
		for (int j = 0; j < 32; j++) {
			hitbox_t* phb = &phi->hitboxes[j];
			if (!phb->used)
				continue;

			int colorIdx = j % ARRAYSIZE(g_HitboxColors);
			glColor4f(g_HitboxColors[colorIdx].r, g_HitboxColors[colorIdx].g, g_HitboxColors[colorIdx].b, 0.5f);
			glLineWidth(phi->hitGroup == phb->groupId ? 5.0f : 1.0f);

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

		//draw traceline

		glColor4f(1, 0, 0, 0.5f);
		glLineWidth(5.0f);

		glBegin(GL_LINES);

		glVertex3fv(phi->serverTraceStart);
		glVertex3fv(phi->serverTraceEnd);

		glEnd();
	}

	RenderPlayerTraces();

	glPopAttrib();
}