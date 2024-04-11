#include "barrier.h"

namespace synchronization
{
   
   barrier::barrier( int numberOfThreads ) {
      // Write your code here
      count = numberOfThreads;
      sem_init(&mutex, 0, 1);
      sem_init(&barrier_sem, 0, 0);

      // return;
   }

   barrier::~barrier( ) {
      // Write your code here
      sem_destroy(&mutex);
      sem_destroy(&barrier_sem);
      // return;
   }

   void barrier::arriveAndWait() {
      sem_wait(&mutex); // lock the mutex
      count--; // decrement the count
      if (count == 0) { // if all threads have arrived
         for (int i = 0; i < numberOfThreads; i++) {
            sem_post(&barrier_sem); // signal all waiting threads
         }
         count = numberOfThreads; // reset the count for reuse
      }
      sem_post(&mutex); // unlock the mutex
      sem_wait(&barrier_sem); // wait at the barrier
   }

}
