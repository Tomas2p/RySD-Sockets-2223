CC = g++
CXXFLAGS = -std=c++17 -lpthread -Wall -o 
OBJ = ftp_server
OBJALL = src/ClientConnection.cpp src/${OBJ}.cpp src/FTPServer.cpp

all: clean ${OBJ}

${OBJ}: 
	${CC} ${CXXFLAGS} $@ ${OBJALL}

clean: 
	rm -f ${OBJ}