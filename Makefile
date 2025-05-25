build:
	@g++ -fopenmp main.cpp -std=c++20 -ljsoncpp utils/utils.cpp utils/Node.cpp

run: 
	@./a.out /data/berlin52.tsp res

test: 
	@./a.out data/20nodes.json res