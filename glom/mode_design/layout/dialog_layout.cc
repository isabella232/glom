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

#include <glom/mode_design/layout/dialog_layout.h>
//#include <libgnome/gnome-i18n.h>

namespace Glom
{

Dialog_Layout::Dialog_Layout(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder, bool with_table_title)
: Gtk::Dialog(cobject),
  m_entry_table_title(nullptr),
  m_label_table_title(nullptr),
  m_editable_layout(true),
  m_modified(false)
{
  Gtk::Button* button = nullptr;
  builder->get_widget("button_close", button);
  button->signal_clicked().connect( sigc::mem_fun(*this, &Dialog_Layout::on_button_close) );

  if(with_table_title)
  {
    builder->get_widget("entry_table_title", m_entry_table_title);
    m_entry_table_title->signal_changed().connect( sigc::mem_fun(*this, &Dialog_Layout::on_entry_table_title_changed) );

    builder->get_widget("label_title", m_label_table_title);
  }
}

void Dialog_Layout::init(const Glib::ustring& layout_name, const Glib::ustring& layout_platform, const std::shared_ptr<Document>& /* document */, const Glib::ustring& table_name, const type_vecConstLayoutFields& /* table_fields */)
{
  m_modified = false;

  m_layout_name = layout_name;
  m_layout_platform = layout_platform;
  m_table_name = table_name;
}

void Dialog_Layout::move_treeview_selection_up(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column)
{
  auto refSelection = treeview->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      auto model = treeview->get_model();
      if(iter != model->children().begin()) //If it is not the first one.
      {
        auto iterBefore = iter;
        --iterBefore;

        auto row = *iter;
        auto rowBefore = *iterBefore;

        //Swap the sequence values, so that the one before will be after:
        guint tempBefore = rowBefore[sequence_column];
        guint tempRow = row[sequence_column];
        rowBefore[sequence_column] = tempRow;
        row[sequence_column] = tempBefore;

        //Because the model is sorted, the visual order should now be swapped.

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout::move_treeview_selection_down(Gtk::TreeView* treeview, const Gtk::TreeModelColumn<guint>& sequence_column)
{
  auto refSelection = treeview->get_selection();
  if(refSelection)
  {
    auto iter = refSelection->get_selected();
    if(iter)
    {
      auto iterNext = iter;
      iterNext++;

      auto model = treeview->get_model();
      if(iterNext != model->children().end()) //If it is not the last one.
      {
        auto row = *iter;
        auto rowNext = *iterNext;

        //Swap the sequence values, so that the one before will be after:
        guint tempNext = rowNext[sequence_column];
        guint tempRow = row[sequence_column];
        rowNext[sequence_column] = tempRow;
        row[sequence_column] = tempNext;

        //Because the model is sorted, the visual order should now be swapped.

        m_modified = true;
      }
    }

  }

  enable_buttons();
}

void Dialog_Layout::on_button_close()
{
  save_to_document();

  hide();
}

void Dialog_Layout::save_to_document()
{

}


void Dialog_Layout::treeview_fill_sequences(const Glib::RefPtr<Gtk::TreeModel> model, const Gtk::TreeModelColumn<guint>& sequence_column)
{
   //Get the highest sequence number:
  guint max_sequence = 1; //0 means no sequence.
  for(const auto& row : model->children())
  {
    guint sequence = row[sequence_column];
    max_sequence = MAX(max_sequence, sequence);
  }

  //Add sequences to any that don't have a sequence:
  //(0 means no sequence)
  guint next_sequence = max_sequence+1; //This could leave holes, of course. But we want new groups to be after the old groups. We can compact it later.
  for(auto& row : model->children())
  {
    guint sequence = row[sequence_column];
    if(sequence == 0)
    {
      row[sequence_column] = next_sequence;
      ++next_sequence;

    }
  }

}


void Dialog_Layout::on_treemodel_row_changed(const Gtk::TreeModel::Path& /* path */, const Gtk::TreeModel::iterator& /* iter */)
{
  m_modified = true;
}

void Dialog_Layout::on_entry_table_title_changed()
{
  m_modified = true;
}

void Dialog_Layout::enable_buttons()
{
}

bool Dialog_Layout::get_modified() const
{
  return m_modified;
}

void Dialog_Layout::make_sensitivity_depend_on_toggle_button(Gtk::ToggleButton& toggle_button, Gtk::Widget& widget)
{
  toggle_button.signal_toggled().connect(
    sigc::bind( sigc::mem_fun(*this, &Dialog_Layout::on_sensitivity_toggle_button), &toggle_button, &widget) );

  //Call the handler once, so that the initial state is set:
  on_sensitivity_toggle_button(&toggle_button, &widget);
}

void Dialog_Layout::on_sensitivity_toggle_button(Gtk::ToggleButton* toggle_button, Gtk::Widget* widget)
{
  if(!toggle_button || !widget)
    return;

  const auto sensitivity = toggle_button->get_active();
  widget->set_sensitive(sensitivity);

  m_modified = true;
}

} //namespace Glom
