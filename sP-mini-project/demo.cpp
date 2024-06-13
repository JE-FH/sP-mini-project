#include <set>
#include <algorithm>
#include "library/stosim.hpp"
#include <plplot/plplot.h>
#include <plplot/plstream.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "samples.hpp"

/* Requirement 6: The simulation is visualized using a plotting library called plplot */
void visualize(const stosim::Vessel& vessel, double duration, const std::vector<stosim::agent_token_t>& agents) {
	std::vector<std::vector<PLFLT>> ys;
	std::vector<PLFLT> xs;
	PLFLT xmin = 0;
	PLFLT xmax = duration;
	PLFLT ymin = 0;
	PLFLT ymax = 0;

	for (auto i = 0; i < agents.size(); i++) {
		ys.push_back(std::vector<PLFLT>());
	}

	auto simulation = vessel.simulate()
		| std::views::take_while([=](const auto& state) { return state.time < duration; });

	for (const auto& step : simulation) {
		for (std::size_t i = 0; i < agents.size(); i++) {
			auto agent_count = (PLFLT)step.agent_count[agents[i]];
			ys[i].push_back(agent_count);
			ymin = std::min(agent_count, ymin);
			ymax = std::max(agent_count, ymax);
		}
		xs.push_back(step.time);
	}

	auto pls = std::make_unique<plstream>();

	// Initialize plplot
	pls->sdev("svg");
	std::string file_name = vessel.get_name() + ".svg";
	pls->sfnam(file_name.c_str());
	pls->init();


	// Create a labelled box to hold the plot.
	pls->env(xmin, xmax, ymin, ymax, 0, 0);
	pls->lab("Time", "Agents", vessel.get_name().c_str());

	PLINT i = 1;
	// Plot the data that was prepared above.
	for (auto y : ys) {
		pls->col0(i++);
		pls->line((PLINT) xs.size(), xs.data(), y.data());
	}
}

/* requirement 7: demonstrating using the generator to sget the max agent count
   withut storing all entire trajectory data*/
stosim::agent_count_t get_max_hospitalizations(int N) {
	auto v = covid19(N);
	auto H_token = v.get_reaction_symbols().lookup_by_value("H");
	return std::ranges::max(
		v.simulate() |
		std::views::take_while([](const auto& state) { return state.time < 100; }) |
		std::views::transform([&](const auto& state) { return state.agent_count[H_token]; })
	);
}

void generate_graphs(std::ostream& results) {
	auto circadian_rhythm_vessel = circadian_rhythm();
	const auto& cr_symbols = circadian_rhythm_vessel.get_reaction_symbols();
	visualize(circadian_rhythm_vessel, 100,
		std::vector<stosim::agent_token_t> {
			cr_symbols.lookup_by_value("C"),
			cr_symbols.lookup_by_value("A"),
			cr_symbols.lookup_by_value("R")
		}
	);
	results << R"(circadian rhythm graph written to "Circadian rhythm.svg")" << '\n';

	auto v = covid19(10000);
	auto covid_symbols = v.get_reaction_symbols();
	visualize(v, 100,
		std::vector<stosim::agent_token_t> {
			covid_symbols.lookup_by_value("S"),
			covid_symbols.lookup_by_value("E"),
			covid_symbols.lookup_by_value("I"),
			covid_symbols.lookup_by_value("H"),
			covid_symbols.lookup_by_value("R")
		}
	);
	results << R"(covid 19 graph written to "Covid 19.svg")" << '\n';

	auto f1 = figure1();
	const auto& f1_symbols = f1.get_reaction_symbols();
	visualize(f1, 2000, std::vector<stosim::agent_count_t> {
		f1_symbols.lookup_by_value("A"),
		f1_symbols.lookup_by_value("B"),
		f1_symbols.lookup_by_value("C")
	});

	results << R"(figure 1 graph written to "Figure 1.svg")" << '\n';

}

void estimate_hospitalizations(std::ostream& results) {
	const auto N_DK = 5822763;
	const auto N_NJ = 589755;
	const auto N_DK_hospitalizations = get_max_hospitalizations(5822763);
	const auto N_NJ_hospitalizations = get_max_hospitalizations(589755);
	
	results << "Max hospitalizations for different populations:\n";
	results << "N_DK (" << N_DK << "): " << N_DK_hospitalizations << "\n";
	results << "N_NJ (" << N_NJ << "): " << N_NJ_hospitalizations << "\n";
}

void print_reactions(std::ostream& results) {
	auto covid_vessel = covid19(10000);
	auto circadian_rhythm_vessel = circadian_rhythm();
	results << "\nCovid 19 reactions:\n";
	results << covid_vessel;
	results << "\nCircadian rhythm reactions:\n";
	results << circadian_rhythm_vessel;

	std::ofstream covid19_dot("covid19.dot");
	std::ofstream circadian_rhythm_dot("circadian rhythm.dot");
	
	covid_vessel.pretty_print_dot(covid19_dot);
	circadian_rhythm_vessel.pretty_print_dot(circadian_rhythm_dot);
	
	results << '\n' << R"(covid 19 reaction rules written to "covid19.dot")" << '\n';
	results << R"(circadian rhythm reaction rules written to "circadian rhythm.dot")" << '\n';
}

/* Requirement 8, demonstrating multithreading to get an 
 * average over multiple simulations
*/
void do_multithreading(std::ostream& results) {
	const auto v = covid19(10000);
	const auto H_token = v.get_reaction_symbols().lookup_by_value("H");
	
	const auto count = 100;

	auto simulation_results = v.multi_simulate(count, [=](coro::generator<const stosim::VesselState&> simulation) -> stosim::agent_count_t {
		return std::ranges::max(simulation |
			std::views::take_while([](const auto& state) { return state.time < 100; }) |
			std::views::transform([&](const auto& state) ->  stosim::agent_count_t { return state.agent_count[H_token]; })
		);
	});

	double total = std::ranges::fold_left(simulation_results, 0,
		[](double acc, stosim::agent_count_t next) { return acc + next; });
	results << "\nMultithreaded testing:\n";
	results << "Peak total: " << total << ", average: " << ((double) total) / count << "\n";
}


// requirement 5: demo the three examples
int main() {
	std::ofstream results("results.txt");

	estimate_hospitalizations(results);
	generate_graphs(results);
	print_reactions(results);
	do_multithreading(results);
	return 0;
}