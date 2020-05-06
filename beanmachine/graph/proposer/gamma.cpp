// Copyright (c) Facebook, Inc. and its affiliates.
#include <cmath>
#include <random>

#include "beanmachine/graph/proposer/gamma.h"

namespace beanmachine {
namespace proposer {

graph::AtomicValue Gamma::sample(std::mt19937& gen) const {
  // C++ gamma has the second parameter as the scale rather than a rate
  std::gamma_distribution<double> dist(alpha, 1.0 / beta);
  return graph::AtomicValue(graph::AtomicType::POS_REAL, dist(gen));
}

double Gamma::log_prob(graph::AtomicValue& value) const {
  double x = value._double;
  return (alpha - 1) * std::log(x) - beta * x + alpha * std::log(beta) -
      std::lgamma(alpha);
}

} // namespace proposer
} // namespace beanmachine
