#ifndef BARRIER_H
#define BARRIER_H


#include <semaphore.h>

namespace synchronization
{


   // Provides a reusable barrier
   class barrier {
   private:
      // Declare your variables here
      sem_t mutex;
      sem_t gate1;
      sem_t gate2;
      int count;

   public:

      // Constructor
      barrier( int numberOfThreads );

      // Destructor
      ~barrier( );

      // Function to wait at the barrier until all threads have reached the barrier
      void arriveAndWait(int numberOfThreads );
   };

}

#endif
