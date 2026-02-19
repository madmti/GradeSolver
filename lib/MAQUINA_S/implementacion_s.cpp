#include "interface_s.hpp"
#include <algorithm>

MaquinaS::MaquinaS(const Contexto& contexto) : ctx(contexto) {}

EspacioSoluciones MaquinaS::calcular_espacio(const std::vector<Evaluacion>& evaluaciones,
                                           const std::vector<Restriccion>& restricciones) {
    EspacioSoluciones espacio;

    // 1. ¿Es físicamente posible pasar?
    espacio.es_posible = caso_extremo_optimista(evaluaciones, restricciones);

    if (!espacio.es_posible) {
        espacio.restricciones_incumplibles = identificar_criticas(evaluaciones, restricciones);
        return espacio;
    }

    // 2. Calcular límites para cada evaluación pendiente
    for (const auto& eval : evaluaciones) {
        if (!eval.valor_actual.has_value()) {
            espacio.rangos_por_evaluacion[eval.id] = buscar_limites(eval.id, evaluaciones, restricciones);
        }
    }

    return espacio;
}

bool MaquinaS::validar_escenario(const std::map<std::string, double>& escenario,
                                const std::vector<Evaluacion>& evaluaciones,
                                const std::vector<Restriccion>& restricciones) {
    // Promedio Ponderado Total
    double total = 0.0;
    for (const auto& eval : evaluaciones) {
        total += escenario.at(eval.id) * eval.peso;
    }
    if (total < ctx.nota_aprobacion) return false;

    // Restricciones por Tag
    for (const auto& res : restricciones) {
        if (!evaluar_restriccion(res, escenario, evaluaciones)) return false;
    }
    return true;
}

bool MaquinaS::evaluar_restriccion(const Restriccion& res,
                                  const std::map<std::string, double>& escenario,
                                  const std::vector<Evaluacion>& evaluaciones) {
    std::vector<double> notas;
    std::vector<double> pesos;

    for (const auto& eval : evaluaciones) {
        if (std::find(eval.tags.begin(), eval.tags.end(), res.tag_objetivo) != eval.tags.end()) {
            notas.push_back(escenario.at(eval.id));
            pesos.push_back(eval.peso);
        }
    }

    if (notas.empty()) return true;

    if (res.tipo == TipoRestriccion::PROMEDIO_SIMPLE_TAG) {
        double suma = 0;
        for (double n : notas) suma += n;
        return (suma / notas.size()) >= res.valor_minimo;
    }
    else if (res.tipo == TipoRestriccion::NOTA_MINIMA_INDIVIDUAL_TAG) {
        for (double n : notas) if (n < res.valor_minimo) return false;
        return true;
    }
    return true;
}

bool MaquinaS::caso_extremo_optimista(const std::vector<Evaluacion>& evaluaciones,
                                     const std::vector<Restriccion>& restricciones) {
    std::map<std::string, double> escenario;
    for (const auto& eval : evaluaciones) {
        escenario[eval.id] = eval.valor_actual.value_or(ctx.nota_maxima);
    }
    return validar_escenario(escenario, evaluaciones, restricciones);
}

RangoFactible MaquinaS::buscar_limites(const std::string& id,
                                     const std::vector<Evaluacion>& evaluaciones,
                                     const std::vector<Restriccion>& restricciones) {
    // 1. Mínimo Supervivencia (Relleno con MAX)
    double bot = ctx.nota_minima, top = ctx.nota_maxima, min_surv = ctx.nota_maxima;
    for (int i = 0; i < 15; i++) {
        double mid = bot + (top - bot) / 2.0;
        if (puede_pasar(id, mid, evaluaciones, restricciones, ctx.nota_maxima)) {
            min_surv = mid; top = mid;
        } else {
            bot = mid;
        }
    }

    // 2. Mínimo Seguridad (Relleno con APROBACION)
    bot = ctx.nota_minima; top = ctx.nota_maxima;
    double min_sec = ctx.nota_maxima;
    for (int i = 0; i < 15; i++) {
        double mid = bot + (top - bot) / 2.0;
        if (puede_pasar(id, mid, evaluaciones, restricciones, ctx.nota_aprobacion)) {
            min_sec = mid; top = mid;
        } else {
            bot = mid;
        }
    }

    return { min_surv, min_sec, ctx.nota_maxima };
}

bool MaquinaS::puede_pasar(const std::string& id, double val,
                          const std::vector<Evaluacion>& evaluaciones,
                          const std::vector<Restriccion>& restricciones,
                          double fill_value) {
    std::map<std::string, double> escenario;
    for (const auto& eval : evaluaciones) {
        escenario[eval.id] = (eval.id == id) ? val : eval.valor_actual.value_or(fill_value);
    }
    return validar_escenario(escenario, evaluaciones, restricciones);
}

std::vector<std::string> MaquinaS::identificar_criticas(const std::vector<Evaluacion>& evaluaciones,
                                                     const std::vector<Restriccion>& restricciones) {
    std::vector<std::string> criticas;
    std::map<std::string, double> opt;
    for (const auto& e : evaluaciones) opt[e.id] = e.valor_actual.value_or(ctx.nota_maxima);

    double total = 0.0;
    for (const auto& e : evaluaciones) total += opt[e.id] * e.peso;
    if (total < ctx.nota_aprobacion) criticas.push_back("GLOBAL_PASS_LIMIT");

    for (const auto& res : restricciones) {
        if (!evaluar_restriccion(res, opt, evaluaciones)) criticas.push_back(res.id);
    }
    return criticas;
}
