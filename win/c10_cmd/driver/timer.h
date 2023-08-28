#pragma once

#include <windows.h>

void tmr_start();
void tmr_stop();
LONGLONG tmr_get_start_counter();
LONGLONG tmr_get_stop_counter();
LONGLONG tmr_get_elapsed();
double tmr_get_elapsed_seconds();
double tmr_get_elapsed_microseconds();
