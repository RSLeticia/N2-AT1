#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MAX_SENSORES 2000
#define MAX_LEITURAS_POR_SENSOR 2000
#define MAX_NOME_SENSOR 50
#define MAX_STRING_VALOR 17  

typedef enum {
    TIPO_INT,
    TIPO_BOOL,
    TIPO_FLOAT,
    TIPO_STRING,
    TIPO_INVALIDO
} TipoSensor;

typedef struct {
    long timestamp;
    union {
        int i_val;
        bool b_val;
        float f_val;
        char s_val[MAX_STRING_VALOR];
    } valor;
} Leitura;

typedef struct {
    char nome[MAX_NOME_SENSOR];
    TipoSensor tipo;
    Leitura *leituras;
    size_t qtd;
    size_t capacidade;
} Sensor;

Sensor sensores[MAX_SENSORES];
size_t total_sensores = 0;

TipoSensor detectar_tipo_valor(const char *valor_str) {
    if (strcmp(valor_str, "true") == 0 || strcmp(valor_str, "false") == 0) {
        return TIPO_BOOL;
    }
    char *endptr;
    long val_int = strtol(valor_str, &endptr, 10);
    if (*endptr == '\0') {
        return TIPO_INT;
    }
    float val_float = strtof(valor_str, &endptr);
    if (*endptr == '\0') {
        return TIPO_FLOAT;
    }
    if (strlen(valor_str) <= 16) {
        return TIPO_STRING;
    }
    return TIPO_INVALIDO;
}

const char *nome_tipo(TipoSensor tipo) {
    switch(tipo) {
        case TIPO_INT: return "int";
        case TIPO_BOOL: return "bool";
        case TIPO_FLOAT: return "float";
        case TIPO_STRING: return "string";
        default: return "invalido";
    }
}

int buscar_sensor(const char *nome) {
    for (size_t i = 0; i < total_sensores; i++) {
        if (strcmp(sensores[i].nome, nome) == 0)
            return (int)i;
    }
    return -1;
}

int adicionar_sensor(const char *nome, TipoSensor tipo) {
    if (total_sensores >= MAX_SENSORES) {
        fprintf(stderr, "Erro: maximo de sensores (%d) atingido.\n", MAX_SENSORES);
        exit(1);
    }
    strncpy(sensores[total_sensores].nome, nome, MAX_NOME_SENSOR);
    sensores[total_sensores].nome[MAX_NOME_SENSOR-1] = '\0';
    sensores[total_sensores].tipo = tipo;
    sensores[total_sensores].qtd = 0;
    sensores[total_sensores].capacidade = 100;
    sensores[total_sensores].leituras = malloc(sizeof(Leitura) * sensores[total_sensores].capacidade);
    if (!sensores[total_sensores].leituras) {
        fprintf(stderr, "Erro de alocação de memoria.\n");
        exit(1);
    }
    total_sensores++;
    return (int)(total_sensores - 1);
}

void adicionar_leitura(int idx_sensor, Leitura *l) {
    Sensor *s = &sensores[idx_sensor];
    if (s->qtd >= MAX_LEITURAS_POR_SENSOR) {
        return;
    }
    if (s->qtd == s->capacidade) {
        size_t nova_cap = s->capacidade * 2;
        if (nova_cap > MAX_LEITURAS_POR_SENSOR) nova_cap = MAX_LEITURAS_POR_SENSOR;
        Leitura *novo = realloc(s->leituras, nova_cap * sizeof(Leitura));
        if (!novo) {
            fprintf(stderr, "Erro de alocação de memoria.\n");
            exit(1);
        }
        s->leituras = novo;
        s->capacidade = nova_cap;
    }
    s->leituras[s->qtd++] = *l;
}

int comparar_leitura(const void *a, const void *b) {
    Leitura *la = (Leitura*)a;
    Leitura *lb = (Leitura*)b;
    if (la->timestamp < lb->timestamp) return 1;   
    if (la->timestamp > lb->timestamp) return -1;  
    return 0;
}

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
int numero_linha = 0;
while (fgets(linha, sizeof(linha), entrada)) {
    numero_linha++;

    long timestamp;
    char nome_sensor[MAX_NOME_SENSOR];
    char valor_str[100];

    int lidos = sscanf(linha, "%ld %49s %99[^\n]", &timestamp, nome_sensor, valor_str);

    if (lidos != 3) {
        fprintf(stderr, "\nLinha %d invalida: \"%s\"", numero_linha, linha);

        if (lidos == 0) {
            fprintf(stderr, " Erro: linha sem timestamp, nome e valor.\n");
        } else if (lidos == 1) {
            fprintf(stderr, " Erro: linha sem nome do sensor e valor.\n");
        } else if (lidos == 2) {
            fprintf(stderr, " Erro: linha sem valor do sensor.\n");
        } else {
            fprintf(stderr, " Erro: formato inesperado.\n");
        }

        continue;
    }

        int idx = buscar_sensor(nome_sensor);

        TipoSensor tipo_valor = detectar_tipo_valor(valor_str);
        if (tipo_valor == TIPO_INVALIDO) {
            fprintf(stderr, "Valor invalido para sensor %s: %s\n", nome_sensor, valor_str);
            continue;
        }

        if (idx == -1) {
            idx = adicionar_sensor(nome_sensor, tipo_valor);
        } else {
            if (sensores[idx].tipo != tipo_valor) {
                fprintf(stderr, "Tipo inconsistente para sensor %s. Ignorando linha.\n", nome_sensor);
                continue;
            }
        }

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

    for (size_t i = 0; i < total_sensores; i++) {
        qsort(sensores[i].leituras, sensores[i].qtd, sizeof(Leitura), comparar_leitura);
        gravar_arquivo_sensor(&sensores[i]);
    }

    for (size_t i = 0; i < total_sensores; i++) {
        free(sensores[i].leituras);
    }

    printf("Processamento concluido. Foram processados %zu sensores.\n", total_sensores);

    return 0;
}
