#include "interface_p.hpp"
#include <algorithm>
#include <random>

MaquinaP::MaquinaP(const Contexto &contexto) : ctx(contexto) {}

ReporteProbabilidad
    MaquinaP::analizar(const EspacioSoluciones &espacio, const Sugerencias &plan,
                   const std::vector<Evaluacion> &evaluaciones,
                   const std::vector<Restriccion> &restricciones,
                   const PerfilEstadistico &perfil, int simulaciones) {
    ReporteProbabilidad reporte;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(perfil.media_historica,
                                            perfil.desviacion_estandar);

    int veces_aprueba = 0;              // Cuántas veces aprueba (cualquier manera)
    int veces_logra_plan_y_aprueba = 0; // Cuántas veces logra plan Y aprueba
    int veces_aprueba_con_plan = 0;     // De las que aprobó, cuántas cumplieron plan

    for (int i = 0; i < simulaciones; ++i) {
        std::map<std::string, double> escenario;
        bool cumple_plan = true;

        // Generar notas aleatorias según perfil
        for (const auto &eval : evaluaciones) {
            if (eval.valor_actual.has_value()) {
                escenario[eval.id] = *eval.valor_actual;
            } else {
                double nota_simulada = std::clamp(dist(gen), ctx.nota_minima, ctx.nota_maxima);
                escenario[eval.id] = nota_simulada;

                // ¿Esta nota cumple o supera el plan?
                if (plan.notas_objetivo.count(eval.id)) {
                    if (nota_simulada < plan.notas_objetivo.at(eval.id)) {
                        cumple_plan = false;
                    }
                }
            }
        }

        // Validar si aprueba con este escenario
        bool aprueba = validar_escenario(escenario, evaluaciones, restricciones);

        if (aprueba) {
            veces_aprueba++;
            
            // Si aprobó cumpliendo el plan
            if (cumple_plan) {
                veces_aprueba_con_plan++;
            }
        }

        // Si cumplió plan Y aprobó
        if (cumple_plan && aprueba) {
            veces_logra_plan_y_aprueba++;
        }
    }

    // 1. Probabilidad general de aprobar (sin considerar plan)
    reporte.probabilidad_general = static_cast<double>(veces_aprueba) / simulaciones;
    
    // 2. Probabilidad de lograr el plan Y aprobar
    reporte.probabilidad_del_plan = static_cast<double>(veces_logra_plan_y_aprueba) / simulaciones;
    
    // 3. Viabilidad: P(cumplió plan | aprobó)
    if (veces_aprueba > 0) {
        reporte.viabilidad = static_cast<double>(veces_aprueba_con_plan) / veces_aprueba;
    } else {
        reporte.viabilidad = 0.0;
    }

    return reporte;
}

bool MaquinaP::validar_escenario(const std::map<std::string, double>& escenario,
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

bool MaquinaP::evaluar_restriccion(const Restriccion& res,
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

double MaquinaP::calcular_probabilidad_base(
    const std::vector<Evaluacion>& evaluaciones,
    const std::vector<Restriccion>& restricciones,
    const PerfilEstadistico& perfil,
    int simulaciones) {

    std::random_device rd;
    std::mt19937 gen(rd());
    std::normal_distribution<double> dist(perfil.media_historica, perfil.desviacion_estandar);

    int exitos = 0;

    for (int i = 0; i < simulaciones; ++i) {
        std::map<std::string, double> escenario;

        for (const auto& eval : evaluaciones) {
            if (eval.valor_actual.has_value()) {
                escenario[eval.id] = *eval.valor_actual;
            } else {
                // Generar nota según perfil y clampear a la escala
                double nota_simulada = std::clamp(dist(gen), ctx.nota_minima, ctx.nota_maxima);
                escenario[eval.id] = nota_simulada;
            }
        }

        // ¿Pasaría el ramo con este escenario aleatorio?
        if (validar_escenario(escenario, evaluaciones, restricciones)) {
            exitos++;
        }
    }

    return static_cast<double>(exitos) / simulaciones;
}
