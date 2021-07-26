#ifndef PARSON_JSON_APP
#define PARSON_JSON_APP
#include <stdint.h>
#include <ArduinoJson.h>
#include <string.h>

int parsear_comandos_json(char * json_file,  char ** vector_comandos, const int numero_comandos);
int check_JSON(char pData, const int maximum_len, const int resetear);

int process_mesage(char pData,  char * buffer_json, const uint32_t buffer_size, char ** vector_comandos, const int numero_comandos, const int resetear);
#endif
