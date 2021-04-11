/*
 * Terminal.cpp
 *
 *  Created on: Apr 11, 2021
 *      Author: elchaschab
 */

#include "Terminal.hpp"

string make_color(const string& s, const TERM_COLORS& c) {
	  return "\033[0;" + std::to_string(c) + "m" + s + "\033[0;39m";
}

string make_bold(const string& s) {
	  return "\033[1m" + s + "\033[0m";
}
