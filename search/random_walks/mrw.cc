/*********************************************************************
 * Author: Hootan Nakhost (Nakhost@ualberta.ca)
 * (C) Copyright 2009-2013 Hootan Nakhost
 *
 * This file is part of Arvand.
 *
 * Arvand is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the license, or (at your option) any later version.
 *
 * Arvand is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/


#include "mrw.h"
#include "walker.h"
#include "mrw_parameters.h"
#include "../globals.h"
#include "../option_parser.h"
#include "../successor_generator.h"
#include "../plugin.h"
#include "../rng.h"


RWStats::RWStats(){
	reset();
}

void RWStats::update(WalkInfo& info){
	num_evaluated += info.num_evaluated;
	num_generated += info.num_generated;
	num_walks ++;
	sum_len_walk += info.path.size();
	sum_bf += info.branching;
}

void RWStats::reset(){
	num_evaluated = 0;
	num_generated = 0;
	num_walks = 0;
	sum_len_walk = 0;
	sum_bf = 0;
}

std::string RWStats::to_string(std::string prefix){
	std::stringstream ss;
	ss << " #" << prefix << "evaluated: " << num_evaluated;
	ss << " #" << prefix << "generated: " << num_generated;
	ss << " #" << prefix << "walks: " << num_walks;
	ss << " " << prefix << "sum walk len: " << sum_len_walk << endl;
	if(num_walks != 0)
		ss << " " << prefix << "avg walk len: " << sum_len_walk / num_walks;
	if(num_evaluated != 0)
		ss << " " << prefix << "avg bf: " << sum_bf / num_evaluated ;
	return ss.str();
}

MRW::MRW(const Options &opts) :SearchEngine(opts), current_state(*g_initial_state){
    params.rl = opts.get<double>("rl");
    params.rg = opts.get<double>("rg");
    params.tg = opts.get<int>("tg");
    params.pe = opts.get<double>("pe");
    /*params.eval_limit = opts.get<int>("eval_limit");
    if(params.eval_limit != -1){
    	cerr << "Error: the current version does not support eval_limit" << endl;
    	exit(1);
    }*/
    params.adaptive_gr = opts.get<bool>("adaptive_gr");
    params.deterministic_gr = opts.get<bool>("deterministic_gr");
    params.adaptive_lr = opts.get<bool>("adaptive_lr");
    params.nj = opts.get<int>("nj");
    params.acceptable_progress = opts.get<bool>("acceptable_progress");
    params.alpha = opts.get<double>("alpha");
    params.mha_temperature = opts.get<double>("mha_t");
    params.mda_temperature = opts.get<double>("mda_t");
    params.mha_w = opts.get<double>("mha_w");
    params.bias = Bias(opts.get_enum("bias"));
    params.sample_unused = opts.get<bool>("sample_unused");
    params.epsilon = opts.get<double>("epsilon");
    params.dump();
    ScalarEvaluator * evaluator = opts.get<ScalarEvaluator *>("eval");
    std::set<Heuristic *> hset;
    evaluator->get_involved_heuristics(hset);
    for (set<Heuristic *>::iterator it = hset.begin(); it != hset.end(); it++) {
    	heuristics.push_back(*it);
    }
    assert(heuristics.size() == 1);
    heuristic = heuristics[0];
    walker = new Walker(params);
}

MRW::~MRW() {
    delete walker;
}

void MRW::update_h_speed(){
	num_episodes ++;
	if(num_nonwasted_walks != 0){
		double current_speed = (initial_value - total_min)/ double(num_nonwasted_walks);
		avg_speed = (current_speed - avg_speed)/ double(num_episodes) + avg_speed;
		cout << "nonwasted walks: " << num_nonwasted_walks << " avg speed: " << avg_speed << endl;
	}else{
		// IF the last_improving_walk is 0. Then, MRW is hopeless
		assert(initial_value == total_min);
	}
}

// determines if the algorithm should restart from the initial state or not
bool MRW::global_restart(int progress){
	if(current_cost >= bound){
		return true;
	}else if(progress == 0){
		// restart only if the last walk did not improve total_min
		if(params.deterministic_gr){
			if(local_stats.num_walks > params.tg)
				return true;
			else
				return false;
		}else{
			double r = g_rng.next_half_open();
			if(r < params.rg){
				return true;
			}else
				return false;
		}
	}else
		return false;
}

void MRW::update_stats(WalkInfo& info){
	global_stats.update(info);
	local_stats.update(info);
	episode_stats.update(info);
}


void MRW::update_rg(){
	update_h_speed();
	if(!params.adaptive_gr){
		// no dynamic adjustment of rg
		return;
	}else if(params.deterministic_gr){
		if(avg_speed != 0 )
			params.tg = int(initial_value/avg_speed);
		else
			params.tg = int(1.1 * params.tg);
			cout << "new tg: " << params.tg << endl;
	}else{
		// changing the mean of the geometric distribution
		params.rg = avg_speed / initial_value;
		cout << "new rg: " << params.rg << endl;
	}
}

bool MRW::should_jump(int progress){
	if(progress != 0){
		if(local_stats.num_walks > params.nj){
			//cout << "jumping ... exceeded nj" << endl;
			return true;
		}else if(params.acceptable_progress && acceptable_progress != -1
					&& progress > acceptable_progress){
			//cout << "jumping... reached the acceptable progress" << endl;
			//cout << "progress: " << progress << " ap: " << acceptable_progress << endl;
			return true;
		} else
			return false;
	}else
		return false;
}

void MRW::get_preferred(vector<const Operator* >& preferred){
	if(params.bias == UNIFORM){
		// preferred ops are not needed
		return;
	}else if(params.bias == MHA){
		heuristic->evaluate(current_state);

		// is at a dead end
		if(heuristic->is_dead_end()){
			return;
		}
		heuristic->get_preferred_operators(preferred);
	}
}

int MRW::search_step() {
    local_stats.reset(); // resets the stats kept locally
    walker->prepare_for_walks(heuristic, params);
	Path best_path;
	int h_min = total_min;
	int progress = 0;
	vector<const Operator*> preferred_ops; // current preferred ops
	get_preferred(preferred_ops);
	//cout << "current preferred_ops size: " << preferred_ops.size() << endl;
	while(true) {
		if(best_path.size() != 0 && should_jump(progress)){
			update_acceptable_progress(progress);
			jump(best_path);
			return IN_PROGRESS;
		}
		if(global_restart(progress)){
			restart();
			return RESTART;
		}
		params.change_rl();
		walker->random_walk(current_state, preferred_ops, total_min, current_cost, bound, params);
		//current_state.dump();
		WalkInfo info = walker->get_info();
		update_stats(info);
		/*if(info.eval_limit_exceeded){
			update_h_speed();
			cout << "global: " << global_stats.to_string("g_") << endl;
			cout << "last h_speed: " << avg_speed << endl;
			return FAILED;
		}*/
		if((info.value < total_min) || info.goal_visited){
			// jump as soon as the total_min is decreased.
			params.feedback_rl(total_min - info.value, info.num_evaluated);
			progress = h_min - info.value;
			last_local_improving_walk = local_stats.num_walks;
			best_path = info.path;
			total_min = info.value;
			report_progress();
			if (info.goal_visited) {
				//update_h_speed();
				jump(best_path);
				set_plan(current_trajectory);
				//save_plan(current_trajectory, 0);
				cout << "episode: " << episode_stats.to_string("e_") << endl;
				cout << "global: " << global_stats.to_string("g_") << endl;
				cout << "current cost: " << current_cost << endl;
				//cout << "last h_speed: " << avg_speed << endl;
				reinit();
				return SOLVED;
			}
		}else{
			params.feedback_rl(0, info.num_evaluated);
		}
	}
}

int MRW::search_episode() {
   // Each episode starts from the initial state and ends
   // when a restarting/termination condition holds.
	//cout << "new episode ..." << endl;
	cout << "bound: " << bound << endl;
	report_progress();
	int search_status = search_step();
	//cout << search_status << endl;
    while (search_status == IN_PROGRESS){
    	//cout << search_status << endl;
    	search_status = search_step();
    }
    if(search_status == RESTART){
    	// a restarting condition is met.
    	// the current episode finishes but we will continue searching
    	return IN_PROGRESS;
    }else{
    	// a termination condition is met: search_status == FAILURE or SOLVED
    	return search_status;
    }
}

int MRW::step() {
	// Each step in MRW is a "search episode" consisting of
	// many "search steps"
	return search_episode();
}

void MRW::update_acceptable_progress(int progress) {
	if(params.acceptable_progress == false)
		return;
	if (acceptable_progress == -1){
		// first step
		acceptable_progress = progress;
	}else
		acceptable_progress = (1 - params.alpha) * acceptable_progress + params.alpha * progress;

	if (acceptable_progress < 0.001)
		acceptable_progress = 0;
}


void MRW::report_progress(){
	//cout << "h_min: " << total_min << global_stats.to_string("gs_") << endl;
	//cout << "h_min: " << total_min << local_stats.to_string("l_") << endl;
	
	cout << "h_min: " << total_min 
		<< " #walks: " << global_stats.num_walks 
		<< " #evaluations: " << global_stats.num_evaluated
		<< " cost: " << current_cost << endl;
	//cout << "#g_evaluated: " << global_stats.num_evaluated << " #g_generated: " << global_stats.num_generated << " g_stats"<< endl;
}

void MRW::initialize() {
	//cout << "MRW ..." << endl;
    current_state = *g_initial_state;
    //current_state->dump();
    // get heuristic values of the initial state
    heuristic->evaluate(current_state);
    if (heuristic->is_dead_end()) {
	    assert(heuristic->dead_ends_are_reliable());
	    cout << "Initial state is a dead end." << endl;
	    exit(0);
    }
    initial_value = heuristic->get_value();
    total_min = initial_value;
    cout << "Initial heuristic value: ";
    cout << initial_value << endl;
    current_cost = 0;
    avg_speed = 0;
    num_episodes = 0;
    num_nonwasted_walks = 0;
    acceptable_progress = -1;
    current_trajectory.clear();
    //report_progress();
}

void MRW::restart(){
	report_progress();
	cout << "restart" << endl;
	reinit();
}

void MRW::reinit(){
	update_rg();
	current_trajectory.clear();
	total_min = initial_value;
    current_state = *g_initial_state;
    num_nonwasted_walks = 0;
    episode_stats.reset();
    current_cost = 0;
}

void MRW::dump_path(Path& p){
	for (int i = 0; i < p.size(); ++i) {
		cout << i << ": "<<p[i]->get_name() << endl;
	}
}

void MRW::jump(Path& path){
	num_nonwasted_walks += last_local_improving_walk;
	// cout << "num_nonwasted_walks: " << num_nonwasted_walks << endl;
	// updating the current state
	for (int i = 0; i < path.size(); ++i) {
		current_state.update(*path[i]);
		current_cost += path[i]->get_cost();
	}
	// updating the current trajectory
	current_trajectory.insert(current_trajectory.end(), path.begin(), path.end());
}

static SearchEngine *_parse(OptionParser &parser) {
    parser.add_option<ScalarEvaluator *>("eval");
    parser.add_option<double>("rl", DEFAULT_RL, "the local restarting rate");
    parser.add_option<double>("rg", DEFAULT_RG,
    		"the global restarting rate; meaningless if deterministic_gr = true");
    parser.add_option<int>("tg", DEFAULT_TG,
    		"the global restarting threshold; meaningless if deterministic_gr = true");
    parser.add_option<bool>("adaptive_lr", DEFAULT_AP_RL, "adaptive local restarting");
    parser.add_option<bool>("adaptive_gr", DEFAULT_DNRG, "adaptive global restarting");
    parser.add_option<bool>("deterministic_gr", DEFAULT_DTRG,
    		"true = use a fix threshold (tg) for global restart; false = use a global restarting rate");
    parser.add_option<int>("nj", DEFAULT_NJ, "the number of walks per step");
    parser.add_option<double>("pe", DEFAULT_PE, "the evaluation rate");
    parser.add_option<bool>("acceptable_progress", DEFAULT_AP, "use acceptable progress for jumping");
    parser.add_option<double>("alpha", DEFUALT_ALPHA,
    		"used to compute acceptable progress: meaningless if acceptable_progress = false");
    //parser.add_option<int>("eval_limit", DEFAULT_EVAL_LIMIT, "eval limit");
    parser.add_option<double>("mda_t", DEFUALT_MDA_T, "MDA temperature: only used for bias = MDA");
    parser.add_option<double>("epsilon", DEFAULT_EPSILON, "Epsilon used for adaptive_lr");
    parser.add_option<double>("mha_t", DEFUALT_MHA_T, "MHA temperature: only used for bias = MHA or AGRESSIVE_MHA");
    parser.add_option<double>("mha_w", DEFUALT_MHA_W, "the weight of current preferred ops in MHA"
    		": only used for bias = MHA or AGRESSIVE_MHA");
    parser.add_option<bool>("sample_unused", DEFAULT_S_UNUSED, "first sample unused actions");

    vector<string> bias_types;
    bias_types.push_back("UNIFORM");
    bias_types.push_back("MHA");
    bias_types.push_back("AGRESSIVE_MHA");
    bias_types.push_back("MDA");
    parser.add_enum_option("bias",
    						bias_types,
                           "MHA",
                           "RW bias");

    SearchEngine::add_options_to_parser(parser);
    Options opts = parser.parse();

    MRW *engine = 0;
    if (!parser.dry_run()) {
        engine = new MRW(opts);
    }

    return engine;
}


static Plugin<SearchEngine> _plugin("mrw", _parse);


