#ifndef _NAIVE_BAYES_H_
#define _NAIVE_BAYES_H_

#include <cmath>
#include <memory>

#include "chunk.h"
#include "scheduler.h"

using namespace std;

// TODO should be modified to allow dynamic assignment
#define NUMBER_CLASSES 2
#define NUMBER_FEATURES 3

struct BayRedObj : public RedObj {
  // NOTE: CURRENT IMPLEMENTATION HAS FEATURE ARRAY - each feature is either 1-yes, or 0-not present
  // TODO future implementation should support many values for a feature (e.g. month number) 
  size_t featureMatrix[NUMBER_FEATURES];
  BayRedObj() {
    memset(featureMatrix, 0, NUMBER_FEATURES * sizeof(size_t));
  }

  size_t total = 0;  

  // Optional, only used for rendering the result.
  string str() const override {
    string str = "(features = [";
    for (size_t i = 0; i < NUMBER_FEATURES - 1; ++i) {
      str += to_string(featureMatrix[i]) + ", ";
    } 
    str += to_string(featureMatrix[NUMBER_FEATURES - 1]) + "]";
    return (str + " total = "+ to_string(total) + ")");
  }

};

template <class In>
class NaiveBayes : public Scheduler<In, size_t> {
public:
 using Scheduler<In, size_t>::Scheduler;

  int gen_key(const Chunk& chunk, const int* data, const map<int, unique_ptr<RedObj>>&combination_map) const override {
    printf("genkey call");
    return (int)data[chunk.start];
  }

  void accumulate(const Chunk& chunk, const int* data, unique_ptr<RedObj>& red_obj) override {
    if (red_obj == nullptr) {
      red_obj.reset(new BayRedObj);
    }

    BayRedObj* b = static_cast<BayRedObj*>(red_obj.get());  
    
    // update total
    b->total++;
    // update features
    for (size_t i = 0; i < NUMBER_FEATURES; ++i) {
      // if the feature is present, add one to the feature matrix
      // NOTE: + 1 since first element is class id
      // NOTE: CURRENT IMPLEMENTATION HAS FEATURE ARRAY - each feature is either 1-yes, or 0-not present
      // TODO future implementation should support many values for a feature (e.g. month number)
      size_t feature_start = chunk.start + 1;
      if(data[feature_start + i]){
        b->featureMatrix[i]++;
      }
    }

  }

  void merge(const RedObj& red_obj, unique_ptr<RedObj>& com_obj) override {
    const BayRedObj* br = static_cast<const BayRedObj*>(&red_obj);
    BayRedObj* bc = static_cast<BayRedObj*>(com_obj.get());
    // accum total
    bc->total += (br->total);
    // for each reduction object accumulate all features observed
    for (size_t i = 0; i < NUMBER_FEATURES; ++i) {
      bc->featureMatrix[i]+=(br->featureMatrix[i]);
    }
  }

  void deserialize(unique_ptr<RedObj>& obj, const char* data) const override {
    obj.reset(new BayRedObj);
    memcpy(obj.get(), data, sizeof(BayRedObj));
  }
};

#endif
