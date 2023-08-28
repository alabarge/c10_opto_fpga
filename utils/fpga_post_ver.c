#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>
#include <sys/types.h>

// Windows does not define the S_ISREG and S_ISDIR macros in stat.h, so we do.
// We have to define _CRT_INTERNAL_NONSTDC_NAMES 1 before #including sys/stat.h
// in order for Microsoft's stat.h to define names like S_IFMT, S_IFREG, and S_IFDIR,
// rather than just defining  _S_IFMT, _S_IFREG, and _S_IFDIR as it normally does.
#define _CRT_INTERNAL_NONSTDC_NAMES 1
#include <sys/stat.h>
#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
  #define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif
#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
  #define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#endif

int main(int argc, char *argv[]) {

   FILE     *file;

   struct tm *now;
   time_t clock;

   uint32_t j;

   uint32_t checksum    = 0;
   uint32_t crc32       = 0;
   char     md5[128]    = {0};

   uint32_t build_pid   = 0;
   uint32_t build_map   = 0;
   uint32_t build_logic = 0;
   uint32_t build_map_date = 0;

   char  build_user[128]   = {0};
   char  build_time[128]   =  {0};
   char  build_date[128]   = {0};

   char  in_line[256], file_name[256];

   char  *env, *token;

   char  cwd[128];

   char  *month[] = {
            "JAN", "FEB", "MAR", "APR",
            "MAY", "JUN", "JUL", "AUG",
            "SEP", "OCT", "NOV", "DEC"
          };

   printf("\nCYC1000 fpga_version.h File Creation 1.3, Post Synthesis\n");

   // command Line
   printf("cmd : ");
   for (j=0;j<argc;j++) {
      printf("%s ", argv[j]);
   }
   printf("\n");

   // current directory
   if (getcwd(cwd, sizeof(cwd)) != 0) printf("cwd : %s\n", cwd);

   if (argc < 5) {
      printf("Error: Filenames not specified.\n");
      printf("usage : fpga_c10_ver c10_fpga_checksum.txt c10_fpga_crc32.txt c10_fpga_md5.txt fpga_build.inc fpga_version.h\n");
      return -1;
   }

   //
   // Parse checksum file
   //
   if  ((file = fopen(argv[1],"rt")) != NULL) {
      while (fgets(in_line, sizeof(in_line), file) != NULL) {
         token = strtok(in_line, "=");
         token = strtok(NULL, "\n");
         if (token == NULL) continue;
         sscanf(token, "%x", &checksum);
         break;
      }
      fclose(file);
   }
   printf("checksum    = %08X\n", checksum);

   //
   // Parse crc32 file
   //
   if  ((file = fopen(argv[2],"rt")) != NULL) {
      while (fgets(in_line, sizeof(in_line), file) != NULL) {
         token = strtok(in_line, "=");
         token = strtok(NULL, "\n");
         if (token == NULL) continue;
         sscanf(token, "%x", &crc32);
         break;
      }
      fclose(file);
   }
   printf("crc32       = %08X\n", crc32);

   //
   // Parse md5 file
   //
   if  ((file = fopen(argv[3],"rt")) != NULL) {
      // hash is on second line
      fgets(in_line, sizeof(in_line), file);
      fgets(in_line, sizeof(in_line), file);
      if (strlen(in_line) != 0)
         sscanf(in_line, "%s", md5);
      fclose(file);
   }
   printf("md5         = %s\n", md5);

   //
   // Parse fpga_build.inc file
   //
   if  ((file = fopen(argv[4],"rt")) != NULL) {
      while (fgets(in_line, sizeof(in_line), file) != NULL) {
         token = strtok(in_line, " ");
         token = strtok(NULL, " ");
         if (token == NULL) continue;
         if (strcmp(token, "FPGA_PID") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &build_pid);
            continue;
         }
         else if (strcmp(token, "FPGA_MAP") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &build_map);
            continue;
         }
         else if (strcmp(token, "FPGA_LOGIC") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &build_logic);
            continue;
         }
         else if (strcmp(token, "FPGA_MAP_DATE") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%x", &build_map_date);
            continue;
         }
      }
      fclose(file);
   }
   printf("status      = %02X%03X%03X\n", build_pid, build_map, build_logic);
   printf("status_date = %08X\n", build_map_date);

   // Get Current Time & Date
   time(&clock);
   now = localtime(&clock);
   sprintf(build_time,"%02d:%02d", now->tm_hour, now->tm_min);
   sprintf(build_date,"%02d.%s.%02d", now->tm_mday, month[now->tm_mon],
         (now->tm_year+1900)-2000);

   // Get Current User
   memset(build_user, 0, sizeof(build_user));
   env = getenv("USERNAME");
   if (env != NULL) {
      strcpy(build_user, env);
   }
   else {
      build_user[0] = '\0';
   }

   //
   // Open fpga_version.h file
   //
   if  ((file = fopen(argv[5],"wt")) != NULL) {
      fprintf(file, "//----------------------------------------------------------------------------\n");
      fprintf(file, "//      Name:    fpga_version.h\n");
      fprintf(file, "//      Purpose: Main Build Version for the CYC1000 board.\n");
      fprintf(file, "//\n");
      fprintf(file, "//      Auto Generated from Quartus II 64-Bit Version 22.1std.1 Build 917\n");
      fprintf(file, "//\n");
      fprintf(file, "//      %s %s - %s\n", build_time, build_date, build_user);
      fprintf(file, "//----------------------------------------------------------------------------\n");
      fprintf(file, "\n");
      fprintf(file, "#pragma once\n");
      fprintf(file, "\n");
      fprintf(file, "#define FPGA_PRESENT_ID     %d\n", build_pid);
      fprintf(file, "#define FPGA_MAP_VER        %d\n", build_map);
      fprintf(file, "#define FPGA_LOGIC_VER      %d\n", build_logic);
      fprintf(file, "#define FPGA_CHECKSUM       0x%08X\n", checksum);
      fprintf(file, "#define FPGA_CRC            0x%08X\n", crc32);
      fprintf(file, "#define FPGA_MD5            %s\n", md5);
      fprintf(file, "\n");
      fclose(file);
   }

   //
   // Create SOF File for Production, c10_fpga_vpresent_id.map_ver.logic_ver.checksum.crc32
   //
   sprintf(file_name, "%02X.%d.%d.%08X.%08X.sof", build_pid, build_map, build_logic, checksum, crc32);
   sprintf(in_line, "echo f|xcopy .\\output_files\\c10_fpga.sof .\\output_files\\c10_fpga_v%s /F /Y /R", file_name);
   printf("%s\n", in_line);

   system(in_line);

   return 0;
}
