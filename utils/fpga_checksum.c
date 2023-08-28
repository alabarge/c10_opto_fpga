#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <limits.h>

int main(int argc, char *argv[]) {

   FILE     *fid;

   char      cwd[128];

   uint32_t  checksum = 0;
   uint8_t   source = 0;
   uint32_t  count = 1;
   uint32_t  i, j = 0;

   printf("\nFPGA 8-Bit Checksum Utility 1.0 [AEL]\n");

   // command Line
   printf("cmd : ");
   for (i=0;i<argc;i++) {
      printf("%s ", argv[i]);
   }
   printf("\n");

   // current directory
   if (getcwd(cwd, sizeof(cwd)) != 0) printf("cwd : %s\n", cwd);

   if (argc < 3) {
      printf("Error: Filenames not specified.\n");
      return -1;
   }

   //
   // Parse the RPD Binary File
   //

   // cycle over all 32-Bit words in binary file and sum
   if ((fid = fopen(argv[1], "rb")) != NULL) {

      while (count != 0) {
         count = fread(&source, sizeof(uint8_t), 1, fid);
         if (count != 0) {
            checksum += source;
            j++;
         }
      }

      fclose(fid);

      printf("%08X, bytes:%d\n", checksum, j);

      if ((fid = fopen(argv[2], "wt")) != NULL) {
         fprintf(fid, "FPGA Checksum = %08x\n", checksum);
         fclose(fid);
      }
      else {
         printf("Fatal Error : Checksum File %s did not Open for Write\n", argv[2]);
         return -1;
      }
   }
   else {
      printf("Fatal Error : RPD File %s did not Open for Read\n", argv[1]);
      return -1;
   }

   return 0;
}
