#pragma once

#include <cstddef>
#include <initializer_list>
#include <tuple>

#include "common/function_ref.h"

namespace lightstep {
using Fragment = std::pair<void*, int>;

/**
 * Interface for iterating over sequences of fragments.
 */
class FragmentInputStream {
 public:
  using Callback = bool(void* data, int size);

  FragmentInputStream() noexcept = default;
  FragmentInputStream(const FragmentInputStream&) noexcept = default;
  FragmentInputStream(FragmentInputStream&&) noexcept = default;

  virtual ~FragmentInputStream() noexcept = default;

  FragmentInputStream& operator=(const FragmentInputStream&) noexcept = default;
  FragmentInputStream& operator=(FragmentInputStream&&) noexcept = default;

  /**
   * @return the number of fragments in the set.
   */
  virtual int num_fragments() const noexcept = 0;

  /**
   * @return true if the set contains no fragments.
   */
  bool empty() const noexcept { return num_fragments() == 0; }

  /**
   * Iterate over each fragment in the set.
   * @param callback a callback to call for each fragment. The callback can
   * return false to break out of the iteration early.
   * @return false if the iteration was interrupted early.
   */
  virtual bool ForEachFragment(FunctionRef<Callback> callback) const
      noexcept = 0;

  /**
   * Repositions the stream to the end of all the fragments.
   */
  virtual void Clear() noexcept = 0;

  /**
   * Adjust the position of the fragment stream.
   * @param fragment_index the new fragment index to reposition to relative to
   * the current fragment index.
   * @param position the position within fragment_index to reposition to
   * relative to the current position.
   */
  virtual void Seek(int fragment_index, int position) noexcept = 0;
};

/**
 * Determine which fragment an index lies in.
 * @param fragment_input_streams a list of fragment sets.
 * @param n an index into the fragment sets.
 * @return a tuple identifying the fragment set, fragment, and position of n.
 */
std::tuple<int, int, int> ComputeFragmentPosition(
    std::initializer_list<const FragmentInputStream*> fragment_input_streams,
    int n) noexcept;

bool Consume(std::initializer_list<FragmentInputStream*> fragment_input_streams,
             int n) noexcept;
}  // namespace lightstep
