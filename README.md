# Homomorphic Compression: Making Text Processing on Compression Unlimited

## Introduction

Homomorphic compression is a new theory of compression. It generalizes and characerizes compression algorithms with direct processing capability in a principled way. Based on this theory, we present HOCO, a text data management system that supports operating on compressed data. 

HOCO currently includes three HC schemes with different compression ratios and direct processing abilities. This directory contains code of the three HC schemes implemented in HOCO. The code is used for reproduce the results shown in the HOCO paper. 

It contains scripts that allow you to compress data from scratch and perform manipulation and analytic tasks. 

## Organization of Supplementary Materials
- **Appendix**: `Appendix_for_Homomorphic_Compression.pdf`.
- **Data**: `input.tar.gz`.
- **HOCO system**: `HOCO/`.
- **Experimental evaluation**: 1) scripts: `src/`, 2) outcomes: `results/`.

## Experimental Setup

We evaluate the three HC schemes on a platform which is equipped with an Intel(R) Core(TM) i7-12700K@3.60 GHz CPU and 128GB memory. The operating system of the platform is Ubuntu 20.04.4.

## Run example

The source code of the three HC schemes is in the directory `src/Compress_methods`, and the code of operating on uncompressed data is in the directory `src/Origin`. 

Run the code as follows.

1. **Get datasets.**

   ```shell
   tar -xzvf input.tar.gz
   ```

   Unzip the compressed package `input.tar.gz`. The uncompressed dataset `COV19` will be located under the `/input/Original_data` directory. 

2. **Compress the data with three HC schemes.** 

   ```shell
   cd HOCO_SIGMOD24/src/scripts
   bash run_compress.sh
   ```

   The default storage path for compressed text is `input/Compressed_data`. You can modify the script to specify an alternative path.

3. **Run manipulation operations.** The following script prompts the three HC schemes to perform extract, insert, and delete operations on all the datasets. It also performs the above operations on uncompressed text as baselines.

   ```shell
   cd HOCO_SIGMOD24/src/scripts
   bash run_manipulate.sh
   ```

4. **Run analytic tasks.** The following script prompts the three HC schemes to perform word count, inverted index, and sequence count tasks on all the datasets. It also performs the above tasks on uncompressed text as baselines.

   ```shell
   cd HOCO_SIGMOD24/src/scripts
   bash run_analyze.sh
   ```

By default, the results of the scripts are output to the directory `results`. We have placed our results under this directory for reference. 

## Test HOCO Framework
The prototype code for the HOCO framework can be found in the  `HOCO/` directory. Please note that the code provided serves as a demonstration of the system's key utility and functionality, showcasing the organization and operation of each module. We will open-source the complete system for reuse by the community upon the paper's acceptance.

Test the system as follows.

1. **Setting up**. To ensure proper functioning of the system, it is necessary to have the Boost library installed. You can install it using the apt-get package manager or by visiting the official website to obtain the latest Boost package and following the provided installation instructions.
   ```shell
   # Option1: install with apt-get
   sudo apt-get install libboost-all-dev
   # Option2: install with source code
   wget https://boostorg.jfrog.io/artifactory/main/release/1.82.0/source/boost_1_82_0.tar.gz
   tar -xzvf boost_1_82_0.tar.gz
   cd boost_1_82_0/
   ./bootstrap.sh
   sudo ./b2 install
   ```


2. **Compile the system.** Execute the following commands to generate executable test files for each module. The test files will be generated in the `build/` directory.
   ```shell
   cd HOCO/
   mkdir build/ && cd build/
   cmake ..
   make
   ```

3. **Test the modules.** To test each module, use ctest by running the following command in the terminal:
   ```shell
   ctest
   ```
   This command will initiate the testing process, and the terminal will display results similar to the following:
   ```shell
   Test project /Your/Path/To/HOCO_SIGMOD24/HOCO/build
      Start 1: CompressModuleTest
   1/3 Test #1: CompressModuleTest ...............   Passed   10.98 sec
      Start 2: EvaluateModuleTest
   2/3 Test #2: EvaluateModuleTest ...............   Passed    2.90 sec
      Start 3: SchemeCollectModuleTest
   3/3 Test #3: SchemeCollectModuleTest ..........   Passed   29.41 sec

   100% tests passed, 0 tests failed out of 3

   Total Test time (real) =  43.31 sec
   ```
   You can find the process outputs at `Your/Path/To/HOCO_SIGMOD24/HOCO/build/Testing/Temporary/LastTest.log`.