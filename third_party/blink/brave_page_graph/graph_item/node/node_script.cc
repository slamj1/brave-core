/* Copyright (c) 2019 The Brave Software Team. Distributed under the MPL2
 * license. This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include "brave/third_party/blink/brave_page_graph/graph_item/node/node_script.h"
#include <sstream>
#include <string>
#include "brave/third_party/blink/brave_page_graph/graph_item/edge/edge_execute.h"
#include "brave/third_party/blink/brave_page_graph/graph_item/node/node.h"
#include "brave/third_party/blink/brave_page_graph/graph_item/node/node_actor.h"
#include "brave/third_party/blink/brave_page_graph/graphml.h"
#include "brave/third_party/blink/brave_page_graph/page_graph.h"
#include "brave/third_party/blink/brave_page_graph/types.h"

using ::std::string;
using ::std::stringstream;
using ::std::to_string;

namespace brave_page_graph {

NodeScript::NodeScript(PageGraph* const graph, const ScriptId script_id,
    const ScriptType type) :
      NodeActor(graph),
      script_id_(script_id),
      type_(type),
      url_(""),
      is_inline_(true) {}

NodeScript::NodeScript(PageGraph* const graph, const ScriptId script_id,
    const ScriptType type, const std::string& url) :
      NodeActor(graph),
      script_id_(script_id),
      type_(type),
      url_(url),
      is_inline_(url.empty()) {}

NodeScript::~NodeScript() {}

ItemName NodeScript::GetItemName() const {
  return "script #" + to_string(id_);
}

ScriptId NodeScript::GetScriptId() const {
  return script_id_;
}

ScriptType NodeScript::GetScriptType() const {
  return type_;
}

bool NodeScript::IsScript() const {
  return true;
}

bool NodeScript::IsInline() const {
  return is_inline_;
}

string NodeScript::GetUrl() const {
  return url_;
}

void NodeScript::SetUrl(const string& url) {
  url_ = url;
}

void NodeScript::AddInEdge(const EdgeExecute* const edge) {
  NodeActor::AddInEdge(edge);
}

GraphMLXMLList NodeScript::GraphMLAttributes() const {
  GraphMLXMLList attrs;
  attrs.push_back(GraphMLAttrDefForType(kGraphMLAttrDefNodeType)
      ->ToValue("script"));
  attrs.push_back(GraphMLAttrDefForType(kGraphMLAttrDefScriptId)
      ->ToValue(script_id_));
  attrs.push_back(GraphMLAttrDefForType(kGraphMLAttrDefScriptType)
      ->ToValue(ScriptTypeToString(type_)));

  if (IsInline() == false) {
    attrs.push_back(GraphMLAttrDefForType(kGraphMLAttrDefUrl)
      ->ToValue(GetUrl()));
  }

  return attrs;
}

ItemDesc NodeScript::GetDescBody() const {
  stringstream builder;
  builder << GetItemName();

  if (!url_.empty()) {
    builder << " (" << url_ << ")";
  }

  return builder.str();
}

}  // namespace brave_page_graph
