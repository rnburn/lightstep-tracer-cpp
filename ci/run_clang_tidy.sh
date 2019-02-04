#!/bin/sh

set -e

CHECKS="\
-checks=*,\
-bugprone-use-after-move,\
-cert-err58-cpp,\
-fuchsia-*,\
-clang-analyzer-alpha.*,\
-clang-diagnostic-deprecated,\
-llvm-include-order,\
-llvm-header-guard,\
-google-runtime-references,\
-google-build-using-namespace,\
-google-runtime-int,\
-google-explicit-constructor,\
-modernize-make-unique,\
-modernize-use-transparent-functors,\
-performance-move-const-arg,\
-hicpp-explicit-conversions,\
-hicpp-member-init,\
-hicpp-invalid-access-moved,\
-hicpp-move-const-arg,\
-hicpp-member-init,\
-hicpp-vararg,\
-hicpp-signed-bitwise,\
-hicpp-no-array-decay,\
-cppcoreguidelines-pro-type-member-init,\
-cppcoreguidelines-owning-memory,\
-cppcoreguidelines-pro-type-reinterpret-cast,\
-cppcoreguidelines-pro-type-const-cast,\
-cppcoreguidelines-pro-bounds-array-to-pointer-decay,\
-cppcoreguidelines-pro-bounds-constant-array-index,\
-cppcoreguidelines-pro-bounds-pointer-arithmetic,\
-cppcoreguidelines-pro-type-vararg"

run-clang-tidy $CHECKS -clang-tidy-binary=ci/clang-tidy-wrapper.sh