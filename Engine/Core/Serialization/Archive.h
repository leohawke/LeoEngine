#pragma once

#include "Core/Coroutine/Task.h"
#include <LBase/linttype.hpp>
#include <LBase/lmathtype.hpp>

#include <concepts>
#include <filesystem>
#include <vector>
#include <optional>
#include <type_traits>
#include <optional>

namespace LeoEngine
{
	using leo::coroutine::Task;

	class ArchiveState
	{
	public:
		ArchiveState()
			:ArIsLoading(false),
			ArIsSaving(false)
		{
		}

		/** Returns true if this archive is for loading data. */
		bool IsLoading() const
		{
			return ArIsLoading;
		}

		/** Returns true if this archive is for saving data, this can also be a pre-save preparation archive. */
		bool IsSaving() const
		{
			return ArIsSaving;
		}
	protected:
		/** Whether this archive is for loading data. */
		leo::uint8 ArIsLoading : 1;

		/** Whether this archive is for saving data. */
		leo::uint8 ArIsSaving : 1;
	};

	class AsyncArchive : public ArchiveState
	{
	public:
		Task<AsyncArchive&> operator>>(std::integral auto& v)
		{
			return Serialize(&v, sizeof(v));
		}

		Task<AsyncArchive&> operator>>(std::string& v)
		{
			std::size_t size;
			co_await Serialize(&size,sizeof(size));
			if (IsLoading())
			{
				v.resize(size);
			}
			co_await Serialize(v.data(), size);
			co_return *this;
		}

		template<typename T> requires std::is_enum_v<T>
		Task<AsyncArchive&> operator>>(T& v)
		{
				std::underlying_type_t<T>& value = reinterpret_cast<std::underlying_type_t<T>&>(v);
				co_await Serialize(&value, sizeof(value));
				co_return *this;
		}
	public:
		virtual Task<AsyncArchive&> Serialize(void* v,leo::uint64 length){
			co_return *this;
		}
	};

	AsyncArchive* CreateFileReader(const std::filesystem::path& filename);
	AsyncArchive* CreateFileWriter(const std::filesystem::path& filename);

	template<class T>
	concept Serializable = 
	requires(AsyncArchive& archive,T& v) 
	{
		 archive>>v;
	};

	template<typename T> 
	requires requires(AsyncArchive& archive, T& v)
	{
		{v.Serialize(archive) } ->std::same_as<Task<void>>;
	}
	Task<void> operator>>(AsyncArchive& archive, T& v)
	{
		return v.Serialize(archive);
	}

	template<typename T> requires Serializable<T> && std::default_initializable<T>
	Task<AsyncArchive&> operator>>(AsyncArchive& archive, std::vector<T>& v)
	{
		std::size_t size;
		co_await(archive >> size);
		if (archive.IsLoading())
		{
			v.resize(size);
		}
		for (int i = 0; i != v.size(); ++i)
			co_await(archive >> v[i]);

		co_return archive;
	}

	template<typename T> requires Serializable<T>&& std::default_initializable<T>
	Task<AsyncArchive&> operator>>(AsyncArchive& archive, std::optional<T>& v)
	{
		bool has_value = v.has_value();
		co_await(archive >> has_value);
		if (archive.IsLoading() && has_value)
		{
			v = {};
			co_await(archive >> *v);
		}
		else if(has_value)
			co_await(archive >> *v);
		co_return archive;
	}

	template<typename T> requires std::integral<T>
	Task<void> operator>>(AsyncArchive& archive, leo::math::vector3<T>& v)
	{
		co_await archive.Serialize(v.begin(), sizeof(T) * 3);
	}

}