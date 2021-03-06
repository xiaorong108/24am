/*
 * experiment_utils.h
 *
 *  Created on: Oct 9, 2012
 *      Author: taki
 */

#ifndef EXPERIMENT_UTILS_H_
#define EXPERIMENT_UTILS_H_


template<typename F>
void logTime(ofstream &stream, mytimer* mt, optimization_Statisticsistics* optimizationStatistics,
		optimization_settings* optimizationSettings, std::vector<F>& x, int m, int n) {
	int nnz = vector_get_nnz(&x[0], n);
	cout << optimizationSettings->formulation << "," << nnz << "," << m << "," << n << ","
			<< mt->getElapsedWallClockTime() << ","
			<< optimizationStatistics->totalTrueComputationTime << "," << optimizationSettings->batchSize << ","
			<< optimizationSettings->on_the_fly_generation
			<< ","<<optimizationStatistics->totalThreadsUsed
			<< ","<<optimizationSettings->totalStartingPoints
			<< ","<<optimizationStatistics->it
			<< endl;
	stream<< optimizationSettings->formulation << "," << nnz << "," << m << "," << n << ","
			<< mt->getElapsedWallClockTime() << ","
			<< optimizationStatistics->totalTrueComputationTime << "," << optimizationSettings->batchSize << ","
			<< optimizationSettings->on_the_fly_generation
			<< ","<<optimizationStatistics->totalThreadsUsed
			<< ","<<optimizationSettings->totalStartingPoints
			<< ","<<optimizationStatistics->it
			<< endl;
}



#endif /* EXPERIMENT_UTILS_H_ */
