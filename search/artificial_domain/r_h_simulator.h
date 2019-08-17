#ifndef ARTIFICAL_PARAMS_H
#define ARTIFICAL_PARAMS_H

#include "../simulator.h"
// based on the Simulator introduced
// in "Preferred Operators and Deferred Evaluation in Satisficing Planning"
// by Silvia Richter and Malte Helmert

class Options;

// default values for RHSimulator
static const int DEFAULT_D = 50;
static const int DEFAULT_BF = 25;
static const int DEFAULT_P = 1;
static const int DEFAULT_R = 1;

class RHSimulator : public Simulator {
 public:
  int solution_depth;
  int branching_factor;
  int progress;
  int regress;
  Heuristic* h_;

  // extern int node_limit;
  // extern int g_preferred_operator_boost;

  /*int prob_forwards_preferred;
  int prob_stay_preferred;
  int prob_backwards_preferred;*/
  RHSimulator(const Options& opts);
  virtual void run();
  virtual void set_heuristic(Heuristic* h) { h_ = h; }
  virtual Heuristic* get_heuristic() { return h_; }
  void dump();
};

#endif
