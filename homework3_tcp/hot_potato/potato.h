#ifndef __POTATO_H__
#define __POTATO_H__

#include <string.h>

#define MAX_HOPS 512

class Potato {
  private:
    int hops;
    int counter;
    int route[MAX_HOPS];

  public:
    Potato():hops(0), counter(0){ 
      memset(this->route, 0, MAX_HOPS); 
    }

    Potato(int _hops): hops(_hops), counter(0){
      memset(this->route, 0, MAX_HOPS); 
    }

    int get_hops();

    void decrement_hop();
    void append_route(int newId);
    void print_trace();
};

#endif