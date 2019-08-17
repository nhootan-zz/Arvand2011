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

#ifndef MRW_H_
#define MRW_H_

#include <queue>

#include "../heuristic.h"
#include "../operator.h"
#include "../search_engine.h"
#include "../state.h"
#include "mrw_parameters.h"
#include "walker.h"

using namespace std;
typedef vector<const Operator*> Path;
class WalkInfo;
class Walker;
class MRW;

class RWStats {
 public:
  unsigned int num_evaluated;
  unsigned int num_generated;
  unsigned int num_walks;
  unsigned int sum_len_walk;
  unsigned int sum_bf;
  RWStats();
  void update(WalkInfo& info);
  void reset();  // resets only stats that are kept for local neighborhood
  std::string to_string(std::string);
};

class MRW : public SearchEngine {
 public:
 private:
  int initial_value;              // initial state value
  int last_local_improving_walk;  // the last local walk that decreased
                                  // heuristic
  int num_nonwasted_walks;  // the total number of walks that are run before
                            // last local improvements
  int total_min;
  int num_episodes;
  double avg_speed;
  double acceptable_progress;
  RWStats global_stats;
  RWStats local_stats;
  RWStats episode_stats;
  State current_state;
  int current_cost;
  Path current_trajectory;
  Walker* walker;
  MRW_Parameters params;
  std::vector<Heuristic*> heuristics;
  std::vector<const Operator*> preferred_ops;
  Heuristic* heuristic;  // current heuristic
  void get_preferred(vector<const Operator*>& preferred);
  void update_acceptable_progress(int progress);
  void update_stats(WalkInfo& info);
  void jump(Path& path);
  void restart();
  void dump_path(Path& p);
  bool global_restart(int progress);
  void report_progress();
  bool should_jump(int progress);
  void update_h_speed();
  void update_rg();
  int search_step();
  void reinit();

 protected:
  virtual int step();

 public:
  virtual void initialize();
  int search_episode();
  enum { A_LOT = 10000000 };
  MRW(const Options& opts);
  virtual ~MRW();
};

#endif /*MRW_H_*/
