// Copyright (C) 2013-2018 Altera Corporation, San Jose, California, USA. All rights reserved.
// Permission is hereby granted, free of charge, to any person obtaining a copy of this
// software and associated documentation files (the "Software"), to deal in the Software
// without restriction, including without limitation the rights to use, copy, modify, merge,
// publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
// whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or
// substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
// OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
// HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
// 
// This agreement shall be governed in all respects by the laws of the State of California and
// by the laws of the United States of America.

///////////////////////////////////////////////////////////////////////////////////
// This host program executes a matrix multiplication kernel to perform:
//  C = A * B
// where A is a N x K matrix, B is a K x M matrix and C is a N x M matrix.
// All dimensions must be a multiple of BLOCK_SIZE, which affects the
// underlying kernel.
//
// This host program supports partitioning the problem across multiple OpenCL
// devices if available. If there are M available devices, the problem is
// divided so that each device operates on N/M rows (with
// processed by each device is . The host program
// assumes that all devices are of the same type (that is, the same binary can
// be used), but the code can be generalized to support different device types
// easily.
//
// Verification is performed against the same computation on the host CPU.
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "CL/opencl.h"
#include "AOCLUtils/aocl_utils.h"
#include "matrixMult.h"
#define MATSIZE 128
#define  STENCIL_RADIUS 2
using namespace aocl_utils;

#define DEBUG 0
#define DISABLE true
// OpenCL runtime configuration
cl_platform_id platform = NULL;
unsigned num_devices = 0;
scoped_array<cl_device_id> device; // num_devices elements
cl_context context = NULL;
scoped_array<cl_command_queue> queue; // num_devices elements
cl_program program = NULL;
scoped_array<cl_kernel> kernel; // num_devices elements
scoped_array<cl_mem> input_a_buf; // num_devices elements
scoped_array<cl_mem> input_b_buf; // num_devices elements
scoped_array<cl_mem> output_buf; // num_devices elements

// Problem data.
unsigned A_height = MATSIZE ;// 32 * BLOCK_SIZE;
unsigned A_width  = MATSIZE ;;
const unsigned &B_height = A_width;
unsigned B_width  = MATSIZE ;
const unsigned &C_height = A_height;
const unsigned &C_width  = B_width;

scoped_array<scoped_aligned_ptr<float> > input_a; // num_devices elements
scoped_aligned_ptr<float> input_b;
scoped_array<scoped_aligned_ptr<float> > output; // num_devices elements
scoped_array<float> ref_output;
scoped_array<unsigned> rows_per_device; // num_devices elements

// Function prototypes
float rand_float();
bool init_opencl();
void init_problem();
void run();
void compute_reference();
void verify();
void cleanup();

// Entry point.
int main(int argc, char **argv) {
  Options options(argc, argv);
  if(options.has("ah")) {
    A_height = options.get<unsigned>("ah");
  }
  if(options.has("aw")) {
    A_width = options.get<unsigned>("aw");
  }
  if(options.has("bw")) {
    B_width = options.get<unsigned>("bw");
  }
  A_height = A_width = B_width = MATSIZE;

  printf("Matrix sizes:\n  A: %d x %d\n  B: %d x %d\n  C: %d x %d\n",
      A_height, A_width, B_height, B_width, C_height, C_width);

  // Spot check matrix sizes. They all must be a multiple of BLOCK_SIZE,
  // although it is relatively straightforward to handle non-multiples
  // by adding padding. For simplicity, this example does not pad.
  if((A_height % BLOCK_SIZE) != 0 || (A_width % BLOCK_SIZE) != 0 ||
     (B_height % BLOCK_SIZE) != 0 || (B_width % BLOCK_SIZE) != 0 ||
     (C_height % BLOCK_SIZE) != 0 || (C_width % BLOCK_SIZE) != 0) {
    printf("Matrix sizes must be a multiple of %d.\n", BLOCK_SIZE);
    return -1;
  }

  // Initialize OpenCL.
  if(!init_opencl()) {
    return -1;
  }

  // Initialize the problem data.
  // Requires the number of devices to be known.
  init_problem();

  // Run the kernel.
  run();

  // Free the resources allocated
  cleanup();

  return 0;
}

/////// HELPER FUNCTIONS ///////

// Randomly generate a floating-point number between -10 and 10.
float rand_float() {
  return float(rand()) / float(RAND_MAX) * 20.0f - 10.0f;
}

// Initializes the OpenCL objects.
bool init_opencl() {
  cl_int status;

  printf("Initializing OpenCL\n");

  if(!setCwdToExeDir()) {
    return false;
  }

  // Get the OpenCL platform.
  platform = findPlatform("Intel(R) FPGA SDK for OpenCL(TM)");
  if(platform == NULL) {
    printf("ERROR: Unable to find Intel(R) FPGA OpenCL platform.\n");
    return false;
  }

  // Query the available OpenCL device.
  device.reset(getDevices(platform, CL_DEVICE_TYPE_ALL, &num_devices));
  num_devices = 1;
  printf("Platform: %s\n", getPlatformName(platform).c_str());
  printf("Using %d device(s)\n", num_devices);
  for(unsigned i = 0; i < num_devices; ++i) {
    printf("  %s\n", getDeviceName(device[i]).c_str());
  }

  // Create the context.
  context = clCreateContext(NULL, num_devices, device, &oclContextCallback, NULL, &status);
  checkError(status, "Failed to create context");

  // Create the program for all device. Use the first device as the
  // representative device (assuming all device are of the same type).
  std::string binary_file = getBoardBinaryFile("matrix_mult", device[0]);
  printf("Using AOCX: %s\n", binary_file.c_str());
  program = createProgramFromBinary(context, binary_file.c_str(), device, num_devices);

  // Build the program that was just created.
  status = clBuildProgram(program, 0, NULL, "", NULL, NULL);
  checkError(status, "Failed to build program");

  // Create per-device objects.
  queue.reset(num_devices);
  kernel.reset(num_devices);
  rows_per_device.reset(num_devices);
  input_a_buf.reset(num_devices);
  input_b_buf.reset(num_devices);
  output_buf.reset(num_devices);

  const unsigned num_block_rows = C_height / BLOCK_SIZE;

  for(unsigned i = 0; i < num_devices; ++i) {
    // Command queue.
    queue[i] = clCreateCommandQueue(context, device[i], CL_QUEUE_PROFILING_ENABLE, &status);
    checkError(status, "Failed to create command queue");

    // Kernel.
    const char *kernel_name = "kernel_inputA_run_on_device";
    kernel[i] = clCreateKernel(program, kernel_name, &status);
    checkError(status, "Failed to create kernel");

    // Determine the number of rows processed by this device.
    // First do this computation in block-rows.
    rows_per_device[i] = num_block_rows / num_devices; // this is the number of block-rows

    // Spread out the remainder of the block-rows over the first
    // N % num_devices.
    if(i < (num_block_rows % num_devices)) {
      rows_per_device[i]++;
    }

    // Multiply by BLOCK_SIZE to get the actual number of rows.
    rows_per_device[i] *= BLOCK_SIZE;

    // Input buffers.
    // For matrix A, each device only needs the rows corresponding
    // to the rows of the output matrix. We specifically
    // assign this buffer to the first bank of global memory.
    input_a_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE,
       rows_per_device[i] * A_width * sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for input A");

    // For matrix B, each device needs the whole matrix. We specifically
    // assign this buffer to the second bank of global memory.
    //input_b_buf[i] = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_CHANNEL_2_INTELFPGA,
    //    B_height * B_width * sizeof(float), NULL, &status);
    //checkError(status, "Failed to create buffer for input B");

    // Output buffer. This is matrix C, for the rows that are computed by this
    // device. We assign this buffer to the first bank of global memory,
    // although it is not material to performance to do so because
    // the reads from the input matrices are far more frequent than the
    // write to the output matrix.
    output_buf[i] = clCreateBuffer(context, CL_MEM_READ_WRITE,
       rows_per_device[i] * A_width * sizeof(float), NULL, &status);
    checkError(status, "Failed to create buffer for output");
  }

  return true;
}

// Initialize the data for the problem. Requires num_devices to be known.
void init_problem() {
  if(num_devices == 0) {
    checkError(-1, "No devices");
  }

  // Generate input matrices A and B. For matrix A, we divide up the host
  // buffers so that the buffers are aligned for each device. The whole of
  // matrix B is used by each device, so it does not need to be divided.
  printf("Generating input matrices\n");
  input_a.reset(num_devices);
  output.reset(num_devices);
  //unsigned count = 0 ;
  float count = 0 ;
  for(unsigned i = 0; i < num_devices; ++i) {
    input_a[i].reset(rows_per_device[i] * A_width*2);
    output[i].reset(rows_per_device[i] * C_width*2);

    for(unsigned j = 0; j < rows_per_device[i] * A_width*2; ++j) {
      input_a[i][j] = (count++) ; //rand_float();
      if (count > 9) count = 0;
      if (DEBUG) printf(" [%d][%d]=%.1f ", i,j,count);
    }
    if (DEBUG) printf("\n");
  }

  //input_b.reset(B_height * B_width);
  //for(unsigned i = 0; i < B_height * B_width; ++i) {
  //  input_b[i] = rand_float();
  //}
}

void run() {
  cl_int status;

#if USE_SVM_API == 0
  // Transfer inputs to each device. Each of the host buffers supplied to
  // clEnqueueWriteBuffer here is already aligned to ensure that DMA is used
  // for the host-to-device transfer.
  for(unsigned i = 0; i < num_devices; ++i) {
    printf("\n copying: %d, %d", rows_per_device[i] , A_width);
    status = clEnqueueWriteBuffer(queue[i], input_a_buf[i], CL_FALSE,
        0, rows_per_device[i] * A_width * sizeof(float), input_a[i], 0, NULL, NULL);
    checkError(status, "Failed to transfer input A");

    status = clEnqueueWriteBuffer(queue[i], output_buf[i], CL_FALSE,
        0, rows_per_device[i] * A_width * sizeof(float), input_a[i], 0, NULL, NULL);
    checkError(status, "Failed to transfer input A");
    //status = clEnqueueWriteBuffer(queue[i], input_b_buf[i], CL_FALSE,
    //    0, B_width * B_height * sizeof(float), input_b, 0, NULL, NULL);
    //checkError(status, "Failed to transfer input B");
  }

  // Wait for all queues to finish.
  for(unsigned i = 0; i < num_devices; ++i) {
    clFinish(queue[i]);
  }
#endif /* USE_SVM_API == 0 */

  // Launch kernels.
  // This is the portion of time that we'll be measuring for throughput
  // benchmarking.
  scoped_array<cl_event> kernel_event(num_devices);

  const double start_time = getCurrentTimestamp();
  for(unsigned i = 0; i < num_devices; ++i) {
    // Set kernel arguments.
    unsigned argi = 0;
    const int matSize = MATSIZE;

  const int _A_min_0 =0,  _A_min_1=0,  _A_min_2=0,        _A_stride_1= matSize;
  const int  _A_stride_2=matSize * matSize , _inputA_extent_0=matSize,_inputA_extent_1=matSize;
  const int _inputA_extent_2=2,   _inputA_min_0=0, _inputA_min_1 = 0,  _inputA_min_2=0;
  const int _inputA_stride_1=matSize,  _inputA_stride_2= matSize*matSize ;
    status = clSetKernelArg(kernel[i], argi++, sizeof(_A_min_0), &_A_min_0 );
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_A_min_1), &_A_min_1 );
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_A_min_2), &_A_min_2 );
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_A_stride_1), &_A_stride_1 );
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_A_stride_2), &_A_stride_2 );
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_extent_0), &_inputA_extent_0);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_extent_1), &_inputA_extent_1);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_extent_2), &_inputA_extent_2);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_min_0), &_inputA_min_0);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_min_1), &_inputA_min_1);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_min_2), &_inputA_min_2);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_stride_1), &_inputA_stride_1);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(_inputA_stride_2), &_inputA_stride_2);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &input_a_buf[i]);
    checkError(status, "Failed to set argument %d", argi - 1);

    status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &output_buf[i]);
    checkError(status, "Failed to set argument %d", argi - 1);

    //status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &input_a_buf[i]);
    //checkError(status, "Failed to set argument %d", argi - 1);

    //status = clSetKernelArg(kernel[i], argi++, sizeof(cl_mem), &input_b_buf[i]);
    //checkError(status, "Failed to set argument %d", argi - 1);

    //status = clSetKernelArg(kernel[i], argi++, sizeof(A_width), &A_width);
    //checkError(status, "Failed to set argument %d", argi - 1);

    //status = clSetKernelArg(kernel[i], argi++, sizeof(B_width), &B_width);
    //checkError(status, "Failed to set argument %d", argi - 1);

    // Enqueue kernel.
    // Use a global work size corresponding to the size of the output matrix.
    // Each work-item computes the result for one value of the output matrix,
    // so the global work size has the same dimensions as the output matrix.
    //
    // The local work size is one block, so BLOCK_SIZE x BLOCK_SIZE.
    //
    // Events are used to ensure that the kernel is not launched until
    // the writes to the input buffers have completed.
    const size_t global_work_size[2] = {1,1};//{C_width, rows_per_device[i]};
    const size_t local_work_size[2]  = {1,1};//{BLOCK_SIZE, BLOCK_SIZE};
    printf("Launching for device %d (global size: %zd, %zd)\n", i, global_work_size[0], global_work_size[1]);

    status = clEnqueueNDRangeKernel(queue[i], kernel[i], 1, NULL,
        global_work_size, local_work_size, 0, NULL, &kernel_event[i]);
    checkError(status, "Failed to launch kernel");
  }

  // Wait for all kernels to finish.
  clWaitForEvents(num_devices, kernel_event);

  const double end_time = getCurrentTimestamp();
  const double total_time = end_time - start_time;

  // Wall-clock time taken.
  printf("\nTime: %0.3f ms\n", total_time * 1e3);

  // Get kernel times using the OpenCL event profiling API.
  for(unsigned i = 0; i < num_devices; ++i) {
    cl_ulong time_ns = getStartEndTime(kernel_event[i]);
    printf("Kernel time (device %d): %0.3f ms\n", i, double(time_ns) * 1e-6);
  }

  // Compute the throughput (GFLOPS).
  // There are C_width * C_height output values, with each value
  // computed using A_width multiplies and adds.
  const float flops = (float)(2.0f * C_width * C_height * A_width / total_time);
  printf("\nThroughput: %0.2f GFLOPS\n\n", flops * 1e-9);

  // Release kernel events.
  for(unsigned i = 0; i < num_devices; ++i) {
    clReleaseEvent(kernel_event[i]);
  }

  // Read the result.
  for(unsigned i = 0; i < num_devices; ++i) {
    printf(" \n copying out %d %d ", rows_per_device[i] , C_width);
#if USE_SVM_API == 0
    status = clEnqueueReadBuffer(queue[i], output_buf[i], CL_TRUE,
        0, rows_per_device[i] * C_width * sizeof(float), output[i], 0, NULL, NULL);
    checkError(status, "Failed to read output matrix");
#else
    status = clEnqueueSVMMap(queue[i], CL_TRUE, CL_MAP_READ,
        (void *)output[i], rows_per_device[i] * C_width * sizeof(float), 0, NULL, NULL);
    checkError(status, "Failed to map output");
#endif /* USE_SVM_API == 0 */
  }

  // Verify results.
  //compute_reference();
  verify();
}
#define DEBUG 0
void compute_reference() {
  // Compute the reference output.
  printf("Computing reference output\n");
  ref_output.reset(C_height * C_width);

//void simple_version(float* A, float *B, float *C, int width, int height , int Radius) {
 int width=MATSIZE , height=MATSIZE, Radius=STENCIL_RADIUS, num_time_steps=2 ;
  for (int t = 0 ; t < num_time_steps ; t++) {
    printf("\n ");
    float *temp ; 
    temp = new float(height*width);
    for (int i = Radius  ; i < height-Radius; i ++) {
      for (int j = Radius ; j < width-Radius; j ++) {
        float reduceTemp = 0 ;         
        for (int R = -Radius   ; R <= Radius  ; R++) {
          //reduceTemp += C[i][j+R] + C[i+R][j];
          reduceTemp += input_a[0][(i+R)*width+j] +input_a[0][(i)*width + j+R];
          if (DEBUG) printf(" %.1f+%.1f ",input_a[0][(i+R)*width+j] ,input_a[0][(i)*width + j+R]);
        }
        if (DEBUG) printf(" =%.1f (%d,%d) ",reduceTemp, i,j);
        temp[i*width+j] = reduceTemp;
      }
    }
    if (DEBUG) printf("\n");
    for (int i = 0 ; i < height; i ++) {
      for (int j = 0 ; j < width; j ++) {
       input_a[0][i*width+j] = temp[i*width+j];
        if (DEBUG) printf ("\t C[%d,%d] = %.1f",i,j,input_a[0][i*width+j]);
      }
    if (DEBUG) printf("\n");
    }
  }
  return;
  for(unsigned y = 0, dev_index = 0; y < C_height; ++dev_index) {
    for(unsigned yy = 0; yy < rows_per_device[dev_index]; ++yy, ++y) {
      for(unsigned x = 0; x < C_width; ++x) {
        // Compute result for C(y, x)
        float sum = 0.0f;
        for(unsigned k = 0; k < A_width; ++k) {
          sum += input_a[dev_index][yy * A_width + k] * input_b[k * B_width + x];
        }
        ref_output[y * C_width + x] = sum;
      }
    }
  }
}

void verify() {
  printf("Verifying\n");

  unsigned height=10, width=10;
  for (int t=0; t < 2; t++) {
    for (int i = 0 ; i < height; i ++) {
      for (int j = 0 ; j < width; j ++) {
        if (DEBUG) printf(" out[%d][%d]=%.1f ", i, j, output[0][t*10*10+i*10+j]);
      }
      if (DEBUG) printf("\n");
    }
  }
  return;
  // Compute the L^2-Norm of the difference between the output and reference
  // output matrices and compare it against the L^2-Norm of the reference.
  float diff = 0.0f;
  float ref = 0.0f;
  for(unsigned y = 0, dev_index = 0; y < C_height; ++dev_index) {
    for(unsigned yy = 0; yy < rows_per_device[dev_index]; ++yy, ++y) {
      for(unsigned x = 0; x < C_width; ++x) {
        const float o = output[dev_index][yy * C_width + x];
        if (DEBUG) printf(" [%d][%d]=%.1f ", yy, x, o);
        if (( yy * C_width + x ) > 500) break;
        const float r = ref_output[y * C_width + x];
        if (DEBUG) printf("REf [%d][%d]=%.1f ", y, x, o);
        const float d = o - r;
        diff += d * d;
        ref += r * r;
      }
      if (DEBUG) printf(" \n ");
    }
  }

  if (DEBUG) printf("Verification: %.1f,%.1f \n", diff, ref );
}

// Free the resources allocated during initialization
void cleanup() {
  for(unsigned i = 0; i < num_devices; ++i) {
    if(kernel && kernel[i]) {
      clReleaseKernel(kernel[i]);
    }
    if(queue && queue[i]) {
      clReleaseCommandQueue(queue[i]);
    }
#if USE_SVM_API == 0
    if(input_a_buf && input_a_buf[i]) {
      clReleaseMemObject(input_a_buf[i]);
    }
    if(output_buf && output_buf[i]) {
      clReleaseMemObject(output_buf[i]);
    }
#else
    if(input_a[i].get())
      input_a[i].reset();
    if(output[i].get())
      output[i].reset();
#endif /* USE_SVM_API == 0 */
  }
#if USE_SVM_API == 1
  if(input_b.get())
    input_b.reset();
#endif /* USE_SVM_API == 1 */

  if(program) {
    clReleaseProgram(program);
  }
  if(context) {
    clReleaseContext(context);
  }
}

