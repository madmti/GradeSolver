#pragma once
#include "types.hpp"

namespace GradeSolver {
    class Calculator {
    public:
        CalculationResult calculate(const CourseConfig& config);
    };
}
