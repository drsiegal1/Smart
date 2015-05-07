#include <memory>
#include <mpi.h>
#include <typeinfo>

#include "naive_bayes.h"
#include "netcdf_partitioner.h"
#include "partitioner.h"
#include "scheduler.h"

#define NUM_THREADS 2  // The # of threads for analytics task.
#define STEP  4  // The size of unit chunk for each single read, which groups a bunch of elements for mapping and reducing. (E.g., for a relational table, STEP should equal the # of columns.) 
#define NUM_ELEMS 32  // The total number of elements of the simulated data.

#define PRINT_COMBINATION_MAP 1
#define PRINT_OUTPUT  1

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



  // Since the number of buckets in the histogram is unknown, here we do not
  // define an output array.
  SchedArgs args(NUM_THREADS, STEP);
  unique_ptr<Scheduler<int, size_t>> nb(new NaiveBayes<int>(args));
  nb->set_red_obj_size(sizeof(BayRedObj));
  nb->run((const int*)p->get_data(), p->get_len(), nullptr, 0);  // Note that here the output array is nullptr.

  if (PRINT_COMBINATION_MAP && rank == 0) {
    printf("\n");
    nb->dump_combination_map();
  }

  MPI_Finalize();
  return 0;
}

