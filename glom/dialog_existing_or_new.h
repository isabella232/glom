/* Glom
 *
 * Copyright (C) 2001-2004 Murray Cumming
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_DIALOG_EXISTING_OR_NEW_H
#define GLOM_DIALOG_EXISTING_OR_NEW_H

#include "config.h"
#include <memory>
#include <ctime> // for time_t
#include <giomm/asyncresult.h>
#include <giomm/file.h>
#include <giomm/fileenumerator.h>
#include <giomm/inputstream.h>
#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/recentinfo.h>
#include <gtkmm/notebook.h>
#include <gtkmm/builder.h>

#ifndef G_OS_WIN32
# include <libepc/service-monitor.h>
#endif

namespace Glom
{

class Dialog_ExistingOrNew
  : public Gtk::Dialog
{
public:
  static const char glade_id[];
  static const bool glade_developer;

  enum Action {
    NONE,
    NEW_EMPTY,
    NEW_FROM_TEMPLATE,
    OPEN_URI,
    OPEN_REMOTE
  };

  Dialog_ExistingOrNew(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_ExistingOrNew();

  Action get_action() const;
  Glib::ustring get_uri() const; // Only when get_action is NEW_FROM_TEMPLATE or OPEN_URI
#ifndef G_OS_WIN32
  EpcServiceInfo* get_service_info() const; // Only when get_action is OPEN_REMOTE
  Glib::ustring get_service_name() const; // Only when get_action is OPEN_REMOTE
#endif

private:
  Action get_action_impl(Gtk::TreeModel::iterator& iter) const;

  std::shared_ptr<Gtk::TreeModel::iterator> create_dummy_item_existing(const Gtk::TreeModel::iterator& parent, const Glib::ustring& text);


  void existing_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  void existing_title_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);

  void on_switch_page(Gtk::Widget* page, guint page_num);
  void on_existing_selection_changed();


  bool on_existing_select_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool path_currently_selected);

  void update_ui_sensitivity();

#ifndef GLOM_ENABLE_CLIENT_ONLY
  std::shared_ptr<Gtk::TreeModel::iterator> create_dummy_item_new(const Gtk::TreeModel::iterator& parent, const Glib::ustring& text);
  void on_new_selection_changed();
  void new_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  void new_title_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  bool on_new_select_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool path_currently_selected);
  void on_new_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
  void on_new_button_clicked(const Gtk::TreeModel::Path& path);

  
  bool list_examples();
  Glib::ustring get_title_from_example(const std::string& resource_name);
  void append_example(const Glib::ustring& title, const std::string& resource_name);
#endif /* !GLOM_ENABLE_CLIENT_ONLY */
    
#ifndef G_OS_WIN32
  static void on_service_found_static(EpcServiceMonitor* monitor, gchar* name, EpcServiceInfo* info, gpointer user_data);
  static void on_service_removed_static(EpcServiceMonitor* monitor, gchar* name, gchar* type, gpointer user_data);

  void on_service_found(const Glib::ustring& name, EpcServiceInfo* info);
  void on_service_removed(const Glib::ustring& name, const Glib::ustring& type);
#endif

  void on_existing_row_activated(const Gtk::TreeModel::Path& path, Gtk::TreeViewColumn* column);
  void on_existing_button_clicked(const Gtk::TreeModel::Path& path);
  void on_select_clicked();
  
  
  Gtk::Notebook* m_notebook;
  Gtk::Button* m_select_button;

  class ExistingModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ExistingModelColumns()
    {
      add(m_col_title);
      add(m_col_time);

#ifndef G_OS_WIN32
      add(m_col_service_name);
      add(m_col_service_info);
#endif

      add(m_col_recent_info);
    }

    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn<std::time_t> m_col_time; // Sort criteria

#ifndef G_OS_WIN32
    // For service discovery:
    Gtk::TreeModelColumn<Glib::ustring> m_col_service_name;
    Gtk::TreeModelColumn<EpcServiceInfo*> m_col_service_info;
#endif

    // For recently used resources:
    Gtk::TreeModelColumn< Glib::RefPtr<Gtk::RecentInfo> > m_col_recent_info;
  };
  

  ExistingModelColumns m_existing_columns;
  Glib::RefPtr<Gtk::TreeStore> m_existing_model;
  Gtk::TreeView* m_existing_view;

#ifndef GLOM_ENABLE_CLIENT_ONLY
  class NewModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    NewModelColumns()
    { add(m_col_title); add(m_col_template_uri); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn<Glib::ustring> m_col_template_uri;
  };
  
  NewModelColumns m_new_columns;
  Gtk::TreeView* m_new_view;
  Glib::RefPtr<Gtk::TreeStore> m_new_model;
  
  Gtk::TreeViewColumn m_new_column_title;
  Gtk::TreeViewColumn m_new_column_button;
  Gtk::CellRendererPixbuf m_new_icon_renderer;
  Gtk::CellRendererText m_new_title_renderer;
  Gtk::TreeModel::iterator m_iter_new_empty;
  Gtk::TreeModel::iterator m_iter_new_template;
  std::shared_ptr<Gtk::TreeModel::iterator> m_iter_new_template_dummy;
#endif //GLOM_ENABLE_CLIENT_ONLY

  Gtk::TreeViewColumn m_existing_column_title;
  Gtk::TreeViewColumn m_existing_column_button;
  Gtk::CellRendererPixbuf m_existing_icon_renderer;
  Gtk::CellRendererText m_existing_title_renderer;

  //Iterators to the parent nodes:
  Gtk::TreeModel::iterator m_iter_existing_recent;
#ifndef G_OS_WIN32
  Gtk::TreeModel::iterator m_iter_existing_network;
#endif
  Gtk::TreeModel::iterator m_iter_existing_other;
 
  // Dummy children to indicate that a parent item has no (real) children
#ifndef G_OS_WIN32
  std::shared_ptr<Gtk::TreeModel::iterator> m_iter_existing_network_dummy;
#endif
  std::shared_ptr<Gtk::TreeModel::iterator> m_iter_existing_recent_dummy;


#ifndef GLOM_ENABLE_CLIENT_ONLY
  struct buffer { static const guint SIZE = 1024; char buf[SIZE]; };
  std::shared_ptr<buffer> m_current_buffer;
#endif /* !GLOM_ENABLE_CLIENT_ONLY */
    
#ifndef G_OS_WIN32
  EpcServiceMonitor* m_service_monitor;
#endif

  // URI chosen in the file chooser
  Glib::ustring m_chosen_uri;
};

} //namespace Glom

#endif //GLOM_DIALOG_EXISTING_OR_NEW_H

