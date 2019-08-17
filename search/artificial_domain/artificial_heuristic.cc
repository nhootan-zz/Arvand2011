#include "artificial_heuristic.h"

#include <cassert>

#include "../globals.h"
#include "../heuristic.h"
#include "../plugin.h"
#include "../rng.h"
#include "../simulator.h"
#include "../state.h"

ArtificialHeuristic::ArtificialHeuristic(const Options &opts)
    : Heuristic(opts) {
  deviation_factor = opts.get<double>("h_deviation_factor");
  rc = opts.get<double>("rc");
  cout << "Initializing artificial heuristic..." << endl;
  cout << "deviation factor: " << deviation_factor << endl;
  cout << "rc: " << rc << endl;
  g_simulator->set_heuristic(this);
}

ArtificialHeuristic::~ArtificialHeuristic() {}

int ArtificialHeuristic::compute_heuristic(int parent_h, int d) {
  double r = g_rng.next_half_open();
  double lower_bound = -1;
  if (parent_h != -1 && r < rc) {
    // the heuristic should be consistent
    lower_bound = parent_h - 1;
  }
  // choose random value between g_heuristic_deviation_limit * distance
  // and distance
  double lower = max(deviation_factor * d, lower_bound);
  double upper = d;
  assert(upper >= lower);
  double result = lower + g_rng.next_closed() * (upper - lower);
  assert(result >= lower && result <= upper);
  int h = (int)result;
  /*if(parent_h != -1 && parent_h > h + 1){
      cout << "h(parent(s)): " << parent_h << " h(s): " << h << endl;
      exit(1);
  }*/
  return h;
}

int ArtificialHeuristic::compute_heuristic(const State &state) {
  // the first variable in the state determines the distance
  int d = state[0];
  // the second variable in the state determines the precomputed heuristic
  int h = state[2];

  if (h != -1)
    return h;
  else
    return compute_heuristic(-1, d);
}

static ScalarEvaluator *_parse(OptionParser &parser) {
  parser.add_option<double>("h_deviation_factor", DEFAULT_HD,
                            "the heuristic deviation factor");
  parser.add_option<double>("rc", DEFAULT_RC, "the heuristic consistency rate");
  Heuristic::add_options_to_parser(parser);
  Options opts = parser.parse();

  if (parser.dry_run())
    return 0;
  else
    return new ArtificialHeuristic(opts);
}

static Plugin<ScalarEvaluator> _plugin("artificial", _parse);
