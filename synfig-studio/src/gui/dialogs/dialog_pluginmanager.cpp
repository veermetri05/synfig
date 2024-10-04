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

#include <libxml++/libxml++.h>
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

enum PluginZipStatus 
{
    PLUGIN_ZIP_NO_PLUGIN_XML = 1 << 1,
    PLUGIN_ZIP_AT_ROOT_DIR = 1 << 2,
    PLUGIN_ZIP_AT_CHILD_DIR = 1 << 3
};

Dialog_PluginManager::Dialog_PluginManager(Gtk::Window& parent):
    Gtk::Dialog(_("Plugin Manager"), parent),
    plugin_file_dialog(*this, "Select plugin zip file", Gtk::FILE_CHOOSER_ACTION_OPEN),
    message_dialog(*this, _(""), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true),
    confirmation_dialog(*this, _("Are you sure you want to delete this plugin?"), false, Gtk::MESSAGE_QUESTION, Gtk::BUTTONS_OK_CANCEL, true),
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
    confirmation_dialog.set_transient_for(*this);
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

        delete_plugin->signal_clicked().connect([plugin, this ](){
            this->confirmation_dialog.set_message(_("Do you want to delete the plugin: ") + plugin.name.get());
            int response = this->confirmation_dialog.run();
            if (response == Gtk::RESPONSE_OK) {              
                App::plugin_manager.remove_plugin(plugin.id);
                this->confirmation_dialog.hide();
                this->message_dialog.set_message("Plugin uninstalled successfully");;
                this->message_dialog.run();
                this->message_dialog.close();
            } else {
                this->confirmation_dialog.close();
            }
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

std::string extract_plugin_name(std::istream &fileStream)
{
    xmlpp::DomParser parser;
    parser.set_substitute_entities();
    parser.parse_stream(fileStream);

    const xmlpp::Node* pNode = parser.get_document()->get_root_node();
    if (std::string(pNode->get_name()) != std::string("plugin") ) {
        synfig::warning("Invalid plugin.xml file (missing root <plugin>)");
        return std::string("");
    }

    auto execlist = pNode->find("./exec");
    if (!execlist.empty())
    {
        for (const xmlpp::Node* node : pNode->find("./name") ) {
            const xmlpp::Element* element = dynamic_cast<const xmlpp::Element*>(node);
            std::string lang = element->get_attribute_value("lang");
            if (lang.empty()) {
                return element->get_child_text()->get_content();
            }
        }
    }

    return "";
}

void
Dialog_PluginManager::on_install_plugin_button_clicked()
{
    // Show the dialog and wait for a user response
    int result = plugin_file_dialog.run();
    if (result != Gtk::RESPONSE_OK) return;
    std::vector<std::string> files;
    synfig::FileSystemNative::Handle native_fs = FileSystemNative::instance();
    synfig::FileContainerZip::Handle zip_fs = new FileContainerZip();
    std::string zip_filename = plugin_file_dialog.get_filename();
    filesystem::Path path_to_user_plugins = synfigapp::Main::get_user_app_directory() / filesystem::Path("plugins");
    std::string output_path ;
    std::string plugin_metadata_file;
    std::string plugin_name ;

    if (!zip_fs->open(plugin_file_dialog.get_filename())) {
        synfig::error("Failed to open zip file: " + zip_filename);
        return;
    }
    zip_fs->directory_scan("", files);
    for (const auto& file : files) {
        if (file == "plugin.xml") {
            plugin_metadata_file = file;
            break;
        } else if(zip_fs->is_directory(file)) {
            std::vector<std::string> child_files;
            if (zip_fs->directory_scan(file, child_files)) {
            auto it = std::find(child_files.begin(), child_files.end(), "plugin.xml");
            if (it != child_files.end()) {
                plugin_metadata_file = file + "/plugin.xml" ;
            }
            if (!plugin_metadata_file.empty())
                break;
            }
        }
    }
    if (plugin_metadata_file.empty() || !zip_fs->is_file(plugin_metadata_file)) {
        synfig::error("Failed to find plugin.xml in zip file: " + zip_filename);
        message_dialog.set_message("Failed to find plugin.xml in zip file: " + zip_filename);
        message_dialog.run();
        message_dialog.close();
        plugin_file_dialog.close();
        return;
    }
    plugin_name = extract_plugin_name(*zip_fs->get_read_stream(plugin_metadata_file));
    output_path = ((path_to_user_plugins / plugin_name).add_suffix("/")).u8string();
    if (native_fs->is_exists(output_path) && native_fs->is_directory(output_path)) {
        confirmation_dialog.set_message(_("Plugin already exists. Do you want to overwrite it?"));
        int response = confirmation_dialog.run();
        if (response != Gtk::RESPONSE_OK) {
            confirmation_dialog.close();
            plugin_file_dialog.close();
            return;
        }
        auto it = std::find_if(App::plugin_manager.plugins().begin(), App::plugin_manager.plugins().end(), [plugin_name](const Plugin& plugin) { return plugin.name.fallback() == plugin_name; });
        if (it != App::plugin_manager.plugins().end()) {
            App::plugin_manager.remove_plugin(it->id);
        }
        native_fs->remove_recursive(output_path);
        confirmation_dialog.close();
    }
    if (native_fs->is_file(output_path)) {
        if (!native_fs->file_remove(output_path)) {
            synfig::error("Failed to remove file: " + output_path);
            return;
        }
    }
    if (native_fs->directory_create(output_path)) {
        if (plugin_metadata_file.find("/") == std::string::npos) {
            for(const auto& file : files) {
                zip_fs->copy_recursive(zip_fs,  file, native_fs, output_path + file);
            }
        } else {
            zip_fs->copy_recursive(zip_fs, files[0], native_fs, output_path);
        }
    }
    App::plugin_manager.load_plugin(output_path + "plugin.xml", output_path, true);
    zip_fs->close();
    Gtk::MessageDialog msg_dialog = Gtk::MessageDialog(plugin_file_dialog, _("Plugin installed successfully."), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true);
    msg_dialog.run();
    plugin_file_dialog.close();
}
