
/* -------------------------------emailserver.c------------------------------------
 Author : Koundinya AE13B010
 ---------------------------------------------------------------------------------
 A TCP based  email server that listens on a specific port for connections,and responds
 to request from the email client.

 RUN : ./eserver <port number>
 
 Available requests :
 -------------------
 -LSTU
 -ADDU <user id>
 -USER <user id>
 -READM
 -DELM
 -SEND <recepient id>
 -DONEU
 -QUIT

 -------------------------------------------------------------------------------
 Acknowledgement : Parts of source code(for resolving both IPv4 and IPv6 addresses) taken from  | http://beej.us/guide |
 */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <time.h> 

#define MSGBUFSIZE 1024     // buffersize for reading input messages
#define DATABUFSIZE 102400    // buffersize for sending mail/other data
#define QUE_LENGTH 10       // Max. number of client connections that can be queued



// Data structure for storing User ID and the user mail pointer
// All user mail files are stored with the same name as user ID
struct userData 
{
  char* userId;
  int numOfMails;
  
  
  struct userData *next;
};

struct userData *userData_current = NULL;
struct userData *userData_head = NULL;



// Takes User ID as input and returns a pointer to userData struct
// Returns NULL pointer if user does not exist
struct userData* searchUser(char* uId)
{
  struct userData *uData = userData_head;
  
  if(uData==NULL)
    return NULL;

  while(strcmp(uData->userId,uId)!=0)
  {
    if(uData->next==NULL)
      return NULL;
    else
      uData=uData->next;
  }
  return uData;  
}


// Adds user to the database with User ID as input
// Returns -1 if user already exists,else returns 0 on successful addition 
int addUser(char* uId)
{
  struct userData *uData = malloc(sizeof(struct userData));
  if(searchUser(uId)==NULL)
  {
    uData->userId=strdup(uId); 
    uData->numOfMails=0;
    FILE* mailPointer=fopen(uId,"a+");
    fclose(mailPointer);
    
    uData->next = userData_head;
    userData_head = uData;
    return 1;
  }
  else
    return 0;
}
  

// Returns the list of User ID's registered with server
char* getUserList()
{
  struct userData *uData = userData_head;
  int i=0,j=0;
  char *ulist=malloc(1);

  if(uData==NULL)
    return "No Users";
  
  while(1)
  { 
    i=0;
    while(1)
    {
      if(uData->userId[i]=='\0')
       break;
      else
      {
        if(j!=0)
       ulist=realloc(ulist,j+1); 
       ulist[j++]=uData->userId[i]; 
      }
      i++;
    }

    uData = uData->next;
    
    if(uData==NULL)
      break;
    else
    {
      ulist=realloc(ulist,j+1); 
      ulist[j++]=' ';
    }
  }
  ulist=realloc(ulist,j+1);
  ulist[j]='\0';
  return ulist;
}



// Returns the current mail as char* and advanced the mail file pointer to the next mail
char* getMail(FILE* mailPointer)
{
  int c,c1,c2,i=0;
  char *mail=malloc(1);
  while(1)
  {
    c=fgetc(mailPointer);
    if(c=='#')
    {
      if(((c1=fgetc(mailPointer))=='#')&& ((c2=fgetc(mailPointer))=='#'))
      { 
        break;
      }
      else
      {
        mail=realloc(mail,i+2);
        mail[i++]=c1;
        mail[i++]=c2;
      }
    }
    else if((c!='\n')&&(c!=EOF))
    {
        if(i!=0)    
        mail=realloc(mail,i+1);
        mail[i++]=c;
    }
     if(c==EOF && i==0)
      return "No mails to show";
  }
  mail=realloc(mail,i+1);
  mail[i]='\0';
  return mail;
}

// Returns the total length of the mail spool file
int getMailSpoolLength(FILE* mailPointer)
{
  fseek(mailPointer,0,SEEK_SET);
  fseek(mailPointer,0,SEEK_END);
  int l=ftell(mailPointer);
  return l;
}

// Writes mail to the uId_dst mail spool file 
void sendMail(char* mail,char* uId_src,char* uId_dst)
{
  FILE* fp=fopen(uId_dst,"a+");

  time_t t = time(NULL);
  struct tm tm = *localtime(&t);

  fprintf(fp,"\n %s %s \n %d-%d-%d %d:%d \n %s \n %s","From : ",uId_src,tm.tm_mday,tm.tm_mon,1900+tm.tm_year,tm.tm_hour,tm.tm_min,mail,"###");
  fclose(fp);
  struct userData* uData=searchUser(uId_dst);
  uData->numOfMails+=1;
}


// deletes current mail item being pointed to and advances the pointer to next,returns 1 on successful deletion
// returns 0 if no mails are left/on error
int deleteMail(FILE* mailPointer,char* uId)
{
  char *newmail=malloc(1);
  int c,i=0,onemail_flag=1;
  struct userData* uData=searchUser(uId);
  
  if(uData->numOfMails==0)
    return 0;

  if(strcmp(getMail(mailPointer),"No mails to show")==0) // moves the mail pointer to next mail
    return 0;

  while(1)  // gets the data from the mail spool file, excluding the mail to be deleted and stores in (newmail)
  {
    c=fgetc(mailPointer);
    if(c==EOF)
       break;
    else
    {
      if(i!=0)
      newmail=realloc(newmail,i+1);
      newmail[i]=c;    
    }
    i++;
  } 
   newmail=realloc(newmail,i+1);
   newmail[i]='\0';

   fclose(mailPointer);  //clears the file
   mailPointer=fopen(uId,"w");
   fclose(mailPointer);
   
   mailPointer=fopen(uId,"a+");
   if(i!=0)
   {
    
   fprintf(mailPointer,"%s",newmail); // Copies the content in (newmail) to the empty mail spool file
   fseek(mailPointer,0,SEEK_SET); // move the pointer to beginning of spool file
  }
  // Update number of mails
  
  uData->numOfMails-=1;
  return 1;
}




// Error handler
void error(char *msg) 
{
    perror(msg);
    exit(0);
}


// Returns the in_addr or in6_addr struct
void *get_in_addr(struct sockaddr *client_sa)
{
    if (client_sa->sa_family == AF_INET) 
    {
        return &(((struct sockaddr_in*)client_sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*)client_sa)->sin6_addr);
}


int main(int argc, char **argv) 
{
    char *port_num;

    int host_socket_fd, client_socket_fd;

    int error_num_ga, error_num_bind;
    
    int num_data_rcvd,num_data_sent;

    struct addrinfo host_sockspecs,*host_addrinfo;
    struct sockaddr_storage client_sockaddr;
    
    int client_sockaddr_len;

    void *ipaddr;

    char client_hostname[NI_MAXHOST],client_ipaddress[INET6_ADDRSTRLEN], client_port[NI_MAXSERV];

    char transmit_buff[DATABUFSIZE] , receive_buff[MSGBUFSIZE];
    
    struct userData* current_user;
    char *current_userId=NULL;
    char *userId;
    char request[10];
    FILE* current_MailPointer=NULL;
    int send_flag=0,num_req_arg=0,quit_flag=0;

    // Check command line arguments
    if (argc != 2)
    {
       fprintf(stderr,"Usage: %s <port>\n", argv[0]);
       return 1;
    }

   
    // Get the port number for Server to listen for connections  
    port_num = argv[1];

    
    // Initialize read/write buffers
   
    memset(receive_buff,'\0',MSGBUFSIZE);
    memset(transmit_buff,'\0',DATABUFSIZE);


    // Initialize the addrinfo struct : server_sockspecs 
    memset(&host_sockspecs,0,sizeof(host_sockspecs));
    
    client_sockaddr_len = sizeof(client_sockaddr);

    // Set the Host Socket Specifications
    host_sockspecs.ai_family   = AF_UNSPEC;     // Use any available host address :IPv4 or IPv6
    host_sockspecs.ai_socktype = SOCK_STREAM;   // TCP
    host_sockspecs.ai_flags    = AI_PASSIVE;     // System finds IP address of Host  
    //host_sockspecs.ai_protocol = 0;             // Default


   
    // Populate the addrinfo(struct) of the host socket with the above specs 
    error_num_ga = getaddrinfo(NULL,port_num,&host_sockspecs,&host_addrinfo);
    if(error_num_ga!=0)
    {
        fprintf(stderr,"ERROR - Unable to get client address info : %s\n",gai_strerror(error_num_ga));
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
    if(listen(host_socket_fd,QUE_LENGTH)<0)
        error("ERROR - Unable to listen for connections");
    
    printf("\n Mail server running, listening on port %s ...\n",port_num);

    while(1)
    {

      // Accept any qeued-up connections
      client_socket_fd = accept(host_socket_fd,(struct sockaddr *)&client_sockaddr,&client_sockaddr_len);
      if(client_socket_fd<0)
      error("ERROR - Unable to accept connection from client");

       
      // Determine Client IP info
      getnameinfo((struct sockaddr *)&client_sockaddr,client_sockaddr_len,client_hostname,NI_MAXHOST,client_port,NI_MAXSERV,NULL);
      inet_ntop(client_sockaddr.ss_family,get_in_addr((struct sockaddr*)&client_sockaddr),client_ipaddress,INET6_ADDRSTRLEN);
      printf("\n--- Connection Established with (%s) %s:%s --- \n",client_hostname,client_ipaddress,client_port);
       

      while(1)
      {
        printf("\n$$...");
        fflush(stdout);

        // Read message from client
        memset(receive_buff,0,MSGBUFSIZE);
        while((num_data_rcvd = read(client_socket_fd, receive_buff, MSGBUFSIZE))==0)
        {
        
        }
        if (num_data_rcvd < 0) 
       	 error("ERROR - reading from socket");

        printf("\rReceived %d bytes of data : %s\n",num_data_rcvd, receive_buff);

       /*------------------------------------------Command Processor----------------------------------------------*/

        // Get and process requests from client 
        if(send_flag==0)
        {

        userId = (char *)malloc(strlen(receive_buff)+1);
        memset(request,0,10);

        if((num_req_arg=sscanf(receive_buff,"%s %s",request,userId))==2)
          printf("$$ > %s %s \n",request,userId);
        else
          printf("$$ > %s  \n",request);
        
        }
        else
          printf("(%s)$$ : Message from %s to %s: %s\n",current_userId,current_userId,userId,receive_buff);

        
        // Adds User to database, checks if user is already present
        if(strcmp(request,"ADDU")==0)
        {
          if(num_req_arg==1)
          {
            strcpy(transmit_buff,"Usage :  ADDU <user id>");
            printf("$$ : %s \n",transmit_buff);
          }
          else if((addUser(userId)==1)&&num_req_arg==2)
          {
            printf("$$ Added User : %s \n",userId);
            strcpy(transmit_buff,"Done");
          }
          else
            {
              strcpy(transmit_buff,"User ID already present");
              printf("$$ : %s \n",transmit_buff);
            }
        }
         
        // Logins User and opens mail spool file for reading from the start
        // checks :  -> if user is present
        //           -> if logged in as a different user
        else if(strcmp(request,"USER")==0)
        {
          if(num_req_arg==1)
          {
            strcpy(transmit_buff,"Usage : USER <user id>");
            printf("$$ : %s \n",transmit_buff);
          }
          else if(((current_user=searchUser(userId))==NULL) && num_req_arg==2)
          {
            strcpy(transmit_buff,"User does not exit");
            printf("$$ : %s \n",transmit_buff);
          }
          else if(current_userId!=NULL)
          {
            strcpy(transmit_buff,"Error-logout to change user");
            printf("(%s)$$ : %s \n",current_userId,transmit_buff);
          }
          else if(current_userId==userId)
          {
            strcpy(transmit_buff,"Already logged in");
            printf("(%s)$$ : %s \n",current_userId,transmit_buff );
          }
          else
          {
            current_userId=userId;
            current_MailPointer=fopen(current_userId,"r+");
            fseek(current_MailPointer,0,SEEK_SET);
            sprintf(transmit_buff,"logged in :%i mails",current_user->numOfMails);
            printf("(%s)$$ : %d Mails \n",current_userId,current_user->numOfMails);
          } 
        }


        // Reads and prints the current mail item of logged in user,prints appropriate message if no mails
        // checks :   -> if user is logged in
        else if(strcmp(request,"READM")==0)
        {
          if(current_MailPointer==NULL)
          {
            strcpy(transmit_buff,"Error Reading Mail,login with USER");
            printf("$$ : %s \n",transmit_buff);
          }
          else
          {
            strcpy(transmit_buff,getMail(current_MailPointer));
            printf("(%s)$$ : %s\n",current_userId,transmit_buff);            
          }
        }
        
        // Deletes the current mail item and advances mail pointer to the next
        // checks :     ->if user is logged in
        else if(strcmp(request,"DELM")==0)
        {
          if(current_MailPointer==NULL)
          {
            strcpy(transmit_buff,"Error Deleting Mail,login with USER");
            printf("$$ : %s \n",transmit_buff);
          }
          else
          {
            if(deleteMail(current_MailPointer,current_userId)==1)
            {
              sprintf(transmit_buff,"Deleted Mail, %d mails left in inbox",current_user->numOfMails);
              fseek(current_MailPointer,0,SEEK_SET);
              printf("(%s)$$ : Deleted Mail, %d mails left in inbox \n",current_userId,current_user->numOfMails);            
            }
            else
            {
              strcpy(transmit_buff,"No mails left to delete");
              printf("(%s)$$ : %s ",current_userId,transmit_buff);
            }
          }
        }
            
        // Writes mail to the spool file of recepient
        // checks   :   -> if logged in as any user
        //              -> if recepient/destination user id exists
        else if(strcmp(request,"SEND")==0)
        {
          if(current_userId!=NULL && searchUser(userId)!=NULL)
          {
            if(send_flag==0)
            {
            strcpy(transmit_buff,"Type Message : ");
            send_flag=1;
            }
            else
            {
              sendMail(receive_buff,current_userId,userId);
              send_flag=0;
              strcpy(transmit_buff,"Mail Sent");
              printf("(%s)$$ : Mail sent to %s\n",current_userId,userId);
            }
          }

          else if(current_userId==NULL)
          {
            strcpy(transmit_buff,"Error Sending Mail,login to continue");
            printf("$$ : %s \n",transmit_buff);
          }
          else
          {
            strcpy(transmit_buff,"Error sending Mail : User doensn't exist");
            printf("(%s)$$ : Error sending mail-user doesn't exist\n",current_userId);
          }
        }

        // Prints the list of users registered
        else if(strcmp(request,"LSTU")==0)
        {
          strcpy(transmit_buff,getUserList());
          printf("$$ : %s \n",transmit_buff);

        }

        // logs out the user, terminates session, and initializes the mail pointer
        else if(strcmp(request,"DONEU")==0)
        {
          if(current_userId==NULL)
          {
            strcpy(transmit_buff,"Not loged in as any User");
            printf("$$ : Not logged in as any User \n");
          }
          else
          {
            fseek(current_MailPointer,0,SEEK_SET);
            fclose(current_MailPointer);
            current_userId=NULL;
            current_MailPointer=NULL;
            strcpy(transmit_buff,"Logged out of current session");
            printf("(%s)$$ : User session closed\n",current_userId);
          }
        }
        
        // Disconnects from the client
        else if(strcmp(request,"QUIT")==0)
        {
          quit_flag=1;
          strcpy(transmit_buff,"Connection Closed");
        }

        else
        {
          strcpy(transmit_buff,"Unkown request");
          printf("(%s)$$ : %s\n",current_userId,transmit_buff);
        }
      /*----------------------------------------------------------------------------------------------------------*/

       // Transmit data to client
       num_data_sent = write(client_socket_fd, transmit_buff,strlen(transmit_buff));
       memset(transmit_buff,0,DATABUFSIZE);

       if (num_data_sent< 0) 
        error("ERROR - writing to socket");
       if(quit_flag==1)
        {
        quit_flag=0;
        break;
        }

      }


       close(client_socket_fd);
       printf("-- Disconnected -- \n");    

     }


}

