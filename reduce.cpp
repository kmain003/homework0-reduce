#include "reduce.h"

#include <cstdlib>
#include <iostream>
#include <cstring>

#include "get_time.h"

using Type = long long;

int main(int argc, char* argv[]) {

  size_t n = 0; // default 1e9
  int num_rounds = 0; // default 3
	bool seq = false;
	size_t n_seq_max = 0; 

	for (int argi = 1; argi < argc; argi++){
		if (strcmp(argv[argi], "-s") == 0){
			seq = true;
			if (n_seq_max) {
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
			n_seq_max = atoll(argv[argi + 1]);
			argi += 1;
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
  printf("n = %ld, num_rounds = %d, n_seq_max = %ld\n", n, num_rounds, n_seq_max);	
  Type* A = (Type*)malloc(n * sizeof(Type));
  parallel_for(0, n, [&](size_t i) { A[i] = i; });
	double total_time = 0;
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
