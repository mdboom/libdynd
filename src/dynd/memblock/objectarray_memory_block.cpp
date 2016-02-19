//
// Copyright (C) 2011-15 DyND Developers
// BSD 2-Clause License, see LICENSE.txt
//

#include <stdexcept>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include <dynd/memblock/objectarray_memory_block.hpp>

using namespace std;
using namespace dynd;

intrusive_ptr<memory_block_data> dynd::make_objectarray_memory_block(const ndt::type &dt, const char *arrmeta,
                                                                     intptr_t stride, intptr_t initial_count,
                                                                     size_t arrmeta_size)
{
  objectarray_memory_block *pmb = new objectarray_memory_block(dt, arrmeta_size, arrmeta, stride, initial_count);
  return intrusive_ptr<memory_block_data>(reinterpret_cast<memory_block_data *>(pmb), false);
}

namespace dynd {
namespace detail {

  void free_objectarray_memory_block(memory_block_data *memblock)
  {
    objectarray_memory_block *emb = reinterpret_cast<objectarray_memory_block *>(memblock);
    delete emb;
  }

  static char *allocate(memory_block_data *self, size_t count)
  {
    //    cout << "allocating " << size_bytes << " of memory with alignment " << alignment << endl;
    // Allocate new POD memory of the requested size and alignment
    objectarray_memory_block *emb = reinterpret_cast<objectarray_memory_block *>(self);
    memory_chunk *mc = &emb->m_memory_handles.back();
    if (mc->capacity_count - mc->used_count < count) {
      emb->append_memory(max(emb->m_total_allocated_count, count));
      mc = &emb->m_memory_handles.back();
    }

    char *result = mc->memory + emb->m_stride * mc->used_count;
    mc->used_count += count;
    if ((emb->m_dt.get_flags() & type_flag_zeroinit) != 0) {
      memset(result, 0, emb->m_stride * count);
    }
    else {
      // TODO: Add a default data constructor to base_type
      //       as well, with a flag for it
      stringstream ss;
      ss << "Expected objectarray data to be zeroinit, but is not with dynd type " << emb->m_dt;
      throw runtime_error(ss.str());
    }
    return result;
  }

  static char *resize(memory_block_data *self, char *previous_allocated, size_t count)
  {
    objectarray_memory_block *emb = reinterpret_cast<objectarray_memory_block *>(self);
    memory_chunk *mc = &emb->m_memory_handles.back();
    size_t previous_index = (previous_allocated - mc->memory) / emb->m_stride;
    size_t previous_count = mc->used_count - previous_index;
    char *result = previous_allocated;

    if (mc->capacity_count - previous_index < count) {
      emb->append_memory(max(emb->m_total_allocated_count, count));
      memory_chunk *new_mc = &emb->m_memory_handles.back();
      // Move the old memory to the newly allocated block
      if (previous_count > 0) {
        // Subtract the previously used memory from the old chunk's count
        mc->used_count -= previous_count;
        memcpy(new_mc->memory, previous_allocated, previous_count);
        // If the old memory only had the memory being resized,
        // free it completely.
        if (previous_allocated == mc->memory) {
          free(mc->memory);
          // Remove the second-last element of the vector
          emb->m_memory_handles.erase(emb->m_memory_handles.begin() + emb->m_memory_handles.size() - 2);
        }
      }
      mc = &emb->m_memory_handles.back();
      result = mc->memory;
      mc->used_count = count;
    }
    else {
      // Adjust the used count (this may mean to grow it or shrink it)
      if (count >= previous_count) {
        mc->used_count += (count - previous_count);
      }
      else {
        // Call the destructor on the elements no longer used
        emb->m_dt.extended()->data_destruct_strided(emb->m_arrmeta + emb->arrmeta_size,
                                                    previous_allocated + emb->m_stride * count, emb->m_stride,
                                                    previous_count - count);
        mc->used_count -= (previous_count - count);
      }
    }

    if ((emb->m_dt.get_flags() & type_flag_zeroinit) != 0) {
      // Zero-init the new memory
      intptr_t new_count = count - (intptr_t)previous_count;
      if (new_count > 0) {
        memset(mc->memory + emb->m_stride * previous_count, 0, emb->m_stride * new_count);
      }
    }
    else {
      // TODO: Add a default data constructor to base_type
      //       as well, with a flag for it
      stringstream ss;
      ss << "Expected objectarray data to be zeroinit, but is not with dynd type " << emb->m_dt;
      throw runtime_error(ss.str());
    }
    return result;
  }

  static void finalize(memory_block_data *self)
  {
    // Finalizes POD memory so there are no more allocations
    objectarray_memory_block *emb = reinterpret_cast<objectarray_memory_block *>(self);

    emb->m_finalized = true;
  }

  static void reset(memory_block_data *self)
  {
    // Resets the POD memory so it can reuse it from the start
    objectarray_memory_block *emb = reinterpret_cast<objectarray_memory_block *>(self);

    if (emb->m_memory_handles.size() > 1) {
      // If there are more than one allocated memory chunks,
      // throw them all away except the last
      for (size_t i = 0, i_end = emb->m_memory_handles.size() - 1; i != i_end; ++i) {
        memory_chunk &mc = emb->m_memory_handles[i];
        emb->m_dt.extended()->data_destruct_strided(emb->m_arrmeta, mc.memory, emb->m_stride, mc.used_count);
        free(mc.memory);
      }
      emb->m_memory_handles.front() = emb->m_memory_handles.back();
      emb->m_memory_handles.resize(1);
      // Reset to zero used elements in the chunk
      memory_chunk &mc = emb->m_memory_handles.front();
      emb->m_dt.extended()->data_destruct_strided(emb->m_arrmeta, mc.memory, emb->m_stride, mc.used_count);
      mc.used_count = 0;
    }
  }

  memory_block_data::api objectarray_memory_block_allocator_api = {&allocate, &resize, &finalize, &reset};
}
} // namespace dynd::detail

void dynd::objectarray_memory_block_debug_print(const memory_block_data *memblock, std::ostream &o,
                                                const std::string &indent)
{
  const objectarray_memory_block *emb = reinterpret_cast<const objectarray_memory_block *>(memblock);
  o << " type: " << emb->m_dt << "\n";
  o << " stride: " << emb->m_stride << "\n";
  if (!emb->m_finalized) {
    o << indent << " allocated count: " << emb->m_total_allocated_count << "\n";
  }
  else {
    o << indent << " finalized count: " << emb->m_total_allocated_count << "\n";
  }
}
