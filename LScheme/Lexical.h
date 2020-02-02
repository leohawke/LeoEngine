/*\par �޸�ʱ�� :
2017-02-05 21:11 +0800
*/

#ifndef LScheme_Lexical_H
#define LScheme_Lexical_H 1

#include <LFramework/Adaptor/LAdaptor.h>
#include "inc_Sxml.h"
#include <cctype>

namespace scheme {
	using leo::list;
	using leo::string;
	using leo::string_view;
	using leo::u8string_view;
	using leo::vector;
	using leo::begin;
	using leo::end;

	class LS_API UnescapeContext
	{
	public:
		/*!
		\brief ת������ǰ׺��
		\note �����Ĵ���ֱ���޸ģ�һ����ת������ǰ׺���������á�
		\sa LexicalAnalyzer::PrefixHandler
		*/
		string Prefix;

	private:
		//! \brief ��Чת�����С�
		string sequence;

	public:
		DefDeCtor(UnescapeContext)
			DefDeCopyMoveCtorAssignment(UnescapeContext)

			DefPred(const lnothrow, Handling, !Prefix.empty())
			/*!
			\pre ���ԣ�����������ָ��ǿա�
			*/
			PDefH(bool, IsHandling, string_view pfx) const
			ImplRet((LAssertNonnull(pfx.data()), Prefix == pfx))

			DefGetter(const lnothrow, const string&, Sequence, sequence)

			PDefH(void, Clear, ) lnothrow
			ImplExpr(Prefix.clear(), sequence.clear())

			string
			Done();

		PDefH(bool, PopIf, byte uc)
			ImplRet(PopIf(char(uc)))
			PDefH(bool, PopIf, char c)
			ImplRet(leo::pop_back_val(sequence,c))
			PDefH(void, Push, byte uc)
			ImplExpr(Push(char(uc)))
			PDefH(void, Push, char c)
			ImplExpr(sequence += c)
	};

	/*!
	\brief ���÷�б��ת��ǰ׺�������� '\\' ʱ����ǰ׺Ϊ "\\" ��
	\sa LexicalAnalyzer::PrefixHandler
	*/
	LS_API bool
		HandleBackslashPrefix(char, string&);

	/*!
	\brief LScheme ת��ƥ���㷨��
	\sa LexicalAnalyzer::Unescaper

	֧��ת������Ϊ "\\" �� "\a" �� "\b" �� "\f" �� "\n" �� "\r" �� "\t" �� "\v" ��
	������˵���⣬ת����������μ� ISO C++11 ���ų� raw-string-literal ����
	����ת������������ʵ�ֶ��塣
	���˷�б��ת���⣬����ת�����������������
	��б��ת�壺����������б�ܱ��滻Ϊһ����б�ܣ�
	����ת�壺��б��֮����ӵ����Ż�˫����ʱ����б�ܻᱻɾ����
	*/
	LS_API bool
		LSLUnescape(string&, const UnescapeContext&, char);


	/*!
	\brief �ʷ���������
	\pre ������ݵ��ֽڵĻ����ַ����ı��룬������ַ���Ϊ ASCII ʱ���� UTF-8 �ı���
	\post �����м��������������ⲻ���ڿո�����Ŀհ׷����������������Ŀո�

	���ֽ�Ϊ������λ�Ĵʷ���������
	�����ֽ���������������룬�������� string �С�
	�����ַ������ַ���ֵ��֤������ [0, 0x7F) �ڡ�
	�ɽ��ܵĵ��ַ��ʷ��ָ�������֤�ڻ����ַ����ڡ�
	������򣨰�����˳�򣩣�
	�������ӣ���б��֮����ӻ��з���˫�ַ�������Ϊ���з����ᱻɾ����
	ת�壺ת�������滻Ϊ��ת���ַ���
	��������������δ��ת��ĵ����Ż�˫���ź��������������״̬���������Ϲ���
		ֱ�����ֽ����ԭʼ���룬ֱ��������Ӧ����һ�����š�
	խ�ַ��հ׷��滻�����ֽڿո�ˮƽ/��ֱ�Ʊ�������з�
		���滻Ϊ��һ�ո񣻻س����ᱻ���ԣ�
	ԭʼ����������ַ��������ֽ������
	֧������ת���㷨��Ĭ��ʵ�ֲμ� LSLUnescape ��
	*/
	class LS_API LexicalAnalyzer
	{
	public:
		/*!
		\brief ת������ǰ׺��������
		\note ����Ϊ��ǰ������ַ��ͷ�ת�������ĵ�ת������ǰ׺���á�
		\note ����ֵ��ʾ�Ƿ��޸���ǰ׺��
		*/
		using PrefixHandler = std::function<bool(char, string&)>;
		/*!
		\brief ָ��ƥ��ת�����еķ�ת���㷨������ת�����в������޸�ָ�����档
		\note ������ʾ������桢��ת�������ĺ͵�ǰ���ڴ���ı߽��ַ��������ţ���
		\note ����ֵ��ʾ�Ƿ��޸���������档
		*/
		using Unescaper
			= std::function<bool(string&, const UnescapeContext&, char)>;

	private:
		/*!
		\brief ָʾ��ǰ���з�״̬��
		*/
		char line_concat = {};
		/*!
		\brief ��ת�������ģ����淴ת���м�����
		\note ��ǰ׺�ǿձ�ʾ���ڴ���ת�塣
		*/
		UnescapeContext unescape_context;
		/*!
		\brief ����ָ���״̬����ʾ���ڴ����������е���Ч�ַ���
		\note ֵΪ���ַ�ʱ��ʾ��ǰ��������������
		*/
		char ld;
		/*!
		\brief �ַ������м�����
		*/
		string cbuf;
		/*!
		\brief �ַ������м����з�ת������ų��ֵ�λ�õ������б�
		*/
		vector<size_t> qlist;

	public:
		LexicalAnalyzer();
		//! \since build 546
		DefDeCopyMoveCtorAssignment(LexicalAnalyzer)

			DefGetter(const lnothrow, const string&, Buffer, cbuf)
			//@{
			DefGetter(const lnothrow, const vector<size_t>&, Quotes, qlist)

	private:
		bool
			CheckEscape(byte, Unescaper);

		bool
			CheckLineConcatnater(char, char = '\\', char = '\n');
		//@}

		bool
			FilterForParse(char, Unescaper, PrefixHandler);

	public:
		/*!
		\note ����ָ����ת���㷨��
		\warning ��ͬһ�������������ϻ��ò��ȼ۵Ķ��ֽ���������ת���㷨�Ľ��δָ����
		*/
		//@{
		/*!
		\brief ���������ַ���������ַ����������
		\note ��¼���Ų����Կ��ַ���
		*/
		void
			ParseByte(char, Unescaper = LSLUnescape,
				PrefixHandler = HandleBackslashPrefix);

		//! \brief ���������������ַ���������ַ������������ת���������ӱ߽��ַ���
		void
			ParseQuoted(char, Unescaper = LSLUnescape,
				PrefixHandler = HandleBackslashPrefix);

		/*!
		\brief ֱ������ַ���
		*/
		PDefH(void, ParseRaw, char c)
			ImplExpr(cbuf += c);
		//@}

		/*!
		\brief �����м���ȡ�ַ����б�
		\note ����ÿһ���������������������������
		*/
		list<string>
			Literalize() const;
	};


	/*!
	\pre ���ԣ��ַ�������������ָ��ǿա�
	*/
	//@{
	/*!
	\brief ���ָ���ַ����Ƿ�Ϊ��������
	\return ��Ϊ����������β�ַ���Ϊ '\'' �� '"' ֮һ������Ϊ���ַ�������Ϊ char() ��
	*/
	LS_API char
		CheckLiteral(string_view) lnothrowv;

	/*!
	\brief ȥ���������߽�ָ�����
	\pre ���ԣ�����������ָ��ǿա�
	\return ����ʶ��������������Ϊȥ����β�ַ�֮��ĸ���������Ϊԭ����
	*/
	//@{
	//! \note ʹ�� CheckLiteral �жϡ�
	LS_API string_view
		Deliteralize(string_view) lnothrowv;

	inline PDefH(string_view, DeliteralizeUnchecked, string_view sv) lnothrowv
		ImplRet(LAssertNonnull(sv.data()), leo::get_mid(sv))
		//@}

		/*!
		\brief ����ת���ַ������滻ָ���ַ����еĿ�ת���ַ�Ϊת�����С�
		\sa LexicalAnalyzer
		*/
		LS_API string
		Escape(string_view);

	/*!
	\brief ����ת���ַ�����������
	\return ���������ַ���������ʱת�����е����ݣ�����Ϊԭ����
	\note ʹ�� Escape ת�塣
	\note ��ת������һ���ַ�Ϊ '\\' �����һ�� '\\' �Ա���ת��ĩβ�ָ�����
	\sa LexicalAnalyzer
	*/
	LS_API string
		EscapeLiteral(string_view);

	/*!
	\brief ���� XML �ַ�����
	\see http://www.w3.org/TR/2006/REC-xml11-20060816/#charsets ��

	���� XML 1.1 �ַ������Կ��ַ�ʹ�� TraceDe ���о��档
	���� XML 1.0 �� XML 1.1 �涨��������ʹ���ַ� \c & �� \c < �� \c > ����ת�����С�
	*/
	LS_API string
		EscapeXML(string_view);

	/*!
	\brief �����ַ���Ϊ��������
	\return ����β�ַ���Ϊ '\'' �� '"' ֮һ��ڶ�����Ϊ���ַ���Ϊԭ����
	����Ϊ��β���ϵڶ������ַ��Ĵ���
	*/
	LS_API string
		Literalize(string_view, char = '"');
	//@}


	/*!
	\brief �ж��Ƿ�Ϊ LSL ͼ�ηָ�����
	*/
	lconstfn bool
		IsGraphicalDelimeter(char c)
	{
		//	return std::ispunct(c);
		return c == '(' || c == ')' || c == ':' || c == ',' || c == ';';
	}

	/*!
	\brief �ж��Ƿ�Ϊ LSL �ָ�����
	*/
	lconstfn bool
		IsDelimeter(char c)
	{
		return byte(c) < 0x80 && (!std::isgraph(c) || IsGraphicalDelimeter(c));
	}


	/*!
	\brief �ֽ��ַ���Ϊ�Ǻš�
	\pre ���ԣ��ַ�������������ָ��ǿա�
	\post ������ַ������˲����� "C" ���� \tt std::isspace ���ط�����ַ���
	*/
	LS_API list<string>
		Decompose(string_view);

	/*!
	\brief �ǺŻ�����ȡ�ַ����б��еļǺš�
	\note �ų����������ֽ������ַ���Ϊ�Ǻ��б�
	*/
	LS_API list<string>
		Tokenize(const list<string>&);
}
#endif
