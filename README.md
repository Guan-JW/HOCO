# Homomorphic Compression: Making Text Processing on Compression Unlimited

## Introduction

Homomorphic compression is a new theory of compression. It generalizes and characerizes compression algorithms with direct processing capability in a principled way. Based on this theory, we present HOCO, a text data management system that supports operating on compressed data. 

HOCO currently includes three HC schemes with different compression ratios and direct processing abilities. This directory contains code of the three HC schemes implemented in HOCO. The code is used for reproduce the results shown in the HOCO paper. 

It contains scripts that allow you to compress data from scratch and perform manipulation and analytic tasks. 

## Experimental Setup

We evaluate the three HC schemes on a platform which is equipped an Intel(R) Core(TM) i7-12700K@3.60 GHz CPU and 128GB memory. The operating system of the platform is Ubuntu 20.04.4.

## Run example

The source code of the three HC schemes is in the directory `src/Compress_methods`, and the code of operating on uncompressed data is in the directory `src/Origin`. The uncompressed datasets are located in the directory `input/Original`, including 19NSR, COV19, DBLP, and a 2.1GB Wikipedia dataset.

Run the code as follows.

1. **Compress the data with three HC schemes.** 

   ```shell
   cd HOCO_SIGMOD24/src/scripts
   bash run_compress.sh
   ```

   The default storage path for compressed text is `input/Compressed_data`. You can modify the script to specify a alternative path.

2. **Run manipulation operations.** The following script prompts the three HC schemes to perform extract, insert, and delete operations on all the datasets. It also performs the above operations on uncompressed text as baselines.

   ```shell
   cd HOCO_SIGMOD24/src/scripts
   bash run_manipulate.sh
   ```

3. **Run analytic tasks.** The following script prompts the three HC schemes to perform word count, inverted index, and sequence count tasks on all the datasets. It also performs the above tasks on uncompressed text as baselines.

   ```shell
   cd HOCO_SIGMOD24/src/scripts
   bash run_analyze.sh
   ```

By default, the results of the scripts are output to the directory `results`. We have placed our results under this directory for reference. 

