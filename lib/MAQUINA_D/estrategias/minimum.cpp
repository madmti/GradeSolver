#include "../interface_d.hpp"
#include <map>

void aplicar_estrategia_minimum(
    std::map<std::string, double>& escenario,
    const EspacioSoluciones& espacio,
    const std::vector<Evaluacion>& evaluaciones) {
    
    // MINIMUM: Usar el mínimo absoluto (min_supervivencia) para cada evaluación
    for (const auto& eval : evaluaciones) {
        if (!eval.valor_actual.has_value() && espacio.rangos_por_evaluacion.count(eval.id)) {
            escenario[eval.id] = espacio.rangos_por_evaluacion.at(eval.id).min_supervivencia;
        }
    }
}