#pragma once

// POST Return Codes
#define POST_OK         0x00000000
#define POST_DDR        0x00000001
#define POST_ESN        0x00000002
#define POST_INT        0x00000004
#define POST_SD         0x00000008
#define POST_FAULT      0x80000000

uint32_t   post_all(void);
uint32_t   post_ddr(void);
uint32_t   post_sd(void);
uint32_t   post_esn(void);
uint32_t   post_int(void);
