#include "potato.h"

#include <iostream>

int Potato::get_hops(){
  return this->hops;
}

void Potato::decrement_hop(){
  this->hops --;
}

void Potato::append_route(int newId){
  this->route[this->counter] = newId;
  this->counter++;
}

void Potato::print_trace(){
  std::cout << "Trace of potato:" << std::endl;
  for(int i = 0; i < this->counter - 1; i++){
    std::cout << this->route[i] << ",";
  }
  std::cout << this->route[this->counter - 1] << std::endl;
}
