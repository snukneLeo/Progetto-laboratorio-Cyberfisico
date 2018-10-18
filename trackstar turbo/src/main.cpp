/*#include "mbed.h"

InterruptIn my_button(USER_BUTTON);

AnalogIn input(PB_1);

//PwmOut my_pwm(PB_3);

PwmOut my_pwm(PA_5);

DigitalOut pwm(LED2);

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
    float value = 0.0f;

    while(1)
    {
        value = input.read();
        my_pwm.write(value*3.30);
        pc.printf("%f\n",value*3.30);
    }

    // Set button
   // my_button.fall(&pressed);
}*/
/*

#include "mbed.h"
 
PwmOut pwm(PB_5);
AnalogIn input(PB_1);
Timer tim;

//define value for timer
#define TIM_USR TIM3
#define TIM_USR_IRQ TIM3_IRQn


//flag for timing
volatile char flag_time = 0;
unsigned int mTime = 0;

TIM_HandleTypeDef mTimUserHandle;

// Interrupt service routine
void M_TIM_USR_HANDLER() {
    if (__HAL_TIM_GET_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE) == SET) {
        __HAL_TIM_CLEAR_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE);
        flag_time = 1;
    }
}

//set different frequency
void set1khz() //1khz
{
    mTimUserHandle.Instance             = TIM_USR;
    mTimUserHandle.Init.Prescaler       = 1000;
    mTimUserHandle.Init.CounterMode     = TIM_COUNTERMODE_UP;
    mTimUserHandle.Init.Period          = 83 + 1;
    mTimUserHandle.Init.ClockDivision   = TIM_CLOCKDIVISION_DIV1;
}

void set10khz() //10khz
{
    mTimUserHandle.Instance             = TIM_USR;
    mTimUserHandle.Init.Prescaler       = 100;
    mTimUserHandle.Init.CounterMode     = TIM_COUNTERMODE_UP;
    mTimUserHandle.Init.Period          = 83 + 1;
    mTimUserHandle.Init.ClockDivision   = TIM_CLOCKDIVISION_DIV1;
}
/*
void set1hz() //1hz
{
    mTimUserHandle.Instance             = TIM_USR;
    mTimUserHandle.Init.Prescaler       = 9999;
    mTimUserHandle.Init.CounterMode     = TIM_COUNTERMODE_UP;
    mTimUserHandle.Init.Period          = 8399;
    mTimUserHandle.Init.ClockDivision   = TIM_CLOCKDIVISION_DIV1;
}

void set100hz() //100hz
{
    mTimUserHandle.Instance             = TIM_USR;
    mTimUserHandle.Init.Prescaler       = 10000;
    mTimUserHandle.Init.CounterMode     = TIM_COUNTERMODE_UP;
    mTimUserHandle.Init.Period          = 83 + 1;
    mTimUserHandle.Init.ClockDivision   = TIM_CLOCKDIVISION_DIV1;
}
 
int main() 
{
    // Enable timer
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    //set frequency
    set1hz();

    HAL_TIM_Base_Init(&mTimUserHandle);
    HAL_TIM_Base_Start_IT(&mTimUserHandle);

    NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
    NVIC_EnableIRQ(TIM_USR_IRQ);

    tim.start(); //active timer
    // specify period first
    //pwm.period(4.0f);      // 4 second period
    //pwm.write(0.50f);      // 50% duty cycle, relative to period
    while(1)
    {
        if(flag_time ==  1)
        {
            pwm.period(4.0f);
            pwm.pulsewidth(1.0f);
            pwm.write(input.read());
            flag_time = 0;
        }
    }
    //pwm = 0.5f;          // shorthand for pwm.write()
    //pwm.pulsewidth(2);   // alternative to pwm.write, set duty cycle time in seconds
    //while(1);
}*/