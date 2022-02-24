#include "ringmaster.h"
#include "clientInfo.h"

Ringmaster::~Ringmaster(){
  for(int i = 0; i < numPlayers; i++){
    close(this->connInfos[i]);
  }
}

void Ringmaster::create_hub(){
  for(int i = 0; i < this->numPlayers; i++){
    ClientInfo* clientInfo = this->accept_connection();
    int playerFd = clientInfo->get_clientFd();

    this->connInfos.push_back(playerFd);
    this->try_send(playerFd, &(playerCounter), sizeof(playerCounter), 0);
    this->try_send(playerFd, &(this->numPlayers), sizeof(this->numPlayers), 0);

    int _port; 
    this->try_recv(playerFd, &(_port), sizeof(_port), MSG_WAITALL);
    this->connPorts.push_back(_port);

    this->connIps.push_back(clientInfo->get_clientAddr());
    delete clientInfo;

    std::cout << "Player " << playerCounter << " is ready to play" << std::endl;

    playerCounter ++;
  }
}

void Ringmaster::send_neighbor_info(){
  for(int i = 0; i < this->numPlayers; i++){
    int leftConnInfo, leftPort;
    std::string leftIp;
    if(i == 0){
      leftConnInfo = this->connInfos[this->numPlayers-1];
      leftIp = this->connIps[this->numPlayers-1];
      leftPort = this->connPorts[this->numPlayers-1];
    }
    else{
      leftConnInfo = this->connInfos[i - 1];
      leftIp = this->connIps[i - 1];
      leftPort = this->connPorts[i - 1];
    }

    const char* leftIpChars = leftIp.c_str();
    this->try_send(this->connInfos[i], &leftConnInfo, sizeof(leftConnInfo), 0);   
    this->try_send(this->connInfos[i], leftIpChars, 100, 0);    
    this->try_send(this->connInfos[i], &leftPort, sizeof(leftPort), 0);
  }
}

void Ringmaster::play_game(){
  Potato potato(this->potatoHops);
  int status;

  // if total hops = 0, signal players to end game immediately
  if (this->potatoHops == 0) {
    for (int i = 0; i < this->numPlayers; i++) {
      this->toss_potato(this->connInfos[i], potato);
    }
    return;
  }

  // send to a random player
  srand((unsigned int)time(NULL));
  int randId = rand() % this->numPlayers;
  std::cout << "Ready to start the game, sending potato to player " << randId << std::endl;
  this->toss_potato(this->connInfos[randId], potato);

  // create select from all players
  fd_set fdSet;
  FD_ZERO(&fdSet);
  for (int i = 0; i < this->numPlayers; i++) {
    FD_SET(this->connInfos[i], &fdSet);
  }
  status = select(this->connInfos[this->numPlayers - 1] + 1, &fdSet, NULL, NULL, NULL);
  if(status == -1){
    throw CustomException("error: cannot select");
  }

  // wait for potato from "it"
  for (int i = 0; i < this->numPlayers; i++) {
    if (FD_ISSET(this->connInfos[i], &fdSet)) {
      this->catch_potato(this->connInfos[i], potato);
      break;
    }
  }
  potato.print_trace();

  // signal all players to close
  Potato endpotato;
  for (int i = 0; i < this->numPlayers; i++) {
    this->toss_potato(this->connInfos[i], endpotato);
  }
}

void Ringmaster::validate_args(int argc, char** argv){
  // check number of args
  if(argc != 4){
    throw CustomException("error: wrong number of arguments, expecting 3");
  }

  // check port number 
  int port = atoi(argv[1]);
  if(port < 1 || port > 65535){
    throw CustomException("error: port must be between 1 and 65535");
  }

  //check num players
  int numPlayers = atoi(argv[2]);
  if(numPlayers < 2){
    throw CustomException("error: number of players must be bigger than 1");
  }

  // check hops
  int numHops = atoi(argv[3]);
  if(numHops < 0 || numHops > 512){
    throw CustomException("error: hops must be between 0 and 512");
  }
}
    
int Ringmaster::playerCounter = 0;

int main(int argc, char** argv){
  try{
    Ringmaster::validate_args(argc, argv);
    Ringmaster* r = new Ringmaster("0.0.0.0", argv[1], atoi(argv[2]), atoi(argv[3]));
    r->play_game();
    delete r;
  }
  catch(CustomException& e){
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}