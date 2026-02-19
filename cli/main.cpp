#include "interface_s.hpp"
#include "interface_d.hpp"
#include "json_serializer.hpp"
#include <cstdio>
#include <cmath>
#include <vector>
#include <string>
#include <iostream>

void print_usage() {
    fprintf(stderr, "Uso: solver_cli <archivo.json> [--raw]\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Opciones:\n");
    fprintf(stderr, "  --raw    Imprime el resultado en formato JSON por stdout\n");
    fprintf(stderr, "\n");
    fprintf(stderr, "Sin --raw, imprime el resultado formateado en texto.\n");
}

int main(int argc, char* argv[]) {
    using namespace GradeSolver;
    using namespace GradeSolver::JSON;

    // Verificar argumentos
    if (argc < 2) {
        fprintf(stderr, "Error: Se requiere un archivo JSON de entrada.\n\n");
        print_usage();
        return 1;
    }

    // Verificar si se activó el modo --raw
    bool modo_raw = false;
    if (argc > 2 && std::string(argv[2]) == "--raw") {
        modo_raw = true;
    }

    // Variables de entrada
    Contexto contexto;
    std::vector<Evaluacion> evaluaciones;
    std::vector<Restriccion> restricciones;
    int simulaciones = 10000;
    PerfilEstadistico perfil;
    bool usar_perfil_del_json = false;

    // Cargar desde archivo JSON
    try {
        std::string filepath = argv[1];
        if (!modo_raw) {
            fprintf(stderr, "Cargando configuracion desde: %s\n\n", filepath.c_str());
        }

        auto entrada = parse_entrada_from_file(filepath);

        contexto = entrada.contexto;
        evaluaciones = entrada.evaluaciones;
        restricciones = entrada.restricciones;

        if (entrada.simulaciones.has_value()) {
            simulaciones = entrada.simulaciones.value();
        }

        if (entrada.perfil.has_value()) {
            perfil = entrada.perfil.value();
            usar_perfil_del_json = true;
        }

    } catch (const std::exception& e) {
        fprintf(stderr, "Error al parsear JSON: %s\n", e.what());
        return 1;
    }

    // Crear las máquinas
    MaquinaS maquina_s { contexto };
    MaquinaD maquina_d { contexto };
    MaquinaP maquina_p { contexto };

    // ========== MAQUINA S: Calcular Espacio de Soluciones ==========
    auto espacio = maquina_s.calcular_espacio(evaluaciones, restricciones);

    // Si no es posible, mostrar error y salir
    if (!espacio.es_posible) {
        if (!modo_raw) {
            fprintf(stderr, "========================================\n");
            fprintf(stderr, "ERROR: NO ES POSIBLE APROBAR\n");
            fprintf(stderr, "========================================\n");
            fprintf(stderr, "\nRESTRICCIONES INCUMPLIBLES:\n");
            for (const auto& restriccion : espacio.restricciones_incumplibles) {
                fprintf(stderr, "  - %s\n", restriccion.c_str());
            }
            fprintf(stderr, "\n>> No es posible aprobar incluso con notas maximas en evaluaciones pendientes.\n");
            fprintf(stderr, ">> Sugerencia: Revisar con el profesor opciones de recuperacion.\n\n");
        }

        // En modo raw, igual generar el JSON con la información
        if (modo_raw) {
            SalidaCompleta salida;
            salida.contexto = contexto;
            salida.evaluaciones = evaluaciones;
            salida.restricciones = restricciones;
            salida.espacio_soluciones = espacio;
            
            auto j = to_json(salida);
            std::cout << j.dump() << std::endl;
        }

        return 0;
    }

    // ========== MAQUINA D: Generar Plan de Notas ==========
    auto plan_minimum = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::MINIMUM);
    auto plan_balanced = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::BALANCED);
    auto plan_max_weight = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::MAX_WEIGHT_FIRST);
    auto plan_min_weight = maquina_d.generar_plan(espacio, evaluaciones, restricciones, TipoEstrategia::MIN_WEIGHT_FIRST);

    // ========== MAQUINA P: Analizar Probabilidades ==========

    // Calcular perfil estadístico si no se proporcionó en el JSON
    if (!usar_perfil_del_json) {
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

            // Desviación mínima: 10% del rango de notas (escalable a cualquier sistema)
            double desv_minima = (contexto.nota_maxima - contexto.nota_minima) * 0.10;
            if (perfil.desviacion_estandar < desv_minima) {
                perfil.desviacion_estandar = desv_minima;
            }
        } else {
            perfil.media_historica = contexto.nota_aprobacion + (contexto.nota_maxima - contexto.nota_aprobacion) * 0.2;
            perfil.desviacion_estandar = (contexto.nota_maxima - contexto.nota_minima) / 4.0;
        }
    }

    auto reporte_minimum = maquina_p.analizar(espacio, plan_minimum, evaluaciones, restricciones, perfil, simulaciones);
    auto reporte_balanced = maquina_p.analizar(espacio, plan_balanced, evaluaciones, restricciones, perfil, simulaciones);
    auto reporte_max_weight = maquina_p.analizar(espacio, plan_max_weight, evaluaciones, restricciones, perfil, simulaciones);
    auto reporte_min_weight = maquina_p.analizar(espacio, plan_min_weight, evaluaciones, restricciones, perfil, simulaciones);

    // ========== GENERAR RECOMENDACIÓN ==========
    double mejor_prob = std::max(std::max(reporte_minimum.probabilidad_del_plan, reporte_balanced.probabilidad_del_plan),
                                 std::max(reporte_max_weight.probabilidad_del_plan, reporte_min_weight.probabilidad_del_plan));

    std::string recomendacion;
    std::string mejor_estrategia;
    if (mejor_prob > 0.5) {
        if (reporte_balanced.probabilidad_del_plan == mejor_prob) {
            mejor_estrategia = "BALANCED";
            recomendacion = "Plan BALANCED (" + std::to_string(int(mejor_prob * 100)) + "% de lograr y aprobar) ✓";
        } else if (reporte_max_weight.probabilidad_del_plan == mejor_prob) {
            mejor_estrategia = "MAX_WEIGHT_FIRST";
            recomendacion = "Plan MAX_WEIGHT_FIRST (" + std::to_string(int(mejor_prob * 100)) + "% de lograr y aprobar) ✓";
        } else if (reporte_min_weight.probabilidad_del_plan == mejor_prob) {
            mejor_estrategia = "MIN_WEIGHT_FIRST";
            recomendacion = "Plan MIN_WEIGHT_FIRST (" + std::to_string(int(mejor_prob * 100)) + "% de lograr y aprobar) ✓";
        } else {
            mejor_estrategia = "MINIMUM";
            recomendacion = "Plan MINIMUM (" + std::to_string(int(mejor_prob * 100)) + "% de lograr y aprobar) ✓";
        }
    } else if (mejor_prob > 0.3) {
        mejor_estrategia = "MINIMUM";
        recomendacion = "Plan mas viable tiene " + std::to_string(int(mejor_prob * 100)) + "% probabilidad - Situacion dificil !";
    } else if (reporte_minimum.probabilidad_general > 0.6) {
        mejor_estrategia = "NINGUNO";
        recomendacion = "Mejor SIN PLAN ESPECIFICO (" + std::to_string(int(reporte_minimum.probabilidad_general * 100)) + "% probabilidad base) ~";
    } else {
        mejor_estrategia = "NINGUNO";
        recomendacion = "ALTO RIESGO - Considera mejorar desempeño general ✗";
    }

    // ========== MODO RAW: Imprimir JSON ==========
    if (modo_raw) {
        SalidaCompleta salida;
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

        auto j = to_json(salida);
        std::cout << j.dump() << std::endl;

        return 0;
    }

    // ========== MODO NORMAL: Imprimir Formateado ==========

    printf("========================================\n");
    printf("MAQUINA S - ESPACIO DE SOLUCIONES\n");
    printf("========================================\n");
    printf("POSIBLE: SI\n");

    printf("\n%-20s | %10s | %10s | %10s\n", "EVALUACION", "MIN SUPER", "MIN SEGUR", "MAX");
    printf("------------------------------------------------------------------\n");
    for (const auto& rango : espacio.rangos_por_evaluacion) {
        printf("%-20s | %10.2f | %10.2f | %10.2f\n",
               rango.first.c_str(),
               rango.second.min_supervivencia,
               rango.second.min_seguridad,
               rango.second.max_posible);
    }

    printf("\n========================================\n");
    printf("MAQUINA D - PLAN DE NOTAS\n");
    printf("========================================\n");

    printf("\n%-20s | %10s | %10s | %10s | %10s\n", "EVALUACION", "MINIMUM", "BALANCED", "MAX_W", "MIN_W");
    printf("--------------------------------------------------------------------------------\n");
    for (const auto& objetivo : plan_minimum.notas_objetivo) {
        printf("%-20s | %10.2f | %10.2f | %10.2f | %10.2f\n",
               objetivo.first.c_str(),
               objetivo.second,
               plan_balanced.notas_objetivo.at(objetivo.first),
               plan_max_weight.notas_objetivo.at(objetivo.first),
               plan_min_weight.notas_objetivo.at(objetivo.first));
    }
    printf("--------------------------------------------------------------------------------\n");
    printf("%-20s | %10.2f | %10.2f | %10.2f | %10.2f\n",
           "PROMEDIO TEORICO",
           plan_minimum.promedio_final_teorico,
           plan_balanced.promedio_final_teorico,
           plan_max_weight.promedio_final_teorico,
           plan_min_weight.promedio_final_teorico);

    printf("\n========================================\n");
    printf("MAQUINA P - ANALISIS DE PROBABILIDADES\n");
    printf("========================================\n");
    printf("PERFIL: Media=%.2f, Desv=%.2f\n", perfil.media_historica, perfil.desviacion_estandar);

    printf("\n%-28s | %10s | %10s | %10s | %10s\n", "METRICA", "MINIMUM", "BALANCED", "MAX_W", "MIN_W");
    printf("------------------------------------------------------------------------------------\n");
    printf("%-28s | %9.2f%% | %9.2f%% | %9.2f%% | %9.2f%%\n", "Prob. General (sin plan)",
           reporte_minimum.probabilidad_general * 100,
           reporte_balanced.probabilidad_general * 100,
           reporte_max_weight.probabilidad_general * 100,
           reporte_min_weight.probabilidad_general * 100);
    printf("%-28s | %9.2f%% | %9.2f%% | %9.2f%% | %9.2f%%\n", "Prob. del Plan",
           reporte_minimum.probabilidad_del_plan * 100,
           reporte_balanced.probabilidad_del_plan * 100,
           reporte_max_weight.probabilidad_del_plan * 100,
           reporte_min_weight.probabilidad_del_plan * 100);
    printf("%-28s | %9.2f%% | %9.2f%% | %9.2f%% | %9.2f%%\n", "Viabilidad (plan|aprobo)",
           reporte_minimum.viabilidad * 100,
           reporte_balanced.viabilidad * 100,
           reporte_max_weight.viabilidad * 100,
           reporte_min_weight.viabilidad * 100);

    printf("\n========================================\n");
    printf("RESUMEN FINAL\n");
    printf("========================================\n");
    printf("\n>> RECOMENDACION: %s\n", recomendacion.c_str());
    printf("\n");

    return 0;
}
