#include <doctest/doctest.h>
#include <string>
#include <sstream>
#include "library/SymbolTable.hpp"
#include "library/stosim.hpp"

TEST_CASE("symbol table")
{
	SUBCASE("Normal store and lookup") {
		auto st = stosim::SymbolTable<std::string, int>();
		st.store("ab", 1);
		st.store("ba", 2);
		st.store("c", 3);
		CHECK(st.lookup("ab") == 1);
		CHECK(st.lookup("ba") == 2);
		CHECK(st.lookup("c") == 3);
	}

	SUBCASE("Duplicate store key") {
		auto st = stosim::SymbolTable<std::string, int>();
		st.store("ab", 1);
		CHECK_THROWS_AS(st.store("ab", 1), stosim::SymbolAlreadyExistsException);
	}

	SUBCASE("Unknown symbol lookup") {
		auto st = stosim::SymbolTable<std::string, int>();
		CHECK_THROWS_AS(st.lookup("ab"), stosim::SymbolDoesNotExistException);
	}
}

TEST_CASE("AgentSet") {
	SUBCASE("Create agent") {
		auto vessel = stosim::Vessel("vessel test");
		auto a = vessel.add("a", 1);
		auto agent_tokens = a.get_agent_tokens();
		CHECK(agent_tokens.size() == 1);
	}

	auto vessel = stosim::Vessel("vessel test");
	auto a = vessel.add("a", 1);
	auto b = vessel.add("b", 1);
	auto c = vessel.add("c", 1);
	auto a_token = *std::cbegin(a.get_agent_tokens());
	auto b_token = *std::cbegin(b.get_agent_tokens());
	auto c_token = *std::cbegin(c.get_agent_tokens());

	SUBCASE("Agents have unique tokens") {
		CHECK(a_token != b_token);
		CHECK(b_token != c_token);
		CHECK(a_token != c_token);
	}

	SUBCASE("Agents can be combined") {
		auto vessel = stosim::Vessel("vessel test");

		auto combined = a + b;
		auto combined_again = combined + c;

		auto combined_tokens = combined.get_agent_tokens();
		auto combined_again_tokens = combined_again.get_agent_tokens();

		CHECK(combined_tokens.contains(a_token));
		CHECK(combined_tokens.contains(b_token));
		CHECK(combined_tokens.size() == 2);

		CHECK(combined_again_tokens.contains(a_token));
		CHECK(combined_again_tokens.contains(b_token));
		CHECK(combined_again_tokens.contains(c_token));
		CHECK(combined_again_tokens.size() == 3);
	}
}

TEST_CASE("AgentSetAndRate") {
	SUBCASE("AgentSet and rate can be combined") {
		auto vessel = stosim::Vessel("vessel test");
		auto a = vessel.add("a", 1);
		auto b = vessel.add("b", 1);

		auto a_token = *std::cbegin(a.get_agent_tokens());
		auto b_token = *std::cbegin(b.get_agent_tokens());

		auto set_and_rate = (a + b) >> 1.0;

		auto set_and_rate_tokens = set_and_rate.get_agent_set().get_agent_tokens();

		CHECK(set_and_rate.get_rate() == 1.0);

		CHECK(set_and_rate_tokens.size() == 2);
		CHECK(set_and_rate_tokens.contains(a_token));
		CHECK(set_and_rate_tokens.contains(b_token));
	}
}

TEST_CASE("ReactionRule") {
	SUBCASE("ReactionRule can be created") {
		auto vessel = stosim::Vessel("vessel test");
		auto a = vessel.add("a", 1);
		auto b = vessel.add("b", 1);
		auto c = vessel.add("c", 1);

		auto a_token = *std::cbegin(a.get_agent_tokens());
		auto b_token = *std::cbegin(b.get_agent_tokens());
		auto c_token = *std::cbegin(c.get_agent_tokens());

		auto rule = (a + b) >> 1.2 >>= c + a;
		auto reactants_tokens = rule.get_reactants().get_agent_tokens();
		auto product_tokens = rule.get_products().get_agent_tokens();

		CHECK(rule.get_rate() == 1.2);

		CHECK(reactants_tokens.contains(a_token));
		CHECK(reactants_tokens.contains(b_token));
		CHECK(reactants_tokens.size() == 2);

		CHECK(product_tokens.contains(a_token));
		CHECK(product_tokens.contains(c_token));
		CHECK(product_tokens.size() == 2);
	}
}

TEST_CASE("Vessel") {
	SUBCASE("Vessel should be printable") {
		auto vessel = stosim::Vessel("vessel test");
		auto a = vessel.add("a", 1);
		auto b = vessel.add("b", 2);
		auto c = vessel.add("c", 3);
		std::stringstream ss;
		ss << vessel;

		CHECK(ss.str() == "vessel<a=1, b=2, c=3>");
	}
}