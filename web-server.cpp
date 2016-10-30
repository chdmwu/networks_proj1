#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <thread>
#include <netdb.h>


#include <fstream>
#include <iostream>
#include <sstream>

#include "HttpMessage.h"

using std::string;
using std::cout;
using std::endl;
using std::ios;
using std::ifstream;

vector<char> getFileBuffer(string filePath);
void handleRequest(string fileDir, int clientSockfd);

string getIP(string host){
	struct addrinfo hints;
	  struct addrinfo* res;

	  // prepare hints
	  memset(&hints, 0, sizeof(hints));
	  hints.ai_family = AF_INET; // IPv4
	  hints.ai_socktype = SOCK_STREAM; // TCP

	  // get address
	  int status = 0;
	  if ((status = getaddrinfo(host.c_str(), "80", &hints, &res)) != 0) {
	    std::cerr << "getaddrinfo: " << gai_strerror(status) << std::endl;
	  }

	  std::cout << "IP addresses for " << host << ": " << std::endl;

	  for(struct addrinfo* p = res; p != 0; p = p->ai_next) {
	    // convert address to IPv4 address
	    struct sockaddr_in* ipv4 = (struct sockaddr_in*)p->ai_addr;

	    // convert the IP to a string and print it:
	    char ipstr[INET_ADDRSTRLEN] = {'\0'};
	    inet_ntop(p->ai_family, &(ipv4->sin_addr), ipstr, sizeof(ipstr));
	    std::cout << "  " << ipstr << std::endl;
	    // std::cout << "  " << ipstr << ":" << ntohs(ipv4->sin_port) << std::endl;
	    return string(ipstr);
	  }

	  freeaddrinfo(res); // free the linked list
	  return "";
}

int main(int argc, char *argv[])
{
	string hostname;
	int port;
	string fileDir;
	string ip;

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

	ip = getIP(hostname);

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
	addr.sin_addr.s_addr = inet_addr(ip.c_str());
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
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	std::vector<char> buffer(size);
	file.read(buffer.data(), size);
	//buffer.back() = '\0';
	return buffer;
}

void handleRequest(string fileDir, int clientSockfd){
	//receive the http request
	bool done = false;
	string fullRequest = "";
	int numcalls = 0;
	while(!done){
		size_t MAX_MSG_SIZE = 10000;
		void* buf[MAX_MSG_SIZE];
		memset(buf, 0, MAX_MSG_SIZE);
		std::stringstream ss;
		memset(buf, '\0', sizeof(buf));
		if (recv(clientSockfd, buf, MAX_MSG_SIZE, 0) == -1) {
			perror("recv");
		}
		fullRequest += std::string((char*)buf);
		if(fullRequest.size() >= 2 && fullRequest.substr(fullRequest.length() - 2) == "\n\n") {
			done = true;
		}
		if(fullRequest.size() >= 4 && fullRequest.substr(fullRequest.length() - 4) == "\r\n\r\n") {
			done = true;
		}
		numcalls++;
	}

	HttpRequest request;
	bool goodRequest = request.consumeMessage(fullRequest);
	cout << "HTTP Request by client:" << endl;
	cout << fullRequest << endl;

	//get the requested file
	HttpResponse response;
	response.setVersion("HTTP/1.0");
	if(request.path_ == "/"){
		request.path_ = "/index.html";
	}
	string fullFilePath = fileDir + request.path_;
	if(!goodRequest){
		cout << "400 Bad Request" << endl;
		response.setStatus("400 Bad Request");
		response.setLength("0");
		string msg = response.createMessage();
		if (send(clientSockfd, msg.c_str(), msg.size(), 0) == -1){
				perror("send");
		}

	} else if(ifstream(fullFilePath, ios::binary|ios::ate)){
		vector<char> fileBytes = getFileBuffer(fileDir + request.path_);
		cout << fileBytes.size() << endl;
		cout << "file bytes size " << fileBytes.size() << endl;

		response.setStatus("200 OK");
		response.setLength(std::to_string(fileBytes.size())); //-1 because of null terminator
		string httpMsg = response.createMessage();

		vector<char> httpMsgBytes(httpMsg.begin(), httpMsg.end());
		cout << "header size " << httpMsgBytes.size() << endl;

		httpMsgBytes.insert(httpMsgBytes.end(), fileBytes.begin(), fileBytes.end());
		cout << "total bytes size " << httpMsgBytes.size() << endl;

		string msg = response.createMessage();
		if (send(clientSockfd, httpMsgBytes.data(), httpMsgBytes.size(), 0) == -1){
				perror("send");
		}

	} else {
		cout << "404 Not Found" << endl;
		response.setStatus("404 Not Found");
		response.setLength("0");
		string msg = response.createMessage();
		if (send(clientSockfd, msg.c_str(), msg.size(), 0) == -1){
				perror("send");
		}
	}

	close(clientSockfd);
}
