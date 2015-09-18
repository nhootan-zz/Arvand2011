#include "r_h_simulator.h"
#include "artificial_successor_generator.h"
#include "../globals.h"
#include "../plugin.h"
#include "../state.h"
#include "../axioms.h"
#include "../heuristic.h"
#include <sstream>
#include <iostream>
using namespace std;

void RHSimulator::run(){
	cout << "Running RHSimulator with following parameters: " << endl;
	dump();
	// replacing the successor_generator, initial state, axiom_evaluator, and goals.
	g_successor_generator = new ArtificialSuccessorGenerator(this);
	g_axiom_evaluator = new AxiomEvaluator;
	stringstream ss;
	//TODO: This seems very hacky. Can we do this better?

	g_variable_domain.push_back(solution_depth * 10);
	g_variable_domain.push_back(INT_MAX);
	g_variable_domain.push_back(INT_MAX);
	//cout << INT_MAX << endl;
	g_variable_name.push_back("distance");
	g_variable_name.push_back("id");
	g_variable_name.push_back("h");
	ss << "begin_state " << solution_depth;
	ss << " " << g_state_id;
	ss << " " << -1;
	ss << " end_state" << endl;
	g_state_id ++;
	g_initial_state = new State(ss);
	h_->evaluate(*g_initial_state);
	int initial_h = h_->get_heuristic();
	g_initial_state->operator[](2) = initial_h;
	// you reach the goal when the goal distance is 0
	g_goal.push_back(make_pair(0, 0));
	g_initial_state->dump();
}

RHSimulator::RHSimulator(const Options &opts){
	solution_depth = opts.get<int>("solution_depth");
	branching_factor = opts.get<int>("branching_factor");
	progress = opts.get<int>("progress");
	regress = opts.get<int>("regress");
	if(progress + regress >  branching_factor){
		cerr << "progress + regress is larger than branching factor.";
		cerr << "progress: " << progress << " regress: ";
		cerr << regress << " bf: " << branching_factor << endl;
		exit(1);
	}
}

void RHSimulator::dump(){
	cout << "solution depth: " << solution_depth << endl;
	cout << "branching factor: " << branching_factor << endl;
	cout << "progress: " << progress << endl;
	cout << "regress: " << regress << endl;
}

static Simulator *_parse(OptionParser &parser) {
    parser.add_option<int>("solution_depth",DEFAULT_D, "the solution depth");
    parser.add_option<int>("branching_factor", DEFAULT_BF, "the branching factor");
    parser.add_option<int>("progress", DEFAULT_P,"the progress chance");
    parser.add_option<int>("regress", DEFAULT_R, "the regress chance");
    Options opts = parser.parse();
    RHSimulator *simulator = 0;
    if (!parser.dry_run()) {
    	simulator = new RHSimulator(opts);
    }
    return simulator;
}


static Plugin<Simulator> _plugin("RH", _parse);
