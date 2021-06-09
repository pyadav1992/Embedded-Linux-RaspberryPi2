/*
 * 
 * File: thermostat.c
 * 
 * This files creates a cooling stage of the thermostat. Cooler is turned on
 * when the temperature goes above a set point and an alarm is alerted when
 * the temperature goes beyond a limit.
 * 
 * This is fork() and posix threads based multitasking implementation.
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
#include <pthread.h>

#include "driver.h"
#include "thermostat.h"
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

int running = 1;

unsigned int setpoint, limit, deadband;

u_int8_t alarm_action;   // variable to track alarm actions
u_int8_t cooler_action;  // variable to track cooler actions

/*
    Signal handler to stop the program gracefully
*/
void done (int sig)
{
    running = 0;
}

int value;
int main (int argc, char *argv[])
{
  int fd;
  unsigned int wait, sample = 0;
  alarm_action = NO_ACTION;
  cooler_action = NO_ACTION;
  setpoint=65, limit=95, deadband=1, value=0;
  // Share memory between parent and child process
  int shared_mem[2];
  pipe(shared_mem);

  signal (SIGINT, done);  // set up signal handler
  if (argc > 1)           // get wait time
      sscanf (argv[1], "%d", &wait);
  else
      wait = 2;

  // Initialize LEDs
  if (init_leds(0) < 0)
  {
    printf ("Couldn't initialize LEDs\n");
  return -1;
  }
  // Initialize ADC
  if ((fd = init_AD (0)) < 0)
  {
      printf ("Couldn't initialize A/D converter\n");
      exit (2);
  }
  // Monitor user input - This is a posix thread that is implemented in
  // monitor.c file. The user can modify 3 parameters with the commands:
  // s <nn> -- change setpoint
  // l <nn> -- change limit
  // d <nn> -- change deadband
  createThread ();

  int pid = fork();
  switch(pid)
  {
  case -1:
    printf("fork failed\n");
    break;
  case 0:  // child process - Alarm actions
    while (running)
    {
      if (read_AD (fd, &value) == 0)
      {
        static u_int8_t state_alarm = NORMAL;
        int temperature_alarm = value;
        static u_int8_t led_state = 0;
        // Child process wants to read shared_mem from the parent process
        int internal_limit;
        close(shared_mem[1]);
        read(shared_mem[0], &internal_limit, sizeof(internal_limit));
        //No need of mutex here - getting data from the parent process
        switch(state_alarm)
        {
          case NORMAL:
            alarm_action = NO_ACTION;
            // Alarm action
            if(temperature_alarm>internal_limit)
            {
              alarm_action = ALARM_ON;
              state_alarm = LIMIT;
            }
            break;

          case HIGH:
            alarm_action = NO_ACTION;
            //Alarm action
            if(temperature_alarm>internal_limit)
            {
              alarm_action = ALARM_ON;
              state_alarm = LIMIT;
            }
            break;

          case LIMIT:
            //Alarm action
            if(temperature_alarm<internal_limit)
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
          sleep(1);
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
        int temperature_cooler = value;
        //get exclusive access to parameters
        pthread_mutex_lock (&paramMutex);
        // Parent wants to write shared_mem for the child process
        close(shared_mem[0]);
        write(shared_mem[1], &limit, sizeof(limit));
        switch(state_cooler)
        {
          case NORMAL:
            cooler_action = NO_ACTION;
            // Cooler action
            if(temperature_cooler>(setpoint+deadband))
            {
              cooler_action = COOLER_ON;
              state_cooler = HIGH;
            }
            break;

          case HIGH:
            //Cooler action
            if(temperature_cooler<(setpoint-deadband))
            {
              cooler_action = COOLER_OFF;
              state_cooler = NORMAL;
            }
            break;

          case LIMIT:
            // Cooler action
            if(temperature_cooler<(setpoint-deadband))
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
        
        // release the parameters
        pthread_mutex_unlock (&paramMutex);
        
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
  }
  // Terminate the posix thread
  terminateThread ();
  return 0;
}
