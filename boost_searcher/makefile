PARSER=parser
DUG=debug
HTTP_SERVER=http_server
cc=g++

.PHONY:all
all:$(PARSER) $(DUG) $(HTTP_SERVER)

$(PARSER):parser.cc
	$(cc) -o $@ $^ -std=c++11 -lboost_system -lboost_filesystem
$(DUG):debug.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp
$(HTTP_SERVER):http_server.cc
	$(cc) -o $@ $^ -std=c++11 -ljsoncpp -lpthread

.PHONY:clean
clean:
	rm $(PARSER) $(DUG) $(HTTP_SERVER)