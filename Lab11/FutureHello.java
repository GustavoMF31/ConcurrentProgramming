/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Exemplo de uso de futures */
/* -------------------------------------------------------------------*/

import java.util.concurrent.Callable;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.Future;

import java.util.ArrayList;
import java.util.List;


//classe runnable
class MyCallable implements Callable<Long> {
  //construtor
  MyCallable() {}
 
  //método para execução
  public Long call() throws Exception {
    long s = 0;
    for (long i=1; i<=100; i++) {
      s++;
    }
    return s;
  }
}

class Prime implements Callable<Boolean> {
  long n;
  public Prime(int n) { this.n = n; }

  public Boolean call() {
    return ehPrimo(n);
  }

  //funcao para determinar se um numero  ́e primo
  boolean ehPrimo(long n) {
    if(n <= 1) return false;
    if(n == 2) return true;
    if(n % 2 == 0) return false;

    for(long i=3; i < Math.sqrt(n)+1; i += 2) {
      if(n%i==0) return false;
    }

    return true;
  }
}


//classe do método main
public class FutureHello  {
  private static final int N = 3;
  private static final int NTHREADS = 10;

  public static void main(String[] args) {
    //cria um pool de threads (NTHREADS)
    ExecutorService executor = Executors.newFixedThreadPool(NTHREADS);
    //cria uma lista para armazenar referencias de chamadas assincronas
    List<Future<Boolean>> list = new ArrayList<Future<Boolean>>();

    int M = 10000;
    for (int i = 0; i < M; i++) {
      Callable<Boolean> worker = new Prime(i);
      /*submit() permite enviar tarefas Callable ou Runnable e obter um objeto Future para acompanhar o progresso e recuperar o resultado da tarefa
       */
      Future<Boolean> submit = executor.submit(worker);
      list.add(submit);
    }

    System.out.println(list.size());
    //pode fazer outras tarefas...

    //recupera os resultados e faz o somatório final
    long primes = 0;
    for (Future<Boolean> future : list) {
      try {
        primes += future.get() ? 1 : 0; //bloqueia se a computação nao tiver terminado
      } catch (InterruptedException e) {
        e.printStackTrace();
      } catch (ExecutionException e) {
        e.printStackTrace();
      }
    }

    System.out.println("Foram encontrados " + primes + " primos menores que " + M);
    executor.shutdown();
  }
}
