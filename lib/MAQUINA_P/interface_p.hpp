#include "interface_s.hpp"
#include "interface_d.hpp"

struct ReporteProbabilidad {
    // Probabilidad de aprobar el ramo según tu perfil histórico (sin considerar plan)
    double probabilidad_general;

    // Probabilidad de lograr las notas del plan (o mejores) Y aprobar
    // P(notas >= plan AND aprueba)
    double probabilidad_del_plan;

    // Dado que aprobaste, probabilidad de que haya sido cumpliendo el plan
    // P(cumplió plan | aprobó) - Mide si el plan es necesario o solo ayuda
    double viabilidad;
};

class MaquinaP {
public:
    MaquinaP(const Contexto& contexto);

    // Punto de entrada: Analiza el riesgo y las probabilidades
    ReporteProbabilidad analizar(
        const EspacioSoluciones& espacio,
        const Sugerencias& plan,
        const std::vector<Evaluacion>& evaluaciones,
        const std::vector<Restriccion>& restricciones,
        const PerfilEstadistico& perfil,
        int simulaciones = 50000
    );

    // Calcula probabilidad base sin plan específico (solo con perfil histórico)
    double calcular_probabilidad_base(
        const std::vector<Evaluacion>& evaluaciones,
        const std::vector<Restriccion>& restricciones,
        const PerfilEstadistico& perfil,
        int simulaciones = 50000
    );

private:
    Contexto ctx;

    // Reutilizamos la lógica de validación (podría estar en una clase base o utility)
    bool validar_escenario(const std::map<std::string, double>& escenario,
                          const std::vector<Evaluacion>& evaluaciones,
                          const std::vector<Restriccion>& restricciones);

    bool evaluar_restriccion(const Restriccion& res,
                            const std::map<std::string, double>& escenario,
                            const std::vector<Evaluacion>& evaluaciones);
};
