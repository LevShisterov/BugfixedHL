/***
*
*	Copyright (c) 2012, AGHL.RU. All rights reserved.
*
****/
//
// Common_utils.h
//
// Functions that can be used on both: client and server.
//

#ifdef _WIN32

#include <windows.h>

/*============
String functions
strrepl: replaces substrings in a string
============*/
bool strrepl(char *str, int size, const char *find, const char *repl);

#endif


/*============
CXMutex
============*/
class CXMutex {
	public:
		CXMutex();
		~CXMutex();
		void Lock();
		void Unlock();
		bool TryLock();

	protected:

#if defined(linux)
		pthread_mutex_t m_Mutex;
#else
		CRITICAL_SECTION m_CritSect;
#endif
};


/*============
CXTime: returns high resolution time
============*/
extern double CXTime();
