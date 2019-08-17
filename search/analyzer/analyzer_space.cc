#include "Analyzer_space.h"
#include "../state.h"
#include "../operator.h"

#include <cassert>
#include <ext/hash_map>
#include "../state_proxy.h"
#include "Analyzer_node_info.h"

using namespace std;
using namespace __gnu_cxx;




AnalyzerNode::AnalyzerNode(state_var_t *state_buffer_, AnalyzerNodeInfo &info_, OperatorCost cost_type_)
    : state_buffer(state_buffer_), info(info_), cost_type(cost_type_) {
}

State AnalyzerNode::get_state() const {
    return State(state_buffer);
}

std::vector<state_var_t *> AnalyzerNode::get_parent_buffers() const {
    return info.parent_states;
}

// like reopen, except doesn't change status
void AnalyzerNode::add_parent(const AnalyzerNode &parent_node) {
    info.parent_states.push_back(parent_node.state_buffer);
}


void AnalyzerNode::dump() {
    cout << state_buffer << ": ";
    State(state_buffer).dump();
}

class AnalyzerSpace::HashTable
    : public __gnu_cxx::hash_map<StateProxy, AnalyzerNodeInfo> {
    // This is more like a typedef really, but we need a proper class
    // so that we can hide the information in the header file by using
    // a forward declaration. This is also the reason why the hash
    // table is allocated dynamically in the constructor.
};


AnalyzerSpace::AnalyzerSpace(OperatorCost cost_type_)
    : cost_type(cost_type_) {
    nodes = new HashTable;
}

AnalyzerSpace::~AnalyzerSpace() {
    delete nodes;
}

int AnalyzerSpace::size() const {
    return nodes->size();
}

AnalyzerNode AnalyzerSpace::get_node(const State &state) {
    static AnalyzerNodeInfo default_info;
    pair<HashTable::iterator, bool> result = nodes->insert(
        make_pair(StateProxy(&state), default_info));
    if (result.second) {
        // This is a new entry: Must give the state permanent lifetime.
        result.first->first.make_permanent();
    }
    HashTable::iterator iter = result.first;
    return AnalyzerNode(iter->first.state_data, iter->second, cost_type);
}


void AnalyzerSpace::dump() {
    int i = 0;
    for (HashTable::iterator iter = nodes->begin(); iter != nodes->end(); iter++) {
        cout << "#" << i++ << " (" << iter->first.state_data << "): ";
        State(iter->first.state_data).dump();
    }
}

void AnalyzerSpace::statistics() const {
    cout << "Analyzer space hash size: " << nodes->size() << endl;
    cout << "Analyzer space hash bucket count: " << nodes->bucket_count() << endl;
}
