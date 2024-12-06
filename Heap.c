#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <locale.h>


// Estrutura para representar um bloco de memória livre
typedef struct {
    int inicio;   // Início do bloco livre
    int tamanho;  // Tamanho do bloco livre
} BlocoLivre;

// Estrutura para representar um bloco de memória alocado
typedef struct {
    int inicio;   // Início do bloco alocado
    int final;    // Final do bloco alocado
} BlocoAlocado;

// Estrutura principal do gerenciador de heap
typedef struct {
    bool *heap;             // Vetor que representa o heap (0 = livre, 1 = ocupado)
    BlocoLivre *livres;     // Lista de blocos livres
    BlocoAlocado *alocados; // Lista de blocos alocados
    int capacidade;         // Tamanho total do heap
    int numLivres;          // Número de blocos livres
    int numAlocados;        // Número de blocos alocados
    int ultimaPosicao;      // Última posição para a estratégia next fit
    char estrategia[10];    // Estratégia de alocação (best, worst, first, next)
} GerenciadorDeHeap;

// Inicializa o gerenciador de heap
void inicializarHeap(GerenciadorDeHeap *gerenciador, int tamanho, const char *estrategia) {
    gerenciador->heap = (bool *)calloc(tamanho, sizeof(bool)); // Inicializa o heap com todos os blocos livres
    gerenciador->livres = (BlocoLivre *)malloc(sizeof(BlocoLivre)); // Cria a lista de blocos livres
    gerenciador->alocados = NULL; // Inicialmente, não há blocos alocados
    gerenciador->livres[0].inicio = 0;
    gerenciador->livres[0].tamanho = tamanho;
    gerenciador->capacidade = tamanho;
    gerenciador->numLivres = 1;   // Apenas um bloco livre no início
    gerenciador->numAlocados = 0; // Nenhum bloco alocado no início
    gerenciador->ultimaPosicao = 0; // Para a estratégia next fit, começa do início
    strcpy(gerenciador->estrategia, estrategia); // Define a estratégia inicial
}

// Define a estratégia de alocação do heap
void definirEstrategiaHeap(GerenciadorDeHeap *gerenciador, const char *estrategia) {
    strcpy(gerenciador->estrategia, estrategia); // Atualiza a estratégia
    printf("Estratégia definida para: %s\n", estrategia);
}

// Encontra um bloco livre com base na estratégia definida
int encontrarBlocoLivre(GerenciadorDeHeap *gerenciador, int tamanho) {
    int indiceEscolhido = -1;

    if (strcmp(gerenciador->estrategia, "best") == 0) {
        // Estratégia Best Fit: encontra o menor bloco que seja suficiente
        int menorTamanho = gerenciador->capacidade + 1;
        for (int i = 0; i < gerenciador->numLivres; i++) {
            if (gerenciador->livres[i].tamanho >= tamanho && gerenciador->livres[i].tamanho < menorTamanho) {
                menorTamanho = gerenciador->livres[i].tamanho;
                indiceEscolhido = i;
            }
        }
    } else if (strcmp(gerenciador->estrategia, "worst") == 0) {
        // Estratégia Worst Fit: encontra o maior bloco que seja suficiente
        int maiorTamanho = -1;
        for (int i = 0; i < gerenciador->numLivres; i++) {
            if (gerenciador->livres[i].tamanho >= tamanho && gerenciador->livres[i].tamanho > maiorTamanho) {
                maiorTamanho = gerenciador->livres[i].tamanho;
                indiceEscolhido = i;
            }
        }
    } else if (strcmp(gerenciador->estrategia, "first") == 0) {
        // Estratégia First Fit: encontra o primeiro bloco que seja suficiente
        for (int i = 0; i < gerenciador->numLivres; i++) {
            if (gerenciador->livres[i].tamanho >= tamanho) {
                indiceEscolhido = i;
                break;
            }
        }
    } else if (strcmp(gerenciador->estrategia, "next") == 0) {
        // Estratégia Next Fit: começa a busca a partir da última posição usada
        int inicio = gerenciador->ultimaPosicao;
        for (int i = 0; i < gerenciador->numLivres; i++) {
            int posicaoAtual = (inicio + i) % gerenciador->numLivres;
            if (gerenciador->livres[posicaoAtual].tamanho >= tamanho) {
                indiceEscolhido = posicaoAtual;
                gerenciador->ultimaPosicao = (posicaoAtual + 1) % gerenciador->numLivres;
                break;
            }
        }
    }

    return indiceEscolhido; // Retorna o índice do bloco escolhido ou -1 se não houver bloco suficiente
}

// Aloca memória no heap
void alocar(GerenciadorDeHeap *gerenciador, int tamanho) {
    int indice = encontrarBlocoLivre(gerenciador, tamanho); // Encontra um bloco adequado
    if (indice != -1) {
        BlocoLivre *bloco = &gerenciador->livres[indice];
        for (int i = bloco->inicio; i < bloco->inicio + tamanho; i++) {
            gerenciador->heap[i] = 1; // Marca o bloco como ocupado
        }

        // Adiciona o bloco alocado à lista de alocados
        gerenciador->alocados = (BlocoAlocado *)realloc(gerenciador->alocados, (gerenciador->numAlocados + 1) * sizeof(BlocoAlocado));
        gerenciador->alocados[gerenciador->numAlocados].inicio = bloco->inicio;
        gerenciador->alocados[gerenciador->numAlocados].final = bloco->inicio + tamanho - 1;
        gerenciador->numAlocados++;

        if (bloco->tamanho == tamanho) {
            // Remove o bloco da lista de livres
            for (int i = indice; i < gerenciador->numLivres - 1; i++) {
                gerenciador->livres[i] = gerenciador->livres[i + 1];
            }
            gerenciador->numLivres--;
        } else {
            // Ajusta o início e o tamanho do bloco restante
            bloco->inicio += tamanho;
            bloco->tamanho -= tamanho;
        }
        printf("Memória alocada com sucesso.\n");
    } else {
        printf("Erro: Não foi possível alocar %d blocos.\n", tamanho);
    }
}

// Libera memória no heap
void desalocar(GerenciadorDeHeap *gerenciador, int inicio, int tamanho) {
    for (int i = inicio; i < inicio + tamanho; i++) {
        gerenciador->heap[i] = 0; // Marca o bloco como livre
    }

    // Remove o bloco da lista de alocados
    for (int i = 0; i < gerenciador->numAlocados; i++) {
        if (gerenciador->alocados[i].inicio == inicio) {
            for (int j = i; j < gerenciador->numAlocados - 1; j++) {
                gerenciador->alocados[j] = gerenciador->alocados[j + 1];
            }
            gerenciador->numAlocados--;
            break;
        }
    }

    // Adiciona um novo bloco à lista de livres
    gerenciador->livres = (BlocoLivre *)realloc(gerenciador->livres, (gerenciador->numLivres + 1) * sizeof(BlocoLivre));
    gerenciador->livres[gerenciador->numLivres].inicio = inicio;
    gerenciador->livres[gerenciador->numLivres].tamanho = tamanho;
    gerenciador->numLivres++;
}

// Exibe o estado do heap e os blocos alocados
void exibe(GerenciadorDeHeap *gerenciador) {
    printf("\nEstado do Heap:\n");
    printf("Legenda: [X] = Ocupado, [ ] = Livre\n\n");
    for (int i = 0; i < gerenciador->capacidade; i++) {
        if (gerenciador->heap[i] == 0) {
            printf("[ ] ");
        } else {
            printf("[X]");
        }
    }

    printf("\n\nBlocos Alocados:\n");
    for (int i = 0; i < gerenciador->numAlocados; i++) {
        printf("Bloco %d: Início = %d, Final = %d\n", i + 1, gerenciador->alocados[i].inicio, gerenciador->alocados[i].final);
    }
}

int main() {
    setlocale(LC_ALL, "");
    GerenciadorDeHeap gerenciador;
    inicializarHeap(&gerenciador, 40, "best"); // Inicializa o heap com tamanho 40 e estratégia "best fit"

    exibe(&gerenciador); // Exibe o estado inicial do heap
    alocar(&gerenciador, 5); // Aloca 5 blocos
    exibe(&gerenciador); // Exibe o estado do heap
    alocar(&gerenciador, 3); // Aloca 3 blocos
    exibe(&gerenciador); // Exibe o estado do heap
    desalocar(&gerenciador, 5, 3); // Desaloca 3 blocos a partir do índice 5
    exibe(&gerenciador); // Exibe o estado do heap

    definirEstrategiaHeap(&gerenciador, "next"); // Define a estratégia como "next fit"
    alocar(&gerenciador, 8); // Aloca 8 blocos
    exibe(&gerenciador); // Exibe o estado final do heap

    free(gerenciador.heap); // Libera a memória alocada para o heap
    free(gerenciador.livres); // Libera a memória alocada para os blocos livres
    free(gerenciador.alocados); // Libera a memória alocada para os blocos alocados

    return 0;
}
