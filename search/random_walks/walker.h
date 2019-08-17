
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

#ifndef WALKER_H_
#define WALKER_H_

#include "../globals.h"
#include "../heuristic.h"
#include "../operator.h"
#include "../state.h"
#include "mrw.h"

using namespace std;

// This class is designed to run random_walks.
class MRW_Parameters;

class WalkInfo {
 public:
  int branching;
  int length_offset;
  vector<const Operator*> path;
  bool goal_visited;
  int value;
  int num_generated;
  int num_evaluated;
  bool eval_limit_exceeded;
  void init();
  WalkInfo() {}
  ~WalkInfo() {}
};

struct MDA_Info {
 public:
  int num;
  int num_failures;
  double score;
  MDA_Info() {
    num = 0;
    num_failures = 0;
    score = 1;
  }
  MDA_Info(int n, int nf, double s) : num(n), num_failures(nf), score(s) {}
};
class Walker {
  WalkInfo my_info;
  Heuristic* heuristic;
  double max_QW;  // max_a{e^(w*n(a)/T)}
  double E1;      // constant keeping the value e^(1/T)
  double EW;      // constant keeping the value e^(w/T)
  // Let n(a) be the number of times that a is a preferred operator
  // MHA_Q(a) = (e^(n(a)/T), e^(w*n(a)/T))
  vector<pair<double, double> > MHA_Q;
  // MDA_Q(a) = (e^(-F(a)/n(a)), )

  vector<MDA_Info> MDA_Q;
  vector<bool> is_preferred;  // determines which actions are preferred
  const Operator* random_successor(vector<const Operator*>& applicable_ops);
  const Operator* mha_successor(vector<const Operator*>& applicable_ops,
                                vector<const Operator*>& preferred_ops,
                                MRW_Parameters& params);
  const Operator* mda_successor(vector<const Operator*>& applicable_ops);
  void update_mha_action_values(vector<const Operator*>& preferred_ops);
  void update_mda_action_values(vector<const Operator*>& path,
                                MRW_Parameters& params, bool deadend);
  const Operator* mu_successor(vector<const Operator*>& applicable_ops,
                               vector<const Operator*>& preferred_ops,
                               MRW_Parameters& params);
  int evaluate(State& current_state);
  bool continue_walk(MRW_Parameters& params, int current_cost, int bound);
  bool should_evaluate(MRW_Parameters& params);

 public:
  Walker(MRW_Parameters& params);
  WalkInfo get_info();
  void random_walk(State initial_state, vector<const Operator*> preferred_ops,
                   int current_min, int current_cost, int bound,
                   MRW_Parameters& params);
  /**
   * Prepares the walker for a series of walks during which information will
   * be reused.
   **/
  void prepare_for_walks(Heuristic*, MRW_Parameters&);
  virtual ~Walker();
};

#endif /*WALKER_H_*/
