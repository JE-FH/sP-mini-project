#include "stosim.hpp"
#include <numeric>
#include <random>
#include <algorithm>
namespace stosim {
	AgentSetAndRate AgentSet::operator>>(double rate) {
		return AgentSetAndRate(*this, rate);
	}

	ReactionRule AgentSetAndRate::operator>>=(AgentSet product) {
		return ReactionRule(_agent_set, _rate, std::move(product));
	}

	std::optional<std::tuple<const ReactionRule&, double>> Vessel::get_next_reaction_rule() const
	{
		static std::random_device rd;
		static auto mt = std::mt19937(rd());

		const ReactionRule* current_best = nullptr;
		double lowest_delay = std::numeric_limits<double>::max();
		for (const auto& reaction_rule : _reaction_rules) {
			auto reactant_tokens = reaction_rule.get_reactants().get_agent_tokens();
			
			auto reactant_product = 1;
			for (const auto& token : reactant_tokens) {
				reactant_product = reactant_product * _state[token];
			}

			if (reactant_product > 0) {
				auto distribution = std::exponential_distribution(reactant_product * reaction_rule.get_rate());
				auto delay = distribution(mt);

				if (delay < lowest_delay) {
					current_best = &reaction_rule;
				}
			}
		}
		if (current_best != nullptr) {
			return std::make_optional(std::make_tuple(*current_best, lowest_delay));
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
		
		auto [rule, delay] = rule_and_delay.value();
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
