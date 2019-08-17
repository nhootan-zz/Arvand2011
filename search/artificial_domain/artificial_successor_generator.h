#ifndef ARTIFICIAL_SUCCESSOR_GENERATOR_H
#define ARTIFICIAL_SUCCESSOR_GENERATOR_H

#include "../successor_generator.h"

class Operator;
class State;
class RHSimulator;
class MTRand_int32;

class ArtificialSuccessorGenerator : public SuccessorGenerator {
  std::vector<Operator *> operators;
  RHSimulator *params;

 public:
  ArtificialSuccessorGenerator(RHSimulator *params);
  virtual ~ArtificialSuccessorGenerator();
  virtual void generate_applicable_ops(const State &curr,
                                       std::vector<const Operator *> &ops);
  virtual void _dump(std::string indent);
};

#endif
