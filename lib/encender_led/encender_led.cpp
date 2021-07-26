#include "encender_led.h"
void iniciar_led(void)
{
    pinMode(5,OUTPUT);
}
void encender_led(void)
{
    digitalWrite(5,0); 
    
}
void apagar_led(void)
{
    digitalWrite(5,1); 
    
}
void toggle_led(void)
{
    digitalWrite(5,!digitalRead(5)); 
    
}