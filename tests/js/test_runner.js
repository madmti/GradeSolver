#!/usr/bin/env node

const fs = require('fs');
const path = require('path');

// Colores para output
const colors = {
    reset: '\x1b[0m',
    green: '\x1b[32m',
    red: '\x1b[31m',
    yellow: '\x1b[33m',
    blue: '\x1b[34m',
    cyan: '\x1b[36m',
};

function log(color, message) {
    console.log(color + message + colors.reset);
}

async function runTests() {
    log(colors.blue, '\n========================================');
    log(colors.blue, 'GradeSolver WASM Test Runner');
    log(colors.blue, '========================================\n');

    try {
        // Cargar el módulo WASM
        log(colors.cyan, 'Cargando módulo WASM...');
        const binding = require('../../dist/js/solver.js');

        let callSolver;
        if (binding && typeof binding.solve === 'function') {
            callSolver = async (inputJson) => {
                const output = await binding.solve(inputJson);
                return JSON.stringify(output);
            };
        } else {
            const createSolverModule = binding;
            const Module = await createSolverModule();
            callSolver = async (inputJson) => {
                return Module.ccall(
                    'solve_process',
                    'string',
                    ['string'],
                    [inputJson]
                );
            };
        }

        log(colors.green, '✓ Módulo WASM cargado correctamente\n');

        // Leer todos los archivos de test
        const casesDir = path.join(__dirname, '../cases');
        const testFiles = fs.readdirSync(casesDir)
            .filter(file => file.endsWith('.json') && !file.includes('.output'))
            .sort();

        if (testFiles.length === 0) {
            log(colors.yellow, 'No se encontraron archivos de test en tests/cases/');
            process.exit(0);
        }

        log(colors.cyan, `Encontrados ${testFiles.length} archivos de test\n`);

        let passed = 0;
        let failed = 0;

        // Ejecutar cada test
        for (const testFile of testFiles) {
            const testPath = path.join(casesDir, testFile);
            const testName = testFile.replace('.json', '');

            log(colors.yellow, `[${testName}]`);

            try {
                // Leer archivo de entrada
                const inputJson = fs.readFileSync(testPath, 'utf8');
                const inputData = JSON.parse(inputJson);

                log(colors.cyan, `  Evaluaciones: ${inputData.S?.evaluaciones?.length || 0}`);
                log(colors.cyan, `  Restricciones: ${inputData.S?.restricciones?.length || 0}`);

                // Ejecutar el solver
                const startTime = Date.now();
                const outputJson = await callSolver(inputJson);
                const endTime = Date.now();
                const duration = endTime - startTime;

                // Parsear resultado
                const output = JSON.parse(outputJson);

                // Verificar si es un error
                if (output.status === 'error') {
                    log(colors.red, `  ✗ ERROR: ${output.message}`);
                    failed++;
                    continue;
                }

                // Mostrar resultados
                log(colors.cyan, `  Tiempo: ${duration}ms`);

                // Máquina S
                if (output.maquina_s) {
                    log(colors.yellow, `\n  Máquina S - Análisis de Factibilidad:`);
                    const posible = output.maquina_s.es_posible;

                    if (posible) {
                        const rangos = output.maquina_s.rangos_por_evaluacion || {};
                        const numRangos = Object.keys(rangos).length;
                        log(colors.green, `     Estado: POSIBLE`);
                        log(colors.cyan, `     Rangos disponibles: ${numRangos}`);

                        // Mostrar algunos rangos de ejemplo
                        const rangoKeys = Object.keys(rangos).slice(0, 3);
                        rangoKeys.forEach(key => {
                            const rango = rangos[key];
                            const minSupervivencia = rango.min_supervivencia !== undefined ?
                                (typeof rango.min_supervivencia === 'number' ? rango.min_supervivencia.toFixed(2) : rango.min_supervivencia) : 'N/A';
                            const minSeguridad = rango.min_seguridad !== undefined ?
                                (typeof rango.min_seguridad === 'number' ? rango.min_seguridad.toFixed(2) : rango.min_seguridad) : 'N/A';
                            const maxPosible = rango.max_posible !== undefined ?
                                (typeof rango.max_posible === 'number' ? rango.max_posible.toFixed(2) : rango.max_posible) :
                                (rango.max !== undefined ? (typeof rango.max === 'number' ? rango.max.toFixed(2) : rango.max) : 'N/A');
                            log(colors.cyan, `       - ${key}: [${minSupervivencia}, ${minSeguridad}, ${maxPosible}]`);
                        });
                        if (Object.keys(rangos).length > 3) {
                            log(colors.cyan, `       ... y ${Object.keys(rangos).length - 3} más`);
                        }
                    } else {
                        const incumplibles = output.maquina_s.restricciones_incumplibles || [];
                        log(colors.red, `     Estado: IMPOSIBLE`);
                        log(colors.red, `     Restricciones incumplibles: ${incumplibles.length}`);

                        // Mostrar restricciones incumplibles
                        incumplibles.slice(0, 3).forEach(rest => {
                            log(colors.red, `       - ${rest}`);
                        });
                        if (incumplibles.length > 3) {
                            log(colors.red, `       ... y ${incumplibles.length - 3} más`);
                        }
                    }
                }

                // Máquina D
                if (output.maquina_d) {
                    const estrategias = Object.keys(output.maquina_d);
                    log(colors.yellow, `\n  Máquina D - Optimización de Notas:`);
                    log(colors.cyan, `     Estrategias evaluadas: ${estrategias.length}`);

                    // Ordenar por promedio y mostrar
                    const estrategiasOrdenadas = estrategias
                        .map(est => ({
                            nombre: est,
                            promedio: output.maquina_d[est].promedio_final_teorico,
                            notas: output.maquina_d[est].notas_objetivo
                        }))
                        .sort((a, b) => b.promedio - a.promedio);

                    estrategiasOrdenadas.forEach((est, idx) => {
                        const simbolo = idx === 0 ? '[MEJOR]' : '       ';
                        const color = idx === 0 ? colors.green : colors.cyan;
                        log(color, `     ${simbolo} ${est.nombre}: ${est.promedio.toFixed(2)}`);

                        // Mostrar algunas notas objetivo
                        if (est.notas) {
                            const notasKeys = Object.keys(est.notas);
                            notasKeys.forEach(key => {
                                log(color, `         ${key}: ${est.notas[key].toFixed(2)}`);
                            });
                        }
                    });
                }

                // Máquina P
                if (output.maquina_p) {
                    const estrategias = Object.keys(output.maquina_p);
                    log(colors.yellow, `\n  Máquina P - Análisis de Probabilidades:`);

                    // Ordenar por probabilidad
                    const estrategiasOrdenadas = estrategias
                        .map(est => ({
                            nombre: est,
                            prob: output.maquina_p[est].probabilidad_del_plan,
                            detalle: output.maquina_p[est].detalle_por_evaluacion
                        }))
                        .sort((a, b) => b.prob - a.prob);

                    estrategiasOrdenadas.forEach((est, idx) => {
                        const simbolo = idx === 0 ? '[MEJOR]' : '       ';
                        const color = idx === 0 ? colors.green : colors.cyan;
                        const probPct = (est.prob * 100).toFixed(1);
                        log(color, `     ${simbolo} ${est.nombre}: ${probPct}%`);

                        // Mostrar detalle de algunas evaluaciones
                        if (est.detalle) {
                            const detalleKeys = Object.keys(est.detalle).slice(0, 2);
                            detalleKeys.forEach(key => {
                                const det = est.detalle[key];
                                log(color, `         ${key}: ${(det.probabilidad * 100).toFixed(1)}% (nota: ${det.nota_objetivo.toFixed(2)})`);
                            });
                        }
                    });
                }

                log(colors.green, `\n  Test completado`);
                log(colors.cyan, `  ${'='.repeat(50)}\n`);
                passed++;

            } catch (error) {
                log(colors.red, `  FALLO: ${error.message}\n`);
                console.error(error.stack);
                failed++;
            }
        }

        // Resumen
        log(colors.blue, '========================================');
        log(colors.blue, 'Resumen');
        log(colors.blue, '========================================');
        log(colors.green, `Exitosos: ${passed}`);
        if (failed > 0) {
            log(colors.red, `Fallidos:  ${failed}`);
        }
        log(colors.cyan, `Total:      ${passed + failed}\n`);

        process.exit(failed > 0 ? 1 : 0);

    } catch (error) {
        log(colors.red, `\nError fatal: ${error.message}`);
        console.error(error.stack);
        process.exit(1);
    }
}

// Ejecutar tests
runTests().catch(err => {
    console.error('Error inesperado:', err);
    process.exit(1);
});
