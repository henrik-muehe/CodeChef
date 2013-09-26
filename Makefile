all: solution

solution: solution.cpp
	g++ -g -O3 -std=c++11 -o $@ $<
