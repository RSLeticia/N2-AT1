**OBJETIVOS DA SOLUÇÃO**
---
O objetivo principal desta solução é simular, organizar e permitir a consulta eficiente de dados sensoriais em um ambiente industrial inteligente, por meio do desenvolvimento de três programas integrados escritos em linguagem C. Esses programas têm como finalidade:

Gerar automaticamente dados simulados de sensores com valores coerentes aos seus respectivos tipos (inteiro, booleano, ponto flutuante e string), associados a timestamps únicos dentro de um intervalo de tempo definido.

Processar e estruturar os dados gerados, organizando-os por tipo de sensor e ordenando as leituras cronologicamente. Cada sensor é tratado de forma independente, com suas leituras salvas em arquivos separados.

Permitir consultas rápidas por timestamp, retornando ao usuário a leitura registrada mais próxima de um instante de tempo informado, por meio de busca binária aplicada sobre os dados organizados.

A solução busca transformar um arquivo bruto e desorganizado de leituras sensoriais em um sistema estruturado, confiável e eficiente para análise e tomada de decisão em tempo real ou posterior.

**ESPECIFICAÇÃO DOS PROGRAMAS**

**É indicado que o programa seja utilizado na seguinte ordem:** Programa 3 - Programa 1 - Programa 2.

**PROGRAMA 1 - organizador_arquivos**

Este programa tem como finalidade ler o arquivo gerado pelo programa 3 - gerador_amostras(esse programa vai gerar o arquivo dados.txt) e organizar os dados por sensor.

**Entrada:**

**Argumento na linha de comando:** ./organizador_arquivos dados.txt.

**Algoritmo de ordenação utilizado:**

O programa usa quicksort **(qsort da biblioteca padrão <stdlib.h>)** com uma função personalizada para comparar timestamps em ordem crescente.

**Estrutura do código e principais funções implementadas:**

**detectar_tipo_valor():** Identifica se o valor é int, bool, float ou string.

**buscar_sensor() / adicionar_sensor():** Gerencia sensores únicos com alocação dinâmica.

**adicionar_leitura():** Insere leituras dinamicamente com realocação se necessário.

**comparar_leitura():** Função de comparação para qsort(), baseada em timestamp.

**gravar_arquivo_sensor** Cria um arquivo para cada sensor, gravando os dados ordenadamente.

**Saída:**

Criar um arquivo separado para cada sensor , com o nome <nome_sensor>.txt.


**PROGRAMA 2 - consultar_instante**

Neste programa o usuário poderá consultar a leitura mais próxima de um instante de tempo para um sensor específico.

**Entrada:**

**Serão dois argumentos na linha de comando:** o nome do sensor(ex:int) e a data/hora(“20/02/2002 00:00:00”).

**Ex:** ./consultar_instante float “03/01/2005 00:00:20”

**Algoritmo de ordenação utilizado:**

Não é feita ordenação, pois os arquivos de sensores já vêm ordenados do organizador_arquivos.

A busca usa algoritmo de **busca binária** adaptado para encontrar o timestamp mais próximo.

**Estrutura do código e principais funções implementadas:**

**converte_para_timestamp():** Converte string de data/hora para time_t.

**Struct SensorReading:** Armazena timestamp, nome do sensor e valor.

**Busca binária:** Localiza a leitura mais próxima considerando aproximação superior/inferior.

**Formatação de saída:** Impressão formatada com strftime.

**Saída:**

Irá imprimir no terminal: O timestamp, o nome do sensor e o valor da leitura.


**PROGRAMA 3 - gerador_amostras**

O programa vai simular a coleta de dados de sensores ao longo de um intervalo de tempo informado pelo usuário.

**Entrada:**

Serão dois argumentos na linha de comando: data e hora inicial e data e hora final.

**Ex:** ./gerador_amostras “20/01/2001 00:00:00” “10/04/2002 00:00:00”.

**Algoritmo de ordenação utilizado:**

Os timestamps não são ordenados globalmente, mas são embaralhados com **Fisher–Yates Shuffle**, garantindo unicidade e aleatoriedade dentro do intervalo.

**Estrutura do código e principais funções implementadas:**

**gerar_timestamps_unicos():**

Gera valores únicos dentro do intervalo usando shuffle.

**Struct Sensor:** Contém nome e tipo do sensor (enum TipoSensor).

**Geradores de valor por tipo:**

**rand() % 1000** → inteiros

**rand() % 2** → booleanos

((float)rand()/RAND_MAX)*1000.0f → floats

**strings aleatórias de 5 a 16 caracteres** → tipo string

**fprintf():** Escrita dos dados no arquivo dados.txt.

**Saída:**

Gera um arquivo (dados.txt) contendo todas as leituras.

<timestamp> <tipo_sensor> <valor>

**PROBLEMAS ENCONTRADOS E SOLUÇÕES ADOTADAS**

**Arquivo de entrada com dados desordenados:** Utilização de qsort para ordenar as leituras de cada sensor por timestamp.

**Timestamps duplicados ou fora do intervalo durante a geração:** Implementação do algoritmo Fisher–Yates Shuffle para garantir unicidade e aleatoriedade dos timestamps.

**Dificuldade em identificar o tipo do valor (int, float, bool ou string):** Regras simples com strtol, strtof e verificação literal para booleanos; strings por exclusão.

**Leituras com tipo diferente do declarado anteriormente para o mesmo sensor:** Validação de tipo na leitura; leituras inconsistentes são ignoradas com aviso ao usuário.

**Crescimento dinâmico e imprevisível da quantidade de leituras por sensor:** Uso de alocação dinâmica com malloc e redimensionamento com realloc.

**Alto volume de dados em memória:** Estratégia de leitura e gravação por sensor, evitando manter todos os dados globais simultaneamente.

**Arquivo de sensor pode não existir no momento da consulta:** Verificação com fopen() e mensagens de erro amigáveis quando o arquivo não é encontrado.

**Consulta por timestamp exato nem sempre possível:** Implementação de busca binária aproximada, retornando a leitura mais próxima.

**Conversão frágil de data/hora em texto para timestamp:** Uso de sscanf com validação completa + mktime() para obter time_t com segurança.

**Strings aleatórias ilegíveis ou inválidas:** Geração controlada de strings com letras minúsculas e tamanho entre 5 e 16 caracteres.

*Código com manipulação de tipos diferentes em uma única estrutura: Uso de union dentro da struct Leitura para armazenar valores de tipos distintos.

**Dificuldade de leitura dos resultados no terminal:** Uso de strftime() para formatar o timestamp de forma legível ao usuário.
