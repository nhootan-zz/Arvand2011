/*
 * postprocessor.h
 *
 *  Created on: Feb 13, 2013
 *      Author: hootan
 */

#ifndef POSTPROCESSOR_H_
#define POSTPROCESSOR_H_

class Operator;
class Postprocessor {
 public:
  virtual ~Postprocessor() {}
  virtual void run() = 0;
  virtual void run(vector<const Operator*>& plan) = 0;
};

#endif /* POSTPROCESSOR_H_ */
