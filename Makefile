build:
	@g++ -fopenmp main.cpp -std=c++20 utils/utils.cpp utils/Node.cpp

run: 
	@ OMP_NUM_THREADS=6 ./a.out /data/rbg358.atsp res6

DATASETS = rbg358 ftv38 ft70 br17
ITERATIONS = 10000 25000 50000 100000

test_all:
	@for dataset in $(DATASETS); do \
		echo "Testing dataset: $$dataset"; \
		for threads in 1 2 4 8; do \
			for iter in $(ITERATIONS); do \
				echo "Running with $$threads threads and $$iter iterations..."; \
				mkdir -p res$$threads/$$dataset/iter_$$iter; \
				touch res$$threads/$$dataset/iter_$$iter/res.txt; \
				OMP_NUM_THREADS=$$threads ./a.out /data/$$dataset.atsp res$$threads/$$dataset/iter_$$iter $$iter > res$$threads/$$dataset/iter_$$iter/res.txt; \
			done \
		done \
	done

REPEATS = 5

test_all_avg:
	@for dataset in $(DATASETS); do \
		echo "Testing dataset: $$dataset"; \
		for threads in 1 2 4 8; do \
			for iter in $(ITERATIONS); do \
				echo "Running with $$threads threads and $$iter iterations..."; \
				mkdir -p res$$threads/$$dataset/iter_$$iter; \
				> res$$threads/$$dataset/iter_$$iter/results.txt; \
				for run in $$(seq 1 $(REPEATS)); do \
					echo "  Run $$run"; \
					start_time=$$(date +%s%3N); \
					OMP_NUM_THREADS=$$threads ./a.out /data/$$dataset.atsp res$$threads/$$dataset/iter_$$iter $$iter > tmp_output.txt; \
					end_time=$$(date +%s%3N); \
					duration=$$((end_time - start_time)); \
					# Wydobywamy wynik końcowy z outputu, np. ostatnia linijka z kosztem \
					final_cost=$$(tail -n 1 tmp_output.txt); \
					echo "$$duration $$final_cost" >> res$$threads/$$dataset/iter_$$iter/results.txt; \
				done; \
				rm tmp_output.txt; \
				# Liczymy średnią i zapisujemy do summary.txt \
				awk '{sum_time+=$$1; sum_cost+=$$2; count+=1} END {printf "Average time: %.2f ms, Average cost: %.2f\n", sum_time/count, sum_cost/count}' res$$threads/$$dataset/iter_$$iter/results.txt > res$$threads/$$dataset/iter_$$iter/summary.txt; \
			done; \
		done; \
	done