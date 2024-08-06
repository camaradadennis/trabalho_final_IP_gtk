#include "carga.h"
#include "item_relatorio_geral.h"

#define PERCENT(num, div) ((div != 0) ? (100 * num / div) : 0)

struct _ItemRelatorioGeral {
    GObject parent;

    int origem;
    float peso_total;
    float fx1_peso_limpo;
    float fx1_tp_trans;
    float fx1_tp_ntrans;
    float fx2_peso_limpo;
    float fx2_tp_trans;
    float fx2_tp_ntrans;
    float fx3_peso_limpo;
    float fx3_tp_trans;
    float fx3_tp_ntrans;
};

typedef enum {
    ORIGEM = 1,
    PESO_TOTAL,
    FX1_PESO_LIMPO,
    FX1_TP_TRANS,
    FX1_TP_NTRANS,
    FX2_PESO_LIMPO,
    FX2_TP_TRANS,
    FX2_TP_NTRANS,
    FX3_PESO_LIMPO,
    FX3_TP_TRANS,
    FX3_TP_NTRANS,
    N_CAMPOS
} ItemRelatorioGeralProp;

static GParamSpec *item_relatorio_geral_properties[N_CAMPOS] = { NULL, };

G_DEFINE_TYPE(ItemRelatorioGeral, item_relatorio_geral, G_TYPE_OBJECT)

int
item_relatorio_geral_get_origem(const ItemRelatorioGeral *self)
{
    return self->origem;
}

/* item_relatorio_geral_add()
 * Esta função soma os valores já armazenados em `self` com os valores de `other`
 * e emite um sinal "notify" quando algum dos valores de fato mudou.
 * O sinal é importante por que é capturado pela columnviewcolumn, que atualiza
 * o valor daquela propriedade na tabela.
 */
void
item_relatorio_geral_add(ItemRelatorioGeral *self, const ItemRelatorioGeral *other)
{
    g_return_if_fail(self->origem == other->origem);

    GObject *gobj = G_OBJECT(self);

    if (!other->peso_total)
        return; // se o carregamento não tem peso, não faz sentido continuar.

    self->peso_total += other->peso_total;
    g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[PESO_TOTAL]);

    // `other` codidifica os valores de um único carregamento,
    // por isso, ele pode estar nas faixas 1, 2 ou 3. Nunca em mais de uma.
    // Nós testamos o peso limpo em cada faixa, para seguir a convenção
    // de só emitir um sinal para as propriedades que de fato mudaram.
    // Note que as propriedades TP_TRANS e TP_NTRANS são, na verdade, percentuais.
    // Assim, elas vão mudar independentemente de a carga recebida ser transgênica
    // ou não.
    if (other->fx1_peso_limpo)
    {
        self->fx1_peso_limpo += other->fx1_peso_limpo;
        self->fx1_tp_trans += other->fx1_tp_trans;
        self->fx1_tp_ntrans += other->fx1_tp_ntrans;

        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX1_PESO_LIMPO]);
        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX1_TP_TRANS]);
        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX1_TP_NTRANS]);
    }
    else if (other->fx2_peso_limpo)
    {
        self->fx2_peso_limpo += other->fx2_peso_limpo;
        self->fx2_tp_trans += other->fx2_tp_trans;
        self->fx2_tp_ntrans += other->fx2_tp_ntrans;

        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX2_PESO_LIMPO]);
        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX2_TP_TRANS]);
        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX2_TP_NTRANS]);
    }
    else if (other->fx3_peso_limpo)
    {
        self->fx3_peso_limpo += other->fx3_peso_limpo;
        self->fx3_tp_trans += other->fx3_tp_trans;
        self->fx3_tp_ntrans += other->fx3_tp_ntrans;

        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX3_PESO_LIMPO]);
        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX3_TP_TRANS]);
        g_object_notify_by_pspec(gobj, item_relatorio_geral_properties[FX3_TP_NTRANS]);
    }
}

static void
item_relatorio_geral_fit(ItemRelatorioGeral *self, const Carga *carga)
{
    int faixa_gu = get_faixa_gu(carga->perc_umidade);
    float peso_limpo = carga->peso_bruto_t * (1 - carga->perc_impurezas / 100);

    self->origem = carga->origem;
    self->peso_total += carga->peso_bruto_t;

    switch (faixa_gu)
    {
        case 0:
            self->fx1_peso_limpo += peso_limpo;
            if (carga->tp_transgenico)
                self->fx1_tp_trans += peso_limpo;
            else
                self->fx1_tp_ntrans += peso_limpo;
            break;
        case 1:
            self->fx2_peso_limpo += peso_limpo;
            if (carga->tp_transgenico)
                self->fx2_tp_trans += peso_limpo;
            else
                self->fx2_tp_ntrans += peso_limpo;
            break;
        case 2:
            self->fx3_peso_limpo += peso_limpo;
            if (carga->tp_transgenico)
                self->fx3_tp_trans += peso_limpo;
            else
                self->fx3_tp_ntrans += peso_limpo;
            break;
        default:
            // o relatório geral só permite 3 faixas gu no momento
            // o peso total deste item vai a zero para não ser computado
            // no relatório.
            self->peso_total = 0;
            return;
    }
}

static void
item_relatorio_geral_get_property(GObject *obj,
                                  guint property_id,
                                  GValue *val,
                                  GParamSpec *pspec)
{
    ItemRelatorioGeral *self = ITEM_RELATORIO_GERAL(obj);

    switch ((ItemRelatorioGeralProp) property_id)
    {
        case ORIGEM:
            g_value_set_int(val, self->origem);
            break;
        case PESO_TOTAL:
            g_value_set_float(val, self->peso_total);
            break;
        case FX1_PESO_LIMPO:
            g_value_set_float(val, self->fx1_peso_limpo);
            break;
        case FX1_TP_TRANS:
            g_value_set_float(val,
                PERCENT(self->fx1_tp_trans, self->fx1_peso_limpo));
            break;
        case FX1_TP_NTRANS:
            g_value_set_float(val,
                PERCENT(self->fx1_tp_ntrans, self->fx1_peso_limpo));
            break;
        case FX2_PESO_LIMPO:
            g_value_set_float(val, self->fx2_peso_limpo);
            break;
        case FX2_TP_TRANS:
            g_value_set_float(val,
                PERCENT(self->fx2_tp_trans, self->fx2_peso_limpo));
            break;
        case FX2_TP_NTRANS:
            g_value_set_float(val,
                PERCENT(self->fx2_tp_ntrans, self->fx2_peso_limpo));
            break;
        case FX3_PESO_LIMPO:
            g_value_set_float(val, self->fx3_peso_limpo);
            break;
        case FX3_TP_TRANS:
            g_value_set_float(val,
                PERCENT(self->fx3_tp_trans, self->fx3_peso_limpo));
            break;
        case FX3_TP_NTRANS:
            g_value_set_float(val,
                PERCENT(self->fx3_tp_ntrans, self->fx3_peso_limpo));
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(obj, property_id, pspec);
    }
}

static void
item_relatorio_geral_init(ItemRelatorioGeral *self)
{
}

static void
item_relatorio_geral_class_init(ItemRelatorioGeralClass *cls)
{
    GObjectClass *obj_cls = G_OBJECT_CLASS(cls);

    obj_cls->get_property = item_relatorio_geral_get_property;

    item_relatorio_geral_properties[ORIGEM] =
        g_param_spec_int("origem", NULL, NULL, 0, G_MAXINT32, 0,
            G_PARAM_READABLE | G_PARAM_STATIC_STRINGS);

    // O campo 0 de item_relatorio_geral_properties é reservado
    // e o campo 1 já foi construído acima
    const char *prop_names[] = {
        NULL, NULL, "peso-total",
        "fx1-peso-limpo", "fx1-tp-trans", "fx1-tp-ntrans",
        "fx2-peso-limpo", "fx2-tp-trans", "fx2-tp-ntrans",
        "fx3-peso-limpo", "fx3-tp-trans", "fx3-tp-ntrans"
    };

    for (ItemRelatorioGeralProp prop = PESO_TOTAL; prop < N_CAMPOS; ++prop)
    {
        item_relatorio_geral_properties[prop] =
            g_param_spec_float(
                prop_names[prop], NULL, NULL, 0, G_MAXFLOAT, 0,
                G_PARAM_READABLE | G_PARAM_STATIC_STRINGS | G_PARAM_EXPLICIT_NOTIFY
            );
    }

    g_object_class_install_properties(obj_cls, N_CAMPOS,
        item_relatorio_geral_properties);
}

ItemRelatorioGeral *
item_relatorio_geral_new()
{
    return g_object_new(ITEM_RELATORIO_GERAL_TYPE, NULL);
}

ItemRelatorioGeral *
item_relatorio_geral_new_from_carga(const Carga *carga)
{
    ItemRelatorioGeral *novo = item_relatorio_geral_new();

    item_relatorio_geral_fit(novo, carga);

    return novo;
}
