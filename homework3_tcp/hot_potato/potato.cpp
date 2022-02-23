#include "potato.h"

#include <iostream>

Potato::Potato(){
  this->hops = 0;
  this->counter = 0;
  for(int i = 0; i < MAX_HOPS; i++){
    this->route[i] = 0;
  }
}

Potato::Potato(int _hops){
  this->hops  = _hops;
  this->counter = 0;
  for(int i = 0; i < MAX_HOPS; i++){
    this->route[i] = 0;
  }
}

Potato::Potato(const Potato& rhs){
  this->hops = rhs.hops;
  this->counter = rhs.counter;
  for(int i = 0; i < MAX_HOPS; i++){
    this->route[i] = rhs.route[i];
  }
}

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
