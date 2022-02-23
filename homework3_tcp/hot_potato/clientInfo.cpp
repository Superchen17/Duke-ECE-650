#include "clientInfo.h"

ClientInfo::~ClientInfo(){
  this->clientFd = -1;
  this->clientAddr = "";
  this->clientPort = -1;
}

int ClientInfo::get_clientFd(){
  return this->clientFd;
}

std::string ClientInfo::get_clientAddr(){
  return this->clientAddr;
}

int ClientInfo::get_clientPort(){
  return this->clientPort;
}

void ClientInfo::set_clientFd(int _clientFd){
  this->clientFd = _clientFd;
}

void ClientInfo::set_clientAddr(int _clientAddr){
  this->clientAddr = _clientAddr;
}

void ClientInfo::set_clientPort(int _clientPort){
  this->clientPort = _clientPort;
}
