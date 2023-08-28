#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

#define DIM(x)    (sizeof(x)/sizeof(*(x)))

#define  CI_BIN_SIZE    512

#define  CI_MAX_KEY     32

#define  CI_INT         0
#define  CI_UINT        1
#define  CI_HEX         6
#define  CI_DBL         7
#define  CI_STR         9

//
// CI TABLE
//
typedef struct _ci_entry_t {
   char        key[CI_MAX_KEY];
   char        value[CI_MAX_KEY];
   uint8_t     type;
   void       *parm;
   uint8_t     dim;
} ci_entry_t, *pci_entry_t;

//
// copied from ci.h
//
//
// CONFIGURABLE ITEMS
//
typedef struct _ci_t {
   uint32_t    rev;                    // CI Revision
   uint32_t    check;                  // CI Boot Checksum
   uint32_t    checksum;               // CI Checksum
   uint32_t    magic;                  // Magic Number
   uint32_t    debug;                  // Software Debug
   uint32_t    trace;                  // Software Trace
   uint32_t    feature;                // Run-Time Features
   uint32_t    mac_addr_hi;            // MAC Address HI
   uint32_t    mac_addr_lo;            // MAC Address LO
   uint32_t    ip_addr;                // IP Address
   uint32_t    cm_udp_port;            // CM UDP Port
   uint32_t    pad[117];               // Sector Pad
} ci_t, *pci_t;

// CI data structure
static ci_t ci = {0};

//
// copied from ci.c
//
static ci_entry_t ci_table[] = {
   //
   // Variable Text Name      Default Value           Data Type      Address to Variable        Dim
   // ==================      =============           =========      ===================        =====
   //
   { "ci.rev",                "0x00000000",           CI_HEX,        &ci.rev,                   1 },
   { "ci.check",              "0x00000000",           CI_HEX,        &ci.check,                 1 },
   { "ci.checksum",           "0x00000000",           CI_HEX,        &ci.checksum,              1 },
   { "ci.magic",              "0x55AA1234",           CI_HEX,        &ci.magic,                 1 },
   { "ci.debug",              "0x00000000",           CI_HEX,        &ci.debug,                 1 },
   { "ci.trace",              "0x00000A82",           CI_HEX,        &ci.trace,                 1 },
   { "ci.feature",            "0x00000003",           CI_HEX,        &ci.feature,               1 },
   { "ci.mac_addr_hi",        "0x0002C94E",           CI_HEX,        &ci.mac_addr_hi,           1 },
   { "ci.mac_addr_lo",        "0x7FC80000",           CI_HEX,        &ci.mac_addr_lo,           1 },
   { "ci.ip_addr",            "0xC0A80146",           CI_HEX,        &ci.ip_addr,               1 },
   { "ci.cm_udp_port",        "0x00000ADD",           CI_HEX,        &ci.cm_udp_port,           1 },
};

int main(int argc, char *argv[]) {

   FILE     *fid;
   char      line[512];
   char     *token;
   char      cwd[128];
   uint8_t   bin[CI_BIN_SIZE] = {0};

   uint32_t  checksum = 0;
   uint32_t  i,j;

   uint32_t   *pci = (uint32_t*)&ci;

   printf("\nCI Checksum Utility 1.0 [AEL]\n");

   // command Line
   printf("cmd : ");
   for (j=0;j<argc;j++) {
      printf("%s ", argv[j]);
   }
   printf("\n");

   // current directory
   if (getcwd(cwd, sizeof(cwd)) != 0) printf("cwd : %s\n", cwd);

   if (argc < 3) {
      printf("Error: Filenames not specified.\n");
      return -1;
   }

   //
   // Parse the CI Text File
   //

   // cycle over CI file and parse parameters
   if ((fid = fopen(argv[1], "rt")) != NULL) {
      // read next line
      while (fgets(line, sizeof(line), fid) != NULL) {
         // ignore comments
         if (line[0] == '#') continue;
         // ignore blank lines
         if (line[0] == '\n') continue;
         // ignore blank lines
         if (line[0] == '\r') continue;
         // ignore lines that begin with a space
         if (line[0] == ' ') continue;
         // extract key
         token = strtok(line, " ;\t=\n\r");
         // end-of-file
         if (strcmp(token, "@EOF") == 0) break;
         // cycle over table
         for (i=0;i<DIM(ci_table);i++) {
            if (strcmp(token, ci_table[i].key) == 0) {
               token = strtok(NULL, " ;\t=\n\r");
               switch(ci_table[i].type) {
                  case CI_INT :
                     sscanf(token, "%ld", (int32_t *)ci_table[i].parm);
                     break;
                  case CI_UINT :
                     sscanf(token, "%ld", (uint32_t *)ci_table[i].parm);
                     break;
                  case CI_HEX :
                     sscanf(token, "%lx", (uint32_t *)ci_table[i].parm);
                     break;
                  case CI_DBL :
                     sscanf(token, "%lf", (double *)ci_table[i].parm);
                     break;
                  case CI_STR :
                     sscanf(token, "%s", (char *)ci_table[i].parm);
                     break;
               }
               break;
            }
         }
         if (i == DIM(ci_table))
            printf("ci_parse() Warning : Unknown Parameter %s\n", token);
      }
      fclose(fid);

      // Calculate CI Checksum
      ci.checksum = 0;
      checksum = 0;
      for (i=0;i<sizeof(ci_t)>>2;i++) {
         checksum += pci[i];
      }
      ci.checksum = checksum;

      // Write CI Parameters, TEXT
      if ((fid = fopen(argv[2], "wt")) != NULL) {
         for (i=0;i<DIM(ci_table);i++) {
            switch(ci_table[i].type) {
               case CI_INT :
                  fprintf(fid, "%s = %d;\n", ci_table[i].key, *(int32_t *)ci_table[i].parm);
                  break;
               case CI_UINT :
                  fprintf(fid, "%s = %d;\n", ci_table[i].key, *(uint32_t *)ci_table[i].parm);
                  break;
               case CI_HEX :
                  fprintf(fid, "%s = 0x%08X;\n", ci_table[i].key, *(uint32_t *)ci_table[i].parm);
                  break;
               case CI_DBL :
                  fprintf(fid, "%s = %lf;\n", ci_table[i].key, *(double *)ci_table[i].parm);
                  break;
               case CI_STR :
                  fprintf(fid, "%s = %s;\n", ci_table[i].key, (char *)ci_table[i].parm);
                  break;
            }
         }
         fprintf(fid, "\n");
         fclose(fid);
      }
      else {
         printf("Fatal Error : CI File %s did not Open for Write\n", argv[2]);
         return -1;
      }

      // Write CI Parameters, BIN
      if ((fid = fopen(argv[3], "wb")) != NULL) {
         for (i=0,j=0;i<DIM(ci_table);i++) {
            switch(ci_table[i].type) {
               case CI_INT :
                  fwrite((int32_t *)ci_table[i].parm, sizeof(int32_t), 1, fid);
                  j += sizeof(int32_t);
                  break;
               case CI_UINT :
                  fwrite((uint32_t *)ci_table[i].parm, sizeof(uint32_t), 1, fid);
                  j += sizeof(uint32_t);
                  break;
               case CI_HEX :
                  fwrite((uint32_t *)ci_table[i].parm, sizeof(uint32_t), 1, fid);
                  j += sizeof(uint32_t);
                  break;
               case CI_DBL :
                  fwrite((double *)ci_table[i].parm, sizeof(double), 1, fid);
                  j += sizeof(uint32_t);
                  break;
               case CI_STR :
                  fwrite((char *)ci_table[i].parm, strlen(ci_table[i].parm), 1, fid);
                  j += strlen(ci_table[i].parm);
                  break;
            }
         }
         fwrite((uint8_t *)bin, CI_BIN_SIZE - j, 1, fid);
         fclose(fid);
      }
      else {
         printf("Fatal Error : CI File %s did not Open for Write\n", argv[3]);
         return -1;
      }
   }
   else {
      printf("Fatal Error : CI File %s did not Open for Read\n", argv[1]);
      return -1;
   }

   return 0;
}
