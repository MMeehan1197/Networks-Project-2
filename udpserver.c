/* udp_server.c */
/* Programmed by Adarsh Sethi */
/* February 21, 2018 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 46920

struct header{
   unsigned short packet_sequence;
   unsigned short msg_len;
   char data[80];
};

int main(void) {

   int sock_server;  /* Socket on which server listens to clients */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char sentence[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
      perror("Server: can't open datagram socket\n");
      exit(1);
   }

   /* initialize server address information */

   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
   server_port = SERV_UDP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address\n");
      close(sock_server);
      exit(1);
   }

   /* wait for incoming messages in an indefinite loop */

   printf("Waiting for incoming messages on port %hu\n\n", 
                           server_port);

   client_addr_len = sizeof (client_addr);

   for (;;) {
      /* receive the message */
     
      bytes_recd = recv(sock_connection, &packetheader, sizeof(packetheader), 0);

      /* Process the Header */

      packetheader.packet_sequence  = ntohs(packetheader.packet_sequence);
      packetheader.msg_len  = ntohs(packetheader.msg_len);
      int filename_size = packetheader.msg_len;

      bytes_recd = recv(sock_connection, filename, filename_size, 0);
      /* Opening the file */

      FILE *file = fopen(filename, "r");

      int count, c ;    /* The number of characters in the line and the current character */
      char* current_line;    /* Buffer for the line of characters */
      short packet_sequence = packetheader.packet_sequence;
      short msg_len = packetheader.msg_len;
      count = 0;
      for( ; ; ){
	/* Get the next line in the file */
	while(fgets(current_line, sizeof(current_line), file) != NULL){
	   packet_sequence += 1;
	   msg_len = strlen(current_line);
	   packetheader.packet_sequence = htons(packet_sequence);
	   packetheader.msg_len = htons(msg_len);
	   bytes_sent = send(sock_connection, &packetheader, sizeof(packetheader), 0);
           bytes_sent = send(sock_connection, current_line, msg_len, 0);
           printf("Packet %d transmitted with %d data bytes\n", packet_sequence, msg_len);
	}
	fclose(file);
	break;
      }

     /* Sending End of transmission packet */

     packetheader.packet_sequence += 1;
     packetheader.msg_len = 0;
     bytes_sent = send(sock_connection, &packetheader, 8, 0);

      /* close the socket */

      close(sock_connection);    
   }
}
