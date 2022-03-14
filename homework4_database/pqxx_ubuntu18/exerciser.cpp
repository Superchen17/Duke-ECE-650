#include "exerciser.h"

void exercise(connection *C)
{
  query1(C, 
    1, 35, 40, 
    0, 0, 0, 
    0, 5, 10, 
    0, 0, 0, 
    0, 0, 0, 
    0, 0, 0
  );

  query2(C, "Orange");
  query3(C, "Clemson");
  query4(C, "MA", "Maroon");
  query5(C, 10);
}
