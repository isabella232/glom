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

#ifndef GLOM_TRANSLATIONS_DIALOG_TRANSLATIONS_H
#define GLOM_TRANSLATIONS_DIALOG_TRANSLATIONS_H

#include <glom/libglom/document/document_glom.h>
#include <gtkmm.h>
#include <libglademm.h>

class ComboBox_Locale;

class Window_Translations
: public Gtk::Window,
  public View_Composite_Glom //So it can use the document.
{
public:
  Window_Translations(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade);
  virtual ~Window_Translations();

  virtual void load_from_document(); //override
  virtual void save_to_document(); //override

protected:

  //Enable/disable buttons, depending on treeview selection:
  virtual void enable_buttons();

  //signal handlers:
  void on_button_identify();
  void on_cell_data_original(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  void on_cell_data_item_typename(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter);
  void on_treeview_edited(const Glib::ustring& path, const Glib::ustring& new_text);

  void on_combo_target_locale_changed();

  void on_button_cancel();
  void on_button_ok();
  void on_button_copy_translation();

  //Tree model columns:
  class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_item); add(m_col_translation); add(m_col_parent_table); }

    Gtk::TreeModelColumn< sharedptr<TranslatableItem> > m_col_item; //The table name, field name, etc.
    Gtk::TreeModelColumn<Glib::ustring> m_col_translation;
    Gtk::TreeModelColumn<Glib::ustring> m_col_parent_table; //Not shown.
  };

  ModelColumns m_columns;

  //Tree model columns:
  Gtk::TreeView* m_treeview;
  Gtk::Button* m_button_identify;
  ComboBox_Locale* m_combo_target_locale;

  Glib::RefPtr<Gtk::ListStore> m_model;

  Gtk::Label* m_label_source_locale;
  Glib::ustring m_translation_locale;

  Gtk::Button* m_button_ok;
  Gtk::Button* m_button_cancel;
  Gtk::Button* m_button_copy_translation;

  bool m_treeview_modified;
};

#endif //GLOM_TRANSLATIONS_DIALOG_TRANSLATIONS_H
