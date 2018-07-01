// Martin Webrant (BulliT)

#ifndef __AGLOCATION_H__
#define __AGLOCATION_H__

const int AG_MAX_LOCATION_NAME = 32;
const int AG_MAX_LOCATIONS = 512;

class AgLocation
{
public:
	AgLocation* m_nextLocation;

	AgLocation();
	virtual ~AgLocation();

	void Show();

	char m_sLocation[AG_MAX_LOCATION_NAME];
	Vector m_vPosition;
};

#endif // __AGLOCATION_H__
