#ifndef MAX_EVALUATOR_H
#define MAX_EVALUATOR_H

#include <vector>

#include "combining_evaluator.h"

class MaxEvaluator : public CombiningEvaluator {
 protected:
  virtual int combine_values(const std::vector<int> &values);

 public:
  MaxEvaluator(const std::vector<ScalarEvaluator *> &subevaluators);
  ~MaxEvaluator();
};

#endif
