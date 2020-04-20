#pragma once
#include <MoodyCamel/concurrentqueue.h>
#include "EgcInternalEvent.h"
using moodycamel::ConcurrentQueue;

namespace EgcCommon {

  struct MyTraits : public moodycamel::ConcurrentQueueDefaultTraits {
    static const size_t BLOCK_SIZE =
      32 * sizeof(EgcTrans::IEgcInnerEvt); // Use bigger blocks
  };

  /* Queue */
  typedef void* egc_queue_t;
  typedef void (*egc_queue_elem_free_t)(void*);

  class Queue : public moodycamel::ConcurrentQueue<void*> {
  protected:
    egc_queue_elem_free_t dtor_;
  public:
    Queue(egc_queue_elem_free_t dtor) : moodycamel::ConcurrentQueue<void*>()
    {
      dtor_ = dtor;
    }

    ~Queue()
    {
      void* ptr;
      while (try_dequeue(ptr))
      {
        FreeElement(ptr);
      }
    }

    inline void FreeElement(void* elem)
    {
      if (dtor_ != nullptr) { dtor_(elem); }
    }
  };
}
