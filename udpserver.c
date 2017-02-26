
/* -------------------------------tcpserver.c------------------------------------
 Author : Koundiya 
 -----------------------------------------------------------------------------
 A simple UDP echo server that listens on a specific port for connections

 

 getaddrinfo---->socket---->bind----->recvfrom---->sendto


 USAGE : server <port number>

 -------------------------------------------------------------------------------
 Acknowledgement : Some part of source code taken from  | http://beej.us/guide |
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
#define QUE_LENGTH 10


// Error Handler
void error(char *msg) 
{
    perror(msg);
    exit(0);
}

// Returns the in_addr or in6_addr struct
void *get_in_addr(struct sockaddr *client_sa)
{
    if (client_sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)client_sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)client_sa)->sin6_addr);
}


int main(int argc, char **argv) 
{
    char *port_num;

    int host_socket_fd;

    int error_num_ga, error_num_bind;
    
    int num_data_recvd,num_data_sent;

    struct addrinfo host_sockspecs,*host_addrinfo;
    struct sockaddr_storage client_sockaddr;
    
    int client_sockaddr_len;

    void *ipaddr;

    char client_hostname[NI_MAXHOST],client_ipaddress[INET6_ADDRSTRLEN], client_port[NI_MAXSERV];
    char *client_ipaddress_version;

    char transmit_buff[BUFSIZE],receive_buff[BUFSIZE];


    

    // Check command line arguments
    if (argc != 2)
    {
       fprintf(stderr,"Usage: %s <port>\n", argv[0]);
       return 1;
    }

   
    // Get the port number for Server to listen for connections  
    port_num = argv[1];

    
    // Initialize read/write buffer
    memset(transmit_buff,0,BUFSIZE);
    memset(receive_buff,0,BUFSIZE);

    // Initialize the addrinfo struct : server_sockspecs 
    memset(&host_sockspecs,0,sizeof(host_sockspecs));
    
    client_sockaddr_len = sizeof(client_sockaddr);

    // Set the Host Socket Specifications
    host_sockspecs.ai_family   = AF_UNSPEC;       // Use any available host address :IPv4 or IPv6
    host_sockspecs.ai_socktype = SOCK_DGRAM;      // UDP
    host_sockspecs.ai_flags    = AI_PASSIVE;      // System finds IP address of Host  
    //host_sockspecs.ai_protocol = 0;             // Default


   
    // Populate the addrinfo(struct) of the host socket with the above specs 
    error_num_ga = getaddrinfo(NULL,port_num,&host_sockspecs,&host_addrinfo);
    if(error_num_ga!=0)
    {
        fprintf(stderr,"ERROR - Unable to get host address info : %s\n",gai_strerror(error_num_ga));
        return 1;
    }

    // Open socket on the Host
    host_socket_fd = socket(host_addrinfo->ai_family,host_addrinfo->ai_socktype,host_addrinfo->ai_protocol);
    if (host_socket_fd < 0) 
        error("ERROR - Unable to open socket");
    
 
   // Workaround for socket availability immediately after killing server
    int optval = 1;
    setsockopt(host_socket_fd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));


    // Bind the socket to the entered port
     error_num_bind = bind(host_socket_fd,host_addrinfo->ai_addr,host_addrinfo->ai_addrlen);
     if(error_num_bind<0)
        error("ERROR - Unable to bind socket to given port");
   
    
    // Listen for connections
     
    
     printf("Listening on port %s ...\n",port_num);

     while(1)
     {
      
      // Read message from client
      num_data_recvd = recvfrom(host_socket_fd, receive_buff, BUFSIZE,0,(struct sockaddr *)&client_sockaddr,&client_sockaddr_len);
      if (num_data_recvd < 0) 
       error("ERROR - Receiving message from client");
      
       printf("Received %d bytes of data : %s from the: ",num_data_recvd, receive_buff);
       // Determine Client IP info 
       if (getnameinfo((struct sockaddr *)&client_sockaddr,client_sockaddr_len,client_hostname,NI_MAXHOST,client_port,NI_MAXSERV,NULL))
        printf("Could not resolve host name \n");
       else
       {
        inet_ntop(client_sockaddr.ss_family,get_in_addr((struct sockaddr*)&client_sockaddr),client_ipaddress,INET6_ADDRSTRLEN);
        printf("(%s) %s:%s \n",client_hostname,client_ipaddress,client_port);
       } 
        

      // ECHO the message to the Client
       printf("Data echo : %s ,%d",receive_buff,num_data_recvd);
      num_data_sent = sendto(host_socket_fd,receive_buff,num_data_recvd,0,(struct sockaddr *)&client_sockaddr,client_sockaddr_len);
      if (num_data_sent < 0) 
      error("ERROR - sending message to client");

      

     }


}

