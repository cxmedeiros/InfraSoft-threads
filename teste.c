#define _XOPEN_SOURCE 600 //necessario para a utilizaçao de barreiras
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define P 10 // definindo a precisao como 10
#define TAM 3// definindo o limite de incognitass como 3

int matriz[TAM][TAM]; // matriz com os valores
int respostas[TAM]; // resultado das equacoes lineares
double incognitas[P+1][TAM+1]; // incognitass
int n; // numero de threads

pthread_barrier_t barrier; // cria uma barreira

// iniciando valores iniciais
void inicio(){

  int i,j;

  for(i=0; i < TAM; i++){
    for(j=0 ;j < TAM; j++){
      matriz[i][j] = 2;
    }
    respostas[i] = 5;
  }
  
  for(i=0; i < P+1; i++){
    for(j=0; j < TAM+1; j++){
      incognitas[i][j] = 1;
    }
  }
}

// refinar o resultado durante as P interaçoes
void *refina(void *thread_id){

  int *in = (int *) thread_id;
  int id = *in;
  int i,j,k; // contadores
  long double valor_; // variavel que vai receber o valor atual
  int teto;
  long double soma = 0;

  // definindo o limite das funcoes
  if(id == n-1) {
    teto = TAM;
  } else {
    teto = (id+1)*(TAM/n);
  }

  // algoritmo que ira refinar as incognitas P vezes
  for(i=1; i <= P; i++){
      for(j=id*(TAM/n); j < teto; j++){
        valor_ = 0;
        valor_ = ((long double)1/(matriz[j][j])); // implementando a formula
        soma = 0;
        for(k=0; k < TAM; k++){
            if(j != k){
              // implementacao da formula
              soma += (long double)(matriz[j][k] * incognitas[i-1][k]);
            }
        }
        soma = (long double)(respostas[j] - soma); // implementacao da formula
        valor_ = (long double)valor_*soma;

        // valor_ assume o valor da variavel refinada
        incognitas[i][j] = valor_;
      }

      // criando barreiras para ele esperar as outras threads acabarem
      pthread_barrier_wait(&barrier);
  }
  pthread_exit(NULL);
}

int main(){

    pthread_t *thread;
    int *thread_id;
    int i;

    inicio();

    printf("Digite o número de threads:\n");
    scanf("%d",&n);

    // allocando memoria para as threads
    thread = (pthread_t *) malloc(n*sizeof(pthread_t));
    thread_id = (int *) malloc(n*sizeof(int));

    // iniciando barreira
    pthread_barrier_init(&barrier,NULL,n); 

    for(i=0; i < n; i++){
        thread_id[i] = i;
        pthread_create(&thread[i],NULL,refina,(void *) &thread_id[i]);
    }

    // espera todas as threads finalizarem
    for(i=0; i < n; i++){
      pthread_join(thread[i],NULL);
    }

    printf("valores das incognitas após P interaçoes:\n");

    for(i=0 ; i < TAM; i++){
      printf("X[%d]= %lf \n",i, incognitas[P-1][i]);
    }

    printf("\n");

    // liberando espaços da memoria
    pthread_barrier_destroy(&barrier);
    pthread_exit(NULL);
    free(thread);
    free(thread_id);
    return 0;
}