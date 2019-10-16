
#include<bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <string.h>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <openssl/sha.h>
#include <pthread.h>
#include <dirent.h>

using namespace std;


string clientsocketstr, trackersocket1str, trackersocket2str;
map<string, string> downloadstatus; //to maintain download status
vector<pair<string, string>> clientfilepath;

int chunk_size = 512 * 1024 ; 

string getsmallhash(string A)
{
    string final_ans;
   
        char k [A.size()+1];
        strcpy(k,A.c_str());
         unsigned char hash[SHA_DIGEST_LENGTH];
    char buf[SHA_DIGEST_LENGTH *2 ];

       SHA1((const unsigned char *) k, A.length(), hash );


      
        for (int i = 0; i < SHA_DIGEST_LENGTH; i++) 
            sprintf((char *)&(buf[i* 2]), "%02x", hash[i]);
    
        for (int i = 0; i < 10; i++)
        {
            final_ans +=buf[i];
        }
      //  cout << final_ans;
        //ans= final_ans;
    
    
    return final_ans;
    
}
string calHashofchunk(char *schunk, int length1, bool shorthashflag)
{

    unsigned char hash1[SHA_DIGEST_LENGTH];
    char buf1[SHA_DIGEST_LENGTH * 2];
    SHA1((const unsigned char *)schunk, length1, hash1);

    //printf("\n*****hash ********");
    for (int i = 0; i < SHA_DIGEST_LENGTH; i++)
        sprintf((char *)&(buf1[i * 2]), "%02x", hash1[i]);

    //cout<<"hash : "<<buf<<endl;
    string ans;
  
        for (int i = 0; i < 20; i++)
        {
            ans += buf1[i];
        }
      string final_ans;
  
      if (shorthashflag == 0)
    {
          ans= ans;
    }


    return ans;
}



string getFileHash(char *fpath)
{
    string fileHash;
    ifstream file1(fpath, ifstream::binary);

    /* basic sanity check */
    if (!file1)
    {
        cout << "FILE DOES NOT EXITST : " << string(fpath) << endl;
        return "NULL ";
    }

    struct stat fstatus;
    stat(fpath, &fstatus);

    // Logic for deviding file1 into chunks
    long int total_size = fstatus.st_size;
    long int chunk_size = chunk_size;

    int total_chunks = total_size / chunk_size;
    int last_chunk_size = total_size % chunk_size;

    if (last_chunk_size != 0) // if file1 is not exactly divisible by chunks size
    {
        ++total_chunks; 
    }
    else 
    {
        last_chunk_size = chunk_size;
    }


    for (int chunk = 0; chunk < total_chunks; ++chunk)
    {
        int cur_cnk_size;
        if (chunk == total_chunks - 1)
            cur_cnk_size = last_chunk_size;
        else
            cur_cnk_size = chunk_size;

        char *chunk_data = new char[cur_cnk_size];
        file1.read(chunk_data,   
                   cur_cnk_size); 

        string sh1out = calHashofchunk(chunk_data, cur_cnk_size, 1);
        fileHash = fileHash + sh1out;
    }
    file1.close();

    return fileHash;
}


vector<string> String_Manipulation(string command, char delimeter)
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

string upload(vector<string> tokens,string socket_port) {

    string A;
    string B;
    cout << "got upload request \n";
    B=tokens[0]+" "+tokens[1];

    char s[tokens[1].size()+1];
    strcpy(s,tokens[1].c_str());

    A=getFileHash(s);
  

    string C= getsmallhash(A);
    B=B+" "+C+" "+socket_port;
    printf("%s\n",B.c_str());
    return B;
}

void *Download_Threads(void *socket_desc){
     int download_thread_fd = *(int *)socket_desc;
  //  cout << "asdasd " << endl;
    char filename[100];
     recv(download_thread_fd ,filename,100,0);

    FILE *f=fopen(filename,"w");
    fclose(f);
    f=fopen(filename,"r+");

    if(!f){
        cout << "something wrong with file"<< endl;
    }
    int filesize;
  //  char *Buffer[512]={0};
    int n=0;
     string po = "";
     char BUFFER1[512*1024];
    recv(download_thread_fd ,&filesize,sizeof(filesize),0);
    
      char Buffer[512*32]={0};
     int count=0;
      bool flag;
    while ( ( n = recv( download_thread_fd , Buffer ,512*32 , 0) ) > 0  ){
     //  recv( download_thread_fd , Buffer ,512 * 64, 0);
	cout << Buffer << endl; 
  
    
  //    po=po+string((char*)Buffer);

  //  cout << po <<endl; 
   // cout << n << endl;
     
     int  g= fwrite(Buffer,sizeof(char),n,f); 
        count=count+n;
        if(g<0){
            perror("fwrite");
        }
     //   cout << "M" << g << endl;
     memset (Buffer, '\0', 512*32);
        if(count==filesize){
            break;
        }
     filesize = filesize - n;
    

    } 
       fclose(f);
  //  cout << po << endl;
 
   
  

    return socket_desc;
}

void *Server_Tracker(void *socket_desc){
 
    while(1){
    char buffer[512]={0};
int filesize;
 memset (buffer, '\0', 512);
    int server_thread_fd = *(int *)socket_desc;
     int  n=  recv(server_thread_fd,&filesize,sizeof(filesize),0);
       //      cout << n << endl;
    
        int   k=  recv( server_thread_fd , buffer ,512, 0);
     //      cout << k << endl;
     //   cout << buffer << endl;
   
    string command= "";    
    command=command+string((char*)buffer);
    vector<string> token = String_Manipulation(command,' ');
     
        if(token[0]=="download"){
            int sock = 0;
        struct sockaddr_in download_from_another_peer;

        memset(&download_from_another_peer, '0', sizeof(download_from_another_peer));
        download_from_another_peer.sin_family = AF_INET;
        download_from_another_peer.sin_port = htons(atoi(token[2].c_str()));
        download_from_another_peer.sin_addr.s_addr=inet_addr("127.0.0.1");
        

        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
        {
            perror("socket failed");
            exit(EXIT_FAILURE);
        }
        
        if (inet_pton(AF_INET, "127.0.0.1", &download_from_another_peer.sin_addr) <= 0)
        {
            printf("\nClient File  : Invalid address/ Address not suppcserveridorted \n");
            return NULL;
        }

        if (connect(sock, (struct sockaddr *)&download_from_another_peer, sizeof(download_from_another_peer)) < 0)
        {
            printf("\nConnection Failed in client side\n");
            return NULL;
        }
        printf("connection done yey!");

            pthread_t download_thread ; 
            if (pthread_create(&download_thread, NULL, Download_Threads, (void *)&sock) < 0)
            {
                perror("\ncould not create thread\n");
            }


            string temp = token[1];
            char *clientrequest = new char[temp.length() + 1];
            strcpy(clientrequest, temp.c_str());
           

            int size=strlen(clientrequest);
            int n=send(sock,&size,sizeof(size),0);
        //    cout << "chunk size sent" << n << endl ; 

           int k = send(sock, clientrequest, strlen(clientrequest), 0);

            cout << "name sent" << k << endl;

        }
    
}
return socket_desc;
}

void *Server_Threads(void *socket_desc)
{
      int server_fd_thread = *(int *)socket_desc;
   
     int filesize;
        
            int  l=  recv(server_fd_thread,&filesize,sizeof(filesize),0);
            char buffer[512]={0};
          //   cout << l << endl;
               
           int   k=  recv( server_fd_thread , buffer ,512, 0);
        //   cout << buffer << endl;
         //   memset (buffer, '\0', 512);
         //   cout << k << endl;
           char filename[512] ;
            strcpy(filename, buffer);
         // cout << buffer << endl;
           
         FILE *fp = fopen ( filename  , "r" );
        if(!fp){
            perror("not opening");
        }
        
        send ( server_fd_thread ,(char *) filename, 100, 0);


	fseek ( fp , 0 , SEEK_END);
  	int size = ftell ( fp );
  	rewind ( fp );

    send ( server_fd_thread , &size, sizeof(size), 0);

char Buffer [ 512*32 ] ; 
int n;
	while ( ( n = fread( Buffer , sizeof(char) , 512 *32, fp ) ) > 0  && size > 0 ){
	int k=	send (server_fd_thread , Buffer, n, 0 );
  //  cout << k << endl;
   	 	memset ( Buffer , '\0', 512*32 );
		size = size - n ;
}

fclose ( fp );

          
        

return socket_desc;
}




void *server(void *socket_desc){
   
   
   char buffer[512]={0};
    //cout << "SDsdsd" << endl;
    string cli_socket = *(string *)socket_desc;
      pthread_t thread_id;

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed in seeder");
        exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
                   &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(atoi(cli_socket.c_str()));

    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        perror("bind failed in seeder");
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

               if (pthread_create(&thread_id, NULL, Server_Threads, (void *)&new_socket) < 0)
            {
                perror("\ncould not create thread\n");
            }

            printf("\nthread ban raha he \n");

        }
   
return socket_desc;
}










string download (vector<string> tokens){
    string A=""; 
    A=tokens[0]+" "+tokens[1];
    return A;
}

int main(int argc, char * argv[]){

   
         clientsocketstr = string(argv[1]);
         trackersocket1str = string(argv[2]);
        
        pthread_t cserverid,cstracker;
        if (pthread_create(&cserverid, NULL, server, (void *)&clientsocketstr) < 0)
        {
            perror("\ncould not create thread in client side\n");
        }

        //socket programming on client side
        int sock = 0;
        struct sockaddr_in serv_addr;
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            printf("\n Socket creation error in client side\n");
            return -1;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(atoi(trackersocket1str.c_str()));
        serv_addr.sin_addr.s_addr=inet_addr("127.0.0.1");

        //Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
        {
            printf("\nClient File  : Invalid address/ Address not suppcserveridorted \n");
            return -1;
        }

        if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\nConnection Failed in client side\n");
            return -1;
        }

        printf("connection okay! yey \n");

        if (pthread_create(&cstracker, NULL, Server_Tracker, (void *)&sock) < 0)
        {
            perror("\ncould not create thread in client side\n");
        }

      

        while (1)
        {
            printf("$$$ ");
            int getflag = 0, closeflag = 0;
            char *mtorrentfilepath;
            string strcmd, destpath, getcmdmtorrentpath;

           
            getline(cin >> ws, strcmd);
          

            vector<string> tokens=String_Manipulation(strcmd,' ');
            string complexdata;
            
          
            if (tokens[0] == "upload")
            {
                if (tokens.size() != 2)
                {
                    cout << "INVALID_ARGUMENTS --- SHARE Command" << endl;
                    continue;
                }
                
              
                complexdata = upload(tokens,clientsocketstr);
                
                if (complexdata == "NULL ")
                    continue;
            }
            else if (tokens[0] == "download")
            {   
            
                if (tokens.size() != 2)
                {
                    cout << "INVALID_ARGUMENTS --- GET Command" << endl;
                    continue;
                }
               
                complexdata = download(tokens);
                // destpath = tokens[2];
                // getcmdmtorrentpath = tokens[1];
               
                if (complexdata == "NULL ")
                    continue;
                
            }
            else if (tokens[0] == "logout")
            {
                // if (tokens.size() != 2)
                // {
                //     cout << "INVALID_ARGUMENTS --- REMOVE Command" << endl;
                //     continue;
                // }
                
                // complexdata = logout(tokens, clientsocketstr);
                // if (complexdata == "NULL ")
                //     continue;
            }
            else if (tokens[0] == "login")
            {
                // if (downloadstatus.empty())
                // {
                //     cout << "NO DOWNLOADS TILL NOW" << endl;
                // }
                // else
                // {
                //     cout << "********* DOWNLOADS **********" << endl;
                //     for (auto item : downloadstatus)
                //     {
                //         cout << item.second << " : " << item.first << endl;
                //     }
                // }
                // continue;
            }
            else if (tokens[0] == "creat_group")
            {
                printf("not done yet");
                // complexdata = "close#" + clientsocketstr;
                // closeflag = 1;
            }
            else
            {
                cout << "INVALID COMMAND" << endl;
                continue;
            }

         

            char *clientreply = new char[complexdata.length() + 1];
            strcpy(clientreply, complexdata.c_str());
         

            int size=strlen(clientreply);
            send(sock,&size,sizeof(size),0);
            send(sock, clientreply, strlen(clientreply), 0);
            
            
         }
         return 0;
    }


 
    











    

   
