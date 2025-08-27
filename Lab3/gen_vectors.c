#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main(int argc, char** argv){
  if (argc != 3){
    printf("Usage: %s vector-len output.bin\n", argv[0]);
    exit(1);
  }

  // Vector length
  long int n = atoi(argv[1]);
  char* filename = argv[2];

  FILE *file = fopen(filename, "wb");
  if (file == NULL){
    printf("Failed to open file %s\n", filename);
    exit(1);
  }

  // Generate random vectors v1 and v2 of size n and
  // Compute their dot product as we go
  float v1[n], v2[n];
  float dot_prod = 0;

  srand(time(0));
  for (int i = 0, c = 1; i < n; i++, c *= -1){
    v1[i] = c * (rand() % 1000) / 3.0;
    v2[i] = (rand() % 1000) / 3.0; 
    dot_prod += v1[i] * v2[i];
  }

  // Write vectors to file
  int b0 = fwrite(&n, sizeof(n), 1, file);
  int b1 = fwrite(v1, sizeof(*v1), n, file);
  int b2 = fwrite(v2, sizeof(*v2), n, file);
  int b3 = fwrite(&dot_prod, sizeof(b3), 1, file);

  // Check if the writes were succesfull
  if (b0 != 1 || b1 !=  n || b2 != n || b3 != 1){
    printf("Failed to write generated vectors to file %s\n", filename);
    exit(1);
  }

  return 0;
}
