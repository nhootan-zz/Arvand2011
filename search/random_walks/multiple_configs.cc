/*********************************************************************
 * Author: Hootan Nakhost (Nakhost@ualberta.ca)
 * (C) Copyright 2009-2013 Hootan Nakhost
 *
 * This file is part of Arvand. It is a modified version of iterated_search.cc
 * from FD code base.
 *
 * Arvand is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 3
 * of the license, or (at your option) any later version.
 *
 * Arvand is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 *********************************************************************/

#include "multiple_configs.h"

#include <limits>

#include "../ext/tree_util.hh"
#include "../plugin.h"
#include "../rng.h"
#include "mrw.h"

void MultipleConfigsSearch::set_bound(int b) {
  bound = b;
  for (int i = 0; i < engines.size(); ++i) {
    engines[i]->set_bound(b);
  }
}

MultipleConfigsSearch::MultipleConfigsSearch(const Options &opts)
    : SearchEngine(opts),
      engine_configs(opts.get_list<ParseTree>("engine_configs")) {
  for (int i = 0; i < engine_configs.size(); ++i) {
    engines.push_back((MRW *)get_search_engine(i));
    engines.back()->initialize();
  }
}

MultipleConfigsSearch::~MultipleConfigsSearch() {}

void MultipleConfigsSearch::initialize() {}

SearchEngine *MultipleConfigsSearch::get_search_engine(
    int engine_configs_index) {
  OptionParser parser(engine_configs[engine_configs_index], false);
  cout << "config #" << engine_configs_index << ":" << endl;
  SearchEngine *engine = parser.start_parsing<SearchEngine *>();
  kptree::print_tree_bracketed(engine_configs[engine_configs_index], cout);
  cout << endl;
  return engine;
}

int MultipleConfigsSearch::step() {
  int search_status = IN_PROGRESS;
  int index = -1;
  do {
    // randomly select one of the engines
    index = g_rng.next32() % engines.size();
    cout << "config #" << index << " is selected" << endl;
    search_status = engines[index]->search_episode();
  } while (search_status == IN_PROGRESS);
  if (search_status == SOLVED) {
    set_plan(engines[index]->get_plan());
  }
  return search_status;
}

void MultipleConfigsSearch::statistics() const {}

static SearchEngine *_parse(OptionParser &parser) {
  parser.add_list_option<ParseTree>("engine_configs", "");
  SearchEngine::add_options_to_parser(parser);
  Options opts = parser.parse();
  opts.verify_list_non_empty<ParseTree>("engine_configs");
  if (parser.help_mode()) {
    return 0;
  } else if (parser.dry_run()) {
    // check if the supplied search engines can be parsed
    vector<ParseTree> configs = opts.get_list<ParseTree>("engine_configs");
    for (size_t i(0); i != configs.size(); ++i) {
      OptionParser test_parser(configs[i], true);
      test_parser.start_parsing<SearchEngine *>();
    }
    return 0;
  } else {
    MultipleConfigsSearch *engine = new MultipleConfigsSearch(opts);
    return engine;
  }
}

static Plugin<SearchEngine> _plugin("multi_configs", _parse);
