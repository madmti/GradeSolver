#include "json_serializer.hpp"
#include <fstream>
#include <stdexcept>

namespace GradeSolver {
namespace JSON {

// ============================================================================
// CONVERSIONES DE ENUMS A STRING Y VICEVERSA
// ============================================================================

std::string tipo_restriccion_to_string(TipoRestriccion tipo) {
    switch (tipo) {
        case TipoRestriccion::NOTA_MINIMA_INDIVIDUAL_TAG:
            return "NOTA_MINIMA_INDIVIDUAL_TAG";
        case TipoRestriccion::PROMEDIO_SIMPLE_TAG:
            return "PROMEDIO_SIMPLE_TAG";
        default:
            throw std::runtime_error("TipoRestriccion desconocido");
    }
}

TipoRestriccion string_to_tipo_restriccion(const std::string& str) {
    if (str == "NOTA_MINIMA_INDIVIDUAL_TAG") {
        return TipoRestriccion::NOTA_MINIMA_INDIVIDUAL_TAG;
    } else if (str == "PROMEDIO_SIMPLE_TAG") {
        return TipoRestriccion::PROMEDIO_SIMPLE_TAG;
    }
    throw std::runtime_error("Tipo de restriccion desconocido: " + str);
}

std::string tipo_estrategia_to_string(TipoEstrategia tipo) {
    switch (tipo) {
        case TipoEstrategia::MINIMUM:
            return "MINIMUM";
        case TipoEstrategia::BALANCED:
            return "BALANCED";
        case TipoEstrategia::MAX_WEIGHT_FIRST:
            return "MAX_WEIGHT_FIRST";
        case TipoEstrategia::MIN_WEIGHT_FIRST:
            return "MIN_WEIGHT_FIRST";
        default:
            throw std::runtime_error("TipoEstrategia desconocido");
    }
}

TipoEstrategia string_to_tipo_estrategia(const std::string& str) {
    if (str == "MINIMUM") {
        return TipoEstrategia::MINIMUM;
    } else if (str == "BALANCED") {
        return TipoEstrategia::BALANCED;
    } else if (str == "MAX_WEIGHT_FIRST") {
        return TipoEstrategia::MAX_WEIGHT_FIRST;
    } else if (str == "MIN_WEIGHT_FIRST") {
        return TipoEstrategia::MIN_WEIGHT_FIRST;
    }
    throw std::runtime_error("Tipo de estrategia desconocido: " + str);
}

// ============================================================================
// DESERIALIZACIÓN: JSON -> ESTRUCTURAS C++
// ============================================================================

Contexto parse_contexto(const json& j) {
    Contexto ctx;
    ctx.nota_minima = j["nota_minima"];
    ctx.nota_maxima = j["nota_maxima"];
    ctx.nota_aprobacion = j["nota_aprobacion"];
    return ctx;
}

Evaluacion parse_evaluacion(const json& j) {
    Evaluacion eval;
    eval.id = j["id"];
    eval.peso = j["peso"];

    if (!j["valor_actual"].is_null()) {
        eval.valor_actual = j["valor_actual"];
    }

    for (const auto& tag : j["tags"]) {
        eval.tags.push_back(tag);
    }

    return eval;
}

Restriccion parse_restriccion(const json& j) {
    Restriccion res;
    res.id = j["id"];
    res.tipo = string_to_tipo_restriccion(j["tipo"]);
    res.tag_objetivo = j["tag_objetivo"];
    res.valor_minimo = j["valor_minimo"];
    return res;
}

PerfilEstadistico parse_perfil_estadistico(const json& j) {
    PerfilEstadistico perfil;
    perfil.media_historica = j["media_historica"];
    perfil.desviacion_estandar = j["desviacion_estandar"];
    return perfil;
}

EntradaCompleta parse_entrada_completa(const json& j) {
    EntradaCompleta entrada;

    // Parsear contexto
    entrada.contexto = parse_contexto(j["contexto"]);

    // Parsear evaluaciones
    if (j.contains("evaluaciones")) {
        for (const auto& eval_json : j["evaluaciones"]) {
            entrada.evaluaciones.push_back(parse_evaluacion(eval_json));
        }
    } else if (j.contains("S") && j["S"].contains("evaluaciones")) {
        for (const auto& eval_json : j["S"]["evaluaciones"]) {
            entrada.evaluaciones.push_back(parse_evaluacion(eval_json));
        }
    }

    // Parsear restricciones
    if (j.contains("restricciones")) {
        for (const auto& res_json : j["restricciones"]) {
            entrada.restricciones.push_back(parse_restriccion(res_json));
        }
    } else if (j.contains("S") && j["S"].contains("restricciones")) {
        for (const auto& res_json : j["S"]["restricciones"]) {
            entrada.restricciones.push_back(parse_restriccion(res_json));
        }
    }

    // Parsear configuración opcional de Máquina P
    if (j.contains("P")) {
        if (j["P"].contains("simulaciones")) {
            entrada.simulaciones = j["P"]["simulaciones"];
        }
        if (j["P"].contains("media_historica") && j["P"].contains("desviacion_estandar")) {
            PerfilEstadistico perfil;
            perfil.media_historica = j["P"]["media_historica"];
            perfil.desviacion_estandar = j["P"]["desviacion_estandar"];
            entrada.perfil = perfil;
        }
    }

    return entrada;
}

EntradaCompleta parse_entrada_from_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo abrir el archivo: " + filepath);
    }

    json j;
    file >> j;

    return parse_entrada_completa(j);
}

// ============================================================================
// SERIALIZACIÓN: ESTRUCTURAS C++ -> JSON
// ============================================================================

json to_json(const Contexto& contexto) {
    return json{
        {"nota_minima", contexto.nota_minima},
        {"nota_maxima", contexto.nota_maxima},
        {"nota_aprobacion", contexto.nota_aprobacion}
    };
}

json to_json(const Evaluacion& eval) {
    json j;
    j["id"] = eval.id;
    j["peso"] = eval.peso;

    if (eval.valor_actual.has_value()) {
        j["valor_actual"] = eval.valor_actual.value();
    } else {
        j["valor_actual"] = nullptr;
    }

    j["tags"] = eval.tags;

    return j;
}

json to_json(const Restriccion& res) {
    return json{
        {"id", res.id},
        {"tipo", tipo_restriccion_to_string(res.tipo)},
        {"tag_objetivo", res.tag_objetivo},
        {"valor_minimo", res.valor_minimo}
    };
}

json to_json(const PerfilEstadistico& perfil) {
    return json{
        {"media_historica", perfil.media_historica},
        {"desviacion_estandar", perfil.desviacion_estandar}
    };
}

json to_json(const RangoFactible& rango) {
    return json{
        {"min_supervivencia", rango.min_supervivencia},
        {"min_seguridad", rango.min_seguridad},
        {"max_posible", rango.max_posible}
    };
}

json to_json(const EspacioSoluciones& espacio) {
    json j;
    j["es_posible"] = espacio.es_posible;

    // Convertir rangos por evaluación
    json rangos_json = json::object();
    for (const auto& [id, rango] : espacio.rangos_por_evaluacion) {
        rangos_json[id] = to_json(rango);
    }
    j["rangos_por_evaluacion"] = rangos_json;

    j["restricciones_incumplibles"] = espacio.restricciones_incumplibles;

    return j;
}

json to_json(const Sugerencias& sugerencias) {
    json j;

    // Convertir notas objetivo
    json notas_json = json::object();
    for (const auto& [id, nota] : sugerencias.notas_objetivo) {
        notas_json[id] = nota;
    }
    j["notas_objetivo"] = notas_json;

    j["estrategia_aplicada"] = tipo_estrategia_to_string(sugerencias.estrategia_aplicada);
    j["promedio_final_teorico"] = sugerencias.promedio_final_teorico;

    return j;
}

json to_json(const ReporteProbabilidad& reporte) {
    return json{
        {"probabilidad_general", reporte.probabilidad_general},
        {"probabilidad_del_plan", reporte.probabilidad_del_plan},
        {"viabilidad", reporte.viabilidad}
    };
}

json to_json(const SalidaCompleta& salida) {
    json j;

    // Contexto
    j["contexto"] = to_json(salida.contexto);

    // Evaluaciones
    json evaluaciones_json = json::array();
    for (const auto& eval : salida.evaluaciones) {
        evaluaciones_json.push_back(to_json(eval));
    }
    j["evaluaciones"] = evaluaciones_json;

    // Restricciones
    json restricciones_json = json::array();
    for (const auto& res : salida.restricciones) {
        restricciones_json.push_back(to_json(res));
    }
    j["restricciones"] = restricciones_json;

    // Máquina S: Espacio de soluciones
    j["maquina_s"] = to_json(salida.espacio_soluciones);

    // Máquina D: Planes (múltiples estrategias)
    json planes_json = json::object();
    for (const auto& [estrategia, plan] : salida.planes) {
        planes_json[estrategia] = to_json(plan);
    }
    j["maquina_d"] = planes_json;

    // Máquina P: Reportes de probabilidad
    json reportes_json = json::object();
    for (const auto& [estrategia, reporte] : salida.reportes_probabilidad) {
        reportes_json[estrategia] = to_json(reporte);
    }
    j["maquina_p"] = reportes_json;

    // Perfil usado (opcional)
    if (salida.perfil_usado.has_value()) {
        j["perfil_usado"] = to_json(salida.perfil_usado.value());
    }

    return j;
}

void save_to_file(const SalidaCompleta& salida, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("No se pudo crear el archivo: " + filepath);
    }

    json j = to_json(salida);
    file << j.dump(2); // Pretty print con indentación de 2 espacios
}

json crear_json_entrada(
    const Contexto& contexto,
    const std::vector<Evaluacion>& evaluaciones,
    const std::vector<Restriccion>& restricciones
) {
    json j;

    j["contexto"] = to_json(contexto);

    json evaluaciones_json = json::array();
    for (const auto& eval : evaluaciones) {
        evaluaciones_json.push_back(to_json(eval));
    }
    j["S"]["evaluaciones"] = evaluaciones_json;

    json restricciones_json = json::array();
    for (const auto& res : restricciones) {
        restricciones_json.push_back(to_json(res));
    }
    j["S"]["restricciones"] = restricciones_json;

    return j;
}

} // namespace JSON
} // namespace GradeSolver
