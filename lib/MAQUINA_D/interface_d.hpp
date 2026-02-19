#pragma once

#include "interface_s.hpp"

enum class TipoEstrategia {
    MAX_WEIGHT_FIRST,  // Priorizar evaluaciones con mayor peso
    MIN_WEIGHT_FIRST,  // Priorizar evaluaciones con menor peso
    BALANCED,          // Misma nota para todas las evaluaciones
    MINIMUM            // Mínima nota posible para cada evaluación
};

struct Sugerencias {
    // ID de evaluación -> Nota sugerida para el usuario
    std::map<std::string, double> notas_objetivo;
    TipoEstrategia estrategia_aplicada;
    double promedio_final_teorico;
};

class MaquinaD {
public:
    MaquinaD(const Contexto& contexto);

    // Genera el plan de notas basado en el espacio y la estrategia elegida
    Sugerencias generar_plan(const EspacioSoluciones& espacio,
                             const std::vector<Evaluacion>& evaluaciones,
                             const std::vector<Restriccion>& restricciones,
                             TipoEstrategia estrategia);

private:
    Contexto ctx;

    bool validar_escenario(const std::map<std::string, double>& escenario,
                          const std::vector<Evaluacion>& evaluaciones,
                          const std::vector<Restriccion>& restricciones);

    bool evaluar_restriccion(const Restriccion& res,
                            const std::map<std::string, double>& escenario,
                            const std::vector<Evaluacion>& evaluaciones);
};
