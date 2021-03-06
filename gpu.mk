#GPU build script

#please put here cuda install path. 
#on Linux type "which nvcc" and if the result is
#                  /exports/applications/apps/cuda/rhel5/4.2/cuda/bin/nvcc
#then the install path is 
CUDA_INSTALL_PATH= /exports/applications/apps/cuda/rhel5/4.2/cuda

# Tested CUDA versions are
# 4.2, 4.0rc2, 4.0
# We tested the code on Tesla M2050
#============================================================================================
# You should not modify the lines below

CUDA_COMPILER=nvcc
CUDA_INCLUDES = -I. -I$(CUDA_INSTALL_PATH)/include -I/usr/local/include
CUDA_LIB =  -L$(CUDA_INSTALL_PATH)/lib -L$(CUDA_INSTALL_PATH)/lib64  -lcublas -lm -arch sm_20 -lgomp
CUDA_COMILER_FLAGS= -O3 -w 


gpu_console:  
	$(CUDA_COMPILER) -O3 -w $(CUDA_INCLUDES) $(FRONTENDFOLDER)gpu_console.cu      $(CUDA_LIB)  -o $(BUILD_FOLDER)gpu_console

gpu_test: gpu_console
	./$(BUILD_FOLDER)gpu_console -i datasets/small.csv  -o results/small_gpu.txt -v true -d double -f 1 -s 3
	./$(BUILD_FOLDER)gpu_console -i datasets/small.csv  -o results/small_2_gpu.txt -v true -d double -l 1000 -r 64  -f 1 -s 2
	./$(BUILD_FOLDER)gpu_console -i datasets/small.csv  -o results/small_3_gpu.txt -v true -d double -l 1000 -r 64 -u 1 -f 1 -s 2

gpu_unit_test: cluster_generator
	$(CUDA_COMPILER) -O3 -w $(GSL_INCLUDE) $(CUDA_INCLUDES) $(SRC)/test/gpu_unit_test.cu       $(CUDA_LIB) $(BLAS_LIB) $(LIBS_GSL) -o $(BUILD_FOLDER)gpu_unittest
	./$(BUILD_FOLDER)gpu_unittest

gpu_paper_experiments_sppedup: 	
	$(CUDA_COMPILER) -O3 -w $(GSL_INCLUDE) $(CUDA_INCLUDES) $(SRC)/paper_experiments/experiment_gpu_speedup.cu       $(CUDA_LIB) $(LIBS_GSL) -o $(BUILD_FOLDER)experiment_gpu_speedup
	./$(BUILD_FOLDER)experiment_gpu_speedup



gpu: gpu_test  gpu_unit_test
