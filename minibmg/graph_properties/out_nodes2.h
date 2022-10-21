/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <list>
#include <unordered_set>
#include "beanmachine/minibmg/graph2.h"
#include "beanmachine/minibmg/node2.h"

namespace beanmachine::minibmg {

// return the set of nodes that have the given node as an input in the given
// graph.
const std::list<Node2p>& out_nodes(const Graph2& graph, Node2p node);

} // namespace beanmachine::minibmg