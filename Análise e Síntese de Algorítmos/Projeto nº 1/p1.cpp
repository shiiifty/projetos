#include <iostream>
#include <vector>
#include <unordered_map>
#include <chrono>

using namespace std;


struct Resultado {
    vector<pair<int, int>> valores;
};

void preencherTabela(vector<vector<Resultado>>& tabela, const vector<int>& sequencia, const vector<vector<int>>& matriz, int m, int n) {
    for (int tamanho = 1; tamanho <= m; tamanho++) {
        for (int i = 0; i <= m - tamanho; i++) {
            int j = i + tamanho - 1;
            if (tamanho == 1) {
                tabela[i][j].valores.push_back({sequencia[i], -1});
                continue;
            }
            vector<bool> valoresExistentes(n + 1, false);
            vector<pair<int, int>>* valoresEsquerdoPtr;
            vector<pair<int, int>>* valoresDireitoPtr;
            if (tamanho == 2) {
                valoresEsquerdoPtr = &tabela[i][i].valores;
                valoresDireitoPtr = &tabela[j][j].valores;
            } 
            for (int k = j - 1; k >= i; k--) {
                if (tamanho != 2) {
                    valoresEsquerdoPtr = &tabela[i][k].valores;
                    valoresDireitoPtr = &tabela[k + 1][j].valores;
                }
                vector<pair<int, int>>& valoresEsquerdo = *valoresEsquerdoPtr;
                vector<pair<int, int>>& valoresDireito = *valoresDireitoPtr;
                for (const auto& pairEsquerdo : valoresEsquerdo) {
                    for (const auto& pairDireito : valoresDireito) {
                        int novoValor = matriz[pairEsquerdo.first - 1][pairDireito.first - 1];
                        if (!valoresExistentes[novoValor]) {
                            tabela[i][j].valores.push_back({novoValor, k});
                            valoresExistentes[novoValor] = true;
                        }
                        if (static_cast<int>(tabela[i][j].valores.size()) == n) break;
                    }
                    if (static_cast<int>(tabela[i][j].valores.size()) == n) break;
                }
            }
        }
    }
}

string reconstruirParentizacao(
    const vector<vector<Resultado>>& tabela,
    const vector<int>& sequencia,
    int i, int j,
    int resultado,
    const vector<vector<int>>& matriz
) {
    if (i == j) {
        return to_string(sequencia[i]);
    }
    unordered_map<int, int> resultadoK;
    for (const auto& pair : tabela[i][j].valores) {
        resultadoK[pair.first] = pair.second;
    }
    int k = resultadoK[resultado];
    for (const auto& pairEsquerdo : tabela[i][k].valores) {
        for (const auto& pairDireito : tabela[k + 1][j].valores) {
            if (matriz[pairEsquerdo.first - 1][pairDireito.first - 1] == resultado) {
                string esquerda = reconstruirParentizacao(tabela, sequencia, i, k, pairEsquerdo.first, matriz);
                string direita = reconstruirParentizacao(tabela, sequencia, k + 1, j, pairDireito.first, matriz); 
                return "(" + esquerda + " " + direita + ")";
            }
        }
    }
    return "";
}

int main() {
    int n, m;
    scanf("%d %d", &n, &m);
    vector<vector<int>> matriz(n, vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%d", &matriz[i][j]);
        }
    }
    vector<int> sequencia(m);
    for (int i = 0; i < m; i++) {
        scanf("%d", &sequencia[i]);
    }
    int resultadoEsperado;
    scanf("%d", &resultadoEsperado);
    vector<vector<Resultado>> tabela(m, vector<Resultado>(m));

    auto inicio = chrono::high_resolution_clock::now();

    preencherTabela(tabela, sequencia, matriz, m, n);
    for (const auto& pair : tabela[0][m - 1].valores) {
        if (pair.first == resultadoEsperado) {
            printf("1\n");
            printf("%s\n", reconstruirParentizacao(tabela, sequencia, 0, m - 1, resultadoEsperado, matriz).c_str());
            return 0;
        }
    }
    printf("0\n");
    return 0;
}