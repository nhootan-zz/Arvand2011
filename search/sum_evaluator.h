#ifndef SUM_EVALUATOR_H
#define SUM_EVALUATOR_H

#include <vector>

#include "combining_evaluator.h"

class Options;

class SumEvaluator : public CombiningEvaluator {
 protected:
  virtual int combine_values(const std::vector<int> &values);

 public:
  SumEvaluator(const Options &opts);
  SumEvaluator(const std::vector<ScalarEvaluator *> &evals);
  ~SumEvaluator();
};

#endif
