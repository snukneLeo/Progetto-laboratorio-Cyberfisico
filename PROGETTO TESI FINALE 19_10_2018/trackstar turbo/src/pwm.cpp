#include "mbed.h"

PwmOut pwm1(PA_5);//D3       //create a PWM output called pmw1 on pin PA_5
DigitalIn btn(USER_BUTTON); //create button
Serial pc(SERIAL_TX,SERIAL_RX); //create Serial data
AnalogIn input(PB_1); //create potenzimeter
float value = 0.0;

int main() 
{
    float i = 0.15; //this is limit that engine can start
    int chip = 0; //enable chip with button
    pwm1.period(0.01);  // set PWM period to 10 ms
    pwm1 = 0.15; //set initial pwm

    //loop
    while(chip == 0)
    {
        if(btn == 0)
            chip = 1; //start engine
    }
    //loop
    while(1) 
    {
        //read value by potenziometer
        value = input.read();
        //increase pwmOut
        //create square wave
        while(i <= value)
        {
            i=i+0.02;
            pwm1=i;
            wait(0.3);
        }
        //decrement pwmOut
        while(i > value)
        {
            i=i-0.01;
            pwm1=i;
            wait(0.001);
        }
        if(i == value)
            pc.printf("regime\n");
    }
}