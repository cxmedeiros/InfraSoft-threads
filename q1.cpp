#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <pthread.h>

// processar_arquivo() é a função responsável por ler os arquivos e incrementar
// a quantidade de um produto n em um vetor de contadores na posição [n]

void processar_arquivo(int i, int *contagem_produtos, pthread_mutex_t *mutexes)
{
    std::ifstream arquivo{"arquivos/" + std::to_string(i) + ".in"};

    int n;
    while (arquivo >> n) // enquanto houver linhas de produtos no arquivo
    {
        pthread_mutex_lock(&mutexes[n]);
        ++contagem_produtos[n];
        pthread_mutex_unlock(&mutexes[n]);

        // bloqueamos o vetor de contadores apenas na posição n para que as threads possam
        // escrever no vetor simultaneamente desde que seja em uma posição diferente
    }
}

// estrutura que guarda os parametros que vamos usar para algumas funções

struct Params {
    int inicio;
    int n_threads;
    int n_arquivos;
    int *contagem_produtos;
    pthread_mutex_t *mutexes;
};


// a função processar_arquivos() é responsável por passar o trabalho para as threads
void* processar_arquivos(void *tipo)
{
    Params *params = static_cast<Params *>(tipo); // convertendo o tipo, já que precisamos passar um tipo void * no parâmetro, mas queremos usar Params
    for (int i = params->inicio + 1; i <= params->n_arquivos; i += params->n_threads) // indo do início até o n_arquivos e usando  n_threads como passo
    {
        processar_arquivo(i, params->contagem_produtos, params->mutexes); // chamando a função que efetivamente faz o trabalho
    }

    return nullptr;
}

int main()
{
    int n_arquivos;
    int n_produtos;
    int n_threads;

    // lendo os inputs
    std::cin >> n_arquivos;
    std::cin >> n_threads;
    std::cin >> n_produtos;

    int contagem_produtos[n_produtos + 1] = {}; // declarando vetor de contadores

    // inicializando vetor de mutexes
    pthread_mutex_t mutexes[n_produtos + 1];
    for (int i = 0; i < n_produtos + 1; ++i)
    {
        pthread_mutex_init(&mutexes[i], nullptr);
    }

    Params thread_params[n_threads]; // criando variável do tipo Params para cada thread

    // criando as threads
    pthread_t threads[n_threads];

    for (int i = 0; i < n_threads; ++i)
    {
        thread_params[i] = {i, n_threads, n_arquivos, contagem_produtos, mutexes};
        pthread_create(&threads[i], nullptr, &processar_arquivos, &thread_params[i]);
    }

    for (int i = 0; i < n_threads; ++i)
    {
        pthread_join(threads[i], nullptr);
    }

    for (int i = 0; i <= n_produtos; ++i)
    {
        std::cout << std::setw(6) << contagem_produtos[i] << ' ' << i << '\n'; // printando o output (quantidade, produto)
    }

    for (int i = 0; i < n_produtos + 1; ++i)
    {
    pthread_mutex_destroy(&mutexes[i]); // destruindo mutexes
    }

    pthread_exit(nullptr); // saindo das threads

    return 0;
}