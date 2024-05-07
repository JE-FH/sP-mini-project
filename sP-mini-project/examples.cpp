#include <set>
#include <algorithm>
#include "library/stosim.hpp"

stosim::Vessel circadian_rhythm() {
	using namespace std;
	const auto alphaA = 50;
	const auto alpha_A = 500;
	const auto alphaR = 0.01;
	const auto alpha_R = 50;
	const auto betaA = 50;
	const auto betaR = 5;
	const auto gammaA = 1;
	const auto gammaR = 1;
	const auto gammaC = 2;
	const auto deltaA = 1;
	const auto deltaR = 0.2;
	const auto deltaMA = 10;
	const auto deltaMR = 0.5;
	const auto thetaA = 50;
	const auto thetaR = 100;
	auto v = stosim::Vessel{ "Circadian Rhythm" };
	const auto env = v.environment();
	const auto DA = v.add("DA", 1);
	const auto D_A = v.add("D_A", 0);
	const auto DR = v.add("DR", 1);
	const auto D_R = v.add("D_R", 0);
	const auto MA = v.add("MA", 0);
	const auto MR = v.add("MR", 0);
	const auto A = v.add("A", 0);
	const auto R = v.add("R", 0);
	const auto C = v.add("C", 0);
	v.add((A + DA) >> gammaA >>= D_A);
	v.add(D_A >> thetaA >>= DA + A);
	v.add((A + DR) >> gammaR >>= D_R);
	v.add(D_R >> thetaR >>= DR + A);
	v.add(D_A >> alpha_A >>= MA + D_A);
	v.add(DA >> alphaA >>= MA + DA);
	v.add(D_R >> alpha_R >>= MR + D_R);
	v.add(DR >> alphaR >>= MR + DR);
	v.add(MA >> betaA >>= MA + A);
	v.add(MR >> betaR >>= MR + R);
	v.add((A + R) >> gammaC >>= C);
	v.add(C >> deltaA >>= R);
	v.add(A >> deltaA >>= env);
	v.add(R >> deltaR >>= env);
	v.add(MA >> deltaMA >>= env);
	v.add(MR >> deltaMR >>= env);
	return v;
}
/*
int main() {
    using namespace matplot;
	auto vessel = circadian_rhythm();

	std::vector<std::vector<double>> Xs;
	std::vector<double> Y;

	for (auto i = 0; i < vessel.get_state().size(); i++) {
		Xs.push_back(std::vector<double>());
	}
	
	
	double minx = 0;
	double maxx = 0;

	while (vessel.get_time() < 100) {
		const auto& state = vessel.get_state();
		for (auto i = 0; i < state.size(); i++) {
			Xs[i].push_back((double)state[i]);
			minx = std::min((double) state[i], minx);
			maxx = std::max((double) state[i], maxx);
		}
		Y.push_back(vessel.get_time());
		vessel.Step();
	}
	std::cout << "min: " << minx << ", max: " << maxx << std::endl;
	hold(on);

	for (const auto& X : Xs) {
		plot(X, Y);
	}

    show();
    return 0;
}*/

int main(int argc, char** argv) {
	std::vector<int> test_data;
	for (int i = 0; i < 20; i++) {
		test_data.push_back(i);
	}

	plt::bar(test_data);
	plt::show();

	return (0);
}