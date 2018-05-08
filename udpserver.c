/* udp_server.c */
/* Project 2: Frank Hulmes and Michael Meehan */
/* May 8 2018 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include <math.h>	    /* for timeout operations */
#define STRING_SIZE 1024
#define MAX_SIZE 80
#define SEGMENT_SIZE 84

/* SERV_UDP_PORT is the port number on which the server listens for
   incoming messages from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_UDP_PORT 46920

 /* Struct and Function Declarations */

struct segment{
   short unsigned int packet_sequence;
   short unsigned int msg_len;
   char data[STRING_SIZE];
};

struct ack{
      unsigned short number;
};

char * getLine(FILE *filename, char *currentLine){
      // Gets the next line of the file, deals with null terminated strings
      memset(currentLine, '\n', MAX_SIZE);
      return fgets(currentLine, MAX_SIZE + 1, filename);
}


int main(int argc, char **argv) {

      /* Variable Declarations */

      int sock_server;  /* Socket on which server listens to clients */

      struct sockaddr_in server_addr;  /* Internet address structure that
                                          stores server address */
      unsigned short server_port;  /* Port number used by server (local port) */

      struct sockaddr_in client_addr;  /* Internet address structure that
                                          stores client address */
      unsigned int client_addr_len;  /* Length of client address structure */

      struct segment message;  /* receive segment */
      char filename[STRING_SIZE]; /* send message */
      unsigned int msg_len;  /* length of message */
      int bytes_sent, bytes_recd; /* number of bytes sent or received */
      unsigned int i;  /* temporary loop variable */
      int count, c ;    /* The number of characters in the line and the current character */
      char current_line[MAX_SIZE];    /* Buffer for the line of characters */
      struct timeval timeout;
      struct ack ack_rec;
      short int ack_sequence = 0;
      int timeout_n;

      // Default timeouts
      timeout.tv_sec = 10;
      timeout.tv_usec = 0;
      setsockopt(sock_server, SOL_SOCKET, SO_RCVTIMEO, (const void *) &timeout, sizeof(timeout));
      // User set timeout
      if(argc > 1){
	  timeout_n = atoi(argv[1]);
          timeout.tv_sec = floor(pow(10, timeout_n)/pow(10, 6));
          timeout.tv_usec =(int) pow(10, timeout_n)% (int)pow(10, 6);
          setsockopt(sock_server, SOL_SOCKET, SO_RCVTIMEO, (const void *) &timeout, sizeof(timeout));
      }
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
      printf("Timeout value is %d seconds and %d microseconds\n",(int) timeout.tv_sec,(int) timeout.tv_usec);
      printf("Waiting for incoming messages on port %hu\n\n", 
                              server_port);

      client_addr_len = sizeof (client_addr);
      /* Initialize all the statistics counters */
      int packet_count = 0;
      int initial_packets = 0;
      int retransmissions = 0;
      int acks_recieved = 0;
      int data_bytes = 0;
      int timeouts = 0;


for (;;) {
      /* Outer Loop: Wait for call from above */
      for( ; ; ){
            /* receive the message */
            bytes_recd = recvfrom(sock_server, &message, STRING_SIZE + 4, 0, (struct sockaddr *) &client_addr, &client_addr_len);

            /* Process the First Packet */

            message.packet_sequence  = ntohs(message.packet_sequence);
            message.msg_len  = ntohs(message.msg_len);
            int filename_size = message.msg_len;
            strcpy(filename, message.data);

            /* Opening the file */

            FILE *file = fopen(filename, "r");
            short packet_sequence = message.packet_sequence;
            short msg_len = message.msg_len;
            count = 0;

            
            /* Send the first line of the file to transition to next state */
            if(getLine(file, current_line) != NULL){
                  message.packet_sequence = htons(packet_sequence);
                  message.msg_len = htons(msg_len);
                  strcpy(message.data, current_line);
                  bytes_sent = sendto(sock_server, &message, strlen(message.data) + 4, 0, (struct sockaddr *) &client_addr, client_addr_len);
		  packet_count++;
		  data_bytes += msg_len;
                  printf("Packet %d transmitted with %d data bytes\n", packet_sequence, msg_len);
            }
            else{
                  // Close connection if the file does not exist
                  printf("File does not exist, terminating connection");
                  break;
            }
            
            /* Inner loop: Iterating through the lines of the file*/

            for( ; ; ){
		  // Reset Timeout
	          setsockopt(sock_server, SOL_SOCKET, SO_RCVTIMEO, (const void *) &timeout, sizeof(timeout));
                  /* Wait for ACK to send next line */
                  bytes_recd = recvfrom(sock_server, &ack_rec, 2, 0, (struct sockaddr *) &client_addr, &client_addr_len);
		  packet_count++;
		  if (bytes_recd <= 0){
                        // Timeout, resend the line
                        bytes_sent = sendto(sock_server, &message, strlen(message.data) + 4, 0, (struct sockaddr *) &client_addr, client_addr_len);
		        data_bytes += msg_len;
			timeouts++;
			retransmissions++;
			acks_recieved--;
                  }
                  else if (ntohs(ack_rec.number) == ack_sequence){
                        // Correct Ack recieved, transmit the next line
                        if(getLine(file, current_line) != NULL){
                              packet_sequence += 1;
                              ack_sequence = 1 - ack_sequence;
                              msg_len = strlen(current_line);
                              message.packet_sequence = htons(ack_sequence);
                              message.msg_len = htons(msg_len);
                              strcpy(message.data, current_line);
			      data_bytes += msg_len;
                              bytes_sent = sendto(sock_server, &message, msg_len + 4, 0, (struct sockaddr *) &client_addr, client_addr_len);
                              printf("Packet %d transmitted with %d data bytes\n", packet_sequence, msg_len);
                        }
                        else{
                              // End of File, end loop
                              break;
                        }
                  }
		  acks_recieved++;
                  // If incorrect ACK is recieved, do nothing
            }
      	fclose(file);

      	/* Sending End of transmission packet */
	struct segment eot_packet;
      	eot_packet.packet_sequence = htons(packet_sequence + 1);
      	eot_packet.msg_len = htons(0);
      	bytes_sent = sendto(sock_server, &eot_packet, 4, 0, (struct sockaddr *)&client_addr, client_addr_len);
	printf("EOT packet sent with sequence number %d and size %d\n", packet_sequence + 1, 0);
      	/* close the socket */
	printf("-----Server Statistics-----\n");
	printf("Number of packets initially transmitted: %d\n", packet_count - retransmissions);
	printf("Number of data bytes transmitted: %d\n", data_bytes );
	printf("Number of retransmissions: %d\n", retransmissions);
	printf("Total number of packets transmitted: %d\n", packet_count);
	printf("Number of ACKs recieved: %d\n", acks_recieved);
	printf("Number of timeouts: %d\n", timeouts);
      	close(sock_server);
      	return 0; 
      	}
    }
}
