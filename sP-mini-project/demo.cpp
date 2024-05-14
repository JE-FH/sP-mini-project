#include <set>
#include <algorithm>
#include "library/stosim.hpp"
#include <plplot/plplot.h>
#include <plplot/plstream.h>
#include <filesystem>
#include <iostream>
#include <fstream>
#include "samples.hpp"


void visualize(stosim::Vessel& vessel, const std::string& name, double duration) {
	std::vector<std::vector<PLFLT>> ys;
	std::vector<PLFLT> xs;
	PLFLT xmin = 0;
	PLFLT xmax = duration;
	PLFLT ymin = 0;
	PLFLT ymax = 0;

	for (auto i = 0; i < vessel.get_initial_state().size(); i++) {
		ys.push_back(std::vector<PLFLT>());
	}
	
	auto simulation = vessel.simulate()
		| std::views::take_while([=](stosim::VesselStep vesselStep) { return vesselStep.time < duration; });

	for (const auto& step : simulation) {
		for (auto i = 0; i < step.state.size(); i++) {
			ys[i].push_back((PLFLT)step.state[i]);
			ymin = std::min((PLFLT)step.state[i], ymin);
			ymax = std::max((PLFLT)step.state[i], ymax);
		}
		xs.push_back(step.time);
	}

	auto pls = std::make_unique<plstream>();

	// Initialize plplot
	pls->sdev("svg");
	std::string file_name = name + ".svg";
	pls->sfnam(file_name.c_str());
	pls->init();


	// Create a labelled box to hold the plot.
	pls->env(xmin, xmax, ymin, ymax, 0, 0);
	pls->lab("Time", "Agents", name.c_str());

	PLINT i = 1;
	// Plot the data that was prepared above.
	for (auto y : ys) {
		pls->col0(i++);
		pls->line(xs.size(), xs.data(), y.data());
	}
}

int get_max_hospitalizations(int N) {
	auto setup = covid19(N);
	auto H_token = setup.H.get_agent_token();
	return std::ranges::max(
		setup.vessel.simulate() |
		std::views::take_while([](stosim::VesselStep vesselStep) { return vesselStep.time < 100; }) |
		std::views::transform([&](stosim::VesselStep step) { return step.state[H_token]; })
	);
}

void generate_graphs(std::ostream& results) {
	auto circadian_rhythm_vessel = circadian_rhythm();
	visualize(circadian_rhythm_vessel, "Circadian rhythm", 100);
	results << R"(circadian rhythm graph written to "Circadian rhythm.svg")" << '\n';

	auto covid_setup = covid19(10000);
	visualize(covid_setup.vessel, "Covid 19", 100);
	results << R"(covid 19 graph written to "Covid 19.svg")" << '\n';
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
	auto setup = covid19(10000);
	auto circadian_rhythm_vessel = circadian_rhythm();
	results << "\nCovid 19 reactions:\n";
	setup.vessel.pretty_print(results);
	results << "\nCircadian rhythm reactions:\n";
	circadian_rhythm_vessel.pretty_print(results);

	std::ofstream covid19_dot("covid19.dot");
	std::ofstream circadian_rhythm_dot("circadian rhythm.dot");
	
	setup.vessel.pretty_print_dot(covid19_dot);
	circadian_rhythm_vessel.pretty_print_dot(circadian_rhythm_dot);
	
	results << '\n' << R"(covid 19 reaction rules written to "covid19.dot")" << '\n';
	results << R"(circadian rhythm reaction rules written to "circadian rhythm.dot")" << '\n';
}

void do_multithreading(std::ostream& results) {
	const auto setup = covid19(10000);
	const auto H_token = setup.H.get_agent_token();
	
	const auto count = 100;

	auto simulation_results = setup.vessel.multi_simulate(count, [=](coro::generator<stosim::VesselStep> simulation) -> stosim::agent_count_t {
		return std::ranges::max(simulation |
			std::views::take_while([](stosim::VesselStep step) { return step.time < 100; }) |
			std::views::transform([&](stosim::VesselStep step) ->  stosim::agent_count_t { return step.state[H_token]; })
		);
	});

	double total = std::ranges::fold_left(simulation_results, 0.0, 
		[](double acc, stosim::agent_count_t next) { return acc + ((double)next); });
	results << "\nMultithreaded testing:\n";
	results << "Peak total: " << total << ", average: " << total / count << "\n";
}

int main(int argc, char** argv) {
	std::ofstream results("results.txt");

	estimate_hospitalizations(results);
	generate_graphs(results);
	print_reactions(results);
	do_multithreading(results);
	
	return 0;
}