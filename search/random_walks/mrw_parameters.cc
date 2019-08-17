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

#include "mrw_parameters.h"

#include <math.h>

#include <cassert>
#include <iostream>
#include <limits>

#include "../globals.h"
#include "../rng.h"

using namespace std;

MRW_Parameters::MRW_Parameters() {
  num_samples = 0;
  rl_candidates.push_back(0.1);
  rl_candidates.push_back(0.01);
  rl_candidates.push_back(0.001);
  value.resize(rl_candidates.size(), 0);
  cost.resize(rl_candidates.size(), 0);
  num.resize(rl_candidates.size(), 0);
  last_rl = 0;
}

void MRW_Parameters::feedback_rl(int val, int c) {
  if (c == 0)  // if there is no cost, there is no improvement
    return;
  value[last_rl] += val;
  cost[last_rl] += c;
  num[last_rl]++;
  num_samples++;
}

void MRW_Parameters::change_rl() {
  if (!adaptive_lr)
    return;
  else {
    double r = g_rng.next_half_open();
    if (r < epsilon || num_samples == 0) {
      last_rl = random_rl();
      rl = rl_candidates[last_rl];
      // cout << "exploration new rl: " << rl << endl;
    } else {
      double best_q = -1;
      double best_cost = std::numeric_limits<double>::max();
      int best_index = -1;
      int i = 0;
      for (i = 0; i < num.size(); ++i) {
        if (num[i] != 0) {
          // if a setting has not been tried do not consider it
          // now we choose the setting with highest Q =value/cost: tie break by
          // cost. this favors shorter walks at the beginning.
          assert(cost[i] != 0);
          double avg_cost = cost[i] / double(num[i]);
          double avg_value = value[i] / double(num[i]);
          double Q = avg_value / avg_cost;
          // double Q = avg_value;
          // cout << rl_candidates[i] << " Q: " << Q << " cost: " << avg_cost <<
          // endl;
          if (Q > best_q) {
            best_q = Q;
            best_cost = avg_cost;
            best_index = i;
          } else if (Q == best_q && avg_cost < best_cost) {
            best_cost = avg_cost;
            best_index = i;
          }
        }
      }
      // since num_samples > 0 best_index cannot be negative.
      assert(best_index != -1);
      // cout << "BI: " << best_index << endl;
      rl = rl_candidates[best_index];
      last_rl = best_index;
      /*if(num_samples % 100 == 0){
              for (int i = 0; i < num.size(); ++i) {
                      cout << " " << rl_candidates[i] << " " << num[i] ;
              }
              cout << endl;
      }*/
    }
  }
}

int MRW_Parameters::random_rl() {
  int index = g_rng.next32() % rl_candidates.size();
  return index;
}

void MRW_Parameters::dump() {
  cout << "MRW with following parameters" << endl;
  cout << "rl: " << rl << endl;
  cout << "nj: " << nj << " pe: " << pe << endl;
  cout << "bias: ";
  if (bias == UNIFORM)
    cout << "UNIFORM" << endl;
  else if (bias == MHA) {
    cout << "MHA"
         << " T: " << mha_temperature << " w: " << mha_w << endl;
  } else if (bias == AGRESSIVE_MHA) {
    cout << "AGRESSIVE_MHA"
         << " T: " << mha_temperature << " w: " << mha_w << endl;
  } else if (bias == MDA) {
    cout << "MDA"
         << " T: " << mda_temperature << endl;
  }

  if (adaptive_gr)
    cout << "adaptive_gr: True" << endl;
  else
    cout << "adaptive_gr: False" << endl;

  if (deterministic_gr)
    cout << "deterministic_gr: True"
         << " tg: " << tg << endl;
  else
    cout << "deterministic_gr: False"
         << " rg: " << rg << endl;

  /*if(deterministic_rl)
          cout << "deterministic_rl: True" << endl;
  else
          cout << "deterministic_rl: False" << endl;*/

  if (acceptable_progress)
    cout << "acceptable_progress: True alpha: " << alpha << endl;
  else
    cout << "acceptable_progress: False" << endl;

  if (sample_unused) {
    cout << "sample_unused: True" << endl;
  } else {
    cout << "sample_unused: False" << endl;
  }

  if (adaptive_lr) {
    cout << "adaptive_lr: Ture" << endl;
    cout << "epsilon: " << epsilon << endl;
  } else {
    cout << "adaptive_lr: False" << endl;
  }

  // if(eval_limit != -1)
  //	cout << "evaluation limit: " << eval_limit << endl;
}
