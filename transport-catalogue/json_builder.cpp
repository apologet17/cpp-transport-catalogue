#include "json.h"
#include "json_builder.h"

namespace json {

	void InternalBuilder::StartDict() {

		Node* current_node;
		if (!nodes_stack_.empty()) {
			current_node = nodes_stack_.back();
		}
		else {
			throw std::logic_error("Bad value");
		}

		if (current_node->IsNull()) {
			current_node->SetValue() = Dict();
		}
		else if (current_node->IsArray()) {
			nodes_stack_.push_back(&current_node->AsArray().emplace_back(Dict()));
		}
		else if (current_node->IsDict() && key_ok_) {
			current_node->SetValue() = Dict();

		}
		else {
			throw std::logic_error("Bad start dict");
		}

		key_ok_ = false;
	}

	void InternalBuilder::StartArray() {
		Node* current_node;
		if (!nodes_stack_.empty()) {
			current_node = nodes_stack_.back();
		}
		else {
			throw std::logic_error("Bad value");
		}

		if (current_node->IsNull()) {
			current_node->SetValue() = Array();
		}
		else if (current_node->IsArray()) {
			nodes_stack_.push_back(&current_node->AsArray().emplace_back(Array()));
		}
		else if (current_node->IsDict() && key_ok_) {
			current_node->SetValue() = Array();
			key_ok_ = false;
		}
		else
			throw std::logic_error("Bad start array");
	}

	void InternalBuilder::EndDict() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsDict()) {
			nodes_stack_.pop_back();
			key_ok_ = false;
		}
		else
			throw std::logic_error("Bad value");
	}

	void InternalBuilder::EndArray() {
		if (!nodes_stack_.empty() && nodes_stack_.back()->IsArray()) {
			nodes_stack_.pop_back();
			key_ok_ = false;
		}
		else
			throw std::logic_error("Bad value");
	}

	void InternalBuilder::Key(std::string key) {
		Node* current_node;
		if (!nodes_stack_.empty()) {
			current_node = nodes_stack_.back();
		}
		else {
			throw std::logic_error("Bad value");
		}

		if (current_node->IsDict() && !key_ok_) {
			current_node->AsDict()[key] = Node();
			nodes_stack_.push_back(&(current_node->AsDict().at(key)));
			key_ = key;
			key_ok_ = true;
		}
		else
			throw std::logic_error("Bad value");
	}

	void InternalBuilder::Value(Node::Value value) {
		Node* current_node;
		if (!nodes_stack_.empty()) {
			current_node = nodes_stack_.back();
		}
		else {
			throw std::logic_error("Bad value");
		}

		if (current_node->IsNull()) {
			current_node->SetValue() = value;
			if (!nodes_stack_.empty()) nodes_stack_.pop_back();
		}

		else if (current_node->IsArray()) {
			current_node->AsArray().emplace_back(Node());
			current_node->AsArray().back().SetValue() = value;
		}
		else if (current_node->IsDict() && key_ok_) {
			current_node->AsDict().at(key_).SetValue() = value;

		}
		else
			throw std::logic_error("Bad value");

		key_ok_ = false;
	}

	Node InternalBuilder::Build() {
		if (!nodes_stack_.empty()) {
			throw std::logic_error("Builder object is not created");
		}
		return root_;
	}

	KeyItemContext BaseContext::Key(std::string key) {
		intern_builder_->Key(key);
		return { *this };
	}

	 DictItemContext BaseContext::StartDict() {
		 intern_builder_->StartDict();
		 return { *this };
	}

	 BaseContext BaseContext::EndDict() {
		 intern_builder_->EndDict();
		 return  *this ;
	}

	ArrayItemContext BaseContext::StartArray() {
		intern_builder_->StartArray();
		return { *this };
	}

	BaseContext BaseContext::EndArray() {
		intern_builder_->EndArray();
		return  *this ;
	}

	ValueItemContext BaseContext::Value(Node::Value value) {
		intern_builder_->Value(value);
		return {*this};
	}
    
	Value2ItemContext ValueItemContext::Value(Node::Value value) {
		intern_builder_->Value(value);
		return {*this};
	}
    
	DictItemContext KeyItemContext::Value(Node::Value value) {
		intern_builder_->Value(value);
		return { *this };
	}

	Node BaseContext::Build() {
		return intern_builder_->Build();
	}
}