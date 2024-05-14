#include <ranges>
#include <algorithm>
#include <benchmark/benchmark.h>
#include "library/stosim.hpp"
#include "samples.hpp"

void single_threaded(benchmark::State& state) {
	auto setup = covid19(10000);
	auto H_token = setup.H.get_agent_token();
	for (auto _ : state) {
		stosim::agent_count_t total = 0;
		for (size_t i = 0; i < 100; i++) {
			total += std::ranges::max(
				setup.vessel.simulate() |
				std::views::take_while([](stosim::VesselStep vesselStep) { return vesselStep.time < 100; }) |
				std::views::transform([&](stosim::VesselStep step) { return step.state[H_token]; })
			);
		}
		benchmark::DoNotOptimize(total);
		benchmark::ClobberMemory();
	}
}

BENCHMARK(single_threaded);

void multi_threaded(benchmark::State& state) {
	auto setup = covid19(10000);
	auto H_token = setup.H.get_agent_token();
	for (auto _ : state) {
		auto simulation_results = setup.vessel.multi_simulate(100, [=](coro::generator<stosim::VesselStep> simulation) -> stosim::agent_count_t {
			return std::ranges::max(simulation |
				std::views::take_while([](stosim::VesselStep step) { return step.time < 100; }) |
				std::views::transform([&](stosim::VesselStep step) ->  stosim::agent_count_t { return step.state[H_token]; })
			);
			});

		stosim::agent_count_t total = std::ranges::fold_left(simulation_results, 0.0,
			[](stosim::agent_count_t acc, stosim::agent_count_t next) { return acc + next; });
		benchmark::DoNotOptimize(total);
		benchmark::ClobberMemory();
	}
}

BENCHMARK(multi_threaded);

void multi_threaded2(benchmark::State& state) {
	auto setup = covid19(10000);
	auto H_token = setup.H.get_agent_token();
	for (auto _ : state) {
		auto simulation_results = setup.vessel.multi_simulate(100, [=](auto simulation) -> stosim::agent_count_t {
			return std::ranges::max(simulation |
				std::views::take_while([](auto step) { return step.time < 100; }) |
				std::views::transform([&](auto step) ->  stosim::agent_count_t { return step.state[H_token]; })
			);
			});

		double total = std::ranges::fold_left(simulation_results, 0.0,
			[](double acc, stosim::agent_count_t next) { return acc + ((double)next); });
		benchmark::DoNotOptimize(total);
		benchmark::ClobberMemory();
	}
}

BENCHMARK(multi_threaded2);

BENCHMARK_MAIN();