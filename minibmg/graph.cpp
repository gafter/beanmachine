/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "beanmachine/minibmg/graph.h"
#include <fmt/core.h>
#include <folly/json.h>
#include <algorithm>
#include <unordered_map>
#include "beanmachine/minibmg/factory.h"
#include "beanmachine/minibmg/topological.h"
#include "graph.h"
#include "operator.h"

namespace {

using namespace beanmachine::minibmg;

std::vector<Nodep> successors(const Nodep& n) {
  switch (n->op) {
    case Operator::CONSTANT:
    case Operator::VARIABLE:
      return {};
    default:
      return std::dynamic_pointer_cast<const OperatorNode>(n)->in_nodes;
  }
}

} // namespace

namespace beanmachine::minibmg {

using dynamic = folly::dynamic;

Graph::Graph(
    std::vector<Nodep> nodes,
    std::vector<Nodep> queries,
    std::unordered_map<Nodep, double> observations)
    : nodes{nodes}, queries{queries}, observations{observations} {}

Graph::~Graph() {}

Graph Graph::create(
    std::vector<Nodep> queries,
    std::unordered_map<Nodep, double> observations) {
  std::list<Nodep> roots;
  for (auto n : queries)
    roots.push_back(n);
  for (auto p : observations) {
    if (p.first->op != Operator::SAMPLE) {
      throw std::invalid_argument(fmt::format("can only observe a sample"));
    }
    roots.push_back(p.first);
  }
  std::vector<Nodep> all_nodes;
  if (!topological_sort<Nodep>(roots, &successors, all_nodes))
    throw std::invalid_argument("graph has a cycle");
  std::reverse(all_nodes.begin(), all_nodes.end());

  Graph::validate(all_nodes);
  return Graph{all_nodes, queries, observations};
}

void Graph::validate(std::vector<Nodep> nodes) {
  std::unordered_set<Nodep> seen;
  // Check the nodes.
  for (int i = 0, n = nodes.size(); i < n; i++) {
    auto node = nodes[i];

    // TODO: improve the exception diagnostics on failure.  e.g. how to identify
    // a node?

    // Check that the operator is in range.
    if (node->op < (Operator)0 || node->op >= Operator::LAST_OPERATOR) {
      throw std::invalid_argument(
          fmt::format("Node {0} has invalid operator {1}", i, (int)node->op));
    }

    // Check the node type
    if (node->type != expected_result_type(node->op)) {
      throw std::invalid_argument(fmt::format(
          "Node {0} has type {1} but should be {2}",
          i,
          to_string(node->type),
          to_string(expected_result_type(node->op))));
    }

    // Check the predecessor nodes
    switch (node->op) {
      case Operator::CONSTANT:
      case Operator::VARIABLE:
        break;

      // Check other operators.
      default: {
        auto op = std::dynamic_pointer_cast<const OperatorNode>(node);
        unsigned ix = (unsigned)node->op;
        auto parent_types = expected_parents[ix];
        if (op->in_nodes.size() != parent_types.size()) {
          throw std::invalid_argument(fmt::format(
              "Node {0} should have {1} parents", i, parent_types.size()));
        }
        for (int j = 0, m = parent_types.size(); j < m; j++) {
          Nodep parent = op->in_nodes[j];
          if (!seen.count(parent)) {
            throw std::invalid_argument(
                fmt::format("Node {0} has a parent not previously seen", i));
          }
          if (parent->type != parent_types[j]) {
            throw std::invalid_argument(fmt::format(
                "Node {0} should have a {1} input",
                i,
                to_string(parent_types[j])));
          }
        }
      }
    }

    seen.insert(node);
  }
}

folly::dynamic graph_to_json(const Graph& g) {
  std::unordered_map<Nodep, unsigned long> id_map{};
  dynamic result = dynamic::object;
  result["comment"] = "created by graph_to_json";
  dynamic a = dynamic::array;

  unsigned long next_sequence = 0;
  for (auto node : g) {
    auto sequence = next_sequence++;
    id_map[node] = sequence;
    dynamic dyn_node = dynamic::object;
    dyn_node["sequence"] = sequence;
    dyn_node["operator"] = to_string(node->op);
    dyn_node["type"] = to_string(node->type);
    switch (node->op) {
      case Operator::CONSTANT: {
        auto n = std::dynamic_pointer_cast<const ConstantNode>(node);
        dyn_node["value"] = n->value;
        break;
      }
      // TODO: case Operator::VARIABLE:
      default: {
        auto n = std::dynamic_pointer_cast<const OperatorNode>(node);
        dynamic in_nodes = dynamic::array;
        for (auto pred : n->in_nodes) {
          in_nodes.push_back(id_map[pred]);
        }
        dyn_node["in_nodes"] = in_nodes;
        break;
      }
    }
    a.push_back(dyn_node);
  }

  dynamic queries = dynamic::array;
  result["queries"] = queries;
  dynamic observations = dynamic::array;
  result["observations"] = observations;

  for (auto q : g.queries) {
    queries.push_back(id_map[q]);
  }
  for (auto q : g.observations) {
    dynamic d = dynamic::object;
    d["node"] = id_map[q.first];
    d["value"] = q.second;
  }

  result["nodes"] = a;
  return result;
}

JsonError::JsonError(const std::string& message) : message(message) {}

Graph json_to_graph(folly::dynamic d) {
  std::unordered_map<int, Nodep> sequence_to_node;

  auto json_nodes = d["nodes"];
  if (!json_nodes.isArray()) {
    throw JsonError("missing \"nodes\" property");
  }
  for (auto json_node : json_nodes) {
    auto sequencev = json_node["sequence"];
    if (!sequencev.isInt()) {
      throw JsonError("missing sequence number.");
    }
    auto sequence = sequencev.asInt();

    auto opv = json_node["operator"];
    if (!opv.isString()) {
      throw JsonError("missing operator.");
    }
    auto op = operator_from_name(opv.asString());
    if (op == Operator::NO_OPERATOR) {
      throw JsonError("bad operator " + opv.asString());
    }

    Type type;
    auto typev = json_node["type"];
    if (!typev.isString()) {
      type = op_type(op);
    } else {
      type = type_from_name(typev.asString());
    }

    Nodep node;
    switch (op) {
      case Operator::CONSTANT: {
        auto valuev = json_node["value"];
        double value;
        if (valuev.isInt()) {
          value = valuev.asInt();
        } else if (valuev.isDouble()) {
          value = valuev.asDouble();
        } else {
          throw JsonError("bad value for constant.");
        }
        if (type != Type::REAL) {
          throw JsonError("bad type for constant.");
        }
        node = std::make_shared<const ConstantNode>(value);
        break;
      }
      case Operator::VARIABLE: {
        auto namev = json_node["name"];
        std::string name = "";
        if (namev.isString()) {
          name = namev.asString();
        } else {
          throw JsonError("bad name for variable.");
        }
        if (type != Type::REAL) {
          throw JsonError("bad type for variable.");
        }
        auto variable_indexv = json_node["variable_index"];
        if (!variable_indexv.isInt()) {
          throw JsonError("bad variable_index for variable.");
        }
        auto variable_index = (unsigned)variable_indexv.asInt();
        node = std::make_shared<const VariableNode>(name, variable_index);
        break;
      }
      default: {
        auto in_nodesv = json_node["in_nodes"];
        if (!in_nodesv.isArray()) {
          throw JsonError("missing in_nodes.");
        }
        std::vector<Nodep> in_nodes;
        for (auto in_nodev : in_nodesv) {
          if (!in_nodev.isInt()) {
            throw JsonError("missing in_node for operator.");
          }
          auto in_node_i = in_nodev.asInt();
          if (sequence_to_node.find(in_node_i) == sequence_to_node.end()) {
            throw JsonError("bad in_node for operator.");
          }
          auto in_node = sequence_to_node.find(in_node_i)->second;
          in_nodes.push_back(in_node);
        }
        node = std::make_shared<const OperatorNode>(in_nodes, op, type);
        break;
      }
    }

    if (sequence_to_node.find(sequence) != sequence_to_node.end()) {
      throw JsonError(fmt::format("duplicate node ID {}.", sequence));
    }
    sequence_to_node[sequence] = node;
  }

  std::vector<Nodep> queries;
  auto query_nodes = d["queries"];
  if (query_nodes.isArray()) {
    for (auto query : query_nodes) {
      if (!query.isInt()) {
        throw JsonError("bad query value.");
      }
      auto query_i = query.asInt();
      if (sequence_to_node.find(query_i) == sequence_to_node.end()) {
        throw JsonError(fmt::format("bad in_node {} for query.", query_i));
      }
      auto query_node = sequence_to_node.find(query_i)->second;
      queries.push_back(query_node);
    }
  }

  std::unordered_map<Nodep, double> observations;
  auto observation_nodes = d["observations"];
  if (observation_nodes.isArray()) {
    for (auto obs : observation_nodes) {
      auto node = obs["node"];
      if (!node.isInt()) {
        throw JsonError("bad observation node.");
      }
      auto node_i = node.asInt();
      if (sequence_to_node.find(node_i) == sequence_to_node.end()) {
        throw JsonError(fmt::format("bad in_node {} for observation.", node_i));
      }
      auto obs_node = sequence_to_node.find(node_i)->second;
      auto value = obs["value"];
      if (!node.isDouble()) {
        throw JsonError("bad value for observation.");
      }
      auto value_d = value.asDouble();
      observations[obs_node] = value_d;
    }
  }

  return Graph::create(queries, observations);
}

} // namespace beanmachine::minibmg
