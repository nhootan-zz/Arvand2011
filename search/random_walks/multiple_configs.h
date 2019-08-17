
/*********************************************************************
 * Author: Hootan Nakhost (Nakhost@ualberta.ca)
 * (C) Copyright 2009-2013 Hootan Nakhost
 *
 * This file is part of Arvand. It is a modified version of iterated_search.h
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

#ifndef MULTIPLE_CONFIGS_H
#define MULTIPLE_CONFIGS_H

#include "../globals.h"
#include "../option_parser.h"
#include "../search_engine.h"
#include "mrw.h"

class Options;

class MultipleConfigsSearch : public SearchEngine {
  const vector<ParseTree> engine_configs;
  vector<MRW *> engines;
  SearchEngine *get_search_engine(int engine_config_start_index);
  virtual void initialize();
  virtual int step();

 public:
  MultipleConfigsSearch(const Options &opts);
  virtual void set_bound(int b);
  virtual ~MultipleConfigsSearch();
  void statistics() const;
};

#endif
