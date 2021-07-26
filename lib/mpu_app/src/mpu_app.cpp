

#include "mpu_app.h"
static MPU9250_asukiaaa mySensor;

#ifdef _ESP32_HAL_I2C_H_
#define SDA_PIN 21
#define SCL_PIN 22
#endif


void mpu_app_init(void) { 
    #ifdef _ESP32_HAL_I2C_H_ // For ESP32
        Wire.begin(SDA_PIN, SCL_PIN);
        mySensor.setWire(&Wire);
    #endif

    mySensor.beginAccel();
    mySensor.beginGyro();
    mySensor.beginMag();

  // You can set your own offset for mag values
  // mySensor.magXOffset = -50;
  // mySensor.magYOffset = -55;
  // mySensor.magZOffset = -10;
}


//Si se devuelve un 1 => No se pudo leer el Accel
// 0 si éxito
 int getAccel_app(Axes_t * axes_accel)
 {
     
     if (mySensor.accelUpdate() == 0) 
     {
        axes_accel->x = mySensor.accelX();
        axes_accel->y = mySensor.accelY();
        axes_accel->z= mySensor.accelZ();
        return 0;
    }
    else
    {
        return 1;
    }
 }
 
 //Si se devuelve un 1 => No se pudo leer el Gyro
// 0 si éxito
 int getGyro_app(Axes_t * axes_gyro)
 {
     
     if (mySensor.gyroUpdate() == 0) 
     {
        axes_gyro->x = mySensor.gyroX();
        axes_gyro->y = mySensor.gyroY();
        axes_gyro->z= mySensor.gyroZ();
        return 0;
    }
    else
    {
        return 1;
    }
 }
 
 //Si se devuelve un 1 => No se pudo leer el Mag
// 0 si éxito
 int getMag_app(Axes_t * axes_mag)
 {
     
     if (mySensor.magUpdate() == 0) 
     {
        axes_mag->x =  mySensor.magX();
        axes_mag->y =  mySensor.magY();
        axes_mag->z=  mySensor.magZ();
        
       
        return 0;
    }
    else
    {
        return 1;
    }
 }
 
 