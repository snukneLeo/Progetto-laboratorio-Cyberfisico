#include "mbed.h"
#include "DigitalEncoderAB.h"
#include "DigitalEncoderAS5601.h"
#include "DigitalEncoderPIC.h"

#define TIM_USR         TIM3
#define TIM_USR_IRQ     TIM3_IRQn

//definizione del timer
Timer tim;
//potenziometro
AnalogIn input(PB_1); //scheda piccola --> PB_1 //scheda grossa --> PA_4

volatile char flag_time = 0;
unsigned int mTime = 0;

DigitalIn btn(USER_BUTTON);

TIM_HandleTypeDef mTimUserHandle;

// Interrupt service routine
void M_TIM_USR_HANDLER() {
    if (__HAL_TIM_GET_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE) == SET) {
        __HAL_TIM_CLEAR_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE);
        flag_time = 1;
    }
}

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
//DigitalOut myled(LED1);

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
DigitalOut en_1(PC_10);
//DigitalOut en_1(PB_15);
// Phase 2 ENABLE PIN
DigitalOut en_2(PC_11);
//DigitalOut en_2(PB_0);
// Phase 3 ENABLE PIN
DigitalOut en_3(PC_12);
//DigitalOut en_3(PB_1);
// MOTOR DRIVER CHIP ENABLE PIN
DigitalOut en_chip(PA_6); //PA_6 oppure PA_11

// This function converts hall sensor's read into 6 electrical position of 
int hallStepRead(int hallA, int hallB, int hallC)
{
      if(hallA == 0 && hallB == 1 && hallC == 0) //0
    {
        return 3;//step_number=1;
    }
    if (hallA == 1 && hallB == 1 && hallC == 0) //5
    {
        return 2;//step_number = 2;
    }
    if (hallA == 1 && hallB == 0 && hallC == 0) //4
    {
        return 1;//step_number = 3;
    }
    if (hallA == 1 && hallB == 0 && hallC == 1) //3
    {
        return 0;//step_number = 4;
    }
    if (hallA == 0 && hallB == 0 && hallC == 1) //2
    {
        return 5;//step_number = 5;
    }
    if (hallA == 0 && hallB == 1 && hallC == 1) //1
    {
        return 4;//step_number = 6;
    }
}

//6-step in base agli enable e al pwm
void stepForaward(int step_number)
{
    switch(step_number){
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


float valuePot()
{
    return input.read();
}

int main()
{
    en_chip = 0;
    int step_number = 0;
    int readFirst = 0;

    uh_1.period(0.00001f); //period in micro
    vh_2.period(0.00001f);
    wh_3.period(0.00001f);
    // for printing float values
    asm(".global _printf_float");


    while(en_chip == 0)
    {
        if (btn == 0)
            en_chip = 1;
    }

    int hallA, hallB, hallC;

    //////////////////////////////////////
    hallA = readA.read();
    hallB = readB.read();
    hallC = readC.read();

    //imposto lo step in base agli hall
    step_number = hallStepRead(hallA,hallB,hallC);
    /////////////////////////////////////

    if ((step_number) == 3 || (step_number) == 4 || (step_number) == 5)
    {
        if (step_number == 5)
        {
            step_number = 1;
            uh_1.write(0.0f);         //DISCONNECTED
            vh_2.write(pwm_positive); //PWM
            wh_3.write(gnd_negative); //GND
            en_1 = 0;                 //DISCONNECTED
            en_2 = 1;                 //PWM
            en_3 = 1;                 //GND
            readFirst = 1;
        }
        else
        {
            step_number = 0;
            //stepForaward(step_number);
            uh_1.write(pwm_positive); //PWM
            vh_2.write(0.0f);         //DISCONNECTED
            wh_3.write(gnd_negative); //GND
            en_1 = 1;                 //PWM
            en_2 = 0;                 //DISCONNECTED
            en_3 = 1;                 //GND
            readFirst = 1;
        }
    }


    //encoder.setAngleFormat(angleMod);

    //int count = 0;
    
    //en_chip = 1;
    //stepForeward(step_number % 6);

    //uint16_t saples[1024]; 

    // Enable timer
    __HAL_RCC_TIM3_CLK_ENABLE();

    set10khz();


    HAL_TIM_Base_Init(&mTimUserHandle);
    HAL_TIM_Base_Start_IT(&mTimUserHandle);

    NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
    NVIC_EnableIRQ(TIM_USR_IRQ);

    tim.start(); //attivo il timer software
    while(1)
    {   
        if (flag_time == 1) //variabile settata nel metodo del timer
        {
            float valueReadPot = valuePot();
            if (valueReadPot >= 0.5f && valueReadPot < 0.6f)
                pwm_positive = valueReadPot;  //lettura del pwm dal potenziometro
            else
                pwm_positive = 0.6f;
            //lettura degli hall
            if (readFirst == 0)
            {
                hallA = readA.read();
                hallB = readB.read();
                hallC = readC.read();
                //imposto lo step in base agli hall
                step_number = hallStepRead(hallA,hallB,hallC);
                stepForaward(step_number);
            }
            flag_time = 0;
            readFirst = 0;
        }
    }
}