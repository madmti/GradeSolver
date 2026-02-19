#pragma once
#include <string>
#include <vector>
#include <optional>

// Configuración de la escala de notas
struct Contexto {
    double nota_minima;    // Ej: 0.0 o 1.0
    double nota_maxima;    // Ej: 100.0 o 7.0
    double nota_aprobacion; // Ej: 55.0 o 3.95
};

// Definición de una evaluación
struct Evaluacion {
    std::string id;
    double peso;                         // 0.0 a 1.0
    std::optional<double> valor_actual;  // nullopt si está pendiente
    std::vector<std::string> tags;       // Ej: {"examen", "teoria"}
};

// Tipos de reglas que el Solver (S) debe validar
enum class TipoRestriccion {
    PROMEDIO_SIMPLE_TAG,
    NOTA_MINIMA_INDIVIDUAL_TAG
};

// Definición de una restricción académica
struct Restriccion {
    std::string id;
    TipoRestriccion tipo;
    std::string tag_objetivo;
    double valor_minimo;
};

// Perfil estadístico para la Maquina P
struct PerfilEstadistico {
    double media_historica;
    double desviacion_estandar;
};
