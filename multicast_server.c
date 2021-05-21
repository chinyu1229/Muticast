#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>


struct in_addr localInterface;
struct sockaddr_in groupSock;
int sd;
struct Ubuf{
	char buf[256];
	uint8_t bye;
};

int main(int argc, char *argv[])
{
	int n;
	struct Ubuf ubuf;
	ubuf.bye = 0;
/* Create a datagram socket on which to send. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("datagram socket error");
		exit(1);
	}
	else printf("Opening the datagram socket...OK\n");
	/* Initialize the group sockaddr structure with a */
	/* group address of 226.1.1.1 and port 4321. */
	memset((char *) &groupSock, 0, sizeof(groupSock));
	groupSock.sin_family = AF_INET;
	groupSock.sin_addr.s_addr = inet_addr("226.1.1.1");
	groupSock.sin_port = htons(4321);
	/* Set local interface for outbound multicast datagrams. */
	/* The IP address specified must be associated with a local, */
	/* multicast capable interface. */
	localInterface.s_addr = inet_addr("192.168.22.198");
	/* IP_MULTICAST_IF:  Sets the interface over which outgoing multicast datagrams are sent. */
	if(setsockopt(sd, IPPROTO_IP, IP_MULTICAST_IF, (char *)&localInterface, sizeof(localInterface)) < 0)
	{
		perror("Setting local interface error");
		exit(1);
  	}
	else printf("Setting the local interface...OK\n");

	// send file
	int f_len = strlen(argv[1]) + 1;
	sendto(sd, &f_len, sizeof(int), 0, (struct sockaddr*)&groupSock,sizeof(groupSock)); // send filename length
	sendto(sd, argv[1], f_len * sizeof(char), 0, (struct sockaddr*)&groupSock, sizeof(groupSock)); // send filename

	int f = open(argv[1],O_RDONLY);
	if(f < 0) 
	{
		perror("file open error");
		exit(-1);
	}
	struct stat st;
	fstat(f, &st);
	int f_size = st.st_size; // calculate file size
	/* send file to clients */
	while(( n = read(f, ubuf.buf,sizeof(ubuf.buf))) > 0)
	{
		sendto(sd, &ubuf, n , 0, (struct sockaddr*)&groupSock, sizeof(groupSock));
	}
	memset(ubuf.buf, '\0' ,sizeof(ubuf.buf));
	ubuf.bye = 1; 
	sendto(sd, &ubuf, n , 0, (struct sockaddr *) &groupSock, sizeof(groupSock)); // send end flag
	printf("Sending datagram message...OK\n");
	printf("file size : %uKB\n",f_size / 1000);
	return 0;
}
