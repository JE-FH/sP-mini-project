#pragma once
#include <string>
#include <set>
#include "SymbolTable.hpp"
#include <ostream>
#include <optional>
#include <future>
#include <coro/coro.hpp>

namespace stosim {
	using agent_token_t = size_t;
	using agent_count_t = size_t;
	
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
			rv._agents.insert_range(_agents);
			rv._agents.insert_range(other._agents);
			return rv;
		}

		const std::set<agent_token_t>& get_agent_tokens() const {
			return _agents;
		}

		const agent_token_t get_agent_token() const {
			if (_agents.size() != 1) {
				throw std::exception("Getting agent token is only supported when the agent set has a single token");
			}
			return *std::cbegin(_agents);
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
	
	struct VesselState {
		std::vector<agent_count_t> agent_count;
		double time;
	};

	class Vessel {
		std::string _name;
		std::vector<ReactionRule> _reaction_rules;
		SymbolTable<agent_token_t, std::string> _reaction_symbols;
		std::vector<agent_count_t> _initial_state;

		std::optional<std::tuple<std::size_t, double>> get_next_reaction_rule(const std::vector<agent_count_t>& agent_count) const;
		void pretty_print(std::ostream& out, const AgentSet& agents) const;

	public:
		Vessel(std::string name) : _name(std::move(name)) {}

		AgentSet add(std::string name, agent_count_t init);
		void add(ReactionRule rule);

		AgentSet environment() const;

		std::vector<std::tuple<std::string, agent_count_t>> translate_state(std::vector<agent_count_t> agent_count) const;
		coro::generator<const VesselState&> simulate() const;

		template<typename F>
		coro::generator<std::invoke_result_t<F, coro::generator<const VesselState&>>> multi_simulate(size_t simulation_count, F f) const {
			std::vector<std::future<std::invoke_result_t<F, coro::generator<const VesselState&>>>> futures;
			for (auto i = 0; i < simulation_count; i++) {
				futures.push_back(std::async(std::launch::async, [&]() {
					return f(simulate());
				}));
			}

			for (auto& future : futures) {
				co_yield future.get();
			}
			
			co_return;
		}

		friend std::ostream& operator<<(std::ostream& out, const Vessel& vessel);

		void pretty_print_dot(std::ostream& out) const;

		const std::vector<agent_count_t>& get_initial_state() const;
	};
}