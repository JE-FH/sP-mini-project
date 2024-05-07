#include "stosim.hpp"
#include <numeric>
#include <random>
#include <algorithm>
#include <optional>
namespace stosim {
	AgentSetAndRate AgentSet::operator>>(double rate) const {
		return AgentSetAndRate(*this, rate);
	}

	ReactionRule AgentSetAndRate::operator>>=(AgentSet product) const {
		return ReactionRule(_agent_set, _rate, std::move(product));
	}

	std::optional<std::tuple<std::size_t, double>> Vessel::get_next_reaction_rule() const
	{
		static std::random_device rd;
		static auto mt = std::mt19937(rd());

		std::optional<std::size_t> current_best = std::nullopt;
		double lowest_delay = std::numeric_limits<double>::max();
		for (auto i = 0; i < _reaction_rules.size(); i++) {
			const auto& reaction_rule = _reaction_rules[i];
			auto reactant_tokens = reaction_rule.get_reactants().get_agent_tokens();
			
			auto reactant_product = 1;
			for (const auto& token : reactant_tokens) {
				reactant_product = reactant_product * _state[token];
			}

			if (reactant_product > 0) {
				auto distribution = std::exponential_distribution(reactant_product * reaction_rule.get_rate());
				auto delay = distribution(mt);

				if (delay < lowest_delay) {
					current_best.emplace(i);
					lowest_delay = delay;
				}
			}
		}
		if (current_best != std::nullopt) {
			return std::make_optional(std::make_tuple(current_best.value(), lowest_delay));
		}
		return std::nullopt;
	}

	std::vector<std::tuple<std::string, int>> Vessel::translate_state(std::vector<int> state) const
	{
		std::vector<std::tuple<std::string, int>> rv;
		for (auto i = 0; i < state.size(); i++) {
			auto name = _reaction_symbols.lookup(i);
			rv.push_back(std::make_tuple(name, state[i]));
		}
		return rv;
	}

	void Vessel::Step()
	{
		auto rule_and_delay = get_next_reaction_rule();
		if (!rule_and_delay.has_value()) {
			return;
		}
		
		auto [rule_index, delay] = rule_and_delay.value();

		const auto& rule = _reaction_rules[rule_index];

		_current_time += delay;
		
		for (auto reactant : rule.get_reactants().get_agent_tokens()) {
			_state[reactant] -= 1;
		}
		
		for (auto product : rule.get_products().get_agent_tokens()) {
			_state[product] += 1;
		}
	}

	std::ostream& operator<<(std::ostream& out, const Vessel& vessel) {
		auto translated = vessel.translate_state(vessel.get_state());
		out << "vessel<";
		bool first = true;
		for (const auto& [agent_name, count] : translated) {
			if (!first) {
				out << ", ";
			}
			first = false;
			out << agent_name << "=" << count;
		}
		out << ">";
		return out;
	}
}
