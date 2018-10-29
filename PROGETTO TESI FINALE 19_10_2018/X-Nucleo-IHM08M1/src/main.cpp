#include "mbed.h"
#include "DigitalEncoderAB.h"
#include "DigitalEncoderAS5601.h"
#include "DigitalEncoderPIC.h"

#define TIM_USR         TIM3
#define TIM_USR_IRQ     TIM3_IRQn

Timer tim; //definizione di timer
//potenziometro
AnalogIn input(PA_4); //scheda piccola --> PB_1 //scheda grossa --> PA_4

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

//metodi per impostare il timer software

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
//setting LED1 to give digital output
DigitalOut myled(LED1);

//DigitalIn readA(PA_13);

// Controllo canali per 6-step
DigitalIn readA(PA_15);
DigitalIn readB(PB_3);
DigitalIn readC(PB_10);

//button pressed
DigitalIn mybutton(USER_BUTTON);
// PIC encoder
// DigitalEncoderPIC encoder(I2C_SDA,I2C_SCL);

// AS5601 encoder
//DigitalEncoderAS5601 encoder(PB_9, PB_8);

//AB encoder (PA0 = A, PA1 = B)
//DigitalEncoderAB encoder(4096);

// PWM duty cycle for motor phase

float pwm_positive = 0.5f;//0.5f;
float gnd_negative = 0.0f;

// ON board LED
//DigitalOut led1(LED1);
// Phase 1 PWM OUT
PwmOut uh_1(PA_8);
// Phase 2 PWM OUT
PwmOut vh_2(PA_9);
// Phase 3 PWM OUT
PwmOut wh_3(PA_10);
// Phase 1 ENABLE PIN
//DigitalOut en_1(PC_10);
DigitalOut en_1(PA_7);
// Phase 2 ENABLE PIN
//DigitalOut en_2(PC_11);
DigitalOut en_2(PB_0);
// Phase 3 ENABLE PIN
//DigitalOut en_3(PC_12);
DigitalOut en_3(PB_1);
// MOTOR DRIVER CHIP ENABLE PIN
DigitalOut en_chip(PA_6); //PA_6

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
    //period pwm
    uh_1.period(0.00001f);
    vh_2.period(0.00001f);
    wh_3.period(0.00001f);
    // for printing float values
    asm(".global _printf_float");

    int hallA, hallB, hallC;

    int step_number = 0;
    //en_chip = 1;

    while(en_chip == 0)
    {
        if (mybutton == 0)
            en_chip = 1;
    }

    //uint16_t saples[1024]; 

    // Enable timer
    __HAL_RCC_TIM3_CLK_ENABLE();

    set10khz(); //set timer

    HAL_TIM_Base_Init(&mTimUserHandle);
    HAL_TIM_Base_Start_IT(&mTimUserHandle);

    NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
    NVIC_EnableIRQ(TIM_USR_IRQ);

    tim.start(); //start timer
    
    while(1) 
    {
        if (flag_time == 1) //clock timer
        {
            pwm_positive = input.read();//set pwm from potenziometer
            //read hall sensors
            hallA = readA.read();
            hallB = readB.read();
            hallC = readC.read();

            //set step_number from method with read hall sensors
            step_number = hallStepRead(hallA,hallB,hallC);
            stepForaward(step_number);
            
            flag_time = 0;
        }
    }
}
