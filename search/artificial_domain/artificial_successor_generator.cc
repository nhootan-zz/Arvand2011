#include "r_h_simulator.h"
#include "artificial_successor_generator.h"
#include "artificial_operator.h"
#include "../successor_generator.h"
#include "../rng.h"
#include <algorithm>
#include <iostream>

using namespace std;

// random generator function:
ptrdiff_t myrandom (ptrdiff_t i) { return g_rng.next32() %i;}

// pointer object to it:
ptrdiff_t (*p_myrandom)(ptrdiff_t) = myrandom;

ArtificialSuccessorGenerator::ArtificialSuccessorGenerator(RHSimulator* p) : params(p) {
	operators.push_back(new ArtificialOperator(-1));
	operators.push_back(new ArtificialOperator(0));
	operators.push_back(new ArtificialOperator(1));
}
void ArtificialSuccessorGenerator::
		generate_applicable_ops(const State &curr, std::vector<const Operator *> &ops){

	operators.reserve(params->branching_factor);
	int effect = -1;
	int i = 0;
	for (; i < params->progress; ++i) {
		ops.push_back(operators[effect + 1]);
	}
	//cout << i << endl;
	effect = 1;
	assert(i <= params->progress + params->regress);
	for (; i < params->progress + params->regress; ++i) {
		ops.push_back(operators[effect + 1]);
	}
	// cout << i << endl;
	assert(i <= params->branching_factor);
	effect = 0;
	for (; i < params->progress + params->regress; ++i) {
		ops.push_back(operators[effect + 1]);
	}
	// cout << i << endl;

	// random shuffle
	random_shuffle ( ops.begin(), ops.end(), p_myrandom);

		//TODO: move the following to the heuristic function
		// compute whether state is a preferred successor
		/* double prob;
		if (distance == parent_d - 1)
			prob = g_prob_forwards_preferred / 100.0;
		else if (distance == parent_d)
			prob = g_prob_stay_preferred / 100.0;
		else {
			assert(distance == parent_d + 1);
			prob = g_prob_backwards_preferred / 100.0;
		}
		rand_val = (double) rand() / RAND_MAX;
		assert(rand_val >= 0 && rand_val <= 1);
		if (rand_val >= prob)
			preferred = false;
		else
			preferred = true;*/
}

void ArtificialSuccessorGenerator::_dump(std::string indent) {
	cout << "artificial" << indent << " successor generator" << endl;
}

ArtificialSuccessorGenerator::~ArtificialSuccessorGenerator(){
	for (int i = 0; i < operators.size(); ++i) {
		delete operators[i];
	}
}

