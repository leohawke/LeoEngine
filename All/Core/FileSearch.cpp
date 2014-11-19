#include "..\IndePlatform\platform.h"

#include "FileSearch.h"



namespace leo{
	static std::vector<std::wstring> mDirs;
	void FileSearch::PushSearchDir(const std::wstring& dir){
		mDirs.push_back(dir);
	}
	void FileSearch::ClearSearchDirs(){
		mDirs.clear();
	}
	const std::vector<std::wstring>& FileSearch::SearchDirectors(){
		return mDirs;
	}

	std::wstring FileSearch::Search(const std::wstring& filename){
#ifdef WIN32
		WIN32_FIND_DATAW findData;
		for (auto & dir : mDirs){
			auto findHandle = FindFirstFileW((dir + filename).c_str(), &findData);
			if (findHandle != INVALID_HANDLE_VALUE){
				FindClose(findHandle);
				return dir+findData.cFileName;
			}
		}
		return filename;
#endif
	}
}