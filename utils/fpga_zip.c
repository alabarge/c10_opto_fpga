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

   struct tm *now;
   time_t clock;

   char  in_line[256], file_name[256];

   printf("\nZip DE0 Rev.F Release 1.0\n");

   // Get Current Time & Date
   time(&clock);
   now = localtime(&clock);

   //
   // Create ZIP File for Production
   //
   sprintf(file_name, "..\\c10_fpga_release_%04d%02d%02d_%02d%02d.zip", now->tm_year+1900,
      now->tm_mon+1, now->tm_mday, now->tm_hour, now->tm_min);
   sprintf(in_line, "\"C:\\Program Files\\7-Zip\\7z.exe\" a -tzip %s *", file_name);
   printf("%s\n", in_line);
   system(in_line);

   return 0;
}
