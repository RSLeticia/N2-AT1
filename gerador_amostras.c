#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define NUM_LEITURAS 2000
#define MAX_SENSORES 4

typedef enum { TIPO_INT, TIPO_BOOL, TIPO_FLOAT, TIPO_STRING } TipoSensor;

typedef struct {
    char nome[20];
    TipoSensor tipo;
} Sensor;

int gerar_timestamps_unicos(time_t inicio, time_t fim, time_t *arr, int n) {
    if (fim - inicio + 1 < n) {
        return 0; // intervalo insuficiente para timestamps únicos
    }

    // Preenche array com todos timestamps possíveis no intervalo
    int total = (int)(fim - inicio + 1);
    time_t *todos = malloc(total * sizeof(time_t));
    if (!todos) return 0;

    for (int i = 0; i < total; i++) {
        todos[i] = inicio + i;
    }

    // Embaralha o array (Fisher–Yates shuffle)
    for (int i = total - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        time_t tmp = todos[i];
        todos[i] = todos[j];
        todos[j] = tmp;
    }

    // Copia os primeiros n timestamps para o array de saída
    for (int i = 0; i < n; i++) {
        arr[i] = todos[i];
    }

    free(todos);
    return 1;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Uso: %s \"dd/mm/aaaa hh:mm:ss\" \"dd/mm/aaaa hh:mm:ss\"\n", argv[0]);
        return 1;
    }

    struct tm tm_inicio, tm_fim;

    // Função para converter data BR "dd/mm/aaaa hh:mm:ss" em time_t
    int dia, mes, ano, hora, min, seg;
    if (sscanf(argv[1], "%d/%d/%d %d:%d:%d", &dia, &mes, &ano, &hora, &min, &seg) != 6) {
        printf("Formato data/hora inválido (inicio).\n");
        return 1;
    }
    tm_inicio.tm_year = ano - 1900;
    tm_inicio.tm_mon = mes - 1;
    tm_inicio.tm_mday = dia;
    tm_inicio.tm_hour = hora;
    tm_inicio.tm_min = min;
    tm_inicio.tm_sec = seg;
    tm_inicio.tm_isdst = -1;
    time_t inicio = mktime(&tm_inicio);

    if (sscanf(argv[2], "%d/%d/%d %d:%d:%d", &dia, &mes, &ano, &hora, &min, &seg) != 6) {
        printf("Formato data/hora inválido (fim).\n");
        return 1;
    }
    tm_fim.tm_year = ano - 1900;
    tm_fim.tm_mon = mes - 1;
    tm_fim.tm_mday = dia;
    tm_fim.tm_hour = hora;
    tm_fim.tm_min = min;
    tm_fim.tm_sec = seg;
    tm_fim.tm_isdst = -1;
    time_t fim = mktime(&tm_fim);

    if (fim <= inicio) {
        printf("Data/hora final deve ser maior que inicial.\n");
        return 1;
    }

    Sensor sensores[MAX_SENSORES] = {
        {"int", TIPO_INT},
        {"bool", TIPO_BOOL},
        {"float", TIPO_FLOAT},
        {"string", TIPO_STRING}
    };

    srand(time(NULL));

    FILE *f = fopen("dados.txt", "w");
    if (!f) {
        perror("Erro ao criar arquivo dados.txt");
        return 1;
    }

    time_t timestamps[NUM_LEITURAS];

    for (int i = 0; i < MAX_SENSORES; i++) {
        if (!gerar_timestamps_unicos(inicio, fim, timestamps, NUM_LEITURAS)) {
            printf("Intervalo muito pequeno para gerar timestamps únicos.\n");
            fclose(f);
            return 1;
        }

        // Ordena timestamps para ficar crescente no arquivo (opcional)
        qsort(timestamps, NUM_LEITURAS, sizeof(time_t), 
            (int(*)(const void*,const void*)) strcmp);

        for (int j = 0; j < NUM_LEITURAS; j++) {
            switch(sensores[i].tipo) {
                case TIPO_INT:
                    fprintf(f, "%ld %s %d\n", timestamps[j], sensores[i].nome, rand() % 1000);
                    break;
                case TIPO_BOOL:
                    fprintf(f, "%ld %s %s\n", timestamps[j], sensores[i].nome, (rand() % 2) ? "true" : "false");
                    break;
                case TIPO_FLOAT:
                    fprintf(f, "%ld %s %.2f\n", timestamps[j], sensores[i].nome, ((float)rand()/RAND_MAX)*1000.0f);
                    break;
                case TIPO_STRING: {
                    // gerar string aleatória até 16 letras
                    char str[17];
                    int len = 5 + rand() % 12; // entre 5 e 16 chars
                    for (int k = 0; k < len; k++) {
                        str[k] = 'a' + (rand() % 26);
                    }
                    str[len] = '\0';
                    fprintf(f, "%ld %s %s\n", timestamps[j], sensores[i].nome, str);
                    break;
                }
            }
        }
    }

    fclose(f);
    printf("Arquivo 'dados.txt' gerado com sucesso.\n");

    return 0;
}
