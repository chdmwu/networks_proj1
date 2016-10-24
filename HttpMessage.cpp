/*
 * httprequest.cpp
 *
 *  Created on: Oct 23, 2016
 *      Author: christopher
 */

#include "HttpMessage.h"
#include <sstream>
#include <iostream>


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

void HttpMessage::consumeMessage(string msg){

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

void HttpRequest::consumeMessage(string msg){
	istringstream ss(msg);
	string to;
	std::getline(ss,to,'\n');
	//get first line
	istringstream lineStream(to);
	getline(lineStream, method_, ' ');
	getline(lineStream, path_, ' ');
	getline(lineStream, version_, ' ');
	//get 2nd line
	std::getline(ss,to,'\n');
	istringstream lineStream2(to);
	getline(lineStream2, host_, ' '); //skip the first thing
	getline(lineStream2, host_, ' ');
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
	message += body_ + "\n";
	message += "\n";
	return message;
}


void HttpResponse::consumeMessage(string msg){
}

