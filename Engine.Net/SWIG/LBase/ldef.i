/*!	\defgroup preprocessor_helpers Perprocessor Helpers
\brief 预处理器通用助手宏。
\since build 1.02
*/
//@{

//\brief 替换列表。
//\note 通过括号保护，可用于传递带逗号的参数。
#define LPP_Args(...) __VA_ARGS__

//! \brief 替换为空的预处理记号。
#define LPP_Empty

/*!
\brief 替换为逗号的预处理记号。
\note 可用于代替宏的实际参数中出现的逗号。
*/
#define LPP_Comma ,

/*
\brief 记号连接。
\sa LPP_Join
*/
#define LPP_Concat(x,y) x ## y

/*
\brief 带宏替换的记号连接。
\see ISO WG21/N4140 16.3.3[cpp.concat]/3 。
\see http://gcc.gnu.org/onlinedocs/cpp/Concatenation.html 。
\see https://www.securecoding.cert.org/confluence/display/cplusplus/PRE05-CPP.+Understand+macro+replacement+when+concatenating+tokens+or+performing+stringification 。

注意 ISO C++ 未确定宏定义内 # 和 ## 操作符求值顺序。
注意宏定义内 ## 操作符修饰的形式参数为宏时，此宏不会被展开。
*/
#define LPP_Join(x,y) LPP_Concat(x,y)
//@}


/*!
\brief 实现标签。
\since build 1.02
\todo 检查语言实现的必要支持：可变参数宏。
*/
#define limpl(...) __VA_ARGS__

#define lnothrow noexcept
