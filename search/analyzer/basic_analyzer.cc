#include "basic_analyzer.h"
#include "../globals.h"
#include "../plugin.h"
#include "../state.h"
#include "../axioms.h"
#include "../heuristic.h"
#include "../successor_generator.h"
#include <sstream>
#include <iostream>
using namespace std;

BasicAnalyzer::BasicAnalyzer(const Options &opts) : space(PLUSONE) {
    ScalarEvaluator * evaluator = opts.get<ScalarEvaluator *>("eval");
    std::set<Heuristic *> hset;
    evaluator->get_involved_heuristics(hset);
    for (set<Heuristic *>::iterator it = hset.begin(); it != hset.end(); it++) {
    	heuristics.push_back(*it);
    }
}

void BasicAnalyzer::run(){
	cout << "Running Basic Analyzer with following parameters: " << endl;
	dump();
	cout << "running forward search ... " << endl;
	std::vector<state_var_t *> goals = forward_search();
	cout << "forward search is done! ";
	cout << "#reachable states: " << space.size() << endl;
	cout << goals.size() << " goals found" << endl;
	backward_search(goals);
	cout << "backward search is done! ";
	cout << "#reachable states: " << space.size() << endl;
}

std::vector<state_var_t *> BasicAnalyzer::forward_search(){
	AnalyzerNode initial_node = space.get_node(*g_initial_state);
	std::queue<state_var_t *> queue;
	initial_node.fwd_open();
	queue.push(initial_node.get_state_buffer());
	std::vector<state_var_t *> goals;
	while(!queue.empty()){
		State current_state(queue.front());
		queue.pop();
		AnalyzerNode current_node = space.get_node(current_state);
		assert(!current_node.is_fwd_closed());
		assert(current_node.is_fwd_open());
		if(test_goal(current_state)){
			// found a goal
			goals.push_back(current_node.get_state_buffer());
		}else{
			// generate successors
			std::vector<const Operator*> applicable_ops;
			g_successor_generator->generate_applicable_ops(current_state, applicable_ops);
			if(applicable_ops.size() == 0){
				current_node.dead_end();
			}else{
				for (int i = 0; i < applicable_ops.size(); ++i) {
					State succ_state(current_state, *applicable_ops[i]);
					AnalyzerNode succ_node = space.get_node(succ_state);
					succ_node.add_parent(current_node);
					if(succ_node.is_fwd_new()){
						// this is a breadth first search we need to expand each state just once.
						succ_node.fwd_open();
						queue.push(succ_node.get_state_buffer());
					}
				}
			}
		}
		current_node.fwd_close();
	}
	return goals;
}

void BasicAnalyzer::backward_search(std::vector<state_var_t *>& goals){
	int max_d = -1;
	std::queue<state_var_t *> queue;
	for (int i = 0; i < goals.size(); ++i) {
		queue.push(goals[i]);
	}
	while(!queue.empty()){
		State current_state(queue.front());
		queue.pop();
		AnalyzerNode current_node = space.get_node(current_state);
		assert(! current_node.is_bwd_closed());
		if(current_state == *g_initial_state){
			// found the initial state
			continue;
		}else{
			// generate predecessors
			std::vector<state_var_t *> parents = current_node.get_parent_buffers();
			assert(parents.size() > 0);
			for (int i = 0; i < parents.size(); ++i) {
				State pred_state(parents[i]);
				AnalyzerNode pred_node = space.get_node(pred_state);
				if(pred_node.is_bwd_new()){
					// this is a breadth first search we need to expand each state just once.
					int d = current_node.get_d() + 1;
					max_d = max(max_d, d);
					pred_node.set_d(current_node.get_d() + 1);
					pred_node.bwd_open();
					queue.push(pred_node.get_state_buffer());
				} else{
					assert(pred_node.get_d() <= current_node.get_d() + 1);
				}
			}
			current_node.bwd_close();
		}
	}
}

void BasicAnalyzer::dump(){
	cout << "NONE" << endl;
}

static Analyzer *_parse(OptionParser &parser) {
	parser.add_option<ScalarEvaluator *>("eval");
    Options opts = parser.parse();
    BasicAnalyzer *analyzer = 0;
    if (!parser.dry_run()) {
    	analyzer = new BasicAnalyzer(opts);
    }
    return analyzer;
}


static Plugin<Analyzer> _plugin("Basic", _parse);
