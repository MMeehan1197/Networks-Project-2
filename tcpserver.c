/* tcpserver.c */
/* Programmed by Adarsh Sethi */
/* February 21, 2018 */    

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024   

/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_TCP_PORT 46920

/* Defining the struct for the header */

struct header{
   unsigned short packet_sequence;
   unsigned short msg_len;
};

int main(void) {

   int sock_server;  /* Socket on which server listens to clients */
   int sock_connection;  /* Socket on which server exchanges data with client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   unsigned int server_addr_len;  /* Length of server address structure */
   unsigned short server_port;  /* Port number used by server (local port) */

   struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
   unsigned int client_addr_len;  /* Length of client address structure */

   char filename[STRING_SIZE];  /* receive message */
   char modifiedSentence[STRING_SIZE]; /* send message */
   unsigned int msg_len;  /* length of message */
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   unsigned int i;  /* temporary loop variable */
   struct header packetheader;

   /* open a socket */

   if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Server: can't open stream socket");
      exit(1);                                                
   }

   /* initialize server address information */
    
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */ 
   server_port = SERV_TCP_PORT; /* Server will listen on this port */
   server_addr.sin_port = htons(server_port);

   /* bind the socket to the local server port */

   if (bind(sock_server, (struct sockaddr *) &server_addr,
                                    sizeof (server_addr)) < 0) {
      perror("Server: can't bind to local address");
      close(sock_server);
      exit(1);
   }                     

   /* listen for incoming requests from clients */

   if (listen(sock_server, 50) < 0) {    /* 50 is the max number of pending */
      perror("Server: error on listen"); /* requests that will be queued */
      close(sock_server);
      exit(1);
   }
   printf("I am here to listen ... on port %hu\n\n", server_port);
  
   client_addr_len = sizeof (client_addr);

   /* wait for incoming connection requests in an indefinite loop */

   for (;;) {

      sock_connection = accept(sock_server, (struct sockaddr *) &client_addr, 
                                         &client_addr_len);
                     /* The accept function blocks the server until a
                        connection request comes from a client */
      if (sock_connection < 0) {
         perror("Server: accept() error\n"); 
         close(sock_server);
         exit(-3);
      }
 
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
