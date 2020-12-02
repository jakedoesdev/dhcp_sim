/*
DHCP Client simulation
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define SERVER "129.120.151.96" //cse03.cse.unt.edu
#define BUFLEN 1024             //Max length of buffer

//kills any function that returns error
void die(char *s)
{
    perror(s);
    exit(1);
}

//printConversion() declaration
void printConversion(unsigned int si, unsigned int yi, unsigned int tr, unsigned short int li);
 
int main(int argc, char **argv)
{
    struct sockaddr_in si_other, si_net;
    int sockfd, i=0, slen=sizeof(si_other), tmp, recv_len;
    char buf[BUFLEN];

    struct dhcp_packet {
    unsigned int siaddr;                  //server IP address
    unsigned int yiaddr;                  //sent IP address
    unsigned int tran_ID;                 //transaction ID
    unsigned short int lifetime;          //lease time of IP (3600)
    } dPkt;

    //seed random
    time_t tm;
    srand((unsigned) time(&tm));

    //check for correct usage
    if (argc != 2)
	{
		die("Usage: ./dclient <portnumber>\n");
	}

    //set up socket vars to connect to cse03 server
    memset((char *) &si_net, 0, sizeof(si_net));
    if (inet_pton(AF_INET, (const void *)SERVER, &(si_net.sin_addr)) == 0)
    {
        die("inet_pton");
    }
    si_net.sin_addr.s_addr = ntohl(si_net.sin_addr.s_addr);
    
    //create DHCP discover packet
    memset(&dPkt, 0, sizeof(dPkt));
    dPkt.siaddr = si_net.sin_addr.s_addr;
    dPkt.yiaddr = 0;
    dPkt.tran_ID = (rand() % 10000 + 25);
    dPkt.lifetime = 0;
 
    //create socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
 
    //set socket vars
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = atoi(argv[1]);
    

    if (inet_aton(SERVER , &si_other.sin_addr) == 0) 
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    //send DHCP discover, convert and print sent values, and store transaction ID in tmp
    if (sendto(sockfd, &dPkt, sizeof(dPkt), 0, (struct sockaddr *) &si_other, slen) == -1)
    {
        die("sendto()");
    }
    printConversion(dPkt.siaddr, dPkt.yiaddr, dPkt.tran_ID, dPkt.lifetime);
    tmp = dPkt.tran_ID;
    memset(&dPkt, 0, sizeof(dPkt));

    //recv DHCP offer, iterate tran_ID if match
    if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
    {
        die("recvfrom()");
    }
    memcpy(&dPkt, buf, recv_len);
    if (dPkt.tran_ID == tmp)
    {
        dPkt.tran_ID++;
    }

    //send DHCP request, convert and print sent values
    if (sendto(sockfd, &dPkt, sizeof(dPkt), 0, (struct sockaddr *) &si_other, slen) == -1)
    {
        die("sendto()");
    }
    printConversion(dPkt.siaddr, dPkt.yiaddr, dPkt.tran_ID, dPkt.lifetime);
    memset(&dPkt, 0, sizeof(dPkt));
    
    //recv DHCP ACK, then sleep 1000 seconds
    if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
    {
        die("recvfrom()");
    }
    memcpy(&dPkt, buf, recv_len);
    if (dPkt.tran_ID == tmp+1)
    {
        sleep(1000);
    }
 
    close(sockfd);
    return 0;
}

/*
Given the elements of a DHCP packet (network ints si/yi in host order, trans id tr, and lifetime li), converts network ints to strings and prints all values. 
*/
void printConversion(unsigned int si, unsigned int yi, unsigned int tr, unsigned short int li)
{
    struct sockaddr_in tmp;
    char siStr[64];
    char yiStr[64];

    tmp.sin_addr.s_addr = htonl(si);
    inet_ntop(AF_INET, &tmp.sin_addr.s_addr, siStr, sizeof(siStr));
    tmp.sin_addr.s_addr = htonl(yi);
    inet_ntop(AF_INET, &tmp.sin_addr.s_addr, yiStr, sizeof(yiStr));

    printf("siaddr:%s\nyiaddr:%s\ntransaction ID:%u\nlifetime:%hu\n\n", siStr, yiStr, tr, li);
}