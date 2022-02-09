#include "player.h"
#include <stdio.h>

void Player::validate_args(int argc, char** argv){
  // check number of args
  if(argc != 3){
    throw CustomException("error: wrong number of arguments, expecting 2");
  }

  // check port number 
  int port = atoi(argv[2]);
  if(port < 1 || port > 65535){
    throw CustomException("error: port must be between 1 and 65535");
  }
}

Player::~Player(){
  close(this->masterSocketFd);
  close(this->acceptedFd);
  close(this->connInfoLeft);
}

void Player::acquire_self_info_from_master(){
  this->try_recv(this->masterSocketFd, &(this->id), sizeof(this->id), 0);
  this->try_recv(this->masterSocketFd, &(this->numPlayers), sizeof(this->numPlayers), 0);

  int myPort = this->get_my_port();
  this->try_send(this->masterSocketFd, &myPort, sizeof(myPort), 0);

  std::string myIp = this->get_my_ip();
  this->try_send(this->masterSocketFd, myIp.c_str(), 100, 0);

  std::cout << "Connected as player " << this->id 
    << " out of " << this->numPlayers << " total players\n";
}

void Player::acquire_neighbor_info_from_master(){
  char ip[100];

  this->try_recv(this->masterSocketFd, &(this->connInfoLeft), sizeof(this->connInfoLeft), 0);
  this->try_recv(this->masterSocketFd, ip, 100, 0);
  this->ipLeft = std::string(ip);
  this->try_recv(this->masterSocketFd, &(this->portLeft), sizeof(this->portLeft), 0);
}

void Player::connect_and_accept_neighbors(){
  char portStr[6];
  sprintf(portStr, "%d", this->portLeft);

  this->connInfoLeft = this->create_socket_and_connect(this->ipLeft.c_str(), portStr);
  this->acceptedFd = this->accept_connection();
}

void Player::try_send_receive_neighbor_messages(){

  std::cout << this->acceptedFd << ", " << this->connInfoLeft << std::endl;

  int msgLeft = 0;
  int msgRight = 1;
  int recvLeft;
  int recvRight;

  if(this->id == 1){
    this->try_send(this->acceptedFd, &msgLeft, sizeof(msgLeft), 0);
  }
  if(this->id == 2){
    this->try_recv(this->connInfoLeft, &recvLeft, sizeof(recvLeft), 0);
  }
  std::cout << recvLeft << ", " << recvRight << std::endl;
}

void Player::play_potato(){
  fd_set readfds;
  int leftId, rightId;
  if(this->id == 0){
    leftId = this->numPlayers - 1;
  }
  else{
    leftId = this->id - 1;
  }
  if(this->id == this->numPlayers - 1){
    rightId = 0;
  }
  else{
    rightId = this->id + 1;
  }

  // fds = [right, left, master]
  int fds[] = {this->acceptedFd, this->connInfoLeft, this->masterSocketFd};
  int maxFd = this->highestFd + 1;

  while (true) {
    Potato potato; 
    FD_ZERO(&readfds);
    for (int i = 0; i < 3; i++) {
      FD_SET(fds[i], &readfds);
    }

    int status = select(maxFd, &readfds, NULL, NULL, NULL);
    if (status == -1) {
      throw CustomException("error: cannot select");
    }

    for (int i = 0; i < 3; i++) {
      if (FD_ISSET(fds[i], &readfds)) {
        this->catch_potato(fds[i], potato);
        break;
      }
    }

    // check if ringmaster sends a endpotato
    if (potato.get_hops() == 0) {
      break;
    }

    // decrement hop, append route
    potato.decrement_hop();
    potato.append_route(this->id);

    // if hop = 0, toss back to master
    if (potato.get_hops() == 0) {
      std::cout << "I\'m it" << std::endl;
      this->toss_potato(this->masterSocketFd, potato);
      break;
    }

    // if still hops left, toss to left/right player
    srand((unsigned int)time(NULL) + this->id);
    int randnum = rand() % 2;
    if (randnum == 0) {
      std::cout << "Sending potato to " << rightId << std::endl;
      this->toss_potato(this->acceptedFd, potato);
    }
    else {
      std::cout << "Sending potato to " << leftId << std::endl;
      this->toss_potato(this->connInfoLeft, potato);
    }
  }
}

int main(int argc, char** argv){
  try{
    Player::validate_args(argc, argv);
    Player* p = new Player(argv[1], argv[2]);
    p->play_potato();
    delete p;
  }
  catch(CustomException& e){
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
