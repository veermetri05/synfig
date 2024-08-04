/* === S Y N F I G ========================================================= */
/*!	\file dialog_pluginmanager.cpp
**	\brief Plugin Manager Dialog
**
**	\legal
**	Copyright (c) 2002-2005 Robert B. Quattlebaum Jr., Adrian Bentley
**
**	This file is part of Synfig.
**
**	Synfig is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 2 of the License, or
**	(at your option) any later version.
**
**	Synfig is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with Synfig.  If not, see <https://www.gnu.org/licenses/>.
**	\endlegal
*/
/* ========================================================================= */

/* === H E A D E R S ======================================================= */

#ifdef USING_PCH
#	include "pch.h"
#else
#ifdef HAVE_CONFIG_H
#	include <config.h>
#endif

#include "dialog_pluginmanager.h"

#include <fstream>

#include <gtkmm/label.h>
#include <gtkmm/notebook.h>
#include <gtkmm/listbox.h>
#include <gtkmm/listboxrow.h>

#include <gui/localization.h>
#include <gui/app.h>

#include <synfig/general.h>
#include <synfigapp/main.h>
#include <synfig/os.h>

#endif

/* === U S I N G =========================================================== */

using namespace synfig;
using namespace studio;
using namespace synfigapp;

/* === M A C R O S ========================================================= */

/* === G L O B A L S ======================================================= */

/* === P R O C E D U R E S ================================================= */

/* === M E T H O D S ======================================================= */

Dialog_PluginManager::Dialog_PluginManager(Gtk::Window& parent):
    Gtk::Dialog(_("Plugin Manager"), parent),
    plugin_file_dialog("Select plugin zip file", Gtk::FILE_CHOOSER_ACTION_OPEN),
    message_dialog(_("Are you sure you want to delete this plugin?"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true),
    plugin_list(App::plugin_manager.plugins())
{
    plugin_file_dialog.add_button(_("Cancel"), Gtk::RESPONSE_CANCEL);
    plugin_file_dialog.add_button(_("Open"), Gtk::RESPONSE_OK);

    // Add a filter to show only text files
    auto filter_zip = Gtk::FileFilter::create();
    filter_zip->set_name("Zip File (A synfig plugin zip file)");
    filter_zip->add_mime_type("application/zip");
    plugin_file_dialog.add_filter(filter_zip);

    App::plugin_manager.signal_list_changed().connect(sigc::mem_fun(*this, &Dialog_PluginManager::refresh));
    // this->set_resizable(false);
    message_dialog.set_transient_for(*this);
    Gtk::Button* install_plugin_button = manage(new Gtk::Button(_("Install Plugin")));
    install_plugin_button->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
    install_plugin_button->set_always_show_image(true);
    install_plugin_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_PluginManager::on_install_plugin_button_clicked));

    Gtk::VBox* plugin_tab = manage(new Gtk::VBox());
    plugin_tab->property_expand().set_value(false);
    plugin_list_box.set_selection_mode(Gtk::SELECTION_NONE);

    build_listbox();

    plugin_tab->pack_start(plugin_list_box, Gtk::PACK_SHRINK);
    plugin_tab->pack_end(*install_plugin_button, Gtk::PACK_SHRINK);

    Gtk::Label* custom_label = manage(new Gtk::Label());
    custom_label->set_text(_("Installed Plugins"));
    custom_label->set_margin_bottom(10);
    custom_label->set_margin_top(10);
    custom_label->set_margin_left(20);
    custom_label->set_margin_right(20);
    notebook.append_page(*plugin_tab);
    notebook.set_tab_label(*plugin_tab, *custom_label);
    
    notebook.set_tab_pos(Gtk::POS_LEFT);
    get_content_area()->pack_start(notebook);
    set_default_size(1200, 600);
    show_all_children();
}

Dialog_PluginManager::~Dialog_PluginManager()
{
}

void
Dialog_PluginManager::refresh()
{
    plugin_list = App::plugin_manager.plugins();
    auto children = plugin_list_box.get_children();
    for (auto child : children) {
        plugin_list_box.remove(*child);
        delete child;
    }

    build_listbox();
}

void
Dialog_PluginManager::build_listbox()
{
    for (const auto& plugin : plugin_list) {
        Gtk::HBox* plugin_list_item = manage(new Gtk::HBox());
        Gtk::Label* plugin_name = manage(new Gtk::Label());

        Gtk::HBox* plugin_option_box = manage(new Gtk::HBox());
        Gtk::Button* restore_settings = manage(new Gtk::Button());
        Gtk::Button* open_folder = manage(new Gtk::Button());
        Gtk::Button* delete_plugin = manage(new Gtk::Button());

        plugin_list_item->property_expand().set_value(false);
        plugin_list_item->set_margin_top(10);
        plugin_option_box->property_expand().set_value(false);

        restore_settings->set_image_from_icon_name("view-refresh", Gtk::ICON_SIZE_BUTTON);
        open_folder->set_image_from_icon_name("document-open", Gtk::ICON_SIZE_BUTTON);
        delete_plugin->set_image_from_icon_name("user-trash-symbolic", Gtk::ICON_SIZE_BUTTON);

        delete_plugin->signal_clicked().connect([plugin, this](){
            this->message_dialog.set_message(_("Do you want to delete the plugin: ") + plugin.name.get());
            int response = this->message_dialog.run();
            if (response == Gtk::RESPONSE_OK) {              
                App::plugin_manager.remove_plugin(plugin.id);
            }
            this->message_dialog.close();
        });

        open_folder->signal_clicked().connect([plugin](){
            plugin.launch_dir();
        });

        plugin_option_box->pack_start(*restore_settings, Gtk::PACK_SHRINK, 10);
        plugin_option_box->pack_start(*open_folder, Gtk::PACK_SHRINK, 10);
        plugin_option_box->pack_start(*delete_plugin, Gtk::PACK_SHRINK, 10);
        plugin_option_box->set_margin_top(10);
        plugin_option_box->set_margin_bottom(10);

        plugin_name->set_text(plugin.name.get());

        plugin_list_item->pack_start(*plugin_name, Gtk::PACK_SHRINK, 20);
        plugin_list_item->pack_end(*plugin_option_box, Gtk::PACK_SHRINK, 20);
        
        plugin_list_box.add(*plugin_list_item);

    }
    plugin_list_box.show_all_children();
}

void
Dialog_PluginManager::on_install_plugin_button_clicked()
{
    // Show the dialog and wait for a user response
    int result = plugin_file_dialog.run();
    switch (result) {
        case (Gtk::RESPONSE_OK):
        { 
            std::vector<std::string> files;
            etl::handle<synfig::FileSystemNative> native_fs = FileSystemNative::instance();
            etl::handle<synfig::FileContainerZip> zip_fs = new FileContainerZip();
            std::string zip_filename = plugin_file_dialog.get_filename();
            filesystem::Path path_to_user_plugins = synfigapp::Main::get_user_app_directory() / filesystem::Path("plugins");
            if (!zip_fs->open(plugin_file_dialog.get_filename())) {
                synfig::error("Failed to open zip file: " + zip_filename);
                return;
            }
            zip_fs->directory_scan("", files );
            // check if the archive has a plugin.xml file at root directory
            enum PluginZipStatus
            {
                PLUGIN_ZIP_NO_PLUGIN_XML = 1 << 1,
                PLUGIN_ZIP_AT_ROOT_DIR = 1 << 2,
                PLUGIN_ZIP_AT_CHILD_DIR = 1 << 3
            } pluginStatus = PluginZipStatus::PLUGIN_ZIP_NO_PLUGIN_XML;
            
            for (const auto& file : files) {
                if (file == "plugin.xml") {
                    pluginStatus = PluginZipStatus::PLUGIN_ZIP_AT_ROOT_DIR;
                    break;
                }
            }
            std::string output_path ;
            if (pluginStatus == PluginZipStatus::PLUGIN_ZIP_AT_ROOT_DIR) {
                output_path = ((path_to_user_plugins / filesystem::Path(zip_filename).stem()).add_suffix("/")).u8string();
                if (native_fs->directory_create(output_path))
                for (const auto &file : files)
                    zip_fs->copy_recursive(zip_fs, file, native_fs, output_path + file);
                App::plugin_manager.load_plugin(output_path + "plugin.xml", output_path, true);
            }
            else {
                for (auto& file : files)
                    if (zip_fs->is_directory(file)) {
                        std::vector<std::string> child_files;
                        zip_fs->directory_scan(file, child_files);
                        for (auto& file : child_files)
                            if (file == "plugin.xml") {
                                pluginStatus = PluginZipStatus::PLUGIN_ZIP_AT_CHILD_DIR;
                                break;
                            }
                    }
                if (pluginStatus == PluginZipStatus::PLUGIN_ZIP_AT_CHILD_DIR) {
                    output_path = path_to_user_plugins.add_suffix("/").u8string();
                    for (auto &file : files)
                        zip_fs->copy_recursive(zip_fs, file, native_fs, output_path + file);
                }
                App::plugin_manager.load_plugin(output_path + files[0] + '/' + "plugin.xml", output_path +  files[0] , true);
            }
            zip_fs->close();
            if (pluginStatus == PluginZipStatus::PLUGIN_ZIP_NO_PLUGIN_XML) {
                synfig::error("Failed to find plugin.xml in zip file: " + zip_filename);
            }
            plugin_file_dialog.close();
            break;
        }
        default:
        {
            std::cout << "Unexpected response" << std::endl;
            break;
        }
    }
}