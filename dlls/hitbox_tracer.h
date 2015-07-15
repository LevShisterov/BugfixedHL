#pragma once

extern int	gmsgHitbox;
extern int gmsgHitInfo;

extern void SetupHitboxesTracing();
extern void TraceHitboxes2(CBasePlayer* who, Vector &start, const TraceResult &tr);
extern void WriteHiresFloat(float fl);