#pragma once
#include "types.hpp"

namespace GradeSolver {
    class Calculator {
    public:
        CalculationResult calculate(const CourseConfig& config);
    private:
        std::vector<RuleStatus> calculate_rule_statuses(const CourseConfig& config, const std::map<std::string, double>& proposed_grades);
    };
}
