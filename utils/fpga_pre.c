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

   FILE  *ver, *git, *qsf;

   struct tm *now;
   time_t clock;

   uint32_t build_inc   = 0;
   uint32_t build_hex   = 0;
   uint32_t error       = 0;

   uint32_t build_pid   = 0;
   uint32_t build_map   = 0;
   uint32_t build_logic = 0;
   uint32_t build_map_date = 0;

   uint32_t build_inc_found = 0;
   uint32_t j, ret;

   char  build_user[128]   = {0};
   char  build_time[128]   =  {0};
   char  build_date[128]   = {0};
   char  build_hi[128]     = {0};
   char  build_lo[128]     = {0};
   char  build_str1[128]   = {0};
   char  build_str2[128]   = {0};
   char  in_line[1024]     = {0};
   char  git_rev[128]      = {0};
   char  git_auth[128]     = {0};
   char  git_email[128]    = {0};
   char  git_date[128]     = {0};
   char  last_quartus[128] = {0};
   char  creation_date[128] = {0};

   char  *env, *token;

   char  cwd[128];

   char  *month[] = {
            "JAN", "FEB", "MAR", "APR",
            "MAY", "JUN", "JUL", "AUG",
            "SEP", "OCT", "NOV", "DEC"
          };

   printf("\nCYC1000 fpga_build.vhd & fpga_build.h File Creation 1.4, Pre-Synthesis\n");

   // command Line
   printf("cmd : ");
   for (j=0;j<argc;j++) {
      printf("%s ", argv[j]);
   }
   printf("\n");

   // current directory
   if (getcwd(cwd, sizeof(cwd)) != 0) printf("cwd : %s\n", cwd);

   if (argc < 6) {
      printf("Error: Filenames not specified.\n");
      return -1;
   }

   //
   // If fpga_build.h exists then parse for FPGA_INC and increment
   //
   if  ((ver = fopen(argv[2],"rt")) != NULL) {
      while (fgets(in_line, sizeof(in_line), ver) != NULL) {
         token = strtok(in_line, " ");
         token = strtok(NULL, " ");
         if (token == NULL) continue;
         if (strcmp(token, "FPGA_INC") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &build_inc);
            build_inc &= 0x00FF;
            build_inc_found = 1;
            break;
         }
      }
      fclose(ver);
   }

   //
   // Parse fpga_build.inc file
   //
   if  ((ver = fopen(argv[1],"rt")) != NULL) {
      while (fgets(in_line, sizeof(in_line), ver) != NULL) {
         token = strtok(in_line, " ");
         token = strtok(NULL, " ");
         if (token == NULL) continue;
         if (strcmp(token, "BUILD_INC") == 0) {
            token = strtok(NULL, "\n");
            // If build_inc not found in fpga_build.h
            if (build_inc_found == 0) {
               sscanf(token, "%d", &build_inc);
               build_inc &= 0x00FF;
            }
            continue;
         }
         else if (strcmp(token, "BUILD_PID") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &build_pid);
               build_pid &= 0x00FF;
            continue;
         }
         else if (strcmp(token, "BUILD_MAP") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &build_map);
               build_map &= 0x0FFF;
            continue;
         }
         else if (strcmp(token, "BUILD_LOGIC") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &build_logic);
               build_logic &= 0x0FFF;
            continue;
         }
         else if (strcmp(token, "BUILD_MAP_DATE") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%x", &build_map_date);
            continue;
         }
      }
      fclose(ver);
   }
   printf("status      = %02X%03X%03X\n", build_pid, build_map, build_logic);
   printf("status_date = %08X\n", build_map_date);

   if (error == 1) {
      fclose(ver);
      printf("Error Parsing %s, %d\n", argv[1], error);
      return -1;
   }

   //
   // If the QSF exists then parse for LAST_QUARTUS_VERSION and PROJECT_CREATION_TIME_DATE
   //
   if  ((qsf = fopen(argv[5],"rt")) != NULL) {
      while (fgets(in_line, sizeof(in_line), qsf) != NULL) {
         token = strtok(in_line, " ");
         token = strtok(NULL, " ");
         token = strtok(NULL, " ");
         if (token == NULL) continue;
         if (strcmp(token, "LAST_QUARTUS_VERSION") == 0) {
            token = strtok(NULL, "\"");
            strcpy(last_quartus, token);
         }
         if (strcmp(token, "PROJECT_CREATION_TIME_DATE") == 0) {
            token = strtok(NULL, "\"");
            strcpy(creation_date, token);
            break;
         }
      }
      fclose(qsf);
   }


   // Increment Build Number, only 0 to 255
   build_inc++;
   build_inc &= 0x00FF;

   // 32-Bit Build Number
   build_hex = (build_pid << 24) | (build_map << 12) | build_logic;

   // Open File for Writing
   if  ((ver = fopen(argv[2],"wt")) == NULL) {
      printf("Error: Unable to open %s for writing\n", argv[2]);
      exit (-1);
   }

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

   remove("git_hash.txt");
   remove("git_porcelain.txt");

   // Check for .git existence
   struct stat status;
   stat(argv[4], &status);

   // If .git directory then gather state info
   if (S_ISDIR(status.st_mode)) {

      ret = system("git show --quiet HEAD > git_hash.txt");
      ret = system("git status --porcelain > git_porcelain.txt");

      // Get git HEAD detail
      //status --porcelain
      // Parse git_hash.txt
      //
      if ((git = fopen("git_hash.txt","rt")) != NULL) {
         while (fgets(in_line, sizeof(in_line), git) != NULL) {
            // ignore comments
            if (in_line[0] == '#') continue;
            // ignore blank lines
            if (in_line[0] == '\n') continue;
            // ignore blank lines
            if (in_line[0] == '\r') continue;
            // ignore lines that begin with a space
            if (in_line[0] == ' ') continue;
            // extract key
            token = strtok(in_line, " ");
            // commit
            if (strcmp(token, "commit") == 0) {
               token = strtok(NULL, " ");
               sprintf(git_rev, "%s", token);
               // example sha-1 hash - 71a42555d9d0c2ca21d5d48b653ef538eeac36fd
               // first seven characters of hash
               memcpy(&git_rev[0], &git_rev[0], 7);
               // ellipses ...
               git_rev[7] = '.';
               git_rev[8] = '.';
               git_rev[9] = '.';
               // last seven characters of hash
               memcpy(&git_rev[10], &git_rev[33], 7);
               git_rev[17] = 0x00;
            }
            // Merge
            if (strcmp(token, "Merge:") == 0) {
               token = strtok(NULL, " ");
            }
            // Author
            if (strcmp(token, "Author:") == 0) {
               token = strtok(NULL, "<");
               sprintf(git_auth, "%s", token);
               token = strtok(NULL, ">");
               sprintf(git_email, "%s", token);
            }
            // Date
            if (strcmp(token, "Date:") == 0) {
               token = strtok(NULL, "\n");
               sprintf(git_date, "%s", token);
            }
         }
         fclose(git);
         remove("git_hash.txt");
      }

      // Get git status detail
      //
      // Parse git_porcelain.txt
      //
      if  ((git = fopen("git_porcelain.txt","rt")) != NULL) {
         while (fgets(in_line, sizeof(in_line), git) != NULL) {
            token = strtok(in_line, " ");
            if ((strcmp(token, "M") == 0) ||
                (strcmp(token, "A") == 0) ||
                (strcmp(token, "D") == 0) ||
                (strcmp(token, "R") == 0) ||
                (strcmp(token, "C") == 0)) {
               strcat(git_rev, "*");
               break;
            }
         }
         fclose(git);
         remove("git_porcelain.txt");
      }
   }
   else {
      printf("Warning : The %s Directory was not found\n", argv[4]);
   }

   // Create Build Strings
   sprintf(build_lo, "%d.%d.%d build %d", build_pid, build_map, build_logic, build_inc);
   sprintf(build_hi, "%s, %s %s [%s] %s %s", build_lo, build_time, build_date,
      build_user, git_rev, last_quartus);
   sprintf(build_str1, "%s (%04d/%02d/%02d)", "", now->tm_year+1900, now->tm_mon+1, now->tm_mday);
   sprintf(build_str2, "%d.%d.%d.%d", build_pid, build_map, build_logic, build_inc);

   fprintf(ver, "#pragma once\n\n");

   fprintf(ver, "#define FPGA_PID            %d\n",     build_pid);
   fprintf(ver, "#define FPGA_MAP            %d\n",     build_map);
   fprintf(ver, "#define FPGA_LOGIC          %d\n",     build_logic);
   fprintf(ver, "#define FPGA_INC            %d\n",     build_inc);
   fprintf(ver, "#define FPGA_MAP_DATE       0x%08X\n", build_map_date);
   fprintf(ver, "#define FPGA_VER_HEX        0x%08X\n", build_hex);
   fprintf(ver, "#define FPGA_TIME           \"%s\"\n", build_time);
   fprintf(ver, "#define FPGA_DATE           \"%s\"\n", build_date);
   fprintf(ver, "#define FPGA_USER           \"%s\"\n", build_user);
   fprintf(ver, "#define FPGA_STR            \"%s\"\n", build_str2);
   fprintf(ver, "#define FPGA_LO             \"%s\"\n", build_lo);
   fprintf(ver, "#define FPGA_HI             \"%s\"\n", build_hi);
   fprintf(ver, "#define FPGA_STRING         \"%s\"\n", build_str2);
   fprintf(ver, "#define FPGA_EPOCH          %d\n", (uint32_t)clock);
   fprintf(ver, "#define FPGA_EPOCH_HEX      0x%08X\n", (uint32_t)clock);
   fprintf(ver, "#define FPGA_DATE_HEX       0x%04d%02d%02d\n", now->tm_year+1900, now->tm_mon+1, now->tm_mday);
   fprintf(ver, "#define FPGA_TIME_HEX       0x00%02d%02d%02d\n", now->tm_hour, now->tm_min, now->tm_sec);
   fprintf(ver, "#define FPGA_GIT_REV        \"%s\"\n", git_rev);
   fprintf(ver, "#define FPGA_GIT_AUTH       \"%s\"\n", git_auth);
   fprintf(ver, "#define FPGA_GIT_EMAIL      \"%s\"\n", git_email);
   fprintf(ver, "#define FPGA_GIT_DATE       \"%s\"\n", git_date);
   fprintf(ver, "#define FPGA_LAST_QUARTUS   \"%s\"\n", last_quartus);
   fprintf(ver, "#define FPGA_CREATION_DATE  \"%s\"\n", creation_date);

   fclose(ver);
   fflush(ver);

   //
   // Create FPGA VHDL Version File
   //
   // Open File for Writing
   if  ((ver = fopen(argv[3],"wt")) == NULL) {
      printf("Error: Unable to open %s for writing\n", argv[3]);
      exit (-1);
   }
   fprintf(ver, "library ieee;\n");
   fprintf(ver, "use ieee.std_logic_1164.all;\n");
   fprintf(ver, "\n");
   fprintf(ver, "package fpga_ver is\n");
   fprintf(ver, "\n");
   fprintf(ver, "   constant C_BUILD_PID           : std_logic_vector(7 downto 0)   := X\"%02X\";\n", build_pid);
   fprintf(ver, "   constant C_BUILD_MAP           : std_logic_vector(11 downto 0)  := X\"%03X\";\n", build_map);
   fprintf(ver, "   constant C_BUILD_LOGIC         : std_logic_vector(11 downto 0)  := X\"%03X\";\n", build_logic);
   fprintf(ver, "   constant C_BUILD_INC           : std_logic_vector(7 downto 0)   := X\"%02X\";\n", build_inc);
   fprintf(ver, "   constant C_BUILD_MAP_DATE      : std_logic_vector(31 downto 0)  := X\"%08X\";\n", build_map_date);
   fprintf(ver, "   constant C_BUILD_VER_HEX       : std_logic_vector(31 downto 0)  := X\"%08X\";\n", build_hex);
   fprintf(ver, "   constant C_BUILD_TIME          : string                         := \"%s\";\n", build_time);
   fprintf(ver, "   constant C_BUILD_DATE          : string                         := \"%s\";\n", build_date);
   fprintf(ver, "   constant C_BUILD_USER          : string                         := \"%s\";\n", build_user);
   fprintf(ver, "   constant C_BUILD_STR           : string                         := \"%s\";\n", build_str2);
   fprintf(ver, "   constant C_BUILD_LO            : string                         := \"%s\";\n", build_lo);
   fprintf(ver, "   constant C_BUILD_HI            : string                         := \"%s\";\n", build_hi);
   fprintf(ver, "   constant C_BUILD_STRING        : string                         := \"%s\";\n", build_str1);
   fprintf(ver, "   constant C_BUILD_EPOCH         : integer                        := %d;\n", (uint32_t)clock);
   fprintf(ver, "   constant C_BUILD_EPOCH_HEX     : std_logic_vector(31 downto 0)  := X\"%08X\";\n", (uint32_t)clock);
   fprintf(ver, "   constant C_BUILD_DATE_HEX      : std_logic_vector(31 downto 0)  := X\"%04d%02d%02d\";\n", now->tm_year+1900, now->tm_mon+1, now->tm_mday);
   fprintf(ver, "   constant C_BUILD_TIME_HEX      : std_logic_vector(31 downto 0)  := X\"00%02d%02d%02d\";\n", now->tm_hour, now->tm_min, now->tm_sec);
   fprintf(ver, "   constant C_BUILD_GIT_REV       : string                         := \"%s\";\n", git_rev);
   fprintf(ver, "   constant C_BUILD_GIT_AUTH      : string                         := \"%s\";\n", git_auth);
   fprintf(ver, "   constant C_BUILD_GIT_EMAIL     : string                         := \"%s\";\n", git_email);
   fprintf(ver, "   constant C_BUILD_GIT_DATE      : string                         := \"%s\";\n", git_date);
   fprintf(ver, "   constant C_BUILD_LAST_QUARTUS  : string                         := \"%s\";\n", last_quartus);
   fprintf(ver, "   constant C_BUILD_CREATION_DATE : string                         := \"%s\";\n", creation_date);
   fprintf(ver, "\n");
   fprintf(ver, "end fpga_ver;\n");

   fclose(ver);
   fflush(ver);

   //
   // Open fpga_build.h and read BUILD_HI, sanity check
   //
   if  ((ver = fopen(argv[2],"rt")) != NULL) {
      while (fgets(in_line, sizeof(in_line), ver) != NULL) {
         token = strtok(in_line, " ");
         token = strtok(NULL, " ");
         if (token == NULL) continue;
         if (strcmp(token, "BUILD_HI") == 0) {
            token = strtok(NULL, "\n");
            break;
         }
      }
      fclose(ver);
   }

   printf("BUILD_HI : %s\n\n", build_hi);

   return 0;
}
