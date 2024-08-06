#include "carga.h"

#include <stddef.h>

struct FaixaGU {
    char *nome;
    float gu_min;
    float gu_max;
};

static const struct FaixaGU faixas_gu[] = {
    {"Faixa 1", 0, 8.5},
    {"Faixa 2", 8.5, 15},
    {"Faixa 3", 15, 25}
};

static const Carga carga_valores_min = {
    .origem = 1,
    .cod_carga = 1,
    .mes_receb = 1,
    .dia_receb = 1,
    .tp_transgenico = 0,
    .peso_bruto_t = 0,
    .perc_impurezas = 0,
    .perc_umidade = 0
};

static const Carga carga_valores_max = {
    .origem = 999,
    .cod_carga = 1000,
    .mes_receb = 12,
    .dia_receb = 31,
    .tp_transgenico = 1,
    .peso_bruto_t = 100000,
    .perc_impurezas = 100,
    .perc_umidade = 100
};

void
copia_carga(Carga *dest, const Carga *src)
{
    dest->origem = src->origem;
    dest->cod_carga = src->cod_carga;
    dest->mes_receb = src->mes_receb;
    dest->dia_receb = src->dia_receb;
    dest->tp_transgenico = src->tp_transgenico;
    dest->peso_bruto_t = src->peso_bruto_t;
    dest->perc_impurezas = src->perc_impurezas;
    dest->perc_umidade = src->perc_umidade;
}

int
get_n_faixas_gu()
{
    return sizeof(faixas_gu) / sizeof(*faixas_gu);
}

int
get_faixa_gu(float umidade)
{
    int fx = 0;
    while (fx < get_n_faixas_gu() && faixas_gu[fx].gu_max < umidade)
        ++fx;

    return fx;
}

const char *
get_faixa_gu_nome(unsigned i)
{
    if (i >= get_n_faixas_gu())
        return "GU Extra";

    return faixas_gu[i].nome;
}

int carga_eh_valida(const Carga *carga)
{
    if (carga->origem            >= carga_valores_min.origem
        && carga->origem         <= carga_valores_max.origem

        && carga->cod_carga      >= carga_valores_min.cod_carga
        && carga->cod_carga      <= carga_valores_max.cod_carga

        && carga->mes_receb      >= carga_valores_min.mes_receb
        && carga->mes_receb      <= carga_valores_max.mes_receb

        && carga->dia_receb      >= carga_valores_min.dia_receb
        && carga->dia_receb      <= carga_valores_max.dia_receb

        && carga->tp_transgenico >= carga_valores_min.tp_transgenico
        && carga->tp_transgenico <= carga_valores_max.tp_transgenico

        && carga->peso_bruto_t   >= carga_valores_min.peso_bruto_t
        && carga->peso_bruto_t   <= carga_valores_max.peso_bruto_t

        && carga->perc_impurezas >= carga_valores_min.perc_impurezas
        && carga->perc_impurezas <= carga_valores_max.perc_impurezas

        && carga->perc_umidade   >= carga_valores_min.perc_umidade
        && carga->perc_umidade   <= carga_valores_max.perc_umidade
    )
        return 1;
    else
        return 0;
}
