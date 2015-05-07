// Author: David Siegal
// Date: April 16, 2015
//
// This algorithm is meant to demonstrate how to create an iterative algorithm
// - with both initial setup and output data generation.  The algorithm 
// iterates through a data set and counts the number of times a particular 
// number is read.  In essence, it is a word count (looping) that uses numbers
// instead of words.  This process is performed iteratively - solely for 
// the purpose of example.  Thus, if the input has 12 instances of the 
// number 42 and is set to iterate 3 times, the output will show 42 was 
// encountered 36 times.
// NOTE: In order to set up input / output objects, the number of distinct 
//       numbers that will be encountered should be known.

#include <memory>
#include <mpi.h>
#include <typeinfo>

#include "netcdf_partitioner.h"
#include "num_count_iterative.h"
#include "partitioner.h"
#include "scheduler.h"

#define NUM_THREADS 2  // The # of threads for analytics task.

#define STEP  1  // process each number individually
#define NUM_ELEMS 32  // The total number of elements of the simulated data.
#define NUM_DISTINCT_ELEMS 3  // The total number of elements of the simulated data.
#define NUM_ITERS 2  // The # of iterations.

#define PRINT_COMBINATION_MAP 1
#define PRINT_OUTPUT 1

#define FILENAME  "data2.nc"
#define VARNAME "point"

using namespace std;

int main(int argc, char* argv[]) {
  // MPI initialization.
  int mpi_status = MPI_Init(&argc, &argv);
  if (mpi_status != MPI_SUCCESS) {
    printf("Failed to initialize MPI environment.\n");
    MPI_Abort(MPI_COMM_WORLD, mpi_status);
  }

  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // Load the data partition.
  unique_ptr<Partitioner> p(new NetCDFPartitioner(FILENAME, VARNAME, STEP));
  p->load_partition();

  // The output is a 1D array that indicates how many times the index was encountered.
  // eg {12,10,1} would indicate 0 was encountered 12 times,
  // 1 was encountered 10 times, and 2 was encountered 1 time
  const size_t out_len = NUM_DISTINCT_ELEMS;
  size_t* out = new size_t[out_len];
  for (size_t i = 0; i < out_len; ++i) {
    out[i] = 0;
  }

  // Set up the initial counts.
  size_t* counts = new size_t[out_len];
  for (size_t i = 0; i < out_len; ++i) {
    counts[i] = 0;
  }
  

  SchedArgs args(NUM_THREADS, STEP, counts, NUM_ITERS);
  unique_ptr<Scheduler<int, size_t>> numCountIt(new NumCountIt<int, size_t>(args));   
  numCountIt->set_red_obj_size(sizeof(CountRedObj));
  numCountIt->run((const int*)p->get_data(), p->get_len(), out, out_len);

  // Print out the combination map if required.
  if (PRINT_COMBINATION_MAP && rank == 0) {
    printf("\n");
    numCountIt->dump_combination_map();
  }

  // Print out the final result on the master node if required.
  if (PRINT_OUTPUT && rank == 0) {
    printf("Final output on the master node:\n");
    for (size_t i = 0; i < out_len; ++i) {
      printf("number %lu was encountered %lu times.",i , out[i]);
      printf("\n");
    }
    printf("\n");
  }

  delete [] out;
  delete [] counts;
  MPI_Finalize();

  return 0;
}
