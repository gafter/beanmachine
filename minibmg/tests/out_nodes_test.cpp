/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include <gtest/gtest.h>
#include <list>
#include <unordered_set>
#include "beanmachine/minibmg/minibmg.h"
#include "beanmachine/minibmg/out_nodes.h"

using namespace ::testing;
using namespace ::beanmachine::minibmg;

std::unordered_set<NodeId> set(std::list<NodeId> values) {
  std::unordered_set<NodeId> result{};
  for (auto x : values) {
    result.insert(x);
  }
  return result;
}

TEST(out_nodes_test, simple) {
  Graph::Factory gf;
  auto k12 = gf.add_constant(1.2);
  auto k34 = gf.add_constant(3.4);
  auto plus = gf.add_operator(Operator::ADD, {k12, k34});
  auto k56 = gf.add_constant(5.6);
  auto beta = gf.add_operator(Operator::DISTRIBUTION_BETA, {k34, k56});
  auto sample = gf.add_operator(Operator::SAMPLE, {beta});
  auto k78 = gf.add_constant(7.8);
  auto observe = gf.add_operator(Operator::OBSERVE, {beta, k78});
  NodeId query;
  /* auto query_ = */ gf.add_query(beta, query);
  Graph g = gf.build();

  ASSERT_EQ(out_nodes(g, k12), set({plus}));
  ASSERT_EQ(out_nodes(g, k34), set({plus, beta}));
  ASSERT_EQ(out_nodes(g, plus), set({}));
  ASSERT_EQ(out_nodes(g, k56), set({beta}));
  ASSERT_EQ(out_nodes(g, beta), set({sample, observe, query}));
  ASSERT_EQ(out_nodes(g, sample), set({}));
  ASSERT_EQ(out_nodes(g, k78), set({observe}));
  ASSERT_EQ(out_nodes(g, observe), set({}));
  ASSERT_EQ(out_nodes(g, query), set({}));
}

TEST(out_nodes_test, not_found1) {
  Graph::Factory gf;
  Graph g = gf.build();
  NodeId not_found_node{};
  ASSERT_THROW(out_nodes(g, not_found_node), std::invalid_argument);
}

TEST(out_nodes_test, not_found2) {
  Graph::Factory gf;
  Graph g = gf.build();
  Node* n = new ConstantNode(0);
  ASSERT_THROW(out_nodes(g, n), std::invalid_argument);
  delete n;
}
