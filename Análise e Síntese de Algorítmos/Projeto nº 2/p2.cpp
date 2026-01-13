#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <limits.h>

using namespace std;

typedef struct {
    int estacao_numero, linha_numero;
} Estacao;

int calculateMetroConectivity(vector<vector<Estacao>>& metro, int n) {
    vector<int> n_mudancas(n, INT_MAX);  // Armazena o número mínimo de mudanças de linha

    // Para cada estação de início
    for (int start = 0; start < n; start++) {
        vector<bool> visited(n, false);  // Marca as estações visitadas
        queue<pair<int, int>> fila;  // Fila para BFS (estação, linha_atual)
        fila.push({start, -1});  // Inicia a BFS na estação 'start', sem linha inicial
        
        vector<int> aux(n, INT_MAX);  // Vetor para armazenar o número mínimo de mudanças por estação
        aux[start] = 0;  // A estação de início não precisa de mudanças de linha

        // Mapeamento de linhas às estações usando unordered_map
        unordered_map<int, vector<int>> linhas;
        for (int i = 0; i < n; i++) {
            for (auto& vizinho : metro[i]) {
                linhas[vizinho.linha_numero].push_back(i);  // Mapeia cada linha a suas estações
            }
        }

        while (!fila.empty()) {
            int estacao_atual = fila.front().first;
            int linha_atual = fila.front().second;
            fila.pop();
            
            if (visited[estacao_atual]) continue;
            visited[estacao_atual] = true;

            // Processa os vizinhos da estação
            for (auto& vizinho : metro[estacao_atual]) {
                int nova_estacao = vizinho.estacao_numero;
                int nova_linha = vizinho.linha_numero;

                // Se a estação não foi visitada, processa
                if (!visited[nova_estacao]) {
                    // Verifica se há troca de linha
                    int mudanca = (linha_atual != -1 && linha_atual != nova_linha) ? 1 : 0;

                    // Atualiza o número mínimo de mudanças para a estação
                    if (aux[nova_estacao] > aux[estacao_atual] + mudanca) {
                        aux[nova_estacao] = aux[estacao_atual] + mudanca;
                        fila.push({nova_estacao, nova_linha});  // Enfileira o vizinho
                    }
                }
            }
        }

        n_mudancas[start] = aux[n - 1];  // Considera que a última estação é o destino (ajustável conforme necessário)
    }

    // Verifica se alguma estação não foi alcançada
    for (int i = 0; i < n; i++) {
        if (n_mudancas[i] == INT_MAX) return -1;  // Retorna -1 se não for possível chegar a uma estação
    }

    // Retorna o número mínimo de mudanças de linha
    int n_min = INT_MAX;
    for (int i = 0; i < n; i++) {
        if (n_mudancas[i] < n_min) {
            n_min = n_mudancas[i];
        }
    }

    return n_min;
}

int main() {
    int n, m, l;
    scanf("%d %d %d", &n, &m, &l);

    vector<vector<Estacao>> metro(n);

    // Leitura das entradas e construção do grafo de estações
    for (int i = 0; i < m; i++) {
        int x, y, linha;
        scanf("%d %d %d", &x, &y, &linha);
        metro[x - 1].push_back({y - 1, linha});
        metro[y - 1].push_back({x - 1, linha});
    }

    // Calcula e imprime o número mínimo de mudanças de linha
    printf("%d\n", calculateMetroConectivity(metro, n));

    return 0;
}
