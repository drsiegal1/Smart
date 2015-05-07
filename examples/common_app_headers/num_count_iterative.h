#ifndef	_NUM_COUNT_IT_H_
#define	_NUM_COUNT_IT_H_
#include <cassert>
#include <cfloat>
#include <cmath>
#include <cstring>
#include <memory>

#include "chunk.h"
#include "scheduler.h"

#define NUM_DISTINCT_ELEMS 3  // The total number of elements of the simulated data.


using namespace std;

struct CountRedObj : public RedObj {
  size_t total = 0;  
  // Optional, only used for rendering the result.
  string str() const override {
    return string("(") + "total = " + to_string(total) + ")";
  }
};

template <class In, class Out>
  class NumCountIt : public Scheduler<In, Out> {
 public:
    using Scheduler<In, Out>::Scheduler;

    // Group elements into buckets.
    int gen_key(const Chunk& chunk, const In* data, const map<int, unique_ptr<RedObj>>& combination_map) const override {
      // data_ is an arrau from the  data partitioner passed in the constructor.
      printf("generated key %d for chunk %d\n",(int)(this->data_[chunk.start]),this->data_[chunk.start]);
      return (int)(this->data_[chunk.start]);
    }

    // Accumulate sum and count from chunk into the reduction object
    void accumulate(const Chunk& chunk, const In* data, unique_ptr<RedObj>& red_obj) override {
      if (red_obj == nullptr) {
	red_obj.reset(new CountRedObj);
      }
      // cast the reduction object to the specific reduction object created for this alg.
      CountRedObj* ro = static_cast<CountRedObj*>(red_obj.get());  
      // iterate the reduction objec for the chunk (differentiated by gen_key result)
      size_t ro_before = ro->total;

      ro->total++;
      printf("Before red, total = %lu; After local reduction for block = %d, total = %lu.\n", ro_before, this->data_[chunk.start],ro->total); 
    }

    // Merge sum and size.
    void merge(const RedObj& red_obj, unique_ptr<RedObj>& com_obj) override {
      const CountRedObj* red = static_cast<const CountRedObj*>(&red_obj);
      CountRedObj* com = static_cast<CountRedObj*>(com_obj.get());
      size_t red_before = red->total;size_t com_before = com->total;
      com->total = (com->total) + (red->total);
      printf("Red before = %lu, Com before = %lu, After merge, total = %lu.\n", red_before, com_before, com->total);
    }

    // Deserialize reduction object. 
    void deserialize(unique_ptr<RedObj>& obj, const char* data) const override {
      obj.reset(new CountRedObj);
      memcpy(obj.get(), data, sizeof(CountRedObj));
    }

    // Convert a reduction object into a desired output element.
    void convert(const RedObj& red_obj, Out* out) const override {
      const CountRedObj* ro = static_cast<const CountRedObj*>(&red_obj);
      // clarify: believe this is setting out cooresponding to the key in the combination map.
      *out = ro->total;
    }

    /* Additional Function Overriding */
    // Set up the initial centroids in combination_map_.
    /*
    void process_extra_data(const void* extra_data, map<int, unique_ptr<RedObj>>& combination_map) override {
      dprintf("Scheduler: Processing extra data...\n");

      assert(this->extra_data_ != nullptr);

      // the initial counts - for example if you ran this on another data
      // set and aleardy saw 0 12 times, should be passed in extra data
      const Out* initialCounts = (const Out*)this->extra_data_;

      for (int i = 0; i < NUM_DISTINCT_ELEMS; ++i) {
	// Initialize the result cluster with the initial centroids
	unique_ptr<CountRedObj> cluster_obj(new CountRedObj);
	cluster_obj->total = initialCounts[i];

	// Update combination_map_.
	this->combination_map_[i] = move(cluster_obj);

	printf("combination_map_[%d] = %s\n", i, this->combination_map_[i]->str().c_str());
      }
    }
    */

    /*
     * I don't think I need to do implement this because no real processing is needed after each iteration.
     */
    // Finalize combinaion_map_.
    // void post_combine(map<int, unique_ptr<RedObj>>& combination_map) override {
    //   for (auto& pair : this->combination_map_) {
    //     ClusterObj<T>* cluster_obj = static_cast<ClusterObj<T>*>(pair.second.get());         
    //     // Update the centroids for each cluster.
    //     cluster_obj->update_centroid();
    //   }
    //   printf("Local combination map before cluster object clearance.\n");
    //   this->dump_combination_map();
    // }
};
#endif  // _NUM_COUNT_IT_
