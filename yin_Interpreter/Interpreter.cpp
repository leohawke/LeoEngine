#include "Interpreter.hpp"

namespace leo
{
	namespace yin
	{
		class Node;
		class ParserException;
		class Parser;
		class Util;
		class Scope;
		Value Interpreter::interp()
		{
			return interp(file);
		}
		Value Interprter::interp(const std::wstring& file)
		{
			Node program;
			try{
				program = Parser::parse(file);
			}
			catch (ParserException& e){
				Util::abort(e.what());
				return nullptr;
			}
			return program.interp(Scope::buildInitScope());
		}
	}
}