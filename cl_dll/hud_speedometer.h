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
	int m_iOldSpeed;
	int m_iSpeed;
	float m_fFade;
	cvar_t *m_pCvarSpeedometer;
};

#endif // __HUD_SPEEDOMETER_H__