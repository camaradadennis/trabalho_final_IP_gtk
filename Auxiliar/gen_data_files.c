#include "carga.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DB_FILE "GraosRecebidos-2024.dat"
#define NOVO_FILE "1_Novo_Carregamento"

#define N_CARREGAMENTOS 250
#define N_AMOSTRAS 25

#define _cleanup_(f) __attribute((cleanup(f)))

#define fatal(msg) do { perror(msg); exit(EXIT_FAILURE); } while (0)

void
cleanup_free(void *_p)
{
    void **p = (void **) _p;
    if (*p)
        free(*p);
}

void
gen_carga(Carga *carga)
{
    carga->origem = rand() % 5 + 1; // só cinco origens diferentes
    carga->cod_carga = rand() % 1000 + 1;
    carga->mes_receb = rand() % 12 + 1;
    carga->dia_receb = rand() % 31 + 1;
    carga->tp_transgenico = rand() % 2;
    carga->peso_bruto_t = (float) (rand() % 250 + 50) * 0.1;
    carga->perc_impurezas = (float) (rand() % 100 + 10) * 0.1;
    carga->perc_umidade = (float) (rand() % 300 + 10) * 0.1;
}

void
gen_amostra(Amostra *amostra, int id)
{
    amostra->ident = id;
    amostra->peso_bruto_kg = (float) (rand() % 50 + 10) * 0.1;
    amostra->peso_impurezas_g = rand() % 100 + 10;
    amostra->grau_umidade = (float) (rand() % 300 + 10) * 0.1;
}

int
main()
{
    _cleanup_(cleanup_free) Carga *carga;
    _cleanup_(cleanup_free) Amostra *amostra;
    FILE *fp;

    srand(time(0));

    carga = malloc(N_CARREGAMENTOS * sizeof(Carga));
    if (!carga)
        fatal("Erro de alocação de memória");

    for (int i = 0; i < N_CARREGAMENTOS; ++i)
        gen_carga(&carga[i]);

    fp = fopen(DB_FILE, "wb");
    if (!fp)
        fatal("Erro de abertura do arquivo");

    fwrite(carga, sizeof(Carga), N_CARREGAMENTOS, fp);

    if (fclose(fp))
        fatal("Erro de fechamento do arquivo");

    amostra = malloc(N_AMOSTRAS * sizeof(Amostra));
    if (!amostra)
        fatal("Erro de alocação de memória");

    for (int i = 0; i < N_AMOSTRAS; ++i)
        gen_amostra(&amostra[i], i + 1);

    fp = fopen(NOVO_FILE, "w");
    if (!fp)
        fatal("Erro de abertura do arquivo");

    fprintf(fp, "%d %d %.1f %d %d\n", 123, 88, 20.5, N_AMOSTRAS, 0);

    for (int i = 0; i < N_AMOSTRAS; ++i)
    {
        fprintf(fp, "%d %.1f %d %.1f\n",
            amostra[i].ident, amostra[i].peso_bruto_kg,
            amostra[i].peso_impurezas_g, amostra[i].grau_umidade);
    }

    if (fclose(fp))
        fatal("Erro de fechamento do arquivo");

    return EXIT_SUCCESS;
}
