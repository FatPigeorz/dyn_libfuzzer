//===- FuzzerExtFunctionsWeak.cpp - Interface to external functions -------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
// Implementation for Linux. This relies on the linker's support for weak
// symbols. We don't use this approach on Apple platforms because it requires
// clients of LibFuzzer to pass ``-U _<symbol_name>`` to the linker to allow
// weak symbols to be undefined. That is a complication we don't want to expose
// to clients right now.
//===----------------------------------------------------------------------===//
#include "FuzzerPlatform.h"
#if LIBFUZZER_LINUX || LIBFUZZER_NETBSD || LIBFUZZER_FUCHSIA ||                \
    LIBFUZZER_FREEBSD || LIBFUZZER_EMSCRIPTEN

#include "FuzzerExtFunctions.h"
#include "FuzzerIO.h"

extern "C" {
// Declare these symbols as weak to allow them to be optionally defined.
#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG, WARN)                            \
  __attribute__((weak, visibility("default"))) RETURN_TYPE NAME FUNC_SIG

#include "FuzzerExtFunctions.def"

#undef EXT_FUNC
}

using namespace fuzzer;

static void CheckFnPtr(void *FnPtr, const char *FnName, bool WarnIfMissing) {
  if (FnPtr == nullptr && WarnIfMissing) {
    Printf("WARNING: Failed to find function \"%s\".\n", FnName);
  }
}

static void* GetLibHandle() {
  dlerror(); // Clear any previous errors.
  const char* libraryName = std::getenv("LLAMUTA_MUTATOR");
  if (libraryName == nullptr) {
      return nullptr;
  } 
  void* libHandle = dlopen(libraryName, RTLD_LAZY);
  return libHandle;
}

template <typename T>
static T GetFnPtr(void* handle, const char *FnName) {
  void* Fn = dlsym(handle, FnName);
  if (Fn != nullptr) {
    Printf("INFO: Found function \"%s\".\n", FnName);
  } 
  return reinterpret_cast<T>(Fn);
}

namespace fuzzer {

ExternalFunctions::ExternalFunctions() {
#define EXT_FUNC(NAME, RETURN_TYPE, FUNC_SIG, WARN)                            \
  this->NAME = ::NAME;                                                         \
  if (this->NAME == nullptr && handle != nullptr && !WARN)                                                   \
    this->NAME = GetFnPtr<decltype(ExternalFunctions::NAME)>(handle, #NAME);     \
  CheckFnPtr(reinterpret_cast<void *>(reinterpret_cast<uintptr_t>(::NAME)),    \
             #NAME, WARN);

#include "FuzzerExtFunctions.def"

#undef EXT_FUNC
}

} // namespace fuzzer

#endif
