#include <Arduino.h>
#include "main.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include "ESP32FtpServer.h"
#include"sd_app.h"
#include"ble_app.h"
#include"mpu_app.h"
#include "parson_json_app.h"
#include"encender_led.h"
#include "ESP32Time.h"
#include <EEPROM.h>



#define MODO_DEBUG_ON 0
/*Frecuencia con la que se va a ejecutar el modo activo.*/
#define mS_TO_S_FACTOR 1000  /* Conversion factor  */
RTC_DATA_ATTR int TIME_TO_SLEEP = 40;        //PERIODO_MUESTREO---Time ESP32 will go to sleep 
#define CABECERA_FICHERO1 "Timestamp;Accel_x;Accel_y;Accel_z;Gyro_x;Gyro_y;Gyro_z;Magne_x;Magne_y;Magne_z\r\n"
#define CABECERA_FICHERO "Accel_x;Accel_y;Accel_z;Gyro_x;Gyro_y;Gyro_z;Magne_x;Magne_y;Magne_z\r\n"
/*
*Defino los parámetros de configuración de la WIFI:
*/
uint32_t WIFI_LIVE_TIME_MS = 30000; // 30 second WiFi connection timeout
#define WIFI_CONNECTED_TIMEOUT_MS 15000 // 15 second WiFi connection timeout
#define USER_FTP "esp32"
#define PASS_FTP "esp32"
#define WIFI_SSID_BY_DEFAULT "A"
#define WIFI_PASS_BY_DEFAULT "A"

/*
*Defino los parámetros de configuración del modo espera BLE:
*/
#define WIFI_CONNECTED_TIMEOUT_MS 15000     // 15 second WiFi connection timeout
#define ITERACIONES_HASTA_CERRAR 100        // Numero de iteraciones entre que cierras y abres la SD.
#define TIEMPO_TOMAR_DATOS_Y_GUARDAR 1      //(Hay 4ms que se van por hacer todas las operaciones)

//---------------------------------------------------------------------------------------------------
//--------------------------------------VARIABLES PRIVADAS ------------------------------------------
//---------------------------------------------------------------------------------------------------

/*Variables privadas para gestionar la WiFi*/
FtpServer ftpSrv;
RTC_DATA_ATTR char MY_WIFI_SSID[20];        // Tamaño de la Wifi SSID
RTC_DATA_ATTR char MY_WIFI_PASSWORD[20];    // Tamaño de la Wifi PASS


/*Variables utilizadas por el programa.*/
RTC_DATA_ATTR Estados_t estado = DORMIDO; // Estado inicial de la placa
RTC_DATA_ATTR int inicializado = 0;       // Indica si se ha ejecutado ya una vez el setUp, en caso de ser así se ignorará hasta RESET por Vcc bajo.
bool oldDeviceConnected = false;          // Indica si se ha establecido alguna conexión.

/*Declaración de las funciones utilizadas por el programa.*/
void enviar_datos(void );
void procesar_datos( void);
void leer_sensores( void );
void levantar_server_ftp( void);
void guardar_SD(void);

/*RTC Variables*/
ESP32Time rtc;
RTC_DATA_ATTR int horas = 20;
RTC_DATA_ATTR int minutos = 31;
RTC_DATA_ATTR int segundos= 50;
RTC_DATA_ATTR int dia = 23;
RTC_DATA_ATTR int mes = 2;
RTC_DATA_ATTR int year = 2021;
#define FILE_NAME_SIZE_CHARS 80
char nombre_fichero[FILE_NAME_SIZE_CHARS];
#define EEPROM_SIZE 100

void setup(){
  
   
  #if MODO_DEBUG_ON
    Serial.begin(115200);
    Serial.println("UART ON");
  #endif
  if(inicializado == 0)
  {
      inicializado = 1;
      #if MODO_DEBUG_ON
        Serial.println("Dentro de inicializado");
      #endif
      
      //strcpy(MY_WIFI_SSID, WIFI_SSID_BY_DEFAULT);
      //strcpy(MY_WIFI_PASSWORD, WIFI_PASS_BY_DEFAULT );
      /*#ifdef COMMENTS_ON
        Serial.begin(115200);
      #endif
      delayMicroseconds(1000); //Take some time to open up the Serºial Monitor
      iniciar_led();*/

     
  }
  EEPROM.begin(EEPROM_SIZE);
  if( ((char)EEPROM.read(0) == '/') && ((char)EEPROM.read(1) == '*') )
  {
    #if MODO_DEBUG_ON
      Serial.println("Cadena Iniciada");
    #endif
    EEPROM.get(2,MY_WIFI_SSID);
    #if MODO_DEBUG_ON
    Serial.println("Holi");
    Serial.println(MY_WIFI_SSID);
    delayMicroseconds(1000);
    #endif
    EEPROM.get(strlen(MY_WIFI_SSID)+3,MY_WIFI_PASSWORD);
    #if MODO_DEBUG_ON
      Serial.println(MY_WIFI_PASSWORD);
    #endif
    
  }
  else
  {
    strcpy(MY_WIFI_SSID, WIFI_SSID_BY_DEFAULT);
    strcpy(MY_WIFI_PASSWORD, WIFI_PASS_BY_DEFAULT );  
    #if MODO_DEBUG_ON
      Serial.println("No hay ssid ni pass en la flash");
    #endif
  }
  EEPROM.end();
  /*segundos = rtc.getSecond();
  minutos = rtc.getMinute();
  rtc.setTime(segundos, minutos, horas, dia, mes, year);*/
  
  sd_app_init(); 
  mpu_app_init();
  //iniciar_led();
  //apagar_led();
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  switch(wakeup_reason)
  {
      case ESP_SLEEP_WAKEUP_EXT0 : 
        estado = ESPERA; 
        ble_app_init(); 
        
        #if MODO_DEBUG_ON
          Serial.println("Wakeup caused by external signal using RTC_IO");
        #endif
      break;
      //case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
      case ESP_SLEEP_WAKEUP_TIMER : 
        #if MODO_DEBUG_ON
          Serial.println("Wakeup caused by timer"); 
        #endif
      break;
      //case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
      //case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
      default : 
        #if MODO_DEBUG_ON
          Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); 
        #endif
        break;
  }
  TIME_TO_SLEEP = TIME_TO_SLEEP - TIEMPO_TOMAR_DATOS_Y_GUARDAR;
  
}

void loop(){

  switch(estado)
  {
          case  ESPERA: // Dormido ON (RTC + BLE Comandos)
          {
              #if MODO_DEBUG_ON
                Serial.println("Modo espera");
              #endif

              unsigned int tiempo_ant = millis();
              oldDeviceConnected = false;

              while((abs((millis()- tiempo_ant)<WIFI_CONNECTED_TIMEOUT_MS)) && (oldDeviceConnected == false)) // Aguanta en modo Espera BT mientras que no hayan pasado 15s o no haya habido conexión.
              {
                #if MODO_DEBUG_ON
                 // Serial.println(millis());
                #endif
                  delayMicroseconds(1);
              }

              if(oldDeviceConnected == true) // Si se ha establecido una conexión debemos mantener la conexión hasta que una IRQ de cambio de modo salte.
              {
                while(oldDeviceConnected== true)
                {
                  #if MODO_DEBUG_ON
                    Serial.println("BLE ON");
                  #endif
                  
                  delayMicroseconds(1);
                };
                if(estado!=STREAMING)
                {
                  #if MODO_DEBUG_ON
                   Serial.println("streaming off");
                  #endif
                   delayMicroseconds(1);
                   ble_app_deinit(); //Si hay desconexión o cambio de modo se apaga la radio. ¿Ponemos modo streaming?
                }
                else
                {
                  #if MODO_DEBUG_ON
                    Serial.println("streaming on");
                  #endif
                  delayMicroseconds(1);
                }
              }
              else // En caso de no haberse establecido una conexión en ese tiempo debemos irnos a modo dormido.
              {
                  estado = DORMIDO;
                  #if MODO_DEBUG_ON
                    Serial.println("No conectado");
                  #endif
              }
        
          }    
          break;
          case  DORMIDO:
          	// Dormido ON (RTC + BLE Comandos)
            #if MODO_DEBUG_ON
              Serial.println("Modo Dormido");
            #endif
            ble_app_deinit(); // Se apaga la radio BLE.
            esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,0); //1 = High, 0 = Low
            delayMicroseconds(1);                                 // No se si es necesario (Probar)
            esp_deep_sleep_start();                      // Modo bajo consumo
          break;

          case  ACTIVO:
          {

            
            strcpy(nombre_fichero,rtc.getTime("/%Y_%m_%d_%H_%M_%S.txt").c_str()); //La longitud del prefijo no va a superar nunca los 70 chars
            while(existe_archivo(nombre_fichero))
            {
              strcpy(nombre_fichero,rtc.getTime("/%Y_%m_%d_%H_%M_%S.txt").c_str()); //Cojo una nueva hora
            }
            open_my_file(nombre_fichero); //NUEVO FICHERO
            append_my_file(CABECERA_FICHERO);
            
            //Activo: ON (RTC + BLE Comandos + PROCESOR + SENSOR + SD) - OFF (WIFI + FTP SERVER + Envío Datos BLE)
            #if MODO_DEBUG_ON
              Serial.println("Modo Activo"); 
              Serial.println("Nombre de fichero:");
              Serial.println(nombre_fichero);
            #endif
            
            for(;;)
            {
              
              /*Tareas a realizar en modo activo.*/
              leer_sensores();
              procesar_datos();
              guardar_SD();
              #if MODO_DEBUG_ON
                Serial.println(rtc.getTime("%A, %B %d %Y %H:%M:%S")); 
                Serial.flush();
                Serial.println("Aquí"); 
              #endif
              
              /*Selecciono los modos de despertar.*/
              esp_sleep_enable_ext0_wakeup(GPIO_NUM_26,0); //Me despierto si tengo en el pin 26 un 0 lógico.
              esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * mS_TO_S_FACTOR);
              delayMicroseconds(1);
              esp_light_sleep_start(); //Podemos o bien hacer un deep sleep (pero creo que entre que se inicia y no el micro pasa mucho tiempo)
              esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
              if(wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
              { 
                  estado = ESPERA; 
                  #if MODO_DEBUG_ON 
                    Serial.println("Wakeup caused by external signal using RTC_IO"); 
                     Serial.println("Aquí 2");
                  #endif
                   
                   
                  ble_app_init();
                  break;
              }
              // esp_deep_sleep_start();
            }
          }
          break;

          case EXTRACCION://Extraccion: ON (RTC + BLE Comandos + WIFI + FTP SERVER) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE)
            
            #if MODO_DEBUG_ON
              Serial.println("Modo Extraccion");
            #endif

            levantar_server_ftp();
            ESP.restart();

          break;

          case STREAMING:
            //Streaming 1: ON (RTC + BLE Comandos +  Envío Datos BLE) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE + WIFI + FTP SERVER) 
            #if MODO_DEBUG_ON
              Serial.println("Modo Streaming");
            #endif
            leer_sensores();
            procesar_datos();
            //guardar_SD();
            enviar_datos();

            //esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * mS_TO_S_FACTOR);
            //esp_light_sleep_start();
             //esp_deep_sleep_start();
           break;

          default:
            #if MODO_DEBUG_ON
              Serial.println("NOT FOUND");
            #endif
          break;
        }
    
}


char datos[400];


void procesar_datos( void)
{
 #if MODO_DEBUG_ON
    Serial.println("Pr");
  #endif
  
  
}


struct generic_data_t
{
    Axes_t accel_axes;
    Axes_t Gyro_axes;
    Axes_t Mag_axes;
};
generic_data_t generic_data;
void leer_sensores( void )
{
    #if MODO_DEBUG_ON
      Serial.println("le");
    #endif
    getAccel_app( &generic_data.accel_axes );
    getGyro_app(&generic_data.Gyro_axes );
    getMag_app( &generic_data.Mag_axes );
    #if MODO_DEBUG_ON
        Serial.print(String(generic_data.accel_axes.x) + "\t" + String(generic_data.accel_axes.y) + "\t" + String(generic_data.accel_axes.z) );
        Serial.print("\t" + String(generic_data.Gyro_axes.x) + "\t" + String(generic_data.Gyro_axes.y) + "\t" + String(generic_data.Gyro_axes.z) );
        Serial.println("\t" + String(generic_data.Mag_axes.x) + "\t" + String(generic_data.Mag_axes.y) + "\t" + String(generic_data.Mag_axes.z) );
    #endif
    //sprintf(datos,"%lu;%s;%s;%s;%s;%s;%s;%s;%s;%s\r\n",millis(),String(generic_data.accel_axes.x).c_str(),String(generic_data.accel_axes.y).c_str(),String(generic_data.accel_axes.z).c_str(),String(generic_data.Gyro_axes.x).c_str(),String(generic_data.Gyro_axes.y).c_str(),String(generic_data.Gyro_axes.z).c_str(),String(generic_data.Mag_axes.x).c_str(),String(generic_data.Mag_axes.y).c_str(),String(generic_data.Mag_axes.z).c_str());
    
    sprintf(datos,"%s;%s;%s;%s;%s;%s;%s;%s;%s\r\n",String(generic_data.accel_axes.x).c_str(),String(generic_data.accel_axes.y).c_str(),String(generic_data.accel_axes.z).c_str(),String(generic_data.Gyro_axes.x).c_str(),String(generic_data.Gyro_axes.y).c_str(),String(generic_data.Gyro_axes.z).c_str(),String(generic_data.Mag_axes.x).c_str(),String(generic_data.Mag_axes.y).c_str(),String(generic_data.Mag_axes.z).c_str());
        #if MODO_DEBUG_ON
        Serial.println(datos);
        #endif
}

void guardar_SD(void )
{
  static uint32_t contador;
  #if MODO_DEBUG_ON
    Serial.println("Gu");
  #endif
  //sprintf(datos,"%s- %d;%d;%d;%d;%d;%d;%d;%d;%d\r\n",rtc.getTime("%S").c_str(),(int)generic_data.accel_axes.x,(int)generic_data.accel_axes.y,(int)generic_data.accel_axes.z,(int)generic_data.Gyro_axes.x,(int)generic_data.Gyro_axes.y,(int)generic_data.Gyro_axes.z,(int)generic_data.Mag_axes.x,(int)generic_data.Mag_axes.y,(int)generic_data.Mag_axes.z);
  //sprintf(datos,"%lu;%d;%d;%d;%d;%d;%d;%d;%d;%d\r\n",millis(),(int)generic_data.accel_axes.x,(int)generic_data.accel_axes.y,(int)generic_data.accel_axes.z,(int)generic_data.Gyro_axes.x,(int)generic_data.Gyro_axes.y,(int)generic_data.Gyro_axes.z,(int)generic_data.Mag_axes.x,(int)generic_data.Mag_axes.y,(int)generic_data.Mag_axes.z);
  
  //appendFile(nombre_fichero,datos);
  if(contador >= ITERACIONES_HASTA_CERRAR)
  {
    close_my_file();
    open_my_file(nombre_fichero); //NUEVO FICHERO
    contador = 0;
  }
  append_my_file(datos);
  contador ++;
  
}



extern BLECharacteristic* pCharacteristic_RX;
void enviar_datos( void  )
{
   //sprintf(datos,"%lu;%d;%d;%d;%d;%d;%d;%d;%d;%d\r\n",millis(),(int)generic_data.accel_axes.x,(int)generic_data.accel_axes.y,(int)generic_data.accel_axes.z,(int)generic_data.Gyro_axes.x,(int)generic_data.Gyro_axes.y,(int)generic_data.Gyro_axes.z,(int)generic_data.Mag_axes.x,(int)generic_data.Mag_axes.y,(int)generic_data.Mag_axes.z);
   //static int Howlab_counter = 0;
   //sprintf(datos,"%d",Howlab_counter);
   
   #if MODO_DEBUG_ON
      Serial.println("En");
    #endif
     ble_app_envio(datos);
     delayMicroseconds(1000);
     //Howlab_counter++;

     
}


void levantar_server_ftp( void )
{
   ble_app_deinit(); // Se apaga la radio BLE.
  #if MODO_DEBUG_ON
    Serial.println("Configurando WiFI");
  #endif
  WiFi.begin(&MY_WIFI_SSID[0], &MY_WIFI_PASSWORD[0]);

  // Wait for connection
  unsigned int tiempo_conexion = millis(); 
  while ((WiFi.status() != WL_CONNECTED) && ((millis()-tiempo_conexion) < WIFI_CONNECTED_TIMEOUT_MS)) {
   delayMicroseconds(200);
    #if MODO_DEBUG_ON
      Serial.print(".");
    #endif
  }
  if(WiFi.status() != WL_CONNECTED)
  {
    #if MODO_DEBUG_ON
      Serial.print("No hay conexion..");
    #endif
      ESP.restart(); //Si no se conecta, se ha de resetear la placa.
  }
  else
  {
      //encender_led();
      //delayMicroseconds(100);
    #if MODO_DEBUG_ON
      Serial.println("");
      Serial.print("Conectado a ");
      Serial.println(&MY_WIFI_SSID[0]);
      Serial.print("Pass: ");
      Serial.println(&MY_WIFI_PASSWORD[0]); 
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    #endif
  }
  

  /////FTP Setup, ensure SD is started before ftp;  /////////
  
  if (SD.begin()) {
      //apagar_led();
    #if MODO_DEBUG_ON
      Serial.println("SD opened!");
    #endif
      ftpSrv.begin(USER_FTP,PASS_FTP);    //username, password for ftp.  set ports in ESP32FtpServer.h  (default 21, 50009 for PASV)
  } 
  //unsigned int tiempo_actual = millis();   
for(;;){
     
     if( (WiFi.status() != WL_CONNECTED) )
     {
       //esp_wifi_deinit();
       #if MODO_DEBUG_ON
        Serial.print("Restarting\r\n");
       #endif
       ESP.restart();

     }
     else
     {
       ftpSrv.handleFTP();
       delayMicroseconds(500);
       #if MODO_DEBUG_ON
       // Serial.print("ftp");
      #endif
       
     }
    }
}