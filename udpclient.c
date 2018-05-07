/* udp_client.c */ 
/* Project 2: Frank Hulmes and Michael Meehan */
/* May 8 2018 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, sendto, and recvfrom */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 1024
#define MAX_SIZE 80

/* Struct and Function Declarations */
struct segment{
    unsigned short packet_sequence;
    unsigned short msg_len;
    char data[STRING_SIZE];
};

struct ACK{
    unsigned short number;
};

int SimulateLoss(double packet_rate){
    double val = (double)rand() / (double)RAND_MAX;
    if (val < packet_rate){
        return 1;
    }
    else {
        return 0;
    }
}

int SimulateAckLoss(double ack_rate){
    double val = (double)rand() / (double)RAND_MAX;
    if (val < packet_rate){
        return 1;
    }
    else {
        return 0;
    }
}





int main(void) {

    int sock_client;  /* Socket used by client */ 

    struct sockaddr_in client_addr;  /* Internet address structure that
                                            stores client address */
    unsigned short client_port;  /* Port number used by client (local port) */

    struct sockaddr_in server_addr;  /* Internet address structure that
                                            stores server address */
    struct hostent * server_hp;      /* Structure to store server's IP
                                            address */
    char server_hostname[STRING_SIZE]; /* Server's hostname */
    unsigned short server_port;  /* Port number used by server (remote port) */

    char sentence[STRING_SIZE];  /* send message */
    char modifiedSentence[STRING_SIZE]; /* receive message */
    unsigned int msg_len;  /* length of message */
    int bytes_sent, bytes_recd; /* number of bytes sent or received */
    int packet_count, ack_count, duplicates, lost_packets, lost_acks, succ_packets, succ_acks, data_delivered, data_recieved;
    int expected_seq;
    double packet_loss_rate;
    double ack_loss_rate;
    char line[MAX_SIZE];
    struct segment send_message;
    struct segment recv_message;
    struct ACK send_ack;

    
    
    /* open a socket */

    if ((sock_client = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
        perror("Client: can't open datagram socket\n");
        exit(1);
    }

    /* Note: there is no need to initialize local client address information
                unless you want to specify a specific local port.
                The local address initialization and binding is done automatically
                when the sendto function is called later, if the socket has not
                already been bound. 
                The code below illustrates how to initialize and bind to a
                specific local port, if that is desired. */

    /* initialize client address information */

    client_port = 0;   /* This allows choice of any available local port */

    /* Uncomment the lines below if you want to specify a particular 
                local port: */
    /*
    printf("Enter port number for client: ");
    scanf("%hu", &client_port);
    */

    /* clear client address structure and initialize with client address */
    memset(&client_addr, 0, sizeof(client_addr));
    client_addr.sin_family = AF_INET;
    client_addr.sin_addr.s_addr = htonl(INADDR_ANY); /* This allows choice of
                                            any host interface, if more than one 
                                            are present */
    client_addr.sin_port = htons(client_port);

    /* bind the socket to the local client port */

    if (bind(sock_client, (struct sockaddr *) &client_addr,
                                        sizeof (client_addr)) < 0) {
        perror("Client: can't bind to local address\n");
        close(sock_client);
        exit(1);
    }

    /* end of local address initialization and binding */

    /* initialize server address information */

    printf("Enter hostname of server: ");
    scanf("%s", server_hostname);
    if ((server_hp = gethostbyname(server_hostname)) == NULL) {
        perror("Client: invalid server hostname\n");
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

    /* user interface */

    printf("Please enter the filename:\n");
    scanf("%s", filename);
    msg_len = strlen(filename) + 1;

    /* Prepare the segment to be sent */
    send_message.packet_sequence = htons(0);
    send_message.msg_len = htons(msg_len);
    strcopy(send_message.data, filename);
    bytes_sent = sendto(sock_client, &send_message, sizeof(send_message), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
    /* Open the output file */

    FILE* output = fopen("out.txt", "w");

    /* Set the first expected packet number */
    expected_seq = 0;
    packet_count = 0;
    ack_count = 0;
    duplicates = 0;
    lost_packets = 0;
    lost_acks = 0;
    succ_packets = 0;
    succ_acks = 0;
    data_delivered = 0;
    data_recieved = 0;
    /* Reciever Loop: Wait for call from above */ 
    
    for ( ; ; ){
        /* Wait for Packet to arrive */
        bytes_recd = recvfrom(sock_client, &recv_message, MAX_SIZE + 4, 0, (struct sockaddr *) 0, (int *) 0);
        // Process Packet
        recv_message.packet_sequence  = ntohs(recv_message.packet_sequence); 
        recv_message.msg_len  = ntohs(recv_message.msg_len);
        packet_count++;
        data_recieved += recv_message.msg_len;
        // Check if EOT or null packet 
        if (recv_message.msg_len == 0 || bytes_recd <= 0){
            // EOT Packet, break loop
            break;
        }
        // Simulate loss, if 0 the packet is recieved normally
        if(SimulateLoss(packet_loss_rate) == 0){
            // The packet is the correct number, deliver normally and return an ACK of the same seq no
            if(recv_message.packet_sequence == expected_seq){
                fputs(line, output);
                send_ack.number = htons(expected_seq);
                // Update the next sequence number
                expected_seq = 1 - expected_seq;
                printf("Packet %d recieved with %d data bytes\n", packet_count, recv_message.msg_len);
                succ_packets++;
                data_delivered += recv_message.msg_len;
            }
            // Packet is incorrect number, return ACK of 1 - expected number
            else{
                duplicates++;
                send_ack.number = htons(1 - expected_seq);
                printf("Duplicate packet %d recieved with %d data bytes\n", packet_count, recv_message.msg_len);
            }

            ack_count += 1;
            // Simulate ack loss, if 0 the ACK is sent
            if(SimulateAckLoss(ack_loss_rate) == 0){
                bytes_sent = sendto(sock_client, &send_ack, sizeof(send_ack), 0, (struct sockaddr *) &server_addr, sizeof(server_addr));
                printf("Ack %d Transmitted", ack_count);
                succ_acks++;                
            }
            else{
                // Ack is lost
                lost_acks++;
                printf("Ack %d Lost", ack_count);
            }
            
        }
        else{
            // Packet is Lost
            printf("Packet %d Lost", packet_count);
        }
    }
    /* EOT Packet was recieved, print stats */
    printf("End of Transmission packet %d recieved with %d data bytes\n", packet_count, recv_message.msg_len);
    printf("%d Packets recieved successfully", packet_count);
    printf("Total Data Delivered: %d  bytes", packet_count);
    printf("Total Data Recieved: %d  bytes", packet_count);
    printf("%d Duplicate Packets", packet_count);
    printf("%d Packets Lost", packet_count);
    printf("Total Packets Recieved: %d", packet_count);
    printf("%d Acks Transmitted", packet_count);
    printf("%d Acks Dropped", packet_count);
    printf("Total ACKs generated: %d", packet_count);

    /* close the socket */

    close (sock_client);
}
