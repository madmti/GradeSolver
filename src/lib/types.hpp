#pragma once
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>

namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt == std::nullopt) j = nullptr; else j = *opt;
        }
        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) opt = std::nullopt; else opt = j.get<T>();
        }
    };
}

namespace GradeSolver {

    enum class StrategyType { UNIFORM, HEAVIEST_FOCUS };
    NLOHMANN_JSON_SERIALIZE_ENUM(StrategyType, {
        {StrategyType::UNIFORM, "uniform"},
        {StrategyType::HEAVIEST_FOCUS, "heaviest_focus"}
    })

    enum class RuleType {
        GLOBAL_AVERAGE,
        TAG_AVERAGE,
        MIN_GRADE_PER_TAG
    };

    NLOHMANN_JSON_SERIALIZE_ENUM(RuleType, {
        {RuleType::GLOBAL_AVERAGE, "global_average"},
        {RuleType::TAG_AVERAGE, "tag_average"},
        {RuleType::MIN_GRADE_PER_TAG, "min_grade_per_tag"}
    })

    struct Rule {
        RuleType type = RuleType::GLOBAL_AVERAGE;
        double target = 0.0;
        std::optional<std::string> tag_filter = std::nullopt;
        std::string description = "";
    };

    struct Assessment {
        std::string name = "";
        double weight = 0.0;
        std::optional<double> grade = std::nullopt;
        std::vector<std::string> tags = {};
    };

    struct CourseConfig {
        std::string name = "";
        StrategyType strategy = StrategyType::UNIFORM;
        std::vector<Rule> rules = {};
        std::vector<Assessment> assessments = {};
    };

    struct CalculationResult {
        std::string strategy_used;
        std::string status;
        std::string message;
        std::string limiting_rule_description;
        double current_global_score;
        std::map<std::string, double> proposed_grades;
    };

    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Rule, type, target, tag_filter, description)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(Assessment, name, weight, grade, tags)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CourseConfig, name, strategy, rules, assessments)
    NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(CalculationResult, strategy_used, status, message, current_global_score, proposed_grades, limiting_rule_description)
}
