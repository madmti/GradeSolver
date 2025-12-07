#pragma once
#include <nlohmann/json.hpp>

namespace GradeSolver {
    class Calculator {
    public:
        nlohmann::json calculate();
    };
}
