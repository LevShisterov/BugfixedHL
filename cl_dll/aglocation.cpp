// Martin Webrant (BulliT)

#include "hud.h"
#include "cl_util.h"
#include "const.h"
#include "cl_entity.h"
#include "event_api.h"
#include "r_efx.h"
#include "aglocation.h"

AgLocation::AgLocation(): m_nextLocation(NULL)
{
	m_vPosition = Vector(0, 0, 0);
}

AgLocation::~AgLocation()
{
}

void AgLocation::Show()
{
	const int spot = gEngfuncs.pEventAPI->EV_FindModelIndex("sprites/laserdot.spr");
	gEngfuncs.pEfxAPI->R_TempSprite(m_vPosition, vec3_origin, 1, spot, kRenderTransAlpha, kRenderFxNoDissipation, 255.0, 10, FTENT_SPRCYCLE);
}
