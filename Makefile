build:
	@g++ -fopenmp main.cpp -std=c++20 utils/utils.cpp utils/Node.cpp

run: 
	@ OMP_NUM_THREADS=6 ./a.out /data/rbg358.atsp res6

test_threads:
	@for i in 1 2 3 4 5 6 7 8; do \
		echo "Running with $$i threads..."; \
		mkdir -p res$$i; \
		touch res$$i/res.txt; \
		OMP_NUM_THREADS=$$i ./a.out /data/rbg358.atsp res$$i > res$$i/res.txt; \
	done