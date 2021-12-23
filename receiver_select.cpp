#include <bits/stdc++.h>
#include <ctype.h>
#include <netdb.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <strings.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>

using namespace std;
//cd /home/w/Desktop/OS/np_hw2/final
#define RECV_TIMEO_USEC 100000
#define SEND_TIMEO_USEC 250000
#pragma pack(1) 
string file;
struct packet{
    int serial_n;
    char data[1020];
    //1024 bytes
};
int select_time(int fd, int usec)
{
    fd_set rset;
    struct timeval tv;

    FD_ZERO(&rset);
    FD_SET(fd, &rset);

    tv.tv_sec = 0;
    tv.tv_usec = usec;

    return(select(fd + 1, &rset, NULL, NULL, &tv));// > 0 if fd is readable
}
void recv(int sockfd, struct sockaddr * sender_addr,socklen_t len) {
    fstream outfile;
    outfile.open(file, ios_base::out | ios_base::binary);
    
    
    vector<packet> storage;
    int write_size = 0;
    char *dest = (char *)malloc(2000000000);
    int cpy = 0, start = 0, now;
    
  
    packet *recvp = new packet;
    char *sendi = (char *) malloc (10);
    string ti;
    strcpy(sendi, "0");
    while(1) {
        now = 0;
        
        if(select_time(sockfd, RECV_TIMEO_USEC) == 0) {
            //cout << "recv ack timeout\n";
            if(start != 0)
                goto ackk;
            else 
                continue;
        }
        else {
            
            if(recvfrom(sockfd, (struct packet*)recvp, sizeof(packet), 0, sender_addr, &len) < 0){ 
                perror("recvmsg error");
                exit(-1);
            }
        }
        if(strcmp(recvp->data, "close") == 0){
            int k = stoi(ti);
            ti = to_string(k + 1);
            strcpy(sendi, ti.c_str());
            sendto(sockfd, (char *)sendi, sizeof(sendi), 0, sender_addr, len);
            write_size = recvp->serial_n;
            break;
        }
        else {
            
            //avoid ack packet loss and retransmission
            if(recvp->serial_n >= storage.size()){
                storage.emplace_back((*recvp));
                memcpy(dest + cpy, recvp->data, 1019);
                cpy += 1019;
            
            }
        }
        now = recvp->serial_n;
        start = now;
        ti = to_string(recvp->serial_n);
        strcpy(sendi, ti.c_str());
        
            
        
               
        
        
        ackk:
        
        if(sendto(sockfd, (char *)sendi, sizeof(sendi), 0, sender_addr, len)  < 0) {
            cout << "send error\n";
            exit(-1);
        }
        
    }
     for(int i = 0; i < 20; i++) {
       
        if(select_time(sockfd, RECV_TIMEO_USEC) == 0)
            continue;
        else {
            if(recvfrom(sockfd, (struct packet*)recvp, sizeof(packet), 0, sender_addr, &len) < 0)
            {
                perror("recv error");
                exit(-1);
            }
        }

      

        if(sendto(sockfd, (char *)sendi, sizeof(sendi), 0, sender_addr, len) < 0) {
            perror("send error");
            exit(-1);
        }
    }

    delete sendi;
    delete recvp;
    
    outfile.write(dest,sizeof(char) * write_size);    
    free(dest);
    outfile.close();
}
int main(int argc, char **argv) {
    
    std::ios::sync_with_stdio(false);
    std::cin.tie(0);
    file = argv[1];
    if(argc != 3) {
        printf("Usage: ./receiver1 <filename> <port>\n");
        exit(0);
    }

    int sockfd;
    struct sockaddr_in receiver_addr;

    //creatre a udp socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    

    bzero(&receiver_addr, sizeof(receiver_addr));
    receiver_addr.sin_family = AF_INET;
    receiver_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    receiver_addr.sin_port = htons(atoi(argv[2]));

    if(bind(sockfd, (struct sockaddr *) &receiver_addr, sizeof(receiver_addr))) {
        perror("Bind error");
        exit(0);
    }

    char ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(receiver_addr.sin_addr), ip, INET_ADDRSTRLEN);

    
    struct sockaddr_in sender_addr;
    recv(sockfd, (struct sockaddr *)&sender_addr, sizeof(sender_addr));
    
    close(sockfd);
    return 0;
}
