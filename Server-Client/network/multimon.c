/*
 * multimon.c
 *
 * Created on: May 23, 2020
 * Author: pratik yadav
 * 
 * A Posix thread to monitor console input for parameter changes
 * Also includes functions to create and terminate the thread called
 * from main() in the thermostat.c file
 * 
 * Server is created to respond to the thermostat parameter queries. Client can
 * use the following query commands to request thermostat parameters from the
 * server:
 * 
 * "? s" : Thermostat setpoint.
 * "? l" : Thermostat limit.
 * "? d" : Thermostat deadband.
 * "? t" : Thermostat temperature.
 * 
 * The client can also set the following parameter with these commands:
 * 
 * "s #" : set the setpoint to new value #
 * "l #" : set the limit to new value #
 * "d #" : set the deadband to new value #
 * "q"   : close socket 
 *
 * There are resources to support multiple monitor servers. 
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
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "thermostat.h"

#define BUFLEN 80
#define NUM_THREADS 10

typedef enum
{
  FREE,
  IN_USE,
  PENDING
}used; // monitor thread status flag

typedef struct
{
  used flag;
  int socket;
  pthread_t *thread;
} meta_pthread_t;

// Threads
pthread_t createServerT;
pthread_t monitorT[NUM_THREADS];
pthread_t resourceT;

// Mutexes
pthread_mutex_t paramMutex;
pthread_mutex_t paramMutex2;

meta_pthread_t mon_thread_data[NUM_THREADS];
//==============================================================================
// resource function
//==============================================================================
void *resource(void *arg)
{
	meta_pthread_t *p_mon_thread_data;
	void *thread_val;
	int i;
	while(1)
	{
		// pthread_mutex_lock (&paramMutex2);   //get exclusive access to parameters
	    // Get the monitor threads meta data
		p_mon_thread_data = (meta_pthread_t*)arg;
	    // Check for monitor threads with pending flags
		for(i=0; i<NUM_THREADS; i++)
		{
			if (p_mon_thread_data[i].flag == PENDING)
			{
				// Cancel the corresponding monitor thread
				pthread_cancel (*(p_mon_thread_data[i].thread));
				// Join the corresponding monitor thread
				pthread_join (*(p_mon_thread_data[i].thread), &thread_val);
			    // Change the pending flag to free
				p_mon_thread_data[i].flag = FREE;
				printf("Flags free\n");
			}
		}
    // pthread_mutex_unlock (&paramMutex2);   // release the parameters
    // Sleep
    sleep(5);
	}
	return NULL;
}
//==============================================================================
// resource function: End
//==============================================================================

//==============================================================================
// monitor function
//==============================================================================
void *monitor (void *arg)
{
  char text[BUFLEN], text2[BUFLEN], *cmd, *cmd2,*query_param;
  unsigned int value1;
  const char *delims = " \t,=\n";
  int write_status, read_status;
  // int len;

  meta_pthread_t *p_mon_thread_data;
  p_mon_thread_data = (meta_pthread_t *)arg;
  while (1)
  {
    // fgets (text, BUFLEN, stdin);
    read_status = read (p_mon_thread_data->socket, &text, BUFLEN);
    if (read_status < 0)
        perror ("read");
    else
    {
      printf("%s", text);
      cmd = strtok (text, delims);
      printf("%s\n", cmd);
      while (cmd)
      {
          // value1 = atoi (strtok (NULL, delims));
          // This step was casuing segmentation fault, I commented it out for
          // debugging purposes. using raw value to parse the command
          if(*cmd == '?')
          {
              query_param = strtok (NULL, delims);
              printf("%s\n", query_param);
          }
          else if(*cmd == 'q')
          {
              printf("Client is terminating.\n");
          }
          else
          {
              cmd2 = strtok (NULL, delims);
              value1 = atoi (cmd2);
              printf("%s\n", cmd2);
          }
          pthread_mutex_lock (&paramMutex); //get exclusive access to parameters
          switch (*cmd)
          {
            case 's':
                setpoint = value1;
                printf("Socket in monitor task: %d\n", p_mon_thread_data->socket);
                write_status = write (p_mon_thread_data->socket, "SERVER> OK", 10);
                if(write_status<0)
                    perror("write");
                printf("write status: %d\n", write_status);
                break;
            case 'l':
                limit = value1;
                write (p_mon_thread_data->socket, "SERVER> OK", 10);
                break;
            case 'd':
                deadband = value1;
                write (p_mon_thread_data->socket, "SERVER> OK", 10);
                break;
            case 'q':
                // pthread_mutex_lock (&paramMutex2);   //get exclusive access to parameters
                p_mon_thread_data->flag = PENDING;
                close(p_mon_thread_data->socket);
                pthread_mutex_unlock (&paramMutex);     // release the parameters
                return NULL;
                // pthread_cancel (*(p_mon_thread_data->thread));
                // exit(1); // Terminate and exit the running monitor thread
                // pthread_mutex_unlock (&paramMutex2);   // release the parameters
                break;
            case '?':
                switch (*query_param) // Third character is thermostat  query paramter
                {
                  case 's':
                      sprintf (text2, "SERVER> %d", setpoint);
                      write (p_mon_thread_data->socket, text2, strlen (text2));
                      break;
                  case 'l':
                      sprintf (text2, "SERVER> %d", limit);
                      write (p_mon_thread_data->socket, text2, strlen (text2));
                      break;
                  case 'd':
                      sprintf (text2, "SERVER> %d", deadband);
                      write (p_mon_thread_data->socket, text2, strlen (text2));
                      break;
                  case 't':
                      sprintf (text2, "SERVER> %d", value);
                      write (p_mon_thread_data->socket, text2, strlen (text2));
                      break;
                }
                break;
                
            default:
                break;
          }
          pthread_mutex_unlock (&paramMutex);     // release the parameters

          cmd = strtok (NULL, delims);

          sleep(1);
      }
    }
  }
  return NULL;
}
//==============================================================================
// monitor function: End
//==============================================================================

//==============================================================================
// createServer funtion: Assignment 5
//==============================================================================
void *createServer(void *arg)
{
  int server_socket, client_len;
  struct sockaddr_in server_addr, client_addr;
  int result;
  int iter = 0;

  int j;
  for (j=0; j<NUM_THREADS; j++)
  {
    mon_thread_data[j].flag = 0;
    mon_thread_data[j].socket = 0;
    mon_thread_data[j].thread = 0;
  }

  // Create unnamed socket and give it a "name"
  server_socket = socket (PF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  result = inet_aton (SERVER, &server_addr.sin_addr);
  if (result == 0)
  {
    printf ("inet_aton failed\n");
    // exit (1);
    return NULL;
  }
  server_addr.sin_port = htons (PORT);

  // Bind to the socket
  result = bind (server_socket, (struct sockaddr *) &server_addr, sizeof (server_addr));
  if (result != 0)
  {
    perror ("bind");
    // exit (1);
    return NULL;
  }

  // Create a client queue
  result = listen (server_socket, 1);
  if (result != 0)
  {
    perror ("listen");
    // exit (1);
    return NULL;
  }
  printf ("Network server running\n");

  // Accept a connection
  while(1)
  {
    // pthread_mutex_lock (&paramMutex2);   //get exclusive access to parameters
    client_len = sizeof (client_addr);
    mon_thread_data[iter].socket = accept (server_socket, (struct sockaddr *) &client_addr, (socklen_t * __restrict__)&client_len);

    printf ("Connection established to %s\n", inet_ntoa (client_addr.sin_addr));
    printf ("socket in create server task: %d\n", mon_thread_data[iter].socket);
    // Populate the monitor thread meta data
    mon_thread_data[iter].flag = IN_USE;
    mon_thread_data[iter].thread = &monitorT[iter];
    // pthread_mutex_unlock (&paramMutex2);   // release the parameters
    // Create monitor thread
    result = pthread_create (&monitorT[iter], NULL, monitor, (void *)&mon_thread_data[iter]);
    iter++;
    printf("iter's new value: %d\n",iter);
    if(iter==NUM_THREADS)
    {
      iter = 0;
      printf("Maximum clients reached, restarting from 0\n");
    }
  }
  return NULL;
}
//==============================================================================
// createServer function: End
//==============================================================================

#define CHECK_ERROR if (error) { \
        printf ("%s\n", strerror (error)); \
        return 1; }

/*
    Creates the mutex and starts up the create server thread
    Create the Posix objects
*/
int createThread ()
{
  int error;
  // Init mutex for setpoint, deadband and limit
  error = pthread_mutex_init (&paramMutex, NULL);
  CHECK_ERROR;
  // init mutex for monitor threads meta data
  error = pthread_mutex_init (&paramMutex2, NULL);
  CHECK_ERROR;
  // Create server thread
  error = pthread_create (&createServerT, NULL, createServer, NULL);
  CHECK_ERROR;
  // Resource thread
  error = pthread_create (&resourceT, NULL, resource, (void *)&mon_thread_data);
  CHECK_ERROR;

  return 0;
}

/*
    Cancel and join the createServerT thread
*/
void terminateThread (void)
{
	void *thread_val;
  // Terminate createServer thread
  pthread_cancel (createServerT);
  pthread_join (createServerT, &thread_val); // This was monitorT
  // Terminate resource thread
  pthread_cancel (resourceT);
  pthread_join (resourceT, &thread_val); // This was monitorT
}


