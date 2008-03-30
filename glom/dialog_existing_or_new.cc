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

#include "config.h"
#include "dialog_existing_or_new.h"

#include <libxml++/parsers/saxparser.h>

#include <glibmm/i18n.h>
#include <giomm/contenttype.h>
#include <gtkmm/recentmanager.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#ifdef G_OS_WIN32
# include <glib/gwin32.h>
#else
# include <libepc/service-type.h>
#endif

#include <iostream>

namespace
{

// Reads the title of an example from the first few characters of the XML
class Parser: public xmlpp::SaxParser
{
public:
  Parser() {}

  Glib::ustring get_example_title(const std::string& beginning)
  {
    parse_chunk(beginning);
    return m_title;
  }

protected:
  virtual void on_start_element(const Glib::ustring& name, const AttributeList& attributes)
  {
    if(m_title.empty()) // Already found name? Wait for parse_chunk() call to return.
    {
      if(name == "glom_document")
      {
        for(AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++ iter)
        {
          if(iter->name == "database_title")
          {
            m_title = iter->value;
            // TODO: We should stop parsing here somehow, but I don't
            // think we can throw an exception through the C library back
            // to get_example_name, and there does not seem to be API to
            // stop parsing.
            break;
          }
        }
      }
    }
  }

  Glib::ustring m_title;
};

}

namespace Glom
{

Dialog_ExistingOrNew::Dialog_ExistingOrNew(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  refGlade->get_widget("existing_or_new_existing_treeview", m_existing_view);
  refGlade->get_widget("existing_or_new_new_treeview", m_new_view);

  if(!m_existing_view || !m_new_view)
    throw std::runtime_error("Glade file does not contain treeviews for ExistingOrNew dialog");

  m_existing_model = Gtk::TreeStore::create(m_existing_columns);
  m_existing_model->set_sort_column(m_existing_columns.m_col_time, Gtk::SORT_DESCENDING);
  m_existing_view->set_model(m_existing_model);

  m_new_model = Gtk::TreeStore::create(m_new_columns);
  m_new_view->set_model(m_new_model);

  m_existing_column_title.set_expand(true);
  m_existing_column_title.pack_start(m_existing_icon_renderer, false);
  m_existing_column_title.pack_start(m_existing_title_renderer, true);
  m_existing_column_title.set_cell_data_func(m_existing_icon_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::existing_icon_data_func));
  m_existing_column_title.add_attribute(m_existing_title_renderer, "text", m_existing_columns.m_col_title.index());
  m_existing_view->append_column(m_existing_column_title);

  m_existing_column_button.pack_end(m_existing_button_renderer, false);
  m_existing_column_button.add_attribute(m_existing_button_renderer, "text", m_existing_columns.m_col_button_text.index());
  m_existing_view->append_column(m_existing_column_button);

  m_new_column_title.set_expand(true);
  m_new_column_title.pack_start(m_new_icon_renderer, false);
  m_new_column_title.pack_start(m_new_title_renderer, true);
  m_new_column_title.set_cell_data_func(m_new_icon_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::new_icon_data_func));
  m_new_column_title.add_attribute(m_new_title_renderer, "text", m_new_columns.m_col_title.index());
  m_new_view->append_column(m_new_column_title);

  m_new_column_button.pack_end(m_new_button_renderer, false);
  m_new_column_button.add_attribute(m_new_button_renderer, "text", m_new_columns.m_col_button_text.index());
  m_new_view->append_column(m_new_column_button);

  m_existing_view->set_headers_visible(false);
  m_existing_view->signal_row_activated().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_row_activated));
  m_existing_button_renderer.signal_clicked().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_button_clicked));
  m_new_view->set_headers_visible(false);
  m_new_view->signal_row_activated().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_row_activated));
  m_new_button_renderer.signal_clicked().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_button_clicked));

  m_iter_existing_recent = m_existing_model->append();
  (*m_iter_existing_recent)[m_existing_columns.m_col_title] = _("Recently Opened");
  
  m_iter_existing_network = m_existing_model->append();
  (*m_iter_existing_network)[m_existing_columns.m_col_title] = _("Local Network");

  m_iter_existing_other = m_existing_model->append();
  (*m_iter_existing_other)[m_existing_columns.m_col_title] = _("Other");
  (*m_iter_existing_other)[m_existing_columns.m_col_button_text] = _("Select File…");
  
  m_iter_new_empty = m_new_model->append();
  (*m_iter_new_empty)[m_new_columns.m_col_title] = _("New Empty Document");
  (*m_iter_new_empty)[m_new_columns.m_col_button_text] = _("Create");

  m_iter_new_template = m_new_model->append();
  (*m_iter_new_template)[m_new_columns.m_col_title] = _("New From Template");

  // Load example files
#ifdef G_OS_WIN32
  gchar* dir = g_win32_get_package_installation_subdirectory(NULL, NULL, "share/doc/examples");
  std::string path(dir);
  g_free(dir);
#else
  const char* path = GLOM_EXAMPLES_DIR;
#endif

  m_examples_dir = Gio::File::create_for_path(path);

  try
  {
    m_examples_dir->enumerate_children_async(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_enumerate_children),
                                             G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE","G_FILE_ATTRIBUTE_STANDARD_NAME);
    // TODO: Monitor example directory for new/removed files?
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "Could not enumerate examples: " << ex.what() << std::endl;
  }

  // Browse local network
#ifndef G_OS_WIN32
  gchar* service_type = epc_service_type_new(EPC_PROTOCOL_HTTPS, "glom");
  m_service_monitor = epc_service_monitor_new_for_types(NULL, service_type, NULL);
  g_signal_connect(m_service_monitor, "service-found", G_CALLBACK(on_service_found_static), this);
  g_signal_connect(m_service_monitor, "service-removed", G_CALLBACK(on_service_removed_static), this);
  g_free(service_type);
#endif

  // Add recently used files
  Gtk::RecentManager::ListHandle_RecentInfos infos = Gtk::RecentManager::get_default()->get_items();
  for(Gtk::RecentManager::ListHandle_RecentInfos::const_iterator iter = infos.begin(); iter != infos.end(); ++ iter)
  {
    Glib::RefPtr<Gtk::RecentInfo> info = *iter;
    if(info->get_mime_type() == "application/x-glom")
    {
      Gtk::TreeIter iter = m_existing_model->append(m_iter_existing_recent->children());
      (*iter)[m_existing_columns.m_col_title] = info->get_display_name();
      (*iter)[m_existing_columns.m_col_time] = info->get_modified();
      (*iter)[m_existing_columns.m_col_button_text] = _("Open");
      (*iter)[m_existing_columns.m_col_recent_info] = new Glib::RefPtr<Gtk::RecentInfo>(info);
    }
  }
}

Dialog_ExistingOrNew::~Dialog_ExistingOrNew()
{
  if(m_service_monitor)
  {
    g_object_unref(m_service_monitor);
    m_service_monitor = NULL;
  }

#ifndef G_OS_WIN32
  // Release the service infos in the treestore
  {
    const Gtk::TreeNodeChildren& children = m_iter_existing_network->children();
    for(Gtk::TreeIter iter = children.begin(); iter != children.end(); ++ iter)
      epc_service_info_unref((*iter)[m_existing_columns.m_col_service_info]);
  }
#endif

  // Release the recent infos (see comment in the header for why these
  // have to be dynamically allocated)
  {
    const Gtk::TreeNodeChildren& children = m_iter_existing_recent->children();
    for(Gtk::TreeIter iter = children.begin(); iter != children.end(); ++ iter)
    {
      Glib::RefPtr<Gtk::RecentInfo>* info = (*iter)[m_existing_columns.m_col_recent_info];
      delete info;
    }
  }
}

void Dialog_ExistingOrNew::existing_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter)
{
  Gtk::CellRendererPixbuf* pixbuf_renderer = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
  if(!pixbuf_renderer) throw std::logic_error("Renderer not a pixbuf renderer in existing_icon_data_func");

  pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_BUTTON;
  pixbuf_renderer->property_stock_id() = "";
  pixbuf_renderer->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>();
      
  if(iter == m_iter_existing_recent)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::INDEX.id; // TODO: More meaningful icon?
  else if(iter == m_iter_existing_network)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::NETWORK.id;
  else if(iter == m_iter_existing_other)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::OPEN.id;
  else
  {
    if(m_existing_model->is_ancestor(m_iter_existing_recent, iter))
    {
      //Glib::RefPtr<Gtk::RecentInfo>* info = (*iter)[m_existing_columns.m_col_recent_info];
      //pixbuf_renderer->property_pixbuf() = (*info)->get_icon(Gtk::ICON_SIZE_BUTTON);
      pixbuf_renderer->set_property("icon-name", Glib::ustring("glom"));
    }
    else if(m_existing_model->is_ancestor(m_iter_existing_network, iter))
    {
      //pixbuf_renderer->property_stock_id() = Gtk::Stock::CONNECT.id;
      pixbuf_renderer->set_property("icon-name", Glib::ustring("glom"));
    }
    else
    {
      throw std::logic_error("Unexpected iterator in existing_icon_data_func");
    }
  }
}

void Dialog_ExistingOrNew::new_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeIter& iter)
{
  Gtk::CellRendererPixbuf* pixbuf_renderer = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
  if(!pixbuf_renderer) throw std::logic_error("Renderer not a pixbuf renderer in new_icon_data_func");

  pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_BUTTON;
  pixbuf_renderer->property_stock_id() = "";
  pixbuf_renderer->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>();
      
  if(iter == m_iter_new_empty)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::NEW.id;
  else if(iter == m_iter_new_template)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::EDIT.id; // TODO: More meaningful icon?
  else
  {
    if(m_new_model->is_ancestor(m_iter_new_template, iter))
    {
      pixbuf_renderer->set_property("icon-name", Glib::ustring("glom"));
    }
    else
    {
      throw std::logic_error("Unexpected iterator in new_icon_data_func");
    }
  }
}

void Dialog_ExistingOrNew::on_enumerate_children(const Glib::RefPtr<Gio::AsyncResult>& res)
{
  try
  {
    m_examples_enumerator = m_examples_dir->enumerate_children_finish(res);
    m_examples_enumerator->next_files_async(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_next_files));
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "Could not enumerate examples: " << ex.what() << std::endl;
    
    m_examples_enumerator.reset();
    m_examples_dir.reset();
  }
}

void Dialog_ExistingOrNew::on_next_files(const Glib::RefPtr<Gio::AsyncResult>& res)
{
  try
  {
    const Glib::ListHandle<Glib::RefPtr<Gio::FileInfo> >& list = m_examples_enumerator->next_files_finish(res);
    if (list.empty())
    {
      // Done
      m_examples_dir.reset();
      m_examples_enumerator.reset();
    }
    else
    {
      // Load file
      Glib::RefPtr<Gio::FileInfo> info = *list.begin();
      Glib::ustring content_type = info->get_attribute_string(G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
      Glib::ustring mime_type = Gio::content_type_get_mime_type(content_type);

      if(mime_type == "application/x-glom")
      {
        m_current_example = Gio::File::create_for_path(Glib::build_filename(m_examples_dir->get_path(), info->get_name()));
        m_current_example->read_async(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_read));
      }
      else
      {
        // File is not a glom file, continue with next
        m_examples_enumerator->next_files_async(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_next_files));
      }
    }
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "Could not enumerate examples: " << ex.what() << std::endl;

    m_examples_dir.reset();
    m_examples_enumerator.reset();
  }
}

void Dialog_ExistingOrNew::on_read(const Glib::RefPtr<Gio::AsyncResult>& res)
{
  try
  {
    m_current_stream = m_current_example->read_finish(res);
    m_current_buffer.reset(new buffer);
    m_current_stream->read_async(m_current_buffer->buf, buffer::SIZE, sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_stream_read));
  }
  catch(const Glib::Exception& exception)
  {
    // Could not read this file, read next
    m_examples_enumerator->next_files_async(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_next_files));

    m_current_example.reset();
    m_current_stream.reset();
    m_current_buffer.reset();
  }
}

void Dialog_ExistingOrNew::on_stream_read(const Glib::RefPtr<Gio::AsyncResult>& res)
{
  try
  {
    gssize size_read = m_current_stream->read_finish(res);
    std::string data(m_current_buffer->buf, size_read);
    // TODO: Check that data is valid UTF-8, the last character might be truncated

    Parser parser;
    Glib::ustring title = parser.get_example_title(data);

    if(title.empty())
    {
      // TODO: Read more data from this file?
    }
    else
    {
      // Add to list
      Gtk::TreeIter iter = m_new_model->append(m_iter_new_template->children());
      (*iter)[m_new_columns.m_col_title] = title;
      (*iter)[m_new_columns.m_col_button_text] = _("Create");
      (*iter)[m_new_columns.m_col_template_uri] = m_current_example->get_uri();
    }
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "Could not read " << m_current_example->get_path() << ": " << ex.what() << std::endl;
  }

  // Done with this, read next file
  m_current_example.reset();
  m_current_stream.reset();
  m_current_buffer.reset();

  m_examples_enumerator->next_files_async(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_next_files));
}

#ifndef G_OS_WIN32
void Dialog_ExistingOrNew::on_service_found(const Glib::ustring& name, EpcServiceInfo* info)
{
  gchar* title = g_strdup_printf(_("%s on %s (via %s)"), name.c_str(), epc_service_info_get_host(info), epc_service_info_get_interface(info));
  Gtk::TreeIter iter = m_existing_model->prepend(m_iter_existing_network->children());
  (*iter)[m_existing_columns.m_col_title] = title;
  (*iter)[m_existing_columns.m_col_button_text] = _("Open");
  (*iter)[m_existing_columns.m_col_time] = std::time(NULL); /* sort more recently discovered items above */
  (*iter)[m_existing_columns.m_col_service_name] = name;
  (*iter)[m_existing_columns.m_col_service_info] = info;

  epc_service_info_ref(info);
  g_free(title);
}

void Dialog_ExistingOrNew::on_service_removed(const Glib::ustring& name, const Glib::ustring& type)
{
  // Find the entry with the given name
  const Gtk::TreeNodeChildren& children = m_iter_existing_network->children();
  for(Gtk::TreeIter iter = children.begin(); iter != children.end(); ++ iter)
  {
    if((*iter)[m_existing_columns.m_col_service_name] == name)
    {
      // Remove from store
      epc_service_info_unref((*iter)[m_existing_columns.m_col_service_info]);
      m_existing_model->erase(iter);
      break;
    }
  }
}
#endif // !G_OS_WIN32

void Dialog_ExistingOrNew::on_existing_row_activated(const Gtk::TreePath& path, Gtk::TreeViewColumn* column)
{
  existing_activated(m_existing_model->get_iter(path));
}

void Dialog_ExistingOrNew::on_existing_button_clicked(const Gtk::TreePath& path)
{
  existing_activated(m_existing_model->get_iter(path));
}

void Dialog_ExistingOrNew::on_new_row_activated(const Gtk::TreePath& path, Gtk::TreeViewColumn* column)
{
  new_activated(m_new_model->get_iter(path));
}

void Dialog_ExistingOrNew::on_new_button_clicked(const Gtk::TreePath& path)
{
  new_activated(m_new_model->get_iter(path));
}

void Dialog_ExistingOrNew::existing_activated(const Gtk::TreeIter& iter)
{
  if(iter == m_iter_existing_other)
  {
    Gtk::FileChooserDialog dialog(*this, "Choose a glom file to open");
    Gtk::FileFilter filter;
    filter.add_mime_type("application/x-glom");
    filter.set_name("Glom files");
    dialog.add_filter(filter);

    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_ACCEPT);
    //dialog.set_default_response(Gtk::RESPONSE_ACCEPT);

    if(dialog.run() == Gtk::RESPONSE_ACCEPT)
    {
      dialog.hide();
      m_signal_open_from_uri.emit(dialog.get_uri());
      response(Gtk::RESPONSE_ACCEPT);
    }
  }
  else if(m_existing_model->is_ancestor(m_iter_existing_recent, iter))
  {
    Glib::RefPtr<Gtk::RecentInfo>* info = (*iter)[m_existing_columns.m_col_recent_info];
    m_signal_open_from_uri.emit((*info)->get_uri());
    response(Gtk::RESPONSE_ACCEPT);
  }
  else if(m_existing_model->is_ancestor(m_iter_existing_network, iter))
  {
#ifndef G_OS_WIN32
    m_signal_open_from_remote.emit((*iter)[m_existing_columns.m_col_service_info], (*iter)[m_existing_columns.m_col_service_name]);
    response(Gtk::RESPONSE_ACCEPT);
#endif
  }
}

void Dialog_ExistingOrNew::new_activated(const Gtk::TreeIter& iter)
{
  if(iter == m_iter_new_empty)
  {
    m_signal_new.emit(std::string());
    response(Gtk::RESPONSE_ACCEPT);
  }
  else if(m_new_model->is_ancestor(m_iter_new_template, iter))
  {
    m_signal_new.emit((*iter)[m_new_columns.m_col_template_uri]);
    response(Gtk::RESPONSE_ACCEPT);
  }
}

} //namespace Glom
