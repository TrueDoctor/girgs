#pragma once

// Serial stand-in for the OpenMP runtime, used for targets without OpenMP support (e.g. Emscripten).
// Without -fopenmp all `#pragma omp` directives are ignored, so every parallel region runs on a single thread.

inline int omp_get_max_threads() { return 1; }
inline int omp_get_num_threads() { return 1; }
inline int omp_get_thread_num() { return 0; }
