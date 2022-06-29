#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct Parametro {
  int esquerda, direita;
  int *vetor;
} Parametro;

int partition (int *v, int l, int r) { //funcao padrao do algoritmo de ordenacao quicksort para encontrar o partition
    int i = l, j = r, temp = 0, aux = 0;
    while (i < j) {
        while (i <= r && v[i] <= v[l]) {
            i++;
        }
        while (v[j] > v[l]) {
            j--;
        }
        if (i < j) {
            temp = v[j];
            v[j] = v[i];
            v[i] = temp;
        }
    }
    aux = v[j];
    v[j] = v[l];
    v[l] = aux;
    return j;
}

void qSort(int *v, int l, int r); //apenas a assinatura da funcao

void *sort (void *args) {
	Parametro *inicio = args;
	qSort(inicio->vetor, inicio->esquerda, inicio->direita);
	return NULL;
}

void qSort(int *v, int l, int r) { 
    if (r <= l) { //se so tiver um ou nenhum numero pra ordenar, a funcao so retorna
      return;
    }

    pthread_t tL, tR; //inicializando as threads

    int p = partition(v, l, r); //encontrando o partition 
  
    //aqui vamos colocar os parametros em cada thread
    Parametro thread_esquerda;
    Parametro thread_direita;
  
    //a thread da esquerda vai do "l" ate o p-1 (a "metade" esquerda do array)
    thread_esquerda.vetor = v;
    thread_esquerda.esquerda = l;
    thread_esquerda.direita = p-1;

    //a thread da direita vai do p+1 ate o r (a "metade" direita do array)
    thread_direita.vetor = v;
    thread_direita.esquerda = p+1;
    thread_direita.direita = r;

    //aqui vamos criar as threads ordenando-as por recursao
    pthread_create(&tL, NULL, sort, &thread_esquerda); //o que era equivalente a qSort(v, l, p-1) sem thread
    pthread_create(&tR, NULL, sort, &thread_direita); //o que era equivalente a qSort(v, p+1, r) sem thread

    //esperando as threads acabarem 
    pthread_join(tL,NULL);
	pthread_join(tR,NULL);
}

int main() {
    int n = 0;

    printf("Insira o tamanho do vetor: \n");
    scanf("%d", &n); //pegando o tamanho do vetor

    int *v = (int*)malloc(n*sizeof(int)); //alocando memoria para o meu vetor do tamanho desejado

    for (int i = 0; i < n; i++) {
        printf("Insira o valor na posicao %d: \n", i);
        scanf("%d", &v[i]); //pegando os valores do vetor
    }

    qSort(v, 0, n-1); //chamando a funcao de ordenacao que vai ordenar o vetor v de 0 (inicio) ate n-1 (tamanho do vetor - 1)

    printf("O vetor ordenado eh: \n");
    for (int i = 0; i < n; i++) {
        printf("%d ", v[i]); //printando o vetor ordenado
    }

    free(v);
    return 0;
}