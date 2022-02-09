#include "server.h"

#include <stdio.h>
#include <arpa/inet.h>
#include <string>

Server::~Server(){
  close(this->socketFd);
}

std::string Server::get_my_ip(){
  sockaddr_in_t s;
  socklen_t len = sizeof(s);
  int status = getsockname(this->socketFd, (sockaddr_t*)&s, &len);
  if(status == -1){
    throw CustomException("error: cannot get host port");
  }
  return std::string(inet_ntoa(s.sin_addr));
}

void Server::try_send(int sockfd, const void *msg, int len, int flags){
  int status = send(sockfd, msg, len, flags);
  if(status != len){
    throw CustomException("error: sent unexpected message");
  }
}

void Server::try_recv(int sockfd, void *buf, int len, int flags){
  int status = recv(sockfd, buf, len, flags);
  if(status != len && status != 0){
    throw CustomException("error: received unexpected message");
  }
}

int Server::get_my_port(){
  sockaddr_in_t s;
  socklen_t len = sizeof(s);
  int status = getsockname(this->socketFd, (sockaddr_t*)&s, &len);
  if(status == -1){
    throw CustomException("error: cannot get host port");
  }
  return ntohs(s.sin_port);
}

void Server::create_addrInfo(const char* _hostname, const char* _port, addrinfo_t** hostInfoList){
  addrinfo_t hostInfo;

  memset(&hostInfo, 0, sizeof(hostInfo));
  hostInfo.ai_family = AF_UNSPEC;
  hostInfo.ai_socktype = SOCK_STREAM;
  hostInfo.ai_flags = AI_PASSIVE;

  int status = getaddrinfo(
    _hostname,
    _port,
    &(hostInfo),
    hostInfoList
  );

  if(_port == NULL){
      this->get_random_port(hostInfoList);
  }

  if(status != 0){
    throw CustomException("error: cannot get address info for host");
  }
}

int Server::create_socket(addrinfo_t* hostInfoList){
  int newSocket = socket(
    hostInfoList->ai_family,
    hostInfoList->ai_socktype,
    hostInfoList->ai_protocol
  );
  if(newSocket == -1){
    throw CustomException("error: cannot create socket");
  }

  return newSocket;
}

void Server::bind_socket(addrinfo_t* hostInfoList){
  int yes = 1;
  int status = setsockopt(
    this->socketFd, 
    SOL_SOCKET, 
    SO_REUSEADDR, 
    &yes, 
    sizeof(yes)
  );
  status = bind(
    this->socketFd,
    hostInfoList->ai_addr,
    hostInfoList->ai_addrlen
  );
  if(status == -1){
    throw CustomException("error: cannot bind socket");
  }
}

void Server::try_listen(){
  int status = listen(this->socketFd, 100);
  if(status == -1){
    throw CustomException("error: cannot listen on socket");
  }
}

int Server::accept_connection(){

  struct sockaddr_storage socketAddr;
  socklen_t socketAddrLen = sizeof(socketAddr);
  int clientConnFd = accept(this->socketFd, (sockaddr_t*)&socketAddr, &socketAddrLen);
  if(clientConnFd == -1){
    throw CustomException("error: cannot accept connection on socket");
  }

  this->highestFd = clientConnFd;
  return clientConnFd;
}

void Server::try_connect(int targetFd, const char* hostname, const char* port){
  addrinfo_t* hostInfoList;  

  this->create_addrInfo(hostname, port, &hostInfoList);

  int status = connect(
    targetFd,
    hostInfoList->ai_addr,
    hostInfoList->ai_addrlen
  );
  if(status == -1){
    throw CustomException("error: cannot connect to socket");
  }
}

void Server::create_socket_and_listen(const char* hostname, const char* port){
  addrinfo_t* hostInfoList = NULL;

  this->create_addrInfo(hostname, port, &hostInfoList);
  this->socketFd = this->create_socket(hostInfoList);
  this->bind_socket(hostInfoList);
  this->try_listen();

  freeaddrinfo(hostInfoList);
}

int Server::create_socket_and_connect(const char* hostname, const char* port){
  addrinfo_t* hostInfoList = NULL;

  this->create_addrInfo(hostname, port, &hostInfoList);
  int receivingSocketFd = this->create_socket(hostInfoList);

  int status = connect(
    receivingSocketFd,
    hostInfoList->ai_addr,
    hostInfoList->ai_addrlen
  );
  if(status == -1){
    throw CustomException("error: cannot connect to socket");
  }

  freeaddrinfo(hostInfoList);

  return receivingSocketFd;
}

void Server::get_random_port(addrinfo_t** hostInfoList){
  sockaddr_in_t* addr = (sockaddr_in_t*)((*hostInfoList)->ai_addr);
  addr->sin_port = 0; 
}


void Server::toss_potato(int fd, Potato& potato){
  int status = send(fd, &potato, sizeof(potato), 0);
  if (status != sizeof(potato)) {
    throw CustomException("error: sent unexpected potato");
  }
}

void Server::catch_potato(int fd, Potato& potato){
  int status = recv(fd, &potato, sizeof(potato), 0);
  if (status != sizeof(potato) && status != 0) {
    throw CustomException("error: received unexpected potato");
  }
}
