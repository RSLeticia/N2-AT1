#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SENSORES 2000
#define MAX_LEITURAS_POR_SENSOR 2000
#define MAX_NOME_SENSOR 50
#define MAX_STRING_VALOR 17  // 16 letras + '\0'

// Tipos possíveis
typedef enum {
    TIPO_INT,
    TIPO_BOOL,
    TIPO_FLOAT,
    TIPO_STRING,
    TIPO_INVALIDO
} TipoSensor;

// Armazena uma leitura
typedef struct {
    long timestamp;
    union {
        int i_val;
        bool b_val;
        float f_val;
        char s_val[MAX_STRING_VALOR];
    } valor;
} Leitura;

// Armazena sensor e suas leituras
typedef struct {
    char nome[MAX_NOME_SENSOR];
    TipoSensor tipo;
    Leitura *leituras;
    size_t qtd;
    size_t capacidade;
} Sensor;

Sensor sensores[MAX_SENSORES];
size_t total_sensores = 0;

// Função para determinar tipo do sensor lendo a string do valor
TipoSensor detectar_tipo_valor(const char *valor_str) {
    // bool: "true" ou "false"
    if (strcmp(valor_str, "true") == 0 || strcmp(valor_str, "false") == 0) {
        return TIPO_BOOL;
    }
    // int: verifica se todos os chars são dígitos (com possível sinal no início)
    char *endptr;
    long val_int = strtol(valor_str, &endptr, 10);
    if (*endptr == '\0') {
        return TIPO_INT;
    }
    // float: tenta converter float
    float val_float = strtof(valor_str, &endptr);
    if (*endptr == '\0') {
        return TIPO_FLOAT;
    }
    // string: se chegou aqui é string (máximo 16 caracteres)
    if (strlen(valor_str) <= 16) {
        return TIPO_STRING;
    }
    return TIPO_INVALIDO;
}

// Retorna string nome do tipo
const char *nome_tipo(TipoSensor tipo) {
    switch(tipo) {
        case TIPO_INT: return "int";
        case TIPO_BOOL: return "bool";
        case TIPO_FLOAT: return "float";
        case TIPO_STRING: return "string";
        default: return "invalido";
    }
}

// Busca sensor pelo nome, ou retorna -1 se não achar
int buscar_sensor(const char *nome) {
    for (size_t i = 0; i < total_sensores; i++) {
        if (strcmp(sensores[i].nome, nome) == 0)
            return (int)i;
    }
    return -1;
}

// Adiciona um novo sensor à lista
int adicionar_sensor(const char *nome, TipoSensor tipo) {
    if (total_sensores >= MAX_SENSORES) {
        fprintf(stderr, "Erro: máximo de sensores (%d) atingido.\n", MAX_SENSORES);
        exit(1);
    }
    strncpy(sensores[total_sensores].nome, nome, MAX_NOME_SENSOR);
    sensores[total_sensores].nome[MAX_NOME_SENSOR-1] = '\0';
    sensores[total_sensores].tipo = tipo;
    sensores[total_sensores].qtd = 0;
    sensores[total_sensores].capacidade = 100;
    sensores[total_sensores].leituras = malloc(sizeof(Leitura) * sensores[total_sensores].capacidade);
    if (!sensores[total_sensores].leituras) {
        fprintf(stderr, "Erro de alocação de memória.\n");
        exit(1);
    }
    total_sensores++;
    return (int)(total_sensores - 1);
}

// Adiciona leitura ao sensor
void adicionar_leitura(int idx_sensor, Leitura *l) {
    Sensor *s = &sensores[idx_sensor];
    if (s->qtd >= MAX_LEITURAS_POR_SENSOR) {
        // Já atingiu limite de leituras por sensor, ignora extras
        return;
    }
    if (s->qtd == s->capacidade) {
        size_t nova_cap = s->capacidade * 2;
        if (nova_cap > MAX_LEITURAS_POR_SENSOR) nova_cap = MAX_LEITURAS_POR_SENSOR;
        Leitura *novo = realloc(s->leituras, nova_cap * sizeof(Leitura));
        if (!novo) {
            fprintf(stderr, "Erro de alocação de memória.\n");
            exit(1);
        }
        s->leituras = novo;
        s->capacidade = nova_cap;
    }
    s->leituras[s->qtd++] = *l;
}

// Função de comparação para qsort por timestamp crescente
int comparar_leitura(const void *a, const void *b) {
    Leitura *la = (Leitura*)a;
    Leitura *lb = (Leitura*)b;
    if (la->timestamp < lb->timestamp) return -1;
    if (la->timestamp > lb->timestamp) return 1;
    return 0;
}

// Grava arquivo do sensor com leituras ordenadas
void gravar_arquivo_sensor(const Sensor *s) {
    char nome_arquivo[100];
    snprintf(nome_arquivo, sizeof(nome_arquivo), "%s.txt", s->nome);

    FILE *f = fopen(nome_arquivo, "w");
    if (!f) {
        fprintf(stderr, "Erro ao criar arquivo %s\n", nome_arquivo);
        return;
    }

    for (size_t i = 0; i < s->qtd; i++) {
        Leitura *l = &s->leituras[i];
        switch (s->tipo) {
            case TIPO_INT:
                fprintf(f, "%ld %s %d\n", l->timestamp, s->nome, l->valor.i_val);
                break;
            case TIPO_BOOL:
                fprintf(f, "%ld %s %s\n", l->timestamp, s->nome, l->valor.b_val ? "true" : "false");
                break;
            case TIPO_FLOAT:
                fprintf(f, "%ld %s %f\n", l->timestamp, s->nome, l->valor.f_val);
                break;
            case TIPO_STRING:
                fprintf(f, "%ld %s %s\n", l->timestamp, s->nome, l->valor.s_val);
                break;
            default:
                break;
        }
    }
    fclose(f);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Uso: %s <arquivo_entrada>\n", argv[0]);
        return 1;
    }

    FILE *entrada = fopen(argv[1], "r");
    if (!entrada) {
        perror("Erro ao abrir arquivo de entrada");
        return 1;
    }

    char linha[256];
    while (fgets(linha, sizeof(linha), entrada)) {
        long timestamp;
        char nome_sensor[MAX_NOME_SENSOR];
        char valor_str[100];

        // Ler a linha no formato: <TIMESTAMP> <ID_SENSOR> <VALOR>
        int lidos = sscanf(linha, "%ld %49s %99[^\n]", &timestamp, nome_sensor, valor_str);
        if (lidos != 3) {
            fprintf(stderr, "Linha ignorada por formato inválido: %s", linha);
            continue;
        }

        int idx = buscar_sensor(nome_sensor);

        TipoSensor tipo_valor = detectar_tipo_valor(valor_str);
        if (tipo_valor == TIPO_INVALIDO) {
            fprintf(stderr, "Valor inválido para sensor %s: %s\n", nome_sensor, valor_str);
            continue;
        }

        if (idx == -1) {
            // Sensor novo: adiciona com o tipo detectado
            idx = adicionar_sensor(nome_sensor, tipo_valor);
        } else {
            // Sensor já existe: checa se o tipo bate
            if (sensores[idx].tipo != tipo_valor) {
                fprintf(stderr, "Tipo inconsistente para sensor %s. Ignorando linha.\n", nome_sensor);
                continue;
            }
        }

        // Monta leitura e adiciona
        Leitura nova_leitura;
        nova_leitura.timestamp = timestamp;

        switch (tipo_valor) {
            case TIPO_INT:
                nova_leitura.valor.i_val = atoi(valor_str);
                break;
            case TIPO_BOOL:
                nova_leitura.valor.b_val = (strcmp(valor_str, "true") == 0);
                break;
            case TIPO_FLOAT:
                nova_leitura.valor.f_val = atof(valor_str);
                break;
            case TIPO_STRING:
                strncpy(nova_leitura.valor.s_val, valor_str, MAX_STRING_VALOR);
                nova_leitura.valor.s_val[MAX_STRING_VALOR-1] = '\0';
                break;
            default:
                break;
        }

        adicionar_leitura(idx, &nova_leitura);
    }

    fclose(entrada);

    // Ordena e grava arquivos para cada sensor
    for (size_t i = 0; i < total_sensores; i++) {
        qsort(sensores[i].leituras, sensores[i].qtd, sizeof(Leitura), comparar_leitura);
        gravar_arquivo_sensor(&sensores[i]);
    }

    // Liberar memória
    for (size_t i = 0; i < total_sensores; i++) {
        free(sensores[i].leituras);
    }

    printf("Processamento concluído. Foram processados %zu sensores.\n", total_sensores);

    return 0;
}
