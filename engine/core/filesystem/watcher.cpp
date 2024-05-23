#include "watcher.h"
#include <sstream>
#include <utility>
#include <base/platform/thread.hpp>
namespace fs
{
using namespace std::literals;

namespace
{

void log_path(const fs::path& /*unused*/)
{
}
auto get_path_filter_pair(const fs::path& path) -> std::pair<fs::path, std::string>
{
    // extract wild card and parent path
    std::string key = path.string();
    fs::path p = path;
    size_t wildCardPos = key.find('*');
    std::string filter;
    if(wildCardPos != std::string::npos)
    {
        filter = path.filename().string();
        p = path.parent_path();
    }

    fs::error_code err;
    if(filter.empty() && !fs::exists(p, err))
    {
        log_path(path);
    }

    return std::make_pair(p, filter);
}

auto visit_wild_card_path(const fs::path& path,
                          bool recursive,
                          bool visit_empty,
                          const std::function<bool(const fs::path&)>& visitor) -> std::pair<fs::path, std::string>
{
    auto path_filter = get_path_filter_pair(path);
    if(!path_filter.second.empty())
    {
        std::string full = (path_filter.first / path_filter.second).string();
        size_t wildcard_pos = full.find('*');
        std::string before = full.substr(0, wildcard_pos);
        std::string after = full.substr(wildcard_pos + 1);
        fs::directory_iterator end;
        fs::error_code err;
        if(visit_empty && fs::is_empty(path_filter.first, err))
        {
            visitor(path_filter.first);
        }
        else if(fs::exists(path_filter.first, err))
        {
            const auto iterate = [&](auto& it)
            {
                for(const auto& entry : it)
                {
                    std::string current = entry.path().string();
                    size_t before_pos = current.find(before);
                    size_t after_pos = current.find(after);
                    if((before_pos != std::string::npos || before.empty()) &&
                       (after_pos != std::string::npos || after.empty()))
                    {
                        if(visitor(entry.path()))
                        {
                            break;
                        }
                    }
                }
            };

            if(recursive)
            {
                fs::recursive_directory_iterator it(path_filter.first, err);
                iterate(it);
            }
            else
            {
                fs::directory_iterator it(path_filter.first, err);
                iterate(it);
            }
        }
    }
    return path_filter;
}

} // namespace

class watcher::impl
{
public:

    struct observed_changes
    {
        std::vector<watcher::entry> entries;

        std::vector<size_t> created;
        std::vector<size_t> modified;

        void append(const observed_changes& rhs)
        {

            for(const auto& e : rhs.entries)
            {
                entries.emplace_back(e);
            }

            auto created_sz_before = created.size();
            for(auto idx : rhs.created)
            {
                created.emplace_back(created_sz_before + idx);
            }

            auto modified_sz_before = modified.size();
            for(auto idx : rhs.modified)
            {
                modified.emplace_back(modified_sz_before + idx);
            }
        }

        void append(observed_changes&& rhs)
        {

            for(auto& e : rhs.entries)
            {
                entries.emplace_back(std::move(e));
            }

            auto created_sz_before = created.size();
            for(auto idx : rhs.created)
            {
                created.emplace_back(created_sz_before + idx);
            }

            auto modified_sz_before = modified.size();
            for(auto idx : rhs.modified)
            {
                modified.emplace_back(modified_sz_before + idx);
            }

            rhs = {};
        }
    };
    //-----------------------------------------------------------------------------
    //  Name : impl ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    impl(const fs::path& path,
         const std::string& filter,
         bool recursive,
         bool initial_list,
         clock_t::duration poll_interval,
         notify_callback list_callback)
        : filter_(filter)
        , callback_(std::move(list_callback))
        , poll_interval_(poll_interval)
        , recursive_(recursive)
    {
        root_ = path;

        observed_changes changes;

        // make sure we store all initial write time
        if(!filter_.empty())
        {
            visit_wild_card_path(path / filter,
                                 recursive,
                                 false,
                                 [this, &changes](const fs::path& p)
                                 {
                                     poll_entry(p, changes);
                                     return false;
                                 });
        }
        else
        {
            poll_entry(root_, changes);
        }

        if(initial_list)
        {
            // this means that the first watch won't call the callback function
            // so we have to manually call it here if we want that behavior
            if(!changes.entries.empty() && callback_)
            {
                callback_(changes.entries, true);
            }
        }
    }

    void pause()
    {
        paused_ = true;
    }

    void resume()
    {
        paused_ = false;
    }

    //-----------------------------------------------------------------------------
    //  Name : watch ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void watch()
    {
        observed_changes changes;
        bool paused = paused_;


        if(!paused)
        {
            if(!buffered_changes_.entries.empty())
            {
                std::swap(changes, buffered_changes_);
            }
        }

        // otherwise we check the whole parent directory
        if(!filter_.empty())
        {
            visit_wild_card_path(root_ / filter_,
                                 recursive_,
                                 false,
                                 [this, &changes](const fs::path& p)
                                 {
                                     poll_entry(p, changes);
                                     return false;
                                 });
        }
        else
        {
            poll_entry(root_, changes);
        }

        if(paused)
        {
            if(!changes.entries.empty())
            {
                buffered_changes_.append(std::move(changes));
            }
        }
        else
        {
            process_modifications(entries_, changes);

            if(!changes.entries.empty() && callback_)
            {
                callback_(changes.entries, false);
            }
        }


    }

    static auto get_original_path(const fs::path& oldPath, const fs::path& renamedPath, const fs::path& newPath) -> fs::path
    {
        fs::path relativePath = fs::relative(newPath, renamedPath);
        fs::path originalPath = oldPath / relativePath;
        return originalPath;
    }

    static auto check_if_same_extension(const fs::path& p1, const fs::path& p2) -> bool
    {
        bool same_extensions = true;

        auto ep = p1;
        auto fp = p2;

        while(ep.has_extension() || fp.has_extension())
        {
            same_extensions &= ep.extension() == fp.extension();
            ep = ep.stem();
            fp = fp.stem();
        }

        return same_extensions;
    };

    static auto check_if_parent_dir_was_renamed(const std::vector<size_t>& renamed_dirs, const std::vector<watcher::entry>& entries, entry& e) -> bool
    {
        //check if parent_dir was renamed
        for(const auto& renamed_idx : renamed_dirs)
        {
            const auto& renamed_e = entries[renamed_idx];

            if(fs::is_any_parent_path(renamed_e.path, e.path))
            {
                e.status = watcher::entry_status::renamed;
                e.last_path = get_original_path(renamed_e.last_path, renamed_e.path, e.path);


                return true;
            }
        }
        return false;
    };


    template<typename Container>
    static auto check_if_renamed(entry& e, Container& container) -> bool
    {

        auto it = std::begin(container);
        while(it != std::end(container))
        {
            auto& fi = it->second;
            fs::error_code err;
            if(!fs::exists(fi.path, err))
            {

                if(e.size == fi.size)
                {
                    auto diff = (e.last_mod_time - fi.last_mod_time);
                    auto d = std::chrono::duration_cast<std::chrono::milliseconds>(diff);

                    if(d <= std::chrono::milliseconds(0))
                    {
                        bool same_extensions = check_if_same_extension(e.path, fi.path);
                        if(same_extensions)
                        {
                            e.status = watcher::entry_status::renamed;
                            e.last_path = fi.path;

                                   // remove the cached old path entry
                            container.erase(it);
                            return true;
                        }
                    }

                }

            }

            it++;
        }

        return false;

    };

    template<typename Container>
    static void check_for_removed(std::vector<watcher::entry>& entries, Container& container)
    {

        auto it = std::begin(container);
        while(it != std::end(container))
        {
            auto& fi = it->second;
            fs::error_code err;
            if(!fs::exists(fi.path, err))
            {
                fi.status = watcher::entry_status::removed;
                entries.push_back(fi);

                it = container.erase(it);
            }
            else
            {
                it++;
            }
        }
    }


    template<typename Container>
    static void process_modifications(Container& old_entries,
                                      observed_changes& changes)
    {
        using namespace std::literals;


        std::vector<size_t> renamed_dirs;

        for(auto idx : changes.created)
        {
            auto& e = changes.entries[idx];

            //check if parent_dir was renamed
            if(check_if_parent_dir_was_renamed(renamed_dirs, changes.entries, e))
            {

                // remove the cached old path entry
                old_entries.erase(e.last_path.string());
                continue;
            }

            // check for rename heuristic
            if(check_if_renamed(e, old_entries))
            {
                if(e.type == fs::file_type::directory)
                {
                    renamed_dirs.emplace_back(idx);
                }
                continue;
            }

        }
        check_for_removed(changes.entries, old_entries);
    }
  
    //-----------------------------------------------------------------------------
    //  Name : poll_entry ()
    /// <summary>
    ///
    ///
    ///
    /// </summary>
    //-----------------------------------------------------------------------------
    void poll_entry(const fs::path& path,
                    observed_changes& changes)
    {
        // get the last modification time
        fs::error_code err;
        auto time = fs::last_write_time(path, err);
        auto size = fs::file_size(path, err);
        fs::file_status status = fs::status(path, err);
        // add a new modification time to the map
        std::string key = path.string();
        auto it = entries_.find(key);
        if(it != entries_.end())
        {
            auto& fi = it->second;

            if(fi.last_mod_time != time || fi.size != size || fi.type != status.type())
            {
                fi.size = size;
                fi.last_mod_time = time;
                fi.status = watcher::entry_status::modified;
                fi.type = status.type();
                changes.entries.push_back(fi);
                changes.modified.push_back(changes.entries.size() - 1);
            }
            else
            {
                fi.status = watcher::entry_status::unmodified;
                fi.type = status.type();
            }
        }
        else
        {
            // or compare with an older one
            auto& fi = entries_[key];
            fi.path = path;
            fi.last_path = path;
            fi.last_mod_time = time;
            fi.status = watcher::entry_status::created;
            fi.size = size;
            fi.type = status.type();

            changes.entries.push_back(fi);
            changes.created.push_back(changes.entries.size() - 1);
        }
    }

protected:
    friend class watcher;


    /// Path to watch
    fs::path root_;
    /// Filter applied
    std::string filter_;
    /// Callback for list of modifications
    notify_callback callback_;
    /// Cache watched files
    std::map<std::string, watcher::entry> entries_;
    ///
    clock_t::duration poll_interval_ = 500ms;

    clock_t::time_point last_poll_ = clock_t::now();
    ///
    bool recursive_ = false;

    std::atomic<bool> paused_ = {false};

    observed_changes buffered_changes_;
};

static watcher& get_watcher()
{
    // create the static watcher instance
    static watcher wd;
    return wd;
}

auto watcher::watch(const fs::path& path,
                    bool recursive,
                    bool initial_list,
                    clock_t::duration poll_interval,
                    notify_callback callback) -> std::uint64_t
{
    return watch_impl(path, recursive, initial_list, poll_interval, callback);
}

void watcher::unwatch(std::uint64_t key)
{
    unwatch_impl(key);
}

void watcher::unwatch_all()
{
    unwatch_all_impl();
}

void watcher::touch(const fs::path& path, bool recursive, fs::file_time_type time)
{
    fs::error_code err;
    // if the file or directory exists change its last write time
    if(fs::exists(path, err))
    {
        fs::last_write_time(path, time, err);
        return;
    }
    // if not, visit each path if there's a wild card
    if(path.string().find('*') != std::string::npos)
    {
        visit_wild_card_path(path,
                             recursive,
                             true,
                             [time](const fs::path& p)
                             {
                                 fs::error_code err;
                                 fs::last_write_time(p, time, err);
                                 return false;
                             });
    }
    // otherwise throw an exception
    else
    {
        log_path(path);
    }
}

watcher::~watcher()
{
    close();
}

void watcher::pause()
{
    auto& wd = get_watcher();

    {
        std::lock_guard<std::mutex> lock(wd.mutex_);
        for(auto& kvp : wd.watchers_)
        {
            auto& w = kvp.second;
            w->pause();
        }
    }


}

void watcher::resume()
{
    auto& wd = get_watcher();

    {
        std::lock_guard<std::mutex> lock(wd.mutex_);
        for(auto& kvp : wd.watchers_)
        {
            auto& w = kvp.second;
            w->resume();
        }
    }
}


void watcher::close()
{
    // stop the thread
    watching_ = false;
    // remove all watchers
    unwatch_all();

    if(thread_.joinable())
    {
        thread_.join();
    }
}

void watcher::start()
{
    watching_ = true;
    thread_ = std::thread(
        [this]()
        {
            platform::set_thread_name("fs::watcher");
            // keep watching for modifications every ms milliseconds
            using namespace std::literals;
            while(watching_)
            {
                clock_t::duration sleep_time = 99999h;

                // iterate through each watcher and check for modification
                std::map<std::uint64_t, std::shared_ptr<impl>> watchers;
                {
                    std::unique_lock<std::mutex> lock(mutex_);
                    watchers = watchers_;
                }

                for(auto& pair : watchers)
                {
                    auto watcher = pair.second;

                    auto now = clock_t::now();

                    auto diff = (watcher->last_poll_ + watcher->poll_interval_) - now;
                    if(diff <= clock_t::duration(0))
                    {
                        watcher->watch();
                        watcher->last_poll_ = now;

                        sleep_time = std::min(sleep_time, watcher->poll_interval_);
                    }
                    else
                    {
                        sleep_time = std::min(sleep_time, diff);
                    }
                }

                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait_for(lock, sleep_time);
            }
        });
}

auto watcher::watch_impl(const fs::path& path,
                         bool recursive,
                         bool initial_list,
                         clock_t::duration poll_interval,
                         notify_callback& list_callback) -> std::uint64_t
{
    auto& wd = get_watcher();
    // and start its thread
    if(!wd.watching_)
    {
        wd.start();
    }

    // add a new watcher
    if(list_callback)
    {
        std::string filter;
        fs::path p = path;
        // try to see if there's a match for the wild card
        if(path.string().find('*') != std::string::npos)
        {
            auto path_filter = visit_wild_card_path(path,
                                                    recursive,
                                                    true,
                                                    [](const fs::path&)
                                                    {
                                                        return true;
                                                    });

            p = path_filter.first;
            filter = path_filter.second;
        }
        else
        {
            fs::error_code err;
            if(!fs::exists(path, err))
            {
                log_path(path);
                return 0;
            }
        }

        static std::atomic<std::uint64_t> free_id = {1};

        auto key = free_id++;
        {
            // we do it like this because if initial_list is true we don't want
            // to call a user callback on a locked mutex
            auto imp =
                std::make_shared<impl>(p, filter, recursive, initial_list, poll_interval, std::move(list_callback));
            std::lock_guard<std::mutex> lock(wd.mutex_);
            wd.watchers_.emplace(key, std::move(imp));
        }
        wd.cv_.notify_all();
        return key;
    }

    return 0;
}

void watcher::unwatch_impl(std::uint64_t key)
{
    auto& wd = get_watcher();

    {
        std::lock_guard<std::mutex> lock(wd.mutex_);
        wd.watchers_.erase(key);
    }
    wd.cv_.notify_all();
}

void watcher::unwatch_all_impl()
{
    auto& wd = get_watcher();
    {
        std::lock_guard<std::mutex> lock(wd.mutex_);
        wd.watchers_.clear();
    }
    wd.cv_.notify_all();
}

auto to_string(const watcher::entry& e) -> std::string
{
    static auto file_type_to_string = [](file_type type) -> std::string
    {
        switch(type)
        {
            case file_type::regular:
                return "file";
            case file_type::directory:
                return "dir";
            default:
                return "other";
        }
    };

    static auto status_to_string = [](watcher::entry_status status) -> std::string
    {
        switch(status)
        {
            case watcher::entry_status::created:
                return "created";
            case watcher::entry_status::modified:
                return "modified";
            case watcher::entry_status::removed:
                return "removed";
            case watcher::entry_status::renamed:
                return "renamed";
            default:
                return "unmodified";
        }
    };

    std::stringstream ss;
    ss << "{\"" << e.last_mod_time.time_since_epoch().count() << "\":[" << e.path << "," << file_type_to_string(e.type)
       << "," << status_to_string(e.status) << "]}";
    return ss.str();
}
} // namespace fs
