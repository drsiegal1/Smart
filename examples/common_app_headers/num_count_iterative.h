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
  size_t count_cur_iter = 0;  
  size_t count_so_far = 0;  
  /*
   *THIS IS NEEDED for iterative programs.  the default clone does not make a 'deep copy' and thus 
   *the the combination map is not distributed correctly if this method is not included.
   */ 
  RedObj* clone() override {
    return new CountRedObj(*this);
  }
  // Optional, only used for rendering the result.
  string str() const override {
    return string("(") + "count_cur_iter = " + to_string(count_cur_iter) + string("; count_so_far = ") + to_string(count_so_far)+ ")";
  }
};

template <class In, class Out>
  class NumCountIt : public Scheduler<In, Out> {
 public:
    using Scheduler<In, Out>::Scheduler;

    // Group elements into buckets.
    int gen_key(const Chunk& chunk, const In* data, const map<int, unique_ptr<RedObj>>& combination_map) const override {
      printf("generated key %d for chunk %d\n",(int)(data[chunk.start]),data[chunk.start]);
      return (int)(data[chunk.start]);
    }

    // Accumulate sum and count from chunk into the reduction object
    void accumulate(const Chunk& chunk, const In* data, unique_ptr<RedObj>& red_obj) override {
      if (red_obj == nullptr) {
	red_obj.reset(new CountRedObj);
      }
      // cast the reduction object to the specific reduction object created for this alg.
      CountRedObj* ro = static_cast<CountRedObj*>(red_obj.get());  
      // iterate the reduction objec for the chunk (differentiated by gen_key result)
      size_t ro_before = ro->count_cur_iter;

      ro->count_cur_iter++;
      printf("Before red, count_cur_iter = %lu; After local reduction for block = %d, count_cur_iter = %lu.\n", ro_before, data[chunk.start],ro->count_cur_iter); 
    }

    // Merge sum and size.
    void merge(const RedObj& red_obj, unique_ptr<RedObj>& com_obj) override {
      const CountRedObj* red = static_cast<const CountRedObj*>(&red_obj);
      CountRedObj* com = static_cast<CountRedObj*>(com_obj.get());
      size_t red_before = red->count_cur_iter;size_t com_before = com->count_cur_iter;
      com->count_cur_iter = (com->count_cur_iter) + (red->count_cur_iter);
      printf("Red before = %lu, Com before = %lu, After merge, count_cur_iter = %lu.\n", red_before, com_before, com->count_cur_iter);
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
      *out = ro->count_so_far;
    }

    /* Additional Function Overriding */
    // Set up the initial centroids in combination_map_. 
      void process_extra_data(const void* extra_data, map<int, unique_ptr<RedObj>>& combination_map) override {
      dprintf("Scheduler: Processing extra data...\n");

      assert(extra_data != nullptr);

      // the initial counts - for example if you ran this on another data
      // set and aleardy saw 0 12 times, should be passed in extra data
      const Out* initialCounts = (const Out*)extra_data;

      for (int i = 0; i < NUM_DISTINCT_ELEMS; ++i) {
	// Initialize the result cluster with the initial centroids
	unique_ptr<CountRedObj> cluster_obj(new CountRedObj);
	cluster_obj->count_so_far = initialCounts[i];

	// Update combination_map_.
	combination_map[i] = move(cluster_obj);

	printf("combination_map_[%d] = %s\n", i, combination_map[i]->str().c_str());
      }
    }

    /*
     * Combines this iteration into the total
     *
     */
     void post_combine(map<int, unique_ptr<RedObj>>& combination_map) override {
      // There should be only one key-value pair is stored in the combination map.
      printf("combination_map size = %lu\n", combination_map.size());
      for (auto& pair : combination_map) {
        CountRedObj* ro = static_cast<CountRedObj*>(pair.second.get()); 
        // Update count so far to include the 'last' iterations values.
        ro->count_so_far += ro->count_cur_iter;
        // reset current iteration counter
        ro->count_cur_iter = 0;
      }
      this->dump_combination_map();

     }
};
#endif  // _NUM_COUNT_IT_
