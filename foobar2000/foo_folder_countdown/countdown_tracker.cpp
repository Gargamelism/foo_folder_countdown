#include "stdafx.h"

#include "../SDK/foobar2000.h"
#include "preferences.h"

template<class T>
class arr_t_wrapper 
{
public:
	arr_t_wrapper() : _arr(nullptr), _size(0) {};

	arr_t_wrapper(int size)
	{
		_arr = new T[size];
		_size = size;
	}

	void set_arr(const T *arr, int size)
	{
		delete_arr();

		_size = size;
		_arr = new T[_size];

		for (int i = 0; i < size; i++)
		{
			_arr[i] = arr[i];
		}
	}

	~arr_t_wrapper()
	{
		delete_arr();
	}

	int size() 
	{
		return _size;
	}

	T* ptr()
	{
		return _arr;
	}

	T &operator[] (int idx)
	{
		return _arr[idx];
	}

private:
	T* _arr;
	int _size;

	void delete_arr()
	{
		if (_arr)
		{
			delete[] _arr;
		}
	}
};

namespace foo_countdown {
	class countdown_tracker : public playback_statistics_collector {
		const int prefix_len = strlen("file://");

	public:
		virtual void on_item_played(metadb_handle_ptr p_item) override {
			auto utf_path = p_item.get_ptr()->get_path();
			int utf_path_len = strlen(utf_path);
			
			arr_t_wrapper<char> char_path;
			utf8_to_win_1252(char_path, utf_path + prefix_len, utf_path_len);

			folders_t & folders = folders_conf.get_folders();
			bool found = false;
			for (int i = 0; !found && i != folders.size(); ++i) {
				if (is_same_path(folders[i].get_path().c_str(), folders[i].get_path().get_length(), char_path.ptr(), char_path.size())) {
					folders[i].listened_to_song(char_path.ptr());
				}
			}
		}

	private:
		bool is_same_path(const char* path_a, int len_a, const char* path_b, int len_b)
		{
			bool is_same = true;
			int len = min(len_a, len_b);
			
			// this check has a bug as it will fail finding the correct path between "c:\path" and "c:\path1"
			// this will also fail for an empty string
			for (int i = 0; is_same && i < len; i++)
			{
				is_same = path_a[i] == path_b[i];
			}
			
			return is_same;
		}

		void utf8_to_win_1252(arr_t_wrapper<char> &dest, const char* source, int len)
		{
			arr_t_wrapper<wchar_t> wchar_path(len);
			pfc::stringcvt::convert_utf8_to_wide(wchar_path.ptr(), wchar_path.size(), source, len);

			arr_t_wrapper<char> char_path(len);
			pfc::stringcvt::convert_wide_to_win1252(char_path.ptr(), char_path.size(), wchar_path.ptr(), wchar_path.size());

			dest.set_arr(char_path.ptr(), len);
		}
	};

	static playback_statistics_collector_factory_t <countdown_tracker> g_countdown_tracker;
}

