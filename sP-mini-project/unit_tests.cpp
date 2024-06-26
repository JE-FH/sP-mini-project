#include <doctest/doctest.h>
#include <string>
#include <algorithm>
#include <sstream>
#include "library/SymbolTable.hpp"
#include "library/stosim.hpp"

//Requirement 3: Demonstrating the usage of the symbol table
//Requirement 9: Unit tests for symbol table
TEST_CASE("symbol table")
{
	auto st = stosim::SymbolTable<std::string, int>();
	st.store("ab", 7);
	st.store("ba", 1);
	st.store("aba", 100);
	st.store("bab", 2);
	st.store("abab", 6);
	st.store("baba", 4);
	st.store("ababa", 200);

	SUBCASE("Duplicate store key") {
		CHECK_THROWS_AS(st.store("ab", 1), stosim::SymbolAlreadyExistsException);
	}

	SUBCASE("Unknown symbol lookup") {
		CHECK_THROWS_AS(st.lookup("pa"), stosim::SymbolDoesNotExistException);
	}

	SUBCASE("Lookup by value throws when value does not exist") {
		CHECK_THROWS_AS(st.lookup_by_value(8), stosim::SymbolDoesNotExistException);
	}

	SUBCASE("Normal store and lookup") {
		CHECK(st.lookup("ab") == 7);
		CHECK(st.lookup("ba") == 1);
		CHECK(st.lookup("aba") == 100);
		CHECK(st.lookup("bab") == 2);
		CHECK(st.lookup("abab") == 6);
		CHECK(st.lookup("baba") == 4);
		CHECK(st.lookup("ababa") == 200);
	}

	SUBCASE("Lookup by value") {
		CHECK(st.lookup_by_value(7) == "ab");
		CHECK(st.lookup_by_value(1) == "ba");
		CHECK(st.lookup_by_value(100) == "aba");
		CHECK(st.lookup_by_value(2) == "bab");
		CHECK(st.lookup_by_value(6) == "abab");
		CHECK(st.lookup_by_value(4) == "baba");
		CHECK(st.lookup_by_value(200) == "ababa");
	}

	SUBCASE("Symbol iterator test") {
		auto contents = st.symbol_table() | std::ranges::to<std::vector>();
		//Even though, under the current implementation, we dont want to rely on that implementation detail in this test
		//Therefore we sort the contents such that we can compare it directly with another vector
		std::ranges::sort(contents, std::less<std::string> {}, [](const std::pair<std::string, int>& p) -> const std::string& {return p.first; });
		CHECK(contents == std::vector<std::pair<std::string, int>> {
			std::make_pair("ab", 7),
			std::make_pair("aba", 100),
			std::make_pair("abab", 6),
			std::make_pair("ababa", 200),
			std::make_pair("ba", 1),
			std::make_pair("bab", 2),
			std::make_pair("baba", 4)
		});
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

		auto a_token = a.get_agent_token();
		auto b_token = b.get_agent_token();

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

		auto a_token = a.get_agent_token();
		auto b_token = b.get_agent_token();
		auto c_token = c.get_agent_token();

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
//Requirement 9: Unit tests for pretty printing
TEST_CASE("Vessel") {
	auto v = stosim::Vessel("vessel test");
	auto A = v.add("A", 23);
	auto DA = v.add("DA", 1);
	auto D_A = v.add("D_A", 673);
	auto DR = v.add("DR", 13);
	auto D_R = v.add("D_R", 5);
	auto MA = v.add("MA", 14);
	auto MR = v.add("MR", 52);

	auto gamma = 2.3;
	auto theta = 6.23;
	auto alpha = 0.53;

	v.add((A + DA) >> gamma >>= D_A);
	v.add(D_A >> theta >>= DA + A);
	v.add((A + DR) >> gamma >>= D_R);
	v.add(D_R >> theta >>= DR + A);
	v.add(D_A >> alpha >>= MA + D_A);
	v.add(DA >> alpha >>= MA + DA);
	v.add(D_R >> alpha >>= MR + D_R);


	SUBCASE("Vessel pretty printing should work") {
		std::stringstream prettyPrinted;
		prettyPrinted << v;

		std::string expected = 
R"(A + DA --2.3> D_A
D_A --6.23> A + DA
A + DR --2.3> D_R
D_R --6.23> A + DR
D_A --0.53> D_A + MA
DA --0.53> DA + MA
D_R --0.53> D_R + MR
)";

		CHECK(prettyPrinted.str() == expected);
	}
}