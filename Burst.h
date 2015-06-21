#pragma once
#include <stdlib.h>

struct Burst
{
  enum Type {
    CPU,
    IO
  };
  
  // The type of the burst
  Type type;

  // The length of the burst
  size_t length;
  bool preempted;

  // constructor
  Burst(Type type, size_t length, bool preempted) : type(type), length(length), preempted(preempted) {}
};
