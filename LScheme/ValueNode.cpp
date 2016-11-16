#include "ValueNode.h"
#include <cstdio>

namespace leo {

	const ValueNode&
		ValueNode::operator%=(const ValueNode& node)
	{
		auto& n((*this)[node.name]);

		n.Value = node.Value;
		return n;
	}
	const ValueNode&
		ValueNode::operator%=(const ValueNode&& node)
	{
		auto& n((*this)[node.name]);

		n.Value = std::move(node.Value);
		return n;
	}

	void
		ValueNode::SetContent(Container con, ValueObject vo) lnothrow
	{
		container.swap(con),
			swap(Value, vo);
	}

	void
		ValueNode::SwapContent(ValueNode& node) lnothrow
	{
		SwapContainer(node),
			swap(Value, node.Value);
	}

	void
		ValueNode::ThrowIndexOutOfRange()
	{
		throw std::out_of_range("Index is out of range.");
	}

	void
		ValueNode::ThrowWrongNameFound()
	{
		throw std::out_of_range("Wrong name found.");
	}

	void
		swap(ValueNode& x, ValueNode& y) lnothrow
	{
		std::swap(x.name, y.name),
			x.SwapContent(y);
	}

	ValueNode&
		AccessNode(ValueNode::Container* p_con, const string& name)
	{
		if (const auto p = AccessNodePtr(p_con, name))
			return *p;
		ValueNode::ThrowWrongNameFound();
	}
	const ValueNode&
		AccessNode(const ValueNode::Container* p_con, const string& name)
	{
		if (const auto p = AccessNodePtr(p_con, name))
			return *p;
		ValueNode::ThrowWrongNameFound();
	}
	ValueNode&
		AccessNode(ValueNode& node, size_t n)
	{
		const auto p(AccessNodePtr(node, n));

		if (p)
			return *p;
		ValueNode::ThrowIndexOutOfRange();
	}
	const ValueNode&
		AccessNode(const ValueNode& node, size_t n)
	{
		const auto p(AccessNodePtr(node, n));

		if (p)
			return *p;
		ValueNode::ThrowIndexOutOfRange();
	}

	observer_ptr<ValueNode>
		AccessNodePtr(ValueNode::Container& con, const string& name) lnothrow
	{
		return make_observer(call_value_or<ValueNode*>(addrof<>(),
			con.find(name), {}, end(con)));
	}
	observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode::Container& con, const string& name) lnothrow
	{
		return make_observer(call_value_or<const ValueNode*>(
			addrof<>(), con.find(name), {}, end(con)));
	}
	observer_ptr<ValueNode>
		AccessNodePtr(ValueNode& node, size_t n)
	{
		auto& con(node.GetContainerRef());

		// XXX: Conversion to 'ptrdiff_t' might be implementation-defined.
		return n < con.size() ? make_observer(&*std::next(con.begin(), ptrdiff_t(n)))
			: nullptr;
	}
	observer_ptr<const ValueNode>
		AccessNodePtr(const ValueNode& node, size_t n)
	{
		auto& con(node.GetContainer());

		// XXX: Conversion to 'ptrdiff_t' might be implementation-defined.
		return n < con.size() ? make_observer(&*std::next(con.cbegin(), ptrdiff_t(n)))
			: nullptr;
	}


	void
		RemoveEmptyChildren(ValueNode::Container& con) lnothrow
	{
		erase_all_if(con, std::mem_fn(&ValueNode::operator!));
	}

	void
		RemoveHead(ValueNode::Container& con) lnothrowv
	{
		LAssert(!con.empty(), "Empty node container found.");
		con.erase(con.cbegin());
	}


	bool
		IsPrefixedIndex(string_view name, char prefix)
	{
		LAssertNonnull(name.data());
		if (name.length() > 1 && name.front() == prefix)
			try
		{
			const auto ss(name.substr(1));

			return MakeIndex(std::stoul(string(ss))) == ss;
		}
		CatchIgnore(std::invalid_argument&)
			return{};
	}

	string
		MakeIndex(size_t n)
	{
		char str[5]{};

		if (n < 10000)
			std::snprintf(str, 5, "%04u", unsigned(n));
		else
			throw std::invalid_argument("Argument is too large.");
		return str;
	}
}