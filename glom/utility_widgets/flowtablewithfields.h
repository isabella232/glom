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

#ifndef GLOM_UTILITYWIDGETS_FLOWTABLEWITHFIELDS_H
#define GLOM_UTILITYWIDGETS_FLOWTABLEWITHFIELDS_H

#include "flowtable.h"
#include "../data_structure/layout/layoutgroup.h"
#include "../data_structure/layout/layoutitem_field.h"
#include "../data_structure/layout/layoutitem_portal.h"
#include "../data_structure/field.h"
#include "../document/document_glom.h"
#include <map>
#include <list>

class DataWidget;

class FlowTableWithFields
  : public FlowTable,
    public View_Composite_Glom
{
public: 
  FlowTableWithFields(const Glib::ustring& table_name = Glib::ustring());
  virtual ~FlowTableWithFields();

  ///The table name is needed to discover details of relationships.
  virtual void set_table(const Glib::ustring& table_name);

  /** Add a field.
   * @param title The title to show to the left of the entry.
   * @param id The unique identifier for this field, to use with get_field().
   * @param group The title of the group in which this field should be shown, if any.
   */
  virtual void add_field(const Field& field, const Glib::ustring& group = Glib::ustring());
  virtual void remove_field(const Glib::ustring& id);

  typedef std::map<int, Field> type_map_field_sequence;
  virtual void add_group(const Glib::ustring& group_name, const Glib::ustring& group_title, const type_map_field_sequence& fields);

  virtual void add_layout_item(const LayoutItem& group);
  virtual void add_layout_group(const LayoutGroup& group);
  

  virtual void set_field_editable(const Field& field, bool editable = true);

  virtual Gnome::Gda::Value get_field_value(const Field& field) const;
  virtual Gnome::Gda::Value get_field_value(const Glib::ustring& id) const;
  virtual void set_field_value(const Field& field, const Gnome::Gda::Value& value);
  virtual void set_field_value(const Glib::ustring& id, const Gnome::Gda::Value& value);


  typedef std::list<Gtk::Widget*> type_list_widgets;
  typedef std::list<const Gtk::Widget*> type_list_const_widgets;
      
  virtual void change_group(const Glib::ustring& id, const Glib::ustring& new_group);

  virtual void set_design_mode(bool value = true);
  
  virtual void remove_all();

  /** For instance,
   * void on_flowtable_field_edited(const Glib::ustring& id, const Gnome::Gda::Value& value);
   */
  typedef sigc::signal<void, const Glib::ustring&, const Gnome::Gda::Value&> type_signal_field_edited;
  type_signal_field_edited signal_field_edited();


protected:

  virtual type_list_widgets get_field(const Glib::ustring& id);
  virtual type_list_const_widgets get_field(const Glib::ustring& id) const;

  virtual type_list_widgets get_field(const Field& field);
  virtual type_list_const_widgets get_field(const Field& field) const;

  ///Get portals whose relationships have @a from_key as the from_key.
  virtual type_list_widgets get_portals(const Glib::ustring& from_key);

  int get_suitable_width(Field::glom_field_type field_type);
  void on_entry_edited(const Gnome::Gda::Value& value, Glib::ustring id);
  void on_flowtable_entry_edited(const Glib::ustring& id, const Gnome::Gda::Value& value);
    
  class Info
  {
  public:
    Info();
    
    Field m_field; //Store the field information so we know the title, ID, and type.
    Glib::ustring m_group;

    Gtk::Alignment* m_first;
    DataWidget* m_second;
    Gtk::CheckButton* m_checkbutton; //Used instead of first and second if it's a bool.
  };

  typedef std::map<Glib::ustring, Info> type_mapFields; //Map of IDs to full info.
  type_mapFields m_mapFields;

  //Remember the nested FlowTables, so that we can search them for fields too:
  typedef std::list< FlowTableWithFields* > type_sub_flow_tables;
  type_sub_flow_tables m_sub_flow_tables;

  Glib::ustring m_table_name;

  type_signal_field_edited m_signal_field_edited;
};


#endif //GLOM_UTILITYWIDGETS_FLOWTABLEWITHFIELDS_H
