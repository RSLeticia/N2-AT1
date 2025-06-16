#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

typedef struct {
    time_t timestamp;
    char sensor_id[50];
    char value[256];
} SensorReading;

// Converte string "DD/MM/YYYY HH:MM:SS" para time_t
time_t converte_para_timestamp(const char *datahora) {
    struct tm tm;
    memset(&tm, 0, sizeof(struct tm));

    if (sscanf(datahora, "%d/%d/%d %d:%d:%d",
               &tm.tm_mday, &tm.tm_mon, &tm.tm_year,
               &tm.tm_hour, &tm.tm_min, &tm.tm_sec) != 6) {
        fprintf(stderr, "Erro: formato inválido. Use: DD/MM/AAAA HH:MM:SS\n");
        exit(1);
    }

    tm.tm_year -= 1900;
    tm.tm_mon -= 1;

    time_t t = mktime(&tm);
    if (t == -1) {
        fprintf(stderr, "Erro ao converter data/hora.\n");
        exit(1);
    }

    return t;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <sensor_id> \"DD/MM/AAAA HH:MM:SS\"\n", argv[0]);
        return 1;
    }

    char *sensorId = argv[1];
    char *datahora_str = argv[2];
    time_t queryTimestamp = converte_para_timestamp(datahora_str);

    char inputFileName[100];
    snprintf(inputFileName, sizeof(inputFileName), "%s.txt", sensorId);

    FILE *inputFile = fopen(inputFileName, "r");
    if (!inputFile) {
        perror("Erro ao abrir o arquivo");
        return 1;
    }

    SensorReading *allReadings = NULL;
    int numReadings = 0, capacity = 0;
    char line[512];

    while (fgets(line, sizeof(line), inputFile)) {
        if (numReadings >= capacity) {
            capacity = capacity == 0 ? 10 : capacity * 2;
            SensorReading *temp = realloc(allReadings, capacity * sizeof(SensorReading));
            if (!temp) {
                perror("Erro de alocação de memória");
                fclose(inputFile);
                free(allReadings);
                return 1;
            }
            allReadings = temp;
        }

        if (sscanf(line, "%ld %49s %255s",
                   &allReadings[numReadings].timestamp,
                   allReadings[numReadings].sensor_id,
                   allReadings[numReadings].value) == 3) {
            numReadings++;
        }
    }

    fclose(inputFile);

    if (numReadings == 0) {
        fprintf(stderr, "Nenhuma leitura encontrada no arquivo.\n");
        free(allReadings);
        return 1;
    }

    // Busca binária
    int low = 0, high = numReadings - 1, mid;
    int closest_idx = -1;

    while (low <= high) {
        mid = (low + high) / 2;
        if (allReadings[mid].timestamp == queryTimestamp) {
            closest_idx = mid;
            break;
        } else if (allReadings[mid].timestamp < queryTimestamp) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (closest_idx == -1) {
        if (low >= numReadings)
            closest_idx = numReadings - 1;
        else if (high < 0)
            closest_idx = 0;
        else {
            long diffLow = labs(allReadings[low].timestamp - queryTimestamp);
            long diffHigh = labs(allReadings[high].timestamp - queryTimestamp);
            closest_idx = (diffLow < diffHigh) ? low : high;
        }
    }

    char data_formatada[30];
    strftime(data_formatada, sizeof(data_formatada), "%d/%m/%Y %H:%M:%S",
             localtime(&allReadings[closest_idx].timestamp));

    printf("\nLeitura mais próxima encontrada:\n");
    printf(" - Timestamp: %ld (%s)\n", allReadings[closest_idx].timestamp, data_formatada);
    printf(" - Sensor ID: %s\n", allReadings[closest_idx].sensor_id);
    printf(" - Valor: %s\n\n", allReadings[closest_idx].value);

    free(allReadings);
    return 0;
}
