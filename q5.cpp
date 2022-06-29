#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

//variáveis inicializadas estaticamente
#define P 5
#define C 5
#define B 10

typedef struct elem {
    int value;
    struct elem *prox;
} Elem;

typedef struct blockingQueue {
    unsigned int sizeBuffer; //armazena o tamanho máximo que a fila pode ter
    unsigned int statusBuffer; //armazena o tamanho atual da fila
    Elem *head, *last;
} BlockingQueue;

BlockingQueue *Q;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //mutex
pthread_cond_t vazia = PTHREAD_COND_INITIALIZER; //variável de condição que sinaliza se a fila ta vazia
pthread_cond_t cheia = PTHREAD_COND_INITIALIZER; //variável de condição que sinaliza se a fila ta cheia

//função que cria uma nova fila Bloqueante do tamanho do valor passado
BlockingQueue* newBlockingQueue (unsigned int SizeBuffer) {

    //fazendo a alocação de memória 
    Elem *nova_BQ = (Elem*)malloc(sizeof(Elem));
    nova_BQ->prox = NULL;

    //inicializando os parâmetros
    BlockingQueue* BQ = (BlockingQueue*)malloc(sizeof(BQ));
    BQ->sizeBuffer = SizeBuffer;
    BQ->statusBuffer = 0;
    BQ->head = nova_BQ;
    BQ->last = BQ->head;
    return BQ;
}

//função que insere um elemento no final da fila bloqueante Q, bloqueando a thread que está inserindo, caso a fila esteja cheia
void putBlockingQueue (BlockingQueue *Q, int newValue) {

    pthread_mutex_lock(&mutex); //travando o mutex

    while ((Q->statusBuffer) == (Q->sizeBuffer)) { //enquanto o tamanho atual for igual ao tamanho máximo de elementos
        printf("Fila cheia\n");
        pthread_cond_wait(&vazia, &mutex); //fazemos a thread esperar/dormir até que seja liberada uma posição na fila 
    }

    //inserindo novo elemento na fila
    Elem *novo_Elemento = (Elem*)(malloc(sizeof(Elem)));
    novo_Elemento->value = newValue;
    novo_Elemento->prox = NULL;
    Q->last->prox = novo_Elemento;
    Q->last = novo_Elemento;
    Q->statusBuffer++; //aumentando um elemento na fila

    if (Q->statusBuffer == 1) {
        pthread_cond_broadcast(&cheia);
    }
    
    pthread_mutex_unlock(&mutex); //destravando o mutex
}

//função que retira o primeiro elemento da fila bloqueante Q, bloqueando a thread que está retirando, caso a fila esteja vazia
int takeBlockingQueue (BlockingQueue *Q) {     
    
    pthread_mutex_lock(&mutex); //travando o mutex
    
    while (Q->statusBuffer == 0 ) { //enquanto o tamanho atual é 0, ou seja, está vazia
        printf("Fila vazia\n");
        pthread_cond_wait(&cheia, &mutex); //fazemos a thread esperar/dormir até que haja elementos no buffer
    }

    //apagando o primeiro elemento da fila
    int resultado;
    Elem *temporario = Q->head->prox;
    Q->head->prox = temporario->prox;
    resultado = temporario->value;    
    free(temporario);
    Q->statusBuffer--; //diminuindo um elemento da fila

    if (Q->statusBuffer == 0) { //se não houver mais elementos na fila, fazemos o ponteiro do último apontar para o primeiro
        Q->last = Q->head;
    }

    if (Q->statusBuffer == (Q->sizeBuffer)-1) {
        pthread_cond_broadcast(&vazia);
    }

    pthread_mutex_unlock(&mutex); //destravando o mutex
    return resultado;
}

void *chama_produtor (void *thread_id) {   
    int flag = 1;
    while (flag) { //rodando em loop infinito 
        putBlockingQueue(Q, rand()%99);
    }
    pthread_exit(NULL);
}

void *chama_consumidor (void *thread_id) {
    int var, flag = 1;
    while (flag) { //rodando em loop infinito
        var = takeBlockingQueue(Q);
    }
    pthread_exit(NULL);
}

int main () {
    int i = 0;
    int var = 0;

    Q = newBlockingQueue(B); //criando uma fila bloqueante

    int *produtor_ID[P];
    int *consumidor_ID[C];

    //criando os vetores de threads
    pthread_t produtor[P];
	pthread_t consumidor[C];
    
    //aqui tratamos o produtor
    i = 0;
    var = 0;
    for (i = 0; i < P; i++) {
        produtor_ID[i] = (int*) malloc(sizeof(int));
        *produtor_ID[i] = i;
        var = pthread_create(&produtor[i], NULL, chama_produtor, (void*)produtor_ID[i]);
    }

    //aqui tratamos o consumidor
    i = 0;
    var = 0;
    for (i = 0; i < C; i++) {
        consumidor_ID[i] = (int*) malloc(sizeof(int));
        *consumidor_ID[i] = i;
        var = pthread_create(&consumidor[i], NULL, chama_consumidor, (void*)consumidor_ID[i]);
    }  
    pthread_exit(NULL); //espera as threads acabarem e encerra o programa (nesse caso, não encerra porque as threads não terminam)
}