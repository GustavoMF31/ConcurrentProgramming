/* Disciplina: Programacao Concorrente */
/* Prof.: Silvana Rossetto */
/* Laboratório: 11 */
/* Codigo: Criando um pool de threads em Java */

import java.util.LinkedList;

//-------------------------------------------------------------------------------

// FilaTarefas representa uma pool de threads, que executa tarefas assincronamente
class FilaTarefas {
    // Número de threads da pool
    private final int nThreads;

    // Threads em execução
    private final MyPoolThreads[] threads;

    // Tarefas atualmente submetidas para pool, a serem executadas
    private final LinkedList<Runnable> queue;

    // Registra um pedido de encerramento da execução de tarefas pela pool
    private boolean shutdown;

    public FilaTarefas(int nThreads) {
        this.shutdown = false;
        this.nThreads = nThreads;
        queue = new LinkedList<Runnable>();
        threads = new MyPoolThreads[nThreads];

        // Inicializa e dispara cada uma das n threads da pool
        for (int i=0; i<nThreads; i++) {
            threads[i] = new MyPoolThreads();
            // Invoca o método start de Thread, que dispara a sua
            // execução assíncrona
            threads[i].start();
        } 
    }

    // Submete uma nova tarefa Runnable para execução pela pool
    public void execute(Runnable r) {
        // Obtém acesso exclusivo à fila
        synchronized(queue) {
            // Se a pool está desligando,
            // ignora todas as novas execuções submetidas
            if (this.shutdown) return;

            // Caso contrário, adiciona a tarefa à fila
            // e acorda uma thread bloqueada, se houver.
            queue.addLast(r);
            queue.notify();
        }
    }

    // Faz o desligamento (bloqueante) da pool de threads,
    // aguardando o encerramento de cada thread
    public void shutdown() {
        synchronized(queue) {
            // Ativa o sinal de desligamento e acorda
            // as threads dormindo para que encerrem a execução
            this.shutdown=true;
            queue.notifyAll();
        }

        // Aguarda todas as threads encerrarem
        for (int i=0; i<nThreads; i++) {
          try { threads[i].join(); } catch (InterruptedException e) { return; }
        }
    }

    // Classe interna que herda de Thread
    private class MyPoolThreads extends Thread {
       public void run() {
         Runnable r;
         while (true) {
           // Obtém acesso exclusivo à fila
           synchronized(queue) {
             // Aguarda a disponibilidade de mais uma tarefa,
             // ou um comando de shutdown
             while (queue.isEmpty() && (!shutdown)) {
               try { queue.wait(); }
               catch (InterruptedException ignored){}
             }
             // Encerra a execução em caso de desligamento
             if (queue.isEmpty()) return;   
             // Caso contrário, obtém o Runnable da fila
             r = (Runnable) queue.removeFirst();
           }

           // e começa a executá-lo
           try { r.run(); }
           catch (RuntimeException e) {}
         } 
       } 
    } 
}
//-------------------------------------------------------------------------------

//--PASSO 1: cria uma classe que implementa a interface Runnable 
class Hello implements Runnable {
   String msg;
   public Hello(String m) { msg = m; }

   //--metodo executado pela thread
   public void run() {
      System.out.println(msg);
   }
}

class Primo implements Runnable {
   long n;
   public Primo(long n){
     this.n = n;
   }

   //...completar implementacao, recebe um numero inteiro positivo e imprime se esse numero eh primo ou nao
   public void run() {
     if (ehPrimo(n)){
       System.out.println(n + " é primo");
     } else {
       System.out.println(n + " não é primo");
     }
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

//Classe da aplicação (método main)
class MyPool {
    private static final int NTHREADS = 10;

    public static void main (String[] args) {
      //--PASSO 2: cria o pool de threads
      FilaTarefas pool = new FilaTarefas(NTHREADS); 

      //--PASSO 3: dispara a execução dos objetos runnable usando o pool de threads
      for (int i = 0; i < 25; i++) {
        final String m = "Hello da tarefa " + i;
        Runnable hello = new Hello(m);
        pool.execute(hello);
        Runnable primo = new Primo(i);
        pool.execute(primo);
      }

      //--PASSO 4: esperar pelo termino das threads
      pool.shutdown();
      System.out.println("Terminou");
   }

