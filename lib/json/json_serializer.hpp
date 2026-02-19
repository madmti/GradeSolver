#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <vector>
#include <optional>
#include <map>

// Incluir en el orden correcto: primero shared, luego las máquinas
#include "index.hpp"
#include "interface_s.hpp"
#include "interface_d.hpp"
#include "interface_p.hpp"

namespace GradeSolver {
namespace JSON {

using json = nlohmann::json;

// ============================================================================
// CONVERSIONES DE ENUMS A STRING Y VICEVERSA
// ============================================================================

std::string tipo_restriccion_to_string(TipoRestriccion tipo);
TipoRestriccion string_to_tipo_restriccion(const std::string& str);

std::string tipo_estrategia_to_string(TipoEstrategia tipo);
TipoEstrategia string_to_tipo_estrategia(const std::string& str);

// ============================================================================
// DESERIALIZACIÓN: JSON -> ESTRUCTURAS C++
// ============================================================================

// Estructuras básicas (Shared)
Contexto parse_contexto(const json& j);
Evaluacion parse_evaluacion(const json& j);
Restriccion parse_restriccion(const json& j);
PerfilEstadistico parse_perfil_estadistico(const json& j);

// Estructura de entrada completa
struct EntradaCompleta {
    Contexto contexto;
    std::vector<Evaluacion> evaluaciones;
    std::vector<Restriccion> restricciones;

    // Opcional: configuración para Máquina P
    std::optional<int> simulaciones;
    std::optional<PerfilEstadistico> perfil;
};

EntradaCompleta parse_entrada_completa(const json& j);
EntradaCompleta parse_entrada_from_file(const std::string& filepath);

// ============================================================================
// SERIALIZACIÓN: ESTRUCTURAS C++ -> JSON
// ============================================================================

// Estructuras básicas (Shared)
json to_json(const Contexto& contexto);
json to_json(const Evaluacion& eval);
json to_json(const Restriccion& res);
json to_json(const PerfilEstadistico& perfil);

// Estructuras de salida de cada máquina
json to_json(const RangoFactible& rango);
json to_json(const EspacioSoluciones& espacio);
json to_json(const Sugerencias& sugerencias);
json to_json(const ReporteProbabilidad& reporte);

// Estructura de salida completa
struct SalidaCompleta {
    Contexto contexto;
    std::vector<Evaluacion> evaluaciones;
    std::vector<Restriccion> restricciones;

    // Salida de Máquina S
    EspacioSoluciones espacio_soluciones;

    // Salida de Máquina D (múltiples estrategias)
    std::map<std::string, Sugerencias> planes;

    // Salida de Máquina P (para cada plan)
    std::map<std::string, ReporteProbabilidad> reportes_probabilidad;

    // Opcional: Perfil usado
    std::optional<PerfilEstadistico> perfil_usado;
};

json to_json(const SalidaCompleta& salida);
void save_to_file(const SalidaCompleta& salida, const std::string& filepath);

// Función de conveniencia para serializar solo una sección
json crear_json_entrada(
    const Contexto& contexto,
    const std::vector<Evaluacion>& evaluaciones,
    const std::vector<Restriccion>& restricciones
);

} // namespace JSON
} // namespace GradeSolver
