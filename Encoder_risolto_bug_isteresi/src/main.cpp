#include "mbed.h"
#include "math.h"
#include "AS5601.h"

// PWM frequency 100kHz
#define PWM_PERIOD 0.00001f

// Internal timer
#define TIM_USR         TIM3
#define TIM_USR_IRQ     TIM3_IRQn

// AS5601
AS5601 encoder(I2C_SDA, I2C_SCL);

//creating an object of serial class to communicate with PC
Serial pc(SERIAL_TX, SERIAL_RX);


//variabile
int i = 0;

// Variable to keep angle value from hall sensor
float angle = 0;
// Actual motor step
char step_number = 0;
// Angle value scaled by electrical poles
//float position = 0;
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
// Potentiometer read
AnalogIn pot(PB_1);
DigitalOut led1(LED1);

volatile char flag_time = 0;
unsigned int mTime = 0;

Timer t;

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


// This function converts hall sensor's angle into 6 electrical positions of a BLDC motor
int stepRead(float position)
{
  int initialStep = -1;
  // Check in which of 6 position the motor is
  if (position <= 8.97f || position > 50.5f)
  {
    initialStep = 5; //5
  }
  else if (position > 8.35f && position <= 17.49f)
  {
    initialStep = 0; //4
  }
  else if (position > 16.87f && position <= 26.11f)
  {
    initialStep = 1; //3
  }
  else if (position > 25.49f && position <= 34.81f)
  {
    initialStep = 2; //2
  }
  else if (position > 33.84f && position <= 43.2f)
  {
    initialStep = 3; //1
  }
  if ((position > 42.47f && position < 53.0f) || position <= 0.87f)
  {
    initialStep = 4; //0
  }
  return initialStep;
}

//int lastPosition = position; //ultima posizione salvata in base agli angoli

float readPotenziometer(float lung_phase)
{
  return pot.read() * lung_phase*2 - lung_phase;
}



int phase_optimization(float position)
{
  //lunghezza di ogni singola fase
  float lengthPhase = (51.43/6);//(360.0f/6);
  //mediana di ogni singola fase (quando sono qui sono neutro)
  float offset = readPotenziometer(lengthPhase);

  int tempStep = stepRead(position);
  
  //posizione di ogni spira
  float initialPosition = offset + lengthPhase * i;

  if (position >=  initialPosition + lengthPhase*1.1) //significa che sono verso dx
  {
    if (tempStep == step_number)
      step_number = (step_number + 1)%6; //attivo la spira dopo
    else
      step_number = tempStep;
  }
  else if (position < initialPosition - lengthPhase*0.1)
  {
    if (tempStep == step_number)
      step_number = (step_number + 1)%6; //attivo la spira dopo
    else
      step_number = tempStep;
  }

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

  //encoder.write_ABN(8);

  // Set PWM period and initializes enables pins
  uh_1.period(PWM_PERIOD);
  vh_2.period(PWM_PERIOD);
  wh_3.period(PWM_PERIOD);
  en_1 = 0;
  en_2 = 0;
  en_3 = 0;

  // Enable timer
  __HAL_RCC_TIM3_CLK_ENABLE();

  set10khz();

  HAL_TIM_Base_Init(&mTimUserHandle);
  HAL_TIM_Base_Start_IT(&mTimUserHandle);
  NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
  NVIC_EnableIRQ(TIM_USR_IRQ);

  t.start();
  //step_number = 0;

  //leggo l'angolo dall'encoder
  angle = encoder.get_angle_degrees();
  //ottengo la posizione con il modulo
  //position = fmod(angle,360.0f);
  position = fmod(angle,51.43f);
  //chiamo lo stepRead per ottenere il primo step_number del motore
  i = stepRead(position); //setto il primo step_number

  step_number = stepRead(position);

  /*pc.printf("Angolo iniziale: %f\n",position);

  pc.printf("Fase iniziale: %i\n",step_number);

  pc.printf("Fase in cui mi trovo: %i\n",i);
  */

  encoder.write_ABN(1024);

  while (true)
  {
    if (flag_time == 1)
    {
      flag_time = 0;

      //pwm_positive = pot.read();

      angle = encoder.get_angle_degrees();

      position = fmod(angle,51.43f);

      //pc.printf("%f\n",position);

      int stepPhase = phase_optimization(position);
      //pc.printf("Fase in cui mi trovo: %i\n",i);

      step_forward(stepPhase);
      //pc.printf("%i\n",stepPhase);
    }
  }
}