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
#include <libxml/parser.h> //For xmlStopParser()

#include <glibmm/i18n.h>
#include <giomm/contenttype.h>
#include <gtkmm/recentmanager.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/stock.h>

#ifdef GLOM_ENABLE_MAEMO
#include <hildon-fmmm/file-chooser-dialog.h>
#endif // GLOM_ENABLE_MAEMO

#ifdef G_OS_WIN32
# include <glib/gwin32.h>
#else
# include <libepc/service-type.h>
#endif

#include <iostream>

#ifdef GLOM_ENABLE_CLIENT_ONLY
#define NEW_PAGE 1
#endif /* GLOM_ENABLE_CLIENT_ONLY */

namespace
{

const char* RECENT_DUMMY_TEXT = N_("No recently used documents available.");
const char* NETWORK_DUMMY_TEXT = N_("No sessions found on the local network.");

#ifndef GLOM_ENABLE_CLIENT_ONLY
const char* TEMPLATE_DUMMY_TEXT = N_("No templates available.");
#endif

//TODO_Performance: A DomParser or XmlReader might be faster, or even a regex.
/// Reads the title of an example from the first few characters of the XML.
class Parser: public xmlpp::SaxParser
{
public:
  Parser()
  {}

  Glib::ustring get_example_title(const std::string& beginning)
  {
    parse_chunk(beginning);
    return m_title;
  }

private:
  virtual void on_start_element(const Glib::ustring& name, const AttributeList& attributes)
  {
    if(m_title.empty()) // Already found name? Wait for parse_chunk() call to return.
    {
      if(name == "glom_document") //See document_glom.cc for the #defines.
      {
        for(AttributeList::const_iterator iter = attributes.begin(); iter != attributes.end(); ++ iter)
        {
          if(iter->name == "database_title")
          {
            m_title = iter->value;

            // Stop parsing here because we have what we need:
            xmlStopParser(context_);

            break;
          }
        }
      }
    }
  }

  Glib::ustring m_title;
};

} //anonymous namespace

namespace Glom
{

const char* Dialog_ExistingOrNew::glade_id("dialog_existing_or_new");
const bool Dialog_ExistingOrNew::glade_developer(false);

Dialog_ExistingOrNew::Dialog_ExistingOrNew(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Gtk::Dialog(cobject)
{
#ifdef GLOM_ENABLE_CLIENT_ONLY
  //Don't mention creation of new documents in client-only mode:
  Gtk::Label* label = 0;
  builder->get_widget("existing_or_new_label", label);
  label->set_text(_("Open a Document"));
#endif //GLOM_ENABLE_CLIENT_ONLY

  builder->get_widget("existing_or_new_existing_treeview", m_existing_view);
  g_assert(m_existing_view);

  builder->get_widget("existing_or_new_notebook", m_notebook);
  builder->get_widget("existing_or_new_button_select", m_select_button);

  if(!m_notebook || !m_select_button)
    throw std::runtime_error("Glade file does not contain the notebook or the select button for ExistingOrNew dialog.");

  m_existing_model = Gtk::TreeStore::create(m_existing_columns);
  m_existing_model->set_sort_column(m_existing_columns.m_col_time, Gtk::SORT_DESCENDING);
  m_existing_view->set_model(m_existing_model);



  m_iter_existing_other = m_existing_model->append();
  (*m_iter_existing_other)[m_existing_columns.m_col_title] = _("Select File");

#ifndef G_OS_WIN32
  m_iter_existing_network = m_existing_model->append();
  (*m_iter_existing_network)[m_existing_columns.m_col_title] = _("Local Network");
#endif

  m_iter_existing_recent = m_existing_model->append();
  (*m_iter_existing_recent)[m_existing_columns.m_col_title] = _("Recently Opened");



  m_existing_column_title.set_expand(true);
  m_existing_column_title.pack_start(m_existing_icon_renderer, false);
  m_existing_column_title.pack_start(m_existing_title_renderer, true);
  m_existing_column_title.set_cell_data_func(m_existing_icon_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::existing_icon_data_func));
  m_existing_column_title.set_cell_data_func(m_existing_title_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::existing_title_data_func));
  m_existing_view->append_column(m_existing_column_title);

  m_existing_view->set_headers_visible(false);
  m_existing_view->signal_row_activated().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_row_activated));


  // Browse local network
#ifndef G_OS_WIN32
  gchar* service_type = epc_service_type_new(EPC_PROTOCOL_HTTPS, "glom");
  m_service_monitor = epc_service_monitor_new_for_types(0, service_type, (void*)0);
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
      Gtk::TreeModel::iterator iter = m_existing_model->append(m_iter_existing_recent->children());
      (*iter)[m_existing_columns.m_col_title] = info->get_display_name();
      (*iter)[m_existing_columns.m_col_time] = info->get_modified();
      (*iter)[m_existing_columns.m_col_recent_info] = info;
    }
  }

  const Gtk::TreeNodeChildren& children = m_iter_existing_recent->children();
  if(children.begin() == children.end())
    m_iter_existing_recent_dummy = create_dummy_item_existing(m_iter_existing_recent, _(RECENT_DUMMY_TEXT));

#ifndef G_OS_WIN32
  // Will be removed when items are added:
  m_iter_existing_network_dummy = create_dummy_item_existing(m_iter_existing_network, _(NETWORK_DUMMY_TEXT));
#endif


  // Expand recently used files and the networked files,
  // because the contents help to explain what this is:
  m_existing_view->expand_row(m_existing_model->get_path(m_iter_existing_recent), false);
#ifndef G_OS_WIN32
  m_existing_view->expand_row(m_existing_model->get_path(m_iter_existing_network), false);
#endif

  m_select_button->signal_clicked().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_select_clicked));
  m_select_button->set_image(*Gtk::manage(new Gtk::Image(Gtk::Stock::APPLY, Gtk::ICON_SIZE_BUTTON)));

#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_notebook->signal_switch_page().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_switch_page));
#endif /* !GLOM_ENABLE_CLIENT_ONLY */

  Glib::RefPtr<Gtk::TreeView::Selection> existing_view_selection = m_existing_view->get_selection();
  existing_view_selection->signal_changed().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_selection_changed));
  existing_view_selection->set_select_function( sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_existing_select_func) );

#ifndef GLOM_ENABLE_CLIENT_ONLY
  builder->get_widget("existing_or_new_new_treeview", m_new_view);
  g_assert(m_new_view);
  m_new_model = Gtk::TreeStore::create(m_new_columns);
  m_new_view->set_model(m_new_model);

  m_iter_new_empty = m_new_model->append();
  (*m_iter_new_empty)[m_new_columns.m_col_title] = _("New Empty Document");

  m_iter_new_template = m_new_model->append();
  (*m_iter_new_template)[m_new_columns.m_col_title] = _("New From Template");

  m_new_column_title.set_expand(true);
  m_new_column_title.pack_start(m_new_icon_renderer, false);
  m_new_column_title.pack_start(m_new_title_renderer, true);
  m_new_column_title.set_cell_data_func(m_new_icon_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::new_icon_data_func));
  m_new_column_title.set_cell_data_func(m_new_title_renderer, sigc::mem_fun(*this, &Dialog_ExistingOrNew::new_title_data_func));
  m_new_view->append_column(m_new_column_title);

  m_new_view->set_headers_visible(false);
  m_new_view->signal_row_activated().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_row_activated));

  m_iter_new_template_dummy = create_dummy_item_new(m_iter_new_template, _(TEMPLATE_DUMMY_TEXT));

  Glib::RefPtr<Gtk::TreeView::Selection> new_view_selection = m_new_view->get_selection();
  new_view_selection->signal_changed().connect(sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_selection_changed));
  new_view_selection->set_select_function( sigc::mem_fun(*this, &Dialog_ExistingOrNew::on_new_select_func) );
#else /* GLOM_ENABLE_CLIENT_ONLY */
  m_notebook->remove_page(NEW_PAGE);
  m_notebook->set_show_tabs(false);
#endif /* !GLOM_ENABLE_CLIENT_ONLY */


 // Load example files:
#ifndef GLOM_ENABLE_CLIENT_ONLY

#ifdef G_OS_WIN32
  gchar* dir = g_win32_get_package_installation_directory_of_module(0);
  std::string path = Glib::build_filename(dir, "share" G_DIR_SEPARATOR_S "doc"
                                                       G_DIR_SEPARATOR_S "glom"
                                                       G_DIR_SEPARATOR_S "examples");
  g_free(dir);

  if(!Glib::file_test(path, Glib::FILE_TEST_EXISTS))
    path = GLOM_DOCDIR_EXAMPLES;
#else
  const char *const path = GLOM_DOCDIR_EXAMPLES;
#endif //G_OS_WIN32

  //Show the installed example files,
  //falling back to the ones from the local source tree if none are installed:
  if(!list_examples_at_path(path))
    list_examples_at_path(GLOM_DOCDIR_EXAMPLES_NOTINSTALLED);

#endif //!GLOM_ENABLE_CLIENT_ONLY

  //Make sure the first item is visible,
  //which is not always the case on Maemo.
  m_existing_view->scroll_to_row(
    Gtk::TreeModel::Path(m_iter_existing_other) );

  update_ui_sensitivity();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Dialog_ExistingOrNew::list_examples_at_path(const std::string& path)
{
  //std::cout << "debug: " << G_STRFUNC << ": path=" << path << std::endl;

  Glib::RefPtr<Gio::File> examples_dir = Gio::File::create_for_path(path);
  Glib::RefPtr<Gio::FileInfo> info;

  try
  {
    Glib::RefPtr<Gio::FileEnumerator> examples = examples_dir->enumerate_children(G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE","G_FILE_ATTRIBUTE_STANDARD_NAME);
    bool example_found = false;
    while(info = examples->next_file())
    {
      const Glib::ustring title = get_title_from_example(info, examples_dir);
      if(!title.empty())
      {
        append_example(title, Gio::File::create_for_path(Glib::build_filename(examples_dir->get_path(), info->get_name())));
      	example_found = true;
      }
    }

    // TODO: Monitor example directory for new/removed files?
    return example_found;
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "Could not enumerate examples at path=" << path << ", Error=" << ex.what() << std::endl;
  }

  return false;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Dialog_ExistingOrNew::~Dialog_ExistingOrNew()
{
#ifndef G_OS_WIN32
  if(m_service_monitor)
  {
    g_object_unref(m_service_monitor);
    m_service_monitor = 0;
  }

  // Release the service infos in the treestore
  if(!m_iter_existing_network_dummy.get())
  {
    const Gtk::TreeNodeChildren& children = m_iter_existing_network->children();
    for(Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++ iter)
      epc_service_info_unref((*iter)[m_existing_columns.m_col_service_info]);
  }
#endif

}

bool Dialog_ExistingOrNew::on_existing_select_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool /* path_currently_selected */)
{
  Gtk::TreeModel::iterator iter = model->get_iter(path);
#ifndef G_OS_WIN32
  if(iter == m_iter_existing_network)
    return false; /* Do not allow parent nodes to be selected. */
#endif
  if(iter == m_iter_existing_recent)
    return false;

  return true;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool Dialog_ExistingOrNew::on_new_select_func(const Glib::RefPtr<Gtk::TreeModel>& model, const Gtk::TreeModel::Path& path, bool /* path_currently_selected */)
{
  Gtk::TreeModel::iterator iter = model->get_iter(path);
  if(iter == m_iter_new_template)
    return false; /* Do not allow parent nodes to be selected. */
  else
    return true;
}
#endif //GLOM_ENABLE_CLIENT_ONLY

Dialog_ExistingOrNew::Action Dialog_ExistingOrNew::get_action_impl(Gtk::TreeModel::iterator& iter) const
{
  if(m_notebook->get_current_page() == 0)
  {
    if(m_existing_view->get_selection()->count_selected_rows() == 0)
      return NONE;

    iter = m_existing_view->get_selection()->get_selected();
    if(m_existing_model->is_ancestor(m_iter_existing_recent, iter))
      return OPEN_URI;
#ifndef G_OS_WIN32
    if(m_existing_model->is_ancestor(m_iter_existing_network, iter))
      return OPEN_REMOTE;
#endif
    if(iter == m_iter_existing_other)
      return OPEN_URI;

    return NONE;
  }
  else
  {
    #ifndef GLOM_ENABLE_CLIENT_ONLY
    if(m_new_view->get_selection()->count_selected_rows() == 0)
      return NONE;

    iter = m_new_view->get_selection()->get_selected();
    if(m_new_model->is_ancestor(m_iter_new_template, iter))
      return NEW_FROM_TEMPLATE;
    else if(iter == m_iter_new_empty)
      return NEW_EMPTY;
    else
      return NONE;
    #endif //GLOM_ENABLE_CLIENT_ONLY
  }

  return NONE;
}

Dialog_ExistingOrNew::Action Dialog_ExistingOrNew::get_action() const
{
  Gtk::TreeModel::iterator iter;
  return get_action_impl(iter);
}

Glib::ustring Dialog_ExistingOrNew::get_uri() const
{
  Gtk::TreeModel::iterator iter;
  const Action action = get_action_impl(iter);

  #ifndef GLOM_ENABLE_CLIENT_ONLY
  if(action == NEW_FROM_TEMPLATE)
  {
    return (*iter)[m_new_columns.m_col_template_uri];
  }
  else
  #endif //GLOM_ENABLE_CLIENT_ONLY
  if(action == OPEN_URI)
  {
    if(iter == m_iter_existing_other)
    {
      return m_chosen_uri;
    }
    else
    {
      Glib::RefPtr<Gtk::RecentInfo> info = (*iter)[m_existing_columns.m_col_recent_info];
      return info->get_uri();
    }
  }
  else
  {
    throw std::logic_error("Dialog_ExistingOrNew::get_uri: action is neither NEW_FROM_TEMPLATE nor OPEN_URI");
  }

  return Glib::ustring();
}

#ifndef G_OS_WIN32
EpcServiceInfo* Dialog_ExistingOrNew::get_service_info() const
{
  Gtk::TreeModel::iterator iter;
  Action action = get_action_impl(iter);

  if(action == OPEN_REMOTE)
    return (*iter)[m_existing_columns.m_col_service_info];
  else
    throw std::logic_error("Dialog_ExistingOrNew::get_service_info: action is not OPEN_REMOTE");

  return 0;
}

Glib::ustring Dialog_ExistingOrNew::get_service_name() const
{
  Gtk::TreeModel::iterator iter;
  Action action = get_action_impl(iter);

  if(action == OPEN_REMOTE)
    return (*iter)[m_existing_columns.m_col_service_name];
  else
    throw std::logic_error("Dialog_ExistingOrNew::get_service_name: action is not OPEN_REMOTE");

  return Glib::ustring();
}
#endif

std::auto_ptr<Gtk::TreeModel::iterator> Dialog_ExistingOrNew::create_dummy_item_existing(const Gtk::TreeModel::iterator& parent, const Glib::ustring& text)
{
  Gtk::TreeModel::iterator iter = m_existing_model->append(parent->children());
  (*iter)[m_existing_columns.m_col_title] = text;
  return std::auto_ptr<Gtk::TreeModel::iterator>(new Gtk::TreeModel::iterator(iter));
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
std::auto_ptr<Gtk::TreeModel::iterator> Dialog_ExistingOrNew::create_dummy_item_new(const Gtk::TreeModel::iterator& parent, const Glib::ustring& text)
{
  Gtk::TreeModel::iterator iter = m_new_model->append(parent->children());
  (*iter)[m_new_columns.m_col_title] = text;
  return std::auto_ptr<Gtk::TreeModel::iterator>(new Gtk::TreeModel::iterator(iter));
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::existing_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  Gtk::CellRendererPixbuf* pixbuf_renderer = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
  if(!pixbuf_renderer)
  throw std::logic_error("Renderer not a pixbuf renderer in existing_icon_data_func");

  pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_BUTTON;
  pixbuf_renderer->property_stock_id() = "";
  pixbuf_renderer->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>();

  if(iter == m_iter_existing_recent)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::INDEX.id; // TODO: More meaningful icon?

  pixbuf_renderer->set_property("stock-size", Gtk::ICON_SIZE_BUTTON);
  pixbuf_renderer->set_property("stock-id", std::string());
  pixbuf_renderer->set_property("pixbuf", Glib::RefPtr<Gdk::Pixbuf>());

  if(iter == m_iter_existing_recent)
    pixbuf_renderer->set_property("stock-id", Gtk::StockID(Gtk::Stock::INDEX));
#ifndef G_OS_WIN32
  else if(iter == m_iter_existing_network)
    pixbuf_renderer->set_property("stock-id", Gtk::StockID(Gtk::Stock::NETWORK));
#endif
  else if(iter == m_iter_existing_other)
    pixbuf_renderer->set_property("stock-id", Gtk::StockID(Gtk::Stock::OPEN));
  else if(m_iter_existing_recent_dummy.get() && iter == *m_iter_existing_recent_dummy)
    pixbuf_renderer->set_property("stock-id", std::string()); // TODO: Use Stock::STOP instead?
#ifndef G_OS_WIN32
  else if(m_iter_existing_network_dummy.get() && iter == *m_iter_existing_network_dummy)
    pixbuf_renderer->set_property("stock-id", std::string()); // TODO: Use Stock::STOP instead?
#endif
  else
  {
    if(m_existing_model->is_ancestor(m_iter_existing_recent, iter))
    {
      //Glib::RefPtr<Gtk::RecentInfo> info = (*iter)[m_existing_columns.m_col_recent_info];
      //pixbuf_renderer->property_pixbuf() = (*info)->get_icon(Gtk::ICON_SIZE_BUTTON);
      pixbuf_renderer->set_property("icon-name", Glib::ustring("glom"));
    }
#ifndef G_OS_WIN32
    else if(m_existing_model->is_ancestor(m_iter_existing_network, iter))
    {
      //pixbuf_renderer->property_stock_id() = Gtk::Stock::CONNECT.id;
      pixbuf_renderer->set_property("icon-name", Glib::ustring("glom"));
    }
#endif
    else
    {
      throw std::logic_error("Unexpected iterator in existing_icon_data_func");
    }
  }
}

void Dialog_ExistingOrNew::existing_title_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  Gtk::CellRendererText* text_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!text_renderer)
    throw std::logic_error("Renderer not a text renderer in existing_title_data_func");

  text_renderer->property_text() = (*iter)[m_existing_columns.m_col_title];

  // Default: Use default color:
  text_renderer->property_foreground_set() = false;

  // Use grey if parent item has no children:
#ifndef G_OS_WIN32
  if( (iter == m_iter_existing_network && m_iter_existing_network_dummy.get()) ||
      (iter == m_iter_existing_recent && m_iter_existing_recent_dummy.get()))
#else
  if(iter == m_iter_existing_recent && m_iter_existing_recent_dummy.get())
#endif
  {
    text_renderer->property_foreground() = "grey";
  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Dialog_ExistingOrNew::new_icon_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  Gtk::CellRendererPixbuf* pixbuf_renderer = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
  if(!pixbuf_renderer)
    throw std::logic_error("Renderer not a pixbuf renderer in new_icon_data_func");

  pixbuf_renderer->property_stock_size() = Gtk::ICON_SIZE_BUTTON;
  pixbuf_renderer->property_stock_id() = "";
  pixbuf_renderer->property_pixbuf() = Glib::RefPtr<Gdk::Pixbuf>();

  if(iter == m_iter_new_empty)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::NEW.id;
  else if(iter == m_iter_new_template)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::EDIT.id; // TODO: More meaningful icon?
  else if(m_iter_new_template_dummy.get() && iter == *m_iter_new_template_dummy)
    pixbuf_renderer->property_stock_id() = Gtk::Stock::DIALOG_ERROR.id; // TODO: Use Stock::STOP instead?
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

void Dialog_ExistingOrNew::new_title_data_func(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter)
{
  Gtk::CellRendererText* text_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!text_renderer)
    throw std::logic_error("Renderer not a text renderer in new_title_data_func");

  text_renderer->property_text() = (*iter)[m_new_columns.m_col_title];

  // Default: Use default color
  text_renderer->property_foreground_set() = false;
  // Use grey if parent item has no children
  if( (iter == m_iter_new_template && m_iter_new_template_dummy.get()))
  {
    text_renderer->property_foreground() = "grey";
  }
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::on_switch_page(Gtk::Widget* /* page */, guint /* page_num */)
{
  update_ui_sensitivity();
}

void Dialog_ExistingOrNew::on_existing_selection_changed()
{
  update_ui_sensitivity();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Dialog_ExistingOrNew::on_new_selection_changed()
{
  update_ui_sensitivity();
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::update_ui_sensitivity()
{
  bool sensitivity = false;

  if(m_notebook->get_current_page() == 0)
  {
    const int count = m_existing_view->get_selection()->count_selected_rows();

    if(count == 0)
    {
      sensitivity = false;
    }
    else
    {
      Gtk::TreeModel::iterator sel = m_existing_view->get_selection()->get_selected();
      sensitivity = (sel != m_iter_existing_recent);
#ifndef G_OS_WIN32
      sensitivity = sensitivity && (sel != m_iter_existing_network);
#endif

      sensitivity = sensitivity && (!m_iter_existing_recent_dummy.get() || sel != *m_iter_existing_recent_dummy);
#ifndef G_OS_WIN32
      sensitivity = sensitivity && (!m_iter_existing_network_dummy.get() || sel != *m_iter_existing_network_dummy);
#endif
    }
  }
#ifndef GLOM_ENABLE_CLIENT_ONLY
  else
  {
    const int count = m_new_view->get_selection()->count_selected_rows();

    if(count == 0)
    {
      sensitivity = false;
    }
    else
    {
      Gtk::TreeModel::iterator sel = m_new_view->get_selection()->get_selected();
      sensitivity = (sel != m_iter_new_template &&
                     (!m_iter_new_template_dummy.get() || sel != *m_iter_new_template_dummy));
    }
  }
#endif //GLOM_ENABLE_CLIENT_ONLY

  m_select_button->set_sensitive(sensitivity);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
Glib::ustring Dialog_ExistingOrNew::get_title_from_example(const Glib::RefPtr<Gio::FileInfo>& info, const Glib::RefPtr<Gio::File>& examples_dir)
{
  try
  {
    // Load file.
    const Glib::ustring content_type = info->get_attribute_string(G_FILE_ATTRIBUTE_STANDARD_FAST_CONTENT_TYPE);
    const Glib::ustring mime_type = Gio::content_type_get_mime_type(content_type);

    if(mime_type == "application/x-glom")
    {
      const Glib::RefPtr<Gio::File> current_example =
        Gio::File::create_for_path(Glib::build_filename(examples_dir->get_path(), info->get_name()));
      Glib::RefPtr<Gio::FileInputStream> stream = current_example->read();
      m_current_buffer.reset(new buffer);
      const int bytes_read = stream->read(m_current_buffer->buf, buffer::SIZE);
      const std::string data(m_current_buffer->buf, bytes_read);
      // TODO: Check that data is valid UTF-8, the last character might be truncated

      Parser parser;
      return (Glib::ustring(parser.get_example_title(data)));
    }
  }
  catch(const Glib::Exception& exception)
  {
    std::cerr << "Could not enumerate files in examples directory: " << exception.what() << std::endl;

    m_current_buffer.reset();
  }

  // File is not a glom file, continue with next.
  return Glib::ustring();
}

void Dialog_ExistingOrNew::append_example(const Glib::ustring& title, const Glib::RefPtr<Gio::File>& file)
{
  if(!m_new_model)
  {
    std::cerr << G_STRFUNC << ": m_new_model is null" << std::endl;
    return;
  }

  if(!m_iter_new_template)
  {
    std::cerr << G_STRFUNC << ": m_iter_new_template is null" << std::endl;
    return;
  }

  try
  {
    const bool is_first_item = m_iter_new_template_dummy.get();

    // Add to list.
    Gtk::TreeModel::iterator iter = m_new_model->append(m_iter_new_template->children());
    (*iter)[m_new_columns.m_col_title] = title;
    (*iter)[m_new_columns.m_col_template_uri] = file->get_uri();

    if(is_first_item)
    {
      // Remove dummy.
      m_new_model->erase(*m_iter_new_template_dummy);
      m_iter_new_template_dummy.reset();
      // Expand if this is the first item.
      m_new_view->expand_row(m_new_model->get_path(m_iter_new_template), false);
    }
  }
  catch(const Glib::Exception& ex)
  {
    std::cerr << "Could not read example: " << file->get_path() << ": " << ex.what() << std::endl;
  }
}
#endif /* !GLOM_ENABLE_CLIENT_ONLY */

#ifndef G_OS_WIN32
void Dialog_ExistingOrNew::on_service_found_static(EpcServiceMonitor* /* monitor */, gchar* name, EpcServiceInfo* info, gpointer user_data)
{
  static_cast<Dialog_ExistingOrNew*>(user_data)->on_service_found(name, info);
}

void Dialog_ExistingOrNew::on_service_removed_static(EpcServiceMonitor* /* monitor */, gchar* name, gchar* type, gpointer user_data)
{
  static_cast<Dialog_ExistingOrNew*>(user_data)->on_service_removed(name, type);
}

void Dialog_ExistingOrNew::on_service_found(const Glib::ustring& name, EpcServiceInfo* info)
{
  //Translator hint: This is <Service Name> on <Host> (via Network Interface such as eth0).
  gchar* title = g_strdup_printf(_("%s on %s (via %s)"), name.c_str(), epc_service_info_get_host(info), epc_service_info_get_interface(info));
  Gtk::TreeModel::iterator iter = m_existing_model->prepend(m_iter_existing_network->children());
  (*iter)[m_existing_columns.m_col_title] = title;
  (*iter)[m_existing_columns.m_col_time] = std::time(0); /* sort more recently discovered items above */
  (*iter)[m_existing_columns.m_col_service_name] = name;
  (*iter)[m_existing_columns.m_col_service_info] = info;

  epc_service_info_ref(info);
  g_free(title);

  // Remove dummy item
  if(m_iter_existing_network_dummy.get())
  {
    m_existing_model->erase(*m_iter_existing_network_dummy);
    m_iter_existing_network_dummy.reset();
  }
}

void Dialog_ExistingOrNew::on_service_removed(const Glib::ustring& name, const Glib::ustring& /* type */)
{
  // Find the entry with the given name
  const Gtk::TreeNodeChildren& children = m_iter_existing_network->children();
  for(Gtk::TreeModel::iterator iter = children.begin(); iter != children.end(); ++ iter)
  {
    if((*iter)[m_existing_columns.m_col_service_name] == name)
    {
      const bool was_expanded = m_existing_view->row_expanded(m_existing_model->get_path(iter));
      // Remove from store
      epc_service_info_unref((*iter)[m_existing_columns.m_col_service_info]);
      m_existing_model->erase(iter);
      // Reinsert dummy, if necessary
      if(children.begin() == children.end())
        m_iter_existing_network_dummy = create_dummy_item_existing(m_iter_existing_network, _(NETWORK_DUMMY_TEXT));
      if(was_expanded) m_existing_view->expand_row(m_existing_model->get_path(iter), false);
      break;
    }
  }
}
#endif // !G_OS_WIN32

void Dialog_ExistingOrNew::on_existing_row_activated(const Gtk::TreeModel::Path& /* path */, Gtk::TreeViewColumn* /* column */)
{
  if(m_select_button->is_sensitive())
    on_select_clicked();
}

void Dialog_ExistingOrNew::on_existing_button_clicked(const Gtk::TreeModel::Path& path)
{
  m_existing_view->get_selection()->select(path);

  if(m_select_button->is_sensitive())
    on_select_clicked();
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void Dialog_ExistingOrNew::on_new_row_activated(const Gtk::TreeModel::Path& /* path */, Gtk::TreeViewColumn* /* column */)
{
  if(m_select_button->is_sensitive())
    on_select_clicked();
}

void Dialog_ExistingOrNew::on_new_button_clicked(const Gtk::TreeModel::Path& path)
{
  m_new_view->get_selection()->select(path);

  if(m_select_button->is_sensitive())
    on_select_clicked();
}
#endif //GLOM_ENABLE_CLIENT_ONLY

void Dialog_ExistingOrNew::on_select_clicked()
{
  Gtk::TreeModel::iterator iter;
  Action action = get_action_impl(iter);

  if(action == OPEN_URI && iter == m_iter_existing_other)
  {
    #ifdef GLOM_ENABLE_MAEMO
    Hildon::FileChooserDialog dialog(Gtk::FILE_CHOOSER_ACTION_OPEN);
    #else
    Gtk::FileChooserDialog dialog(*this, "Open Glom File");
    dialog.add_button(Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);
    dialog.add_button(Gtk::Stock::OPEN, Gtk::RESPONSE_OK);
    dialog.set_default_response(Gtk::RESPONSE_OK);
    #endif // GLOM_ENABLE_MAEMO

    Gtk::FileFilter filter;
    filter.add_mime_type("application/x-glom");
    filter.set_name("Glom files");
    dialog.add_filter(filter);

    const int response_id = dialog.run();
    if(response_id == Gtk::RESPONSE_OK)
    {
      m_chosen_uri = dialog.get_uri();
      response(Gtk::RESPONSE_ACCEPT);
    }
  }
  else
  {
    response(Gtk::RESPONSE_ACCEPT);
  }
}


} //namespace Glom
