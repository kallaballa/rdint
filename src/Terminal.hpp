/*
 * Terminal.hpp
 *
 *  Created on: Apr 11, 2021
 *      Author: elchaschab
 */

#ifndef SRC_TERMINAL_HPP_
#define SRC_TERMINAL_HPP_

#include <string>

using std::string;

enum TERM_COLORS {
	BLACK = 30,
	RED = 31,
	GREEN = 32,
	YELLOW = 33,
	BLUE = 34,
	PINK = 35,
	CYAN = 36,
	WHITE = 37,
	NORMAL = 39
};

string make_color(const string& s, const TERM_COLORS& c);
string make_bold(const string& s);

#endif /* SRC_TERMINAL_HPP_ */
