#pragma once

uint32_t cp_hal_init(void);
void     cp_hal_isr(void);
void     cp_hal_intack(uint8_t int_type);
