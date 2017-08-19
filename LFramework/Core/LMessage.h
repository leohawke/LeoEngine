/*!	\file LMessage.h
\ingroup LFrameWork/Core
\brief 消息处理。
*/


#ifndef LFrameWork_Core_LMessage_h
#define LFrameWork_Core_LMessage_h 1

#include <LFramework/Core/LObject.h>
#include <ctime>

namespace leo
{

namespace Messaging
{

/*!
\brief 消息标识。
\since build 175
*/
using ID = limpl(std::uint32_t);

/*!
\brief 消息优先级。
*/
using Priority = std::uint8_t;

/*!
\brief 默认消息优先级。
*/
lconstexpr const Priority NormalPriority(0x80);


/*!
\brief 消息。
\warning 非虚析构。
*/
class LF_API Message
{
	friend class MessageQueue;

private:
	ID id; //!< 消息标识。
	ValueObject content{}; //消息内容。

public:
	/*!
	\brief 构造：使用消息标识和空消息内容。
	*/
	Message(ID msg_id = 0)
		: id(msg_id)
	{}
	/*!
	\brief 构造：使用消息标识和消息内容。
	*/
	//@{
	Message(ID m, const ValueObject& vo)
		: id(m), content(vo)
	{}
	Message(ID m, ValueObject&& vo) lnothrow
		: id(m), content(std::move(vo))
	{}
	//@}

	/*!
	\brief 复制构造：默认实现。
	*/
	DefDeCopyCtor(Message)
	/*!
	\brief 转移构造：默认实现。
	*/
	DefDeMoveCtor(Message)

	PDefHOp(Message&, = , const ValueObject& c) lnothrow
		ImplRet(content = c, *this)
	/*!
	\brief 成员赋值：使用值类型对象。
	*/
	PDefHOp(Message&, =, ValueObject&& c) lnothrow
		ImplRet(content = std::move(c), *this)
	//@{
	//! \brief 复制赋值：使用复制和交换。
	PDefHOp(Message&, =, const Message& msg)
		ImplRet(leo::copy_and_swap(*this, msg))
	//! \brief 转移赋值：使用交换。
	DefDeMoveAssignment(Message)
	//@}

	/*!
	\brief 判断无效或有效性。
	*/
	DefBoolNeg(explicit, id)

	/*!
	\brief 比较：相等关系。
	*/
	LF_API friend bool
	operator==(const Message&, const Message&);

	DefGetter(const lnothrow, ID, MessageID, id) //!< 取消息标识。
	DefGetter(const lnothrow, const ValueObject&, Content, content) \
		//!< 取消息内容。

	/*!
	\brief 交换。
	*/
	LF_API friend void
	swap(Message&, Message&) lnothrow;
};


/*!
\brief 消息队列。
\note 使用 multiset 模拟。
\warning 非虚析构。
*/
class LF_API MessageQueue : private noncopyable,
	private multimap<Priority, Message, std::greater<Priority>>
{
public:
	using BaseType = multimap<Priority, Message, std::greater<Priority>>;
	/*!
	\brief 迭代器。
	*/
	//@{
	using BaseType::iterator;
	using BaseType::const_iterator;
	//@}

	/*!
	\brief 无参数构造：默认实现。
	*/
	DefDeCtor(MessageQueue)
	DefDeDtor(MessageQueue)

	/*!
	\brief 取消息队列中消息的最大优先级。
	\return 若消息队列为空则 0 ，否则为最大优先级。
	*/
	DefGetter(const lnothrow, Priority, MaxPriority,
		empty() ? 0 : cbegin()->first)

	/*!
	\brief 合并消息队列：移动指定消息队列中的所有消息至此消息队列中。
	*/
	void
	Merge(MessageQueue&);

	/*!
	\brief 从消息队列中取优先级最高的消息存至 msg 中。
	\note 在队列中保留消息；不检查消息是否有效。
	*/
	void
	Peek(Message&) const;

	/*!
	\brief 丢弃消息队列中优先级最高的消息。
	\note 消息队列为空时忽略。
	*/
	void
	Pop();

	/*!
	\brief 若消息有效，以指定优先级插入至消息队列中。
	*/
	void
	Push(const Message&, Priority);
	/*!
	\brief 若消息有效，以指定优先级插入至消息队列中。
	*/
	void
	Push(Message&&, Priority);

	/*!
	\brief 移除不大于指定优先级的消息。
	*/
	void
	Remove(Priority);

	//@{
	using BaseType::begin;

	using BaseType::cbegin;

	using BaseType::cend;

	using BaseType::clear;

	using BaseType::crbegin;

	using BaseType::crend;

	using BaseType::empty;

	using BaseType::end;

	using BaseType::erase;

	using BaseType::insert;

	using BaseType::max_size;

	using BaseType::rbegin;

	using BaseType::rend;

	using BaseType::size;
	//@}
};


/*!
\ingroup exceptions
\brief 消息异常。
*/
class LF_API MessageException : public LoggedEvent
{
public:
	using LoggedEvent::LoggedEvent;

	DefDeCopyCtor(MessageException)
	/*!
	\brief 虚析构：类定义外默认实现。
	*/
	~MessageException() override;
};


/*!
\ingroup exceptions
\brief 消息信号：表示单一处理中断的异常。
*/
class LF_API MessageSignal : public MessageException
{
public:
	//@{
	MessageSignal(const std::string& msg = {})
		: MessageException(msg)
	{}

	DefDeCopyCtor(MessageSignal)
	//! \brief 虚析构：类定义外默认实现。
	~MessageSignal() override;
	//@}
};

} // namespace Messaging;

using Messaging::Message;

} // namespace leo;

#endif

