#!/bin/bash

#PBS -l select=2:ncpus=10:mem=1mb
#PBS -l walltime=0:05:00
#PBS -q short_cpuQ

# Change to the directory where the job was submitted
cd $PBS_O_WORKDIR

# Load MPI environment
module load mpich-3.2

# Compile the MPI program
mpicc -g -Wall -o mpi_hello main.c

# Run the MPI program with N processes
mpiexec -n 100 ./mpi_hello
