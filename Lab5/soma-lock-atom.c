/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Codigo: Comunicação entre threads usando variável compartilhada e exclusao mutua com bloqueio */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

long int soma = 0; //variavel compartilhada entre as threads
bool logged = false; //registra se a soma atual já foi logada ou não, sendo significativa apenas
                     // quando a soma for múltipla de 1000
pthread_mutex_t mutex; //variavel de lock para exclusao mutua

// Variáveis de condição associadas as condições 'logged' e '!logged'.
pthread_cond_t cond_logged;
pthread_cond_t cond_not_logged;

//funcao executada pelas threads
void *ExecutaTarefa (void *arg) {
  long int id = (long int) arg;
  printf("Thread : %ld esta executando...\n", id);

  for (int i=0; i<100000; i++) {
     //--entrada na SC
     pthread_mutex_lock(&mutex);

     // Aguarda até que o valor atual de soma seja logado para seguir somando
     while (soma % 1000 == 0 && !logged) pthread_cond_wait(&cond_logged, &mutex);

     soma++; //incrementa a variavel compartilhada

     // Caso tenhamos entrado em um múltiplo de 1000, alertamos a thread extra
     if (soma % 1000 == 0){
       logged = false;
       pthread_cond_signal(&cond_not_logged);
     }

     //--saida da SC
     pthread_mutex_unlock(&mutex);
  }
  printf("Thread : %ld terminou!\n", id);
  pthread_exit(NULL);
}

//funcao executada pela thread de log
void *extra (void *args) {
  printf("Extra : esta executando...\n");
  for (int i=0; i<101; i++) {
    pthread_mutex_lock(&mutex);

    // Aguarda o momento de printar a soma e depois a printa
    while (soma % 1000 != 0 || logged) pthread_cond_wait(&cond_not_logged, &mutex);
    printf("soma = %ld \n", soma);

    logged = true;
    pthread_cond_broadcast(&cond_logged);

    pthread_mutex_unlock(&mutex);
  }
  printf("Extra : terminou!\n");
  pthread_exit(NULL);
}

//fluxo principal
int main(int argc, char *argv[]) {

   //--le e avalia os parametros de entrada
   if(argc != 2) {
      printf("Digite: %s <numero de threads>\n", argv[0]);
      return 1;
   }

   //qtde de threads (passada linha de comando)
   int nthreads = atoi(argv[1]);

   //identificadores das threads no sistema
   pthread_t* tid = (pthread_t*) malloc(sizeof(pthread_t)*(nthreads+1));
   if(tid==NULL) {puts("ERRO--malloc"); return 2;}

   //--inicilaiza o mutex (lock de exclusao mutua)
   pthread_mutex_init(&mutex, NULL);
   pthread_cond_init(&cond_logged, NULL);
   pthread_cond_init(&cond_not_logged, NULL);

   //--cria as threads
   for(long int t=0; t<nthreads; t++) {
     if (pthread_create(&tid[t], NULL, ExecutaTarefa, (void *)t)) {
       printf("--ERRO: pthread_create()\n"); exit(-1);
     }
   }

   //--cria thread de log
   if (pthread_create(&tid[nthreads], NULL, extra, NULL)) {
      printf("--ERRO: pthread_create()\n"); exit(-1);
   }

   //--espera todas as threads terminarem
   for (int t=0; t<nthreads+1; t++) {
     if (pthread_join(tid[t], NULL)) {
         printf("--ERRO: pthread_join() \n"); exit(-1);
     }
   }

   //--finaliza o mutex
   pthread_mutex_destroy(&mutex);

   printf("Valor de 'soma' = %ld\n", soma);

   return 0;
}
