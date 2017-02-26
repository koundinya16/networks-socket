
/* -------------------------------tcpclient.c---------------------------------------
 Author : Koundinya AE13B010
 ---------------------------------------------------------------------------------
 A simple UDP network client that transmits messages to a server
 


 getaddrinfo---->socket---->sendto----->recvfrom


 USAGE : ./client <hostname/ip> <port number>


 -------------------------------------------------------------------------------
 Acknowledgement : Parts of source code taken from  | http://beej.us/guide |
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define BUFSIZE 1024


// Error handler
void error(char *msg) 
{
    perror(msg);
    exit(0);
}


int main(int argc, char **argv) 
{
    char *hostname, *port_num;

    int socket_fd;

    int error_num_ga;

    int num_data_recvd, num_data_sent;
    
    struct addrinfo server_sockspecs, *server_addrinfo;
    
    void *ipaddr;

    char server_ipaddress[INET6_ADDRSTRLEN];
    char *server_ipaddress_version;

    char transmit_buff[BUFSIZE],receive_buff[BUFSIZE];
    

    // Check command line arguments
    if (argc != 3)
    {
       fprintf(stderr,"Usage: %s <hostname> <port>\n", argv[0]);
       return 1;
    }

   
    // Get the port number and hostname of Server for establshing connection  
    hostname = argv[1];
    port_num = argv[2];

    
    // Initialize read/write buffer
    memset(transmit_buff,0,BUFSIZE);
    memset(receive_buff,0,BUFSIZE);

    // Initialize the addrinfo struct : server_sockspecs 
    memset(&server_sockspecs,0,sizeof(server_sockspecs));
    
    // Filter for the Server socket specifications 
    server_sockspecs.ai_family   = AF_UNSPEC;     // IPv4 or IPv6
    server_sockspecs.ai_socktype = SOCK_DGRAM;    // UDP
    server_sockspecs.ai_protocol = 0;             // Default


   
    // Get the addrinfo(structs) of all the server sockets which match the given specs 
    error_num_ga = getaddrinfo(hostname,port_num,&server_sockspecs,&server_addrinfo);
    if(error_num_ga!=0)
    {
        fprintf(stderr,"ERROR - Unable to get host address info : %s\n",gai_strerror(error_num_ga));
        return 1;
    }

    // Open socket on the client side
    socket_fd = socket(server_addrinfo->ai_family, server_addrinfo->ai_socktype, 0);
    if (socket_fd < 0) 
        error("ERROR - Unable to open socket");
    
 
    // Take User Input
     memset(transmit_buff,0,BUFSIZE);
     printf("Enter message :  ");
     fgets(transmit_buff, BUFSIZE, stdin);
   
    // Establish connection with Server (the first socket from the list of available server sockets is selected)    
    num_data_sent=sendto(socket_fd,transmit_buff,strlen(transmit_buff),0, server_addrinfo->ai_addr,server_addrinfo->ai_addrlen);
    if(num_data_sent<0) 
      error("ERROR - could not send Message");
    
    // Determine server IP address
    if(server_addrinfo->ai_family==AF_INET)
    {
        struct sockaddr_in *ipv4= (struct sockaddr_in *)server_addrinfo->ai_addr;
        ipaddr = &(ipv4->sin_addr);
        server_ipaddress_version="IPv4";

    }
    else
    {
       struct sockaddr_in6 *ipv6= (struct sockaddr_in *)server_addrinfo->ai_addr;
       ipaddr = &(ipv6->sin6_addr);
       server_ipaddress_version="IPv6";
    }

    inet_ntop(server_addrinfo->ai_family,ipaddr,server_ipaddress,INET6_ADDRSTRLEN);
    printf("\n\n Message sent to : %s (%s address) \n",server_ipaddress,server_ipaddress_version);   
     
     
    // Read Message from the Server
    memset(receive_buff,0,BUFSIZE);

    num_data_recvd = recvfrom(socket_fd,receive_buff,BUFSIZE,0, server_addrinfo->ai_addr,&(server_addrinfo->ai_addrlen));
    if (num_data_recvd < 0) 
      error("ERROR - reading from socket");

    
     printf("$$ Server : %s \n",receive_buff);

    memset(receive_buff,0,BUFSIZE);
    
    
    close(socket_fd);
    
    return 0;
}
