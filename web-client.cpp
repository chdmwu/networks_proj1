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
#include <fstream>


using namespace std;


bool writeResponseToFile(string filePath, string msg){
	istringstream ss(msg);
	string temp;
	bool inBody = false;
	std::ofstream out(filePath);
	while(std::getline(ss,temp,'\n')){
		if(inBody){
			out << temp << endl;
		}
		if(temp.size() == 0) {inBody = true;}
	}
	return true;
}


int main(int argc, char *argv[]){

	//TODO URLs
	//string url(argv[1]);


	HttpRequest r;
	r.setMethod("GET");
	r.setPath("/index.html");
	r.setVersion("HTTP/1.1");
	r.setHost("localhost");
	r.setBody("");
	string filePath = "deleteme.txt";
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
	HttpResponse response;
	bool isOK = true;
	bool done = false;
	int numcalls = 0;
	while(isOK && !done){
		size_t MAX_MSG_SIZE = 10000;
		void* buf[MAX_MSG_SIZE];
		memset(buf, '\0', sizeof(buf));
		if (recv(sockfd, buf, MAX_MSG_SIZE, 0) == -1) {
			perror("recv");
			return 5;
		}
		//std::cout << "HTTP Response by server: " << endl;
		//std::cout << string((char*) buf) << std::endl;
		string responseMsg((char*) buf);
		cout << responseMsg;

		isOK = response.writeFile(responseMsg, filePath);
		ifstream file(filePath, ios::binary | ios::ate);
		cout << file.tellg() << endl;
		cout << response.bodySize_ << endl;
		done = (file.tellg() == response.bodySize_);
		numcalls++;
	}

	cout << "made " << numcalls << "calls";

	close(sockfd);

	return 0;
}
