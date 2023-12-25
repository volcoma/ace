#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace native
{

bool open_file_dialog(std::string& out_path,
                      const std::vector<std::string>& filters,  //{"*.jpg","*.png"}
                      const std::string& filter_desc = "",      //"image files"
                      const std::string& title = "",            //""
                      const std::string& default_path = "");    //""

bool open_files_dialog(std::vector<std::string>& out_paths,
                      const std::vector<std::string>& filters,  //{"*.jpg","*.png"}
                      const std::string& filter_desc = "",      //"image files"
                      const std::string& title = "",            //""
                      const std::string& default_path = "");    //""

bool pick_folder_dialog(std::string& out_path,
                        const std::string& title = "",          //""
                        const std::string& default_path = "");  //""


bool save_file_dialog(std::string& out_path,
                      const std::vector<std::string>& filters,  //{"*.jpg","*.png"}
                      const std::string& filter_desc = "",      //"image files"
                      const std::string& title = "",            //""
                      const std::string& default_path = "");    //""

std::string color_picker(uint8_t result_rgb[3], const std::string& title = "");

enum class dialog_type
{
    ok,
    ok_cancel,
    yes_no,
    yes_no_cancel
};

enum class icon_type
{
    info,
    warning,
    error,
    question
};

enum class action_type
{
    no_or_cancel,
    ok_or_yes,
    no_in_yes_no_cancel
};

action_type message_box(const std::string& message,
                 dialog_type d_type,
                 icon_type i_type,
                 const std::string& title = "");


void notify_popup(const std::string& message,
                  icon_type i_type,
                  const std::string& title = "");

void beep();

}

