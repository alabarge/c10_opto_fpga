#include <SDKDDKVer.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <string.h>
#include    <ctype.h>
#include    <time.h>

int main(int argc, char *argv[]) {

   FILE  *pVer;

   struct tm *newtime;
   time_t aclock;

   unsigned int   buildMajor = 0;
   unsigned int   buildMinor = 0;
   unsigned int   buildInc   = 0;
   unsigned int   buildNum   = 0;
   unsigned int   buildHex   = 0;
   unsigned int   bError     = 0;

   unsigned int   buildIncFound = 0;

   char  buildUser[60];
   char  buildTime[30];
   char  buildDate[30];
   char  buildHi[60];
   char  buildLo[60];
   char  buildStr[60];
   char  inLine[1024];
   char  *pEnv, *token;

   char  *month[] = {
            "JAN", "FEB", "MAR", "APR",
            "MAY", "JUN", "JUL", "AUG",
            "SEP", "OCT", "NOV", "DEC"
          };

   printf("\nVersion Utility 1.8 [AEL]\n");

   if (argc < 3) {
      printf("Error: Filenames not specified.\n");
      return -1;
   }

   //
   // If build.h exists then parse for BUILD_INC
   //
   if  ((pVer = fopen(argv[2],"rt")) != NULL) {
      while (fgets(inLine, sizeof(inLine), pVer) != NULL) {
         token = strtok(inLine, " ");
         token = strtok(NULL, " ");
         if (token == NULL) continue;
         if (_stricmp(token, "BUILD_INC") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &buildInc);
            buildInc &= 0x00FF;
            buildIncFound = 1;
            break;
         }
      }
      fclose(pVer);
   }

   //
   // Parse build.inc
   //
   if  ((pVer = fopen(argv[1],"rt")) != NULL) {
      if (fgets(inLine, sizeof(inLine), pVer) != NULL) {
         token = strtok(inLine, " ");
         token = strtok(NULL, " ");
         if (_stricmp(token, "BUILD_MAJOR") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &buildMajor);
            buildMajor &= 0x00FF;
         }
         else {
            bError = 1;
            goto ERROR;
         }
      }
      else {
         bError = 1;
         goto ERROR;
      }
      if (fgets(inLine, sizeof(inLine), pVer) != NULL) {
         token = strtok(inLine, " ");
         token = strtok(NULL, " ");
         if (_stricmp(token, "BUILD_MINOR") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &buildMinor);
            buildMinor &= 0x00FF;
         }
         else {
            bError = 1;
            goto ERROR;
         }
      }
      else {
         bError = 1;
         goto ERROR;
      }
      if (fgets(inLine, sizeof(inLine), pVer) != NULL) {
         token = strtok(inLine, " ");
         token = strtok(NULL, " ");
         if (_stricmp(token, "BUILD_NUM") == 0) {
            token = strtok(NULL, "\n");
            sscanf(token, "%d", &buildNum);
            buildNum &= 0x00FF;
         }
         else {
            bError = 1;
            goto ERROR;
         }
      }
      else {
         bError = 1;
         goto ERROR;
      }
      if (fgets(inLine, sizeof(inLine), pVer) != NULL) {
         token = strtok(inLine, " ");
         token = strtok(NULL, " ");
         if (_stricmp(token, "BUILD_INC") == 0) {
            token = strtok(NULL, "\n");
            // If buildInc not found in build.h
            if (buildIncFound == 0) {
               sscanf(token, "%d", &buildInc);
               buildInc &= 0x00FF;
            }
         }
         else {
            bError = 1;
            goto ERROR;
         }
      }
      else {
         bError = 1;
         goto ERROR;
      }
      fclose(pVer);
   }

   ERROR:

   if (bError == 1) {
      fclose(pVer);
      printf("Error Parsing %s\n", argv[1]);
      return -1;
   }

   // Increment Build Number
   buildInc++;
   buildInc &= 0x00FF;

   // 32-Bit Build Number
   buildHex = (buildMajor << 24) | (buildMinor << 16) | (buildNum << 8) | buildInc;

   // Open File for Writing
   if  ((pVer = fopen(argv[2],"wt")) == NULL) {
      printf("Error: Unable to open %s for writing\n", argv[2]);
      exit (-1);
   }

   // Get Current Time & Date
   time(&aclock);
   newtime = localtime(&aclock);
   sprintf(buildTime,"%02d:%02d", newtime->tm_hour, newtime->tm_min);
   sprintf(buildDate,"%02d.%s.%02d", newtime->tm_mday, month[newtime->tm_mon],
         (newtime->tm_year+1900)-2000);

   // Get Current User
   memset(buildUser, 0, sizeof(buildUser));
   pEnv = getenv("USERNAME");
   if (pEnv != NULL) {
      strcpy(buildUser, pEnv);
   }
   else {
      buildUser[0] = '\0';
   }

   // Create Build Strings
   sprintf(buildLo,"%d.%d.%d build %d", buildMajor, buildMinor, buildNum, buildInc);
   sprintf(buildHi,"%s, %s %s [%s]", buildLo, buildTime, buildDate, buildUser);
   sprintf(buildStr,"%d.%d.%d.%d", buildMajor, buildMinor, buildNum, buildInc);

   fprintf(pVer, "#define BUILD_MAJOR     %d\n", buildMajor);
   fprintf(pVer, "#define BUILD_MINOR     %d\n", buildMinor);
   fprintf(pVer, "#define BUILD_NUM       %d\n", buildNum);
   fprintf(pVer, "#define BUILD_INC       %d\n", buildInc);
   fprintf(pVer, "#define BUILD_HEX       %08X\n", buildHex);
   fprintf(pVer, "#define BUILD_TIME      \"%s\"\n", buildTime);
   fprintf(pVer, "#define BUILD_DATE      \"%s\"\n", buildDate);
   fprintf(pVer, "#define BUILD_USER      \"%s\"\n", buildUser);
   fprintf(pVer, "#define BUILD_STR       \"%s\"\n", buildStr);
   fprintf(pVer, "#define BUILD_LO        \"%s\"\n", buildLo);
   fprintf(pVer, "#define BUILD_HI        \"%s\"\n", buildHi);
   fprintf(pVer, "#define BUILD_STAMP     %d\n", (unsigned int)aclock);

   fclose(pVer);

   printf("Build String:  %s\n\n", buildHi);

   return 0;

}
