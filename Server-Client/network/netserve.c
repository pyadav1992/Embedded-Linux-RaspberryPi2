/*
 *  file:   netserve.c
 * 
 *  Simple sockets server
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
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main (void)
{
  int server_socket, client_socket, client_len;
  struct sockaddr_in server_addr, client_addr;
  char text[80];
  int len, result;

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

  strcpy (text, "Server> ");
  do
  {
    len = read (client_socket, &text[8], sizeof (text)-8);
    write (client_socket, text, len+8);
  }
  while (text[8] != 'q');
  close (client_socket);
  close (server_socket);
  printf ("Connection terminated.  Server shutting down\n");
  return 0;
}

