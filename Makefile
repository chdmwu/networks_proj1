CXX=g++
CXXOPTIMIZE= -O2
CXXFLAGS= -g -Wall -pthread -std=c++11 $(CXXOPTIMIZE)
USERID=EDIT_MAKE_FILE
CLASSES=
DEPS=

all: web-client web-server

web-client: HttpMessage.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp
	
web-server: HttpMessage.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $@.cpp

HttpMessage:
	$(CXX) -c $^ $(CXXFLAGS) $@.cpp
	
clean:
	rm -rf *.o *~ *.gch *.swp *.dSYM web-server web-client *.tar.gz

tarball: clean
	tar -cvf $(USERID).tar.gz *
