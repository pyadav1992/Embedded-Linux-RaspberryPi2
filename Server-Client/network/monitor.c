/*
 * 
 * File: monitor.c
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

const char *delims = " \t,=\n";

#define BUFLEN 80

#define CHECK_ERROR if (error) { \
        printf ("%s\n", strerror (error)); \
        return 1; }
        
//==============================================================================
// createServer funtion: Assignment 5
//==============================================================================
int createServer()
{
  int server_socket, client_socket, client_len;
  struct sockaddr_in server_addr, client_addr;
  int result;

  // Create unnamed socket and give it a "name"
  server_socket = socket (PF_INET, SOCK_STREAM, 0);
  server_addr.sin_family = AF_INET;
  result = inet_aton (SERVER, &server_addr.sin_addr);
  if (result == 0)
  {
    printf ("inet_aton failed\n");
    exit (1);
  }
  server_addr.sin_port = htons (PORT);

  // Bind to the socket
  result = bind (server_socket, (struct sockaddr *) &server_addr, sizeof (server_addr));
  if (result != 0)
  {
    perror ("bind");
    exit (1);
  }

  // Create a client queue
  result = listen (server_socket, 1);
  if (result != 0)
  {
    perror ("listen");
    exit (1);
  }
  printf ("Network server running\n");

  // Accept a connection
  client_len = sizeof (client_addr);
  client_socket = accept (server_socket, (struct sockaddr *) &client_addr, (socklen_t * __restrict__)&client_len);

  printf ("Connection established to %s\n", inet_ntoa (client_addr.sin_addr));

  return client_socket;
}
//==============================================================================
// createServer function: End
//==============================================================================

pthread_mutex_t paramMutex;
pthread_t monitorT;

void *monitor (void *arg)
{
  char text[BUFLEN], *cmd, text2[BUFLEN], *query_param;
  unsigned int value1;
  // int len;

  int client_socket = createServer();
  if(client_socket  == -1)
  {
    perror("Accept");
    exit(1);
  }

  while (1)
  {
      // fgets (text, BUFLEN, stdin);
    read (client_socket, &text, BUFLEN);

    cmd = strtok (text, delims);

    while (cmd)
    {
      if(*cmd == '?')
      {
        query_param = strtok (NULL, delims);
      }
      else
      {
        value1 = atoi (strtok (NULL, delims));
      }
      pthread_mutex_lock (&paramMutex);   //get exclusive access to parameters 
      switch (*cmd)
      {
         case 's':
              setpoint = value1;
              write (client_socket, "SERVER> OK", 10);
              break;
              
          case 'l':
              limit = value1;
              write (client_socket, "SERVER> OK", 10);
              break;
              
          case 'd':
              deadband = value1;
              write (client_socket, "SERVER> OK", 10);
              break;
              
          case '?':
            switch (*query_param) // Third character is thermostat  query paramter
            {
              // Considering setpoint, limit, deadband, temp to be 3
              // digit maximum
              case 's':
                  sprintf (text2, "SERVER> %d", setpoint);
                  write (client_socket, text2, strlen (text2));
                break;
              case 'l':
                  sprintf (text2, "SERVER> %d", limit);
                  write (client_socket, text2, strlen (text2));
                break;
              case 'd':
                  sprintf (text2, "SERVER> %d", deadband);
                  write (client_socket, text2, strlen (text2));
                break;
              case 't':
                  sprintf (text2, "SERVER> %d", value);
                  write (client_socket, text2, strlen (text2));
                break;
        }
        break;
          
        default:
            break;
      }
      pthread_mutex_unlock (&paramMutex);     // release the parameters
      
      cmd = strtok (NULL, delims);
    }
  }
  return NULL;
}

/*
    Creates the mutex and starts up the monitor thread
    Create the Posix objects
*/
int createThread ()
{
  int error;
  error = pthread_mutex_init (&paramMutex, NULL);
  CHECK_ERROR;
  
  error = pthread_create (&monitorT, NULL, monitor, NULL);
  CHECK_ERROR;
  
  return 0;
}

void terminateThread (void)
/*
    Cancel and join the monitor thread
*/
{
	void *thread_val;
  pthread_cancel (monitorT);
  pthread_join (monitorT, &thread_val);
}
