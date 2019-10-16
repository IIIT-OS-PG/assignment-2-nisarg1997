


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
#include <string>
#include <dirent.h>

using namespace std;
struct trackerdara{ 
    string socketdata;
    string trackerdara;
};
map<string, string> userfiles;
bool downloadflag;
vector<string> stringProcessing(string command, char delimeter)
{
    vector<string> temptokens;
    string token="";
    for(unsigned int i=0;i<command.length();i++)
    {
        char ch=command[i];
        if(ch=='\\')
        {
            i++;
            token += command[i];
        }
        else if(ch==delimeter)
        {
            temptokens.push_back(token);
            token="";
        }
        else{
            token += ch;
        }
    }
    temptokens.push_back(token);
    return temptokens;
}


string download (vector<string> tokens) {

    string A=tokens[0];

    auto itr = userfiles.find(tokens[1]);
        if(itr!=userfiles.end()){
            A=A+" "+tokens[1]+" "+itr->second;
        }

        else{
            A = "no file like that in the goup";
        }

        downloadflag=1;
        
        cout << A << endl;
        // cout << "haha : "<< A << endl;
        return A;
}


string upload(vector<string> tokens) {
   // cout << "haha" << endl ;
    string A=tokens[1];
    userfiles[A] = tokens[3];
     auto it=userfiles.find(A) ;
   //  cout << it->second << endl;

   cout << "A : " <<  it->second << endl;
     downloadflag=0;
    return tokens[3];
}

void * server( void *socket_desc){
    
     
    

     char buffer[512 * 1024 ]={0};
    int new_socket = *(int *)socket_desc;
    // int n=0;
    while (1){
        int filesize;
    
    
    
    recv(new_socket ,&filesize,sizeof(filesize),0);
    memset (buffer, '\0', 512*32);
    recv( new_socket  , buffer ,512*32, 0); 
    string complexdata =""; 
    string command = "";
    // printf("%s\n",buffer); 

    //printf("%s\n",to_string(buffer[512*1024]).c_str()); 

    command=command+string((char*)buffer);
    printf("%s\n",command.c_str());
    vector<string> tokens = stringProcessing(command,' ');
    printf("thi is the command : %s\n",tokens[0].c_str());
    if (tokens[0] == "upload")
            {
                if (tokens.size() != 4)
                {
                    cout << "INVALID_ARGUMENTS --- SHARE Command" << endl;
                    continue;
                }
              
                complexdata = complexdata+upload(tokens);
                
                if (complexdata == "-1")
                    continue;
            }
    
            else if (tokens[0] == "download")
            {

                if (tokens.size() != 2)
                {
                    cout << "INVALID_ARGUMENTS --- SHARE Command" << endl;
                    continue;
                }
               complexdata = download(tokens);
               
               if (complexdata == "-1")
                    continue;
            }
            else {

                cout << "haha" << endl;
                break;
            }
	
    
   
   if (downloadflag==1){
    
            char *clientreply = new char[complexdata.length() + 1];
            strcpy(clientreply, complexdata.c_str());
            cout<<"clien nt request to tracker\n" ; 

            int size=strlen(clientreply);
          int n =   send(new_socket,&size,sizeof(size),0);
     //       cout << n << endl; 
           int s=  send(new_socket, clientreply, strlen(clientreply), 0);\
       //     cout << s << endl;
         
   }
    

    
}
close(new_socket );
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