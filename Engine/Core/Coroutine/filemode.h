#pragma once

namespace leo::coroutine {
	enum class file_share_mode
	{
		/// Don't allow any other processes to open the file concurrently.
		none = 0,

		/// Allow other processes to open the file in read-only mode
		/// concurrently with this process opening the file.
		read = 1,

		/// Allow other processes to open the file in write-only mode
		/// concurrently with this process opening the file.
		write = 2,

		/// Allow other processes to open the file in read and/or write mode
		/// concurrently with this process opening the file.
		read_write = read | write,

		/// Allow other processes to delete the file while this process
		/// has the file open.
		delete_ = 4
	};

	constexpr file_share_mode operator|(file_share_mode a, file_share_mode b)
	{
		return static_cast<file_share_mode>(
			static_cast<int>(a) | static_cast<int>(b));
	}

	constexpr file_share_mode operator&(file_share_mode a, file_share_mode b)
	{
		return static_cast<file_share_mode>(
			static_cast<int>(a) & static_cast<int>(b));
	}

	enum class file_buffering_mode
	{
		default_ = 0,
		sequential = 1,
		random_access = 2,
		unbuffered = 4,
		write_through = 8,
		temporary = 16
	};

	constexpr file_buffering_mode operator&(file_buffering_mode a, file_buffering_mode b)
	{
		return static_cast<file_buffering_mode>(
			static_cast<int>(a) & static_cast<int>(b));
	}

	constexpr file_buffering_mode operator|(file_buffering_mode a, file_buffering_mode b)
	{
		return static_cast<file_buffering_mode>(static_cast<int>(a) | static_cast<int>(b));
	}

	enum class file_open_mode
	{
		/// Open an existing file.
		///
		/// If file does not already exist when opening the file then raises
		/// an exception.
		open_existing,

		/// Create a new file, overwriting an existing file if one exists.
		///
		/// If a file exists at the path then it is overwitten with a new file.
		/// If no file exists at the path then a new one is created.
		create_always,

		/// Create a new file.
		///
		/// If the file already exists then raises an exception.
		create_new,

		/// Open the existing file if one exists, otherwise create a new empty
		/// file.
		create_or_open,

		/// Open the existing file, truncating the file size to zero.
		///
		/// If the file does not exist then raises an exception.
		truncate_existing
	};
}