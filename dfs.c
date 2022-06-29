#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define filhos 8 //definindo a quantidade de nos
int thread_id = 0; // conta o id das threads

// inicializando mutex
pthread_mutex_t meu_mutex = PTHREAD_MUTEX_INITIALIZER;

// estrutura do no
typedef struct no {
    int vertice;
    struct no* proximo;
}no;

// estrutura dos argumentos das threads
typedef struct {
    struct no **Grafo;
    int v;
    int inicio;
    int *nos_visitados;
    int id;
}vertice;
 
//criando um no
no* criar_no(int v) {
 
    no *novo = (no *) malloc(sizeof(no));
    novo->vertice = v;
    novo->proximo = NULL;
 
    return novo;
}

//funçao que visita os nos do grafo
void *visita_DFS(void *arg){
    
    int th = 0; //calcular o numero de threads

    vertice *argumento = (vertice *) arg;
    struct no **Grafo = argumento->Grafo;
    int v = argumento-> v;
    int inicio = argumento->inicio;
    int *nos_visitados = argumento->nos_visitados;
    int id = argumento->id;
   
    printf("thread %d visitou o no %d\n", id, inicio);
    no *aresta_atual;
    int temp;
    aresta_atual = Grafo[inicio];

    pthread_t *threads; // para criar mais de uma thread
 
    // alocando memoria para as threads
    threads = (pthread_t *) malloc((filhos) * sizeof(pthread_t));
 
    while(aresta_atual != NULL){

        temp = aresta_atual->vertice;

        // criando estrutura para passar os parametros para a thread
        vertice *parametros = malloc(sizeof(vertice));
        parametros->Grafo = Grafo;
        parametros->v = v;
        parametros->inicio = temp;
        parametros->nos_visitados = nos_visitados;
        parametros->id = thread_id;

        pthread_mutex_lock(&meu_mutex); // trava todas as threads
        if (nos_visitados[temp] == 0){

            nos_visitados[temp] = 1; // afirmando que o no ja foi visto
 
            pthread_create(&threads[th],NULL,visita_DFS,(void *) parametros); // criando as threads

            //printf("thread %d viu o no %d a partir de %d\n", id, temp, inicio);

            th++;
            thread_id++;

        }
        pthread_mutex_unlock(&meu_mutex); // desbloqueia 
        aresta_atual = aresta_atual->proximo;
    }

    // espera todas as threads acabarem
    for(int i=0; i<th; i++){
        pthread_join(threads[i], NULL);
    }  
}

//funçao que chama a dfs que vai visitar os nos
void DFS(no **Grafo, int v, int vertice_inicial){
 
    int *nos_visitados;

    nos_visitados = (int *) malloc(v * sizeof(int));
 
    for(int i = 0; i < v; i++){
        nos_visitados[i] = 0;
    }
 
    pthread_t thread; //cria uma therad que vai ser mandada para a proxima funçao

    // criando uam estrutura para passar os parametros para a thread
    vertice *parametros = malloc(sizeof(vertice));
    parametros->Grafo = Grafo;
    parametros->v = v;
    parametros->inicio = vertice_inicial;
    parametros->nos_visitados = nos_visitados;
    parametros->id = thread_id;
    thread_id++;

    nos_visitados[vertice_inicial] = 1;

    pthread_create(&thread,NULL,visita_DFS,(void *) parametros); // criando as threads
    pthread_join(thread, NULL);
        
}

//adicionar aresta
no *add_aresta(no **Grafo, int inicio, int destino){
 
    no *novo_no = criar_no(destino);
 
    //ida
    novo_no->proximo = Grafo[inicio];
    Grafo[inicio] = novo_no;
 
    //volta
    novo_no = criar_no(inicio);
    novo_no->proximo = Grafo[destino];
    Grafo[destino] = novo_no;
 
    return novo_no;
}
 
//imprimir o grafo - estava usando para teste
/*void imprimir_grafo(no **Grafo, int nos){
    int v;
    for (v = 0; v < nos; v++){
        no *temp = Grafo[v];

        printf("lista de adjacencia do vertice %d\n", v);
        //cout << "lista de adjacencia do vertice " << v << endl;

        while (temp){
            printf(" -> %d", temp->vertice);
            //cout << " -> " << temp->vertice;
            temp = temp->proximo;
        }
        printf("\n");
        //cout << endl;
    }
}*/
 
int main (){
 
    int vertices = 8;
    no **Grafo = NULL;
 
    Grafo = (no **) malloc (8 * sizeof(no*)); //criando o grafo com o numero de vertices
    for(int i = 0; i < 8; i++){
        Grafo[i] = NULL;
    }
 
    add_aresta(Grafo, 0, 1);
    add_aresta(Grafo, 0, 3);
    add_aresta(Grafo, 0, 2);
    add_aresta(Grafo, 0, 6);
    add_aresta(Grafo, 1, 4);
    add_aresta(Grafo, 1, 5);
    add_aresta(Grafo, 4, 6);
    add_aresta(Grafo, 5, 3);
    add_aresta(Grafo, 5, 2);
    add_aresta(Grafo, 2, 7);
 
    //imprimir_grafo(Grafo, vertices); 
 
    DFS(Grafo, vertices, 0);//chamando a funçao Dfs e falando que começa pelo vertice 0
 
    return 0 ;
}