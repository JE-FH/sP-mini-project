#include "stosim.hpp"
#include <numeric>
#include <random>
#include <algorithm>
#include <optional>
#include <iterator>
#include <numeric>
#include <ranges>
namespace stosim {
	AgentSetAndRate AgentSet::operator>>(double rate) const {
		return AgentSetAndRate(*this, rate);
	}

	ReactionRule AgentSetAndRate::operator>>=(AgentSet product) const {
		return ReactionRule(_agent_set, _rate, std::move(product));
	}

	/*Requirement 4: here the next reaction rule that will be used is calculated using the given algorithm*/
	std::optional<std::tuple<std::size_t, double>> Vessel::get_next_reaction_rule(const std::vector<agent_count_t>& agent_count, std::mt19937& mt) const
	{
		std::optional<std::size_t> current_best = std::nullopt;
		double lowest_delay = std::numeric_limits<double>::max();
		for (auto i = 0; i < _reaction_rules.size(); i++) {
			const auto& reaction_rule = _reaction_rules[i];
			const auto& reactant_tokens = reaction_rule.get_reactants().get_agent_tokens();
			
			auto reactant_product = (std::size_t) 1;
			for (const auto& token : reactant_tokens) {
				reactant_product = reactant_product * agent_count[token];
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

	AgentSet Vessel::add(std::string name, agent_count_t init) {
		auto id = _initial_state.size();
		_reaction_symbols.store(id, std::move(name));
		_initial_state.push_back(init);
		return AgentSet(id);
	}

	void Vessel::add(ReactionRule rule) {
		_reaction_rules.push_back(std::move(rule));
	}

	AgentSet Vessel::environment() const {
		return AgentSet();
	}

	std::vector<std::tuple<std::string, agent_count_t>> Vessel::translate_state(std::vector<agent_count_t> agent_count) const
	{
		std::vector<std::tuple<std::string, agent_count_t>> rv;
		for (auto i = 0; i < agent_count.size(); i++) {
			auto name = _reaction_symbols.lookup(i);
			rv.push_back(std::make_tuple(name, agent_count[i]));
		}
		return rv;
	}

	/*Requirement 4 here we implement the simulation using the rules.
	* The simulation steps are returned by yielding them.
	* */
	coro::generator<const VesselState&> Vessel::simulate() const
	{
		VesselState state {
			.agent_count = _initial_state,
			.time = 0
		};

		auto rd = std::random_device();
		auto mt = std::mt19937(rd());

		co_yield state;

		while (true) {
			auto rule_and_delay = get_next_reaction_rule(state.agent_count, mt);
			if (!rule_and_delay.has_value()) {
				co_return;
			}

			auto [rule_index, delay] = rule_and_delay.value();

			const auto& rule = _reaction_rules[rule_index];

			state.time += delay;

			for (auto reactant : rule.get_reactants().get_agent_tokens()) {
				state.agent_count[reactant] -= 1;
			}

			for (auto product : rule.get_products().get_agent_tokens()) {
				state.agent_count[product] += 1;
			}

			co_yield state;
		}
	}

	void Vessel::pretty_print_dot(std::ostream& out) const
	{
		out << "digraph {\n";
		out << R"(env[label="Environment", shape="box", style="filled", fillcolor="red"];)" << '\n';
		for (const auto& agent : _reaction_symbols.symbol_table()) {
			out << "s" << agent.first << "[label=\"" << agent.second << R"(", shape="box", style="filled", fillcolor="cyan"];)" << "\n";
		}
		int reaction_id = 0;
		for (const auto& reaction : _reaction_rules) {
			std::string reaction_name = "r" + std::to_string(reaction_id++);
			out << reaction_name << "[label=\"" << reaction.get_rate() << R"(", shape="oval", style="filled", fillcolor="yellow"];)" << "\n";
			for (const auto& reactant : reaction.get_reactants().get_agent_tokens()) {
				out << "s" << reactant << " -> " << reaction_name << ";\n";
			}
			if (reaction.get_products().get_agent_tokens().size() == 0) {
				out << reaction_name << " -> env;\n";
			}
			else {
				for (const auto& product : reaction.get_products().get_agent_tokens()) {
					out << reaction_name << " -> " << "s" << product << ";\n";
				}
			}
		}

		out << "}";
	}

	const std::vector<agent_count_t>& Vessel::get_initial_state() const
	{
		return _initial_state;
	}

	const SymbolTable<agent_token_t, std::string>& Vessel::get_reaction_symbols() const {
		return _reaction_symbols;
	}

	const std::string& Vessel::get_name() const {
		return _name;
	}

	void concat_agents(std::ostream& out, const AgentSet& agent_set, const SymbolTable<agent_token_t, std::string>& symbol_table) {
		if (agent_set.get_agent_tokens().size() == 0) {
			out << "Environment";
			return;
		}
		bool first = true;
		for (auto agent_token : agent_set.get_agent_tokens()) {
			if (!first) {
				out << " + ";
			}
			first = false;
			out << symbol_table.lookup(agent_token);
		}
	}
	
	std::ostream& operator<<(std::ostream& out, const Vessel& vessel) {
		for (const auto& reaction_rule : vessel._reaction_rules) {
			concat_agents(out, reaction_rule.get_reactants(), vessel._reaction_symbols);

			out << " --" << reaction_rule.get_rate() << "> ";

			concat_agents(out, reaction_rule.get_products(), vessel._reaction_symbols);

			out << "\n";
		}
		return out;
	}
}
