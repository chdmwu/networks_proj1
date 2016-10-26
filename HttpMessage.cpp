/*
 * httprequest.cpp
 *
 *  Created on: Oct 23, 2016
 *      Author: christopher
 */

#include "HttpMessage.h"
#include <sstream>
#include <iostream>
#include <fstream>

bool checkMethod(string str){
	return str == "GET";
}
bool checkPath(string str){
	return str.length() > 0;
}
bool checkVersion(string str){
	//cout << "version " << str << endl;
	//return ((str.compare("HTTP/1.0") == 0) || (str.compare("HTTP/1.1") == 0));
	return str.length() > 0;
}

bool isOK(string status){
	return status == "200 OK" || status == "200";
}

vector<char> getLineFromVec(vector<char> vec, int start){
	int curr = start;
	char lastChar = 'x';
	vector<char> line;
	while(lastChar != '\n'){
		lastChar = vec.at(curr);
		line.push_back(lastChar);
		curr++;
	}
	line.pop_back(); //delete trailing newline
	return line;
}

vector<string> split(string str, char delimiter) {
  vector<string> internal;
  stringstream ss(str); // Turn the string into a stream.
  string tok;

  while(getline(ss, tok, delimiter)) {
    internal.push_back(tok);
  }

  return internal;
}

HttpMessage::HttpMessage() {
	// TODO Auto-generated constructor stub
}

HttpMessage::~HttpMessage() {
	// TODO Auto-generated destructor stub
}

HttpRequest::HttpRequest() {
	// TODO Auto-generated constructor stub
}

HttpRequest::~HttpRequest() {
	// TODO Auto-generated destructor stub
}

HttpResponse::HttpResponse() {
	inBody_=false;
	OK_=false;
	bodySize_ = -1;
	// TODO Auto-generated constructor stub
}

HttpResponse::~HttpResponse() {
	// TODO Auto-generated destructor stub
}

void HttpMessage::setVersion(string version){
	version_ = version;
}

string HttpMessage::createMessage(){
	return "";
}

bool HttpMessage::consumeMessage(string msg){
	return false;
}

void HttpMessage::setBody(string body) {
	body_ = body;
}

void HttpRequest::setUrl(string url) {
	url_ = url;
}

void HttpRequest::setMethod(string method){
	method_ = method;
}

void HttpRequest::setHost(string host){
	host_ = host;
}

void HttpRequest::setPath(string path){
	path_ = path;
}

string HttpRequest::createMessage(){
	string message = method_ + " " + path_ + " " + version_ + "\n";
	message += "Host: " + host_+ "\n";
	message += "\n";
	if(body_.length() > 0){
		message += body_ + "\n";
		message += "\n";
	}
	return message;
}

bool HttpRequest::consumeMessage(string msg){
	istringstream ss(msg);
	string to;
	std::getline(ss,to,'\n');
	//get first line
	istringstream lineStream(to);
	getline(lineStream, method_, ' ');
	if(!checkMethod(method_)) { cout << "method fail" << endl; return false;}
	getline(lineStream, path_, ' ');
	if(!checkPath(path_)) {cout << "path fail" << endl; return false;}
	getline(lineStream, version_, ' ');
	if(!checkVersion(version_)) {cout << "version fail" << endl; return false;}
	//get header info
	while(getline(ss, to, '\n')){
		//do nothing? maybe check headers TODO
	}
	return true;
}



void HttpResponse::setStatus(string status){
	status_ = status;
}
void HttpResponse::setLength(string length){
	length_ = length;
}

string HttpResponse::createMessage(){
	string message = version_ + " " + status_ + "\n";
	message += "Content-Length: " + length_ + "\n";
	message += "\n";
	//if(body_.length() > 0){
	//	message += body_;
	//}
	return message;
}
bool HttpResponse::consumeMessage(string msg){
	return false;
}

//returns if this reponse is ok
bool HttpResponse::writeFile(vector<char> recved, string filePath){
	// deal with status
	vector<char> curr;
	int currInd = 0;
	if(!inBody_){
		if(!OK_){
			curr = getLineFromVec(recved, currInd);
			currInd += curr.size() + 1;
			string currStr = string(curr.begin(),curr.end());
			if( currStr == "HTTP/1.0 200 OK"){
				OK_ = true;
				remove(filePath.c_str()); //delete old file if it exists
			} else {
				return false;
			}
		}
	}
	while(curr.size()>0 && !inBody_){
		curr = getLineFromVec(recved, currInd);
		currInd += curr.size() + 1;
		string currStr = string(curr.begin(),curr.end());
		if(currStr.find("Content-Length:") != string::npos){
			vector<string> line = split(currStr, ' ');
			bodySize_ = std::stoi(line.at(1));
		}
	}
	if(curr.size() == 0) {
		inBody_ = true;
		if(bodySize_ == -1){
			//Didnt find body size.
			cout << "Http response lacks Content-Length header" << endl;
		}
	}
	if(inBody_){
		ofstream myfile;
		myfile.open(filePath.c_str(), std::ios_base::app | ios::binary | ios::out);
		myfile.write(recved.data() + currInd, recved.size() - currInd);
	}
	return true;
}
