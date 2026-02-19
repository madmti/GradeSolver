#include "../interface_d.hpp"
#include <map>
#include <algorithm>
#include <vector>

void aplicar_estrategia_min_weight_first(
    std::map<std::string, double>& escenario,
    const EspacioSoluciones& espacio,
    const std::vector<Evaluacion>& evaluaciones) {
    
    // MIN_WEIGHT_FIRST: Concentrar esfuerzo en evaluaciones de menor peso
    // Las de menor peso: nota alta (más fáciles de sacar)
    // Las de mayor peso: nota mínima posible

    // Ordenar evaluaciones pendientes por peso (menor a mayor)
    std::vector<const Evaluacion*> pendientes;
    for (const auto& eval : evaluaciones) {
        if (!eval.valor_actual.has_value()) {
            pendientes.push_back(&eval);
        }
    }
    std::sort(pendientes.begin(), pendientes.end(),
              [](const Evaluacion* a, const Evaluacion* b) { return a->peso < b->peso; });

    // Inicializar todas con el mínimo
    for (const auto* eval : pendientes) {
        if (espacio.rangos_por_evaluacion.count(eval->id)) {
            escenario[eval->id] = espacio.rangos_por_evaluacion.at(eval->id).min_supervivencia;
        }
    }

    // Subir solo las primeras (menor peso) lo necesario para alcanzar nota_aprobacion
    // dejando las últimas (mayor peso) en el mínimo
}