#ifndef __PLAYER_H__
#define __PLAYER_H__

#include "server.h"
#include "potato.h"

/**
 * @brief player class
 * 
 */
class Player: public Server{
  private:
    int id;
    int masterSocketFd;
    int acceptedFd;
    int numPlayers;
    int connInfoLeft;
    std::string ipLeft;
    int portLeft;

  public:
    static void validate_args(int argc, char** argv);

    Player(const char* hostname, const char* port): 
      Server(hostname, NULL),
      id(-1),
      numPlayers(-1),
      connInfoLeft(-1),
      portLeft(-1)      
      {
        this->masterSocketFd = this->create_socket_and_connect(hostname, port); // connect to ringmaster
        this->acquire_self_info_from_master(); // get player id & total player from master
        this->acquire_neighbor_info_from_master(); // get left neighbor info from master
        this->connect_and_accept_neighbors(); // connect to left neighbor
      }

    virtual ~Player();

    void acquire_self_info_from_master();
    void acquire_neighbor_info_from_master();
    void connect_and_accept_neighbors();
    void try_send_receive_neighbor_messages();
    void play_potato();
};

#endif