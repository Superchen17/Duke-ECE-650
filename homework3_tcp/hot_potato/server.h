#ifndef __SERVER_H__
#define __SERVER_H__

#include "potato.h"
#include "clientInfo.h"

#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>

typedef struct addrinfo addrinfo_t;
typedef struct sockaddr sockaddr_t;
typedef struct sockaddr_in sockaddr_in_t;

/**
 * @brief custom exception class
 * 
 */
class CustomException: public std::exception{
  private:
    const char* errMsg;

  public:
    CustomException(): errMsg("error: something\'s wrong"){}
    CustomException(const char* _errMsg): errMsg(_errMsg){}
    virtual const char* what() const throw() {
      return this->errMsg;
    }
};

/**
 * @brief server class
 * create a basic server
 * 
 */
class Server{
  protected:
    int socketFd;
    int highestFd;

  private:
    void create_addrInfo(const char* _hostname, const char* _port, addrinfo_t** hostInfoList);
    int create_socket(addrinfo_t* hostInfoList);
    void bind_socket(addrinfo_t* hostInfoList);
    void try_listen();
    void get_random_port(addrinfo_t** hostInfoList);
  
  public:
    Server(const char* hostname, const char* port):socketFd(-1),highestFd(-1){
      this->create_socket_and_listen(hostname, port);
    }  
    
    virtual ~Server();
    
    std::string get_my_ip();
    int get_my_port();

    void try_send(int sockfd, const void *msg, int len, int flags);
    void try_recv(int sockfd, void *buf, int len, int flags);

    void try_connect(int targetFd, const char* hostname, const char* port);
    ClientInfo* accept_connection();

    void create_socket_and_listen(const char* hostname, const char* port);
    int create_socket_and_connect(const char* hostname, const char* port);    

    void toss_potato(int fd, Potato& potato);
    void catch_potato(int fd, Potato& potato);
};

#endif