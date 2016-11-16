#include <windows.h>
#include <math.h>

// Time is in seconds
// First call returns 0.0 and subsequent calls return the change in time
double NGetTime() 
{
  // returns the amount of time that has elapsed 
  // since the FIRST call to NGetTime, in seconds.
  // (starts at zero)
  // 10000 = 0.1 ms accuracy.  Don't go higher or cast to (double) 
  // @ end will start to lose precision!
  const DWORD desired_ticks_per_second = 10000;   
  static DWORD quadpart_shift_bits = 0xFFFFFFFF;
  static double freq_after_shift;
  static LARGE_INTEGER orig_time;
  static double extra_time;
  static double last_time;

  if (quadpart_shift_bits == 0xFFFFFFFF) {
    LARGE_INTEGER freq;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&orig_time);

    // if freq is 10 trillion and we only really need 'desired_ticks_per_second',
    // figure out how many bits we can lop off (>>) before doing any casts to (double),
    // to minimize precision losses.
    double instant_div = (double)freq.QuadPart / (double)desired_ticks_per_second;
    quadpart_shift_bits = (DWORD)(log(instant_div) / log(2.0));
    freq_after_shift = (double)freq.QuadPart / (double)pow(2.0, (double)quadpart_shift_bits);

    // for when .QuadPart wraps:
    extra_time = 0;
    last_time = 0;
  }

  /*
  // check to make sure freq isn't squirreling on us...
  LARGE_INTEGER freq_check;
  static LARGE_INTEGER freq;
  static bool bFreqReady = false;
  if (!bFreqReady) {
    QueryPerformanceFrequency(&freq);
    bFreqReady = true;
  }
  QueryPerformanceFrequency(&freq_check);
  assert(freq.LowPart == freq_check.LowPart);
  assert(freq.HighPart == freq_check.HighPart);
  */

  //if (!g_bTimeIsBeingOverridden)
  {
    LARGE_INTEGER time;
    QueryPerformanceCounter(&time);

    // catch wraps:
    if (time.QuadPart < orig_time.QuadPart) {
      OutputDebugStringA("NCommon: timer wrap\n");
      extra_time = last_time;
      orig_time.QuadPart = time.QuadPart;
    }
    
    // very important: NEVER ever cast the absolute .QuadPart straight to a double; 
    //   precision will suck.  (you drop down to 30-ms ticks)
    // you could just case the .LowPart to a double, but then 
    //   you have to watch it the .HighPart for ticks
    //   (which happens every 20 minutes if the freq is 3,579,000),
    //   and adjust accordingly.
    // so we tried subtracting the .QuadPart's as int64's
    //   to get the relative change (since the app began)
    //   (this has no precision loss), then casting to double hoping it would be fine,
    //   even if the app has been running for months...
    //   but it turns out the 'freq' is so damn huge, this can still give
    //   you VERY jump time sampling.
    // so now, we preemptively lop off some # of the bits in the LARGE_INTEGER domain,
    //   and THEN use that technique.  After lopping off those early bits 
    //   the cast to (double) is just fine.
    LONGLONG steps = (time.QuadPart - orig_time.QuadPart) >> quadpart_shift_bits;
    last_time = (double)steps / (double)freq_after_shift + extra_time;
  }
  /*else
  {
    last_time = g_dOverrideTime;
  }*/

  return last_time;
}
