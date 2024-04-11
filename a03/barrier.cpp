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

   void barrier::arriveAndWait( void ) {
      // Write your code here
      sem_wait(&mutex);
      count --;
      sem_post(&mutex);
      if (count == 0) {
         for (int i = 0; i < count; i++) {
            sem_post(&barrier_sem);
         }
      }
      sem_wait(&barrier_sem);
      sem_post(&barrier_sem);
      return;
   }

}
