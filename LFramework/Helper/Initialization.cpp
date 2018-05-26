#include <LBase/scope_gurad.hpp>

#include "Initialization.h"
#include "LFramework/LCLib/Platform.h"
#include "LFramework/Win32/LCLib/NLS.h"
#include "LFramework/Service/Filesystem.h"
#include "LFramework/Adaptor/LAdaptor.h"
#include "LFramework/Service/TextFile.h"

#include <LScheme/Configuration.h>

using namespace Test;
using namespace platform_ex;
using namespace scheme;

namespace leo {
	using namespace IO;
	namespace {
#undef CONF_PATH
#undef DATA_DIRECTORY
#undef DEF_FONT_DIRECTORY
#undef DEF_FONT_PATH
#if LFL_Win32 || LFL_Linux
		const string&
			FetchWorkingRoot()
		{
			static const struct Init
			{
				string Path;

				Init()
					: Path([] {
#	if LFL_Win32
					IO::Path image(platform::ucast(
						platform_ex::Windows::FetchModuleFileName().data()));

					if (!image.empty())
					{
						image.pop_back();

						const auto& dir(image.Verify());

						if (!dir.empty() && dir.back() == FetchSeparator<char16_t>())
							return dir.GetMBCS();
					}
#	elif LFL_Android
					const char*
						sd_paths[]{ "/sdcard/", "/mnt/sdcard/", "/storage/sdcard0/" };

					for (const auto& path : sd_paths)
						if (IO::VerifyDirectory(path))
						{
							YTraceDe(Informative, "Successfully found SD card path"
								" '%s' as root path.", path);
							return path;
						}
						else
							YTraceDe(Informative,
								"Failed accessing SD card path '%s'.", path);
#	elif LFL_Linux
					// FIXME: What if link reading failed (i.e. permission denied)?
					// XXX: Link content like 'node_type:[inode]' is not supported.
					// TODO: Use implemnetation for BSD family OS, etc.
					auto image(IO::ResolvePath<leo::path<vector<string>,
						IO::PathTraits>>(string_view("/proc/self/exe")));

					if (!image.empty())
					{
						image.pop_back();

						const auto& dir(IO::VerifyDirectoryPathTail(
							leo::to_string_d(image)));

						if (!dir.empty() && dir.back() == FetchSeparator<char>())
							return dir;
					}
#	else
#		error "Unsupported platform found."
#	endif
					throw GeneralEvent("Failed finding working root path.");
				}())
				{
					TraceDe(Informative, "Initialized root directory path '%s'.",
						Path.c_str());
				}
			} init;

			return init.Path;
		}

		// TODO: Reduce overhead?
#	define CONF_PATH (FetchWorkingRoot() + "lconf.lsl").c_str()
#endif
#if LFL_DS
#	define DATA_DIRECTORY "/Data/"
#	define DEF_FONT_DIRECTORY "/Font/"
#	define DEF_FONT_PATH "/Font/FZYTK.TTF"
#elif LFL_Win32
#	define DATA_DIRECTORY FetchWorkingRoot()
#	define DEF_FONT_PATH (FetchSystemFontDirectory_Win32() + "SimSun.ttc")
	//! \since build 693
		inline PDefH(string, FetchSystemFontDirectory_Win32, )
			// NOTE: Hard-coded as Shell32 special path with %CSIDL_FONTS or
			//	%CSIDL_FONTS. See https://msdn.microsoft.com/en-us/library/dd378457.aspx.
			ImplRet(Windows::WCSToUTF8(Windows::FetchWindowsPath()) + "Fonts\\")
#elif LFL_Android
#	define DATA_DIRECTORY (FetchWorkingRoot() + "Data/")
#	define DEF_FONT_DIRECTORY "/system/fonts/"
#	define DEF_FONT_PATH "/system/fonts/DroidSansFallback.ttf"
#elif LFL_Linux
#	define DATA_DIRECTORY FetchWorkingRoot()
#	define DEF_FONT_PATH "./SimSun.ttc"
#else
#	error "Unsupported platform found."
#endif
#ifndef CONF_PATH
#	define CONF_PATH "yconf.txt"
#endif
#ifndef DATA_DIRECTORY
#	define DATA_DIRECTORY "./"
#endif
#ifndef DEF_FONT_DIRECTORY
#	define DEF_FONT_DIRECTORY DATA_DIRECTORY
#endif
	}

}

namespace Test {
	using namespace leo;

	leo::ValueNode&
		FetchRoot() lnothrow {
		static ValueNode Root = LoadConfiguration(true);
		if (Root.GetName() == "LFramework")
			Root = PackNodes(string(), std::move(Root));
		LF_Trace(Debug, "Root lifetime began.");
		return Root;
	}

	void
		WriteLSLA1Stream(std::ostream& os, scheme::Configuration&& conf)
	{
		leo::write_literal(os, Text::BOM_UTF_8) << std::move(conf);
	}

	ValueNode
		TryReadRawNPLStream(std::istream& is)
	{
		scheme::Configuration conf;

		is >> conf;
		TraceDe(Debug, "Plain configuration loaded.");
		if (!conf.GetNodeRRef().empty())
			return conf.GetNodeRRef();
		TraceDe(Warning, "Empty configuration found.");
		throw GeneralEvent("Invalid stream found when reading configuration.");
	}

	LB_NONNULL(1, 2)leo::ValueNode LoadLSLV1File(const char * disp, const char * path, leo::ValueNode(*creator)(), bool show_info)
	{
		auto res(TryInvoke([=]() -> ValueNode {
			if (!ufexists(path))
			{
				TraceDe(Debug, "Path '%s' access failed.", path);
				if (show_info)
					TraceDe(Notice, "Creating %s '%s'...", disp, path);

				swap_guard<int, void, decltype(errno)&> gd(errno, 0);

				// XXX: Failed on race condition detected.
				if (UniqueLockedOutputFileStream uofs{ std::ios_base::out
					| std::ios_base::trunc | platform::ios_noreplace, path })
					WriteLSLA1Stream(uofs, Nonnull(creator)());
				else
				{
					TraceDe(Warning, "Cannot create file, possible error"
						" (from errno) = %d: %s.", errno, std::strerror(errno));
					TraceDe(Warning, "Creating default file failed.");
					return {};
				}
				TraceDe(Debug, "Created configuration.");
			}
			if (show_info)
				TraceDe(Notice, "Found %s '%s'.", Nonnull(disp), path);
			// XXX: Race condition may cause failure, though file would not be
			//	corrupted now.
			if (SharedInputMappedFileStream sifs{ path })
			{
				TraceDe(Debug, "Accessible configuration file found.");
				if (Text::CheckBOM(sifs, Text::BOM_UTF_8))
					return TryReadRawNPLStream(sifs);
				TraceDe(Warning, "Wrong encoding of configuration file found.");
			}
			TraceDe(Err, "Configuration corrupted.");
			return {};
		}));

		if (res)
			return res;
		TraceDe(Notice, "Trying fallback in memory...");

		std::stringstream ss;

		ss << Nonnull(creator)();
		return TryReadRawNPLStream(ss);
	}

	leo::ValueNode LoadConfiguration(bool show_info)
	{
		return LoadLSLV1File("configuration file", CONF_PATH, [] {
			return leo::ValueNode(leo::NodeLiteral{ "LFramework",
			{ { "DataDirectory", DATA_DIRECTORY },{ "FontFile", DEF_FONT_PATH },
			{ "FontDirectory", DEF_FONT_DIRECTORY } } });
		}, show_info);
	}
}

