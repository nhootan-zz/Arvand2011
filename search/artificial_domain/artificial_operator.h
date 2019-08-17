#ifndef ARTIFICIAL_OPERATOR_H
#define ARTIFICIAL_OPERATOR_H

#include <iostream>
#include <sstream>

#include "../globals.h"
#include "../operator.h"
#include "../state.h"
#include "artificial_heuristic.h"
#include "r_h_simulator.h"

using namespace std;
class ArtificialOperator : public Operator {
 public:
  int effect;  // determines how this operator changes the goal distance d
               // after applying the operator the goal distance d will be d +
               // effect where effect = 0, -1, or 1.
  ArtificialOperator(int e) : effect(e) {
    std::stringstream ss;
    ss << e;
    name = ss.str();
    cost = 1;
  }
  virtual void dump() const { cout << "effect: " << effect << endl; }
  virtual void get_effects(const State& state,
                           std::vector<std::pair<int, int> >& effects) const {
    ArtificialHeuristic* heuristic =
        (ArtificialHeuristic*)g_simulator->get_heuristic();
    int parent_d = state[0];
    int d = parent_d + effect;
    effects.push_back(make_pair(0, d));
    effects.push_back(make_pair(1, g_state_id));
    g_state_id++;
    int parent_h = state[2];
    int h = heuristic->compute_heuristic(parent_h, d);
    effects.push_back(make_pair(2, h));
  }
};

#endif
