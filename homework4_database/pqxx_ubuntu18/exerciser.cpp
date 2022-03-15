#include "exerciser.h"

void exercise(connection *C)
{
  query1(C, 
    1, 33, 40, 
    1, 15, 25, 
    1, 3, 8, 
    1, 0, 4, 
    1, 0.3, 1.5, 
    1, 0.6, 2.2
  );

  query2(C, "Orange");
  query3(C, "Clemson");
  query4(C, "MA", "Maroon");
  query5(C, 10);
}
