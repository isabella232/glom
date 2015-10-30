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

#ifndef GLOM_MODE_DESIGN_DIALOG_LAYOUT_CALENDAR_RELATED_H
#define GLOM_MODE_DESIGN_DIALOG_LAYOUT_CALENDAR_RELATED_H

#include <glom/mode_design/layout/dialog_layout_list.h>
#include <libglom/data_structure/layout/layoutitem_calendarportal.h>
#include <glom/mode_design/layout/combobox_relationship.h>
#include <glom/mode_design/layout/combobox_fields.h>
#include <gtkmm/checkbutton.h>
#include <gtkmm/radiobutton.h>

namespace Glom
{

class Dialog_Layout_Calendar_Related : public Dialog_Layout_List
{
public:
  static const char* glade_id;
  static const bool glade_developer;
  
  Dialog_Layout_Calendar_Related(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder);
  virtual ~Dialog_Layout_Calendar_Related();

  /**
   * @param layout "list" or "details"
   * @param document The document, so that the dialog can load the previous layout, and save changes.
   * @param table_name The table name.
   * @param table_fields: The actual fields in the table, in case the document does not yet know about them all.
   */
  void init_with_portal(const Glib::ustring& layout, const Glib::ustring& layout_platform, Document* document, const std::shared_ptr<const LayoutItem_CalendarPortal>& portal);

  void init_with_tablename(const Glib::ustring& layout, const Glib::ustring& layout_platform, Document* document, const Glib::ustring& parent_table);

  void update_ui(bool including_relationships_list = true);

  std::shared_ptr<Relationship> get_relationship() const;
  std::shared_ptr<LayoutItem_CalendarPortal>  get_portal_layout();

private:

  void save_to_document() override;

  //signal handlers:
  void on_button_add_field() override;
  void on_button_edit() override;
 
  void on_combo_relationship_changed();
  
  void on_combo_navigation_specific_changed();
  void on_combo_date_field_changed();
  void on_checkbutton_show_child_relationships();


  ComboBox_Relationship* m_combo_relationship;
  Gtk::CheckButton* m_checkbutton_show_child_relationships;
  std::shared_ptr<LayoutItem_CalendarPortal> m_portal;

  Gtk::RadioButton* m_radio_navigation_automatic;
  Gtk::RadioButton* m_radio_navigation_specify;
  Gtk::Label* m_label_navigation_automatic;
  ComboBox_Relationship* m_combo_navigation_specify;
    
  ComboBox_Fields* m_combobox_date_field;
};

} //namespace Glom

#endif // GLOM_MODE_DESIGN_DIALOG_LAYOUT_CALENDAR_RELATED_H
