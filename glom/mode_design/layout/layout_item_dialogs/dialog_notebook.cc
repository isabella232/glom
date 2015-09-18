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

#include "dialog_notebook.h"
#include <glom/mode_design/layout/dialog_layout.h>
#include <glom/appwindow.h>

//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

const char* Dialog_Notebook::glade_id("dialog_notebook");
const bool Dialog_Notebook::glade_developer(true);

Dialog_Notebook::Dialog_Notebook(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Layout(cobject, builder, false /* means no table title */),
  m_treeview(nullptr),
  m_button_up(nullptr),
  m_button_down(nullptr),
  m_button_add(nullptr),
  m_button_delete(nullptr)
{
  builder->get_widget("treeview", m_treeview);
  if(m_treeview)
  {
    m_model = Gtk::ListStore::create(m_ColumnsTabs);
    m_treeview->set_model(m_model);

    // Append the View columns:
    m_treeview->append_column_editable(_("Name"), m_ColumnsTabs.m_col_name);
    m_treeview->append_column_editable(_("Title"), m_ColumnsTabs.m_col_title);


    //Sort by sequence, so we can change the order by changing the values in the hidden sequence column.
    m_model->set_sort_column(m_ColumnsTabs.m_col_sequence, Gtk::SORT_ASCENDING);


    //Respond to changes of selection:
    Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview->get_selection();
    if(refSelection)
    {
      refSelection->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Notebook::on_treeview_selection_changed) );
    }

    m_model->signal_row_changed().connect( sigc::mem_fun(*this, &Dialog_Notebook::on_treemodel_row_changed) );
  }


  builder->get_widget("button_up", m_button_up);
  m_button_up->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Notebook::on_button_up) );

  builder->get_widget("button_down", m_button_down);
  m_button_down->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Notebook::on_button_down) );

  builder->get_widget("button_delete", m_button_delete);
  m_button_delete->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Notebook::on_button_delete) );

  builder->get_widget("button_add", m_button_add);
  m_button_add->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Notebook::on_button_add) );

  show_all_children();
}

Dialog_Notebook::~Dialog_Notebook()
{
}

void Dialog_Notebook::set_notebook(const std::shared_ptr<const LayoutItem_Notebook>& start_notebook)
{
  m_layout_item = start_notebook; //So we can preserve information for later.
  m_modified = false;

  //auto document = get_document();

  guint sequence = 1;
  for(const auto& base_item : start_notebook->m_list_items)
  {
    std::shared_ptr<const LayoutGroup> item = std::dynamic_pointer_cast<const LayoutGroup>(base_item);
    if(item)
    {
      auto iterTree = m_model->append();
      Gtk::TreeModel::Row row = *iterTree;

      row[m_ColumnsTabs.m_col_item] = glom_sharedptr_clone(item);
      row[m_ColumnsTabs.m_col_name] = item->get_name();
      row[m_ColumnsTabs.m_col_title] = item_get_title(item);
      row[m_ColumnsTabs.m_col_sequence] = sequence;
      ++sequence;
  }

    //treeview_fill_sequences(m_model, m_ColumnsTabs.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  m_modified = false;
}

void Dialog_Notebook::enable_buttons()
{
  //Fields:
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_treeview->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      //Disable Up if It can't go any higher.
      bool enable_up = true;
      if(iter == m_model->children().begin())
        enable_up = false;  //It can't go any higher.

      m_button_up->set_sensitive(enable_up);


      //Disable Down if It can't go any lower.
      auto iterNext = iter;
      iterNext++;

      bool enable_down = true;
      if(iterNext == m_model->children().end())
        enable_down = false;

      m_button_down->set_sensitive(enable_down);

      m_button_delete->set_sensitive(true);
    }
    else
    {
      //Disable all buttons that act on a selection:
      m_button_down->set_sensitive(false);
      m_button_up->set_sensitive(false);
      m_button_delete->set_sensitive(false);
    }
  }


}


void Dialog_Notebook::on_button_up()
{
  move_treeview_selection_up(m_treeview, m_ColumnsTabs.m_col_sequence);
}

void Dialog_Notebook::on_button_down()
{
  move_treeview_selection_down(m_treeview, m_ColumnsTabs.m_col_sequence);
}

std::shared_ptr<LayoutItem_Notebook> Dialog_Notebook::get_notebook() const
{
  std::shared_ptr<LayoutItem_Notebook> result = glom_sharedptr_clone(m_layout_item);
  result->remove_all_items();

  for(const auto& row : m_model->children())
  {
    const Glib::ustring name = row[m_ColumnsTabs.m_col_name];
    if(!name.empty())
    {
      std::shared_ptr<LayoutGroup> item = row[m_ColumnsTabs.m_col_item];
      std::shared_ptr<LayoutGroup> group_copy;
      if(item)
        group_copy = glom_sharedptr_clone(item);
      else
        group_copy = std::make_shared<LayoutGroup>();

      group_copy->set_name(name);
      group_copy->set_title( row[m_ColumnsTabs.m_col_title] , AppWindow::get_current_locale());
      //group_copy->set_sequence( row[m_ColumnsTabs.m_col_sequence] );

      result->add_item(group_copy);
    }
  }

  return result;
}

void Dialog_Notebook::on_treeview_selection_changed()
{
  enable_buttons();
}

void Dialog_Notebook::on_button_add()
{
  //Add the field details to the layout treeview:
  auto iter =  m_model->append();

  if(iter)
  {
    //Scroll to, and select, the new row:
    Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview->get_selection();
    if(refTreeSelection)
      refTreeSelection->select(iter);

    m_treeview->scroll_to_row( Gtk::TreeModel::Path(iter) );

    treeview_fill_sequences(m_model, m_ColumnsTabs.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }
}

void Dialog_Notebook::on_button_delete()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      m_model->erase(iter);

      m_modified = true;
    }
  }
}

} //namespace Glom


