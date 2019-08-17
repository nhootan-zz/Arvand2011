
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

#include "walker.h"

#include "../globals.h"
#include "../rng.h"
#include "../successor_generator.h"
#include "math.h"

void WalkInfo::init() {
  branching = 0;
  goal_visited = false;
  length_offset = 0;
  path.clear();
  value = MRW::A_LOT;
  num_generated = 0;
  num_evaluated = 0;
  eval_limit_exceeded = false;
}

Walker::Walker(MRW_Parameters& params) {
  E1 = exp(1 / params.mha_temperature);             // E1 = e^(1/T)
  EW = exp(params.mha_w / params.mha_temperature);  // EW = e^(w/T)
}

Walker::~Walker() {}

void Walker::prepare_for_walks(Heuristic* h, MRW_Parameters& params) {
  heuristic = h;
  max_QW = 1;
  // initialize appropriate q value counters if necessary
  if (params.bias == MHA || params.bias == AGRESSIVE_MHA) {
    if (MHA_Q.size() == 0) {
      MHA_Q.resize(g_operators.size(), make_pair(1, 1));
    } else {
      for (int i = 0; i < MHA_Q.size(); i++) {
        MHA_Q[i] = make_pair(1, 1);
      }
    }
    if (is_preferred.size() == 0)
      is_preferred.resize(g_operators.size(), false);
  }

  if (params.bias == MDA || params.sample_unused) {
    if (MDA_Q.size() == 0) {
      MDA_Q.resize(g_operators.size(), MDA_Info(0, 0, 1));
    } else {
      for (int i = 0; i < MDA_Q.size(); i++) {
        MDA_Q[i] = MDA_Info(0, 0, 1);
      }
    }
  }
}

// TODO Probably shouldn't do it this way
WalkInfo Walker::get_info() { return my_info; }
// determines if the algorithm should evaluate a state or not
bool Walker::should_evaluate(MRW_Parameters& params) {
  double r = g_rng.next_half_open();
  if (r < params.pe) {
    return true;
  } else
    return false;
}

// TODO: currently the path can lead to a state that has higher value than
// info.value. However, this is fine the current version because this does not
// happen when info.value < current_min but for the next versions this should be
// fixed
void Walker::random_walk(State initial_state,
                         vector<const Operator*> preferred_ops, int current_min,
                         int current_cost, int bound, MRW_Parameters& params) {
  my_info.init();
  State current_state = initial_state;
  const Operator* op = 0;
  bool endpoint_evaluated = false;
  // performs bounded random walk
  do {
    endpoint_evaluated = false;
    vector<const Operator*> applicable_ops;
    g_successor_generator->generate_applicable_ops(current_state,
                                                   applicable_ops);
    my_info.branching += applicable_ops.size();
    if (params.sample_unused) {
      op = mu_successor(applicable_ops, preferred_ops, params);
    } else {
      if (params.bias == MHA || params.bias == AGRESSIVE_MHA) {
        update_mha_action_values(preferred_ops);
        op = mha_successor(applicable_ops, preferred_ops, params);
      } else if (params.bias == MDA) {
        op = mda_successor(applicable_ops);
      } else {
        op = random_successor(applicable_ops);
      }
    }

    if (op == 0) {
      my_info.value = MRW::A_LOT;
      update_mda_action_values(my_info.path, params, true);
      return;
    } else {
      current_cost += op->get_cost();
      // add new state to path
      my_info.path.push_back(op);
      current_state.update(*op);
      my_info.num_generated++;
      if (test_goal(current_state)) {
        // goal visited
        if (current_cost >= bound) {
          // treat as if it hit deadend
          my_info.value = MRW::A_LOT;
          update_mda_action_values(my_info.path, params, true);
          return;
        }

        my_info.goal_visited = true;
        my_info.value = 0;
        return;
      } else if (should_evaluate(params)) {
        int h = evaluate(current_state);
        endpoint_evaluated = true;
        if (h == MRW::A_LOT) {
          // hit a deadend
          update_mda_action_values(my_info.path, params, true);
          return;
        } else if (h < my_info.value) {
          my_info.value = h;
        }
        if (h < current_min) {
          update_mda_action_values(my_info.path, params, false);
          return;
        }
        // compute preferred ops only if the walk continues
        preferred_ops.clear();
        heuristic->get_preferred_operators(preferred_ops);
      } else {
        preferred_ops.clear();
        // cout << " new preferred size: " << preferred_ops.size();
      }
    }
  } while (continue_walk(params, current_cost, bound));
  if (!endpoint_evaluated) {
    // no matter what is pe evaluate the endpoint of the walk
    int h = evaluate(current_state);
    // cout << "endpoint got evaluated" << endl;
    if (h == MRW::A_LOT) {
      update_mda_action_values(my_info.path, params, true);
      return;
    }
    if (h < my_info.value) {
      my_info.value = h;
    }
    if (params.bias == MHA || params.bias == MHA) {
      // updating mha scores
      preferred_ops.clear();
      heuristic->get_preferred_operators(preferred_ops);
      update_mha_action_values(preferred_ops);
    }
    update_mda_action_values(my_info.path, params, false);
  }
}

// determines if we should terminate the walk or not
bool Walker::continue_walk(MRW_Parameters& params, int current_cost,
                           int bound) {
  if (current_cost >= bound)
    return false;
  else {
    double r = g_rng.next_half_open();
    // cout << "random: " << r << " rl: " << params.rl << endl;
    if (r < params.rl) {
      return false;
    } else
      return true;
  }
}

const Operator* Walker::mu_successor(vector<const Operator*>& applicable_ops,
                                     vector<const Operator*>& preferred_ops,
                                     MRW_Parameters& params) {
  if (applicable_ops.size() == 0) return 0;
  int op_index = 0;
  double num_unused_actions = 0;
  for (int i = 0; i < applicable_ops.size(); ++i) {
    int global_index = applicable_ops[i]->get_op_index();
    // cout << i << ": num " << MDA_Q[global_index].num << endl;
    //		  << " num_failures " << MDA_Q[global_index].num_failures
    //		  << " score " << MDA_Q[global_index].score << endl;
    if (MDA_Q[global_index].num == 0) {
      // found an unused action
      num_unused_actions++;
      double random_number = g_rng.next_half_open();
      if (random_number < 1 / num_unused_actions) {
        // with probability 1/num_unused_actions replace the selected action
        op_index = i;
      }
    }
  }
  if (num_unused_actions == 0) {
    if (params.bias == MHA || params.bias == AGRESSIVE_MHA) {
      update_mha_action_values(preferred_ops);
      return mha_successor(applicable_ops, preferred_ops, params);
    } else if (params.bias == MDA) {
      return mda_successor(applicable_ops);
    } else {
      return random_successor(applicable_ops);
    }
  } else {
    assert(op_index >= 0 && op_index < applicable_ops.size());
    // cout << op_index << " is selected" << endl;
    return applicable_ops[op_index];
  }
}

const Operator* Walker::mda_successor(vector<const Operator*>& applicable_ops) {
  if (applicable_ops.size() == 0) return 0;

  double sum_scores = 0;
  int op_index = 0;
  for (int i = 0; i < applicable_ops.size(); ++i) {
    int global_index = applicable_ops[i]->get_op_index();
    // cout << i << ": num " << MDA_Q[global_index].num
    //		  << " num_failures " << MDA_Q[global_index].num_failures
    //		  << " score " << MDA_Q[global_index].score << endl;

    double score = MDA_Q[global_index].score;
    if (MDA_Q[global_index].num == 0) {
      // found an unused action
      assert(score == 1);
    }
    sum_scores += score;
    double random_number = g_rng.next_half_open();
    if (random_number < score / sum_scores) {
      // with probability score/sum_score replace the selected action
      op_index = i;
    }
  }
  // cout << op_index << " is selected" << endl;

  assert(op_index >= 0 && op_index < applicable_ops.size());
  return applicable_ops[op_index];
}

const Operator* Walker::mha_successor(
    vector<const Operator*>& applicable_ops,
    vector<const Operator*>& preferred_operators, MRW_Parameters& params) {
  if (applicable_ops.size() == 0) return 0;
  // Determine which actions are preferred
  for (int i = 0; i < preferred_operators.size(); i++) {
    int global_index = preferred_operators[i]->get_op_index();
    is_preferred[global_index] = true;
  }
  // max_QW = max_a{e^(w*n(a)/T)}
  double preferred_score = max_QW;
  double sum_scores = 0;
  int op_index = 0;

  for (int i = 0; i < applicable_ops.size(); ++i) {
    int global_index = applicable_ops[i]->get_op_index();
    // cout << i << ": ( " << Q[global_index].first << " " <<
    // Q[global_index].second << " ) " << endl;
    double score = 0;
    // Q[global_index] == (e^(n(a)/T), e^(w*n(a)/T))
    // E1 == e^(1/T)
    if (preferred_operators.size() == 0) {
      // score = e^(n(a)/T)
      score = MHA_Q[global_index].first;
    } else if (is_preferred[global_index]) {
      // score = e^((w*max_n + (1-w)* n(a))/T)
      // cout << "the action is preferred" << endl;
      score = (MHA_Q[global_index].first / MHA_Q[global_index].second) *
              preferred_score;
    } else {
      // score = e^(((1-w)* n(a))/T)
      // cout << "the action is not preferred" << endl;
      score = (MHA_Q[global_index].first / MHA_Q[global_index].second);
    }
    // cout << "score: " << score << endl;
    if (score == 1 && params.bias == AGRESSIVE_MHA) {
      //  AGRESSIVE_MHA ignores operators that are not preferred
      // cout << "ignoring the action" << endl;
      continue;
    }
    sum_scores += score;
    double random_number = g_rng.next_half_open();
    if (random_number < score / sum_scores) {
      // with probability score/sum_score replace the selected action
      op_index = i;
    }
    // cout << "i: " << i << " score: " << score << " sum_scores: " <<
    // sum_scores; cout << " r: " << random_number << " op_index: "  << op_index
    // << endl;
  }
  if (sum_scores == 0) {
    assert(params.bias == MRW_Parameters::AGRESSIVE_MHA);
    // cout << "selecting randomly" << endl;

    // this only happens when bias==AGRESSIVE_MHA
    // and none of the operators are preferred
    op_index = g_rng.next32() % applicable_ops.size();
    // select one of the applicable operators randomly
  }
  // clean up the is_preferred structure
  for (int i = 0; i < preferred_operators.size(); i++) {
    int global_index = preferred_operators[i]->get_op_index();
    is_preferred[global_index] = false;
  }
  assert(op_index >= 0 && op_index < applicable_ops.size());
  return applicable_ops[op_index];
}

int Walker::evaluate(State& current_state) {
  my_info.num_evaluated++;
  heuristic->evaluate(current_state);

  // is at a dead end
  if (heuristic->is_dead_end()) {
    return MRW::A_LOT;
  }

  // get heuristic value
  return heuristic->get_heuristic();
}

const Operator* Walker::random_successor(
    vector<const Operator*>& applicable_ops) {
  if (applicable_ops.size() == 0) return 0;
  int index = g_rng.next32() % applicable_ops.size();
  const Operator* op = applicable_ops[index];
  return op;
}

void Walker::update_mda_action_values(vector<const Operator*>& path,
                                      MRW_Parameters& params, bool deadend) {
  if (params.bias != MDA && !params.sample_unused) return;
  set<int> added;
  for (int i = 0; i < path.size(); ++i) {
    int op_index = path[i]->get_op_index();
    set<int>::iterator it = added.find(op_index);
    if (it == added.end()) {
      if (deadend) MDA_Q[op_index].num_failures++;

      MDA_Q[op_index].num++;
      MDA_Q[op_index].score =
          exp(-MDA_Q[op_index].num_failures /
              (MDA_Q[op_index].num * params.mda_temperature));
      added.insert(op_index);
    }
  }
}

void Walker::update_mha_action_values(vector<const Operator*>& preferred_ops) {
  // E1 = e^(1/T)
  // EW = e^(w/T)
  // cout << "updating action values" << endl;
  for (int i = 0; i < preferred_ops.size(); ++i) {
    int global_index = preferred_ops[i]->get_op_index();
    // cout << "(" << Q[global_index].first << " " << Q[global_index].second <<
    // ")";
    MHA_Q[global_index].first *= E1;
    MHA_Q[global_index].second *= EW;
    // cout << " >>> (" << Q[global_index].first << " " <<
    // Q[global_index].second << ")" << endl;
    if (MHA_Q[global_index].second > max_QW) {
      max_QW = MHA_Q[global_index].second;
      // cout << "updating max: " <<  max_QW << endl;
    }
  }
  // cout << "max_QW: " <<  max_QW << endl;

  // Q[a].first = e^(n(a)/T)
  // Q[a].second = e^(w * n(a)/T)
}
