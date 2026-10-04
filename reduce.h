#include "parallel.h"

using namespace parlay;

template <class T>
T reduce_seq(T *A, size_t n) {
	T total = A[0];
	for (size_t i = 1; i < n; i++) {
		total += A[i];
	}
	return total;
}

template <class T>
T reduce(T *A, size_t n, size_t n_seq_max) {
  if (n == 0) {
    return 0;
  } else if (n == 1) {
    return A[0];
  } else if (n > n_seq_max){
    T v1, v2;
    auto f1 = [&]() { v1 = reduce(A, n / 2, n_seq_max); };
    auto f2 = [&]() { v2 = reduce(A + n / 2, n - n / 2, n_seq_max); };
    par_do(f1, f2);
    return v1 + v2;
  } else {
		T sum = reduce_seq(A, n);
		return sum;
	}
}

