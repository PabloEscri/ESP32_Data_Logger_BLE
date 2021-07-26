#include "sd_app.h"

#define COMENTS_ON 0




/*
return 1 si no se pudo montar la SD
return 2 si no se ha adjuntado la SD
return 0 si todo OK
*/
int sd_app_init(void)
{
     
    if(!SD.begin()){
     #if COMENTS_ON
        Serial.println("Card Mount Failed");
    #endif
        return 1;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
    #if COMENTS_ON
        Serial.println("No SD card attached");
    #endif
        return 2;
    }
  #if COMENTS_ON
        Serial.println("SD Initialized.");
    #endif
	return 0; // OK
}

/*
    Ejemplos:
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    createDir(SD, "/mydir");
    deleteFile(SD, "/hello.txt");
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
	listDir(SD, "/", 0);
*/


void listDir(const char * dirname, uint8_t levels){
    
     #if COMENTS_ON
        Serial.printf("Listing directory: %s\n", dirname);
    #endif
    File root = SD.open(dirname);
    if(!root){
    #if COMENTS_ON
        Serial.println("Failed to open directory");
    #endif
        return;
    }
    if(!root.isDirectory()){
    #if COMENTS_ON
        Serial.println("Not a directory");
    #endif
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            #if COMENTS_ON
                Serial.print("  DIR : ");
                Serial.print (file.name());
            #endif
                time_t t= file.getLastWrite();
                struct tm * tmstruct = localtime(&t);
           
            #if COMENTS_ON
                Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
            #endif
            if(levels){
                listDir(file.name(), levels -1);
            }
        } else {
         #if COMENTS_ON
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.print(file.size());
        #endif
            time_t t= file.getLastWrite();
            struct tm * tmstruct = localtime(&t);
            #if COMENTS_ON
                Serial.printf("  LAST WRITE: %d-%02d-%02d %02d:%02d:%02d\n",(tmstruct->tm_year)+1900,( tmstruct->tm_mon)+1, tmstruct->tm_mday,tmstruct->tm_hour , tmstruct->tm_min, tmstruct->tm_sec);
            #endif
        }
        file = root.openNextFile();
    }
}

void createDir( const char * path){
    #if COMENTS_ON
        Serial.printf("Creating Dir: %s\n", path);
    #endif
    if(SD.mkdir(path)){
    #if COMENTS_ON
        Serial.println("Dir created");
    #endif
    } else {
    #if COMENTS_ON
        Serial.println("mkdir failed");
    #endif
    }
}

void removeDir(const char * path){
    #if COMENTS_ON
        Serial.printf("Removing Dir: %s\n", path);
    #endif
    if(SD.rmdir(path)){
    #if COMENTS_ON
        Serial.println("Dir removed");
    #endif
    } else {
    #if COMENTS_ON
        Serial.println("rmdir failed");
    #endif
    }
}

void readFile( const char * path){
    #if COMENTS_ON
    Serial.printf("Reading file: %s\n", path);
    #endif

    File file = SD.open(path);
    if(!file){
    #if COMENTS_ON
        Serial.println("Failed to open file for reading");
    #endif
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
    #if COMENTS_ON
        Serial.write(file.read());
    #endif
    }
    file.close();
}

void writeFile( const char * path, const char * message){
    #if COMENTS_ON
    Serial.printf("Writing file: %s\n", path);
    #endif
    File file = SD.open(path, FILE_WRITE);
    if(!file){
    #if COMENTS_ON
        Serial.println("Failed to open file for writing");
    #endif
        return;
    }
    if(file.print(message)){
    #if COMENTS_ON
        Serial.println("File written");
    #endif
    } else {
    #if COMENTS_ON
        Serial.println("Write failed");
    #endif
    }
    file.close();
}

void appendFile(const char * path, const char * message)
{
    #if COMENTS_ON
    Serial.printf("Appending to file: %s\n", path);
    #endif

    File file = SD.open(path, FILE_APPEND);
    if(!file){
    #if COMENTS_ON
        Serial.println("Failed to open file for appending");
    #endif
        return;
    }
    if(file.print(message)){
    #if COMENTS_ON
        Serial.println("Message appended");
    #endif
    } else {
    #if COMENTS_ON
        Serial.println("Append failed");
    #endif
    }
    file.close();
}

void renameFile( const char * path1, const char * path2){
    #if COMENTS_ON
        Serial.printf("Renaming file %s to %s\n", path1, path2);
    #endif
    if (SD.rename(path1, path2)) {
    #if COMENTS_ON
        Serial.println("File renamed");
    #endif
    } else {
    #if COMENTS_ON
        Serial.println("Rename failed");
    #endif
    }
}

void deleteFile( const char * path){
    #if COMENTS_ON
        Serial.printf("Deleting file: %s\n", path);
    #endif
    if(SD.remove(path)){
    #if COMENTS_ON
        Serial.println("File deleted");
    #endif
    
    } else {
    
    #if COMENTS_ON
        Serial.println("Delete failed");
    #endif
    }
}

bool existe_archivo(const char * path)
{
    #if COMENTS_ON
    Serial.printf("Appending to file: %s\n", path);
    #endif
    return SD.exists(path); //Aquí va mi triple, la idea es que la capa FS detecte el archivo y modifique el path en consecuencia
}

static File my_file;
void open_my_file(const char * path)
{
    
    my_file = SD.open(path, FILE_APPEND);
}

void append_my_file(const char * message)
{
   
    my_file.print(message);
    
}

void close_my_file(void)
{

    my_file.close();
}
