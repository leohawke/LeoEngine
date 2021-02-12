/*! \file Engine\Asset\LSLAssetX.h
\ingroup Engine
\brief 当资源格式是LSL 的帮助函数 ...
*/
#ifndef LE_ASSET_LSL_X_H
#define LE_ASSET_LSL_X_H 1

#include <LScheme/LScheme.h>
#include "../Core/LFile.h"

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

		inline typename const scheme::TermNode* SelectNode(const char* name, const scheme::TermNode& node) {
			for (auto& child : node)
			{
				if (child.size() && leo::Access<std::string>(*child.begin()) == name) {
					return &child;
				}
			}
			return nullptr;
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

		inline scheme::TermNode LoadNode(const std::filesystem::path& path)
		{
			platform::File internal_file(path.wstring(), platform::File::kToRead);
			FileRead file{ internal_file };

			constexpr size_t bufferSize = 4096;
			auto buffer = std::make_unique<char[]>(bufferSize);

			scheme::LexicalAnalyzer lexer;
			for (std::uint64_t offset = 0, fileSize = internal_file.GetSize(); offset < fileSize;)
			{
				const auto bytesToRead = static_cast<size_t>(
					std::min<std::uint64_t>(bufferSize, fileSize - offset));

				const auto bytesRead = file.Read(buffer.get(), bytesToRead);

				std::for_each(buffer.get(), buffer.get() + bytesRead, [&](char c) {lexer.ParseByte(c); });

				offset += bytesRead;
			}

			return  scheme::SContext::Analyze(scheme::Tokenize(lexer.Literalize()));
		}
	}
}

#endif