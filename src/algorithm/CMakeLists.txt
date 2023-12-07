add_library(libcomp netgen.cpp )
target_include_directories(libcomp PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(libcomp PRIVATE ${CUDA_LIBRARIES} libdb)
target_compile_options(libcomp PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -O3 -std=c++17>
  $<$<COMPILE_LANGUAGE:CUDA>:-O3 -std=c++17 -lcudadevrt --extended-lambda --expt-relaxed-constexpr>
)
set_property(TARGET libcomp PROPERTY CUDA_SEPARABLE_COMPILATION ON)
