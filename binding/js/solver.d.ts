/** Tipos de restricción soportados por el motor. */
export type RestriccionTipo =
  | "PROMEDIO_SIMPLE_TAG"
  | "NOTA_MINIMA_INDIVIDUAL_TAG";

/** Contexto general del sistema de calificaciones. */
export interface Contexto {
  /** Nota mínima posible en el sistema. */
  nota_minima: number;
  /** Nota máxima posible en el sistema. */
  nota_maxima: number;
  /** Nota mínima para aprobar. */
  nota_aprobacion: number;
}

/** Evaluación individual dentro del curso. */
export interface Evaluacion {
  /** Identificador único de la evaluación. */
  id: string;
  /** Peso relativo en la nota final (0..1). */
  peso: number;
  /** Nota ya obtenida, o null si está pendiente. */
  valor_actual: number | null;
  /** Etiquetas para agrupar evaluaciones. */
  tags: string[];
}

/** Regla que debe cumplirse sobre un subconjunto de evaluaciones. */
export interface Restriccion {
  /** Identificador de la restricción. */
  id: string;
  /** Tipo de restricción a aplicar. */
  tipo: RestriccionTipo;
  /** Tag objetivo sobre el que aplica la restricción. */
  tag_objetivo: string;
  /** Valor mínimo requerido para cumplir la restricción. */
  valor_minimo: number;
}

/** Entrada para la Máquina S (espacio de soluciones). */
export interface EntradaS {
  /** Evaluaciones definidas en el curso. */
  evaluaciones: Evaluacion[];
  /** Restricciones a cumplir. */
  restricciones: Restriccion[];
}

/** Parámetros de simulación para la Máquina P. */
export interface EntradaP {
  /** Número de simulaciones Monte Carlo. */
  simulaciones: number;
  /** Media histórica de notas. */
  media_historica: number;
  /** Desviación estándar histórica de notas. */
  desviacion_estandar: number;
}

/** Entrada completa del solver. */
export interface EntradaCompleta {
  /** Contexto de calificaciones. */
  contexto: Contexto;
  /** Definición de evaluaciones y restricciones. */
  S: EntradaS;
  /** Parámetros probabilísticos. */
  P: EntradaP;
}

/** Rango de factibilidad para una evaluación. */
export interface RangoEvaluacion {
  /** Mínimo absoluto para no quedar descartado. */
  min_supervivencia: number;
  /** Mínimo recomendado para cumplir restricciones. */
  min_seguridad: number;
  /** Máximo posible dadas las condiciones actuales. */
  max_posible: number;
}

/** Salida de la Máquina S (factibilidad). */
export interface MaquinaSOutput {
  /** Indica si es posible aprobar con las restricciones actuales. */
  es_posible: boolean;
  /** Rangos por evaluación, indexado por id. */
  rangos_por_evaluacion: Record<string, RangoEvaluacion>;
  /** Restricciones que ya no pueden cumplirse. */
  restricciones_incumplibles: string[];
}

/** Estrategias determinísticas de planificación. */
export type Estrategia =
  | "MINIMUM"
  | "BALANCED"
  | "MAX_WEIGHT_FIRST"
  | "MIN_WEIGHT_FIRST";

/** Plan de notas objetivo para una estrategia. */
export interface PlanEstrategia {
  /** Estrategia aplicada para generar el plan. */
  estrategia_aplicada: Estrategia;
  /** Notas objetivo por evaluación. */
  notas_objetivo: Record<string, number>;
  /** Promedio final teórico si se ejecuta el plan. */
  promedio_final_teorico: number;
}

/** Reporte probabilístico para una estrategia. */
export interface ReporteProbabilidad {
  /** Probabilidad de lograr exactamente el plan. */
  probabilidad_del_plan: number;
  /** Probabilidad general de aprobar. */
  probabilidad_general: number;
  /** Viabilidad del plan (del_plan / general). */
  viabilidad: number;
  /** Detalle por evaluación si está disponible. */
  detalle_por_evaluacion?: Record<
    string,
    {
      /** Probabilidad de cumplir la nota objetivo de la evaluación. */
      probabilidad: number;
      /** Nota objetivo asociada a la evaluación. */
      nota_objetivo: number;
    }
  >;
}

/** Perfil estadístico utilizado en las simulaciones. */
export interface PerfilEstadistico {
  /** Media histórica de calificaciones. */
  media_historica: number;
  /** Desviación estándar histórica de calificaciones. */
  desviacion_estandar: number;
}

/** Salida completa del solver cuando el análisis es exitoso. */
export interface SalidaCompleta {
  /** Contexto de calificaciones. */
  contexto: Contexto;
  /** Evaluaciones procesadas. */
  evaluaciones: Evaluacion[];
  /** Restricciones procesadas. */
  restricciones: Restriccion[];
  /** Resultado de factibilidad (Máquina S). */
  maquina_s: MaquinaSOutput;
  /** Planes generados (Máquina D). */
  maquina_d: Record<Estrategia, PlanEstrategia>;
  /** Reportes probabilísticos (Máquina P). */
  maquina_p: Record<Estrategia, ReporteProbabilidad>;
  /** Perfil estadístico usado en simulaciones. */
  perfil_usado: PerfilEstadistico;
}

/** Salida de error en caso de fallo del solver. */
export interface SalidaError {
  /** Indicador de error. */
  status: "error";
  /** Mensaje explicativo del error. */
  message: string;
}

/** Resultado del solver: éxito o error. */
export type Salida = SalidaCompleta | SalidaError;

/**
 * Ejecuta el solver y devuelve el resultado parseado.
 * @param input JSON de entrada como objeto o string.
 */
export function solve(input: EntradaCompleta | string): Promise<Salida>;

/** Módulo Emscripten con la función expuesta para resolver. */
export interface SolverModule {
  /**
   * Llama a la función nativa `solve_process` exportada por WASM.
   * @param ident Debe ser "solve_process".
   * @param returnType Tipo de retorno ("string").
   * @param argTypes Tipos de argumentos (["string"]).
   * @param args Argumentos (JSON de entrada como string).
   */
  ccall(
    ident: "solve_process",
    returnType: "string",
    argTypes: ["string"],
    args: [string]
  ): string;
}

/** Factory asíncrono generado por Emscripten (MODULARIZE=1). */
export interface SolverModuleFactory {
  /** Inicializa y devuelve el módulo WASM. */
  (options?: Record<string, unknown>): Promise<SolverModule>;
}

/** Factory por defecto del binding WASM. */
declare const createSolverModule: SolverModuleFactory;
export { createSolverModule };

/** Export principal del paquete: función de alto nivel. */
export default solve;