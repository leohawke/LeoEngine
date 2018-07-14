/*! \file Engine\Asset\LSLAssetX.h
\ingroup Engine
\brief 当资源格式是LSL 的帮助函数 ...
*/
#ifndef LE_ASSET_LSL_X_H
#define LE_ASSET_LSL_X_H 1

#include <LScheme/LScheme.h>

namespace platform {
	namespace X {

		inline typename scheme::TermNode::Container SelectNodes(const char* name, const scheme::TermNode& node) {
			return node.SelectChildren([&](const scheme::TermNode& child) {
				if (child.size()) {
					return leo::Access<std::string>(*child.begin()) == name;
				}
				return false;
			});
		}

		inline void ReduceLFToTab(std::string& str, size_t length) {
			auto index = str.find('\n');
			while (index != std::string::npos) {
				auto next_index = str.find('\n', index+1);
				if (next_index != std::string::npos)
					if(next_index -index >length || ((next_index - index) < length && (next_index % length) < (index % length))) {
						index = next_index;
						continue;
					}
				str[index] = '\t';
				index = next_index;
			}
		}
	}
}

#endif