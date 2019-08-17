#ifndef ARTIFICIAL_HEURISTIC_H
#define ARTIFICIAL_HEURISTIC_H

#include "../heuristic.h"
static const double DEFAULT_HD = 0.8;
static const double DEFAULT_RC = 0;

class ArtificialState;
class Options;

class ArtificialHeuristic : public Heuristic {
  // determines how accurate the heuristic is
  double deviation_factor;
  // determines how consistent the heuristic is
  double rc;

 protected:
  virtual int compute_heuristic(const State &state);

 public:
  int compute_heuristic(int parent_h, int d);
  ArtificialHeuristic(const Options &opts);
  ~ArtificialHeuristic();
};

#endif
