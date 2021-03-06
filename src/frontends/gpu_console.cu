/*
 *
 * This is a parallel sparse PCA solver
 *
 * The solver is based on a simple alternating maximization (AM) subroutine 
 * and is based on the paper
 *    P. Richtarik, M. Takac and S. Damla Ahipasaoglu 
 *    "Alternating Maximization: Unified Framework and 24 Parallel Codes for L1 and L2 based Sparse PCA"
 *
 * The code is available at https://code.google.com/p/24am/
 * under GNU GPL v3 License
 * 
 *    GPU SOLVER FOR SPARSE PCA - frontend console interface
 *
 */


#include "../class/optimization_settings.h"
#include "../class/optimization_statistics.h"
#include "../utils/file_reader.h"
#include "../utils/option_console_parser.h"
#include "../gpugpower/gpu_headers.h"


template<typename F>
int load_data_and_run_solver(SolverStructures::OptimizationSettings* optimizationSettings) {
	mytimer* mt = new mytimer();
	mt->start();
	SolverStructures::OptimizationStatistics* optimizationStatistics =
			new OptimizationStatistics();
	cudaDeviceProp dp;
	cudaGetDeviceProperties(&dp, 0);
	optimizationSettings->gpu_sm_count = dp.multiProcessorCount;
	optimizationSettings->gpu_max_threads = dp.maxThreadsPerBlock;

	unsigned int ldB;
	unsigned int m;
	unsigned int n;
	std::vector<F> B_mat;
	InputOuputHelper::readCSVFile(B_mat, ldB, m, n, optimizationSettings->inputFilePath);
	optimizationStatistics->n = n;

	const int MEMORY_BANK_FLOAT_SIZE = MEMORY_ALIGNMENT / sizeof(F);
	const unsigned int LD_M = (
			m % MEMORY_BANK_FLOAT_SIZE == 0 ?
					m :
					(m / MEMORY_BANK_FLOAT_SIZE + 1) * MEMORY_BANK_FLOAT_SIZE);
	const unsigned int LD_N = (
			n % MEMORY_BANK_FLOAT_SIZE == 0 ?
					n :
					(n / MEMORY_BANK_FLOAT_SIZE + 1) * MEMORY_BANK_FLOAT_SIZE);
	thrust::host_vector<F> h_B(LD_M * n, 0);
	// get data into h_B;
	for (unsigned int row = 0; row < m; row++) {
		for (unsigned int col = 0; col < n; col++) {
			h_B[row + col * LD_M] = B_mat[row + col * m];
		}
	}
	// allocate vector for solution
	thrust::host_vector<F> h_x(n, 0);
	// move data to DEVICE
	thrust::device_vector<F> d_B = h_B;

	cublasStatus_t optimizationStatisticsus;
	cublasHandle_t handle;
	optimizationStatisticsus = cublasCreate(&handle);
	if (optimizationStatisticsus != CUBLAS_STATUS_SUCCESS) {
		fprintf(stderr, "! CUBLAS initialization error\n");
		return EXIT_FAILURE;
	} else {
		printf("CUBLAS initialized.\n");
	}
//FIXME
	optimizationSettings->useKSelectionAlgorithmGPU = true;
	optimizationSettings->useKSelectionAlgorithmGPU = false;
	SPCASolver::GPUSolver::denseDataSolver(handle, m, n, d_B, h_x, optimizationSettings, optimizationStatistics,
			LD_M, LD_N);
	mt->end();
	optimizationStatistics->totalElapsedTime = mt->getElapsedWallClockTime();
	InputOuputHelper::save_results(optimizationStatistics, optimizationSettings, &h_x[0], n);
	InputOuputHelper::saveSolverStatistics(optimizationStatistics, optimizationSettings);
	optimizationStatisticsus = cublasDestroy(handle);
	if (optimizationStatisticsus != CUBLAS_STATUS_SUCCESS) {
		fprintf(stderr, "!cublas shutdown error\n");
		return EXIT_FAILURE;
	}
	return 0;
}

int main(int argc, char *argv[]) {
	SolverStructures::OptimizationSettings* optimizationSettings =
			new OptimizationSettings();
	int optimizationStatisticsus = parseConsoleOptions(optimizationSettings, argc, argv);
	if (optimizationStatisticsus > 0)
		return optimizationStatisticsus;
	if (optimizationSettings->useDoublePrecision) {
		load_data_and_run_solver<double>(optimizationSettings);
	} else {
		load_data_and_run_solver<float>(optimizationSettings);
	}
	return 0;
}

