/*\par �޸�ʱ�� :
	2017-03-27 15:14 +0800
*/

#ifndef LScheme_LSchemeA_H
#define LScheme_LSchemeA_H 1

#include <LBase/sutility.h> // for leo::derived_entity
#include "SContext.h" 
#include "LEvent.hpp"
#include <LBase/any.h> // for leo::any

namespace scheme {

	/*!	\defgroup ThunkType Thunk Types
	\brief �м�ֵ���͡�

	����ض���ֵ���ԣ������� TermNode �� Value ���ݳ�Ա�в�ֱ�ӱ�ʾ�������Զ�������͡�
	*/


		using leo::pair;
		using leo::to_string;
		using leo::MakeIndex;
		using leo::NodeSequence;
		using leo::NodeLiteral;


		/*!
		\brief LA Ԫ��ǩ��
		\note LA �� LScheme �ĳ���ʵ�֡�
		*/
		struct LS_API LSLATag : LTag
		{};


		/*!
		\brief �����﷨�ڵ㡣

		��ָ���Ľڵ�����Խڵ��С�ַ���Ϊ���ƵĽڵ㣬�������﷨��������
		*/
		//@{
		template<class _tNodeOrCon, typename... _tParams>
		ValueNode::iterator
			InsertSyntaxNode(_tNodeOrCon&& node_or_con,
				std::initializer_list<ValueNode> il, _tParams&&... args)
		{
			return node_or_con.emplace_hint(node_or_con.end(), ValueNode::Container(il),
				MakeIndex(node_or_con), lforward(args)...);
		}
		template<class _tNodeOrCon, typename _type, typename... _tParams>
		ValueNode::iterator
			InsertSyntaxNode(_tNodeOrCon&& node_or_con, _type&& arg, _tParams&&... args)
		{
			return node_or_con.emplace_hint(node_or_con.end(), lforward(arg),
				MakeIndex(node_or_con), lforward(args)...);
		}
		//@}


		/*!
		\brief �ڵ�ӳ��������ͣ��任�ڵ�Ϊ��һ���ڵ㡣
		*/
		using NodeMapper = std::function<ValueNode(const TermNode&)>;

		//@{
		using NodeToString = std::function<string(const ValueNode&)>;

		template<class _tCon>
		using GNodeInserter = std::function<void(TermNode&&, _tCon&)>;

		using NodeInserter = GNodeInserter<TermNode::Container&>;

		using NodeSequenceInserter = GNodeInserter<NodeSequence>;
		//@}


		//! \return �������½ڵ㡣
		//@{
		/*!
		\brief ӳ�� LSLA Ҷ�ڵ㡣
		\sa ParseLSLANodeString

		�����½ڵ㡣������Ϊ���򷵻�ֵΪ�մ����½ڵ㣻����ֵ�� ParseLSLANodeString ȡ�á�
		*/
		LS_API ValueNode
			MapLSLALeafNode(const TermNode&);

		/*!
		\brief �任�ڵ�Ϊ�﷨������Ҷ�ڵ㡣
		\note ��ѡ����ָ��������ơ�
		*/
		LS_API ValueNode
			TransformToSyntaxNode(const ValueNode&, const string& = {});
		//@}

		/*!
		\brief ת�� LSLA �ڵ���������
		\return ���� EscapeLiteral ת������ַ����Ľ����
		\exception leo::bad_any_cast �쳣�������� Access �׳���
		*/
		LS_API string
			EscapeNodeLiteral(const ValueNode&);

		/*!
		\brief ת�� LSLA �ڵ���������
		\return ����Ϊ�սڵ���մ���������� Literalize �������� EscapeNodeLiteral �Ľ����
		\exception leo::bad_any_cast �쳣�������� EscapeNodeLiteral �׳���
		\sa EscapeNodeLiteral
		*/
		LS_API string
			LiteralizeEscapeNodeLiteral(const ValueNode&);

		/*!
		\brief ���� LSLA �ڵ��ַ�����

		�� string ���ͷ��ʽڵ㣬��ʧ������Ϊ�մ���
		*/
		LS_API string
			ParseLSLANodeString(const ValueNode&);


		/*!
		\brief �����﷨�ӽڵ㡣

		��ָ���Ľڵ�����Խڵ��С�ַ���Ϊ���ƵĽڵ㣬�������﷨��������
		*/
		//@{
		template<class _tNodeOrCon>
		ValueNode::iterator
			InsertChildSyntaxNode(_tNodeOrCon&& node_or_con, ValueNode& child)
		{
			return InsertSyntaxNode(lforward(node_or_con), child.GetContainerRef());
		}
		template<class _tNodeOrCon>
		ValueNode::iterator
			InsertChildSyntaxNode(_tNodeOrCon&& node_or_con, ValueNode&& child)
		{
			return InsertSyntaxNode(lforward(node_or_con),
				std::move(child.GetContainerRef()));
		}
		template<class _tNodeOrCon>
		ValueNode::iterator
			InsertChildSyntaxNode(_tNodeOrCon&& node_or_con, const NodeLiteral& nl)
		{
			return
				InsertChildSyntaxNode(lforward(node_or_con), TransformToSyntaxNode(nl));
		}
		//@}


		//! \brief ����ǰ׺�����ĺ������͡�
		using IndentGenerator = std::function<string(size_t)>;

		//! \brief ����ˮƽ�Ʊ��Ϊ��λ��������
		LS_API string
			DefaultGenerateIndent(size_t);

		/*!
		\brief ��ӡ������
		\note �����һ���������������޸����á�
		*/
		LS_API void
			PrintIndent(std::ostream&, IndentGenerator = DefaultGenerateIndent, size_t = 1);

		/*!
		\brief ��ӡ LSLA �ڵ㡣
		\sa PrintIdent
		\sa PrintNodeChild
		\sa PrintNodeString

		���õ��Ĳ���������һ������������������Ϊǰ׺��һ���ո�Ȼ���ӡ�ڵ����ݣ�
		�ȳ��Ե��� PrintNodeString ��ӡ�ڵ��ַ��������ɹ�ֱ�ӷ��أ�
		�����ӡ���У�Ȼ���Ե��� PrintNodeChild ��ӡ NodeSequence ��
		�ٴ�ʧ������� PrintNodeChild ��ӡ�ӽڵ㡣
		���� PrintNodeChild ��ӡ������س���
		*/
		LS_API void
			PrintNode(std::ostream&, const ValueNode&, NodeToString = EscapeNodeLiteral,
				IndentGenerator = DefaultGenerateIndent, size_t = 0);

		/*!
		\brief ��ӡ��Ϊ�ӽڵ�� LSLA �ڵ㡣
		\sa IsPrefixedIndex
		\sa PrintIdent
		\sa PrintNodeString

		���õ��Ĳ���������һ������������������Ϊǰ׺��Ȼ���ӡ�ӽڵ����ݡ�
		������ IsPrefixedIndex �Ľڵ���� PrintNodeString ��Ϊ�ڵ��ַ�����ӡ��
		���򣬵��� PrintNode �ݹ��ӡ�ӽڵ㣬���Դ˹����е� std::out_of_range �쳣��
		*/
		LS_API void
			PrintNodeChild(std::ostream&, const ValueNode&, NodeToString
				= EscapeNodeLiteral, IndentGenerator = DefaultGenerateIndent, size_t = 0);

		/*!
		\brief ��ӡ�ڵ��ַ�����
		\return �Ƿ�ɹ����ʽڵ��ַ����������
		\note leo::bad_any_cast ���쳣������
		\sa PrintNode

		ʹ�����һ������ָ���ķ��ʽڵ㣬��ӡ�õ����ַ����ͻ��з���
		���� leo::bad_any_cast ��
		*/
		LS_API bool
			PrintNodeString(std::ostream&, const ValueNode&,
				NodeToString = EscapeNodeLiteral);


		namespace sxml
		{

			//@{
			/*!
			\brief ת�� sxml �ڵ�Ϊ XML �����ַ�����
			\throw LoggedEvent û���ӽڵ㡣
			\note ��ǰ��֧�� annotation ���ڳ��� 2 ���ӽڵ�ʱʹ�� TraceDe ���档
			*/
			LS_API string
				ConvertAttributeNodeString(const TermNode&);

			/*!
			\brief ת�� sxml �ĵ��ڵ�Ϊ XML �ַ�����
			\throw LoggedEvent ���������һ������Լ�������ݱ�������
			\throw leo::unimplemented ָ�� ParseOption::Strict ʱ����δʵ�����ݡ�
			\sa ConvertStringNode
			\see http://okmij.org/ftp/Scheme/sxml.html#Annotations ��
			\todo ֧�� *ENTITY* �� *NAMESPACES* ��ǩ��

			ת�� sxml �ĵ��ڵ�Ϊ XML ��
			����ʹ�� ConvertStringNode ת���ַ����ڵ㣬��ʧ����Ϊ��Ҷ�ӽڵ�ݹ�ת����
			��Ϊ��ǰ sxml �淶δָ��ע��(annotation) ������ֱ�Ӻ��ԡ�
			*/
			LS_API string
				ConvertDocumentNode(const TermNode&, IndentGenerator = DefaultGenerateIndent,
					size_t = 0, ParseOption = ParseOption::Normal);

			/*!
			\brief ת�� sxml �ڵ�Ϊ��ת��� XML �ַ�����
			\sa EscapeXML
			*/
			LS_API string
				ConvertStringNode(const TermNode&);

			/*!
			\brief ��ӡ SContext::Analyze ����ȡ�õ� sxml �﷨���ڵ㲢ˢ������
			\see ConvertDocumentNode
			\see SContext::Analyze
			\see Session

			�����ڵ���ȡ��һ���ڵ���Ϊ sxml �ĵ��ڵ���� ConvertStringNode �����ˢ������
			*/
			LS_API void
				PrintSyntaxNode(std::ostream& os, const TermNode&,
					IndentGenerator = DefaultGenerateIndent, size_t = 0);
			//@}


			//@{
			//! \brief ���� sxml �ĵ������ڵ㡣
			//@{
			template<typename... _tParams>
			ValueNode
				MakeTop(const string& name, _tParams&&... args)
			{
				return leo::AsNodeLiteral(name,
				{ { MakeIndex(0), "*TOP*" }, NodeLiteral(lforward(args))... });
			}
			inline PDefH(ValueNode, MakeTop, )
				ImplRet(MakeTop({}))
				//@}

				/*!
				\brief ���� sxml �ĵ� XML �����ڵ㡣
				\note ��һ������ָ���ڵ����ƣ��������ָ���ڵ��� XML ������ֵ���汾������Ͷ����ԡ�
				\note �������������ѡΪ��ֵ����ʱ�����������Ӧ�����ԡ�
				\warning ���Բ����Ϲ��Խ��м�顣
				*/
				LS_API ValueNode
				MakeXMLDecl(const string& = {}, const string& = "1.0",
					const string& = "UTF-8", const string& = {});

			/*!
			\brief ������� XML ������ sxml �ĵ��ڵ㡣
			\sa MakeTop
			\sa MakeXMLDecl
			*/
			LS_API ValueNode
				MakeXMLDoc(const string& = {}, const string& = "1.0",
					const string& = "UTF-8", const string& = {});

			//! \brief ���� sxml ���Ա����������
			//@{
			inline PDefH(NodeLiteral, MakeAttributeTagLiteral,
				std::initializer_list<NodeLiteral> il)
				ImplRet({ "@", il })
				template<typename... _tParams>
			NodeLiteral
				MakeAttributeTagLiteral(_tParams&&... args)
			{
				return sxml::MakeAttributeTagLiteral({ NodeLiteral(lforward(args)...) });
			}
			//@}

			//! \brief ���� XML ������������
			//@{
			inline PDefH(NodeLiteral, MakeAttributeLiteral, const string& name,
				std::initializer_list<NodeLiteral> il)
				ImplRet({ name,{ MakeAttributeTagLiteral(il) } })
				template<typename... _tParams>
			NodeLiteral
				MakeAttributeLiteral(const string& name, _tParams&&... args)
			{
				return{ name,{ sxml::MakeAttributeTagLiteral(lforward(args)...) } };
			}
			//@}

			//! \brief ����ֻ�� XML ���Խڵ㵽�﷨�ڵ㡣
			//@{
			template<class _tNodeOrCon>
			inline void
				InsertAttributeNode(_tNodeOrCon&& node_or_con, const string& name,
					std::initializer_list<NodeLiteral> il)
			{
				InsertChildSyntaxNode(node_or_con, MakeAttributeLiteral(name, il));
			}
			template<class _tNodeOrCon, typename... _tParams>
			inline void
				InsertAttributeNode(_tNodeOrCon&& node_or_con, const string& name,
					_tParams&&... args)
			{
				InsertChildSyntaxNode(node_or_con,
					sxml::MakeAttributeLiteral(name, lforward(args)...));
			}
			//@}
			//@}

		} // namespace sxml;
		  //@}


		  //@{
		  //! \ingroup exceptions
		  //@{
		  //! \brief LSL �쳣���ࡣ
		class LS_API LSLException : public LoggedEvent
		{
		public:
			LB_NONNULL(2)
				LSLException(const char* str, leo::RecordLevel lv = leo::Err)
				: LoggedEvent(str, lv)
			{}
			LSLException(const leo::string_view sv,
				leo::RecordLevel lv = leo::Err)
				: LoggedEvent(sv, lv)
			{}
			DefDeCtor(LSLException)

				//! \brief ���������ඨ����Ĭ��ʵ�֡�
			virtual	~LSLException() override;
		};


		/*!
		\brief �б��Լʧ�ܡ�
		\todo ���񲢱�����������Ϣ��
		*/
		class LS_API ListReductionFailure : public LSLException
		{
		public:
			using LSLException::LSLException;
			DefDeCtor(ListReductionFailure)

				//! \brief ���������ඨ����Ĭ��ʵ�֡�
				~ListReductionFailure() override;
		};


		//! \brief �﷨����
		class LS_API InvalidSyntax : public LSLException
		{
		public:
			using LSLException::LSLException;
			DefDeCtor(InvalidSyntax)

				//! \brief ���������ඨ����Ĭ��ʵ�֡�
				~InvalidSyntax() override;
		};

		class LS_API ParameterMismatch : public InvalidSyntax
		{
		public:
			using InvalidSyntax::InvalidSyntax;
			DefDeCtor(ParameterMismatch)

				//! \brief ���������ඨ����Ĭ��ʵ�֡�
				~ParameterMismatch() override;
		};

		/*!
		\brief Ԫ����ƥ�����
		\todo ֧�ַ�Χƥ�䡣
		*/
		class LS_API ArityMismatch : public LSLException
		{
		private:
			size_t expected;
			size_t received;

		public:
			DefDeCtor(ArityMismatch)
				/*!
				\note ǰ����������ʾ������ʵ�ʵ�Ԫ����
				*/
				ArityMismatch(size_t, size_t, leo::RecordLevel = leo::Err);

			//! \brief ���������ඨ����Ĭ��ʵ�֡�
			~ArityMismatch() override;

			DefGetter(const lnothrow, size_t, Expected, expected)
				DefGetter(const lnothrow, size_t, Received, received)
		};

		/*!
		\brief ��ʶ������
		*/
		class LS_API BadIdentifier : public LSLException
		{
		private:
			leo::shared_ptr<string> p_identifier;

		public:
			/*!
			\brief ���죺ʹ����Ϊ��ʶ�����ַ�������֪ʵ�����ͺͼ�¼�ȼ���
			\pre ��Ӷ��ԣ���һ����������ָ��ǿա�

			������һ����������Ϊ��ʶ���ĺϷ��ԣ�ֱ����Ϊ������ı�ʶ����
			��ʼ����ʾ��ʶ��������쳣����
			ʵ�������� 0 ʱ��ʾδ֪��ʶ����
			ʵ�������� 1 ʱ��ʾ�Ƿ���ʶ����
			ʵ�������� 1 ʱ��ʾ�ظ���ʶ����
			*/
			LB_NONNULL(2)
				BadIdentifier(const char*, size_t = 0, leo::RecordLevel = leo::Err);
			BadIdentifier(string_view, size_t = 0, leo::RecordLevel = leo::Err);
			DefDeCtor(BadIdentifier)

				//! \brief ���������ඨ����Ĭ��ʵ�֡�
				~BadIdentifier() override;

			DefGetter(const lnothrow, const string&, Identifier,
				leo::Deref(p_identifier))
		};


		
		//@}
		//@}


		/*!
		\brief ��Լ״̬��һ���Լ���ܵ��м�����
		*/
		enum class ReductionStatus : limpl(size_t)
		{
			//@{
			//! \brief ��Լ�ɹ���ֹ�Ҳ���Ҫ�������
			Clean = 0,
				//! \brief ��Լ�ɹ�����Ҫ�������
				Retained,
				//! \brief ��Ҫ�ع�Լ��
				Retrying
				//@}
		};


		/*!
		\ingroup ThunkType
		\brief �ӳ���ֵ�
		\note �ͱ��ӳ���ֵ��������ڵ��ǲ�ͬ�İ�װ���͡�
		\warning �ǿ�������

		ֱ����Ϊ���ֵ�����װ���ӳ���ֵ���
		*/
		using DelayedTerm = leo::derived_entity<TermNode, LSLATag>;

		//@{
		//! \brief �����Ľڵ����͡�
		using ContextNode = limpl(ValueNode);

		using leo::AccessChildPtr;

		//! \brief �����Ĵ��������͡�
		using ContextHandler = leo::GHEvent<ReductionStatus(TermNode&, ContextNode&)>;
		//! \brief ���������������͡�
		using LiteralHandler = leo::GHEvent<ReductionStatus(const ContextNode&)>;

		//! \brief ע�������Ĵ�������
		inline PDefH(void, RegisterContextHandler, ContextNode& node,
			const string& name, ContextHandler f)
			ImplExpr(node[name].Value = std::move(f))

			//! \brief ע����������������
			inline PDefH(void, RegisterLiteralHandler, ContextNode& node,
				const string& name, LiteralHandler f)
			ImplExpr(node[name].Value = std::move(f))
			//@}


			//@{
			//! \brief ���������
			enum class LexemeCategory
		{
			//! \brief �ޣ�����������
			Symbol,
			//! \brief ������������
			Code,
			//! \brief ������������
			Data,
			/*!
			\brief ��չ��������������ʵ�ֶ����������������
			*/
			Extended
		};



		//! \sa LexemeCategory
		//@{
		/*!
		\pre ���ԣ��ַ�������������ָ��ǿ����ַ����ǿա�
		\return �жϵķ���չ���������ࡣ
		*/
		//@{
		/*!
		\brief ���ų���չ�������Ĵ��ط��ࡣ
		\note ��չ��������Ϊ����������
		*/
		LS_API LexemeCategory
			CategorizeBasicLexeme(string_view) lnothrowv;

		/*!
		\brief �Դ��ط��ࡣ
		\sa CategorizeBasicLexeme
		*/
		LS_API LexemeCategory
			CategorizeLexeme(string_view) lnothrowv;
		//@}

		/*!
		\brief �жϲ��Ƿ���չ�������Ĵ����Ƿ�Ϊ LSLA ��չ��������
		\pre ���ԣ��ַ�������������ָ��ǿ����ַ����ǿա�
		\pre ���ز��Ǵ�����������������������
		*/
		LS_API bool
			IsLSLAExtendedLiteral(string_view) lnothrowv;

		/*!
		\brief �ж��ַ��Ƿ�Ϊ LSLA ��չ������������ǰ׺��
		*/
		lconstfn PDefH(bool, IsLSLAExtendedLiteralNonDigitPrefix, char c) lnothrow
			ImplRet(c == '#' || c == '+' || c == '-')

			//! \brief �ж��ַ��Ƿ�Ϊ LSLA ��չ������ǰ׺��
			inline PDefH(bool, IsLSLAExtendedLiteralPrefix, char c) lnothrow
			ImplRet(std::isdigit(c) || IsLSLAExtendedLiteralNonDigitPrefix(c))

			/*!
			\brief �жϴ����Ƿ�Ϊ LSLA ���š�
			\pre ��Ӷ��ԣ��ַ�������������ָ��ǿ����ַ����ǿա�
			*/
			inline PDefH(bool, IsLSLASymbol, string_view id) lnothrowv
			ImplRet(CategorizeLexeme(id) == LexemeCategory::Symbol)
			//@}
			//@}


			/*!
			\ingroup ThunkType
			\brief �Ǻ�ֵ��
			\note �ͱ���ֵ���ַ�����ͬ�İ�װ���͡�
			\warning �ǿ�������
			*/
			using TokenValue = leo::derived_entity<string, LSLATag>;


		/*!
		\brief ��ǼǺŽڵ㣺�ݹ�任�ڵ㣬ת�����еĴ���Ϊ�Ǻ�ֵ��
		\note �ȱ任�ӽڵ㡣
		*/
		LS_API void
			TokenizeTerm(TermNode& term);


		/*!
		\brief �Ա�ʾֵ�� ValueObject ���л�������Ȩ�������ڼ�鲢ȡ��ʾ�����õļ��ֵ��
		\throw LSLException ���ʧ�ܣ��������ж����Ψһ����Ȩ�����ܱ��ⲿ���ñ��档
		\throw leo::invalid_construction ����������ֵ��
		\todo ʹ�þ������������쳣���͡�
		*/
		LS_API ValueObject
			ReferenceValue(const ValueObject&);

		//@{
		//! \brief ��ָ�������Ĳ������ƶ�Ӧ�Ľڵ㡣
		template<typename _tKey>
		inline observer_ptr<ValueNode>
			LookupName(ContextNode& ctx, const _tKey& id) lnothrow
		{
			return leo::AccessNodePtr(ctx, id);
		}

		template<typename _tKey>
		inline observer_ptr<const ValueNode>
			LookupName(const ContextNode& ctx, const _tKey& id) lnothrow
		{
			return leo::AccessNodePtr(ctx, id);
		}

		//! \brief ��ָ��������ȡָ������ָ�Ƶ�ֵ��
		template<typename _tKey>
		ValueObject
			FetchValue(const ContextNode& ctx, const _tKey& name)
		{
			return GetValueOf(scheme::LookupName(ctx, name));
		}

		template<typename _tKey>
		static observer_ptr<const ValueObject>
			FetchValuePtr(const ContextNode& ctx, const _tKey& name)
		{
			return GetValuePtrOf(scheme::LookupName(ctx, name));
		}
		//@}

		/*!
		\brief �������ֵ��Ϊ���ơ�
		\return ͨ���������ֵȡ�õ����ƣ����ָ���ʾ�޷�ȡ�����ơ�
		*/
		LS_API observer_ptr<const string>
			TermToNamePtr(const TermNode&);

		/*!
		//@{
		\pre �ַ�������������ָ��ǿա�
		\note ���һ��������ʾǿ�Ƶ��á�
		\throw BadIdentifier ��ǿ�Ƶ���ʱ���ֱ�ʶ�������ڻ��ͻ��
		*/
		//@{
		//! \brief ���ַ���Ϊ��ʶ����ָ���������ж���ֵ��
		LS_API void
			DefineValue(ContextNode&, string_view, ValueObject&&, bool);

		/*!
		\brief ���ַ���Ϊ��ʶ����ָ���������и��Ƕ���ֵ��
		*/
		LS_API void
			RedefineValue(ContextNode&, string_view, ValueObject&&, bool);

		//! \brief ���ַ���Ϊ��ʶ����ָ���������Ƴ�����
		LS_API void
			RemoveIdentifier(ContextNode&, string_view, bool);
		//@}


		/*!
		\brief ���ݹ�Լ״̬����Ƿ�ɼ�����Լ��
		\see TraceDe

		ֻ��������״̬ȷ����������ҽ�����Լ�ɹ�ʱ����Ϊ������Լ��
		�����ֲ�֧�ֵ�״̬��Ϊ���ɹ���������档
		*/
		LB_PURE LS_API bool
			CheckReducible(ReductionStatus);

		inline PDefH(ReductionStatus, CheckNorm, const TermNode& term) lnothrow
			ImplRet(IsBranch(term) ? ReductionStatus::Retained : ReductionStatus::Clean)

		/*!
		\sa CheckReducible
		*/
		template<typename _func, typename... _tParams>
		void
			CheckedReduceWith(_func f, _tParams&&... args)
		{
			leo::retry_on_cond(CheckReducible, f, lforward(args)...);
		}


		//@{
		/*!
		\brief ��ϲ�������ε���ֱ���ɹ���
		\note �ϲ��������ڱ�ʾ�����ж��Ƿ�Ӧ������Լ������ѭ����ʵ���ٴι�Լһ���
		*/
		struct PassesCombiner
		{
			/*!
			\note �Ա�����쳣������
			*/
			template<typename _tIn>
			ReductionStatus
				operator()(_tIn first, _tIn last) const
			{
				auto res(ReductionStatus::Clean);

				return leo::fast_any_of(first, last, [&](ReductionStatus r) lnothrow{
					if (r == ReductionStatus::Retained)
					res = r;
				return r == ReductionStatus::Retrying;
				}) ? ReductionStatus::Retrying : res;
			}
		};


		/*!
		\note �����ʾ�ж��Ƿ�Ӧ������Լ��
		\sa PassesCombiner
		*/
		//@{
		//! \brief һ��ϲ��顣
		template<typename... _tParams>
		using GPasses = leo::GEvent<ReductionStatus(_tParams...),
			leo::GCombinerInvoker<ReductionStatus, PassesCombiner>>;
		//! \brief ��ϲ��顣
		using TermPasses = GPasses<TermNode&>;
		//! \brief ��ֵ�ϲ��顣
		using EvaluationPasses = GPasses<TermNode&, ContextNode&>;
		/*!
		\brief �������ϲ��顣
		\pre �ַ�������������ָ��ǿա�
		*/
		using LiteralPasses = GPasses<TermNode&, ContextNode&, string_view>;
		//@}


		//! \brief �������ػ����͡�
		using Guard = leo::any;
		/*!
		\brief �������ػ��飺�������ڹ�Լ���̵���ںͳ��ڹ���ִ�еĲ�����
		\todo ֧�ֵ���ʹ�þ�ֵ��
		*/
		using GuardPasses = leo::GEvent<Guard(TermNode&, ContextNode&),
			leo::GDefaultLastValueInvoker<Guard>>;


		/*!
		\brief ���ô���飺��ָ�����ƵĽڵ��з���ָ�����͵ı鲢��ָ�������ĵ��á�
		//@{
		*/
		template<class _tPasses, typename... _tParams>
		typename _tPasses::result_type
			InvokePasses(observer_ptr<const ValueNode> p, TermNode& term, ContextNode& ctx,
				_tParams&&... args)
		{
			return leo::call_value_or([&](const _tPasses& passes) {
				// XXX: Blocked. 'yforward' cause G++ 5.3 crash: internal compiler
				//	error: Segmentation fault.
				return passes(term, ctx, std::forward<_tParams>(args)...);
			}, leo::AccessPtr<const _tPasses>(p));
		}
		template<class _tPasses, typename... _tParams>
		inline typename _tPasses::result_type
			InvokePasses(const string& name, TermNode& term, ContextNode& ctx,
				_tParams&&... args)
		{
			return scheme::InvokePasses<_tPasses>(leo::AccessNodePtr(
				leo::as_const(ctx), name), term, ctx, lforward(args)...);
		}
		//@}

		/*!
		\brief ����ָ��ֵ�����ָ��Ϊ TermNode ���͵�ֵ��
		\return ���ɹ���Ϊ�������ָ�룬����Ϊָ��ԭֵ�����ָ�롣
		*/
		//@{
		inline PDefH(observer_ptr<const ValueObject>, AdjustTermValuePtr,
			observer_ptr<const TermNode> p_term)
			ImplRet(leo::nonnull_or(GetValuePtrOf(p_term)))
			inline PDefH(observer_ptr<const ValueObject>, AdjustTermValuePtr,
				observer_ptr<const ValueObject> p_vo)
			ImplRet(leo::nonnull_or(
				AdjustTermValuePtr(leo::AccessPtr<const TermNode>(p_vo)), p_vo))
			template<typename _tKey>
		observer_ptr<const ValueObject>
			AdjustTermValuePtr(const ContextNode& ctx, const _tKey& name)
		{
			return AdjustTermValuePtr(scheme::FetchValuePtr(ctx, name));
		}
		//@}

		//! \brief ʹ�õڶ�������ָ������������滻��һ��������ݡ�
		//@{
		inline PDefH(void, LiftTerm, TermNode& term, TermNode& tm)
			ImplExpr(term.SetContent(std::move(tm)))

			inline PDefH(void, LiftTerm, ValueObject& term_v, ValueObject& vo)
			ImplExpr(term_v = std::move(vo))
			inline PDefH(void, LiftTerm, TermNode& term, ValueObject& vo)
			ImplExpr(LiftTerm(term.Value, vo))
			//@}

			//@{
			inline PDefH(void, LiftTermRef, TermNode& term, TermNode& tm)
			ImplExpr(term.SetContentIndirect(tm))
			inline PDefH(void, LiftTermRef, ValueObject& term_v, const ValueObject& vo)
			ImplExpr(term_v = vo.MakeIndirect())
			inline PDefH(void, LiftTermRef, TermNode& term, const ValueObject& vo)
			ImplExpr(LiftTermRef(term.Value, vo))
			//@}
			//@}

			/*!
			\brief �����ӳ��
			*/
			inline PDefH(void, LiftDelayed, TermNode& term, DelayedTerm& tm)
			ImplExpr(LiftTermRef(term, tm))

			//! \pre ���ԣ�����ָ��������֦�ڵ㡣
			//@{
			/*!
			\brief �������ʹ���׸������滻������ݡ�
			*/
			inline PDefH(void, LiftFirst, TermNode& term)
			ImplExpr(IsBranch(term), LiftTerm(term, Deref(term.begin())))

			/*!
			\brief ����ĩ�ʹ�����һ�������滻������ݡ�
			*/
			inline PDefH(void, LiftLast, TermNode& term)
			ImplExpr(IsBranch(term), LiftTerm(term, Deref(term.rbegin())))
			//@}

			/*!
			\sa RemoveHead
			\note ʹ�� ADL RemoveHead ��
			*/
			//@{
			/*!
			\brief ��Լ��һ���ǽ�β���б����
			\return ReductionStatus::Clean ��

			������в�����һ�������ҵ�һ�������ǿ��б����Ƴ���
			������б���Ϊ��һ�������Ա��û�в������ĺ���Ӧ�á�
			*/
			LS_API ReductionStatus
			ReduceHeadEmptyList(TermNode&) lnothrow;

		/*!
		\brief ��ԼΪ�б���֦�ڵ��Ƴ���һ������������µ�������Ϊ�б�
		\return ���ɹ��Ƴ��� ReductionStatus::Retained ������Ϊ ReductionStatus::Clean��
		*/
		LS_API ReductionStatus
			ReduceToList(TermNode&) lnothrow;
		//@}


} // namespace scheme;

#endif
