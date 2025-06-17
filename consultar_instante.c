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

time_t converte_para_timestamp(const char *datahora) {
    struct tm tm_input = {0};
    int d, m, y, H, M, S;

    if (sscanf(datahora, "%d/%d/%d %d:%d:%d", &d, &m, &y, &H, &M, &S) != 6) {
        fprintf(stderr, "Erro: formato invalido. Use: DD/MM/AAAA HH:MM:SS\n");
        exit(1);
    }

    tm_input.tm_mday = d;
    tm_input.tm_mon = m - 1;
    tm_input.tm_year = y - 1900;
    tm_input.tm_hour = H;
    tm_input.tm_min = M;
    tm_input.tm_sec = S;

    time_t t = mktime(&tm_input);
    if (t == -1) {
        fprintf(stderr, "Erro: data/hora invalida.\n");
        exit(1);
    }

    struct tm *check = localtime(&t);
    if (!check ||
        check->tm_mday != d || check->tm_mon != m - 1 || check->tm_year != y - 1900 ||
        check->tm_hour != H || check->tm_min != M || check->tm_sec != S) {
        fprintf(stderr, "Erro: data ou hora invalida.\n");
        exit(1);
    }

    return t;
}

int comparar_desc(const void *a, const void *b) {
    const SensorReading *ra = (const SensorReading *)a;
    const SensorReading *rb = (const SensorReading *)b;
    if (ra->timestamp < rb->timestamp) return 1;
    if (ra->timestamp > rb->timestamp) return -1;
    return 0;
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
                perror("Erro de alocação de memoria");
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

    qsort(allReadings, numReadings, sizeof(SensorReading), comparar_desc);

    int low = 0, high = numReadings - 1, mid;
    int closest_idx = -1;

    while (low <= high) {
        mid = (low + high) / 2;
        if (allReadings[mid].timestamp == queryTimestamp) {
            closest_idx = mid;
            break;
        } else if (allReadings[mid].timestamp < queryTimestamp) {
            high = mid - 1;
        } else {
            low = mid + 1;
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

    printf("\nLeitura mais proxima encontrada:\n");
    printf(" - Timestamp: %ld (%s)\n", allReadings[closest_idx].timestamp, data_formatada);
    printf(" - Sensor ID: %s\n", allReadings[closest_idx].sensor_id);
    printf(" - Valor: %s\n\n", allReadings[closest_idx].value);

    free(allReadings);
    return 0;
}
