#ifndef __HUD_SPEEDOMETER_H__
#define __HUD_SPEEDOMETER_H__

class CHudSpeedometer : public CHudBase
{
public:
	virtual int Init();
	virtual int VidInit();
	virtual int Draw(float time);
	void UpdateSpeed(const float velocity[2]);

private:
	int m_iSpeed;
	cvar_t *m_pCvarSpeedometer;
};

#endif // __HUD_SPEEDOMETER_H__