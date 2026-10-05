#include "reduce.h"

#include <cstdlib>
#include <iostream>
#include <cstring>
#include <cmath>

#include "get_time.h"

using Type = long long;

int main(int argc, char* argv[]) {

  size_t n = 0; // default 1e9
  int num_rounds = 0; // default 3
	bool seq = false;
	size_t n_seq_max = 0; 
	int iteration_count = 0;

	for (int argi = 1; argi < argc; argi++){
		if (strcmp(argv[argi], "-s") == 0){
			seq = true;
			if (n_seq_max || iteration_count > 0) {
				fprintf(stderr, "Cannot set sequential only mode while specifying parallel execution threshold\n");
				return 1;
			}
			continue;
		}
		if (strcmp(argv[argi], "-t") == 0){
			if (argi == (argc - 1)) {
				printf("Specify parallel execution input size threshold\n");	
				return 1;
			}
			if (seq) {
				fprintf(stderr, "Cannot set sequential only mode while specifying parallel execution threshold\n");
				return 1;
			}
			argi += 1;
			n_seq_max = atoll(argv[argi]);
			continue;
		}
		if (strcmp(argv[argi], "-i") == 0){
			if (argi == (argc - 1)) {
				printf("Specify max iteration count for binary search\n");
				return 1;
			}
			if (seq) {
				fprintf(stderr, "Cannot set sequential only mode while specifying parallel execution threshold\n");
				return 1;
			}
			argi += 1;
			iteration_count = atoi(argv[argi]); 
			continue;
		}
		if (strcmp(argv[argi], "-r") == 0){
			if (argi == (argc - 1)) {
				fprintf(stderr, "Specify the number of rounds\n");
				return 1;
			}
			argi += 1;
			num_rounds = atoi(argv[argi]);
			continue;
		}
		if (n == 0) {
			n = atoll(argv[argi]);
			continue;
		}
		if (num_rounds == 0){
			num_rounds = atoi(argv[argi]);
			continue;
		}
	}
	
  n = (n == 0) ? 1e9 : n;
	num_rounds = (num_rounds == 0) ? 3 : num_rounds;	
	seq = (n_seq_max > n) ? true : seq;
  printf("n = %ld, num_rounds = %d, n_seq_max = %ld, iterations = %d\n", n, num_rounds, n_seq_max, iteration_count);	
  Type* A = (Type*)malloc(n * sizeof(Type));
  parallel_for(0, n, [&](size_t i) { A[i] = i; });
	double total_time = 0;
	int test_n_seq_max = n / 2;
	int min_step = 100;
	for (int iteration = 0; iteration < iteration_count; iteration++){
		printf("(%d/%d) n_seq_max: %d ", iteration + 1, iteration_count, test_n_seq_max);
		int step_size = n / (std::pow(2, iteration + 2));
		double duration_l = 0;
		for (int i = 0; i < num_rounds; i++) {
			parlay::timer t;
			reduce(A, n, test_n_seq_max - step_size);
			double duration = t.total_time();
			duration_l += duration;
		}
		duration_l = duration_l / (num_rounds > 0 ? num_rounds : 1);

		double duration_r = 0;
		for (int i = 0; i < num_rounds; i++) {
			parlay::timer t;
			reduce(A, n, test_n_seq_max + step_size);
			double duration = t.total_time();
			duration_r += duration;
		}
		duration_r = duration_r / (num_rounds > 0 ? num_rounds : 1);

		if (duration_l < duration_r) {
			test_n_seq_max -= step_size;
		} else {
			test_n_seq_max += step_size;
		}

		printf("t: %f, l: %f, r: %f step: %d\n", (duration_l < duration_r) ? duration_l : duration_r, duration_l, duration_r, step_size); 
		if (step_size < min_step) {
			break;
		}	
	}

	n_seq_max = test_n_seq_max;
	const char* mode = seq ? " sequential execution" : " parallel execution";
	const char* threshold = (n_seq_max > 0 && !seq) ? " with sequential execution where n < n_seq_max" : "";
		
  for (int i = 0; i <= num_rounds; i++) {
    parlay::timer t;
    long long ans = seq ? reduce_seq(A, n) : reduce(A, n, n_seq_max);
    t.stop();

    if (i == 0) {
      std::cout << "Total sum (while using" << mode  << threshold << "): " << ans << std::endl;
      std::cout << "Warmup round running time: " << t.total_time() << std::endl;
    } else {
      std::cout << "Round " << i << " running time: " << t.total_time()
                << std::endl;
      total_time += t.total_time();
    }
  }
  std::cout << "Average running time: " << total_time / num_rounds << std::endl;

  free(A);
  return 0;
}
