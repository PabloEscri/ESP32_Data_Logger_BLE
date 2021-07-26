#ifndef MPU_APP
#define MPU_APP




#include <MPU9250_asukiaaa.h>
#include "stdint.h"


typedef struct {
    float x;
    float y;
    float z;
}Axes_t;

void mpu_app_init(void);
 int getAccel_app(Axes_t * axes_accel);
 int getGyro_app(Axes_t * axes_gyro);
 int getMag_app(Axes_t * axes_mag);
 
 

#endif
