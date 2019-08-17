#ifndef ANALYZER_NODE_INFO_H
#define ANALYZER_NODE_INFO_H

#include "../state.h"

class AnalyzerNodeInfo {
    friend class AnalyzerNode;
    friend class AnalyzerSpace;
    vector<state_var_t *>parent_states;

    enum NodeFWDStatus {NEW = 0, OPEN = 1, CLOSED = 2, DEAD_END = 3};

    unsigned int fwd_status : 2;
    unsigned int bwd_status : 2;
    int h : 30;
    int d : 30;
    AnalyzerNodeInfo(){}
};


#endif
