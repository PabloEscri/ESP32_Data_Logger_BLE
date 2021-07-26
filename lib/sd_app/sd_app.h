#ifndef SD_APP
#define SD_APP

#include "FS.h"
#include <SD.h>
#include "SPI.h"
#include <time.h> 
#include <Arduino.h>
int sd_app_init(void);
void listDir(const char * dirname, uint8_t levels);
void createDir(const char * path);
void removeDir(const char * path);
void readFile( const char * path);
void writeFile(const char * path, const char * message);
void appendFile(const char * path, const char * message);
void renameFile(const char * path1, const char * path2);
void deleteFile(const char * path);

#endif
