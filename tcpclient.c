/* tcp_ client.c */ 
/* Programmed by Adarsh Sethi */
/* February 21, 2018 */     

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024

struct header {
   unsigned short packet_sequence;
   unsigned short msg_len;
};

int main(void) {

   int sock_client;  /* Socket used by client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   char server_hostname[STRING_SIZE]; /* Server's hostname */
   unsigned short server_port;  /* Port number used by server (remote port) */

   char filename[STRING_SIZE];  /* send message */
   char modifiedSentence[STRING_SIZE]; /* receive message */
   unsigned int msg_len;  /* length of message */                      
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
   short packet_sequence; /* Packet Sequence number */
  
   /* open a socket */

   if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
      perror("Client: can't open stream socket");
      exit(1);
   }

   /* Note: there is no need to initialize local client address information 
            unless you want to specify a specific local port
            (in which case, do it the same way as in udpclient.c).
            The local address initialization and binding is done automatically
            when the connect function is called later, if the socket has not
            already been bound. */

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", server_hostname);
   if ((server_hp = gethostbyname(server_hostname)) == NULL) {
      perror("Client: invalid server hostname");
      close(sock_client);
      exit(1);
   }

   printf("Enter port number for server: ");
   scanf("%hu", &server_port);

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(server_port);

    /* connect to the server */
 		
   if (connect(sock_client, (struct sockaddr *) &server_addr, 
                                    sizeof (server_addr)) < 0) {
      perror("Client: can't connect to server");
      close(sock_client);
      exit(1);
   }
  
   /* user interface */

   printf("Please enter the filename:\n");
   scanf("%s", filename);
   msg_len = strlen(filename) + 1;

   /* Prepare the header to be sent */
   struct header packetheader;
   packetheader.packet_sequence = htons(0);
   packetheader.msg_len = htons(msg_len);

   /* send message */
   bytes_sent = send(sock_client, &packetheader, sizeof(packetheader), 0);
   bytes_sent = send(sock_client, filename, msg_len, 0);
  /* Open the output file */

   FILE* output = fopen("out.txt", "w");

   /* get response from server */ 
   char line[80];
   for ( ; ; ){
      bytes_recd = recv(sock_client, &packetheader, sizeof(packetheader), 0);
      if (bytes_recd <= 0){
	  break;
      }
      packetheader.packet_sequence  = ntohs(packetheader.packet_sequence); 
      packetheader.msg_len  = ntohs(packetheader.msg_len); 
      int line_size = packetheader.msg_len;
      if (line_size == 0){
	  break;
      }
      bytes_recd = recv(sock_client, line, line_size, 0);
      printf("%s",line);
      if (bytes_recd <= 0){
          break;
      }
      fputs(line, output); 
 
 
}

   /* close the socket */

   close (sock_client);
}
