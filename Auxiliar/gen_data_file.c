#include "carga.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define DB_FILE "GraosRecebidos-2024.dat"

#define N_CARREGAMENTOS 250

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
    carga->cod_carga = rand() % 1000 + 1; // valores entre 1 e 999
    carga->mes_receb = rand() % 12 + 1; // número de meses
    carga->dia_receb = rand() % 31 + 1; // número de dias
    carga->tp_transgenico = rand() % 2; // apenas 0 e 1 permitidos
    carga->peso_bruto_t = (float) (rand() % 250 + 50) * 0.1;
    carga->perc_impurezas = (float) (rand() % 100 + 10) * 0.1;
    carga->perc_umidade = (float) (rand() % 300 + 10) * 0.1;
}

int
main()
{
    _cleanup_(cleanup_free) Carga *carga;
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

    return EXIT_SUCCESS;
}
