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
#include <errno.h>
#include <io.h>
#include <string.h>
#include "sys/param.h"
#include "alt_types.h"
#include "sys/alt_debug.h"
#include "sys/alt_cache.h"
#include "altera_remote_update.h"
#include "altera_remote_update_regs.h"

ALT_LLIST_HEAD(altera_remote_update_list);

/*
 * altera_remote_update_init
 * Driver initialization code
 */

int  altera_remote_update_init(altera_remote_update_state* sp)
{
	extern alt_llist altera_remote_update_list;
	alt_dev_llist_insert((alt_dev_llist*) sp, &altera_remote_update_list);
	return 0;
}

/*
 * altera_remote_update_open
 * Return - Pointer to the hardware remote update state
 * The return value will be NULL on failure, and non-NULL otherwise.
 */

altera_remote_update_state* altera_remote_update_open (const char *name)
{
    altera_remote_update_state *sp;
    /* Find requested device */
    sp = (altera_remote_update_state*) alt_find_dev (name, &altera_remote_update_list);
    if (sp == NULL)
        return NULL;
	return sp;
}

/*
 * altera_remote_update_trigger_reconfig
 * The return value will be 0 on success, and non-zero otherwise.
 */

int altera_remote_update_trigger_reconfig (altera_remote_update_state* sp,
	alt_u32 mode, alt_u32 boot_address, alt_u32 watch_dog_timeout)
{
	/* Configure watchdog timer */
	if( watch_dog_timeout > 0 ) {
		IOWR_ALTERA_RU_WATCHDOG_ENABLE(sp->base, ALTERA_RU_WATCHDOG_ENABLE);
		IOWR_ALTERA_RU_WATCHDOG_TIMEOUT(sp->base, watch_dog_timeout & ALTERA_RU_WATCHDOG_TIMEOUT_WRITE_MASK);
	} else {
		IOWR_ALTERA_RU_WATCHDOG_ENABLE(sp->base, ~ALTERA_RU_WATCHDOG_ENABLE);
	}
	/* Configure boot address */
	IOWR_ALTERA_RU_PAGE_SELECT(sp->base, boot_address);
	/* Configure reconfig mode */
	IOWR_ALTERA_RU_CONFIG_MODE(sp->base, mode & ALTERA_RU_RECONFIG_MODE_MASK);
	/* Trigger reconfig */
	IOWR_ALTERA_RU_RECONFIG(sp->base, 0x1);
	return 0;
}
