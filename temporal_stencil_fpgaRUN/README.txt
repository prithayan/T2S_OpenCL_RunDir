#

Intel FPGA® OpenCL™ Design Example

Matrix Multiplication

This readme file for the Matrix Multiplication OpenCL Design Example contains
information about the design example package. For more examples, please visit
the [ Intel FPGA OpenCL Design Examples page](https://www.altera.com/products
/design-software/embedded-software-developers/opencl/developer-zone.html).

## Contents

  * Description
  * Software & Hardware Requirements
  * Package Contents
  * Compiling the OpenCL Kernel
  * Compiling the Host Program
  * Running the Host Program
  * Release History
  * Legal
  * Contacting Intel

## Description

This example provides a kernel that implements the standard matrix
multiplication operation C = A × B, where

  * A is a N × K matrix, and
  * B is a K × M matrix, and
  * C is a N × M matrix.

The kernel uses the standard loop tiling optimization to take advantage of
data reuse among different output values. Each tile has the dimensions of
BLOCK_SIZE × BLOCK_SIZE, where the value of BLOCK_SIZE is fixed at kernel
compilation time.

## Software & Hardware Requirements

Requirement Version OpenCL KernelHost Program

Hardware  
CompileEmulation  
CompileHardwareEmulation

CompileRunCompileRun

Quartus Prime Design Software (Quartus II)

16.1 or later

✓

✓

Intel(R) FPGA SDK for OpenCL(TM)

16.1 or later

✓

✓

✓

(either)

✓

(either)

✓

(either)

✓

(either)

Intel(R) FPGA Runtime Environment for OpenCL(TM)

16.1 or later

Board Support Package

16.1-compatible

✓

✓

✓

✓

✓

✓

Board Hardware

-
✓

gcc

4.4.7 or later

✓

✓

✓

✓

GNU Make

3.8.1 or later

✓

✓

## Package Contents

Path Description

[matrix_mult/](./)

[Makefile](./Makefile)

Makefile for host program

[bin/](./bin/)

Host program, AOCX files

[device/](./device/)

OpenCL kernel files

[matrix_mult.cl](./device/matrix_mult.cl)

Top-level OpenCL kernel file

[host/](./host/)

[inc/](./host/inc/)

Host include files

[src/](./host/src/)

Host source files

## Compiling the OpenCL Kernel

The top-level OpenCL kernel file is device/matrix_mult.cl.

To compile the OpenCL kernel, run:

aoc device/matrix_mult.cl -o bin/matrix_mult.aocx -fp-relaxed -fpc -no-
interleaving=default \--board <_board_>

where <_board_> matches the board you want to target. The -o
bin/matrix_mult.aocx argument is used to place the compiled binary in the
location that the host program expects.

If you are unsure of the boards available, use the following command to list
available boards:

aoc --list-boards

### Compiling for Emulator

To use the emulation flow, the compilation command just needs to be modified
slightly:

aoc -march=emulator device/matrix_mult.cl -o bin/matrix_mult.aocx -fp-relaxed
-fpc -no-interleaving=default \--board <_board_>

### Kernel Preprocessor Definitions

The kernel has the following preprocessor definitions:

Define Type Default Description

-DBLOCK_SIZE=<_#_>
Optional

64

The OpenCL kernel is a blocked matrix multiplication with a block size of
BLOCK_SIZE × BLOCK_SIZE. The value of BLOCK_SIZE affects the amount of on-chip
memory used and the performance of the kernel. Higher values lead to higher
performance. The same value must be used for kernel and host compilation.

-DSIMD_WORK_ITEMS=<_#_>
Optional

4

This controls the num_simd_work_items kernel attribute. The value should be
set based on available resources on the FPGA device. Higher values lead to
higher performance.

## Compiling the Host Program

To compile the host program, run:

make

The compiled host program will be located at bin/host.

### Host Preprocessor Definitions

The host program has the following preprocessor definitions:

Define Type Default Description

-DBLOCK_SIZE=<_#_>
Optional

64

This value must be the same as the one used when compiling the kernel.

-DUSE_SVM_API=<_#_>
Optional

0

This option when set to 1 will use the OpenCL 2.0 shared virtual memory (SVM)
API.

On Linux, custom values for preprocessor defines can be specified by setting
the value of CPPFLAGS when invoking the Makefile.

## Running the Host Program

Before running the host program, you should have compiled the OpenCL kernel
and the host program. Refer to the above sections if you have not completed
those steps.

To run the host program on hardware, execute:

bin/host

### Running with the Emulator

Prior to running the emulation flow, ensure that you have compiled the kernel
for emulation. Refer to the above sections if you have not done so. Also,
please set up your environment for emulation. Please see the [Intel(R) FPGA
SDK for OpenCL(TM) Programming Guide](http://www.altera.com/literature/hb
/opencl-sdk/aocl_programming_guide.pdf) for more information.

For this example design, the suggested emulation command is:

CL_CONTEXT_EMULATOR_DEVICE_INTELFPGA=1 bin/host -ah=512 -aw=512 -bw=512

### Host Parameters

The general command-line for the host program is:

bin/host [-ah=<_#_>] [-aw=<_#_>] [-bw=<_#_>]

where the parameters are:

Parameter Type Default Description

-ah=<_#_>
Optional

32 × BLOCK_SIZE

Height of matrix A. Must be a multiple of BLOCK_SIZE.

-aw=<_#_>
Optional

16 × BLOCK_SIZE

Width of matrix A. Must be a multiple of BLOCK_SIZE.

-bw=<_#_>
Optional

16 × BLOCK_SIZE

Width of matrix B. Must be a multiple of BLOCK_SIZE.

### OpenCL Binary Selection

The host program requires a OpenCL binary (AOCX) file to run. For this example
design, OpenCL binary files should be placed in the bin directory.

By default, the host program will look for a binary file in the following
order (earlier pattern matches take priority):

  1. A file named matrix_mult.aocx.
  2. A file named matrix_mult_<_board_>_161.aocx, where <_board_> is the name of the board (as passed as the \--board argument to aoc).

## Release History

Example Version SDK Version Date Changes

1.5

16.0

November 2016

  * Add SVM API option.

1.4

16.0

June 2016

  * Fixed makefile.

1.3

14.1

December 2014

  * New readme documentation.
  * Provide suggested emulation-specific arguments.

1.2

14.0

July 2014

  * Update kernel compilation flags documentation for 14.0 release.

1.1

13.1

January 2014

  * On Linux, fix possible compilation issues (missing include files).

1.0

13.1

December 2013

  * First release of example.

## Legal

    
    
    Copyright (C) 2013-2018 Altera Corporation, San Jose, California, USA. All rights reserved.
    Permission is hereby granted, free of charge, to any person obtaining a copy of this
    software and associated documentation files (the "Software"), to deal in the Software
    without restriction, including without limitation the rights to use, copy, modify, merge,
    publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to
    whom the Software is furnished to do so, subject to the following conditions:
    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.
    
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
    OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
    OTHER DEALINGS IN THE SOFTWARE.
    
    This agreement shall be governed in all respects by the laws of the State of California and
    by the laws of the United States of America.
    

### Trademarks

OpenCL and the OpenCL logo are trademarks of Apple Inc. used by permission by
Khronos.

Product is based on a published Khronos Specification, and has passed the
Khronos Conformance Testing Process. Current conformance status can be found
at [www.khronos.org/conformance](www.khronos.org/conformance).

## Contacting Intel

Although we have made every effort to ensure that this design example works
correctly, there might be problems that we have not encountered. If you have a
question or problem that is not answered by the information provided in this
readme file or the example's documentation, please contact Intel support
([myAltera](http://www.altera.com/myaltera)).

