#pragma once

#include "resource.h"
#include "folder_countdown_helpers.h"

namespace foo_countdown {
#ifndef DEFAULT_PATH
#define DEFAULT_PATH "folder path"
#endif // !DEFAULT_PATH

	const int npos = ~0;

	typedef unsigned int uint;

	class file_count_t {
	public:
		file_count_t() : _path(DEFAULT_PATH), _count(0) {};
		file_count_t(const char* p, uint c) : _path(p), _count(c) {};

		file_count_t(file_count_t& f) {
			set_path(f.path());
			set_count(f.count());
		};

		file_count_t(const file_count_t& f) {
			set_path(f._path.c_str());
			set_count(f._count);
		};

		uint count() { return _count; };
		void set_count(uint count) { _count = count; };
		const char* path() { return _path.c_str(); };
		void set_path(const char* path) { _path = path; };

		void inc_count() { ++_count; };

	private:
		pfc::string8 _path;
		uint _count;
	};

	typedef array_t<file_count_t> files_count_t;

	class folder_countdown_t {
	public:
		folder_countdown_t() : _path(DEFAULT_PATH), _global_count(0), _play_count(0) {
		};

		void add_folder(const char* path, unsigned int count);
		void update_play_count(const char* file_name, unsigned int count);
		void reset();
		pfc::string8 get_path();
		unsigned int get_count();
		unsigned int get_max_plays();

		bool listened_to_song(const char* path);

		files_count_t get_files_count();

	private:
		static const int _allowed_extensions_size = 4;
		pfc::string8 _allowed_extensions[_allowed_extensions_size] = { ".mp3", ".mp4", ".ogg", ".m4a" };
		bool is_in_allowed_extensions(const char* ext);

		unsigned int _global_count;
		unsigned int _play_count;
		pfc::string8 _path;

		files_count_t _files_count = files_count_t();

		void initialize_files();
		void reset_play_count();
	};

	typedef array_t<folder_countdown_t> folders_t;

	struct folder_conf_id {
		int path;
		int max_count;
		int current_count;
	};

	const int CONF_IDS_SIZE = 10;
	const folder_conf_id conf_ids[CONF_IDS_SIZE] = {
				{IDC_FOLDER1, IDC_MAX_COUNT1, IDC_COUNT1},
				{IDC_FOLDER2, IDC_MAX_COUNT2, IDC_COUNT2},
				{IDC_FOLDER3, IDC_MAX_COUNT3, IDC_COUNT3},
				{IDC_FOLDER4, IDC_MAX_COUNT4, IDC_COUNT4},
				{IDC_FOLDER5, IDC_MAX_COUNT5, IDC_COUNT5},
				{IDC_FOLDER6, IDC_MAX_COUNT6, IDC_COUNT6},
				{IDC_FOLDER7, IDC_MAX_COUNT7, IDC_COUNT7},
				{IDC_FOLDER8, IDC_MAX_COUNT8, IDC_COUNT8},
				{IDC_FOLDER9, IDC_MAX_COUNT9, IDC_COUNT9},
				{IDC_FOLDER10, IDC_MAX_COUNT10, IDC_COUNT10}
	};

	class folders_countdown_conf : public cfg_var {
	public:
		folders_countdown_conf();
		virtual ~folders_countdown_conf() = default;

		void set_folder(int idx, const char* path, int count);
		void reset();
		folders_t& get_folders();

		bool listened_to_song(const char* path);

	private:
		static unsigned const Version = 1;

		bool _is_enabled;
		folders_t _folders = folders_t(CONF_IDS_SIZE);

		virtual void get_data_raw(stream_writer* p_stream, abort_callback& p_abort) override;
		virtual void set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort) override;
	};

	extern folders_countdown_conf folders_conf;

} // namespace foo_countdown