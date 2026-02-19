#include "../interface_d.hpp"
#include <map>
#include <algorithm>

void aplicar_estrategia_balanced(
    std::map<std::string, double>& escenario,
    const Contexto& ctx,
    const std::vector<Evaluacion>& evaluaciones) {
    
    // BALANCED: Calcular la nota común mínima necesaria para aprobar
    // considerando el promedio ponderado y las evaluaciones ya realizadas
    
    // Calcular el peso total de evaluaciones pendientes
    double peso_pendiente = 0.0;
    double suma_ponderada_actual = 0.0;
    
    for (const auto& eval : evaluaciones) {
        if (eval.valor_actual.has_value()) {
            // Ya tiene nota, sumar al ponderado actual
            suma_ponderada_actual += eval.valor_actual.value() * eval.peso;
        } else {
            // Pendiente, acumular peso
            peso_pendiente += eval.peso;
        }
    }
    
    // Calcular la nota común necesaria
    // Fórmula: suma_ponderada_actual + (nota_comun * peso_pendiente) = nota_aprobacion
    // Despejando: nota_comun = (nota_aprobacion - suma_ponderada_actual) / peso_pendiente
    
    double nota_comun;
    
    if (peso_pendiente > 0.0) {
        nota_comun = (ctx.nota_aprobacion - suma_ponderada_actual) / peso_pendiente;
        
        // Asegurar que la nota común esté en el rango válido
        nota_comun = std::max(nota_comun, ctx.nota_minima);
        nota_comun = std::min(nota_comun, ctx.nota_maxima);
    } else {
        // No hay evaluaciones pendientes, usar nota de aprobación por defecto
        nota_comun = ctx.nota_aprobacion;
    }
    
    // Aplicar la nota común a todas las evaluaciones pendientes
    for (const auto& eval : evaluaciones) {
        if (!eval.valor_actual.has_value()) {
            escenario[eval.id] = nota_comun;
        }
    }
}