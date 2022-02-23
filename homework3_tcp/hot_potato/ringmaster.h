#ifndef __RINGMASTER_H__
#define __RINGMASTER_H__

#include "server.h"
#include "potato.h"

#include <vector>

/**
 * @brief ringmaster class
 * 
 */
class Ringmaster: public Server{
  private:
    std::vector<int> connInfos;
    std::vector<std::string> connIps;
    std::vector<int> connPorts;
    int potatoHops;
    int numPlayers;  

    void get_max_fd();
    void create_hub();
    void send_neighbor_info();

  public:

    static int playerCounter;

    static void validate_args(int argc, char** argv);

    /**
     * @brief Construct a new Ringmaster object
     * @note superclass has 
     * 
     * @param hostname 
     * @param port 
     * @param numPlayers 
     */
    Ringmaster(const char* hostname, const char* port, int numPlayers, int potatoHops): 
      Server(hostname, port),
      numPlayers(numPlayers),
      potatoHops(potatoHops){
        std::cout << "Potato Ringmaster" << std::endl;
        std::cout << "Players = " << this->numPlayers << std::endl;
        std::cout << "Hops = " << this->potatoHops << std::endl;

        this->create_hub();  // get player socket id, host & port; assign player id
        this->send_neighbor_info(); // send each player its left neighbor's host & port
    }

    virtual ~Ringmaster();

    void play_game();

};

#endif