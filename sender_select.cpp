#include "headers.h"

#pragma pack(1) 
struct packet{
    
    int serial_n;
     char data[1020];
    //1024 bytes
};
#pragma pack(0) 
int select_time(int fd, int usec) {
    fd_set rset;
    struct timeval tv;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tv.tv_sec = 0;
    tv.tv_usec = usec;

    return(select(fd + 1, &rset, NULL, NULL, &tv));// > 0 if fd is readable
}
//g++ -std=c++17 -o s s2.cpp -lstdc++fs
/*
sudo tc qdisc del dev lo root
sudo tc qdisc add dev lo root netem loss 10%
*/
void sendFile(int sockfd, struct sockaddr  *recv_addr, socklen_t recvlen, const char *filename) {  
    string file(filename);
    fstream readfile;
    readfile.open(file, ios_base::in | ios_base::binary);

    size_t size = experimental::filesystem::file_size(file);
    vector<packet> send_string;
    int file_size = size; 
    int serial = 0;
    //	g++ -std=c++17 -O3 -o s sns.cpp -lstdc++fs
    // g++ -O3 -o r rns.cpp
    //get 1024 - 8 bytes
    for(int j = 0;j < file_size;) {
        packet temp;
        int tempsize = 0;
        temp.serial_n = serial++;
        if(file_size - j >= 1019)
            tempsize = 1019;
        else {
            tempsize = (file_size - j);
        }
        char buffer[tempsize];
        readfile.read(buffer, tempsize);
        memcpy(temp.data, buffer, tempsize);
        j += tempsize;
        send_string.emplace_back(temp);
    }
    time_t t1,t2; 
    int n = 0;
    packet close ;
    close.serial_n = file_size;
    strcpy(close.data, "close");
    send_string.emplace_back((close));

    t1 =  time(NULL);
    
    char *recvi = (char *)malloc (10) ;
    while(n < send_string.size()) {

        /* Send the string to the server */
        
        if(sendto(sockfd, (struct packet*)&send_string[n], sizeof(packet), 0, recv_addr, recvlen) < 0) {
            cout << "send error\n";
            exit(0);
        }
        
        if(select_time(sockfd, SEND_TIMEO_USEC) == 0) {
                //cout << "recv ack timeout\n";
                continue;
        }
        else {
           
            if(recvfrom(sockfd, (char *)recvi,  sizeof(recvi), 0, recv_addr, &recvlen) < 0) {
                perror("recvmsg error");
                exit(-1);
            }
        }
        
        if(atoi(recvi) == n )n++;
        //if(atoi(recvi) == send_string.end()->serial_n) break;
    }
    delete recvi;
    t2 = time(NULL);   
    cout<< "Transferring Time: " << t2 - t1 <<" second\nFile size: "; 
    cout << file_size << '\n';
    return;
}
int main(int argc ,char **argv) {
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    if(argc != 4) {
        printf("Usage: ./sender2  <file name> <receiver IP> <receiver port>\n");
        exit(0);
    }
    int recvfd;
    struct addrinfo hints, *res;
    struct sockaddr_in receiver_addr;
    char receiver_ip[INET6_ADDRSTRLEN];
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
    if(getaddrinfo(argv[2], NULL, &hints, &res) != 0) {
        perror("getaddrinfo error");
        exit(EXIT_FAILURE);
    }

    struct in_addr host_addr;
    host_addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
    inet_ntop(AF_INET, &host_addr, receiver_ip, INET6_ADDRSTRLEN);
    recvfd = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in recv_addr;
    bzero(&recv_addr, sizeof(recv_addr));
    recv_addr.sin_family = AF_INET;
    recv_addr.sin_port = htons(atoi(argv[3]));
    if(inet_pton(AF_INET, receiver_ip, &recv_addr.sin_addr) <= 0)
        printf("inet_pton error for %s\n", argv[2]);
    sendFile(recvfd, (struct sockaddr *)&recv_addr, res->ai_addrlen, argv[1]);
    freeaddrinfo(res);
    
    return 0;
}
