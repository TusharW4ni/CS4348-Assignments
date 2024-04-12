#include "barrier.h"


namespace synchronization
{
   // Constructor
   barrier::barrier( int numberOfThreads ) {
      // Initializing mutex to guard the count and let only one thread to change it at a time
      sem_init(&mutex, 0, 1);
      // Initializing gate1 to block threads initially
      sem_init(&gate1, 0, 0);
      // Initializing gate2 to allow one thread to pass
      sem_init(&gate2, 0, 1);
      return;
   }

   // Destructor
   barrier::~barrier( ) {
      sem_destroy(&mutex);
      sem_destroy(&gate1);
      sem_destroy(&gate2);
      return;
   }

   void barrier::arriveAndWait(int numberOfThreads ) {
      //-----------Part One----------------//
      sem_wait(&mutex);                    // Acquire the lock to increment the count
      count++;
      if (count == numberOfThreads) {      // If all threads have arrived
        sem_wait(&gate2);                  // Allow one thread to pass because initialized to 1. Also only one thread will arrive here.
        sem_post(&gate1);                  // Unblock one thread that is waiting on the gate1
      }
      sem_post(&mutex);                    // Release the lock for changing count
      sem_wait(&gate1);                    // Wait for all threads to arrive
      sem_post(&gate1);                    // Unblock the next thread
      //-----------Part Two----------------//
      /*
         Without this second part, which is practically the same thing as the first part, the barrier would not be resusable. After all threads have passed the barrier once, the count variable would remain at the value of numberOfThreads and gate1 would remain blocked. So, no thread would be able to pass the barrier again.
      */
      sem_wait(&mutex); 
      count--;
      if (count == 0) {
        sem_wait(&gate1);
        sem_post(&gate2);
      }
      sem_post(&mutex);
      sem_wait(&gate2);
      sem_post(&gate2);
      //----------------------------------//
      return;
   }

}
