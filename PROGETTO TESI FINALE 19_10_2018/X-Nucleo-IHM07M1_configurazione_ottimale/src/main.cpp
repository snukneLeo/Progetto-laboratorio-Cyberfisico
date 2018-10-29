#include "mbed.h"

//define value for timer
#define TIM_USR TIM3
#define TIM_USR_IRQ TIM3_IRQn

//define timer
Timer tim;
//potenziometer
AnalogIn input(PB_1);

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


// 1 Hz, 100Hz, 1KHz, 10KHz
// CLOCK_FREQUENCY = 84000000.0f
// TIM_FREQUENCY = (int) (CLOCK_FREQUENCY / mTimUserHandle.Init.Prescaler + 1 / mTimUserHandle.Init.Period + 1)


//creating an object of serial class to communicate with PC
Serial pc(SERIAL_TX, SERIAL_RX);

// define hall sensors A,B,C
DigitalIn readA(PA_15);
DigitalIn readB(PB_3);
DigitalIn readC(PB_10);

//button
DigitalIn mybutton(USER_BUTTON);

// PWM duty cycle for motor phase
float pwm_positive = 0.5f;
float gnd_negative = 0.0f;

// Phase 1 PWM OUT
PwmOut uh_1(PA_8);
// Phase 2 PWM OUT
PwmOut vh_2(PA_9);
// Phase 3 PWM OUT
PwmOut wh_3(PA_10);
// Phase 1 ENABLE PIN
DigitalOut en_1(PC_10);
// Phase 2 ENABLE PIN
DigitalOut en_2(PC_11);
// Phase 3 ENABLE PIN
DigitalOut en_3(PC_12);

// MOTOR DRIVER CHIP ENABLE PIN
DigitalOut en_chip(PA_6); //default en_chip = 0

// This function converts hall sensor's read into 6 electrical position of 
int hallStepRead(int hallA, int hallB, int hallC)
{
    if(hallA == 0 && hallB == 1 && hallC == 0)
    {
        return 1;
    }
    if (hallA == 1 && hallB == 1 && hallC == 0)
    {
        return 0;
    }
    if (hallA == 1 && hallB == 0 && hallC == 0)
    {
        return 5;
    }
    if (hallA == 1 && hallB == 0 && hallC == 1)
    {
        return 4;
    }
    if (hallA == 0 && hallB == 0 && hallC == 1)
    {
        return 3;
    }
    if (hallA == 0 && hallB == 1 && hallC == 1)
    {
        return 2;
    }
}

//turn on different phase
void stepForaward(int step_number)
{
    switch(step_number)
    {
        //In this case i turn on the phase following the next one.
        case 0: 
            uh_1.write(pwm_positive); //PWM
            vh_2.write(0.0f);         //DISCONNECTED
            wh_3.write(gnd_negative); //GND
            en_1 = 1;                 //PWM
            en_2 = 0;                 //DISCONNECTED
            en_3 = 1;                 //GND
        break;

        case 1:
            uh_1.write(0.0f);         //DISCONNECTED
            vh_2.write(pwm_positive); //PWM
            wh_3.write(gnd_negative); //GND
            en_1 = 0;                 //DISCONNECTED
            en_2 = 1;                 //PWM
            en_3 = 1;                 //GND
        break;

        case 2:
            uh_1.write(gnd_negative); //GND
            vh_2.write(pwm_positive); //PWM
            wh_3.write(0.0f);         //DISCONNECTED
            en_1 = 1;                 //GND
            en_2 = 1;                 //PWM
            en_3 = 0;                 //DISCONNECTED
        break;

        case 3:
            uh_1.write(gnd_negative); //GND
            vh_2.write(0.0f);         //DISCONNECTED
            wh_3.write(pwm_positive); //PWM
            en_1 = 1;                 //GND
            en_2 = 0;                 //DISCONNECTED
            en_3 = 1;                 //PWM
        break;

        case 4:
            uh_1.write(0.0f);         //DISCONNECTED
            vh_2.write(gnd_negative); //GND
            wh_3.write(pwm_positive); //PWM
            en_1 = 0;                 //DISCONNECTED
            en_2 = 1;                 //GND
            en_3 = 1;                 //PWM
        break;

        case 5:
            uh_1.write(pwm_positive); //PWM
            vh_2.write(gnd_negative); //GND
            wh_3.write(0.0f);         //DISCONNECTED
            en_1 = 1;                 //PWM
            en_2 = 1;                 //GND
            en_3 = 0;                 //DISCONNECTED
        break;

        default:
        break;
    }
}

int main()
{
    //set period
    uh_1.period(0.00001f); //period in micro
    vh_2.period(0.00001f);
    wh_3.period(0.00001f);

    // for printing float values
    asm(".global _printf_float");

    //create istance for hall-sensor
    int hallA, hallB, hallC;

    //loop 
    //i need to press blue button for setting en_chip = 1
    //so engine can start
    while(en_chip == 0)
    {
        if (mybutton == 0)
        {
            en_chip = 1;

        }
    }

    // Enable timer
    __HAL_RCC_TIM3_CLK_ENABLE();
    
    //set frequency
    set10khz();

    HAL_TIM_Base_Init(&mTimUserHandle);
    HAL_TIM_Base_Start_IT(&mTimUserHandle);

    NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
    NVIC_EnableIRQ(TIM_USR_IRQ);

    tim.start(); //active timer

    //loop
    while(1) 
    {
        //set pwm by the potenziometer
        pwm_positive = input.read();
        
        //execute istruction based on the frequency of the timer
        if (flag_time == 1)
        {
            //read by hall-sensor
            hallA = readA.read();
            hallB = readB.read();
            hallC = readC.read();

            //active corresponding phase to ready by the hall-sensor
            stepForaward(hallStepRead(hallA,hallB,hallC));

            flag_time = 0; 
        }
    }    
}