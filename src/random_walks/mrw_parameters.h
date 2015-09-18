/*********************************************************************
 * Author: Hootan Nakhost (Nakhost@ualberta.ca)
 * (C) Copyright 2009-2013 Hootan Nakhost
 *
 * This file is part of Arvand.
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

#ifndef MRW_PARAMS_H
#define MRW_PARAMS_H

#include <vector>

// default values for MRW parameters
static const double DEFAULT_RL = 0.1;
static const double DEFAULT_RG = 0.001;
static const int DEFAULT_TG = 1000;
static const bool DEFAULT_DNRG = true;
static const bool DEFAULT_DTRG = true;
static const bool DEFAULT_AP = false;
static const bool DEFAULT_AP_RL = true;
static const bool DEFAULT_DTRL = false;
static const int DEFAULT_NJ = 1;
static const double DEFAULT_PE = 1;
static const int DEFAULT_EVAL_LIMIT = -1;
static const int NUM_LR_PARAMS = 4;
static const bool DEFAULT_S_UNUSED = false;
// Adaptive Local Restarting
static const double DEFAULT_EPSILON = 0.1;
// Bias
//static const int DEFUALT_BIAS = 0;
// MHA
static const double DEFUALT_MHA_T = 10;
static const double DEFUALT_MHA_W = 1;
// MDA
static const double DEFUALT_MDA_T = 0.5;
// AP
static const double DEFUALT_ALPHA = 0.9;

enum Bias {UNIFORM = 0, MHA = 1, AGRESSIVE_MHA = 2, MDA = 3};
class MRW_Parameters{
// This class is a container for the parameters used in MRW
public:	

	Bias bias; // determines the biasing
	//int eval_limit;
    double rl; // the local restarting rate
    double rg; // the global restarting rate
    double pe; // evaluation rate
    double alpha; // the parameter alpha used in acceptable_progress
    double mha_temperature;
    double mha_w; // the weight of current preferred operators
    double mda_temperature;
    bool adaptive_gr;
    bool deterministic_gr;
    bool acceptable_progress;
    bool adaptive_lr;
    bool sample_unused; // first samples actions that are not used before
    double epsilon;
    int nj; // number of walks run before each jumping
    int tg;
    // TODO move the adaptive_lr system to another class
    int num_samples;
    int last_rl;
    std::vector<double> rl_candidates;
    std::vector<double> value;
    std::vector<double> cost;
    std::vector<double> num;
    void feedback_rl(int value, int cost);
    void change_rl();
    int random_rl();
    MRW_Parameters();
    void dump();
};

#endif

