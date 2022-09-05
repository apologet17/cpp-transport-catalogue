#include <vector>
#include <memory>
#include "json.h"

namespace json {

	class DictItemContext;
	class ArrayItemContext;
	class KeyItemContext;
	class Builder;
	class BaseContext;
    class Value2ItemContext;
    class ValueItemContext;

class InternalBuilder {
public:
	InternalBuilder() {
		root_ = {};
		nodes_stack_.push_back(&root_);
		key_ = "";
		key_ok_ = false;
	}

	void StartDict();

	void StartArray();

	void EndDict();

	void EndArray();

	void Key(std::string);

	void Value(Node::Value value);

	Node Build();
private:
	Node root_;
	std::vector<Node*> nodes_stack_;
	std::string key_;
	bool key_ok_;
};
	
class BaseContext {
public:

	BaseContext(BaseContext& prev_context)
		: intern_builder_(prev_context.intern_builder_)	{
	}

	BaseContext()
		: intern_builder_(std::make_shared<InternalBuilder>() )	{
	}

	DictItemContext StartDict();
	ArrayItemContext StartArray();
	KeyItemContext Key(std::string key);
	BaseContext EndDict();
	BaseContext EndArray();
	ValueItemContext Value(Node::Value value);

	Node Build();

protected:
	//InternalBuilder* intern_builder_;
	std::shared_ptr<InternalBuilder> intern_builder_;
};

class Builder: public  BaseContext {
	using BaseContext::EndDict;
	using BaseContext::EndArray;

public:
	using BaseContext::BaseContext;

};

class DictItemContext:public BaseContext {
	using BaseContext::StartDict;
	using BaseContext::StartArray;
	using BaseContext::EndArray;
	using BaseContext::Value;
    using BaseContext::Build;
public:

	DictItemContext(BaseContext& prev_context)
	  : BaseContext(prev_context) {
	}
};

class ArrayItemContext :public BaseContext {

	using BaseContext::Key;
	using BaseContext::EndDict;
	using BaseContext::Build;
public:
	ArrayItemContext(BaseContext& prev_context)
		: BaseContext(prev_context) {
	}
};

class KeyItemContext :public BaseContext {

	using BaseContext::EndDict;
	using BaseContext::EndArray;
	using BaseContext::Build;
	using BaseContext::Key;
    
public:

	KeyItemContext(BaseContext& prev_context)
		: BaseContext(prev_context) {
	}
	DictItemContext Value(Node::Value value);

};
    
class ValueItemContext :public BaseContext {
   
public:
	ValueItemContext(BaseContext& prev_context)
		: BaseContext(prev_context) {
	}
 Value2ItemContext Value(Node::Value value);  
};
    
class Value2ItemContext :public BaseContext {
	using BaseContext::EndDict;
	using BaseContext::Build;
	using BaseContext::Key;

public:
	Value2ItemContext(BaseContext& prev_context)
		: BaseContext(prev_context) {
	}
 
};
}

