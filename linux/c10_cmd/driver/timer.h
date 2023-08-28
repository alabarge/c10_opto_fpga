#pragma once

#include <stdlib.h>

typedef enum {
   TIMER_SINGLE_SHOT = 0,
   TIMER_PERIODIC
} t_timer;

typedef void (*time_handler)(size_t timer_id, void * user_data);

int    timer_init(void);
size_t timer_start(unsigned int interval, time_handler handler, t_timer type, void *user_data);
void   timer_stop(size_t timer_id);
void   timer_final();
