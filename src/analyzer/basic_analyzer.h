#ifndef BASIC_ANALYZER_H
#define BASIC_ANALYZER_H

#include "../heuristic.h"
#include "../analyzer.h"
#include "analyzer_space.h"

class Options;

// default values for Analyzer

class BasicAnalyzer : public Analyzer {
	AnalyzerSpace space;
public:	
	std::vector<Heuristic*> heuristics;
	BasicAnalyzer(const Options &opts);
	virtual void run();
	std::vector<state_var_t *> forward_search();
	void backward_search(std::vector<state_var_t *>& goals);
	void dump();

};

#endif

