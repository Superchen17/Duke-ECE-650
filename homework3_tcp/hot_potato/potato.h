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
    Potato();
    Potato(int _hops);
    Potato(const Potato& rhs);

    int get_hops();
    void decrement_hop();
    void append_route(int newId);
    void print_trace();
};

#endif