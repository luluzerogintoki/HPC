#include <stdio.h>
#include <stdlib.h>
//#include <stddef.h>
#include <string.h>
#define CL_TARGET_OPENCL_VERSION 300
#include <CL/cl.h>

int main(int argc, char*argv[]) {
	//Arguments are passed in the following way:
	//./diffusion -n20 -d0.02
	//to compute 20 iterations with diffusion constant 0.02. 
    int iterNum;
	float diffusion;
    for (int i = 0; i < argc; i++) {
		//printf("argv[%i]=%s\n", i, argv[i]);
		if (strncmp(argv[i], "-n", 2) == 0) {
			if (argv[i][2] != '\0')
				iterNum = atoi(argv[i]+2);
			else
				iterNum = atoi(argv[i+1]);
		}

		if 	(strncmp(argv[i], "-d", 2) == 0) {
			//printf("argv[%i]=%s\n", i, argv[i]);
			if (argv[i][2] != '\0')
				diffusion = atof(argv[i]+2);
			else
				diffusion = atof(argv[i+1]);
		}
	}

    //printf("iterNum = %d\n", iterNum);
    //printf("diffusion = %.2f\n", diffusion);

	//Reads array size and initial values from a text file called "init".
	FILE *fp;
	fp = fopen("init", "r");
	if (fp == NULL) {
		printf("Could not open file init\n");
		exit(1);
	}
	//3 3       two positive integers, which you may assume are positive(width and height)
	//1 1 1e6   first two int are valid coordinates, third is the initial value as a float

	int width, height;
	int x, y;
	float initVal;
	fseek(fp, 0, SEEK_SET); //go to the beginning of the file
	fscanf(fp, "%d %d", &width, &height); //read the first line
	//printf("width = %d, height = %d\n", width, height);
	int size = width * height;
	float *currentBox = malloc(size*sizeof(float));
	float *nextBox = malloc(size*sizeof(float));
	while(fscanf(fp, "%d %d %f", &x, &y, &initVal) != EOF) {
		//printf("x = %d, y = %d, initVal = %f\n", x, y, initVal);
		currentBox[y*width+x] = initVal;
	}
	fclose(fp);

	//initial openCL
	cl_int error;

	//get platform
	cl_platform_id platform_id;
	cl_uint nmb_platforms;
	if ( clGetPlatformIDs(1, &platform_id, &nmb_platforms) != CL_SUCCESS ) {
		fprintf(stderr, "cannot get platform\n" );
		return 1;
	}

	//get device(GPU)
	cl_device_id device_id;
	cl_uint nmb_devices;
	if ( clGetDeviceIDs(platform_id, CL_DEVICE_TYPE_GPU, 1, &device_id, &nmb_devices) != CL_SUCCESS ) {
		fprintf(stderr, "cannot get device\n" );
		return 1;
	}

    //cl_platform_id platforms[64];
	//unsigned int platformCount;
	//cl_int platformResult = clGetPlatformIDs( 64, platforms, &platformCount );

	// cl_device_id device = NULL;
	// int foundGpus = 0;
	// for( int i = 0; i < platformCount; ++i ) {
	// 	cl_device_id devices[64];
	// 	unsigned int deviceCount;
	// 	cl_int deviceResult = clGetDeviceIDs( platforms[i], CL_DEVICE_TYPE_GPU, 64, devices, &deviceCount );

	// 	if ( deviceResult == CL_SUCCESS ) {
	// 		for( int j = 0; j < deviceCount; ++j ) {
	// 			char vendorName[256];
	// 			size_t vendorNameLength;
	// 			cl_int deviceInfoResult = clGetDeviceInfo( devices[j], CL_DEVICE_VENDOR, 256, vendorName, &vendorNameLength );
	// 			if ( deviceInfoResult == CL_SUCCESS) {
	// 				foundGpus++;
	// 				if (foundGpus == 2){      // <----- USE 2ND GPU!!!!!
	// 					device_id = devices[j];
	// 				}
	// 			}
	// 		}
	// 	}
	// }

	//create context for the device
	cl_context context;
	cl_context_properties properties[] =
	{
		CL_CONTEXT_PLATFORM,
		(cl_context_properties) platform_id,
		0
	};
	context = clCreateContext(properties, 1, &device_id, NULL, NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create context\n");
		return 1;
	}

	//create command queue for the device in the context
	cl_command_queue command_queue;
	command_queue = clCreateCommandQueueWithProperties(context, device_id, NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create command queue\n");
		return 1;
	}

	//read kernel program
	char *opencl_program_src;
	{
		FILE *clfp = fopen("./diffusion.cl", "r");
		if ( clfp == NULL ) {
		fprintf(stderr, "could not load cl source code\n");
		return 1;
		}
		fseek(clfp, 0, SEEK_END);
		int clfsz = ftell(clfp);
		fseek(clfp, 0, SEEK_SET);
		opencl_program_src = (char*) malloc((clfsz+1)*sizeof(char));
		fread(opencl_program_src, sizeof(char), clfsz, clfp);
		opencl_program_src[clfsz] = 0;
		fclose(clfp);
	}

	//create program from source
	cl_program program;
	size_t src_len = strlen(opencl_program_src);
	program = clCreateProgramWithSource(
					context, 1, (const char **) &opencl_program_src, (const size_t*) &src_len, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create program\n");
		return 1;
	}

	free(opencl_program_src);
	
	//build program by compiling it
	error = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot build program. log:\n");
		
		size_t log_size = 0;
		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

		char *log = malloc(log_size*sizeof(char));
		if ( log == NULL ) {
		fprintf(stderr, "could not allocate memory\n");
		return 1;
		}

		clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

		fprintf(stderr, "%s\n", log );

		free(log);

		return 1;
	}

	//create kernel
	//compute diffusion for each box each step
	cl_kernel kernel_computeIter = clCreateKernel(program, "computeIter", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel computeIter\n");
		return 1;
	}

	//compute the sum of all the boxes
	cl_kernel kernel_reduction = clCreateKernel(program, "reduction", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel reduction\n");
		return 1;
	}

	//compute the absolute difference 
	cl_kernel kernel_computeAbsDiff = clCreateKernel(program, "computeAbsDiff", &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create kernel computeAbsDiff\n");
		return 1;
	}

	//create buffers
	cl_mem box_buffer_current, box_buffer_next, output_buffer_sum;
	box_buffer_current = clCreateBuffer(context, CL_MEM_READ_WRITE, size*sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create buffer box_buffer_current\n");
		return 1;
	}
	box_buffer_next = clCreateBuffer(context, CL_MEM_READ_WRITE, size*sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create buffer box_buffer_next\n");
		return 1;
	}

	//enqueue write of buffers -- copy data from host to device only the currentBox
	if ( clEnqueueWriteBuffer(command_queue, box_buffer_current, CL_TRUE, 0, size*sizeof(float), currentBox, 0, NULL, NULL) != CL_SUCCESS ) {
		fprintf(stderr, "cannot enqueue write of buffer box_buffer_current\n");
		return 1;
	}
	if ( clEnqueueWriteBuffer(command_queue, box_buffer_next, CL_TRUE, 0, size*sizeof(float), nextBox, 0, NULL, NULL) != CL_SUCCESS ) {
		fprintf(stderr, "cannot enqueue write of buffer box_buffer_next\n");
		return 1;
	}

	//compute the iteration of diffusion
	//Executes a given number of steps of heat diffusion with given diffusion constant
	//100 * 100	 10000 * 10000	 100000 * 100
	const size_t globalWorkSize[] = {width, height};
/* 	size_t localWorkSize[] = {10, 10};
	if(width > 100 && width % 10 == 0){
		localWorkSize[0] = 1000;
	}
	if(height > 100 && height % 10 == 0){
		localWorkSize[1] = 1000;
	} */
	size_t localWorkSize[] = {1,1};
	if (width >= 10 && height >= 10 && width % 10 == 0 && height % 10 == 0) {
		localWorkSize[0] = width/10;
		localWorkSize[1] = height/10;
	}
	if (width > 100 && height == 100 && width % 10 == 0 && height % 10 == 0) {
		localWorkSize[0] = width/10;
		localWorkSize[1] = height/10;
	}
	//printf("localWorkSize[0] = %zu, localWorkSize[1] = %zu\n", localWorkSize[0], localWorkSize[1]);

	cl_uint status1 = 0; 
	cl_uint status2 = 1;

    //set kernel arguments constant
    clSetKernelArg(kernel_computeIter, 2, sizeof(cl_int), &width);
    clSetKernelArg(kernel_computeIter, 3, sizeof(cl_int), &height);
    clSetKernelArg(kernel_computeIter, 4, sizeof(cl_float), &diffusion);

	for(size_t iter = 0; iter < iterNum; iter++){
		//set kernel arguments
		clSetKernelArg(kernel_computeIter, iter%2 == 0 ? 0 : 1, sizeof(cl_mem), &box_buffer_current);
		clSetKernelArg(kernel_computeIter, iter%2 == 0 ? 1 : 0, sizeof(cl_mem), &box_buffer_next);

		//execute kernel
		//if(clEnqueueNDRangeKernel(command_queue, kernel_computeIter, (cl_uint)2, NULL, (const size_t*) &globalWorkSize, NULL, 0, NULL, NULL) != CL_SUCCESS){
		clEnqueueNDRangeKernel(command_queue, kernel_computeIter, (cl_uint)2, NULL, (const size_t*) &globalWorkSize, (const size_t*) &localWorkSize, 0, NULL, NULL);
			//fprintf(stderr, "cannot enqueue kernel computeIter\n");
			//return 1;
		

		//synchonize
		if(clFinish(command_queue) != CL_SUCCESS){
			fprintf(stderr, "cannot finish queue\n");
			return 1;
		}

		//change the status of current and next box buffer
		status1 = status1 == 0 ? 1 : 0;
		status2 = status2 == 1 ? 0 : 1;

		//printf("iter = %d finish.\n", iter);

	}

	//compute the sum of all the boxes
	//Outputs the average of temperatures, say X, as average: X.
	const int global_redsz = 1024;
	const int local_redsz = 32;
	const int nmb_redgps = global_redsz / local_redsz;

	//buffer for reduction result
	output_buffer_sum = clCreateBuffer(context, CL_MEM_WRITE_ONLY, nmb_redgps*sizeof(float), NULL, &error);
	if ( error != CL_SUCCESS ) {
		fprintf(stderr, "cannot create buffer output_buffer_sum\n");
		return 1;
	}

	//set kernel arguments
	const cl_int sz_clint = (cl_int)size;
	if(status1 == 0){
		clSetKernelArg(kernel_reduction, 0, sizeof(cl_mem), &box_buffer_current);
	}else{
		clSetKernelArg(kernel_reduction, 0, sizeof(cl_mem), &box_buffer_next);
	}
	clSetKernelArg(kernel_reduction, 1, local_redsz*sizeof(float), NULL);
	clSetKernelArg(kernel_reduction, 2, sizeof(cl_int), &sz_clint);
	clSetKernelArg(kernel_reduction, 3, sizeof(cl_mem), &output_buffer_sum);
	
	size_t global_redsz_szt = (size_t) global_redsz;
	size_t local_redsz_szt = (size_t) local_redsz;
	//execute kernel
	// if(clEnqueueNDRangeKernel(command_queue, kernel_reduction, (cl_uint)1, NULL, (const size_t*)&global_redsz_szt, (const size_t*)&local_redsz_szt, 0, NULL, NULL) != CL_SUCCESS){
	// 	fprintf(stderr, "cannot enqueue kernel reduction\n");
	// 	return 1;
	// }
	clEnqueueNDRangeKernel(command_queue, kernel_reduction, (cl_uint)1, NULL, (const size_t*)&global_redsz_szt, (const size_t*)&local_redsz_szt, 0, NULL, NULL);

	//copy the reduction sum result from device to host
	float *sum = malloc(nmb_redgps*sizeof(float));
	if ( clEnqueueReadBuffer(command_queue, output_buffer_sum, CL_TRUE, 0, nmb_redgps*sizeof(float), sum, 0, NULL, NULL) != CL_SUCCESS ) {
		fprintf(stderr, "cannot enqueue read of buffer output_buffer_sum for average\n");
		return 1;
	}

	//synchonize
	if(clFinish(command_queue) != CL_SUCCESS){
		fprintf(stderr, "cannot finish queue\n");
		return 1;
	}

	//compute the sum of all the boxes
	float sum_all = 0;
	for(int i = 0; i < nmb_redgps; ++i){
		sum_all += sum[i];
	}
	//printf("sum_all: %f\n", sum_all);
	float average = sum_all / size;
	printf("Average: %f\n", average);

	//compute the absolute difference
	//Outputs the average absolute difference of each temperature to the average of all temperatures, say Y
	//as average absolute difference: Y.
	//set kernel arguments
	if(status1 == 0){
		clSetKernelArg(kernel_computeAbsDiff, 0, sizeof(cl_mem), &box_buffer_current);
	}else{
		clSetKernelArg(kernel_computeAbsDiff, 0, sizeof(cl_mem), &box_buffer_next);
	}
	clSetKernelArg(kernel_computeAbsDiff, 1, sizeof(cl_float), &average);
	clSetKernelArg(kernel_computeAbsDiff, 2, sizeof(cl_int), &height);

	//execute kernel
	// if(width > 100 && height == 100){
	// 	clEnqueueNDRangeKernel(command_queue, kernel_computeAbsDiff, (cl_uint)2, NULL, (const size_t*) &globalWorkSize, (const size_t*) &localWorkSize, 0, NULL, NULL);
	// 		//fprintf(stderr, "cannot enqueue kernel computeAbsDiff\n");
	// 		//return 1;
		
	// }else{
	if(clEnqueueNDRangeKernel(command_queue, kernel_computeAbsDiff, (cl_uint)2, 0, (const size_t*) &globalWorkSize, NULL, 0, NULL, NULL) != CL_SUCCESS){
		fprintf(stderr, "cannot enqueue kernel computeAbsDiff\n");
		return 1;
	}
	//}

	//synchonize
	// if(clFinish(command_queue) != CL_SUCCESS){
	// 	fprintf(stderr, "cannot finish queue bug1\n");
	// 	return 1;
	// }

	//compute the average absolute difference
	// if(clEnqueueNDRangeKernel(command_queue, kernel_reduction, (cl_uint)1, NULL, (const size_t*) &global_redsz_szt, (const size_t*) &local_redsz_szt, 0, NULL, NULL) != CL_SUCCESS){
	// 	fprintf(stderr, "cannot enqueue kernel reduction\n");
	// 	return 1;
	// }
	clEnqueueNDRangeKernel(command_queue, kernel_reduction, (cl_uint)1, NULL, (const size_t*) &global_redsz_szt, (const size_t*) &local_redsz_szt, 0, NULL, NULL);

	//synchonize
	// if(clFinish(command_queue) != CL_SUCCESS){
	// 	fprintf(stderr, "cannot finish queue\n");
	// 	return 1;
	// }

	//copy the reduction sum absolution difference result from device to host
	float *absDiff = malloc(nmb_redgps*sizeof(float));
	if ( clEnqueueReadBuffer(command_queue, output_buffer_sum, CL_TRUE, 0, nmb_redgps*sizeof(float), absDiff, 0, NULL, NULL) != CL_SUCCESS ) {
		fprintf(stderr, "cannot enqueue read of buffer output_buffer_sum for absdiff\n");
		return 1;
	}
	//

	//synchonize
	if(clFinish(command_queue) != CL_SUCCESS){
		fprintf(stderr, "cannot finish queue\n");
		return 1;
	}

	//compute the sum of all the reduction result
	float absDiff_all = 0;
	for(int i = 0; i < nmb_redgps; i++){
		absDiff_all += absDiff[i];
	}

	//compute the average absolute difference
	float averageAbsDiff = absDiff_all / size;
	printf("Average absolute difference: %f\n", averageAbsDiff);

	//free memory
	free(currentBox);
	free(nextBox);
	free(sum);
	free(absDiff);

	//release openCL resources
	clReleaseMemObject(box_buffer_current);
	clReleaseMemObject(box_buffer_next);
	clReleaseMemObject(output_buffer_sum);

	clReleaseProgram(program);
	clReleaseKernel(kernel_computeIter);
	clReleaseKernel(kernel_reduction);
	clReleaseKernel(kernel_computeAbsDiff);
	
	clReleaseCommandQueue(command_queue);
	clReleaseContext(context);

    return 0;
}