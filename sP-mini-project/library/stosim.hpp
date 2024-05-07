#pragma once
#include <string>
#include <set>
#include "SymbolTable.hpp"
#include <ostream>
#include <optional>

namespace stosim {
	using agent_token_t = size_t;
	
	class AgentSetAndRate;
	class ReactionRule;

	class AgentSet {
		std::set<agent_token_t> _agents;
	public:
		AgentSet(const AgentSet& other) = default;
		AgentSet& operator=(const AgentSet& other) = default;

		AgentSet(AgentSet&& other) = default;
		AgentSet& operator=(AgentSet&& other) = default;

		AgentSet() { }

		AgentSet(agent_token_t agent_token) {
			_agents.insert(agent_token);
		}

		AgentSet operator+(const AgentSet& other) const {
			AgentSet rv;
			rv._agents.insert(std::cbegin(_agents), std::cend(_agents));
			rv._agents.insert(std::cbegin(other._agents), std::cend(other._agents));
			return rv;
		}

		const std::set<agent_token_t>& get_agent_tokens() const {
			return _agents;
		}
		
		AgentSetAndRate operator>>(double rate) const;
	};

	class AgentSetAndRate {
		AgentSet _agent_set;
		double _rate;
	public:
		AgentSetAndRate(AgentSet agent_set, const double rate)
			: _agent_set(std::move(agent_set)), _rate(rate) { }
		AgentSetAndRate(const AgentSetAndRate& other) = default;
		AgentSetAndRate& operator=(const AgentSetAndRate& other) = default;
		AgentSetAndRate(AgentSetAndRate&& other) = default;
		AgentSetAndRate& operator=(AgentSetAndRate&& other) = default;

		const AgentSet& get_agent_set() const {
			return _agent_set;
		}

		const double get_rate() const {
			return _rate;
		}

		ReactionRule operator>>=(AgentSet product) const;
	};

	class ReactionRule {
		AgentSet _reactants;
		double _rate;
		AgentSet _products;
	public:
		ReactionRule(AgentSet reactants, double rate, AgentSet products)
			: _reactants(std::move(reactants)), _rate(rate), _products(std::move(products)) {}
		ReactionRule(const ReactionRule& other) = default;
		ReactionRule& operator=(const ReactionRule& other) = default;
		ReactionRule(ReactionRule&& other) = default;
		ReactionRule& operator=(ReactionRule&& other) = default;

		const AgentSet& get_reactants() const {
			return _reactants;
		}

		const double& get_rate() const {
			return _rate;
		}

		const AgentSet& get_products() const {
			return _products;
		}
	};
	
	class Vessel {
		std::string _name;
		std::vector<ReactionRule> _reaction_rules;
		SymbolTable<agent_token_t, std::string> _reaction_symbols;
		std::vector<int> _state;
		double _current_time;

		std::optional<std::tuple<std::size_t, double>> get_next_reaction_rule() const;
	public:
		Vessel(std::string name) : _name(std::move(name)), _current_time(0.0) {}

		AgentSet add(std::string name, int init) {
			auto id = _state.size();
			_reaction_symbols.store(id, std::move(name));
			_state.push_back(init);
			return AgentSet(id);
		}

		void add(ReactionRule rule) {
			_reaction_rules.push_back(std::move(rule));
		}

		AgentSet environment() {
			return AgentSet();
		}

		std::vector<std::tuple<std::string, int>> translate_state(std::vector<int> state) const;

		const std::vector<int>& get_state() const {
			return _state;
		}

		double get_time() const {
			return _current_time;
		}

		void Step();
	};

	std::ostream& operator<<(std::ostream& out, const Vessel& vessel);
}