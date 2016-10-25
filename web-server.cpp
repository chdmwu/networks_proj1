#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <thread>

#include <fstream>
#include <iostream>
#include <sstream>

#include "HttpMessage.h"

using namespace std;
vector<char> getFileBuffer(string filePath);
void handleRequest(string fileDir, int clientSockfd);

int main(int argc, char *argv[])
{
	string hostname;
	int port;
	string fileDir;

	if(argc <= 1){
		hostname = "localhost";
		port = 4000;
		fileDir = ".";
	}
	else if(argc == 4){
		hostname = string(argv[1]);
		port = atoi(argv[2]);
		fileDir = argv[3];
	}
	std::cout << "Creating server" << std::endl;
	// create a socket using TCP IP
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);


	// allow others to reuse the address
	int yes = 1;
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		return 1;
	}

	// bind address to socket
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);     // short, network byte order
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	memset(addr.sin_zero, '\0', sizeof(addr.sin_zero));

	if (bind(sockfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		perror("bind");
		return 2;
	}

	// set socket to listen status
	if (listen(sockfd, 1) == -1) {
		perror("listen");
		return 3;
	}

	bool runServer = true;


	while(runServer){
		// accept a new connection
		struct sockaddr_in clientAddr;
		socklen_t clientAddrSize = sizeof(clientAddr);
		int clientSockfd = accept(sockfd, (struct sockaddr*)&clientAddr, &clientAddrSize);

		if (clientSockfd == -1) {
			perror("accept");
			return 4;
		}

		char ipstr[INET_ADDRSTRLEN] = {'\0'};
		inet_ntop(clientAddr.sin_family, &clientAddr.sin_addr, ipstr, sizeof(ipstr));
		std::cout << "Accept a connection from: " << ipstr << ":" <<
				ntohs(clientAddr.sin_port) << std::endl;

		//spawn a new thread to handle the incoming request
		std::thread requestThread(handleRequest, fileDir, clientSockfd);
		requestThread.detach();
	}

}

vector<char> getFileBuffer(string filePath) {
	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg() + 1;
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);
	buffer.back() = '\0';
	return buffer;
}

void handleRequest(string fileDir, int clientSockfd){
	//receive the http request
	size_t MAX_MSG_SIZE = 10000;
	void* buf[MAX_MSG_SIZE];
	memset(buf, 0, MAX_MSG_SIZE);
	std::stringstream ss;
	memset(buf, '\0', sizeof(buf));
	if (recv(clientSockfd, buf, MAX_MSG_SIZE, 0) == -1) {
		perror("recv");
	}
	std::string str((char*)buf);
	HttpRequest request;
	bool goodRequest = request.consumeMessage(str);
	cout << goodRequest << endl;
	cout << "HTTP Request by client:" << endl;
	cout << str << endl;

	//get the requested file
	HttpResponse response;
	response.setVersion("HTTP/1.0");
	string fullFilePath = fileDir + request.path_;
	if(!goodRequest){
		cout << "400 Bad Request" << endl;
		response.setStatus("400 Bad Request");
		response.setLength("0");
		response.setBody("");
	} else if(ifstream(fullFilePath)){
		cout << string(getFileBuffer(fullFilePath).data()) << endl;
		vector<char> fileBuffer = getFileBuffer(fileDir + request.path_);
		string fileBody = string(fileBuffer.data());
		response.setStatus("200 OK");
		response.setLength(std::to_string(fileBuffer.size()-1)); //-1 because of null terminator
		response.setBody(fileBody);
	} else {
		cout << "404 Not Found" << endl;
		response.setStatus("404 Not Found");
		response.setLength("0");
		response.setBody("");
	}
	string responseMsg = response.createMessage();
	if (send(clientSockfd, responseMsg.c_str(), responseMsg.size(), 0) == -1){
		perror("send");
	}
	close(clientSockfd);
}
