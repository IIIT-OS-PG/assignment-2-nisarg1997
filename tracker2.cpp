


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

// void *serverservice(void *socket_desc)
// {
//     int new_socket = *(int *)socket_desc;
//     while (1)
//     {
//         int closeflag = 0;
//         char buffer[1024] = {0};
//         int rc = read(new_socket, buffer, 1024);
//         if (rc == 0)
//         {
//             close(new_socket);
//             return socket_desc;
//         }
     

//         // stringstream check2(data);
//         // string intermediate1;
//         // // Tokenizing w.r.t. space '#'
//         // while (getline(check2, intermediate1, '#'))
//         // {
//         //     tokens1.push_back(intermediate1);
//         // }

       
//         //cout<<"serverreply : "<<string(serverreply)<<endl;
//         char *serverreply = new char[clientreplymsg.length() + 1];
//         strcpy(serverreply, clientreplymsg.c_str());
//         //cout<<"serverreply : "<<serverreply<<endl;
//         send(new_socket, serverreply, strlen(serverreply), 0);



//         if (closeflag == 1)
//         {
//             close(new_socket);
//             break;
//         }
//     }

//     return socket_desc;




// using namespace std;
// class socketclass
// {
//   public:
//     char *ip; //IP Address
//     int port; //Port Address

//     socketclass()
//     {
//         // ip="";
//         port = 0;
//     }

//     // socketclass(string ip1, int port1)
//     // {
//     //     ip = ip1;
//     //     port = port1;
//     // }

//     void setsocketdata(string sc)
//     {
//         vector<string> tokens;

//         stringstream check1(sc);

//         string intermediate;

//         // Tokenizing w.r.t. space ' '
//         while (getline(check1, intermediate, ':'))
//         {
//             tokens.push_back(intermediate);
//         }

//         string strip = tokens[0];
//         ip = new char[strip.length() + 1];
//         strcpy(ip, strip.c_str());
//         port = stoi(tokens[1]);
//     }
// };


void * server( void *socket_desc){
    
    int new_socket = *(int *)socket_desc;
    int sock = 0;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error in client side\n");
        return NULL;
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

    return socket_desc;
}


int main(int argc, char *argv[]){
pthread_t thread_id;
       
        int server_fd, new_socket;
        struct sockaddr_in address;
        int opt = 1;
        int addrlen = sizeof(address);

        // Creating socket file descriptor
        if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }

        // Forcefully attaching socket to the port 8080
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                       &opt, sizeof(opt)))
        {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
        address.sin_family = AF_INET;
        address.sin_addr.s_addr =inet_addr("127.0.0.1") ;
      //  cout << inet_addr(trackersocket1.ip); 
          address.sin_port = htons(atoi(argv[1]));
        // Forcefully attaching socket to the port 8080
        if (bind(server_fd, (struct sockaddr *)&address,
                 sizeof(address)) < 0)
        {
            perror("bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(server_fd, 10) < 0)
        {
            perror("listen");
            exit(EXIT_FAILURE);
        }

        while (1)
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("Error in accept connection");
                exit(EXIT_FAILURE);
            }

               if (pthread_create(&thread_id, NULL, server, (void *)&new_socket) < 0)
            {
                perror("\ncould not create thread\n");
            }

        }
        return 0;
}