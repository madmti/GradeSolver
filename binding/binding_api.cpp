#include "interface_s.hpp"
#include "interface_d.hpp"
#include "json_serializer.hpp"
#include <iostream>
#include <string>
#include <cmath>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    const char* solve_process(const char* input_json_raw) {
        static std::string output_buffer;
        output_buffer.clear();

        try {
            if (input_json_raw == nullptr) {
                nlohmann::json err;
                err["status"] = "error";
                err["message"] = "Input JSON is null";
                output_buffer = err.dump();
                return output_buffer.c_str();
            }

            // Parsear entrada JSON
            std::string input_json(input_json_raw);
            auto j_in = nlohmann::json::parse(input_json);

            auto entrada = GradeSolver::JSON::parse_entrada_completa(j_in);

            // Extraer configuración
            Contexto contexto = entrada.contexto;
            std::vector<Evaluacion> evaluaciones = entrada.evaluaciones;
            std::vector<Restriccion> restricciones = entrada.restricciones;
            int simulaciones = entrada.simulaciones.value_or(10000);

            // Crear las máquinas
            MaquinaS maquina_s { contexto };
            MaquinaD maquina_d { contexto };
            MaquinaP maquina_p { contexto };

            // ========== MAQUINA S: Calcular Espacio de Soluciones ==========
            auto espacio = maquina_s.calcular_espacio(evaluaciones, restricciones);

            // Si no es posible, devolver resultado con error
            if (!espacio.es_posible) {
                GradeSolver::JSON::SalidaCompleta salida;
                salida.contexto = contexto;
                salida.evaluaciones = evaluaciones;
                salida.restricciones = restricciones;
                salida.espacio_soluciones = espacio;

                auto j_out = GradeSolver::JSON::to_json(salida);
                output_buffer = j_out.dump();
                return output_buffer.c_str();
            }

            // ========== MAQUINA D: Generar Planes ==========
            auto plan_minimum = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::MINIMUM);
            auto plan_balanced = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::BALANCED);
            auto plan_max_weight = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::MAX_WEIGHT_FIRST);
            auto plan_min_weight = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::MIN_WEIGHT_FIRST);

            // ========== MAQUINA P: Calcular Perfil y Probabilidades ==========
            PerfilEstadistico perfil;

            // Si el JSON tiene perfil, usarlo
            if (entrada.perfil.has_value()) {
                perfil = entrada.perfil.value();
            } else {
                // Calcular perfil basado en notas existentes o valores por defecto
                std::vector<double> notas_existentes;
                for (const auto& eval : evaluaciones) {
                    if (eval.valor_actual.has_value()) {
                        notas_existentes.push_back(eval.valor_actual.value());
                    }
                }

                if (!notas_existentes.empty()) {
                    double suma = 0.0;
                    for (double n : notas_existentes) suma += n;
                    perfil.media_historica = suma / notas_existentes.size();

                    double suma_cuadrados = 0.0;
                    for (double n : notas_existentes) {
                        double diff = n - perfil.media_historica;
                        suma_cuadrados += diff * diff;
                    }
                    perfil.desviacion_estandar = std::sqrt(suma_cuadrados / notas_existentes.size());

                    // Desviación mínima: 10% del rango de notas
                    double desv_minima = (contexto.nota_maxima - contexto.nota_minima) * 0.10;
                    if (perfil.desviacion_estandar < desv_minima) {
                        perfil.desviacion_estandar = desv_minima;
                    }
                } else {
                    perfil.media_historica = contexto.nota_aprobacion + (contexto.nota_maxima - contexto.nota_aprobacion) * 0.2;
                    perfil.desviacion_estandar = (contexto.nota_maxima - contexto.nota_minima) / 4.0;
                }
            }

            // Analizar probabilidades para cada plan
            auto reporte_minimum = maquina_p.analizar(espacio, plan_minimum, evaluaciones, restricciones, perfil, simulaciones);
            auto reporte_balanced = maquina_p.analizar(espacio, plan_balanced, evaluaciones, restricciones, perfil, simulaciones);
            auto reporte_max_weight = maquina_p.analizar(espacio, plan_max_weight, evaluaciones, restricciones, perfil, simulaciones);
            auto reporte_min_weight = maquina_p.analizar(espacio, plan_min_weight, evaluaciones, restricciones, perfil, simulaciones);

            // ========== CONSTRUIR SALIDA ==========
            GradeSolver::JSON::SalidaCompleta salida;
            salida.contexto = contexto;
            salida.evaluaciones = evaluaciones;
            salida.restricciones = restricciones;
            salida.espacio_soluciones = espacio;

            salida.planes["MINIMUM"] = plan_minimum;
            salida.planes["BALANCED"] = plan_balanced;
            salida.planes["MAX_WEIGHT_FIRST"] = plan_max_weight;
            salida.planes["MIN_WEIGHT_FIRST"] = plan_min_weight;

            salida.reportes_probabilidad["MINIMUM"] = reporte_minimum;
            salida.reportes_probabilidad["BALANCED"] = reporte_balanced;
            salida.reportes_probabilidad["MAX_WEIGHT_FIRST"] = reporte_max_weight;
            salida.reportes_probabilidad["MIN_WEIGHT_FIRST"] = reporte_min_weight;

            salida.perfil_usado = perfil;

            // Convertir a JSON y devolver
            auto j_out = GradeSolver::JSON::to_json(salida);
            output_buffer = j_out.dump();

        } catch (const std::exception& e) {
            nlohmann::json err;
            err["status"] = "error";
            err["message"] = e.what();
            output_buffer = err.dump();
            std::cerr << "[Binding Error] " << e.what() << std::endl;
        }

        return output_buffer.c_str();
    }
}
