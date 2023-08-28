#include <windows.h>

static LARGE_INTEGER m_liStart = {0};
static LARGE_INTEGER m_liStop = {0};

// Starts the timer
void tmr_start()
{
    QueryPerformanceCounter(&m_liStart);
}

// Stops the timer
void tmr_stop()
{
    QueryPerformanceCounter(&m_liStop);
}

// Returns the counter at the last Start()
LONGLONG tmr_get_start_counter()
{
    return m_liStart.QuadPart;
}

// Returns the counter at the last Stop()
LONGLONG tmr_get_stop_counter()
{
    return m_liStop.QuadPart;
}

// Returns the interval between the last Start() and Stop()
LONGLONG tmr_get_elapsed()
{
    return (m_liStop.QuadPart - m_liStart.QuadPart);
}

// Returns the interval between the last Start() and Stop() in seconds
double tmr_get_elapsed_seconds()
{
    LARGE_INTEGER liFrequency;
    QueryPerformanceFrequency(&liFrequency);
    return ((double)tmr_get_elapsed() / (double)liFrequency.QuadPart);
}

// Returns the interval between the last Start() and Stop() in microseconds
double tmr_get_elapsed_microseconds()
{
    LARGE_INTEGER liFrequency;
    QueryPerformanceFrequency(&liFrequency);
    QueryPerformanceCounter(&m_liStop);
    return ((double)(tmr_get_elapsed() * 1000000) / (double)liFrequency.QuadPart);
}