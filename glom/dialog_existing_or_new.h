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
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef GLOM_DIALOG_EXISTING_OR_NEW_H
#define GLOM_DIALOG_EXISTING_OR_NEW_H

#ifndef G_OS_WIN32
# include <libepc/service-monitor.h>
#endif

#include <memory>
#include <giomm/asyncresult.h>
#include <giomm/file.h>
#include <giomm/fileenumerator.h>
#include <giomm/inputstream.h>
#include <gtkmm/dialog.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treestore.h>
#include <gtkmm/recentinfo.h>
#include <libglademm/xml.h>
#include <glom/utility_widgets/db_adddel/cellrenderer_buttontext.h>

namespace Glom
{

class Dialog_ExistingOrNew
  : public Gtk::Dialog
{
  typedef sigc::signal<void, const std::string&> SignalNew;
  typedef sigc::signal<void, const std::string&> SignalOpenFromUri;
#ifndef G_OS_WIN32
  typedef sigc::signal<void, EpcServiceInfo*, const Glib::ustring&> SignalOpenFromRemote;
#endif

public:
  Dialog_ExistingOrNew(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Dialog_ExistingOrNew();

  SignalNew signal_new() const { return m_signal_new; }
  SignalOpenFromUri signal_open_from_uri() const { return m_signal_open_from_uri; }

#ifndef G_OS_WIN32
  SignalOpenFromRemote signal_open_from_remote() const { return m_signal_open_from_remote; }
#endif

protected:
  void existing_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter);
  void new_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter);

  void on_enumerate_children(const Glib::RefPtr<Gio::AsyncResult>& res);
  void on_next_files(const Glib::RefPtr<Gio::AsyncResult>& res);
  void on_read(const Glib::RefPtr<Gio::AsyncResult>& res);
  void on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& res);

#ifndef G_OS_WIN32
  static void on_service_found_static(EpcServiceMonitor* monitor, gchar* name, EpcServiceInfo* info, gpointer user_data) { static_cast<Dialog_ExistingOrNew*>(user_data)->on_service_found(name, info); }
  static void on_service_removed_static(EpcServiceMonitor* monitor, gchar* name, gchar* type, gpointer user_data) { static_cast<Dialog_ExistingOrNew*>(user_data)->on_service_removed(name, type); }

  void on_service_found(const Glib::ustring& name, EpcServiceInfo* info);
  void on_service_removed(const Glib::ustring& name, const Glib::ustring& type);
#endif

  void on_existing_row_activated(const Gtk::TreePath& path, Gtk::TreeViewColumn* column);
  void on_existing_button_clicked(const Gtk::TreePath& path);
  void on_new_row_activated(const Gtk::TreePath& path, Gtk::TreeViewColumn* column);
  void on_new_button_clicked(const Gtk::TreePath& path);

  void existing_activated(const Gtk::TreeIter& iter);
  void new_activated(const Gtk::TreeIter& iter);

  class ExistingModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ExistingModelColumns()
    {
      add(m_col_title);
      add(m_col_button_text); 
      add(m_col_time);

#ifndef G_OS_WIN32
      add(m_col_service_name);
      add(m_col_service_info);
#endif

      add(m_col_recent_info);
    }

    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn<Glib::ustring> m_col_button_text;
    Gtk::TreeModelColumn<std::time_t> m_col_time; // Sort criteria

#ifndef G_OS_WIN32
    // For service discovery:
    Gtk::TreeModelColumn<Glib::ustring> m_col_service_name;
    Gtk::TreeModelColumn<EpcServiceInfo*> m_col_service_info;
#endif

    // For recently used resources:
    // TODO: We can't use Glib::RefPtr<Gtk::RecentInfo> directly here, due to
    // bug #. Therefore, the refptrs are dynamically allocated and explicitely
    // freed in the destructor.
    Gtk::TreeModelColumn<Glib::RefPtr<Gtk::RecentInfo>*> m_col_recent_info;
  };
  
  class NewModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    NewModelColumns()
    { add(m_col_title); add(m_col_button_text); add(m_col_template_uri); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn<Glib::ustring> m_col_button_text;
    Gtk::TreeModelColumn<std::string> m_col_template_uri;
  };

  ExistingModelColumns m_existing_columns;
  Glib::RefPtr<Gtk::TreeStore> m_existing_model;
  Gtk::TreeView* m_existing_view;

  NewModelColumns m_new_columns;
  Gtk::TreeView* m_new_view;
  Glib::RefPtr<Gtk::TreeStore> m_new_model;

  Gtk::TreeViewColumn m_existing_column_title;
  Gtk::TreeViewColumn m_existing_column_button;
  Gtk::CellRendererPixbuf m_existing_icon_renderer;
  Gtk::CellRendererText m_existing_title_renderer;
  GlomCellRenderer_ButtonText m_existing_button_renderer;

  Gtk::TreeViewColumn m_new_column_title;
  Gtk::TreeViewColumn m_new_column_button;
  Gtk::CellRendererPixbuf m_new_icon_renderer;
  Gtk::CellRendererText m_new_title_renderer;
  GlomCellRenderer_ButtonText m_new_button_renderer;

  Gtk::TreeIter m_iter_existing_recent;
  Gtk::TreeIter m_iter_existing_network;
  Gtk::TreeIter m_iter_existing_other;

  Gtk::TreeIter m_iter_new_empty;
  Gtk::TreeIter m_iter_new_template;
  
  Glib::RefPtr<Gio::File> m_examples_dir;
  Glib::RefPtr<Gio::FileEnumerator> m_examples_enumerator;
  Glib::RefPtr<Gio::File> m_current_example;
  Glib::RefPtr<Gio::InputStream> m_current_stream;

  struct buffer { static const guint SIZE = 1024; char buf[SIZE]; };
  std::auto_ptr<buffer> m_current_buffer;

#ifndef G_OS_WIN32
  EpcServiceMonitor* m_service_monitor;
#endif

  SignalNew m_signal_new;
  SignalOpenFromUri m_signal_open_from_uri;

#ifndef G_OS_WIN32
  SignalOpenFromRemote m_signal_open_from_remote;
#endif
};

} //namespace Glom

#endif //GLOM_DIALOG_DATABASE_PREFERENCES_H

