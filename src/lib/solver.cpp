#include "solver.hpp"
#include <algorithm>
#include <sstream>

namespace GradeSolver {

    bool has_tag(const Assessment& a, const std::string& tag) {
        return std::find(a.tags.begin(), a.tags.end(), tag) != a.tags.end();
    }

    std::string check_past_failures(const CourseConfig& config, CalculationResult& res) {
        for (const auto& rule : config.rules) {
            if (rule.type == RuleType::MIN_GRADE_PER_TAG) {
                for (const auto& item : config.assessments) {
                    if (item.grade.has_value()) {
                        bool applies = false;
                        if (rule.tag_filter.has_value() && has_tag(item, rule.tag_filter.value())) applies = true;

                        if (applies && item.grade.value() < (rule.target - 0.001)) {
                            std::stringstream ss;
                            ss << "Regla Rota: " << rule.description << ". Tienes un " << item.grade.value()
                               << " en '" << item.name << "' (Mínimo: " << rule.target << ")";
                            return ss.str();
                        }
                    }
                }
            }
        }
        return "";
    }

    void solve_uniform(const CourseConfig& config, CalculationResult& res) {
        res.strategy_used = "Uniforme Multi-Objetivo";
        std::string worst_case_rule = "Ninguna";
        double max_grade_seen = -1.0;

        for (const auto& item : config.assessments) {
            if (item.grade.has_value()) continue;

            double required_for_this_item = 0.0;
            std::string rule_blame = "Base";

            for (const auto& rule : config.rules) {
                double current_accum = 0;
                double evaluated_w = 0;
                double total_rule_w = 0;
                bool item_in_rule = false;

                if (rule.type == RuleType::MIN_GRADE_PER_TAG) {
                    bool applies = false;
                    if (rule.tag_filter.has_value() && has_tag(item, rule.tag_filter.value())) applies = true;
                    if(applies && rule.target > required_for_this_item) {
                        required_for_this_item = rule.target;
                        rule_blame = rule.description;
                    }
                    continue;
                }

                for(const auto& a : config.assessments) {
                    bool applies = true;
                    if (rule.type == RuleType::TAG_AVERAGE && rule.tag_filter.has_value()) {
                        if (!has_tag(a, rule.tag_filter.value())) applies = false;
                    }
                    if (applies) {
                        total_rule_w += a.weight;
                        if (a.name == item.name) item_in_rule = true;
                        if (a.grade.has_value()) {
                            current_accum += a.grade.value() * a.weight;
                            evaluated_w += a.weight;
                        }
                    }
                }

                if (item_in_rule && total_rule_w > 0) {
                    double remaining_w = total_rule_w - evaluated_w;
                    if (remaining_w > 0.0001) {
                        double needed = ((rule.target * total_rule_w) - current_accum) / remaining_w;
                        if (needed > required_for_this_item) {
                            required_for_this_item = needed;
                            rule_blame = rule.description;
                        }
                    }
                }
            }
            res.proposed_grades[item.name] = std::max(0.0, required_for_this_item);
            if(required_for_this_item > max_grade_seen) {
                max_grade_seen = required_for_this_item;
                worst_case_rule = rule_blame;
            }
        }
        res.limiting_rule_description = worst_case_rule;
    }


    void solve_heaviest_focus(const CourseConfig& config, CalculationResult& res) {
        res.strategy_used = "Enfoque en Mayor Peso (Lazy)";

        for (const auto& item : config.assessments) {
            if (!item.grade.has_value()) {
                double floor = 0.0;
                for (const auto& rule : config.rules) {
                    if (rule.type == RuleType::MIN_GRADE_PER_TAG && rule.tag_filter.has_value()) {
                        if (has_tag(item, rule.tag_filter.value())) {
                            floor = std::max(floor, rule.target);
                        }
                    }
                }
                res.proposed_grades[item.name] = floor;
            }
        }

        const Assessment* mvp = nullptr;
        double max_w = -1.0;
        for (const auto& item : config.assessments) {
            if (!item.grade.has_value() && item.weight > max_w) {
                max_w = item.weight;
                mvp = &item;
            }
        }

        if (!mvp) return;

        std::string limit_rule = "Ninguna";

        std::vector<const Rule*> sorted_rules;
        for(const auto& r : config.rules) sorted_rules.push_back(&r);

        std::sort(sorted_rules.begin(), sorted_rules.end(), [](const Rule* a, const Rule* b){
            return (int)a->type > (int)b->type;
        });

        for (const auto* rule_ptr : sorted_rules) {
            const auto& rule = *rule_ptr;
            if (rule.type == RuleType::MIN_GRADE_PER_TAG) continue;

            double current_pts = 0;
            double total_rule_w = 0;
            std::vector<std::string> participants;
            bool mvp_participates = false;

            for (const auto& item : config.assessments) {
                bool applies = true;
                if (rule.type == RuleType::TAG_AVERAGE && rule.tag_filter.has_value()) {
                    if (!has_tag(item, rule.tag_filter.value())) applies = false;
                }

                if (applies) {
                    total_rule_w += item.weight;
                    double val = item.grade.has_value() ? item.grade.value() : res.proposed_grades[item.name];
                    current_pts += val * item.weight;

                    if (!item.grade.has_value()) {
                        participants.push_back(item.name);
                        if (item.name == mvp->name) mvp_participates = true;
                    }
                }
            }

            if (participants.empty()) continue;

            double target_pts = rule.target * total_rule_w;
            double deficit = target_pts - current_pts;

            if (deficit > 0.001) {
                limit_rule = rule.description;
                if (mvp_participates) {
                    res.proposed_grades[mvp->name] += (deficit / mvp->weight);
                } else {
                    double participants_w = 0;
                    for(const auto& pname : participants) {
                        for(const auto& x : config.assessments) if(x.name == pname) participants_w += x.weight;
                    }

                    for(const auto& pname : participants) {
                         res.proposed_grades[pname] += (deficit / participants_w);
                    }
                }
            }
        }
        res.limiting_rule_description = limit_rule;
    }

    std::vector<RuleStatus> Calculator::calculate_rule_statuses(const CourseConfig& config, const std::map<std::string, double>& proposed_grades) {
        std::vector<RuleStatus> statuses;
        
        for (const auto& rule : config.rules) {
            RuleStatus status;
            status.type = rule.type;
            status.target = rule.target;
            status.tag_filter = rule.tag_filter;
            
            if (rule.type == RuleType::GLOBAL_AVERAGE) {
                // Calcular score actual (ponderado)
                double current_points = 0.0;
                double total_weight = 0.0;
                
                for (const auto& assessment : config.assessments) {
                    total_weight += assessment.weight;
                    if (assessment.grade.has_value()) {
                        current_points += assessment.grade.value() * assessment.weight;
                    }
                }
                
                status.current_score = (total_weight > 0) ? (current_points / total_weight) : 0.0;
                
                // Calcular score proyectado con proposed_grades
                double projected_points = current_points;
                for (const auto& assessment : config.assessments) {
                    if (!assessment.grade.has_value()) {
                        auto it = proposed_grades.find(assessment.name);
                        if (it != proposed_grades.end()) {
                            projected_points += it->second * assessment.weight;
                        }
                    }
                }
                
                double projected_score = (total_weight > 0) ? (projected_points / total_weight) : 0.0;
                
                if (status.current_score >= status.target - 0.001) {
                    status.status = "guaranteed";
                } else if (projected_score < status.target - 0.001) {
                    status.status = "impossible";
                } else {
                    status.status = "possible";
                }
                
            } else if (rule.type == RuleType::TAG_AVERAGE) {
                // Calcular score actual (promedio aritmético)
                double current_sum = 0.0;
                int count_with_grade = 0;
                int total_count = 0;
                
                for (const auto& assessment : config.assessments) {
                    if (rule.tag_filter.has_value() && has_tag(assessment, rule.tag_filter.value())) {
                        total_count++;
                        if (assessment.grade.has_value()) {
                            current_sum += assessment.grade.value();
                            count_with_grade++;
                        }
                    }
                }
                
                status.current_score = (count_with_grade > 0) ? (current_sum / count_with_grade) : 0.0;
                
                // Calcular score proyectado
                double projected_sum = current_sum;
                for (const auto& assessment : config.assessments) {
                    if (rule.tag_filter.has_value() && has_tag(assessment, rule.tag_filter.value()) && !assessment.grade.has_value()) {
                        auto it = proposed_grades.find(assessment.name);
                        if (it != proposed_grades.end()) {
                            projected_sum += it->second;
                        }
                    }
                }
                
                double projected_score = (total_count > 0) ? (projected_sum / total_count) : 0.0;
                
                if (count_with_grade == total_count && status.current_score >= status.target - 0.001) {
                    status.status = "guaranteed";
                } else if (projected_score < status.target - 0.001) {
                    status.status = "impossible";
                } else {
                    status.status = "possible";
                }
                
            } else if (rule.type == RuleType::MIN_GRADE_PER_TAG) {
                // Verificar cada evaluación individual
                bool all_satisfied = true;
                bool any_impossible = false;
                status.current_score = 0.0; // No aplica para este tipo
                
                for (const auto& assessment : config.assessments) {
                    if (rule.tag_filter.has_value() && has_tag(assessment, rule.tag_filter.value())) {
                        if (assessment.grade.has_value()) {
                            if (assessment.grade.value() < status.target - 0.001) {
                                any_impossible = true;
                                break;
                            }
                        } else {
                            all_satisfied = false;
                            auto it = proposed_grades.find(assessment.name);
                            if (it != proposed_grades.end() && it->second > 100.0) {
                                any_impossible = true;
                                break;
                            }
                        }
                    }
                }
                
                if (any_impossible) {
                    status.status = "impossible";
                } else if (all_satisfied) {
                    status.status = "guaranteed";
                } else {
                    status.status = "possible";
                }
            }
            
            statuses.push_back(status);
        }
        
        return statuses;
    }

    CalculationResult Calculator::calculate(const CourseConfig& config) {
        CalculationResult res;

        std::string fail_msg = check_past_failures(config, res);
        if (!fail_msg.empty()) {
            res.status = "impossible";
            res.message = fail_msg;
            for(const auto& a : config.assessments) if(!a.grade.has_value()) res.proposed_grades[a.name] = 0.0;
            res.rule_statuses = calculate_rule_statuses(config, res.proposed_grades);
            return res;
        }

        if (config.strategy == StrategyType::HEAVIEST_FOCUS) {
            solve_heaviest_focus(config, res);
        } else {
            solve_uniform(config, res);
        }

        // Calcular estados de las reglas
        res.rule_statuses = calculate_rule_statuses(config, res.proposed_grades);

        double max_g = 0;
        for(auto const& [name, val] : res.proposed_grades) {
            if(val > max_g) max_g = val;
        }

        if (max_g > 100.0) {
            res.status = "impossible";
            res.message = "Requiere nota > 100 (" + res.limiting_rule_description + ")";
        } else if (max_g <= 0.001) {
            res.status = "guaranteed";
            res.message = "Aprobado con mínimos.";
        } else {
            res.status = "possible";
            res.message = "Limitante principal: " + res.limiting_rule_description;
        }

        return res;
    }
}
