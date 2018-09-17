#include "mbed.h"
#include "math.h"
#include "AS5601.h"

// Internal timer
#define TIM_USR         TIM3
#define TIM_USR_IRQ     TIM3_IRQn

//creating an object of serial class to communicate with PC
Serial pc(SERIAL_TX, SERIAL_RX);

// AS5601
AS5601 encoder(I2C_SDA, I2C_SCL);

//button
DigitalIn btn1(USER_BUTTON);

// Variable to keep angle value from hall sensor
float angle = 0;
// Actual motor step
int step_number = 0;
// Angle value scaled by electrical poles
float position = 0;
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
DigitalOut en_chip(PA_6);
// Potentiometer read
AnalogIn potem(PB_1);
//DigitalOut led1(LED1);

volatile char flag_time = 0;
volatile float power = 1; 
const float updelta = 0.6f;
volatile float fase = 0;
Timer t;


volatile float angle;
volatile bool angleChanged;
volatile float pot_value;

TIM_HandleTypeDef mTimUserHandle;

// Interrupt service routine
void M_TIM_USR_HANDLER() {
    if (__HAL_TIM_GET_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE) == SET) {
        __HAL_TIM_CLEAR_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE);
        flag_time = 1;
    }
}



//imposto la fase
float get_phase(float angle) 
{
    float mean = 8.576008;
    float value = 354.287109;
    float phase = (value - angle) / mean;
    phase += 6;
    phase = fmodf(phase + 1, 6);
    return phase;
}

//uso l'interpolazione
float interpolateup(float x) 
{
    if (x < updelta) {
        return x / updelta * (power-0.2f) + 0.2f;
    }
    return power;
}

float interpolatedown(float x) 
{
    // return interpolateup(1.0f-x);
    return power;
}

//6-step
void set_engine(float phase) 
{
    phase = fmodf(phase + 1, 6);
    float phase_status = fmodf(phase, 1);

    switch ((int)phase) {
    case 0:
        en_1 = 1;
        en_2 = 0;
        en_3 = 1; 
        uh_1 = interpolatedown(phase_status); 
        wh_3 = 0;
        break;
    case 1:
        en_1 = 0; 
        en_2 = 1;
        en_3 = 1; 
        vh_2 = interpolateup(phase_status);
        wh_3 = 0;
        break;
    case 2:
        en_1 = 1; 
        en_2 = 1;
        en_3 = 0; 
        uh_1 = 0;
        vh_2 = interpolatedown(phase_status);
        break;
    case 3:
        en_1 = 1; 
        en_2 = 0;
        en_3 = 1; 
        uh_1 = 0;
        wh_3 = interpolateup(phase_status);
        break;
    case 4:
        en_1 = 0; 
        en_2 = 1;
        en_3 = 1; 
        vh_2 = 0;
        wh_3 = interpolatedown(phase_status);
        break;
    case 5:
        en_1 = 1; 
        en_2 = 1;
        en_3 = 0; 
        vh_2 = 0;
        uh_1 = interpolateup(phase_status);
        break;
    }
}

// This is the main function
int main()
{
  /**
   * 
   * TIMER
   * 
  */

 // Enable timer
  __HAL_RCC_TIM3_CLK_ENABLE();

  HAL_TIM_Base_Init(&mTimUserHandle);
  HAL_TIM_Base_Start_IT(&mTimUserHandle);
  NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
  NVIC_EnableIRQ(TIM_USR_IRQ);

  t.start();

  //imposto la frequenza delle fasi
  uh_1.period(0.00001f);
  uh_1.period(0.00001f);
  uh_1.period(0.00001f);

  while(true)
  {
    float tmpAngle = encoder.get_angle_degrees();
    angle = tmpAngle;
    angleChanged = true;

    //set pwm value with potenziometer
    pwm_positive = potem.read();
    float correctAngle = angle + pwm_positive * 20;
    fase = get_phase(fmodf(correctAngle,360));

    set_engine(fase);
  }
}
