#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
 
struct sockaddr_in localSock;
struct ip_mreq group;
int sd;
struct Ubuf{
	char buf[256];
	uint8_t bye;
};
 
int main(int argc, char *argv[])
{
	int n, ret;
	char f_name[256];
	int f_size = 0, f_len = 0;
	struct Ubuf ubuf;
	ubuf.bye = 0;

/* Create a datagram socket on which to receive. */
	sd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sd < 0)
	{
		perror("Opening datagram socket error");
		exit(1);
	}
	else
	printf("Opening datagram socket....OK.\n");
		 
	/* Enable SO_REUSEADDR to allow multiple instances of this */
	/* application to receive copies of the multicast datagrams. */
	
	int reuse = 1;
	if(setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, (char *)&reuse, sizeof(reuse)) < 0)
	{
		perror("Setting SO_REUSEADDR error");
		close(sd);
		exit(1);
	}
	else
		printf("Setting SO_REUSEADDR...OK.\n");
	
	 
	/* Bind to the proper port number with the IP address */
	/* specified as INADDR_ANY. */
	memset((char *) &localSock, 0, sizeof(localSock));
	localSock.sin_family = AF_INET;
	localSock.sin_port = htons(4321);
	localSock.sin_addr.s_addr = INADDR_ANY;
	if(bind(sd, (struct sockaddr*)&localSock, sizeof(localSock)))
	{
		perror("Binding datagram socket error");
		close(sd);
		exit(1);
	}
	else
		printf("Binding datagram socket...OK.\n");
	 
	/* Join the multicast group 226.1.1.1 on the local address*/
	/* interface. Note that this IP_ADD_MEMBERSHIP option must be */
	/* called for each local interface over which the multicast */
	/* datagrams are to be received. */
	group.imr_multiaddr.s_addr = inet_addr("226.1.1.1");
	/* your ip address */ 
	group.imr_interface.s_addr = inet_addr("192.168.22.198"); 
	/* IP_ADD_MEMBERSHIP:  Joins the multicast group specified */ 
	if(setsockopt(sd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&group, sizeof(group)) < 0)
	{
		perror("Adding multicast group error");
		close(sd);
		exit(1);
	}
	else
		printf("Adding multicast group...OK.\n");
	 
	/* Read from the socket. */
	
	recvfrom(sd, &f_len, sizeof(int), 0, NULL, NULL); // receive filename length
	recvfrom(sd, f_name, f_len * sizeof(char), 0, NULL, NULL); // receive filename

	/* create receive filename*/
	char ch[20] = "recv_";
	char newname[512];
	memset(newname, '\0', sizeof(newname));
	strcat(newname,ch);
	strcat(newname, f_name);
	int f = open(newname, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	/* receive file from socket*/
	while(( n = recv(sd, &ubuf, sizeof(ubuf) - 1, 0)) > 0)
	{
		if(ubuf.bye) break;
		ret = write(f, &ubuf,n);
	}
	struct stat st;
	fstat(f, &st);
	f_size = st.st_size; // calculate file size

	printf("Reading datagram message...OK.\n");
	printf("receive file size: %uKB\n",f_size/1000);
	close(f);
	close(sd);
	return 0;
}

