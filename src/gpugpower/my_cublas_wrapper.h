/*
 *
 * This is a parallel sparse PCA solver
 *
 * The solver is based on a simple alternating maximization (AM) subroutine 
 * and is based on the paper
 *    P. Richtarik, M. Takac and S. Damla Ahipasaoglu 
 *    "Alternating Maximization: Unifying Framework for 8 Sparse PCA Formulations and Efficient Parallel Codes"
 *
 * The code is available at https://code.google.com/p/24am/
 * under GNU GPL v3 License
 * 
 */


#ifndef MY_CUBLAS_WRAPPER_H_
#define MY_CUBLAS_WRAPPER_H_

#include <cuda.h>
#include "cublas_v2.h"
#include <thrust/random.h>

#include "../ggks/bucketSelect.h"

const float FLOAT_ZERO = 0;
const double DOUBLE_ZERO = 0;
const int INT_ONE = 1;

__device__
int gpu_sgn(float val) {
	return ((val > 0) - (val < 0));
}

__device__
int gpu_sgn(double val) {
	return ((val > 0) - (val < 0));
}

//__device__
//int gpu_abs(float val) {
//	return ((val > 0) ? val : -val);
//}
//
//__device__
//int gpu_abs(double val) {
//	return ((val > 0) ? val : -val);
//}

template<typename F>
void gpu_computeNorm(cublasHandle_t &handle, F* d_z, int n, F &result) {
}

template<>
void gpu_computeNorm(cublasHandle_t &handle, float* d_z, int n, float &result) {
	cublasSnrm2(handle, n, d_z, 1, &result);
}

template<>
void gpu_computeNorm(cublasHandle_t &handle, double* d_z, int n, double &result) {
	cublasDnrm2(handle, n, d_z, 1, &result);
}

template<typename F>
void gpu_compute_l1_Norm(cublasHandle_t &handle, F* d_z, int n, F &result) {
}

template<>
void gpu_compute_l1_Norm(cublasHandle_t &handle, float* d_z, int n,
		float &result) {
	cublasSasum(handle, n, d_z, 1, &result);
}
template<>
void gpu_compute_l1_Norm(cublasHandle_t &handle, double* d_z, int n,
		double &result) {
	cublasDasum(handle, n, d_z, 1, &result);
}

template<typename F>
void gpu_matrix_vector_multiply(cublasHandle_t &handle,
		const cublasOperation_t trans, const F alpha, const int m, const int n,
		const F* matrix, F* vector, F* result) {
}

template<>
void gpu_matrix_vector_multiply(cublasHandle_t &handle,
		const cublasOperation_t trans, const float alpha, const int m,
		const int n, const float* matrix, float* vector, float* result) {
	cublasSgemv(handle, trans, m, n, &alpha, matrix, m, vector, 1, &FLOAT_ZERO,
			result, 1);
}

template<>
void gpu_matrix_vector_multiply(cublasHandle_t &handle,
		const cublasOperation_t trans, const double alpha, const int m,
		const int n, const double* matrix, double* vector, double* result) {
	cublasDgemv(handle, trans, m, n, &alpha, matrix, m, vector, 1,
			&DOUBLE_ZERO, result, 1);
}

template<typename F>
void gpu_matrix_matrix_multiply(cublasHandle_t &handle,
		const cublasOperation_t trans, const F alpha, const int m, const int n,
		const F* matrix, F* vectorMatrix, F* result, const int experiments,
		const int LDM, const int LDN) {
}

template<>
void gpu_matrix_matrix_multiply(cublasHandle_t &handle,
		const cublasOperation_t trans, const float alpha, const int m,
		const int n, const float* matrix, float* vectorMatrix, float* result,
		const int experiments, const int LDM, const int LDN) {
	if (trans == CUBLAS_OP_N) {
		cublasSgemm(handle, trans, CUBLAS_OP_N, m, experiments, n, &alpha,
				matrix, LDM, vectorMatrix, LDN, &FLOAT_ZERO, result, LDM);
	} else {
		cublasSgemm(handle, trans, CUBLAS_OP_N, n, experiments, m, &alpha,
				matrix, LDM, vectorMatrix, LDM, &FLOAT_ZERO, result, LDN);
	}
}

template<>
void gpu_matrix_matrix_multiply(cublasHandle_t &handle,
		const cublasOperation_t trans, const double alpha, const int m,
		const int n, const double* matrix, double* vectorMatrix,
		double* result, const int experiments, const int LDM, const int LDN) {
	if (trans == CUBLAS_OP_N) {
		cublasDgemm(handle, trans, CUBLAS_OP_N, m, experiments, n, &alpha,
				matrix, LDM, vectorMatrix, LDN, &DOUBLE_ZERO, result, LDM);
	} else {
		cublasDgemm(handle, trans, CUBLAS_OP_N, n, experiments, m, &alpha,
				matrix, LDM, vectorMatrix, LDM, &DOUBLE_ZERO, result, LDN);
	}
}

template<typename F>
void gpu_vector_scale(cublasHandle_t &handle, F* d_z, const unsigned int n,
		const F alpha) {
	cout << "ERROR in vector scale!"<<endl;
}

template<>
void gpu_vector_scale(cublasHandle_t &handle, float* d_z, const unsigned int n,
		const float alpha) {
	cublasSscal(handle, n, &alpha, d_z, INT_ONE);
}

template<>
void gpu_vector_scale(cublasHandle_t &handle, double* d_z,
		const unsigned int n, const double alpha) {
	cublasDscal(handle, n, &alpha, d_z, INT_ONE);
}

template<typename F>
void gpu_vector_copy(cublasHandle_t &handle, F* from, const unsigned int n,
		F* to) {
}

template<>
void gpu_vector_copy(cublasHandle_t &handle, double* from,
		const unsigned int n, double* to) {
	cublasDcopy(handle, n, from, 1, to, 1);
}

template<>
void gpu_vector_copy(cublasHandle_t &handle, float* from, const unsigned int n,
		float* to) {
	cublasScopy(handle, n, from, 1, to, 1);
}

template<typename F>
struct gpu_l0_penalized_tresholing {
	F penaltyParameter;
	gpu_l0_penalized_tresholing(F _penaltyParameter) {
		penaltyParameter = _penaltyParameter;
	}

	__host__                                          __device__
	F operator()(F x) {
		if (x * x - penaltyParameter <= 0) {
			return 0;
		} else {
			return x;
		}
	}
};

template<typename F>
struct gpu_l0_penalized_objval {
	F penaltyParameter;
	gpu_l0_penalized_objval(F _penaltyParameter) {
		penaltyParameter = _penaltyParameter;
	}

	__host__                                          __device__
	F operator()(F x) {
		F tmp = x * x - penaltyParameter;
		if (tmp <= 0) {
			return 0;
		} else {
			return tmp;
		}
	}
};

template<typename F>
struct gpu_l1_penalized_tresholing {
	F penaltyParameter;
	gpu_l1_penalized_tresholing(F _penaltyParameter) {
		penaltyParameter = _penaltyParameter;
	}
	__host__                                          __device__
	F operator()(F x) {
		F tmp = abs(x) - penaltyParameter;
		if (tmp > 0) {
			return tmp * gpu_sgn(x);
		} else {
			return 0;
		}
	}
};

template<typename F>
struct gpu_l1_penalized_objval {
	F penaltyParameter;
	gpu_l1_penalized_objval(F _penaltyParameter) {
		penaltyParameter = _penaltyParameter;
	}

	__host__                                          __device__
	F operator()(F x) {
		F tmp = abs(x) - penaltyParameter;
		if (tmp >= 0) {
			return tmp * tmp;
		} else {
			return 0;
		}
	}
};

template<typename F>
struct gpu_hard_treshholding {
	F treshhold;
	gpu_hard_treshholding(F _treshhold) {
		treshhold = _treshhold;
	}

	__host__                                          __device__
	F operator()(F x) {
		if (abs(x) < treshhold) {
			return 0;
		} else {
			return x;
		}
	}
};

template<typename F>
struct gpu_sgn_transformator {
	__host__                                          __device__
	F operator()(F x) {
		return gpu_sgn(x);
	}
};

template<typename F>
struct comparator_abs_val: public binary_function<F, F, bool> {
	__host__                                          __device__
	bool operator()(const F &lhs, const F &rhs) {
		F a = abs(lhs);
		F b = abs(rhs);
		return a > b;
	}
};

struct comparator_abs_val_for_tuple_float: public binary_function<
		thrust::tuple<float, int>, thrust::tuple<float, int>, bool> {
	__host__                                          __device__
	bool operator()(const thrust::tuple<float, int> &lhs,
			const thrust::tuple<float, int> &rhs) {
		int ai = thrust::get<1>(lhs);
		int bi = thrust::get<1>(rhs);
		if (ai != bi)
			return ai < bi;
		float a = abs(thrust::get<0>(lhs));
		float b = abs(thrust::get<0>(rhs));
		return a > b;
	}
};

struct comparator_abs_val_for_tuple_double: public binary_function<
		thrust::tuple<double, int>, thrust::tuple<double, int>, bool> {
	__host__                                          __device__
	bool operator()(const thrust::tuple<double, int> &lhs,
			const thrust::tuple<double, int> &rhs) {
		int ai = thrust::get<1>(lhs);
		int bi = thrust::get<1>(rhs);
		if (ai != bi)
			return ai < bi;
		float a = abs(thrust::get<0>(lhs));
		float b = abs(thrust::get<0>(rhs));
		return a > b;
	}
};

struct comparator_abs_val_for_tuple_with_sequence: public binary_function<
		thrust::tuple<float, int>, thrust::tuple<float, int>, bool> {
	float LDN;
	comparator_abs_val_for_tuple_with_sequence(int _LDN) {
		LDN = (float) _LDN;
	}
	__host__                                          __device__
	bool operator()(const thrust::tuple<float, int> &lhs,
			const thrust::tuple<float, int> &rhs) {
		int ai = (int) (floor(thrust::get<1>(lhs) / LDN));
		int bi = (int) (floor(thrust::get<1>(rhs) / LDN));
		if (ai != bi)
			return ai < bi;
		float a = abs(thrust::get<0>(lhs));
		float b = abs(thrust::get<0>(rhs));
		return a > b;
	}
};

struct init_sequence_with_LDN {
	int LDN;
	init_sequence_with_LDN(int _LDN) {
		LDN = _LDN;
	}
	__host__                                          __device__
	int operator()(int x) {
		return x / LDN;
	}
};

template<typename F>
F compute_soft_thresholding_parameter(F * myvector, const unsigned int length,
		const unsigned int constrain) {
	F w = sqrt(constrain);
	F lambda_Low = 0;
	F lambda_High = abs(myvector[0]);//lambda = \|x\|_\infty
	F sum_abs_x = 0;
	F sum_abs_x2 = 0;
	F tmp = abs(myvector[0]);
	F subgrad = 0;
	int total_elements = 0;
	for (unsigned int i = 0; i < length; i++) {
		tmp = abs(myvector[i]);
		sum_abs_x += tmp;
		sum_abs_x2 += tmp * tmp;
		total_elements++;
		//compute subgradient close to lambda=x_{(i)}
		lambda_Low = tmp;
		subgrad = (sum_abs_x - total_elements * tmp) / sqrt(
				sum_abs_x2 - 2 * tmp * sum_abs_x + total_elements * tmp * tmp);
		if (subgrad >= w) {
			sum_abs_x -= tmp;
			sum_abs_x2 -= tmp * tmp;
			total_elements--;
			break;
		}
		lambda_High = tmp;
	}

	// solve quadratic
	F a = total_elements * (total_elements - w * w);
	F b = 2 * sum_abs_x * w * w - 2 * total_elements * sum_abs_x;
	F c = sum_abs_x * sum_abs_x - w * w * sum_abs_x2;
	w = (-b - sqrt(b * b - 4 * a * c)) / (2 * a);

	if (w < lambda_Low || w > lambda_High) {
		printf("Problem detected! ID%d   %f < %f < %f\n", total_elements,
				lambda_Low, w, lambda_High);
	}
	return w;
}

template<typename F>
struct gpu_soft_treshholding {
	F treshhold;
	gpu_soft_treshholding(F _treshhold) {
		treshhold = _treshhold;
	}

	__host__                                          __device__
	F operator()(F x) {

		F tmp2 = abs(x) - treshhold;
		if (tmp2 > 0) {
			return tmp2 * gpu_sgn(x);
		} else {
			return 0;
		}

	}
};

template<typename F>
struct generate_random_number {
	int optimizationStatisticse;
	generate_random_number(int _optimizationStatisticse) {
		optimizationStatisticse = _optimizationStatisticse;
	}

	__host__                          __device__
	F operator()(F index) {
		thrust::default_random_engine rng(optimizationStatisticse);
		// skip past numbers used in previous threads
		rng.discard((int) index);
		return (F) (rng() / (thrust::default_random_engine::max + 0.0));
	}
};

template<typename F>
void perform_hard_and_soft_thresholdingNEW(thrust::device_vector<F> &d_V,
		OptimizationSettings* optimizationSettings, const unsigned int n,
		thrust::host_vector<F>& h_x, const unsigned int LDN,
		thrust::device_vector<int> &d_IDX, thrust::device_vector<F> &dataToSort) {
	cout << "ERROR IN TEMPLATES"<<endl;
}

template<>
void perform_hard_and_soft_thresholdingNEW(thrust::device_vector<float> &d_V,
		OptimizationSettings* optimizationSettings, const unsigned int n,
		thrust::host_vector<float>& h_x, const unsigned int LDN,
		thrust::device_vector<int> &d_IDX,
		thrust::device_vector<float> &dataToSort) {
	thrust::device_vector<float>::iterator it_begin = d_V.begin();
	thrust::copy(d_V.begin(), d_V.end(), dataToSort.begin());
	//	 sort data on the device
	thrust::sort(
			make_zip_iterator(make_tuple(dataToSort.begin(), d_IDX.begin())),
			make_zip_iterator(make_tuple(dataToSort.end(), d_IDX.end())),
			comparator_abs_val_for_tuple_float());// maybe use stable_sort ???
	//	thrust::counting_iterator<int> indices_begin(0);
	//	thrust::counting_iterator<int> indices_end(d_IDX.size());
	//	// sort data on the device
	//	thrust::sort(
	//			make_zip_iterator(make_tuple(dataToSort.begin(), indices_begin)),
	//			make_zip_iterator(make_tuple(dataToSort.end(), indices_end)),
	//			comparator_abs_val_for_tuple_with_sequence(LDN));// maybe use stable_sort ???
	for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
		// copy data to x_for_sort
		if (i > 0)
			thrust::advance(it_begin, LDN);
		thrust::device_vector<float>::iterator it_end = it_begin + n;
		if (optimizationSettings->isL1ConstrainedProblem()) {
			thrust::device_vector<float>::iterator it_sorted =
					dataToSort.begin();
			thrust::advance(it_sorted, LDN * i);
			thrust::copy(it_sorted, it_sorted + n, h_x.begin());
			float tresh_hold = compute_soft_thresholding_parameter(&h_x[0], n,
					optimizationSettings->constraintParameter);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_soft_treshholding<float> (tresh_hold));
		} else {
			float tresh_hold = abs(
					dataToSort[i * LDN + optimizationSettings->constraintParameter - 1]);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_hard_treshholding<float> (tresh_hold));
		}
	}
}

template<>
void perform_hard_and_soft_thresholdingNEW(thrust::device_vector<double> &d_V,
		OptimizationSettings* optimizationSettings, const unsigned int n,
		thrust::host_vector<double>& h_x, const unsigned int LDN,
		thrust::device_vector<int> &d_IDX,
		thrust::device_vector<double> &dataToSort) {
	thrust::device_vector<double>::iterator it_begin = d_V.begin();
	thrust::copy(d_V.begin(), d_V.end(), dataToSort.begin());
	//	 sort data on the device
	thrust::sort(
			make_zip_iterator(make_tuple(dataToSort.begin(), d_IDX.begin())),
			make_zip_iterator(make_tuple(dataToSort.end(), d_IDX.end())),
			comparator_abs_val_for_tuple_double());// maybe use stable_sort ???
	//	thrust::counting_iterator<int> indices_begin(0);
	//	thrust::counting_iterator<int> indices_end(d_IDX.size());
	//	// sort data on the device
	//	thrust::sort(
	//			make_zip_iterator(make_tuple(dataToSort.begin(), indices_begin)),
	//			make_zip_iterator(make_tuple(dataToSort.end(), indices_end)),
	//			comparator_abs_val_for_tuple_with_sequence(LDN));// maybe use stable_sort ???
	for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
		// copy data to x_for_sort
		if (i > 0)
			thrust::advance(it_begin, LDN);
		thrust::device_vector<double>::iterator it_end = it_begin + n;
		if (optimizationSettings->isL1ConstrainedProblem()) {
			thrust::device_vector<double>::iterator it_sorted =
					dataToSort.begin();
			thrust::advance(it_sorted, LDN * i);
			thrust::copy(it_sorted, it_sorted + n, h_x.begin());
			double tresh_hold = compute_soft_thresholding_parameter(&h_x[0], n,
					optimizationSettings->constraintParameter);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_soft_treshholding<double> (tresh_hold));
		} else {
			double tresh_hold = abs(
					dataToSort[i * LDN + optimizationSettings->constraintParameter - 1]);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_hard_treshholding<double> (tresh_hold));
		}
	}
}

template<typename F>
void perform_hard_and_soft_thresholding(thrust::device_vector<F> &d_V,
		OptimizationSettings* optimizationSettings, const unsigned int n,
		thrust::device_vector<F>& d_x_for_sort, thrust::host_vector<F>& h_x,
		const unsigned int LDN) {

	if (optimizationSettings->isL1ConstrainedProblem()
			|| !optimizationSettings->useKSelectionAlgorithmGPU) {
		perform_hard_and_soft_thresholding_with_sorting(d_V, optimizationSettings, n,
				d_x_for_sort, h_x, LDN);
	} else {
		perform_hard_thresholding_with_k_selection(d_V, optimizationSettings, n,
				d_x_for_sort, h_x, LDN);
	}

}
//===========================================================================
template<typename F>
void perform_hard_thresholding_with_k_selection(thrust::device_vector<F> &d_V,
		OptimizationSettings* optimizationSettings, const unsigned int n,
		thrust::device_vector<F>& d_x_for_sort, thrust::host_vector<F>& h_x,
		const unsigned int LDN) {

}

template<>
void perform_hard_thresholding_with_k_selection(
		thrust::device_vector<float> &d_V, OptimizationSettings* optimizationSettings,
		const unsigned int n, thrust::device_vector<float>& d_x_for_sort,
		thrust::host_vector<float>& h_x, const unsigned int LDN) {
	thrust::device_vector<float>::iterator it_begin = d_V.begin();
	float * V = thrust::raw_pointer_cast(&d_V[0]);
	for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
		if (i > 0)
			thrust::advance(it_begin, LDN);
		thrust::device_vector<float>::iterator it_end = it_begin + n;
		float tresh_hold = BucketSelect::bucketSelectWrapper(&V[i*LDN], n, optimizationSettings->constraintParameter,
				optimizationSettings->gpu_sm_count, optimizationSettings->gpu_max_threads);
		thrust::transform(it_begin, it_end, it_begin,
				gpu_hard_treshholding<float> (tresh_hold));
	}
}

template<>
void perform_hard_thresholding_with_k_selection(
		thrust::device_vector<double> &d_V, OptimizationSettings* optimizationSettings,
		const unsigned int n, thrust::device_vector<double>& d_x_for_sort,
		thrust::host_vector<double>& h_x, const unsigned int LDN) {
	thrust::device_vector<double>::iterator it_begin = d_V.begin();
	double * V = thrust::raw_pointer_cast(&d_V[0]);
	for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
		if (i > 0)
			thrust::advance(it_begin, LDN);
		thrust::device_vector<double>::iterator it_end = it_begin + n;
		double tresh_hold = BucketSelect::bucketSelectWrapper(&V[i*LDN], n, optimizationSettings->constraintParameter,
				optimizationSettings->gpu_sm_count, optimizationSettings->gpu_max_threads);
		thrust::transform(it_begin, it_end, it_begin,
				gpu_hard_treshholding<double> (tresh_hold));
	}
}



//===========================================================================
template<typename F>
void perform_hard_and_soft_thresholding_with_sorting(
		thrust::device_vector<F> &d_V, OptimizationSettings* optimizationSettings,
		const unsigned int n, thrust::device_vector<F>& d_x_for_sort,
		thrust::host_vector<F>& h_x, const unsigned int LDN) {
}

template<>
void perform_hard_and_soft_thresholding_with_sorting(
		thrust::device_vector<float> &d_V, OptimizationSettings* optimizationSettings,
		const unsigned int n, thrust::device_vector<float>& d_x_for_sort,
		thrust::host_vector<float>& h_x, const unsigned int LDN) {
	thrust::device_vector<float>::iterator it_begin = d_V.begin();
	for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
		// copy data to x_for_sort
		if (i > 0)
			thrust::advance(it_begin, LDN);
		thrust::device_vector<float>::iterator it_end = it_begin + n;
		thrust::copy(it_begin, it_end, d_x_for_sort.begin());
		// sort data on the device
		thrust::sort(d_x_for_sort.begin(), d_x_for_sort.end(),
				comparator_abs_val<float> ());// maybe use stable_sort ???
		if (optimizationSettings->isL1ConstrainedProblem()) {
			thrust::copy(d_x_for_sort.begin(), d_x_for_sort.end(), h_x.begin());
			float tresh_hold = compute_soft_thresholding_parameter(&h_x[0], n,
					optimizationSettings->constraintParameter);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_soft_treshholding<float> (tresh_hold));
		} else {
			float tresh_hold = abs(d_x_for_sort[optimizationSettings->constraintParameter - 1]);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_hard_treshholding<float> (tresh_hold));
		}
	}
}

template<>
void perform_hard_and_soft_thresholding_with_sorting(
		thrust::device_vector<double> &d_V, OptimizationSettings* optimizationSettings,
		const unsigned int n, thrust::device_vector<double>& d_x_for_sort,
		thrust::host_vector<double>& h_x, const unsigned int LDN) {
	thrust::device_vector<double>::iterator it_begin = d_V.begin();
	for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
		// copy data to x_for_sort
		if (i > 0)
			thrust::advance(it_begin, LDN);
		thrust::device_vector<double>::iterator it_end = it_begin + n;
		thrust::copy(it_begin, it_end, d_x_for_sort.begin());
		// sort data on the device
		thrust::sort(d_x_for_sort.begin(), d_x_for_sort.end(),
				comparator_abs_val<double> ());// maybe use stable_sort ???
		if (optimizationSettings->isL1ConstrainedProblem()) {
			thrust::copy(d_x_for_sort.begin(), d_x_for_sort.end(), h_x.begin());
			double tresh_hold = compute_soft_thresholding_parameter(&h_x[0], n,
					optimizationSettings->constraintParameter);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_soft_treshholding<double> (tresh_hold));
		} else {
			double tresh_hold = abs(d_x_for_sort[optimizationSettings->constraintParameter - 1]);
			thrust::transform(it_begin, it_end, it_begin,
					gpu_hard_treshholding<double> (tresh_hold));
		}
	}
}

template<typename F>
void perform_hard_and_soft_thresholding_for_penalized(
		thrust::device_vector<F> &d_V, SolverStructures::OptimizationSettings* optimizationSettings,
		const unsigned int n, ValueCoordinateHolder<F>* vals,
		const unsigned int LDN) {
}

template<>
void perform_hard_and_soft_thresholding_for_penalized(
		thrust::device_vector<double> &d_V, SolverStructures::OptimizationSettings* optimizationSettings,
		const unsigned int n, ValueCoordinateHolder<double>* vals,
		const unsigned int LDN) {
	thrust::device_vector<double>::iterator it_begin = d_V.begin();
	if (optimizationSettings->isL1PenalizedProblem()) {
		//------------L1 PENALIZED
		for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
			if (i > 0)
				thrust::advance(it_begin, LDN);

			thrust::device_vector<double>::iterator it_end = it_begin + n;
			vals[i].val = thrust::transform_reduce(it_begin, it_end,
					gpu_l1_penalized_objval<double> (optimizationSettings->penaltyParameter), 0.0f,
					thrust::plus<double>());
			vals[i].val = std::sqrt(vals[i].val);
		}
		thrust::transform(d_V.begin(), d_V.end(), d_V.begin(),
				gpu_l1_penalized_tresholing<double> (optimizationSettings->penaltyParameter));
	} else {
		//------------L0 PENALIZED
		for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
			if (i > 0)
				thrust::advance(it_begin, LDN);
			thrust::device_vector<double>::iterator it_end = it_begin + n;
			vals[i].val = thrust::transform_reduce(it_begin, it_end,
					gpu_l0_penalized_objval<double> (optimizationSettings->penaltyParameter), 0.0f,
					thrust::plus<double>());
		}
		thrust::transform(d_V.begin(), d_V.end(), d_V.begin(),
				gpu_l0_penalized_tresholing<double> (optimizationSettings->penaltyParameter));
	}
}

template<>
void perform_hard_and_soft_thresholding_for_penalized(
		thrust::device_vector<float> &d_V, OptimizationSettings* optimizationSettings,
		const unsigned int n, ValueCoordinateHolder<float>* vals,
		const unsigned int LDN) {
	thrust::device_vector<float>::iterator it_begin = d_V.begin();
	if (optimizationSettings->isL1PenalizedProblem()) {
		//------------L1 PENALIZED
		for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
			if (i > 0)
				thrust::advance(it_begin, LDN);
			thrust::device_vector<float>::iterator it_end = it_begin + n;
			vals[i].val = thrust::transform_reduce(it_begin, it_end,
					gpu_l1_penalized_objval<float> (optimizationSettings->penaltyParameter), 0.0f,
					thrust::plus<float>());
			vals[i].val = std::sqrt(vals[i].val);
		}
		thrust::transform(d_V.begin(), d_V.end(), d_V.begin(),
				gpu_l1_penalized_tresholing<float> (optimizationSettings->penaltyParameter));
	} else {
		//------------L0 PENALIZED
		for (unsigned int i = 0; i < optimizationSettings->totalStartingPoints; i++) {
			if (i > 0)
				thrust::advance(it_begin, LDN);
			thrust::device_vector<float>::iterator it_end = it_begin + n;
			vals[i].val = thrust::transform_reduce(it_begin, it_end,
					gpu_l0_penalized_objval<float> (optimizationSettings->penaltyParameter), 0.0f,
					thrust::plus<float>());
		}
		thrust::transform(d_V.begin(), d_V.end(), d_V.begin(),
				gpu_l0_penalized_tresholing<float> (optimizationSettings->penaltyParameter));
	}
}

#endif /* MY_CUBLAS_WRAPPER_H_ */
