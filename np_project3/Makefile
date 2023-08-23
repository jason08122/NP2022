CXX=g++
CXXFLAGS=-std=c++14 -Wall -pedantic -pthread -lboost_system -lboost_filesystem
CXX_INCLUDE_DIRS=/usr/local/include
CXX_INCLUDE_PARAMS=$(addprefix -I , $(CXX_INCLUDE_DIRS))
CXX_LIB_DIRS=/usr/local/lib
CXX_LIB_PARAMS=$(addprefix -L , $(CXX_LIB_DIRS))

all: 

part1:
	$(CXX) http_server.cpp -o http_server $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)
	$(CXX) console.cpp -o console.cgi $(CXX_INCLUDE_PARAMS) $(CXX_LIB_PARAMS) $(CXXFLAGS)

part2:
	g++ cgi_server.cpp -o cgi_server -lws2 32 -lwsock32 -std=c++14
	
	
clean:
	rm -f http_server
	rm -f console.cgi
	rm -f cgi_server