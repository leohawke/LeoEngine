#include "Util.hpp"
#include "Scope.hpp"
int main()
{
	auto string =leo::yin::Util::readfile(L"SSHKEY.txt");
	string = leo::yin::Util::unifypath(L"SSHKEY.txt");
	return 0;
}