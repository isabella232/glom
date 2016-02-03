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

#ifndef GLOM_MODE_DESIGN_DIALOG_CHOOSE_RELATIONSHIP_H
#define GLOM_MODE_DESIGN_DIALOG_CHOOSE_RELATIONSHIP_H

#include <gtkmm/dialog.h>
#include <glom/utility_widgets/dialog_properties.h>
#include <libglom/document/document.h>
#include <glom/box_withbuttons.h>

namespace Glom
{

class Dialog_ChooseRelationship : public Gtk::Dialog
{
public:
  static const char* glade_id;
  static const bool glade_developer;
	
  Dialog_ChooseRelationship();
  Dialog_ChooseRelationship(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);

  /**
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   */
  void set_document(const std::shared_ptr<Document>& document, const Glib::ustring& table_name);

  void select_item(const std::shared_ptr<const Relationship>& relationship);

  std::shared_ptr<Relationship> get_relationship_chosen() const;

private:

  //Tree model columns:
  class ModelColumns_Relationships : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns_Relationships()
    { add(m_col_name); /* add(m_col_title); */ add(m_col_relationship); }

    Gtk::TreeModelColumn<Glib::ustring> m_col_name;
    //Gtk::TreeModelColumn<Glib::ustring> m_col_title;
    Gtk::TreeModelColumn< std::shared_ptr<Relationship> > m_col_relationship;
  };

  ModelColumns_Relationships m_ColumnsRelationships;

  Gtk::Label* m_label_table_name;
  Gtk::Button* m_button_select;
  Gtk::TreeView* m_treeview;
  Glib::RefPtr<Gtk::ListStore> m_model;

  Glib::ustring m_table_name;

  std::shared_ptr<Document> m_document;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_CHOOSE_RELATIONSHIP_H
