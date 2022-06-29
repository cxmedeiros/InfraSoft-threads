#include <cmath>
#include <iostream>
#include <functional>
#include <pthread.h>

constexpr int OMP_NUM_THREADS = 4;

// possiveis algoritmos de escalonamento
constexpr int ESTATICO = 1;
constexpr int DINAMICO = 2;
constexpr int GUIADO = 3;

// inicio e fim de cada bloco de iteracoes
struct Chunk
{
    int inicio;
    int final_;
};

// classe base dos escalonadores: guarda os parametros de escalonamento e controla o mutex
class Escalonador
{
public:
    Escalonador(pthread_mutex_t* mutex, int inicio, int passo, int chunk_size, int final_) :
        mutex{mutex}, inicio{inicio}, passo_{passo}, chunk_size{chunk_size}, final_{final_}
    {}

    // escalonadores precisam implementar esta função para calcular os indices de cada bloco
    virtual Chunk pedir_trabalho(int id, int iteracao) = 0;

    // funções para bloquear e desbloquear o mutex
    void lock() { pthread_mutex_lock(mutex); }
    void unlock() { pthread_mutex_unlock(mutex); }

    int passo() const { return passo_; }

protected:
    pthread_mutex_t* mutex;
    int inicio;
    int passo_;
    int chunk_size;
    int final_;
};

// escalonador estatico: divide o trabalho entre todos os threads de forma determistica, com blocos do mesmo tamanho
struct EscalonadorEstatico : public Escalonador
{
    EscalonadorEstatico(pthread_mutex_t* mutex, int inicio, int passo, int chunk_size, int final_) :
        Escalonador{mutex, inicio, passo, chunk_size, final_}
    {}

    Chunk pedir_trabalho(int id, int iteracao) override
    {
        int chunk_passo = chunk_size * passo_;

        int chunk_inicio = iteracao * chunk_passo * OMP_NUM_THREADS + id * chunk_passo;
        int chunk_final = chunk_inicio + chunk_passo;

        // para que não ultrapasse o limite do iteraçoes
        return {std::min(chunk_inicio, final_), std::min(chunk_final, final_)};
    }
};

// escalonador dinamico: divide o trabalho de forma que cada bloco tenha o mesmo tamanho, e as threads são alocadas os blocos por ordem de pedido
struct EscalonadorDinamico : public Escalonador
{
    EscalonadorDinamico(pthread_mutex_t* mutex, int inicio, int passo, int chunk_size, int final_) :
        Escalonador{mutex, inicio, passo, chunk_size, final_}
    {}

    Chunk pedir_trabalho(int id, int) override
    {
        lock();
        int chunk_inicio = inicio;

        // calcula o menor valor entre o final calculado e o final total, para que não ultrapasse o limite do iteraçoes
        int chunk_final = std::min(inicio + passo_ * chunk_size, final_);
        inicio = chunk_final;
        unlock();

        return {chunk_inicio, chunk_final};
    }
};

// escalonador guiado: divide o trabalho em blocos cada vez menores de acordo com a quantidade restante de iterações, e as threads são alocadas os blocos por ordem de pedido
struct EscalonadorGuiado : public Escalonador
{
    EscalonadorGuiado(pthread_mutex_t* mutex, int inicio, int passo, int chunk_size, int final_) :
        Escalonador{mutex, inicio, passo, chunk_size, final_}
    {}

    Chunk pedir_trabalho(int id, int iteracao) override
    {
        lock();
        float restante = float(final_ - inicio) / passo_;

        // tamanho maximo do chunk atual, definido de acordo com a quantidade de iterações restantes, sendo no mínimo igual a chunk_size
        int chunk_maximo = std::max<int>(chunk_size, std::round(restante / OMP_NUM_THREADS));

        int chunk_inicio = inicio;

        // calcula o menor valor entre o final calculado e o final total, para que não ultrapasse o limite do iteraçoes
        int chunk_final = std::min(inicio + passo_ * chunk_maximo, final_);

        // invariante: ajusta o inicio do proximo bloco para o final do bloco atual
        inicio = chunk_final;
        unlock();

        return {chunk_inicio, chunk_final};
    }
};

struct Params
{
    // id de cada thread (para calcular o escalonamento estático)
    int id;

    // escalonador a ser usado pelas threads;
    Escalonador* escalonador;

    // função que será escalonada
    std::function<void(int)> f;
};

// função que será escalonada, faz qualquer trabalho para simular demora de uma computação
void f(int i)
{
    for (int j = 0; j < 1000000; ++j) {
        i = i + j;
    }
}

void* trabalho(void* void_params)
{
    // é garantido que void_params é um ponteiro para Params, mas pthread não guarda o tipo do parametro
    Params* params = static_cast<Params*>(void_params);

    int iteracao = 0;

    Chunk chunk = params->escalonador->pedir_trabalho(params->id, iteracao);
    while (chunk.inicio != chunk.final_)
    {
        std::cout << "Thread " << params->id << " pede iterações e recebe iterações: " << chunk.inicio << ' ' << chunk.final_ << '\n';

        // executa este bloco de iterações
        for (int i = chunk.inicio; i < chunk.final_; i += params->escalonador->passo())
        {
            params->f(i);
        }

        chunk = params->escalonador->pedir_trabalho(params->id, ++iteracao);
    }

    std::cout << "Thread " << params->id << " pede iterações: é fechada.\n";
    return nullptr;
}


void omp_for(int inicio, int passo, int final, int schedule, int chunk_size, void (*f)(int))
{
    pthread_t threads[OMP_NUM_THREADS];

    // mutex que será usado pelos escalonadores dinamico e guiado para sincronizar
    pthread_mutex_t mutex;
    pthread_mutex_init(&mutex, nullptr);

    Escalonador* escalonador;

    // escolhe escalonador de acordo com o parametro schedule
    switch (schedule)
    {
        case ESTATICO:
            std::cout << "Schedule: Estático\n";
            escalonador = new EscalonadorEstatico(&mutex, inicio, passo, chunk_size, final);
            break;
        case DINAMICO:
            std::cout << "Schedule: Dinâmico\n";
            escalonador = new EscalonadorDinamico(&mutex, inicio, passo, chunk_size, final);
            break;
        case GUIADO:
            std::cout << "Schedule: Guiado\n";
            escalonador = new EscalonadorGuiado(&mutex, inicio, passo, chunk_size, final);
            break;
        default:
            std::cout << "schedule não reconhecido\n";
            return;
    }

    // cria threads e envia os parametros para a função trabalho da thread
    Params params[OMP_NUM_THREADS];
    for (int i = 0; i < OMP_NUM_THREADS; ++i)
    {
        params[i] = {i, escalonador, f};
        pthread_create(&threads[i], nullptr, trabalho, &params[i]);

    }

    // espera todas as threads terminarem
    for (int i = 0; i < OMP_NUM_THREADS; ++i)
    {
        pthread_join(threads[i], nullptr);
    }

    // para nao vazar memoria
    delete escalonador;
}

int main()
{
    int inicio, passo, final_, chunk_size, schedule;

    // le os parametros
    std::cin >> inicio;
    std::cin >> passo;
    std::cin >> final_;
    std::cin >> chunk_size;
    std::cin >> schedule;

    omp_for(inicio, passo, final_, schedule, chunk_size, f);
    return 0;
}