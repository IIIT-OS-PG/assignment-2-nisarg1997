
#include<bits/stdc++.h>
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
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

int CSIZE = 512 * 1024 ; 
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
        return "-1";
    }

    struct stat fstatus;
    stat(fpath, &fstatus);

    // Logic for deviding file1 into chunks
    long int total_size = fstatus.st_size;
    long int chunk_size = CSIZE;

    int total_chunks = total_size / chunk_size;
    int last_chunk_size = total_size % chunk_size;

    if (last_chunk_size != 0) // if file1 is not exactly divisible by chunks size
    {
        ++total_chunks; // add last chunk to count
    }
    else //when file1 is completely divisible by chunk size
    {
        last_chunk_size = chunk_size;
    }

    // loop to getting each chunk
    for (int chunk = 0; chunk < total_chunks; ++chunk)
    {
        int cur_cnk_size;
        if (chunk == total_chunks - 1)
            cur_cnk_size = last_chunk_size;
        else
            cur_cnk_size = chunk_size;

        char *chunk_data = new char[cur_cnk_size];
        file1.read(chunk_data,    /* address of buffer start */
                   cur_cnk_size); /* this many bytes is to be read */

        string sh1out = calHashofchunk(chunk_data, cur_cnk_size, 1);
        fileHash = fileHash + sh1out;
    }

    return fileHash;
}

void *seederservice(void *socket_desc)
{
    int new_socket = *(int *)socket_desc;
    char buffer[1024] = {0};
    read(new_socket, buffer, 1024);
    
    string actualfilepath = string(buffer);

    char *fpath = new char[actualfilepath.length() + 1];
    strcpy(fpath, actualfilepath.c_str());

    ifstream file1(fpath, ifstream::binary);

    if (!file1)
    {
        cout << "Can't Open file1  : " << string(fpath) << endl;
        //return "-1";
    }

    struct stat fstatus;
    stat(fpath, &fstatus);

    // Logic for deviding file1 into chunks
    long int total_size = fstatus.st_size;
    long int chunk_size = CSIZE;

    int total_chunks = total_size / chunk_size;
    int last_chunk_size = total_size % chunk_size;

    if (last_chunk_size != 0) // if file1 is not exactly divisible by chunks size
    {
        ++total_chunks; // add last chunk to count
    }
    else //when file1 is completely divisible by chunk size
    {
        last_chunk_size = chunk_size;
    }

    // loop to getting each chunk
    for (int chunk = 0; chunk < total_chunks; ++chunk)
    {
        int cur_cnk_size;
        if (chunk == total_chunks - 1)
            cur_cnk_size = last_chunk_size;
        else
            cur_cnk_size = chunk_size;

        char *chunk_data = new char[cur_cnk_size];
        file1.read(chunk_data,    /* address of buffer start */
                   cur_cnk_size); /* this many bytes is to be read */

        send(new_socket, chunk_data, cur_cnk_size, 0);
    }

    //printf("Reply message sent from seeder\n");
    close(new_socket);
    file1.close();
    return socket_desc;
}

void *seederserverservice(void *socket_desc)
{
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
        // accept connection from any leecher
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Error in accept connection in seeder");
            exit(EXIT_FAILURE);
        }

        //create new thread to provide data to that leecher and seeder again listen for other
        if (pthread_create(&thread_id, NULL, seederservice, (void *)&new_socket) < 0)
        {
            perror("\ncould not create thread in seeder\n");
        }
        if (new_socket < 0)
        {
            perror("accept failed in seeder");
        }
    }

    return socket_desc;
}

string getsmallhash(string A)
{
    string final_ans;
   // const char * k = A.c_str();
    //    char * s;
    //    strcpy(s,k);
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
        cout << final_ans;
        //ans= final_ans;
    
    
    return final_ans;
    
}

string logout (string token,string sockatport){

}



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

string upload(vector<string> tokens) {

    string A;
    char s[tokens[1].size()+1];
    strcpy(s,tokens[1].c_str());
   
    A=getFileHash(s);

    return A;
}




// extern int globalcurchunksize;

int main(int argc, char * argv[]){

    // char filename[] = "P.SILoveYou.avi";

    // string A = getFileHash(filename);
    // string B= getsmallhash(A);




    // socketclass clientsocket;
    // socketclass trackersocket1;
    // socketclass trackersocket2;
    // if (argc != 5)
    // {
    //     cout << "Invalid Argument in client!!!" << endl;
    // }
    // else
    
         clientsocketstr = string(argv[1]);
         trackersocket1str = string(argv[2]);
        
        // clientsocket.setsocketdata(clientsocketstr);
        // trackersocket1.setsocketdata(trackersocket1str);
        // trackersocket2.setsocketdata(trackersocket2str);
        // logpath = argv[4];
        // ofstream myfile(logpath, std::ios_base::out);
        // myfile.close();
        // writelog("********new client started *********");
        // writelog("Client socket : " + clientsocketstr);
        // writelog("Tracker 1 socket : " + trackersocket1str);
        // writelog("Tracker 2 socket : " + trackersocket2str);
        // // cout<<"Tracker 1 socket: "<<trackersocket1.ip<<" : "<<trackersocket1.port<<endl;
        // // cout<<"Tracker 2 socket: "<<trackersocket2.ip<<" : "<<trackersocket2.port<<endl;

        // //create new thread from client which act as seeder(server) to provide data to others
        pthread_t cserverid;
        if (pthread_create(&cserverid, NULL, seederserverservice, (void *)&clientsocketstr) < 0)
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

        printf("yey ");
       
        //continuously listening to client for his entring command
        while (1)
        {

            int getflag = 0, closeflag = 0;
            char *mtorrentfilepath;
            string strcmd, destpath, getcmdmtorrentpath;

            //writelog("Enter the command : ");
            getline(cin >> ws, strcmd);
          

            vector<string> tokens=stringProcessing(strcmd,' ');
            string complexdata;

            // To handle which command enter by client
            if (tokens[0] == "upload")
            {
                if (tokens.size() != 2)
                {
                    cout << "INVALID_ARGUMENTS --- SHARE Command" << endl;
                    continue;
                }
              
                complexdata = upload(tokens);
                
                if (complexdata == "-1")
                    continue;
            }
            else if (tokens[0] == "download")
            {
                // if (tokens.size() != 2)
                // {
                //     cout << "INVALID_ARGUMENTS --- GET Command" << endl;
                //     continue;
                // }
               
                // complexdata = download(tokens);
                // destpath = tokens[2];
                // getcmdmtorrentpath = tokens[1];
                // if (complexdata == "-1")
                //     continue;
                // else
                // {
                //     getflag = 1;
                // }
            }
            else if (tokens[0] == "logout")
            {
                // if (tokens.size() != 2)
                // {
                //     cout << "INVALID_ARGUMENTS --- REMOVE Command" << endl;
                //     continue;
                // }
                
                // complexdata = logout(tokens, clientsocketstr);
                // if (complexdata == "-1")
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
            //cout<<"clientreply : "<<clientreply<<endl;

            //to send client request to tracker
            send(sock, clientreply, strlen(clientreply), 0);

            
            //to recieve tracker responce
            char buffer[1024] = {0};
            read(sock, buffer, 1024);
           
            if (getflag != 1)
                cout << string(buffer) << endl;

            string responce = string(buffer);

            //when getting response of get command as list of  client socket having file
            if (getflag == 1)
            {
                // struct complexData obj1;
                // obj1.replydata1 = new char[responce.length() + 1];
                // strcpy(obj1.replydata1, responce.c_str());
                // obj1.destpath1 = new char[destpath.length() + 1];
                // strcpy(obj1.destpath1, destpath.c_str());
                // obj1.getcmdmtorrentpath1 = new char[getcmdmtorrentpath.length() + 1];
                // strcpy(obj1.getcmdmtorrentpath1, getcmdmtorrentpath.c_str());
                // obj1.sock1 = sock;

                // //for non-blocking get command (create separate thread for downloading file)
                // pthread_t getclientid;
                // if (pthread_create(&getclientid, NULL, getcommandExecution, (void *)&obj1) < 0)
                // {
                //     perror("\ncould not create thread in client side\n");
                // }
            }
            getflag = 0;

            //When Server Send Response for remove command
            if (responce == "FILE SUCCESSFULLY REMOVED")
            {
                if (remove(mtorrentfilepath) != 0)
                    perror("\nError deleting mtorrent file\n");
            }

            //When Client is closed
            if (closeflag == 1)
            {
                cout << "Thank You !!!" << endl;
                close(sock);
                break;
            }
        }
         return 0;
    }


 
    











    

   
