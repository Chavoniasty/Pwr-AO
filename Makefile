build:
	@g++ -fopenmp main.cpp -std=c++20 utils/utils.cpp utils/Node.cpp

run: 
	@./a.out /data/rbg358.atsp res