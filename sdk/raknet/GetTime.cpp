/// \file
///
/// This file is part of RakNet Copyright 2003 Kevin Jenkins.
///
/// Usage of RakNet is subject to the appropriate license agreement.
/// Creative Commons Licensees are subject to the
/// license found at
/// http://creativecommons.org/licenses/by-nc/2.5/
/// Single application licensees are subject to the license found at
/// http://www.rakkarsoft.com/SingleApplicationLicense.html
/// Custom license users are subject to the terms therein.
/// GPL license users are subject to the GNU General Public
/// License as published by the Free
/// Software Foundation; either version 2 of the License, or (at your
/// option) any later version.

#include "GetTime.h"

#ifdef _WIN32
#include <Windows.h>
#else
#include <sys/time.h>
#include <unistd.h>
static RakNet::Time64 initialTime;
static bool initialized = false;
#endif

#if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
/// This constraints timer forward jumps to 1 second, and does not let it jump backwards
/// See http://support.microsoft.com/kb/274323 where the timer can sometimes jump forward by hours or days
/// This also has the effect where debugging a sending system won't treat the time spent halted past 1 second as elapsed network time
#include "SimpleMutex.h"
static RakNet::Time64 lastNormalizedReturnedValue = 0;
static RakNet::Time64 lastNormalizedInputValue = 0;
static RakNet::Time64 NormalizeTime(RakNet::Time64 timeIn)
{
	RakNet::Time64 diff, lastNormalizedReturnedValueCopy;
	static SimpleMutex mutex;

	mutex.Lock();
	if (timeIn >= lastNormalizedInputValue)
	{
		diff = timeIn - lastNormalizedInputValue;
		if (diff > GET_TIME_SPIKE_LIMIT)
			lastNormalizedReturnedValue += GET_TIME_SPIKE_LIMIT;
		else
			lastNormalizedReturnedValue += diff;
	}
	else
		lastNormalizedReturnedValue += GET_TIME_SPIKE_LIMIT;

	lastNormalizedInputValue = timeIn;
	lastNormalizedReturnedValueCopy = lastNormalizedReturnedValue;
	mutex.Unlock();

	return lastNormalizedReturnedValueCopy;
}
#endif

RakNet::Time64 RakNet::GetTime64(void)
{
#ifdef _WIN32
	Time64 curTime;
	LARGE_INTEGER PerfVal, yo1;

	QueryPerformanceFrequency(&yo1);
	QueryPerformanceCounter(&PerfVal);

	__int64 quotient, remainder;
	quotient = ((PerfVal.QuadPart) / yo1.QuadPart);
	remainder = ((PerfVal.QuadPart) % yo1.QuadPart);
	curTime = ((Time64)quotient * (Time64)1000000 +
		(remainder * (Time64)1000000 / yo1.QuadPart));

#if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
	return NormalizeTime(curTime);
#else
	return curTime;
#endif

#else // _WIN32
	timeval tp;
	if (initialized == false)
	{
		gettimeofday(&tp, 0);
		initialized = true;
		initialTime = ((tp.tv_sec) * (Time64)1000000 + (tp.tv_usec));
	}

	Time64 curTime;
	gettimeofday(&tp, 0);
	curTime = ((tp.tv_sec) * (Time64) 1000000 + (tp.tv_usec));

#if defined(GET_TIME_SPIKE_LIMIT) && GET_TIME_SPIKE_LIMIT>0
	return NormalizeTime(curTime - initialTime);
#else
	return curTime - initialTime;
#endif 

#endif // _WIN32
}

RakNet::Time32 RakNet::GetTime32(void)
{
	return (Time32)(GetTime64() / 1000);
}

RakNet::Time RakNet::GetTime(void)
{
	return (Time)(GetTime64() / 1000);
}

