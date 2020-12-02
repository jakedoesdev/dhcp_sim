/*
DHCP Server simulation
*/
#include <stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <ctype.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
 
#define BUFLEN 1024  //Max length of buffer

//kills any function that returns error
void die(char *s)
{
    perror(s);
    exit(1);
}

//struct stored in ip_rng array
typedef struct ip_addr {
    unsigned int addr;        //int value of network address
    unsigned short int taken; //taken flag, 0 or 1
} ip_addr;

//printConversion() declaration
void printConversion(unsigned int si, unsigned int yi, unsigned int tr, unsigned short int li);
 
int main(int argc, char **argv)
{
    struct sockaddr_in si_me, si_other, si_net;        //socket structs
    char *strAddr;                                     //user-entered network address string
    unsigned short int mask;                           //user-entered subnet mask
    unsigned short int avail;                          //calculated number of available networks
    unsigned int netAddr;                              //integer conversion of network address
    int sockfd, i, slen = sizeof(si_other) , recv_len; //socket, flow, length variables
    char buf[BUFLEN];                                  //buffer

    //dhcp packet structure
    struct dhcp_packet {
    unsigned int siaddr;                               //server IP address
    unsigned int yiaddr;                               //offered IP address
    unsigned int tran_ID;                              //transaction ID
    unsigned short int lifetime;                       //lease time of IP (3600)
    } dPkt;

    ip_addr* ip_rng;                                  //array of unsigned int IP address ranges w/ taken (0/1) flag 

    //check for correct usage
    if (argc != 2)
	{
		die("Usage: ./dserver <portnumber>\n");
    }

    //allocate memory for strAddr
    strAddr = (char *)malloc(16 * sizeof(char));
    if (strAddr == NULL)
        die("malloc");

    memset((char *) &si_net, 0, sizeof(si_net));
    
    //get network address from user (while strAddr is not valid input for inet_pton (must be n.n.n.n))
    //valid input is converted from string to network int by inet_pton, then converted from network to host order by ntohl
    while (inet_pton(AF_INET, strAddr, &(si_net.sin_addr)) == 0)
    {
        printf("network address:");
        scanf("%s", strAddr);
    }
    si_net.sin_addr.s_addr = ntohl(si_net.sin_addr.s_addr);

    //get subnet mask from user (while mask is less than 24 or more than 30)
    mask = 0;
    while (mask < 24 || mask > 30)
    {
        printf("subnet_part:");
        scanf("%hu", &mask);
    }

    //calculate available IPs based on user-entered mask, allocate space in ip_rng array, populate array
    netAddr = si_net.sin_addr.s_addr;
    avail = (pow(2, (32 - mask)) - 2);
    ip_rng = (struct ip_addr*)malloc(avail * sizeof(*ip_rng));
    for (i = 0; i < avail; i++)
    {
        ip_rng[i].addr = netAddr +(i+1);
        ip_rng[i].taken = 0;
        //printf("ip_rng: %u\n", ip_rng[i].addr);
    }

    //create a UDP socket, populate si_me socket struct
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = atoi(argv[1]);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);  //host format to net format (little to big endian)

    //bind socket to port
    if(bind(sockfd, (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }
    
    //start listening for DHCP discover requests
    while(1)
    {

        //clear variables
        fflush(stdout);
        bzero (buf, BUFLEN);
        memset(&dPkt, 0, sizeof(dPkt));
        int flag = 0;                    //flag to indicate when all IPs are taken
        i=0;

        //recv DHCP discover, memcpy into dPkt
        if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }
        memcpy(&dPkt, buf, recv_len);

        //while dPkt.yiaddr is not populated, and all ip_rng elements have not been iterated through...
        while (!dPkt.yiaddr && i < avail)
        {
            //if IP is not taken, set dPkt.yiaddr to available IP and set lifetime to 3600 (taken variable set after DHCP request is received)
            if (ip_rng[i].taken == 0)
            {
                dPkt.yiaddr = ip_rng[i].addr;
                dPkt.lifetime = 3600;
            }
            //if there are no available IPs left, print msg to console and set flag
            else
            {
                if (i == avail-1)
                {
                    printf("No available IPs\n");
                    flag = -1;
                }
                i++;
            }       
        }

        //if a valid IP was assigned...
        if (flag != -1)
        {
            //send DHCP offer, convert and print dPkt values
            if (sendto(sockfd, &dPkt, sizeof(dPkt), 0, (struct sockaddr *) &si_other, slen) == -1)
            {
                die("sendto()");
            }  
            printConversion(dPkt.siaddr, dPkt.yiaddr, dPkt.tran_ID, dPkt.lifetime);

            //recv DHCP request and set taken flag to 1
            bzero (buf, BUFLEN);
            if ((recv_len = recvfrom(sockfd, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
            {
                die("recvfrom()");
            }
            memcpy(&dPkt, buf, recv_len);
            ip_rng[i].taken = 1;

            //send DHCP ACK, convert and print dPkt values
            if (sendto(sockfd, &dPkt, sizeof(dPkt), 0, (struct sockaddr *) &si_other, slen) == -1)
            {
                die("sendto()");
            }
            printConversion(dPkt.siaddr, dPkt.yiaddr, dPkt.tran_ID, dPkt.lifetime);
        }
    }
 
    //free allocated memory
    free(ip_rng);
    free(strAddr);
    
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