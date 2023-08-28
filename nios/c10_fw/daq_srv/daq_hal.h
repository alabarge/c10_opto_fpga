#pragma once

#include "daq_srv.h"

#define  DAQH_INT_TICK         0x00
#define  DAQH_INT_DONE         0x01
#define  DAQH_INT_ALL          0x02

#define  DAQH_OK               0x0000
#define  DAQH_ERROR            0x0001

#define  DAQH_STATE_IDLE       0x00
#define  DAQH_STATE_RUN        0x01
#define  DAQH_STATE_SCAN       0x02

uint32_t daq_hal_init(void);
void     daq_hal_isr(void *arg);
void     daq_hal_intack(uint8_t int_type);
void     daq_hal_run(pdaq_sv_t sv);
