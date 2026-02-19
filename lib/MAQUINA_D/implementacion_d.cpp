#include "interface_d.hpp"
#include <map>
#include <algorithm>

// Declaraciones de las funciones de estrategias
void aplicar_estrategia_minimum(
    std::map<std::string, double>& escenario,
    const EspacioSoluciones& espacio,
    const std::vector<Evaluacion>& evaluaciones);

void aplicar_estrategia_balanced(
    std::map<std::string, double>& escenario,
    const Contexto& ctx,
    const std::vector<Evaluacion>& evaluaciones);

void aplicar_estrategia_max_weight_first(
    std::map<std::string, double>& escenario,
    const EspacioSoluciones& espacio,
    const std::vector<Evaluacion>& evaluaciones);

void aplicar_estrategia_min_weight_first(
    std::map<std::string, double>& escenario,
    const EspacioSoluciones& espacio,
    const std::vector<Evaluacion>& evaluaciones);

MaquinaD::MaquinaD(const Contexto& contexto) : ctx(contexto) {}

Sugerencias MaquinaD::generar_plan(const EspacioSoluciones& espacio,
                                  const std::vector<Evaluacion>& evaluaciones,
                                  const std::vector<Restriccion>& restricciones,
                                  TipoEstrategia estrategia) {
    Sugerencias sug;
    sug.estrategia_aplicada = estrategia;

    // Construir escenario inicial con las notas según estrategia
    std::map<std::string, double> escenario;

    // Inicializar con valores conocidos
    for (const auto& eval : evaluaciones) {
        if (eval.valor_actual.has_value()) {
            escenario[eval.id] = *eval.valor_actual;
        }
    }

    // Aplicar estrategia para evaluaciones pendientes
    if (estrategia == TipoEstrategia::MINIMUM) {
        aplicar_estrategia_minimum(escenario, espacio, evaluaciones);
    }
    else if (estrategia == TipoEstrategia::BALANCED) {
        aplicar_estrategia_balanced(escenario, ctx, evaluaciones);
    }
    else if (estrategia == TipoEstrategia::MAX_WEIGHT_FIRST) {
        aplicar_estrategia_max_weight_first(escenario, espacio, evaluaciones);
    }
    else if (estrategia == TipoEstrategia::MIN_WEIGHT_FIRST) {
        aplicar_estrategia_min_weight_first(escenario, espacio, evaluaciones);
    }

    // Crear lista ordenada según estrategia para ajustes
    std::vector<const Evaluacion*> orden_prioridad;
    for (const auto& eval : evaluaciones) {
        if (!eval.valor_actual.has_value()) {
            orden_prioridad.push_back(&eval);
        }
    }

    // Ordenar según estrategia
    if (estrategia == TipoEstrategia::MAX_WEIGHT_FIRST) {
        std::sort(orden_prioridad.begin(), orden_prioridad.end(),
                 [](const Evaluacion* a, const Evaluacion* b) { return a->peso > b->peso; });
    } else if (estrategia == TipoEstrategia::MIN_WEIGHT_FIRST) {
        std::sort(orden_prioridad.begin(), orden_prioridad.end(),
                 [](const Evaluacion* a, const Evaluacion* b) { return a->peso < b->peso; });
    }

    // Ajustar iterativamente hasta cumplir todas las restricciones
    for (int iter = 0; iter < 1000; ++iter) {
        if (validar_escenario(escenario, evaluaciones, restricciones)) {
            break; // Ya cumple todo
        }

        // 1. Verificar y ajustar promedio global
        double promedio_actual = 0.0;
        for (const auto& eval : evaluaciones) {
            promedio_actual += escenario.at(eval.id) * eval.peso;
        }

        if (promedio_actual < ctx.nota_aprobacion) {
            // Incrementar según estrategia
            if (estrategia == TipoEstrategia::MAX_WEIGHT_FIRST || estrategia == TipoEstrategia::MIN_WEIGHT_FIRST) {
                // Subir en orden de prioridad
                bool ajustado_promedio = false;
                for (const auto* eval : orden_prioridad) {
                    if (escenario[eval->id] < ctx.nota_maxima) {
                        escenario[eval->id] = std::min(escenario[eval->id] + 1.0, ctx.nota_maxima);
                        ajustado_promedio = true;
                        break; // Una a la vez
                    }
                }
                if (!ajustado_promedio) {
                    break; // No se puede ajustar más
                }
            } else {
                // Para BALANCED y MINIMUM, subir todas
                for (const auto& eval : evaluaciones) {
                    if (!eval.valor_actual.has_value()) {
                        escenario[eval.id] = std::min(escenario[eval.id] + 1.0, ctx.nota_maxima);
                    }
                }
            }
            continue; // Volver a verificar promedio
        }

        // 2. Revisar cada restricción por tag que falle
        bool alguna_restriccion_fallo = false;
        for (const auto& res : restricciones) {
            if (!evaluar_restriccion(res, escenario, evaluaciones)) {
                alguna_restriccion_fallo = true;
                // Esta restricción falla, ajustar evaluaciones pendientes con ese tag
                for (const auto& eval : evaluaciones) {
                    if (!eval.valor_actual.has_value() &&
                        std::find(eval.tags.begin(), eval.tags.end(), res.tag_objetivo) != eval.tags.end()) {
                        escenario[eval.id] = std::min(escenario[eval.id] + 3.0, ctx.nota_maxima);
                    }
                }
                break; // Ajustar una restricción a la vez
            }
        }

        // Si no hay restricciones que arreglar y el promedio ya está bien, terminar
        if (!alguna_restriccion_fallo && promedio_actual >= ctx.nota_aprobacion) {
            break;
        }
    }

    // Guardar las sugerencias finales
    for (const auto& eval : evaluaciones) {
        if (!eval.valor_actual.has_value()) {
            sug.notas_objetivo[eval.id] = escenario[eval.id];
        }
    }

    // Calcular promedio ponderado final
    double suma_ponderada = 0.0;
    for (const auto& eval : evaluaciones) {
        suma_ponderada += escenario[eval.id] * eval.peso;
    }

    sug.promedio_final_teorico = suma_ponderada;
    return sug;
}

bool MaquinaD::validar_escenario(const std::map<std::string, double>& escenario,
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

bool MaquinaD::evaluar_restriccion(const Restriccion& res,
                                    const std::map<std::string, double>& escenario,
                                    const std::vector<Evaluacion>& evaluaciones) {
    std::vector<double> notas;

    for (const auto& eval : evaluaciones) {
        if (std::find(eval.tags.begin(), eval.tags.end(), res.tag_objetivo) != eval.tags.end()) {
            notas.push_back(escenario.at(eval.id));
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