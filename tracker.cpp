#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/socket.h>
#include<arpa/inet.h>
#include <signal.h>
#include <netinet/in.h>
#include<netdb.h>
#include<errno.h>
#include<unistd.h>
#include <pthread.h>
#include <dirent.h>

using namespace std;



// void * server( void *socket_desc){
    
//     int new_socket = *(int *)socket_desc;
//     int sock = 0;
//     struct sockaddr_in serv_addr;
//     if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
//     {
//         printf("\n Socket creation error in client side\n");
//         return NULL;
//     }
//     memset(&serv_addr, '0', sizeof(serv_addr));
//     serv_addr.sin_family = AF_INET;
//     serv_addr.sin_port = htons(2000);
//     serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

//     int addrlen=sizeof(serv_addr);

//     bind(sock,(struct sockaddr*)&serv_addr,sizeof(sockaddr));
    
//     int status=listen(sock,5);

//     int client_sockfd=accept(sock,(struct sockaddr*)&serv_addr,(socklen_t*)&addrlen);
//     int filesize;
//     recv(client_sockfd,&filesize,sizeof(filesize),0);
//     FILE *f=fopen("file","wb");

//     char buffer[512]={0};
//     int n=0;
//     while ( ( n = recv( client_sockfd , buffer ,512, 0) ) > 0  && filesize > 0){
	
//     fwrite (buffer , sizeof (char), n, f);
//     memset (buffer, '\0', 512);
//     filesize = filesize - n;
//     } 

//     close(client_sockfd);
//     close(sock);
//     fclose(f);

//     return socket_desc;
// }



int main(int argc,char *argv[])
{

     int new_socket ;
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
        return -1;
    }
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(2000);
    serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

    int addrlen=sizeof(serv_addr);

    bind(sock,(struct sockaddr*)&serv_addr,sizeof(sockaddr));
    
    int status=listen(sock,5);

    int client_sockfd=accept(sock,(struct sockaddr*)&serv_addr,(socklen_t*)&addrlen);
    int filesize;
    recv(client_sockfd,&filesize,sizeof(filesize),0);
    FILE *f=fopen("file","wb");

    char buffer[512]={0};
    int n=0;
    while ( ( n = recv( client_sockfd , buffer ,512, 0) ) > 0  && filesize > 0){
	
    fwrite (buffer , sizeof (char), n, f);
    memset (buffer, '\0', 512);
    filesize = filesize - n;
    } 

    close(client_sockfd);
    close(sock);
    fclose(f);








    // int server_fd, new_socket;
    // int sock = 0;
    // struct sockaddr_in serv_addr;
    // int addrlen= sizeof(serv_addr);
    //  while (1)
    //     {
    //         // pthread_t thread_id;
    //         //  if ((new_socket = accept(server_fd, (struct sockaddr *)&serv_addr, (socklen_t *)&addrlen)) < 0)
    //         // {
    //         //     perror("Error in accept connection");
    //         //     exit(EXIT_FAILURE);
    //         // }

    //         if (pthread_create(&thread_id, NULL, server, (void *)&new_socket) < 0)
    //         {
    //             perror("\ncould not create thread\n");
    //         }
    //     }
    
    return 0;
}
