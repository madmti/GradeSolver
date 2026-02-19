#pragma once
#include <map>
#include <string>
#include <vector>
#include "index.hpp"

struct RangoFactible {
    double min_supervivencia; // Mínimo absoluto (relleno optimista con MAX)
    double min_seguridad;     // Mínimo seguro (relleno pesimista con APROBACION)
    double max_posible;       // El techo de la escala (o limitado por reglas)
};

struct EspacioSoluciones {
    bool es_posible;
    std::map<std::string, RangoFactible> rangos_por_evaluacion;
    std::vector<std::string> restricciones_incumplibles; // IDs de las que fallan en el mejor caso
};

class MaquinaS {
public:
    MaquinaS(const Contexto& contexto);

    // Punto de entrada único: Calcula el espacio de soluciones
    EspacioSoluciones calcular_espacio(const std::vector<Evaluacion>& evaluaciones,
                                      const std::vector<Restriccion>& restricciones);

private:
    Contexto ctx;

    bool validar_escenario(const std::map<std::string, double>& escenario,
                          const std::vector<Evaluacion>& evaluaciones,
                          const std::vector<Restriccion>& restricciones);

    bool evaluar_restriccion(const Restriccion& res,
                            const std::map<std::string, double>& escenario,
                            const std::vector<Evaluacion>& evaluaciones);

    bool caso_extremo_optimista(const std::vector<Evaluacion>& evaluaciones,
                               const std::vector<Restriccion>& restricciones);

    std::vector<std::string> identificar_criticas(const std::vector<Evaluacion>& evaluaciones,
                                                 const std::vector<Restriccion>& restricciones);

    RangoFactible buscar_limites(const std::string& id,
                                const std::vector<Evaluacion>& evaluaciones,
                                const std::vector<Restriccion>& restricciones);

    bool puede_pasar(const std::string& id, double val,
                    const std::vector<Evaluacion>& evaluaciones,
                    const std::vector<Restriccion>& restricciones,
                    double fill_value);
};
