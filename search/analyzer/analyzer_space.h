#ifndef ANALYZER_SPACE_H
#define ANALYZER_SPACE_H

#include <cassert>
#include <ext/hash_map>

#include "../operator_cost.h"
#include "../state.h"  // for state_var_t
#include "../state_proxy.h"
#include "analyzer_node_info.h"

class Operator;
class State;
class StateProxy;

class AnalyzerNode {
  state_var_t *state_buffer;
  AnalyzerNodeInfo &info;
  OperatorCost cost_type;

 public:
  AnalyzerNode(state_var_t *state_buffer_, AnalyzerNodeInfo &info_,
               OperatorCost cost_type_);

  state_var_t *get_state_buffer() { return state_buffer; }

  State get_state() const;
  bool is_bwd_new() const { return info.bwd_status == AnalyzerNodeInfo::NEW; }
  bool is_fwd_open() const { return info.fwd_status == AnalyzerNodeInfo::OPEN; }
  bool is_fwd_new() const { return info.fwd_status == AnalyzerNodeInfo::NEW; }
  bool is_bwd_closed() const {
    return info.bwd_status == AnalyzerNodeInfo::CLOSED;
  }
  bool is_fwd_closed() const {
    return info.fwd_status == AnalyzerNodeInfo::CLOSED;
  }
  bool is_dead_end() const {
    return info.fwd_status == AnalyzerNodeInfo::DEAD_END;
  }
  void set_d(int d) { info.d = d; }

  int get_d() const { return info.d; }

  void set_h(int h) { info.h = h; }

  int get_h() const { return info.h; }

  void dead_end() {
    assert(info.fwd_status == AnalyzerNodeInfo::OPEN);
    info.fwd_status = AnalyzerNodeInfo::DEAD_END;
  }

  void bwd_close() {
    assert(info.bwd_status == AnalyzerNodeInfo::OPEN);
    info.bwd_status = AnalyzerNodeInfo::CLOSED;
  }
  void fwd_close() {
    assert(info.fwd_status == AnalyzerNodeInfo::OPEN ||
           info.fwd_status == AnalyzerNodeInfo::DEAD_END);
    info.fwd_status = AnalyzerNodeInfo::CLOSED;
  }
  void bwd_open() {
    assert(info.bwd_status == AnalyzerNodeInfo::NEW);
    info.bwd_status = AnalyzerNodeInfo::OPEN;
  }
  void fwd_open() {
    assert(info.fwd_status == AnalyzerNodeInfo::NEW);
    info.fwd_status = AnalyzerNodeInfo::OPEN;
  }

  vector<state_var_t *> get_parent_buffers() const;

  void add_parent(const AnalyzerNode &parent_node);
  void dump();
};

class AnalyzerSpace {
  class HashTable;
  HashTable *nodes;
  OperatorCost cost_type;

 public:
  AnalyzerSpace(OperatorCost cost_type_);
  ~AnalyzerSpace();
  int size() const;
  AnalyzerNode get_node(const State &state);

  void dump();
  void statistics() const;
};

#endif
