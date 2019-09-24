#include "stdafx.h"
#include "../SDK/foobar2000.h"

#include "preferences.h"

namespace foo_countdown {
	class countdown_tracker : public playback_statistics_collector {
	public:
		virtual void on_item_played(metadb_handle_ptr p_item) override {
			pfc::string8 path = p_item.get_ptr()->get_path();
			folders_t & folders = folders_conf.get_folders();

			bool found = false;
			for (int i = 0; !found && i != folders.size(); ++i) {
				if (path.find_first(folders[i].get_path()) != npos) {
					folders[i].listened_to_song(path.c_str());
				}
			}
		}
	};

	static playback_statistics_collector_factory_t <countdown_tracker> g_countdown_tracker;
}

