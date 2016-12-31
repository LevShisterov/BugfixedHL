// Martin Webrant (BulliT)

#ifndef __AGLOCATION_H__
#define __AGLOCATION_H__

class AgLocation
{
public:
	AgLocation();
	virtual ~AgLocation();

	void Show();

	//std::string m_sLocation;
	Vector m_vPosition;
};

#endif // __AGLOCATION_H__
