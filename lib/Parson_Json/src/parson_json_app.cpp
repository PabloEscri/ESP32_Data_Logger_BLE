#include "parson_json_app.h"

#define SIZE_OF_STATIC_JSON_DOCUMENT 200
#define SIZE_OF_COMMAND 100
int parsear_comandos_json(char * json_file,  char ** vector_comandos, const int numero_comandos)
{
    int status_Comando_Json = -1;

	StaticJsonDocument<SIZE_OF_STATIC_JSON_DOCUMENT> doc;
	String json_file_s(json_file);
	deserializeJson(doc,json_file_s);
	

	//Chequear si es un objeto
	JsonObject obj = doc.as<JsonObject>();
	char msg[SIZE_OF_COMMAND];
	String comando_str = obj[String("c")];
    sprintf(msg, "%s", comando_str);
	
	if( strcmp(msg, "null" ) != 0)
	{
		
		for(int i = 0; i<numero_comandos;i++ )
		{
			if(strcmp(vector_comandos[i],msg) == 0)
			{
				status_Comando_Json = i;
				break;
			}
		}
		if(status_Comando_Json < 0)
		{

			status_Comando_Json = -2;
		}
	}
	else
	{
		status_Comando_Json = -1;
	}
	return status_Comando_Json;
}

int check_JSON(char pData, const int maximum_len, const int resetear)
{
		static int num_llaves_abiertas = 0;	// Contador de llaves abiertas
		static int num_llaves_cerradas = 0; // Contador de llaves cerradas
		static int index_buffer = 0;		// Contador de tamaño de buffer
		int Json_current_status = 1; // JSON está en curso

		if(resetear != 1)			// Si no hay un reset por parte del usuario...
		{

			if((char)pData == '{')
			{
				num_llaves_abiertas++;
			}

			if(num_llaves_abiertas > 0)
			{
				index_buffer++ ;

				if((char)pData == '}')
				{
					num_llaves_cerradas++;
					if(num_llaves_cerradas == num_llaves_abiertas)
					{
						
						index_buffer = 0;
						num_llaves_abiertas = 0;
						num_llaves_cerradas = 0;
						Json_current_status = 2;  // JSON completo
					}
				}

				if(index_buffer == maximum_len)
				{
					
					index_buffer = 0;
					num_llaves_abiertas = 0;
					num_llaves_cerradas = 0;
					Json_current_status = 3;  // JSON demasiado grande
				}
			}
			else
			{
				
				Json_current_status = 0; // JSON no se ha abierto
			}
		}
		else
		{
			
			index_buffer = 0;
			num_llaves_abiertas = 0;
			num_llaves_cerradas = 0;
			Json_current_status = 4;  // JSON Reseteado
		}

		return Json_current_status;
}

int process_mesage(char pData,  char * buffer_json, const uint32_t buffer_size, char ** vector_comandos, const int numero_comandos, const int resetear)
{
	int process_message =-6; // Inicio del estado de la función
	static int index = 0;
	int JSON_status = check_JSON(pData, buffer_size, resetear);

	switch(JSON_status)
	{
		case 0: //JSON NO ABIERTO
		{	
			process_message = -4;
        }
		break;
		case 1: //JSON ABIERTO Y EN CURS0. JSON INCOMPLETO.
        {
			buffer_json[index] = pData;
			index++;
			process_message = -3;
        }
		break;
		case 2://JSON TERMINADO
        {
			buffer_json[index] = pData;
			index = 0;
			int status = parsear_comandos_json(buffer_json, vector_comandos, numero_comandos);
			switch(status)
			{
				case -1:
				{
					// No contiene Instrucción "c"

					process_message = -5;
				}
				break;
				case -2:
				{
					// Contiene Instrucción "c" pero COMANDO NO VÁLIDO
					process_message = -6;
                }
				break;
				default: // Devuelve número de comando encontrado:
				{
					process_message = status;
                }
				break;
			}
        }
		break;
		case 3:
		{
		//JSON DEMASIADO GRANDE
			index = 0;
			process_message = -2;

			break;
		}
		case 4: 
		{
		// Contadores reseteados
			index = 0;
			process_message = -1;
        }
		break;
		}
	return process_message;
}
