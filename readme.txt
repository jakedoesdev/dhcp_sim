Author: Jacob Everett

Make file instructions 

To compile:  make
To clean:    make clean
To run: ./dserver <port number>
        ./dclient <port number>

Usage instructions

When server is exectued, user will be prompted for network address--must be in a valid IPv4 format as accepted by inet_pton (n.n.n.n).
After valid IPv4 network address is entered, user will be prompted for subnet mask--must be between 24-30, inclusive.
Given a valid network address and subnet mask, server will start listening for DHCP discover packets.
When a packet is received, the server will find an available IP address, send a DHCP offer to the client and print it to the console,
wait for DHCP request, and send a DHP ACK and print it to the console before listening for new discovers.
If no available IPs exist, the server will print "No available IPs" at each request.

When client is executed, it will create and send a DHCP discover to server cse03, wait for an offer, send a request, wait for an ACK,
and then, upon receipt of a valid DHCP ACK, sleep for 1000 seconds.

Example server:
./dserver 5555
network address: 10.10.10.0
subnet_part: 30
siaddr:129.120.151.96
yiaddr:10.10.10.1
transaction ID:9803
lifetime:3600

siaddr:129.120.151.96
yiaddr:10.10.10.1
transaction ID:9804
lifetime:3600

Example client:
./dclient 5555
siaddr:129.120.151.96
yiaddr:0.0.0.0
transaction ID:9803
lifetime:0

siaddr:129.120.151.96
yiaddr:10.10.10.1
transaction ID:9804
lifetime:3600