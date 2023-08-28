/******************************************************************************
*                                                                             *
* License Agreement                                                           *
*                                                                             *
* Copyright (c) 2015 Altera Corporation, San Jose, California, USA.           *
* All rights reserved.                                                        *
*                                                                             *
* Permission is hereby granted, free of charge, to any person obtaining a     *
* copy of this software and associated documentation files (the "Software"),  *
* to deal in the Software without restriction, including without limitation   *
* the rights to use, copy, modify, merge, publish, distribute, sublicense,    *
* and/or sell copies of the Software, and to permit persons to whom the       *
* Software is furnished to do so, subject to the following conditions:        *
*                                                                             *
* The above copyright notice and this permission notice shall be included in  *
* all copies or substantial portions of the Software.                         *
*                                                                             *
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR  *
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,    *
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE *
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER      *
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING     *
* FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER         *
* DEALINGS IN THE SOFTWARE.                                                   *
*                                                                             *
* This agreement shall be governed in all respects by the laws of the State   *
* of California and by the laws of the United States of America.              *
*                                                                             *
******************************************************************************/

#ifndef __ALT_REMOTE_UPDATE_H__
#define __ALT_REMOTE_UPDATE_H__

#include "alt_types.h"
#include "sys/alt_llist.h"
#include "priv/alt_dev_llist.h"
#include "sys/alt_dev.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/*
 * The function alt_find_dev() is used to search the device list "list" to
 * locate a device named "name". If a match is found, then a pointer to the
 * device is returned, otherwise NULL is returned.
 */
extern alt_dev* alt_find_dev (const char* name, alt_llist* list);

typedef enum reconfig_mode     
{
	FACTORY = 0,
	APPLICATION
} mode;

/**
 *  Description of the Remote Update
 */
typedef struct altera_remote_update_s
{
	alt_dev				dev;
	unsigned int				base;	/* The base address of the device */

} altera_remote_update_state;

/**
*   Macros used by alt_sys_init.c to create data storage for driver instance
*/
#define ALTERA_REMOTE_UPDATE_INSTANCE(name, state) \
static altera_remote_update_state state = \
{{									\
	ALT_LLIST_ENTRY,					\
	name##_NAME,						\
	NULL,			\
	NULL,								\
	NULL,								\
	NULL,								\
	NULL,								\
	NULL,								\
	NULL,								\
	}, 									\
	name##_BASE,						\
}

/*
    Public API
*/
altera_remote_update_state* altera_remote_update_open (const char *name);
int altera_remote_update_trigger_reconfig(
			altera_remote_update_state* sp,
			alt_u32 mode,
			alt_u32 boot_address_value,
			alt_u32 watch_dog_value
		);

/*
 * Initialization function
 */
extern int altera_remote_update_init(altera_remote_update_state* sp);

/*
 * alt_sys_init.c will call this macro automatically initialize the driver instance
 */
#define ALTERA_REMOTE_UPDATE_INIT(name, state) 	\
		altera_remote_update_init(&state);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __ALT_REMOTE_UPDATE_H__ */
