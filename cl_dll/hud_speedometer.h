#ifndef __HUD_SPEEDOMETER_H__
#define __HUD_SPEEDOMETER_H__

class CHudSpeedometer : public CHudBase
{
	int speed;

	cvar_t *hud_speedometer;

public:
	virtual int Init();
	virtual int VidInit();
	virtual int Draw(float time);

	void UpdateSpeed(const float velocity[2]);

};

#endif // __HUD_SPEEDOMETER_H__