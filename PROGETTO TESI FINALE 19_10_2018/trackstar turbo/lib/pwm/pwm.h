/* mbed Microcontroller Library
    2  * Copyright (c) 2006-2013 ARM Limited
    3  *
    4  * Licensed under the Apache License, Version 2.0 (the "License");
    5  * you may not use this file except in compliance with the License.
    6  * You may obtain a copy of the License at
    7  *
    8  *     http://www.apache.org/licenses/LICENSE-2.0
    9  *
   10  * Unless required by applicable law or agreed to in writing, software
   11  * distributed under the License is distributed on an "AS IS" BASIS,
   12  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   13  * See the License for the specific language governing permissions and
   14  * limitations under the License.
   15  */
    #ifndef MBED_PWMOUT_H
    #define MBED_PWMOUT_H
    
    #include "platform/platform.h"
    
    #if defined (DEVICE_PWMOUT) || defined(DOXYGEN_ONLY)
    #include "hal/pwmout_api.h"
    #include "platform/mbed_critical.h"
    #include "platform/mbed_power_mgmt.h"
    
    namespace mbed {
    /** \addtogroup drivers */
    
    /** A pulse-width modulation digital output
   30  *
   31  * @note Synchronization level: Interrupt safe
   32  *
   33  * Example
   34  * @code
   35  * // Fade a led on.
   36  * #include "mbed.h"
   37  *
   38  * PwmOut led(LED1);
   39  *
   40  * int main() {
   41  *     while(1) {
   42  *         led = led + 0.01;
   43  *         wait(0.2);
   44  *         if(led == 1.0) {
   45  *             led = 0;
   46  *         }
   47  *     }
   48  * }
   49  * @endcode
   50  * @ingroup drivers
   51  */
    class PwmOut {
    
    public:
    
        /** Create a PwmOut connected to the specified pin
   57      *
   58      *  @param pin PwmOut pin to connect to
   59      */
        PwmOut(PinName pin) : _deep_sleep_locked(false)
        {
            core_util_critical_section_enter();
            pwmout_init(&_pwm, pin);
            core_util_critical_section_exit();
        }
    
        ~PwmOut()
        {
            core_util_critical_section_enter();
            unlock_deep_sleep();
            core_util_critical_section_exit();
        }
    
        /** Set the ouput duty-cycle, specified as a percentage (float)
   75      *
   76      *  @param value A floating-point value representing the output duty-cycle,
   77      *    specified as a percentage. The value should lie between
   78      *    0.0f (representing on 0%) and 1.0f (representing on 100%).
   79      *    Values outside this range will be saturated to 0.0f or 1.0f.
   80      */
        void write(float value)
        {
            core_util_critical_section_enter();
            lock_deep_sleep();
            pwmout_write(&_pwm, value);
            core_util_critical_section_exit();
        }
    
        /** Return the current output duty-cycle setting, measured as a percentage (float)
   90      *
   91      *  @returns
   92      *    A floating-point value representing the current duty-cycle being output on the pin,
   93      *    measured as a percentage. The returned value will lie between
   94      *    0.0f (representing on 0%) and 1.0f (representing on 100%).
   95      *
   96      *  @note
   97      *  This value may not match exactly the value set by a previous write().
   98      */
        float read()
      {
           core_util_critical_section_enter();
           float val = pwmout_read(&_pwm);
           core_util_critical_section_exit();
           return val;
       }
   
       /** Set the PWM period, specified in seconds (float), keeping the duty cycle the same.
  108      *
  109      *  @param seconds Change the period of a PWM signal in seconds (float) without modifying the duty cycle
  110      *  @note
  111      *   The resolution is currently in microseconds; periods smaller than this
  112      *   will be set to zero.
  113      */
       void period(float seconds)
       {
           core_util_critical_section_enter();
           pwmout_period(&_pwm, seconds);
           core_util_critical_section_exit();
       }
   
       /** Set the PWM period, specified in milli-seconds (int), keeping the duty cycle the same.
  122      *  @param ms Change the period of a PWM signal in milli-seconds without modifying the duty cycle
  123      */
       void period_ms(int ms)
       {
          core_util_critical_section_enter();
           pwmout_period_ms(&_pwm, ms);
           core_util_critical_section_exit();
       }
   
       /** Set the PWM period, specified in micro-seconds (int), keeping the duty cycle the same.
  132      *  @param us Change the period of a PWM signal in micro-seconds without modifying the duty cycle
  133      */
       void period_us(int us)
       {
           core_util_critical_section_enter();
           pwmout_period_us(&_pwm, us);
           core_util_critical_section_exit();
       }
   
      /** Set the PWM pulsewidth, specified in seconds (float), keeping the period the same.
  142      *  @param seconds Change the pulse width of a PWM signal specified in seconds (float)
  143      */
       void pulsewidth(float seconds)
       {
           core_util_critical_section_enter();
           pwmout_pulsewidth(&_pwm, seconds);
           core_util_critical_section_exit();
       }
   
       /** Set the PWM pulsewidth, specified in milli-seconds (int), keeping the period the same.
  152      *  @param ms Change the pulse width of a PWM signal specified in milli-seconds
  153      */
       void pulsewidth_ms(int ms)
       {
           core_util_critical_section_enter();
           pwmout_pulsewidth_ms(&_pwm, ms);
           core_util_critical_section_exit();
       }
   
       /** Set the PWM pulsewidth, specified in micro-seconds (int), keeping the period the same.
  162      *  @param us Change the pulse width of a PWM signal specified in micro-seconds
  163      */
       void pulsewidth_us(int us)
       {
           core_util_critical_section_enter();
           pwmout_pulsewidth_us(&_pwm, us);
           core_util_critical_section_exit();
       }
   
       /** A operator shorthand for write()
  172      *  \sa PwmOut::write()
  173      */
       PwmOut &operator= (float value)
       {
           // Underlying call is thread safe
           write(value);
           return *this;
       }
   
       /** A operator shorthand for write()
  182      * \sa PwmOut::write()
  183      */
       PwmOut &operator= (PwmOut &rhs)
       {
           // Underlying call is thread safe
           write(rhs.read());
           return *this;
       }
   
       /** An operator shorthand for read()
  192      * \sa PwmOut::read()
  193      */
       operator float()
       {
           // Underlying call is thread safe
           return read();
       }
   
   protected:
       /** Lock deep sleep only if it is not yet locked */
       void lock_deep_sleep()
       {
           if (_deep_sleep_locked == false) {
               sleep_manager_lock_deep_sleep();
               _deep_sleep_locked = true;
           }
       }
   
       /** Unlock deep sleep in case it is locked */
       void unlock_deep_sleep()
       {
           if (_deep_sleep_locked == true) {
               sleep_manager_unlock_deep_sleep();
               _deep_sleep_locked = false;
           }
       }
   
       pwmout_t _pwm;
       bool _deep_sleep_locked;
   };
   
   } // namespace mbed
   
   #endif
   
   #endif