// Copyright (c) 2000 - 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Portable implementation - just use glibc
//
// Note:  The glibc implementation may cause a call to malloc.
// This can cause a deadlock in HeapProfiler.
#include <execinfo.h>
#include <string.h>
#include "stacktrace.h"

_START_GOOGLE_NAMESPACE_

int GetStackTrace(void** result, int max_depth, int skip_count) {
  static thread_local bool is_collecting_stack = false;

  if (is_collecting_stack) {
    // It might be unsafe to call backtrace recursively. Return an empty
    // stack trace. A thread can get here if while it was collecting its own
    // stack, it got interrupted by a signal request from another thread.
    return 0;
  }

  static const int kMaxStackDepth = 128;
  skip_count++;  // we want to skip the current frame as well
  if (skip_count > kMaxStackDepth) {
    return 0;
  }
  skip_count = std::max(skip_count, 0);
  max_depth = std::max(max_depth, 0);
  int capacity = std::min(max_depth + skip_count, kMaxStackDepth);
  void** stack = reinterpret_cast<void**>(alloca(capacity * sizeof(void*)));

  is_collecting_stack = true;
  int size = backtrace(stack, capacity);
  is_collecting_stack = false;

  int result_count = std::max(size - skip_count, 0);
  if (result_count > 0) {
    memcpy(result, stack + skip_count, sizeof(void*) * result_count);
  }
  return result_count;
}

_END_GOOGLE_NAMESPACE_
