#ifndef __CLIENTINFO_H__
#define __CLIENTINFO_H__

#include <iostream>

class ClientInfo{
  private:
    int clientFd;
    std::string clientAddr;
    int clientPort;

  public:
    ClientInfo(int _clientFd, std::string _hostAddr, int _port):
      clientFd(_clientFd),
      clientAddr(_hostAddr),
      clientPort(_port){}
    ~ClientInfo();

    int get_clientFd();
    std::string get_clientAddr();
    int get_clientPort();
    void set_clientFd(int _clientFd);
    void set_clientAddr(int _clientAddr);
    void set_clientPort(int _clientPort);
};

#endif