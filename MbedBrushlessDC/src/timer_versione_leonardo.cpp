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

// Variable to keep angle value from hall sensor
float angle = 0;
// Actual motor step
char step_number = 0;
// Angle value scaled by electrical poles
float position = 0;
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
AnalogIn ain(PB_1);
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

void read_potentiometer()
{
  pwm_positive = ain.read();
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
void stepRead()
{
  // Check in which of 6 position the motor is
  if (position <= 8.97f || position > 50.5f)
  {
    step_number = 5; //5
  }
  else if (position > 8.35f && position <= 17.49f)
  {
    step_number = 0; //4
  }
  else if (position > 16.87f && position <= 26.11f)
  {
    step_number = 1; //3
  }
  else if (position > 25.49f && position <= 34.81f)
  {
    step_number = 2; //2
  }
  else if (position > 33.84f && position <= 43.2f)
  {
    step_number = 3; //1
  }
  if ((position > 42.47f && position < 53.0f) || position <= 0.87f)
  {
    step_number = 4; //0
  }
}

// This function applies the 6-step algorithm to run the motor
void step_forward()
{
  // Enable motor driver chip
  if (!en_chip)
  {
    en_chip = 1;
  }

  // Read step
  stepRead();

  // Switch through step values to activate correct phases
  // For each step we have one phase with positive voltage PWM, one phase connected to ground and one phase disconnected
  switch (step_number)
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

  // Set PWM period and initializes enables pins
  uh_1.period(PWM_PERIOD);
  vh_2.period(PWM_PERIOD);
  wh_3.period(PWM_PERIOD);
  en_1 = 0;
  en_2 = 0;
  en_3 = 0;

  // Set timer 7 values to work at 200us
  // instance indica l'indirizzo del registro base, ovvero, del timer che vogliamo utilizzare in base al processore che stiamo adoperando
  //mTimUserHandle.Instance = TIM_USR;

  // Sotto struttura init
  /*
        Prescaler: valore per il quale viene diviso il tim clock.
        CounterMode: modalità di incremento del count specifica se contare per ogni fronte di salita o di discesa (?)
        Period: Specifica il valore del periodo che deve essere caricato dentro il registro auto-reload e viene caricato al prossimo evento di update.

        Freq = 1 kHz = 90000000 / 900 / (99+1)
     */
  /*mTimUserHandle.Init.Prescaler = 450 - 1; //4799;
  mTimUserHandle.Init.CounterMode = TIM_COUNTERMODE_UP;
  mTimUserHandle.Init.Period = 100 - 1;
  mTimUserHandle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
*/

  // Enable timer
  __HAL_RCC_TIM3_CLK_ENABLE();

  set10khz();


  HAL_TIM_Base_Init(&mTimUserHandle);
  HAL_TIM_Base_Start_IT(&mTimUserHandle);
  NVIC_SetVector(TIM_USR_IRQ, (uint32_t)M_TIM_USR_HANDLER);
  NVIC_EnableIRQ(TIM_USR_IRQ);

  t.start();

  int t1=t.read_us(), t2=0;

  // Main loop
  while (true)
  {
    if (flag_time >= 1)
    {
      flag_time = 0;
      // Read hall sensor's angle
      angle = encoder.get_angle_degrees();

      // Read position from hall sensor's angle divided by electrical poles
      position = fmod(angle, 51.43f); //360/7, where 7 is number of magnets divided by 2
      //pc.printf("%f\n",position);

      // Read the potentiometer value and set the duty cycle
      read_potentiometer();

      // Checks step value and eventually runs motor
      step_forward();
      //pc.printf("%i\n",step_number);
      //pc.printf("%f\n",position);
      t2=t.read_us();
      

      /*if((t2-t1)>897) //f401re --> 897/898 f446re --> 851
        led1=1;
      else
        led1=0;
      t1=t2;*/
    }
  }
}