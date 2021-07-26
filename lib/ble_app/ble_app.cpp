#include "ble_app.h"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic_RX = NULL;
BLECharacteristic* pCharacteristic_TX = NULL;
bool deviceConnected = false;
extern bool oldDeviceConnected;
uint32_t value = 0;
extern ESP32Time rtc;


#define BLE_DEVICE_NAME     "HOWLAB_ESP32"
#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_RX_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define CHARACTERISTIC_TX_UUID "beb5483e-36e1-4688-b7f5-ea07361b26b9"
#define BUFFER_COMANDO_SIZE 200
#define STRING_HORA_LENGTH 14
class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      /*TODO: LECTURA COMANDOS*/
      oldDeviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
       /*TODO: */
       oldDeviceConnected = false;
    }
    
    
};

#define SIZE_OF_STATIC_JSON_DOCUMENT 200
#define SIZE_OF_COMMAND 100
extern Estados_t estado;
extern RTC_DATA_ATTR int TIME_TO_SLEEP;
extern RTC_DATA_ATTR char MY_WIFI_SSID[20];
extern RTC_DATA_ATTR char MY_WIFI_PASSWORD[20];
static char buffer_mio[100];
static char * vector_comandos[8] = {"SD","GD","GW","GS","GL","LS","DL","SW"};

extern RTC_DATA_ATTR int horas;
extern RTC_DATA_ATTR int minutos ;
extern RTC_DATA_ATTR int segundos;
extern RTC_DATA_ATTR int dia ;
extern RTC_DATA_ATTR int mes;
extern RTC_DATA_ATTR int year;



class MyCallbacks: public BLECharacteristicCallbacks 
{
    void onWrite(BLECharacteristic *pCharacteristic) 
    {
          int numero_comando=-1;
          std::string value =  pCharacteristic->getValue() ;
                            
          for(int i = 0; i<value.length(); i++ )
          {
            numero_comando = process_mesage(value[i], buffer_mio, 100, vector_comandos, 8, 0);
          }
          if(numero_comando > (-1) )
          {
            char respuesta_aux[50];
            StaticJsonDocument<SIZE_OF_STATIC_JSON_DOCUMENT> doc;
            String json_file_s(buffer_mio);
            deserializeJson(doc,json_file_s);
	          //Chequear si es un objeto
            JsonObject obj = doc.as<JsonObject>();
            switch(numero_comando)
				    {
              case 0: // Se ha enviado un comando de SetDevice
              {
                /*StaticJsonDocument<SIZE_OF_STATIC_JSON_DOCUMENT> doc;
                String json_file_s(buffer_mio);
                deserializeJson(doc,json_file_s);
                //Chequear si es un objeto
                JsonObject obj = doc.as<JsonObject>();*/
                /*Se meterá asi la hora: (hora)(minuto)(segundo)(dia)(mes)(año) -> 23050202062020  =   23:05:02/02/06/2020*/
                String respuest_sd_t = obj[String("t")];
                if( strcmp(respuesta_aux, "null" ) != 0)
                {
                  if(respuest_sd_t.length() == STRING_HORA_LENGTH) 
                  {
                    horas = (respuest_sd_t.c_str()[0]-'0')*10 + (respuest_sd_t.c_str()[1]-'0');
                    minutos = (respuest_sd_t.c_str()[2]-'0')*10 + (respuest_sd_t.c_str()[3]-'0');
                    segundos = (respuest_sd_t.c_str()[4]-'0')*10 + (respuest_sd_t.c_str()[5]-'0');
                    dia = (respuest_sd_t.c_str()[6]-'0')*10 + (respuest_sd_t.c_str()[7]-'0');
                    mes = (respuest_sd_t.c_str()[8]-'0')*10 + (respuest_sd_t.c_str()[9]-'0');
                    year = (respuest_sd_t.c_str()[10]-'0')*1000 + (respuest_sd_t.c_str()[11]-'0')*100 +(respuest_sd_t.c_str()[12]-'0')*10+ (respuest_sd_t.c_str()[13]-'0');

                    rtc.setTime(segundos, minutos, horas, dia, mes, year);   
                  }      
                }
                String respuest_sd_p = obj[String("p")];
                sprintf(respuesta_aux, "%s", respuest_sd_p);
                if( strcmp(respuesta_aux, "null" ) != 0)
                {
                    TIME_TO_SLEEP = (respuest_sd_p.toInt());    	
                }
                          
                String respuest_sd_m = obj[String("m")];
                sprintf(respuesta_aux, "%s", respuest_sd_m);
                if( strcmp(respuesta_aux, "null" ) != 0)
                {
                  switch(respuest_sd_m.toInt())
                  {
                    case 0:

                      // Dormido ON (RTC + BLE Comandos)
                      pCharacteristic_RX->setValue("Bajo Consumo");
                      pCharacteristic_RX->notify();
                      estado = ESPERA;
                      
                    break;
                    case 1:
                      //Activo: ON (RTC + BLE Comandos + PROCESOR + SENSOR + SD) - OFF (WIFI + FTP SERVER + Envío Datos BLE)
                      pCharacteristic_RX->setValue("modo_activo");
                      pCharacteristic_RX->notify();
                      estado = ACTIVO;
                      oldDeviceConnected = false;
                      
                    break;
                    case 2:
                      //Extraccion: ON (RTC + BLE Comandos + WIFI + FTP SERVER) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE)
                        pCharacteristic_RX->setValue("modo_extraccion");
                        pCharacteristic_RX->notify();
                        estado = EXTRACCION;
                        oldDeviceConnected = false;
                        
                    break;
                    case 3:
                      //Streaming 1: ON (RTC + BLE Comandos +  Envío Datos BLE) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE + WIFI + FTP SERVER)
                        pCharacteristic_RX->setValue("modo_streaming");
                        pCharacteristic_RX->notify();	
                        estado = STREAMING;
                        oldDeviceConnected = false;
                    break;
                    case 4:
                      //Streaming 1: ON (RTC + BLE Comandos +  Envío Datos BLE) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE + WIFI + FTP SERVER)
                        pCharacteristic_RX->setValue("DORMIDO");
                        pCharacteristic_RX->notify();	
                        estado = DORMIDO;
                        oldDeviceConnected = false;

                    break;
                    default:/*Lo incluyo?*/
                      //Streaming 1: ON (RTC + BLE Comandos +  Envío Datos UART) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE + WIFI + FTP SERVER)
                        pCharacteristic_RX->setValue("NOT FOUND");
                        pCharacteristic_RX->notify();
                      
                    break;
                  }

						    }

              }
                break;
                case 1://Comando GD: "Get Device"
                {     
                    String respuest_sd_e = obj[String("p")];
                    sprintf(respuesta_aux, "%s", respuest_sd_e);
                    if( strcmp(respuesta_aux, "null" ) != 0)
                    {
                      if(strcmp(respuesta_aux, "h" ) == 0) //Devuelve la hora
                      {
                        pCharacteristic_RX->setValue( rtc.getTime().c_str() );
                        pCharacteristic_RX->notify(); 
                      }
                      else if(strcmp(respuesta_aux, "e" )== 0) //Devuelve el estado
                      {
                        switch(estado)
                        {
                          case ESPERA:
                            // Dormido ON (RTC + BLE Comandos)
                            pCharacteristic_RX->setValue("E: Espera");
                            pCharacteristic_RX->notify();
                          break;
                          case ACTIVO:
                            //Activo: ON (RTC + BLE Comandos + PROCESOR + SENSOR + SD) - OFF (WIFI + FTP SERVER + Envío Datos BLE)
                            pCharacteristic_RX->setValue("E: Activo");
                            pCharacteristic_RX->notify();
                          break;
                          case STREAMING:
                            //Streaming 1: ON (RTC + BLE Comandos +  Envío Datos BLE) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE + WIFI + FTP SERVER)
                              pCharacteristic_RX->setValue("E: Streaming");
                              pCharacteristic_RX->notify();	
                          break;
                          case EXTRACCION:
                            //Extraccion: ON (RTC + BLE Comandos + WIFI + FTP SERVER) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE)
                              pCharacteristic_RX->setValue("E: Extraccion");
                              pCharacteristic_RX->notify();
                          break;
                          case DORMIDO:
                            //Extraccion: ON (RTC + BLE Comandos + WIFI + FTP SERVER) - OFF (PROCESOR + SENSOR + SD + Envío Datos BLE)
                              pCharacteristic_RX->setValue("E: Dormido");
                              pCharacteristic_RX->notify();
                          break;

                        }
                      }
                      else if(strcmp(respuesta_aux, "f" )== 0)
                      {
                        pCharacteristic_RX->setValue( rtc.getDate(false).c_str() );
                        pCharacteristic_RX->notify(); 
                      }
                      else if(strcmp(respuesta_aux, "b" )== 0)
                      {
                        pCharacteristic_RX->setValue( "80%" );
                        pCharacteristic_RX->notify(); 
                      }
                    }
                    

                }
                break;
                case 2://Comando GW
                {
                  
                  pCharacteristic_RX->setValue("GW");
                  pCharacteristic_RX->notify();
                }
                break;
                case 3://Comando GS
                {
                  pCharacteristic_RX->setValue(vector_comandos[numero_comando]);
                  pCharacteristic_RX->notify();
                }
                break;
                case 4://Comando GL
                {
                  pCharacteristic_RX->setValue(vector_comandos[numero_comando]);
                  pCharacteristic_RX->notify();
                }
                break;
                case 5: //Comando LS
                {
                  pCharacteristic_RX->setValue(vector_comandos[numero_comando]);
                  pCharacteristic_RX->notify();
                }
                break;
                case 6://Comando DL
                {
                  pCharacteristic_RX->setValue(vector_comandos[numero_comando]);
                  pCharacteristic_RX->notify();
                }
                break;
                case 7://Comando SW
                {
                  String respuest_sw_s = obj[String("s")];
                  char respuesta_aux_1[20];
                  sprintf(respuesta_aux_1, "%s", respuest_sw_s);
                  if( strcmp(respuesta_aux_1, "null" ) != 0)
                  {
                      String respuest_sw_p = obj[String("p")];
                      sprintf(respuesta_aux, "%s", respuest_sw_p);
                      if( strcmp(respuesta_aux, "null" ) != 0)
                      {
                        strcpy(&MY_WIFI_PASSWORD[0],&respuesta_aux[0]);  
                        strcpy(&MY_WIFI_SSID[0],&respuesta_aux_1[0]);
                        EEPROM.begin(100);
                        EEPROM.write(0, '/');
                        EEPROM.write(1, '*');
                        int cadena1_size = strlen(MY_WIFI_SSID);
                        EEPROM.put(2,MY_WIFI_SSID);
                        EEPROM.put(cadena1_size+3,MY_WIFI_PASSWORD);
                        EEPROM.end(); 
   
                      }
                  }
                  pCharacteristic_RX->setValue(vector_comandos[numero_comando]);
                  pCharacteristic_RX->notify();
                }
                break;
              }
            }
           
                      //  BaseType_t xHigherPriorityTaskWoken = pdFALSE;
                      /* un-block the interrupt processing task now */
                      //  xSemaphoreGiveFromISR( xBinarySemaphore, &xHigherPriorityTaskWoken );
          }
   
    
};


/*Función de inicialización de la interfaz de BLE.*/

int ble_app_init(void)
{
  
    // Create the BLE Device
    
  BLEDevice::init(BLE_DEVICE_NAME);
  
  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pCharacteristic_RX = pService->createCharacteristic(
                      CHARACTERISTIC_RX_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_NOTIFY
                    );
  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic_RX->addDescriptor(new BLE2902());

  pCharacteristic_TX = pService->createCharacteristic(
                      CHARACTERISTIC_TX_UUID,
                      BLECharacteristic::PROPERTY_WRITE
                    );
pCharacteristic_TX->setCallbacks(new MyCallbacks());


  // Start the service
  pService->start();
  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();
  return 0;
}

int ble_app_deinit(void)
{
BLEDevice::deinit(false);

}

int ble_app_envio(char * cadena)
{
pCharacteristic_RX->setValue(cadena);
                  pCharacteristic_RX->notify();
return 1;
}