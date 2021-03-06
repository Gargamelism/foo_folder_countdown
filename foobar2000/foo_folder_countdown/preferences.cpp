#include "stdafx.h"

#include "preferences.h"
#include <helpers/atl-misc.h>
#include <windows.h>
#include <cstring>

namespace foo_countdown {

	LPWSTR mb_to_wide(LPSTR mb_str)
	{
		int w_length = MultiByteToWideChar(CP_UTF8, 0, mb_str, strlen(mb_str), NULL, 0);
		w_length += 10;
		LPWSTR w_str = new wchar_t[w_length];
		MultiByteToWideChar(CP_UTF8, 0, mb_str, -1, w_str, w_length);

		return w_str;
	}

	LPSTR wide_to_mb(LPWSTR w_str)
	{
		int mb_length = WideCharToMultiByte(CP_UTF8, 0, w_str, lstrlen(w_str) + 1, NULL, 0, NULL, NULL);
		mb_length += 10;
		LPSTR mb_str = new char[mb_length];
		WideCharToMultiByte(CP_UTF8, 0, w_str, lstrlen(w_str) + 1, mb_str, mb_length, NULL, NULL);

		return mb_str;
	}

	// {85027768-2FE9-4AC5-96C2-AEF9D36B3D0F}
	static const GUID folder_countdown_conf_id =
	{ 0x85027768, 0x2fe9, 0x4ac5, { 0x96, 0xc2, 0xae, 0xf9, 0xd3, 0x6b, 0x3d, 0xf } };


	void folder_countdown_t::add_folder(const char* path, unsigned int count) {
		_path = path;
		_global_count = count;
		_play_count = 0;

		initialize_files();

		if (_files_count.size() == 0) {
			_global_count = 0;
			_path = DEFAULT_PATH;
		}
	}

	void folder_countdown_t::update_play_count(const char* file_name, unsigned int count) {
		bool found = false;
		
		for (int i = 0; !found && i < _files_count.size(); i++) {
			found = !std::strcmp(_files_count[i].path(), file_name);
			if (found) {
				_files_count[i].set_count(count);
			}
		}

		_play_count = (_global_count > count) ? count : _global_count;
	}

	void folder_countdown_t::reset() {
		_files_count.clear();
		_path = DEFAULT_PATH;
		_global_count = 0;
	}

	pfc::string8 folder_countdown_t::get_path() {
		return _path;
	}

	unsigned int folder_countdown_t::get_max_plays() {
		return _global_count;
	}

	unsigned int folder_countdown_t::get_count() {
		return _play_count;
	}

	void folder_countdown_t::initialize_files() {
		pfc::string8 path = _path;
		path += "\\*";

		WIN32_FIND_DATA find_data;
		auto wPath = mb_to_wide((LPSTR)path.c_str());
		HANDLE first_file = FindFirstFile(wPath, &find_data);

		if (INVALID_HANDLE_VALUE != first_file) {
			do {
				auto wExtension = PathFindExtension(find_data.cFileName);

				auto mbExtension = wide_to_mb(wExtension);

				if (is_in_allowed_extensions(mbExtension)) {
					_files_count.push_back(file_count_t(wide_to_mb(find_data.cFileName), _play_count));
				}

				delete[] mbExtension;
			} while (FindNextFile(first_file, &find_data) != FALSE);
		}

		delete[] wPath;
	}

	void folder_countdown_t::reset_play_count() {
		for (int i = 0; i < _files_count.size(); i++) {
			_files_count[i].set_count(0);
		}
	}

	bool folder_countdown_t::listened_to_song(const char* path) {
		bool updated = false;
		pfc::string8 path_(path);

		unsigned int lowest_count = _global_count;

		for (int i = 0; i < _files_count.size(); i++) {
			if (!updated && path_.find_first(_files_count[i].path()) != npos) {
				_files_count[i].inc_count();
				updated = true;
			}

			lowest_count = (lowest_count > _files_count[i].count()) ? _files_count[i].count() : lowest_count;
		}

		_play_count = lowest_count;

		if (_global_count == _play_count) {
			pfc::string8 msg("Folder count of folder <");
			msg.add_string(_path);
			msg.add_string("> reached it's peak.");

			popup_message::g_show(msg.c_str(), "Folder Countdown");
			reset_play_count();
		}

		return updated;
	}

	files_count_t folder_countdown_t::get_files_count() {
		return _files_count;
	}

	bool folder_countdown_t::is_in_allowed_extensions(const char* ext) {
		bool is_allowed = false;
		pfc::string8 ext_;
		ext_.convert_to_lower_ascii(ext);

		for (int i = 0; !is_allowed && i < folder_countdown_t::_allowed_extensions_size; i++) {
			is_allowed = _allowed_extensions[i].equals(ext_);
		}

		return is_allowed;
	}

	// folder_countdown ^

	//folders_countdown_conf V

	folders_countdown_conf::folders_countdown_conf() :
		cfg_var(folder_countdown_conf_id),
		_is_enabled(true) {
		_folders = folders_t(CONF_IDS_SIZE);
	}

	void folders_countdown_conf::get_data_raw(stream_writer* p_stream, abort_callback& p_abort) {
		p_stream->write_lendian_t(Version, p_abort);

		p_stream->write_lendian_t(_is_enabled, p_abort);

		for (int i = 0; i < _folders.size(); i++) {
			p_stream->write_string(_folders[i].get_path().c_str(), p_abort);
			p_stream->write_lendian_t(_folders[i].get_max_plays(), p_abort);

			auto files_count = _folders[i].get_files_count();
			for (int j = 0; j < files_count.size(); j++) {
				p_stream->write_string(files_count[j].path(), p_abort);
				p_stream->write_lendian_t(files_count[j].count(), p_abort);
			}
		}
	}

	void folders_countdown_conf::set_data_raw(stream_reader* p_stream, t_size p_sizehint, abort_callback& p_abort) {
		unsigned int ver;
		p_stream->read_lendian_t(ver, p_abort);
		// this allows for version based actions

		p_stream->read_lendian_t(_is_enabled, p_abort);

		pfc::string8 path;
		unsigned int max_plays;
		
		pfc::string8 file_name;
		unsigned int file_count;

		for (int i = 0; i < _folders.size(); i++) {
			p_stream->read_string(path, p_abort);
			p_stream->read_lendian_t(max_plays, p_abort);

			_folders[i].add_folder(path.c_str(), max_plays);
			auto files_count = _folders[i].get_files_count();
			for (int j = 0; j < files_count.size(); j++) {
				p_stream->read_string(file_name, p_abort);
				p_stream->read_lendian_t(file_count, p_abort);

				_folders[i].update_play_count(file_name.c_str(), file_count);
			}
		}
	}

	void folders_countdown_conf::set_folder(int idx, const char* path, int count) {
		_folders[idx].add_folder(path, count);
	}

	void folders_countdown_conf::reset() {
		for (int i = 0; i < _folders.size(); i++) {
			_folders[i].reset();
		}
	}

	folders_t& folders_countdown_conf::get_folders() {
		return _folders;
	}

	bool folders_countdown_conf::listened_to_song(const char* path) {
		const int file_prefix_idx = strlen("file//") + 1;

		pfc::string played_file = path;
		size_t file_name_len = played_file.lastIndexOf('\\') - file_prefix_idx;

		pfc::string8 played_path(played_file.subString(file_prefix_idx, file_name_len).c_str());

		for (int i = 0; i < _folders.size(); ++i) {
			if (played_path.find_first(_folders[i].get_path()) != npos) {
				_folders[i].listened_to_song(played_file.subString(file_name_len + 1).c_str());
			}
		}

		return false;
	}

	class CMyPreferences : public CDialogImpl<CMyPreferences>, public preferences_page_instance {
	public:
		//Constructor - invoked by preferences_page_impl helpers - don't do Create() in here, preferences_page_impl does this for us
		CMyPreferences(preferences_page_callback::ptr callback) : m_callback(callback) {}

		//Note that we don't bother doing anything regarding destruction of our class.
		//The host ensures that our dialog is destroyed first, then the last reference to our preferences_page_instance object is released, causing our object to be deleted.


		//dialog resource ID
		enum { IDD = IDD_MYPREFERENCES };
		// preferences_page_instance methods (not all of them - get_wnd() is supplied by preferences_page_impl helpers)
		t_uint32 get_state();
		void apply();
		void reset();

		//WTL message map
		BEGIN_MSG_MAP_EX(CMyPreferences)
			MSG_WM_INITDIALOG(OnInitDialog)
			COMMAND_HANDLER_EX(IDC_FOLDER1, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER2, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER3, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER4, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER5, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER6, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER7, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER8, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER9, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_FOLDER10, EN_CHANGE, OnEditChange)

			COMMAND_HANDLER_EX(IDC_COUNT1, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT2, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT3, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT4, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT5, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT6, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT7, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT8, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT9, EN_CHANGE, OnEditChange)
			COMMAND_HANDLER_EX(IDC_COUNT10, EN_CHANGE, OnEditChange)

			END_MSG_MAP()

	private:
		BOOL OnInitDialog(CWindow, LPARAM);
		void OnEditChange(UINT, int, CWindow);
		bool HasChanged();
		void OnChanged();

		void UpdateDialog();

		const preferences_page_callback::ptr m_callback;
	};

	void CMyPreferences::UpdateDialog() {
		auto folders = folders_conf.get_folders();

		for (int i = 0; i < CONF_IDS_SIZE; i++) {
			uSetDlgItemText(m_hWnd, conf_ids[i].path, folders[i].get_path().c_str());
			SetDlgItemInt(conf_ids[i].max_count, folders[i].get_max_plays());
			SetDlgItemInt(conf_ids[i].current_count, folders[i].get_count());
		}
	}

	BOOL CMyPreferences::OnInitDialog(CWindow, LPARAM) {

		UpdateDialog();

		return FALSE;
	}

	void CMyPreferences::OnEditChange(UINT, int, CWindow) {
		// not much to do here
		OnChanged();
	}

	t_uint32 CMyPreferences::get_state() {
		t_uint32 state = preferences_state::resettable;
		if (HasChanged()) state |= preferences_state::changed;
		return state;
	}

	void CMyPreferences::reset() {
		folders_conf.reset();

		UpdateDialog();

		OnChanged();
	}

	void CMyPreferences::apply() {
		folders_t& folders = folders_conf.get_folders();

		pfc::string8 folder_text;
		unsigned int folder_count;

		for (int i = 0; i < CONF_IDS_SIZE; i++) {
			folder_text = uGetDlgItemText(m_hWnd, conf_ids[i].path).c_str();
			folder_count = GetDlgItemInt(conf_ids[i].max_count, NULL, false);
			if (folders[i].get_path() != folder_text ||
				folders[i].get_max_plays() != folder_count) {

				folders[i].add_folder(folder_text.c_str(), folder_count);
			}
		}

		OnChanged(); //our dialog content has not changed but the flags have - our currently shown values now match the settings so the apply button can be disabled
	}

	bool CMyPreferences::HasChanged() {
		//returns whether our dialog content is different from the current configuration (whether the apply button should be enabled or not)
		auto folders = folders_conf.get_folders();

		bool has_changed = false;

		for (int i = 0; !has_changed && i < folders.size(); i++) {
			has_changed = has_changed ||
				folders[i].get_path() != uGetDlgItemText(m_hWnd, conf_ids[i].path).c_str() ||
				folders[i].get_max_plays() != GetDlgItemInt(conf_ids[i].max_count);
		}

		return has_changed;
	}
	void CMyPreferences::OnChanged() {
		//tell the host that our state has changed to enable/disable the apply button appropriately.
		m_callback->on_state_changed();
	}

	class preferences_page_myimpl : public preferences_page_impl<CMyPreferences> {
		// preferences_page_impl<> helper deals with instantiation of our dialog; inherits from preferences_page_v3.
	public:
		const char* get_name() { return "Folder Play Countdown"; }
		GUID get_guid() {
			// {69F7645E-D586-4B63-B2E7-B68683D3F4BC}
			static const GUID guid =
			{ 0x69f7645e, 0xd586, 0x4b63, { 0xb2, 0xe7, 0xb6, 0x86, 0x83, 0xd3, 0xf4, 0xbc } };


			return guid;
		}
		GUID get_parent_guid() { return guid_tools; }

	};

	folders_countdown_conf folders_conf;

	static preferences_page_factory_t<preferences_page_myimpl> g_preferences_page_myimpl_factory;

} // namespace foo_countdown
