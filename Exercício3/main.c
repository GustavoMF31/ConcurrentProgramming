#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<math.h>

#include<semaphore.h>
#include<pthread.h>

// Constant during the program lifetime, once initialized in main:
int max_int; // Number of integers for which we will check primality
int consumers; // Number of consumer threads
int capacity; // Capacity of the channel for comunication between threads
int* buffer; // Channel buffer (constant pointer, values it points to change)

// Global mutable variables:

// The index of the first filled entry in the buffer.
// (Elements are consumed from left to right)
// 0 <= buffer_next <= capacity;
int buffer_next;

// Binary semaphore which tracks whether buffer_next == capacity
sem_t buffer_empty;

// Semaphore for consumer threads.
// Has a signal for each available element in the buffer plus a signal
// for each consumer thread in case the channel has been closed. In this case,
// the accompanying buffer entry is set to -1.
sem_t has_buffer_entry;

// Binary semaphore which protects the shared variables buffer[i] and buffer_next.
sem_t buffer_access;

void* integer_producer(void* args){
  int i = 0; // First integer which was not yet pushed onto the channel
  int closed = 0; // How many close signals have been sent

  // Loop while there is still work to do: push more integers or more close signals
  while (i < max_int || closed < consumers){
    sem_wait(&buffer_empty);
    sem_wait(&buffer_access);

    // Sanity check assertion
    if (buffer_next != capacity) {
      puts("Panic: buffer_empty semaphore has signal but buffer_next != capacity");
      exit(1);
    }

    buffer_next = 0;

    // Push more integers or close signals onto the channel
    for (int j = 0; j < capacity && (i < max_int || closed < consumers); j++){
      if (i < max_int) {
        buffer[j] = i++;
      } else {
        // Writing -1 as an element signals that the consumer thread should halt
        // (there will be no more integers produced), which must be done for
        // every consumer thread
        buffer[j] = -1;
        closed++;
      }

      sem_post(&has_buffer_entry);
    }

    sem_post(&buffer_access);
  }

  // Once every integer < max_int has been produced and every close signal sent,
  // the producer thread may halt
  return NULL;
}

int is_prime(long long int n) {
  if (n == 2) return true;
  if (n <= 1 || n % 2 == 0) return false;

  for (int i = 3; i < sqrt(n) + 1; i += 2)
    if(n % i == 0) return 0;

  return 1;
}

// Primality tester consumer thread
void* primality_tester(void* args){
  long int primes_found = 0;

  while (1) {
    sem_wait(&has_buffer_entry);

    // Read our entry to be proccessed
    sem_wait(&buffer_access);
    int i = buffer[buffer_next++];
    if (buffer_next == capacity) sem_post(&buffer_empty);
    sem_post(&buffer_access);

    // An entry of -1 signals that the channel is being closed
    if (i == -1) break;

    primes_found += is_prime(i);
  }

  // Return the number of primes found
  return (void*) primes_found;
}

int main(int argc, char** argv){
  if (argc != 4){
    printf("Usage: %s max_int buffer_capacity consumers\n", argv[0]);
    exit(1);
  }

  // Initialize the global variables

  // It would be appropriate to check for parse errors
  max_int = atoi(argv[1]);
  capacity = atoi(argv[2]);
  consumers = atoi(argv[3]);

  // Validate inputs
  if (capacity <= 0){
    puts("Channel capacity must be at least 1");
    return 1;
  }

  if (max_int <= 0){
    puts("Number of integers to test should be greater than zero");
    return 1;
  }

  if (consumers <= 0){
    puts("At least one consumer thread is required");
    return 1;
  }

  buffer = malloc(capacity * sizeof(int));
  if (buffer == NULL){
    puts("Failed to allocate buffer");
    return 1;
  }

  // The buffer starts empty
  buffer_next = capacity;

  sem_init(&buffer_empty, 0, 1); // The buffer is empty on startup,
  sem_init(&has_buffer_entry, 0, 0); // has no entries
  sem_init(&buffer_access, 0, 1); // and access is available

  // Spawn the producer thread
  pthread_t producer_id;
  bool err =  pthread_create(&producer_id,  NULL, integer_producer, NULL);
  if (err){
    puts("Failed to create producer thread");
    return 1;
  }

  // Spawn the consumer threads
  pthread_t consumer_ids[consumers];
  for (int i = 0; i < consumers; i++){
    bool err = pthread_create(&consumer_ids[i], NULL, primality_tester, NULL);
    if (err){
      puts("Failed to create a consumer thread");
      return 1;
    }
  }

  // Wait for the consumers to finish.
  // (Waiting on the producer is unnecessary, as the halting of all consumers implies
  // the producer has halted too).
  int total_primes = 0;
  int winner_thread; // Index of the consumer thread that has found the most primes
  int winner_count = -1; // And how many primes it found
  for (int i = 0; i < consumers; i++){
    long int primes_found;
    bool err = pthread_join(consumer_ids[i], (void**) &primes_found);
    if (err){
      puts("Error when attempting to join a consumer thread");
      return 1;
    }

    total_primes += primes_found;

    // Track the winner thread
    if (primes_found > winner_count){
      winner_thread = i;
      winner_count = primes_found;
    }
  }

  printf("Found %d primes\n", total_primes);
  printf("The thread that found the most primes was consumer thread %d\n", winner_thread);

  // Free resources and exit
  free(buffer);
  sem_destroy(&buffer_empty);
  sem_destroy(&has_buffer_entry);
  sem_destroy(&buffer_access);

  return 0;
}
