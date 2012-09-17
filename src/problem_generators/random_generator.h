/*
 * random_generator.h
 *
 *  Created on: Sep 5, 2012
 *      Author: taki
 */

#ifndef RANDOM_GENERATOR_H_
#define RANDOM_GENERATOR_H_


void generate_random_instance(int n, int m, gsl_matrix * B, gsl_matrix * BT,
		gsl_vector * x) {

#ifdef _OPENMP
	//#pragma omp parallel for
#endif
	for (int i = 0; i < n; i++) {
		gsl_vector_set(x, i, (double) rand_r(&myseed) / RAND_MAX);
		for (int j = 0; j < m; j++) {
			double tmp = (double) rand_r(&myseed) / RAND_MAX;
			tmp = tmp * 2 - 1;
			gsl_matrix_set(B, j, i, tmp);
		}
		gsl_vector_view col = gsl_matrix_column(B, i);
		double col_norm = gsl_blas_dnrm2(&col.vector);
		gsl_vector_scale(&col.vector, 1 / col_norm);
	}
#ifdef _OPENMP
#pragma omp parallel for
#endif
	for (int i = 0; i < n; i++) {
		for (int j = 0; j < m; j++) {
			gsl_matrix_set(BT, i, j, gsl_matrix_get(B, j, i));
		}
	}
}




#endif /* RANDOM_GENERATOR_H_ */
