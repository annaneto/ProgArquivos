#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define constantes para os tamanhos máximos de linhas e transações.
#define MAX_LINHA 5000
#define MAX_TRANSACAO 5000

// Estrutura para armazenar informações de uma transação.
typedef struct {
    int dia;
    int mes;
    int ano;
    char agencia_origem[10];
    char conta_origem[10];
    float valor;
    char agencia_destino[10];
    char conta_destino[10];
} Transacao;

// Estrutura para armazenar informações de movimentação consolidada.
typedef struct {
    float valor_especie;
    float valor_eletronica;
    int num_transacoes;
    char agencia_origem[10];
    char conta_origem[10];
} Movimentacao;

// Arrays para armazenar transações e movimentações.
Transacao transacoes[MAX_TRANSACAO];
Movimentacao movimentacoes[MAX_TRANSACAO];
int num_transacoes = 0;
int num_movimentacoes = 0;

// Função para ler dados de um arquivo CSV e armazenar no array de transações.
void ler_csv(const char* nome_arquivo) {
    FILE* arquivo_csv = fopen(nome_arquivo, "r");
    if (!arquivo_csv) {
        perror("Erro ao abrir o arquivo CSV"); // Adiciona a mensagem de erro
    }

    char linha[MAX_LINHA];
    while (fgets(linha, MAX_LINHA, arquivo_csv)) {
        Transacao transacao;

        char* token = strtok(linha, ",");
        if (token == NULL) {
            continue;
        }
        transacao.dia = atoi(token); // converter token para inteiro

        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        transacao.mes = atoi(token);

        token = strtok(NULL, ",");
        if (token == NULL) {
            continue;
        }
        transacao.ano = atoi(token);

        token = strtok(NULL, ",");
        if (token == NULL) {
            strcpy(transacao.agencia_origem, "");
        } else {
            strcpy(transacao.agencia_origem, token);
        }

        token = strtok(NULL, ",");
        if (token == NULL) {
            strcpy(transacao.conta_origem, "");
        } else {
            strcpy(transacao.conta_origem, token);
        }

        token = strtok(NULL, ",");
        if (token == NULL) {
            transacao.valor = 0.0f;
        } else {
            transacao.valor = atof(token);
        }

        token = strtok(NULL, ",");
        if (token == NULL) {
            strcpy(transacao.agencia_destino, "");
        } else {
            strcpy(transacao.agencia_destino, token);
        }

        token = strtok(NULL, ",");
        if (token == NULL) {
            strcpy(transacao.conta_destino, "");
        } else {
            strcpy(transacao.conta_destino, token);
        }

        transacoes[num_transacoes++] = transacao;
    }

    fclose(arquivo_csv);
}

// Função para verificar se uma transação é eletrônica.
int eh_transacao_eletronica(const char* agencia_destino, const char* conta_destino) {
    if (strlen(agencia_destino) > 0 && strlen(conta_destino) > 0) {
        return 1;
    } else {
        return 0;
    }
}

// Função para consolidar movimentações para um determinado mês e ano.
void consolidar_movimentacao(int mes, int ano) {
    num_movimentacoes = 0;  // Reinicia o contador de movimentações
    for (int i = 0; i < num_transacoes; i++) {
        Transacao* transacao = &transacoes[i];
        if (transacao->mes == mes && transacao->ano == ano) {
            int found = 0;
            for (int j = 0; j < num_movimentacoes; j++) {
                if (strcmp(movimentacoes[j].agencia_origem, transacao->agencia_origem) == 0 &&
                    strcmp(movimentacoes[j].conta_origem, transacao->conta_origem) == 0) {
                    found = 1;
                    movimentacoes[j].num_transacoes++;
                    if (eh_transacao_eletronica(transacao->agencia_destino, transacao->conta_destino)) {
                        movimentacoes[j].valor_eletronica += transacao->valor;
                    } else {
                        movimentacoes[j].valor_especie += transacao->valor;
                    }
                    break;
                }
            }
            if (!found) {
                Movimentacao nova_movimentacao;
                nova_movimentacao.valor_especie = 0;
                nova_movimentacao.valor_eletronica = 0;
                nova_movimentacao.num_transacoes = 1;
                strcpy(nova_movimentacao.agencia_origem, transacao->agencia_origem);
                strcpy(nova_movimentacao.conta_origem, transacao->conta_origem);
                if (eh_transacao_eletronica(transacao->agencia_destino, transacao->conta_destino)) {
                    nova_movimentacao.valor_eletronica = transacao->valor;
                } else {
                    nova_movimentacao.valor_especie = transacao->valor;
                }
                movimentacoes[num_movimentacoes++] = nova_movimentacao;
            }
        }
    }
}

// Função para salvar os resultados consolidados em um arquivo binário.
void salvar_resultados(int mes, int ano) {
    char nome_arquivo[50];
    sprintf(nome_arquivo, "consolidadas%02d%d.bin", mes, ano);
    FILE* arquivo_binario = fopen(nome_arquivo, "wb");
    if (!arquivo_binario) {
        perror("Erro ao abrir o arquivo binário");
        exit(EXIT_FAILURE);
    }

    fwrite(movimentacoes, sizeof(Movimentacao), num_movimentacoes, arquivo_binario);
    fclose(arquivo_binario);
}

// Função para carregar movimentações consolidadas de um arquivo binário.
void carregar_movimentacoes(int mes, int ano) {
    char nome_arquivo[50];
    sprintf(nome_arquivo, "consolidadas%02d%d.bin", mes, ano);
    FILE* arquivo_binario = fopen(nome_arquivo, "rb");
    if (!arquivo_binario) {
        perror("Erro ao abrir o arquivo binário");
    }

    num_movimentacoes = fread(movimentacoes, sizeof(Movimentacao), MAX_TRANSACAO, arquivo_binario);
    fclose(arquivo_binario);
}

// Função para verificar se um arquivo consolidado já existe.
int verificar_existencia_arquivo(int mes, int ano) {
    char nome_arquivo[50];
    sprintf(nome_arquivo, "consolidadas%02d%d.bin", mes, ano);
    FILE* arquivo = fopen(nome_arquivo, "rb");
    if (arquivo) {
        fclose(arquivo);
        printf("Ja existe um arquivo consolidado para %02d/%d.\n", mes, ano);
        return 1;
    }
    return 0;
}

// Função para filtrar movimentações com base em valores e operador.
void filtrar_movimentacao(float valor_x, float valor_y, const char* operador) {
    for (int i = 0; i < num_movimentacoes; i++) {
        Movimentacao* mov = &movimentacoes[i];
        if ((strcmp(operador, "E") == 0 && mov->valor_especie >= valor_x && mov->valor_eletronica >= valor_y) ||
            (strcmp(operador, "OU") == 0 && (mov->valor_especie >= valor_x || mov->valor_eletronica >= valor_y))) {
            printf("Agencia: %s Conta: %s, Especie: %.2f, Eletronica: %.2f\n",
                   mov->agencia_origem, mov->conta_origem, mov->valor_especie, mov->valor_eletronica);
        }
    }
}

// Função do menu interativo para o usuário.
void menu_interativo() {
  int opcao;
  do {
      printf("\nMenu:\n");
      printf("1. Consultar movimentacao consolidada\n");
      printf("2. Filtrar movimentacoes\n");
      printf("3. Sair\n");
      printf("Escolha uma opcao: ");
      scanf("%d", &opcao);

      if (opcao == 1) {
          int mes, ano;
          printf("Informe o mes (1-12): ");
          scanf("%d", &mes);
          printf("Informe o ano: ");
          scanf("%d", &ano);

          if (!verificar_existencia_arquivo(mes, ano)) {
              ler_csv("transacoes.csv");
              consolidar_movimentacao(mes, ano);
              salvar_resultados(mes, ano);
          } else {
              carregar_movimentacoes(mes, ano);
          }
      } else if (opcao == 2) {
          int mes, ano;
          printf("Informe o mes (1-12): ");
          scanf("%d", &mes);
          printf("Informe o ano: ");
          scanf("%d", &ano);

          if (!verificar_existencia_arquivo(mes, ano)) {
              ler_csv("transacoes2.csv");
              consolidar_movimentacao(mes, ano);
              salvar_resultados(mes, ano);
          }

          carregar_movimentacoes(mes, ano);

          float valor_x, valor_y;
          char operador[3];
          printf("Informe o valor X (em especie): ");
          scanf("%f", &valor_x);
          printf("Informe o valor Y (em transacoes eletronicas): ");
          scanf("%f", &valor_y);
          printf("Informe o operador (E/OU): ");
          scanf("%s", operador);

          filtrar_movimentacao(valor_x, valor_y, operador);
      } else if (opcao == 3) {
          printf("Saindo...\n");
      } else {
          printf("Opcao invalida! Tente novamente.\n");
      }
  } while (opcao != 3);
}

int main() {
  menu_interativo();
  return EXIT_SUCCESS;
}
