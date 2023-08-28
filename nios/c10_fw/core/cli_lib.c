/*
 * MIT License
 *
 * Copyright (c) 2019 Sean Farrelly
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * File        cli.c
 * Created by  Sean Farrelly
 * Version     1.0
 *
 */

/*! @file cli.c
 * @brief Implementation of command-line interface.
 */
#include "main.h"

#include <stdint.h>
#include <string.h>

static uint8_t new_cmd = 0;
static uint8_t buf[MAX_BUF_SIZE];      /* CLI Rx byte-buffer */
static uint8_t *buf_ptr;               /* Pointer to Rx byte-buffer */

static uint8_t cmd_buf[MAX_BUF_SIZE];  /* CLI command buffer */

static uint8_t cmd_hst[32][MAX_BUF_SIZE] = {0};
static uint8_t head = 0, tail = 0, esc = 0;

const char cli_prompt[] = ">> ";       /* CLI prompt displayed to the user */


/*!
 * @brief This internal API prints a message to the user on the CLI.
 */
static void cli_print(cli_t *cli, const char *msg);

/*!
 * @brief This API initialises the command-line interface.
 */
cli_status_t cli_lib_init(cli_t *cli)
{
    /* Set buffer ptr to beginning of buf */
    buf_ptr = buf;

   cli->looping = 0;
   cli->loop_end = 0;
   cli->loop_snap = 0;
   cli->loop_cnt = 0;
   cli->loop_addr = 0;
   cli->loop_val = 0;
   cli->loop_flags = 0;
   cli->loop_len = 1;
   cli->loop_ms = 0;
   cli->loop_fn = NULL;
   cli->argc = 0;
   for (uint8_t i=0;i<32;i++) {
      cli->argv[i] = NULL;
   }

    /* Print the CLI prompt. */
    cli_print(cli, cli_prompt);

    return CLI_OK;
}

/*!
 * @brief This API deinitialises the command-line interface.
 */
cli_status_t cli_deinit(cli_t *cli)
{
    return CLI_OK;
}


/*! @brief This API must be periodically called by the user to process and execute
 *         any commands received.
 */
cli_status_t cli_process(cli_t *cli) {
    uint8_t i;
    uint8_t argc = 0;
    char *argv[32] = {0};

    if (new_cmd == 1) {
       /* Get the first token (cmd name) */
       argv[argc] = strtok((char *)cmd_buf, " ");

       /* Walk through the other tokens (parameters) */
       while((argv[argc] != NULL) && (argc < 32)) {
           argv[++argc] = strtok(NULL, " ");
       }

       /* Search the command table for a matching command, using argv[0]
        * which is the command name. */
       for(i=0;i<cli->cmd_cnt;i++) {
           if (strcmp(argv[0], cli->cmd_tbl[i].cmd) == 0) {
               cli->cmd_tbl[i].func(argc, argv);
               break;
           }
       }
       cli->argc = argc;
       for (uint8_t i=0;i<argc;i++) cli->argv[i] = argv[i];
       new_cmd = 0;
       cli_print(cli, cli_prompt);
    }
    // command looping end
    else if (cli->loop_end == 1) {
       cli->loop_end = 0;
       cli->looping = 0;
       xlprint("\r                                                                                    \r");
       new_cmd = 0;
       cli_print(cli, cli_prompt);
    }
    // command looping
    else if (cli->looping == 1) {
       if ((alt_timestamp() - cli->loop_snap) > cli->loop_ms) {
          cli->loop_snap = alt_timestamp();
          if (cli->loop_fn != NULL) {
             cli->loop_cnt++;
             cli->loop_fn(cli->argc, cli->argv);
          }
       }
    }

    return CLI_OK;
}

/*!
 * @brief This API should be called from the devices interrupt handler whenever a
 *        character is received over the input stream.
 */
cli_status_t cli_put(cli_t *cli, char c) {
   uint8_t i;

   // check for ending loop
   if (cli->looping == 1) {
      cli->loop_end = 1;
      xlprint("*** Looping Halted ***\n");
   }
   else {
       switch(c) {
       case '\n':
           *buf_ptr = '\0';            /* Terminate the msg and reset the msg ptr.      */
           strcpy((char *)cmd_buf, (char *)buf);       /* Copy string to command buffer for processing. */
           // no blank commands or comments in history
           if (buf_ptr != buf && buf[0] != '#') {
              strcpy((char *)cmd_hst[head], (char *)buf);
              ++head;
              head &= 0x1F;
              tail = (head - 1) & 0x1F;
           }
           buf_ptr = buf;              /* Reset buf_ptr to beginning.                   */
           new_cmd = 1;
           break;

       case '\b':
           /* Backspace. Delete character. */
           if(buf_ptr > buf) {
               buf_ptr--;
              xlprint("\b \b");
           }
           break;

       case '\x5B':
           // escape character, ignore
           esc = 1;
           break;

       case '\x41':
           // up-arrow
           if (esc == 1) {
              xlprint("\r                                                                                    \r");
              xlprint(">> ");
              xlprint("%s", cmd_hst[tail]);
              strcpy((char *)buf, (char *)cmd_hst[tail]);
              buf_ptr = buf + strlen((char *)buf);
              tail--;
              tail &= 0x1F;
              if (cmd_hst[tail][0] == 0x00) {
                 tail++;
                 tail &= 0x1F;
              }
              esc = 0;
           }
           else {
              if((buf_ptr - buf) < MAX_BUF_SIZE)
                  *buf_ptr++ = c;
              else {
                  buf_ptr = buf;
                  return CLI_E_BUF_FULL;
              }
           }
           break;

       case '\x42':
           // down-arrow
           if (esc == 1) {
              i = (tail + 1) & 0x1F;
              if (i != head && cmd_hst[i][0] != 0x00) {
                 xlprint("\r                                                     \r");
                 xlprint(">> ");
                 xlprint("%s", cmd_hst[i]);
                 strcpy((char *)buf, (char *)cmd_hst[i]);
                 buf_ptr = buf + strlen((char *)buf);
                 tail++;
                 tail &= 0x1F;
              }
              else {
                 xlprint("\r                                                     \r");
                 xlprint(">> ");
                 *buf_ptr = '\0';
                 buf_ptr = buf;
              }
              esc = 0;
           }
           else {
              if((buf_ptr - buf) < MAX_BUF_SIZE)
                  *buf_ptr++ = c;
              else {
                  buf_ptr = buf;
                  return CLI_E_BUF_FULL;
              }
           }
           break;

       default:
           /* Normal character received, add to buffer. */
           if((buf_ptr - buf) < MAX_BUF_SIZE)
               *buf_ptr++ = c;
           else {
               buf_ptr = buf;
               return CLI_E_BUF_FULL;
           }
           break;
       }
    }

    return CLI_OK;
}

/*!
 * @brief Print a message on the command-line interface.
 */
static void cli_print(cli_t *cli, const char *msg) {
    char buf[128];
    strcpy(buf, msg);
    cli->println(buf);
}
