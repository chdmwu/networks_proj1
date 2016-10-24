#include <string>
#include <thread>
#include <iostream>
#include "HttpMessage.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sstream>


using namespace std;

int main(int argc, char *argv[]){

	//TODO URLs

	HttpRequest r;
	r.setMethod("GET");
	r.setPath("/index.html");
	r.setVersion("HTTP/1.0");
	r.setHost("localhost");
	r.setBody("");
	string msg = r.createMessage();
	cout << msg << endl;
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(4000);     // short, network byte order
	serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(serverAddr.sin_zero, '\0', sizeof(serverAddr.sin_zero));
	if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) {
		perror("connect");
		return 2;
	}
	cout << "Connection established..." << endl;

	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	if (getsockname(sockfd, (struct sockaddr *)&clientAddr, &clientAddrLen) == -1) {
		perror("getsockname");
		return 3;
	}

	char ipstr[INET_ADDRSTRLEN] = {'\0'};
	inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
	std::cout << "Set up a connection from: " << ipstr << ":" <<
			ntohs(clientAddr.sin_port) << std::endl;

	//SEND
	if (send(sockfd, msg.c_str(), msg.size(), 0) == -1) {
		perror("send");
		return 4;
	}
	//RECV
	size_t MAX_MSG_SIZE = 8000;
	void* buf[MAX_MSG_SIZE];
	memset(buf, 0, MAX_MSG_SIZE);
	std::stringstream ss;
	memset(buf, '\0', sizeof(buf));
	if (recv(sockfd, buf, MAX_MSG_SIZE, 0) == -1) {
		perror("recv");
		return 5;
	}
	ss << buf << std::endl;
	std::cout << "HTTP Response by server: " << endl;
	std::cout << string((char*) buf) << std::endl;

	close(sockfd);

	return 0;
}
