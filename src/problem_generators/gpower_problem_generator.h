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


#ifndef GPOWER_PROBLEM_GENERATOR2_H_
#define GPOWER_PROBLEM_GENERATOR2_H_






template<typename F>
int generateProblem(const int n, const int m,F* h_B,bool uniformScale=false) {

	printf("%d x %d \n", m, n);

	char final_file[1000];
	unsigned int seed = 0;
	for (int i = 0; i < n; i++) {
		F total = 0;
		for (int j = 0; j < m; j++) {
			F tmp = (F) rand_r(&seed) / RAND_MAX;
			tmp=-1+2*tmp;
			h_B[j+i*m]=tmp;
			total+=tmp*tmp;
		}
		total=sqrt(total);
		if (uniformScale){
			total=total*((F) rand_r(&seed) / (RAND_MAX+0.0));
		}
		for (int j = 0; j < m; j++) {
			h_B[j+i*m]=h_B[j+i*m]/total;
		}
	}
	return 0;
}


template<typename F>
int generateProblem(const int n, const int m,F* h_B,const unsigned int LD_M,const unsigned int LD_N,bool uniformScale=false) {

	printf("%d x %d \n", m, n);

	//	char final_file[1000];
	unsigned int seed = 0;
	for (int i = 0; i < n; i++) {
		F total = 0;
		for (int j = 0; j < m; j++) {
			F tmp = (F) rand_r(&seed) / RAND_MAX;
			tmp=-1+2*tmp;
			h_B[j+i*LD_M]=tmp;
			total+=tmp*tmp;
		}
		total=sqrt(total);
		if (uniformScale){
			total=1/total*(+((F) rand_r(&seed) / (RAND_MAX+0.0)));
		}

		for (int j = 0; j < m; j++) {
			h_B[j+i*LD_M]=h_B[j+i*LD_M]*total;
		}
	}
	return 0;
}





#endif /* GPOWER_PROBLEM_GENERATOR2_H_ */
