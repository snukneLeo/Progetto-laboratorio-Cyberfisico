#include "mbed.h"

PwmOut pwm1(PA_5);//D3       //create a PWM output called pmw1 on pin D3
//InterruptIn my_btn(USER_BUTTON);
DigitalIn btn(USER_BUTTON);
Serial pc(SERIAL_TX,SERIAL_RX);
AnalogIn input(PB_1);

float value = 0.0;

int main() 
{
    float i = 0.15; //limite minimo per cui il motore deve per forza partire per inziare a girare
    int chip = 0;
    float temp = 0.0;
    pwm1.period(0.01);  // set PWM period to 10 ms
    pwm1 = 0.15;

    while(chip == 0)
    {
        if(btn == 0)
            chip = 1;
    }

    while(1) 
    {
        value = input.read();

        while(i <= value)
        {
            i=i+0.02;
            pwm1=i;
            wait(0.3);
        }
        while(i > value)
        {
            i=i-0.01;
            pwm1=i;
            wait(0.01);
        }
    }
}