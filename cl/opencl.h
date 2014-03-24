/* header for opencl.cpp */

#ifndef OPENCL_H
#define OPENCL_H

#include <iostream>
#include <fstream>
#include <chrono>

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <OpenCL/opencl.h>

using namespace std;

/* parallelizable section */
const char * KernelSource =
"__kernel void matrix(__global int * matrix, __global long int * size, __global int * output){\n"\
"for (long int i = 0; i < *size; i++){\n"\
"output[i] = matrix[i] * 2; \n"\
"}\n"\
"}\n"\
"\n";

class cl {
    
public:
    
    cl_kernel kernel;
    cl_command_queue commands;
    cl_context context;
    cl_mem matrix, s, output;
    cl_int err;
    cl_device_id device_id;
    cl_program program;
    
    long int size;
    
    size_t global;

    cl();
    ~cl();
    void createQueue(int * inputmatrix, long int defsize);
    bool processMatrix();

};

cl::cl(){

    int gpu = 1;
    if (clGetDeviceIDs(NULL, gpu ? CL_DEVICE_TYPE_GPU : CL_DEVICE_TYPE_GPU, 1, &device_id, NULL) != CL_SUCCESS){
        printf("unable to determine device id\n");
        exit(1);
    }
    
    /* Now I must create a context to gain access to the gpu */
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if(!context){
        printf("No Context generated!\n");
        exit(1);
    }
    
    /* Create a queue of commands torun on the device */
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if(!context){
        printf("No commands generated!\n");
        exit(1);
    }
    
    /* create a program from the kernel source code (marked up above) */
    program = clCreateProgramWithSource(context, 1, (const char **) &KernelSource, NULL, &err);
    if(!program){
        printf("Error: Failed to create program!\n");
        exit(1);
    }
    
    /* Now compile the program via clBuilder */
    if (clBuildProgram(program, 0, NULL, NULL, NULL, NULL) != CL_SUCCESS){
        size_t len;
        char buffer[2048];
        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }
    
    /**
     * Specify which program to exectue
     * This allows for multiple programs
     **/
    kernel = clCreateKernel(program, "matrix", &err);
    if (!kernel || err != CL_SUCCESS){
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }
}

void cl::createQueue(int * inputmatrix, long int defsize){
    
    size = defsize;

    /* create buffers for the input, essentially allocating gpu memory */
    s        = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(long int), NULL, NULL);
    matrix   = clCreateBuffer(context, CL_MEM_READ_ONLY, sizeof(int) * size, NULL, NULL);
    output   = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(int) * size, NULL, NULL);
    
    if (!s || !matrix){
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }
    
    /* Now copy over the data/load it up for execution */
    err = clEnqueueWriteBuffer(commands, matrix, CL_TRUE, 0, sizeof(int) * size, inputmatrix, 0, NULL, NULL);
    
    long int * ptr_size = new long int(size);
    
    int err_s = clEnqueueWriteBuffer(commands, s, CL_TRUE, 0, sizeof(long int), ptr_size, 0, NULL, NULL);
    
    if(err_s){
        printf("Failed to writer buffer\n");
        exit(1);
    }
    
    /* set the argument list for the kernel commands */
    int errarg = 0;
    errarg  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &matrix);
    errarg |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &s);
    errarg |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &output);
    global = size * size;
    
    if (errarg != CL_SUCCESS){
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }
    
    errarg = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(int) * size, NULL, NULL);
    
    if (errarg != CL_SUCCESS){
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }
    
    /* Now put the kernel on the queue for execution */
    clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, NULL, 0, NULL, NULL);

}

cl::~cl(){

    clReleaseMemObject(matrix);
    clReleaseMemObject(s);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);
    
}


bool cl::processMatrix(){
    
    
    /* TODO execute all queue */
    clFinish(commands);
    
    /* Copy results back to cpu memory */
    int outmatrix[size];
    clEnqueueReadBuffer(commands, output, CL_TRUE, 0, sizeof(int) * size, outmatrix, 0, NULL, NULL);
    
    if(outmatrix[5] == 2){
        cout << "Passed: " << size << endl;
        return true;
    }else{
        cout << "Fail: " << size << endl;
        return false;
    }
}

#endif
