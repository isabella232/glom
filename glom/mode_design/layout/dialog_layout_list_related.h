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

#ifndef GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_RELATED_H
#define GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_RELATED_H

#include <glom/mode_design/layout/dialog_layout_list.h>
#include <glom/utility_widgets/combo_textglade.h>
#include <glom/mode_design/layout/combobox_relationship.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>
#include <gtkmm/colorbutton.h>

namespace Glom
{

class Dialog_Layout_List_Related : public Dialog_Layout_List
{
public:
  static const char* glade_id;
  static const bool glade_developer;
  
  Dialog_Layout_List_Related(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Layout_List_Related();

  /**
   * @param layout_name "list" or "details"
   * @param layout_platform As in the document. Empty or "maemo".
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param portal The layout item, which knows its from_table, for instance.
   */
  virtual void set_document(const Glib::ustring& layout_name, const Glib::ustring& layout_platform, Document* document, const sharedptr<const LayoutItem_Portal>& portal, const Glib::ustring& from_table);

  virtual void update_ui(bool including_relationships_list = true);

  sharedptr<Relationship> get_relationship() const;
  sharedptr<LayoutItem_Portal>  get_portal_layout();

protected:

  virtual void save_to_document();

  ///@inheritdoc
  virtual Glib::ustring get_fields_table() const;

  //signal handlers:
  virtual void on_button_add_field(); //override
  virtual void on_button_edit(); //override
 
  void on_combo_relationship_changed();
  
  void on_combo_navigation_specific_changed();
  void on_checkbutton_show_child_relationships();


  ComboBox_Relationship* m_combo_relationship;
  Gtk::CheckButton* m_checkbutton_show_child_relationships;
  sharedptr<LayoutItem_Portal> m_portal;

  Gtk::RadioButton* m_radio_navigation_automatic;
  Gtk::RadioButton* m_radio_navigation_none;
  Gtk::RadioButton* m_radio_navigation_specify;
  Gtk::Label* m_label_navigation_automatic;
  ComboBox_Relationship* m_combo_navigation_specify;
};

} //namespace Glom

#endif //GLOM_MODE_DATA_DIALOG_LAYOUT_LIST_RELATED_H
