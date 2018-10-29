#include "mbed.h"
#include "math.h"
#include "AS5601.h"

// PWM frequency 100kHz
#define PWM_PERIOD 0.00001f

// Internal timer
#define TIM_USR TIM3
#define TIM_USR_IRQ TIM3_IRQn

#define LENGTHVALUE 30 //length array

// AS5601
AS5601 encoder(I2C_SDA, I2C_SCL);

//creating an object of serial class to communicate with PC
Serial pc(SERIAL_TX, SERIAL_RX);


//initial position for each phase
int i = 0;

//array contain 30 value for offset
float averageOffset [LENGTHVALUE];

// Variable to keep angle value from hall sensor
float angle = 0;
// Actual motor step
char step_number = 0;

// PWM duty cycle for motor phase
float pwm_positive = 1.0f;
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

// Potentiometer set offset
AnalogIn pot(PB_1);

//set timer into loop
volatile char flag_time = 0;
unsigned int mTime = 0;

//create istance of Timer
Timer t;
TIM_HandleTypeDef mTimUserHandle;

// Interrupt service routine
void M_TIM_USR_HANDLER() 
{
    if (__HAL_TIM_GET_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE) == SET) 
    {
        __HAL_TIM_CLEAR_FLAG(&mTimUserHandle, TIM_FLAG_UPDATE);
        flag_time = 1;
    }
}

//set different timer software
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

/////////////////////////////////////////////////////////////////////////////////////////


// This function converts to read sensor's angle into 6 electrical positions of a BLDC motor
int stepRead(float position)
{
  int initialStep = -1;
  // Check in which of 6 position the motor is
  if (position <= 8.97f || position > 50.5f)
  {
    initialStep = 5;
  }
  else if (position > 8.35f && position <= 17.49f)
  {
    initialStep = 0;
  }
  else if (position > 16.87f && position <= 26.11f)
  {
    initialStep = 1;
  }
  else if (position > 25.49f && position <= 34.81f)
  {
    initialStep = 2;
  }
  else if (position > 33.84f && position <= 43.2f)
  {
    initialStep = 3;
  }
  if ((position > 42.47f && position < 53.0f) || position <= 0.87f)
  {
    initialStep = 4;
  }
  return initialStep;
}

//read potenziometer for offset
/**
 * value without average -1.68 ~ -1.71
 * value with average -1.89
*/
float readPotenziometer(float lung_phase)
{
  return pot.read() * lung_phase*2 - lung_phase;
}


float readPotenziometerOverlap(float lung_phase)
{
  return pot.read() * lung_phase*2;
}

//This function resolve problem of hysteresis
int phase_optimization(float position)
{
  //This is length of phase
  float lengthPhase = (51.43/6);
  //setup offset (range is from -1... to 8....)
  float offset = readPotenziometer(lengthPhase);//-1.71;
  //Read correct angle from encoder
  int tempStep = stepRead(position);
  
  //AVERAGE----------------------------------------------------------------
  /*int count = 0;
  int j = 0;
  float sum = 0.0f;

  while(count < LENGTHVALUE) //Read potenziometer LENTHVALUE times 
  {
    count++;
    averageOffset[j] = readPotenziometer(lengthPhase);
    sum += averageOffset[j]; //sum of different value 
    j++;
  }

  offset = sum/LENGTHVALUE; //average of LENGTHVALUE into array
  */
  //----------------------------------------------------------------------


  //Initial position for each spire
  float initialPosition = offset + lengthPhase * i;
  /*
  * value = 1.1 -> this value is obtained from algoritm with encoder
  * value = 0.1 -> this value is obtained from algoritm with encoder
  * */


  //upper margin
  float overlap = lengthPhase*1.1; //0.0f
  //lower margin
  float overlapDown = lengthPhase*0.1; //0.0f

  /*
  //overlap con potenziometro
  float overlapPotenziometer = readPotenziometerOverlap(lengthPhase);
  overlap = overlapPotenziometer;
  overlapDown = overlapPotenziometer;
  */
  //check position that encoder is reading is in the middle or slightly phase. 
  //Overlap means that it's occupy a certain area of ​​the phase
  if (position >=  initialPosition + overlap) 
  {
    //check position that at the moment it's reading by encoder and check that this position is correct with
    //position that i set
    if (tempStep == step_number)
      step_number = (step_number + 1)%6; //set manual step_number
    else
      step_number = tempStep; //otherwise you will step number with the position where I have to go
                              //tempStep is the reading deriving from the encoder
  }
  //The same algoritm is obtained in case engine is under the middle of phase.
  else if (position < initialPosition - overlapDown)
  {
    if (tempStep == step_number)
      step_number = (step_number + 1)%6;
    else
      step_number = tempStep;
  }
  //set i-esima position with step_number
  //this variable set at the begining of each phase
  i = (step_number)%6;

  return step_number;
}

// This function applies the 6-step algorithm to run the motor
void step_forward(int numberstep)
{
  // Enable motor driver chip
  if (!en_chip)
  {
    en_chip = 1;
  }

  // Switch through step values to activate correct phases
  // For each step we have one phase with positive voltage PWM, one phase connected to ground and one phase disconnected
  switch (numberstep)
  {
  case 3:
    uh_1.write(pwm_positive); //PWM
    vh_2.write(0.0f);         //DISCONNECTED
    wh_3.write(gnd_negative); //GND
    en_1 = 1;                 //PWM
    en_2 = 0;                 //DISCONNECTED
    en_3 = 1;                 //GND
    break;

  case 2:
    uh_1.write(0.0f);         //DISCONNECTED
    vh_2.write(pwm_positive); //PWM
    wh_3.write(gnd_negative); //GND
    en_1 = 0;                 //DISCONNECTED
    en_2 = 1;                 //PWM
    en_3 = 1;                 //GND
    break;

  case 1:
    uh_1.write(gnd_negative); //GND
    vh_2.write(pwm_positive); //PWM
    wh_3.write(0.0f);         //DISCONNECTED
    en_1 = 1;                 //GND
    en_2 = 1;                 //PWM
    en_3 = 0;                 //DISCONNECTED
    break;

  case 0:
    uh_1.write(gnd_negative); //GND
    vh_2.write(0.0f);         //DISCONNECTED
    wh_3.write(pwm_positive); //PWM
    en_1 = 1;                 //GND
    en_2 = 0;                 //DISCONNECTED
    en_3 = 1;                 //PWM
    break;

  case 5:
    uh_1.write(0.0f);         //DISCONNECTED
    vh_2.write(gnd_negative); //GND
    wh_3.write(pwm_positive); //PWM
    en_1 = 0;                 //DISCONNECTED
    en_2 = 1;                 //GND
    en_3 = 1;                 //PWM
    break;

  case 4:
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

// This is the main function
int main()
{
  en_chip = 1; // Enable motor driver chip
  float position = 0.0f;


  // Set PWM period and initializes enables pins
  uh_1.period(PWM_PERIOD);
  vh_2.period(PWM_PERIOD);
  wh_3.period(PWM_PERIOD);
  en_1 = 0;
  en_2 = 0;
  en_3 = 0;

  // Enable timer
  __HAL_RCC_TIM3_CLK_ENABLE();
  //Set timer software
  set10khz();

  HAL_TIM_Base_Init(&mTimUserHandle);
  HAL_TIM_Base_Start_IT(&mTimUserHandle);
  NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
  NVIC_EnableIRQ(TIM_USR_IRQ);

  t.start();

  //Read angle from encoder for initial
  angle = encoder.get_angle_degrees();
  //Position modulated
  position = fmod(angle,51.43f);
  //Initial position on first spire
  i = stepRead(position);

  //set initial step
  step_number = stepRead(position);

  //encoder.write_ABN(1024);
  float lengthPhase = (51.43/6);
  while (true)
  {
    //this flag set timer
    if (flag_time == 1)
    {
      flag_time = 0;
      angle = encoder.get_angle_degrees();
      position = fmod(angle,51.43f);

      //Return phase from position
      int stepPhase = phase_optimization(position);
      //set step of motor
      step_forward(stepPhase);
    }
  }
}