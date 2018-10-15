#include "mbed.h"

InterruptIn my_button(USER_BUTTON);
PwmOut my_pwm(PB_3);

DigitalOut led(LED2);

Serial pc(SERIAL_TX,SERIAL_RX);

 
void pressed() 
{
    float value = 3.30;
    float readPwm = my_pwm.read();

    
    if (readPwm == 0.25)
        my_pwm.write(0.50);

    else if (readPwm == 0.50)
        my_pwm.write(0.75);

    else if (readPwm == 0.75)
        my_pwm.write(1.0);

    else
        my_pwm.write(0.25);

    pc.printf("%.2f -- ",my_pwm.read());
    pc.printf("%.2f\n",3.30*my_pwm.read());
}

int main()
{
    // Set PWM
    my_pwm.period_ms(10);
    my_pwm.write(0.5);
    // Set button
    my_button.fall(&pressed);
}