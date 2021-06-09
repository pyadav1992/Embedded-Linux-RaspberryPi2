/*
 * File: thermostat.c
 *
 * This files creates a cooling stage of the thermostat. Cooler is turned on
 * when the temperature goes above a set point and an alarm is alerted when
 * the temperature goes beyond a limit.
 *
 * This is fork() based multitasking implementation.
 * 
 * Written by: Pratik yadav
 *   
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  
 * If not, see <https://www.gnu.org/licenses/>    
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "driver.h"

#include "libmc-pcf8591.h"
#include "libmc-gpio.h"

/*
    Thermostat application definitions
*/
#define NO_ACTION   0
#define COOLER_ON   1
#define COOLER_OFF  2
#define ALARM_ON    3
#define ALARM_OFF   4

// Thermostat states
#define NORMAL      10
#define HIGH        11
#define LIMIT       12

//LEDs for cooler and alarm
#define ALARM_LED_ON    ledON(ALARM)
#define ALARM_LED_OFF   ledOFF(ALARM)
#define COOLER_LED_ON   ledON(COOLER)
#define COOLER_LED_OFF  ledOFF(COOLER)

// Deadband temperature
#define DEADBAND  1

// LIMIT_TEMPERATURE
#define LIMIT_TEMP  95

int running = 1;

u_int8_t setpoint = 65;  // Setpoint - hardcoded for now
u_int8_t alarm_action;   // variable to track alarm actions
u_int8_t cooler_action;  // variable to track cooler actions

void done (int sig)
/*
    Signal handler to stop the program gracefully
*/
{
    running = 0;
}

int main (int argc, char *argv[])
{
    int  value, fd;
    unsigned int wait, sample = 0;
    alarm_action = NO_ACTION;
    cooler_action = NO_ACTION;

    signal (SIGINT, done);  // set up signal handler
    if (argc > 1)           // get wait time
        sscanf (argv[1], "%d", &wait);
    else
        wait = 2;

    // Initialize LEDs
    if (init_leds(0) < 0)
    {
      printf ("Child process: couldn't initialize LEDs\n");
    return -1;
    }
    
    // Initialize ADC
    if ((fd = init_AD (0)) < 0)
    {
        printf ("Child process:  couldn't initialize A/D converter\n");
        exit (2);
    }

    int pid = fork();
    switch(pid)
    {
    case -1:
      printf("fork failed\n");
      break;
      
    case 0:  // child process - Alarm actions
        while (running)
        {
          sleep(1);
          if (read_AD (fd, &value) == 0)
          {
            static u_int8_t state_alarm = NORMAL;
            u_int8_t temperature_alarm = value;
            static u_int8_t led_state = 0;
            
            switch(state_alarm)
            {
              case NORMAL:
                alarm_action = NO_ACTION;
                // Alarm action
                if(temperature_alarm>LIMIT_TEMP)
                {
                  alarm_action = ALARM_ON;
                  state_alarm = LIMIT;
                }
                break;

              case HIGH:
                alarm_action = NO_ACTION;
                //Alarm action
                if(temperature_alarm>LIMIT_TEMP)
                {
                  alarm_action = ALARM_ON;
                  state_alarm = LIMIT;
                }
                break;

              case LIMIT:
                //Alarm action
                if(temperature_alarm<LIMIT_TEMP)
                {
                  alarm_action = ALARM_OFF;
                  state_alarm = HIGH;
                }
                break;
              default:
                state_alarm = NORMAL;
                alarm_action = NO_ACTION;
                break;
            }
            
            // Alarm Action
                if(alarm_action == ALARM_ON)
            {
                    if(led_state == 0)
                    {
                  ALARM_LED_ON;
                led_state = 1;
                printf ("Child process: Alarm on.\n");
                    }
                    else
                    {
                ALARM_LED_OFF;
                led_state = 0;
                    }
            }
                else if(alarm_action == ALARM_OFF)
            {
                ALARM_LED_OFF;
                led_state = 0;
                printf ("Child process: Alarm off.\n");
            }
          }
        }

        // Unexport the leds and ADC
        close_leds ();
        close_AD (fd);
        printf ("Child process exit. \n");
      break;
      
    default: // parent process - Cooler actions
        while (running)
        {
          sleep (wait);
          if (read_AD (fd, &value) == 0)
          {
            printf ("Parent process: Sample %d = %d\n", sample, value);
            sample++;
            static u_int8_t state_cooler = NORMAL;
            u_int8_t temperature_cooler = value;
            switch(state_cooler)
            {
              case NORMAL:
                cooler_action = NO_ACTION;
                // Cooler action
                if(temperature_cooler>(setpoint+DEADBAND))
                {
                  cooler_action = COOLER_ON;
                  state_cooler = HIGH;
                }
                break;

              case HIGH:
                //Cooler action
                if(temperature_cooler<(setpoint-DEADBAND))
                {
                  cooler_action = COOLER_OFF;
                  state_cooler = NORMAL;
                }
                break;

              case LIMIT:
                // Cooler action
                if(temperature_cooler<(setpoint-DEADBAND))
                {
                  cooler_action = COOLER_OFF;
                  state_cooler = NORMAL;
                }
                break;
              default:
                state_cooler = NORMAL;
                cooler_action = NO_ACTION;
                break;
            }
            // cooler actions
            switch(cooler_action)
            {
              case NO_ACTION:
                break;
              case COOLER_ON:
                COOLER_LED_ON;
                printf ("Parent process: Cooler on.\n");
                break;
              case COOLER_OFF:
                COOLER_LED_OFF;
                printf ("Parent process: Cooler off.\n");
                break;
            }
          }
        }
        printf ("Parent process exit. \n");
        // Unexport the leds and ADC
        close_leds ();
        close_AD (fd);
      break;
    }

    return 0;
}
