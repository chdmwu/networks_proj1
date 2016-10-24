/*
 * httprequest.h
 *
 *  Created on: Oct 23, 2016
 *      Author: christopher
 */

#ifndef HTTPREQUEST_H_
#define HTTPREQUEST_H_

#include <string>
#include <vector>
using namespace std;
class HttpMessage {
public:
	HttpMessage();
	virtual ~HttpMessage();
	virtual string createMessage();
	virtual void consumeMessage(string msg);
	//virtual vector<uint8_t> encode() = 0;
	void setVersion(string version);
	void setBody(string body);

	string version_;
	string body_;
};


class HttpRequest : public HttpMessage{
public:
	HttpRequest();
	~HttpRequest();
	void setUrl(string url);
	void setMethod(string method);
	void setPath(string path);
	void setHost(string host);
	string createMessage() override;
	void consumeMessage(string msg) override;

	string url_;
	string method_;
	string path_;
	string host_;
};

class HttpResponse : public HttpMessage{
public:
	HttpResponse();
	~HttpResponse();
	void setStatus(string status);
	string createMessage() override;
	void consumeMessage(string msg) override;
	void setLength(string length);

	string status_;
	string length_;
};
#endif /* HTTPREQUEST_H_ */
