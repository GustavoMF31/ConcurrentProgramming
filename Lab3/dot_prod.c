#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <pthread.h>

typedef struct {
  float *v1, *v2;
  int start;
  int count;
} task_t;

void* thread_main(void* args){
  task_t task = * (task_t *) args;
  free(args);

  float total = 0;
  for (int i = task.start; i < task.start + task.count; i++)
    total += task.v1[i] * task.v2[i];

  float* ptr = malloc(sizeof(total));
  *ptr = total;
  return (void*) ptr;
}

int main(int argc, char** argv){
  if (argc != 3){
    printf("Usage: %s num-threads input.bin\n", argv[0]);
    exit(1);
  }

  // Number of threads
  int t = atoi(argv[1]);
  char* filename = argv[2];

  FILE* file = fopen(filename, "rb");
  if (file == NULL){
    printf("Failed to open file %s\n", filename);
    exit(1);
  }

  // Read in vector dimension and the content of the two vectors
  long int n;
  if (fread(&n, sizeof(n), 1, file) != 1){
    printf("Failed to read vector length from file %s\n", filename);
    exit(1);
  }

  float v1[n], v2[n];
  int b0 = fread(v1, sizeof(*v1), n, file);
  int b1 = fread(v2, sizeof(*v2), n, file);
  if (b0 != n || b1 != n){
    printf("Failed to read vector contents from file %s\n", filename);
    exit(1);
  }

  float seq_dot_prod;
  if (fread(&seq_dot_prod, sizeof(seq_dot_prod), 1, file) != 1){
    printf("Failed to read dot product result from file %s\n", filename);
    exit(1);
  }

  // Compute the dot product of the two vector sequentially
  pthread_t ids[t];
  for (int i = 0; i < t; i++){
    int block_size = n / t;

    task_t* task = malloc(sizeof(task));
    task->v1 = v1;
    task->v2 = v2;
    task->start = i * block_size;
    task->count = block_size;
    if (i == t-1) task->count += n - block_size * t;

    int err = pthread_create(&ids[i], NULL, thread_main, (void*) task);
    if (err){
      printf("Error creating threads\n");
      exit(1);
    }
  }

  // Wait for each thread to finish and aggregate their results
  float dot_prod = 0;
  for (int i = 0; i < t; i++){
    void* result_ptr;
    int err = pthread_join(ids[i], &result_ptr);
    if (err){
      printf("Error when waiting for threads\n");
      exit(1);
    }

    dot_prod += * (float*) result_ptr;
    free(result_ptr);
  }

  printf("Concurrent dot product: %.16f\n", dot_prod);
  printf("Sequential dot product: %.16f\n", seq_dot_prod);

  float relative_error = fabsf((seq_dot_prod - dot_prod) / seq_dot_prod);
  printf("Relative error: %.16f\n", relative_error);

  return 0;
}
