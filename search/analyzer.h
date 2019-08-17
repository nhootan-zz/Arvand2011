/*
 * simulator.h
 */

#ifndef ANALYZER_H_
#define ANALYZER_H_

class Heuristic;

class Analyzer {
 public:
  virtual ~Analyzer() {}
  virtual void run() = 0;
};

#endif /* SIMULATOR_H_ */
