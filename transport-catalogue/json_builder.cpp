#include <stdexcept>
#include <utility>
#include <variant>

#include "json_builder.h"

namespace json {

    using namespace std;
    using namespace std::literals;

    BuildConstructor::BuildConstructor(Builder& builder)
        : builder_(builder) {}

    BuildContextFirst::BuildContextFirst(Builder& builder)
        : BuildConstructor(builder) {}

    DictContext& BuildContextFirst::StartDict() {
        return builder_.StartDict();
    }

    ArrayContext& BuildContextFirst::StartArray() {
        return builder_.StartArray();
    }

    BuildContextSecond::BuildContextSecond(Builder& builder)
        : BuildConstructor(builder) {}

    KeyContext& BuildContextSecond::Key(std::string key) {
        return builder_.Key(key);
    }

    Builder& BuildContextSecond::EndDict() {
        return builder_.EndDict();
    }

    KeyContext::KeyContext(Builder& builder)
        : BuildContextFirst(builder) {}

    ValueKeyContext& KeyContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    ValueKeyContext::ValueKeyContext(Builder& builder)
        : BuildContextSecond(builder) {}

    ValueArrayContext::ValueArrayContext(Builder& builder)
        : BuildContextFirst(builder) {}

    ValueArrayContext& ValueArrayContext::Value(Node::Value value) {
        return builder_.Value(value);
    }

    Builder& ValueArrayContext::EndArray() {
        return builder_.EndArray();
    }

    DictContext::DictContext(Builder& builder)
        : BuildContextSecond(builder) {}

    ArrayContext::ArrayContext(Builder& builder)
        : ValueArrayContext(builder) {}

    Builder::Builder()
        : KeyContext(*this)
        , ValueKeyContext(*this)
        , DictContext(*this)
        , ArrayContext(*this) {}

    KeyContext& Builder::Key(string key) {
        if (UnableUseKey()) {
            throw logic_error("Key сan't be applied"s);
        }
        nodes_.push(make_unique<Node>(key));
        return *this;
    }

    Builder& Builder::Value(Node::Value value) {
        if (UnableUseValue()) {
            throw std::logic_error("Value сan't be applied"s);
        }
        PushNode(value);
        return AddNode(*nodes_.top().release());
    }

    DictContext& Builder::StartDict() {
        if (UnableUseStartDict()) {
            throw logic_error("StartDict сan't be applied"s);
        }
        nodes_.push(make_unique<Node>(Dict()));
        return *this;
    }

    Builder& Builder::EndDict() {
        if (UnableUseEndDict()) {
            throw logic_error("EndDict сan't be applied"s);
        }
        return AddNode(*nodes_.top().release());
    }

    ArrayContext& Builder::StartArray() {
        if (UnableUseStartArray()) {
            throw logic_error("StartArray сan't be applied"s);
        }
        nodes_.push(make_unique<Node>(Array()));
        return *this;
    }

    Builder& Builder::EndArray() {
        if (UnableUseEndArray()) {
            throw logic_error("EndArray сan't be applied"s);
        }
        return AddNode(*nodes_.top().release());
    }

    Node Builder::Build() const {
        if (UnableUseBuild()) {
            throw logic_error("Builder сan't be applied"s);
        }
        return root_;
    }

    bool Builder::UnableAdd() const {
        return !(nodes_.empty()
            || nodes_.top()->IsArray()
            || nodes_.top()->IsString());
    }

    bool Builder::IsMakeObj() const {
        return !root_.IsNull();
    }

    bool Builder::UnableUseKey() const {
        return IsMakeObj()
            || nodes_.empty()
            || !nodes_.top()->IsDict();
    }

    bool Builder::UnableUseValue() const {
        return IsMakeObj()
            || UnableAdd();
    }

    bool Builder::UnableUseStartDict() const {
        return UnableUseValue();
    }

    bool Builder::UnableUseEndDict() const {
        return IsMakeObj()
            || nodes_.empty()
            || !nodes_.top()->IsDict();
    }

    bool Builder::UnableUseStartArray() const {
        return UnableUseValue();
    }

    bool Builder::UnableUseEndArray() const {
        return IsMakeObj()
            || nodes_.empty()
            || !nodes_.top()->IsArray();
    }

    bool Builder::UnableUseBuild() const {
        return !IsMakeObj();
    }

    Builder& Builder::AddNode(const Node& node) {
        nodes_.pop();
        if (nodes_.empty()) {
            root_ = node;
        }
        else if (nodes_.top()->IsArray()) {
            nodes_.top()->AsChangeableArray().push_back(node);
        }
        else {
            Node& key = *nodes_.top().release();
            nodes_.pop();
            nodes_.top()->AsChangeableDict().emplace(key.AsString(), node);
        }
        return *this;
    }

    void Builder::PushNode(Node::Value value) {
        visit([this](auto&& val) {
            nodes_.push(make_unique<Node>(val));
            }, value);
    }

}