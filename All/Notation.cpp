#include "Notation.hpp"

#include <stack>
#include <cwchar>
#include <assert.h>


using namespace MCF;
using namespace std;

// 静态成员函数。
void Notation::xEscapeAndAppend(wstring &wcspush_backTo, const wchar_t *pwchBegin, std::size_t uLength){
	wcspush_backTo.reserve(wcspush_backTo.length() + uLength * 2);


	for (std::size_t i = 0; i < uLength; ++i){
		const auto ch = pwchBegin[i];
		switch (ch){
		case L'\\':
		case L'=':
		case L'{':
		case L'}':
		case L';':
			wcspush_backTo.push_back( L'\\');
			wcspush_backTo.push_back(ch);
			break;
		case L'\n':
			wcspush_backTo.push_back(L'\\');
			wcspush_backTo.push_back(L'n');
			break;
		case L'\b':
			wcspush_backTo.push_back(L'\\');
			wcspush_backTo.push_back(L'b');
			break;
		case L'\r':
			wcspush_backTo.push_back(L'\\');
			wcspush_backTo.push_back(L'r');
			break;
		case L'\t':
			wcspush_backTo.push_back(L'\\');
			wcspush_backTo.push_back(L't');
			break;
		default:
			wcspush_backTo.push_back(ch);
			break;
		}
	}
}
wstring Notation::xUnescapeAndConstruct(const wchar_t *pwchBegin, std::size_t uLength){
	wstring wcsRet;
	wcsRet.reserve(uLength + 1);
	wchar_t* pwchWrite = const_cast<wchar_t*>(wcsRet.c_str());


	enum STATE {
		NORMAL,
		SLASH_MATCH,
		HEX_WAIT_FOR_NEXT
	} eState = NORMAL;


	int nDecodedDigit;
	wchar_t chDecoded = 0;
	wchar_t awchHexBuffer[sizeof(wchar_t)* CHAR_BIT / 4 - 1];
	std::size_t uBufferIndex = 0;


	for (std::size_t i = 0; i < uLength; ++i){
		const auto ch = pwchBegin[i];
		switch (eState){
		case NORMAL:
			if (ch == L'\\'){
				eState = SLASH_MATCH;
			}
			else {
				*(pwchWrite++) = ch;
			}
			break;
		case SLASH_MATCH:
			switch (ch){
			case L'b':
				*(pwchWrite++) = L'\b';
				eState = NORMAL;
				break;
			case L'n':
				*(pwchWrite++) = L'\n';
				eState = NORMAL;
				break;
			case L'r':
				*(pwchWrite++) = L'\r';
				eState = NORMAL;
				break;
			case L't':
				*(pwchWrite++) = L'\t';
				eState = NORMAL;
				break;
			case L'x':
				uBufferIndex = 0;
				eState = HEX_WAIT_FOR_NEXT;
				break;
			case L'\n':
				eState = NORMAL;
				break;
			default:
				*(pwchWrite++) = ch;
				eState = NORMAL;
				break;
			}
			break;
		case HEX_WAIT_FOR_NEXT:
			if ((L'0' <= ch) && (ch <= L'9')){
				nDecodedDigit = ch - L'0';
			}
			else if ((L'a' <= ch) && (ch <= L'f')){
				nDecodedDigit = ch - L'a' + 0x0A;
			}
			else if ((L'A' <= ch) && (ch <= L'F')){
				nDecodedDigit = ch - L'A' + 0x0A;
			}
			else {
				*(pwchWrite++) = L'x';
				for (std::size_t _i = 0; _i < uBufferIndex; ++_i){
					*(pwchWrite++) = awchHexBuffer[_i];
				}
				eState = NORMAL;
				break;
			}
			if (uBufferIndex == sizeof(awchHexBuffer)/sizeof(wchar_t)){
				*(pwchWrite++) = (chDecoded << 4) | nDecodedDigit;
				eState = NORMAL;
			}
			else {
				awchHexBuffer[uBufferIndex++] = ch;
				chDecoded = (chDecoded << 4) | nDecodedDigit;
			}
			break;
		}
	}
	*pwchWrite = 0;


	return std::move(wcsRet);
}


void Notation::xExportPackageRecur(
	wstring &wcspush_backTo,
	const Notation::Package &pkgWhich,
	wstring &wcsPrefix,
	const wchar_t *pwchIndent,
	std::size_t uIndentLen
	){
	const auto uCurrentPrefixLen = wcsPrefix.length();
	auto pwchCurrentPrefix = wcsPrefix.c_str();


	if (!pkgWhich.xm_mapPackages.empty()){
		if (pwchIndent){
			wcsPrefix.append(pwchIndent, uIndentLen);
		}
		pwchCurrentPrefix = wcsPrefix.c_str();


		for (const auto &SubPackageItem : pkgWhich.xm_mapPackages){
			wcspush_backTo.append(pwchCurrentPrefix, uCurrentPrefixLen);
			xEscapeAndAppend(wcspush_backTo, SubPackageItem.first.m_pwchBegin, SubPackageItem.first.m_uLength);
			wcspush_backTo.append(L" {\n", 3);


			xExportPackageRecur(wcspush_backTo, SubPackageItem.second, wcsPrefix, pwchIndent, uIndentLen);


			wcspush_backTo.append(pwchCurrentPrefix, uCurrentPrefixLen);
			wcspush_backTo.append(L"}\n", 2);
		}


		if (pwchIndent){
			wcsPrefix.resize(wcsPrefix.size()-uIndentLen);
		}
	}


	for (const auto &ValueItem : pkgWhich.xm_mapValues){
		wcspush_backTo.append(pwchCurrentPrefix, uCurrentPrefixLen);
		xEscapeAndAppend(wcspush_backTo, ValueItem.first.m_pwchBegin, ValueItem.first.m_uLength);
		wcspush_backTo.append(L" = ", 3);
		xEscapeAndAppend(wcspush_backTo, ValueItem.second.c_str(), ValueItem.second.length());
		wcspush_backTo.push_back(L'\n');
	}
}


// 构造函数和析构函数。
Notation::Notation(){
}
Notation::Notation(const wchar_t *pwszText){
	Parse(pwszText);
}
Notation::Notation(const wchar_t *pwchText, std::size_t uLen){
	Parse(pwchText, uLen);
}


// 其他非静态成员函数。
std::pair<Notation::ERROR_TYPE, const wchar_t *> Notation::Parse(const wchar_t *pwszText){
	return Parse(pwszText, std::wcslen(pwszText));
}
std::pair<Notation::ERROR_TYPE, const wchar_t *> Notation::Parse(const wchar_t *pwchText, std::size_t uLen){
	Clear();


	if (uLen == 0){
		return std::make_pair(ERR_NONE, nullptr);
	}


	const wchar_t *pwszRead = pwchText;


	stack<Package *> vecPackageStack;
	vecPackageStack.push(this);


	const wchar_t *pNameBegin = pwszRead;
	const wchar_t *pNameEnd = pwszRead;
	const wchar_t *pValueBegin = pwszRead;
	const wchar_t *pValueEnd = pwszRead;
	enum STATE {
		NAME_INDENT,
		NAME_BODY,
		NAME_PADDING,
		VAL_INDENT,
		VAL_BODY,
		VAL_PADDING,
		COMMENT
	} eState = NAME_INDENT;
	bool bEscaped = false;


	const auto PushPackage = [&]() -> void {
		assert(pNameBegin != pNameEnd);


		auto wcsName = xUnescapeAndConstruct(pNameBegin, (std::size_t)(pNameEnd - pNameBegin));
		auto &pkgNew = vecPackageStack.top()->xm_mapPackages.emplace(std::move(wcsName), Package()).first->second;
		vecPackageStack.push(&pkgNew);


		pNameBegin = pwszRead;
		pNameEnd = pwszRead;
	};
	const auto PopPackage = [&]() -> void {
		assert(vecPackageStack.size() > 0);


		vecPackageStack.pop();


		pNameBegin = pwszRead;
		pNameEnd = pwszRead;
	};
	const auto SubmitValue = [&]() -> void {
		assert(pNameBegin != pNameEnd);


		auto wcsName = xUnescapeAndConstruct(pNameBegin, (std::size_t)(pNameEnd - pNameBegin));
		auto wcsValue = xUnescapeAndConstruct(pValueBegin, (std::size_t)(pValueEnd - pValueBegin));
		vecPackageStack.top()->xm_mapValues[std::move(wcsName)] = std::move(wcsValue);


		pValueBegin = pwszRead;
		pValueEnd = pwszRead;
	};


	for (std::size_t i = 0; i < uLen; ++i, ++pwszRead){
		const wchar_t ch = *pwszRead;


		if (bEscaped){
			bEscaped = false;
		}
		else {
			switch (ch){
			case L'\\':
				bEscaped = true;
				continue;
			case L'=':
				switch (eState){
				case NAME_INDENT:
					return std::make_pair(ERR_NO_VALUE_NAME, pwszRead);
				case NAME_BODY:
				case NAME_PADDING:
					eState = VAL_INDENT;
					continue;
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					break;
				case COMMENT:
					continue;
				};
				break;
			case L'{':
				switch (eState){
				case NAME_INDENT:
					return std::make_pair(ERR_NO_VALUE_NAME, pwszRead);
				case NAME_BODY:
				case NAME_PADDING:
					PushPackage();
					eState = NAME_INDENT;
					continue;
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					PushPackage();
					eState = NAME_INDENT;
					continue;
				case COMMENT:
					continue;
				};
				break;
			case L'}':
				switch (eState){
				case NAME_INDENT:
					if (vecPackageStack.size() == 1){
						return std::make_pair(ERR_UNEXCEPTED_PACKAGE_CLOSE, pwszRead);
					}
					PopPackage();
					eState = NAME_INDENT;
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					if (vecPackageStack.size() == 1){
						return std::make_pair(ERR_UNEXCEPTED_PACKAGE_CLOSE, pwszRead);
					}
					PopPackage();
					eState = NAME_INDENT;
					continue;
				case COMMENT:
					continue;
				};
				break;
			case L';':
				switch (eState){
				case NAME_INDENT:
					eState = COMMENT;
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					eState = COMMENT;
					continue;
				case COMMENT:
					continue;
				};
				break;
			case L'\n':
				switch (eState){
				case NAME_INDENT:
					continue;
				case NAME_BODY:
				case NAME_PADDING:
					return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
				case VAL_INDENT:
				case VAL_BODY:
				case VAL_PADDING:
					SubmitValue();
					eState = NAME_INDENT;
					continue;
				case COMMENT:
					eState = NAME_INDENT;
					continue;
				};
				break;
			}
		}


		if (ch != L'\n'){
			switch (eState){
			case NAME_INDENT:
				if ((ch == L' ') || (ch == L'\t')){
					// eState = NAME_INDENT;
				}
				else {
					pNameBegin = pwszRead;
					pNameEnd = pwszRead + 1;
					eState = NAME_BODY;
				}
				continue;
			case NAME_BODY:
				if ((ch == L' ') || (ch == L'\t')){
					eState = NAME_PADDING;
				}
				else {
					pNameEnd = pwszRead + 1;
					// eState = NAME_BODY;
				}
				continue;
			case NAME_PADDING:
				if ((ch == L' ') || (ch == L'\t')){
					// eState = NAME_PADDING;
				}
				else {
					pNameEnd = pwszRead + 1;
					eState = NAME_BODY;
				}
				continue;
			case VAL_INDENT:
				if ((ch == L' ') || (ch == L'\t')){
					// eState = VAL_INDENT;
				}
				else {
					pValueBegin = pwszRead;
					pValueEnd = pwszRead + 1;
					eState = VAL_BODY;
				}
				continue;
			case VAL_BODY:
				if ((ch == L' ') || (ch == L'\t')){
					eState = VAL_PADDING;
				}
				else {
					pValueEnd = pwszRead + 1;
					// eState = VAL_BODY;
				}
				continue;
			case VAL_PADDING:
				if ((ch == L' ') || (ch == L'\t')){
					// eState = VAL_PADDING;
				}
				else {
					pValueEnd = pwszRead + 1;
					eState = VAL_BODY;
				}
				continue;
			case COMMENT:
				continue;
			}
		}
	}
	if (bEscaped){
		return std::make_pair(ERR_ESCAPE_AT_EOF, pwszRead);
	}
	if (vecPackageStack.size() > 1){
		return std::make_pair(ERR_UNCLOSED_PACKAGE, pwszRead);
	}
	switch (eState){
	case NAME_BODY:
	case NAME_PADDING:
		return std::make_pair(ERR_EQU_EXPECTED, pwszRead);
	case VAL_INDENT:
	case VAL_BODY:
	case VAL_PADDING:
		SubmitValue();
		break;
	default:
		break;
	};


	return std::make_pair(ERR_NONE, nullptr);
}
wstring Notation::Export(const wchar_t *pwchIndent) const {
	wstring wcsResult;
	wstring wcsPrefix;
	const std::size_t uIndentLength = pwchIndent ? (std::size_t)0 : std::wcslen(pwchIndent);
	xExportPackageRecur(wcsResult, *this, wcsPrefix, pwchIndent, uIndentLength);
	return std::move(wcsResult);
}

