#pragma once

typedef struct {
    int origem;
    int cod_carga;
    int mes_receb;
    int dia_receb;
    int tp_transgenico;
    float peso_bruto_t;
    float perc_impurezas;
    float perc_umidade;
} Carga;

typedef struct {
    int ident;
    float peso_bruto_kg;
    int peso_impurezas_g;
    float grau_umidade;
} Amostra;

void copia_carga(Carga *dest, const Carga *src);
int get_faixa_gu(float umidade);
int carga_eh_valida(const Carga *carga);
int get_n_faixas_gu();
const char *get_faixa_gu_nome(unsigned i);
