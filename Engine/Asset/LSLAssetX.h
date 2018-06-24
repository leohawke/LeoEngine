/*! \file Engine\Asset\LSLAssetX.h
\ingroup Engine
\brief ����Դ��ʽ��LSL �İ������� ...
*/
#ifndef LE_ASSET_LSL_X_H
#define LE_ASSET_LSL_X_H 1

#include <LScheme/LScheme.h>

namespace platform {
	namespace X {

		typename scheme::TermNode::Container SelectNodes(const char* name, const scheme::TermNode& node) {
			return node.SelectChildren([&](const scheme::TermNode& child) {
				if (child.size()) {
					return leo::Access<std::string>(*child.begin()) == name;
				}
				return false;
			});
		}
	}
}

#endif