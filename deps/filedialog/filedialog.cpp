#include "filedialog.h"
#include "tfd/tinyfiledialogs.h"

#include <sstream>

namespace native
{


bool open_file_dialog(std::string& out_path,
                      const std::vector<std::string>& filters,
                      const std::string& filter_desc,
                      const std::string& title,
                      const std::string& default_path)
{
    static std::string cached_default_path{};

    auto def_path = !default_path.empty() ? default_path.c_str() : cached_default_path.c_str();

    std::vector<const char*> filters_cptr;
    filters_cptr.resize(filters.size());
    for(size_t i = 0; i < filters.size(); ++i)
    {
        filters_cptr[i] = filters[i].c_str();
    }
    auto result = tinyfd_openFileDialog(title.c_str(),
                                        def_path,
                                        int(filters_cptr.size()),
                                        filters_cptr.data(),
                                        filter_desc.c_str(),
                                        0);
    if(!result)
    {
        return false;
    }

    out_path = result;
    cached_default_path = out_path;

    return true;
}


bool open_files_dialog(std::vector<std::string>& out_path,
                      const std::vector<std::string>& filters,
                      const std::string& filter_desc,
                      const std::string& title,
                      const std::string& default_path)
{
    static std::string cached_default_path{};

    auto def_path = !default_path.empty() ? default_path.c_str() : cached_default_path.c_str();

    std::vector<const char*> filters_cptr;
    filters_cptr.resize(filters.size());
    for(size_t i = 0; i < filters.size(); ++i)
    {
        filters_cptr[i] = filters[i].c_str();
    }
    auto result = tinyfd_openFileDialog(title.c_str(),
                                        def_path,
                                        int(filters_cptr.size()),
                                        filters_cptr.data(),
                                        filter_desc.c_str(),
                                        1);
    if(!result)
    {
        return false;
    }

    std::istringstream f(result);
    std::string s;
    while (std::getline(f, s, '|'))
    {
        out_path.push_back(s);
        cached_default_path = s;
    }

    return true;
}


bool pick_folder_dialog(std::string& out_path,
                        const std::string& title,
                        const std::string& default_path)
{
    static std::string cached_default_path{};

    auto def_path = !default_path.empty() ? default_path.c_str() : cached_default_path.c_str();

    auto result = tinyfd_selectFolderDialog(title.c_str(), def_path);
	if(!result)
	{
		return false;
	}


    out_path = result;
    cached_default_path = out_path;
	return true;
}

bool save_file_dialog(std::string& out_path,
                      const std::vector<std::string>& filters,
                      const std::string& filter_desc,
                      const std::string& title,
                      const std::string& default_path)
{
    static std::string cached_default_path{};

    auto def_path = !default_path.empty() ? default_path.c_str() : cached_default_path.c_str();

    std::vector<const char*> filters_cptr;
    filters_cptr.resize(filters.size());
    for(size_t i = 0; i < filters.size(); ++i)
    {
        filters_cptr[i] = filters[i].c_str();
    }
    auto result = tinyfd_saveFileDialog(title.c_str(),
                                        def_path,
                                        int(filters_cptr.size()),
                                        filters_cptr.data(),
                                        filter_desc.c_str());
    if(!result)
    {
        return false;
    }

    out_path = result;
    cached_default_path = out_path;

    return true;
}

action_type message_box(const std::string& message,
                        dialog_type d_type,
                        icon_type i_type,
                        const std::string& title)
{
    auto get_dialog_str = [](dialog_type d_type)
    {
        switch(d_type)
        {
        case dialog_type::ok:
            return "ok";
        case dialog_type::ok_cancel:
            return "okcancel";
        case dialog_type::yes_no:
            return "yesno";
        case dialog_type::yes_no_cancel:
            return "yesnocancel";
        }

        return "ok";
    };

    auto get_icon_str = [](icon_type i_type)
    {
        switch(i_type)
        {
        case icon_type::info:
            return "info";
        case icon_type::warning:
            return "warning";
        case icon_type::error:
            return "error";
        case icon_type::question:
            return "question";
        }

        return "info";
    };

    auto get_fallback_title_str = [](icon_type i_type)
    {
        switch(i_type)
        {
        case icon_type::info:
            return "Info.";
        case icon_type::warning:
            return "Warning!";
        case icon_type::error:
            return "Error!";
        case icon_type::question:
            return "Question?";
        }

        return "Info.";
    };

    auto get_action_type = [](int result)
    {
        switch(result)
        {
        case 0:
            return action_type::no_or_cancel;
        case 1:
            return action_type::ok_or_yes;
        case 2:
            return action_type::no_in_yes_no_cancel;
        }

        return action_type::no_or_cancel;
    };


    auto result = tinyfd_messageBox(!title.empty() ? title.c_str() : get_fallback_title_str(i_type),
                      message.c_str(),
                      get_dialog_str(d_type),
                      get_icon_str(i_type), 0);


    return get_action_type(result);
}

std::string color_picker(uint8_t result_rgb[], const std::string& title)
{
    uint8_t default_rgb[] = {255, 255, 255};
    auto result = tinyfd_colorChooser(title.c_str(), nullptr, default_rgb, result_rgb);
    if(!result)
    {
        return {};
    }

    return result;
}

void notify_popup(const std::string &message, icon_type i_type, const std::string &title)
{
    auto get_icon_str = [](icon_type i_type)
    {
        switch(i_type)
        {
            case icon_type::info:
                return "info";
            case icon_type::warning:
                return "warning";
            case icon_type::error:
                return "error";
            case icon_type::question:
                return "question";
        }

        return "info";
    };

    tinyfd_notifyPopup(title.c_str(), message.c_str(), get_icon_str(i_type));
}

void beep()
{
    tinyfd_beep();
}


}
