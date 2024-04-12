#ifndef BARRIER_H
#define BARRIER_H

#include <semaphore.h>

namespace synchronization
{
   // Provides a reusable barrier
   class barrier {
   private:
      sem_t mutex; // Semaphore to guard the count
      sem_t gate1; // Semaphore to block threads initially
      sem_t gate2; // Semaphore to allow one thread to pass initially
      int count;   // Count of threads that have arrived at the barrier

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
