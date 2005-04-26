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
#include "../mode_data/box_data_list_related.h"
#include "layoutwidgetbase.h"
#include "../mode_data/treestore_layout.h" //Forthe enum.
#include <map>
#include <list>

class DataWidget;

class FlowTableWithFields
  : public FlowTable,
    public View_Composite_Glom,
    public LayoutWidgetBase
{
public: 
  FlowTableWithFields(const Glib::ustring& table_name = Glib::ustring());
  virtual ~FlowTableWithFields();

  ///The table name is needed to discover details of relationships.
  virtual void set_table(const Glib::ustring& table_name);

  /** Add a field.
   * @param layoutitem_field The layout item that describes this field,
   * @param table_name The table on which this layout appears.
   */
  virtual void add_field(const LayoutItem_Field& layoutitem_field, const Glib::ustring& table_name);
  virtual void remove_field(const Glib::ustring& id);

  typedef std::map<int, Field> type_map_field_sequence;
  //virtual void add_group(const Glib::ustring& group_name, const Glib::ustring& group_title, const type_map_field_sequence& fields);

  virtual void add_layout_item(const LayoutItem& group);
  virtual void add_layout_group(const LayoutGroup& group);

  virtual void set_field_editable(const LayoutItem_Field& field, bool editable = true);

  virtual Gnome::Gda::Value get_field_value(const LayoutItem_Field& field) const;
  //virtual Gnome::Gda::Value get_field_value(const Glib::ustring& id) const;
  virtual void set_field_value(const LayoutItem_Field& field, const Gnome::Gda::Value& value);
  //virtual void set_field_value(const Glib::ustring& id, const Gnome::Gda::Value& value);


  typedef std::list<Gtk::Widget*> type_list_widgets;
  typedef std::list<const Gtk::Widget*> type_list_const_widgets;

  virtual void change_group(const Glib::ustring& id, const Glib::ustring& new_group);

  virtual void set_design_mode(bool value = true);

  virtual void remove_all();

  /** Get the layout structure, which might have changed in the child widgets since 
   * the whole widget structure was built.
   * for instance, if the user chose a new field for a DataWidget, 
   * or a new relationship for a portal.
   */
  virtual void get_layout_groups(Document_Glom::type_mapLayoutGroupSequence& groups);
  virtual void get_layout_group(LayoutGroup& group);

  /** For instance,
   * void on_flowtable_field_edited(const LayoutItem_Field& field, const Gnome::Gda::Value& value);
   */
  typedef sigc::signal<void, const LayoutItem_Field&, const Gnome::Gda::Value&> type_signal_field_edited;
  type_signal_field_edited signal_field_edited();

 /** For instance,
   * void on_related_record_changed(const Glib::ustring& relationship_name);
   */
  typedef sigc::signal<void, const Glib::ustring&> type_signal_related_record_changed;
  type_signal_related_record_changed signal_related_record_changed();


protected:

  virtual type_list_widgets get_field(const LayoutItem_Field& field);
  virtual type_list_const_widgets get_field(const LayoutItem_Field& field) const;

  ///Get portals whose relationships have @a from_key as the from_key.
  virtual type_list_widgets get_portals(const LayoutItem_Field& from_key);

  //int get_suitable_width(Field::glom_field_type field_type);

  void on_entry_edited(const Gnome::Gda::Value& value,  LayoutItem_Field field);
  void on_flowtable_entry_edited(const LayoutItem_Field& field, const Gnome::Gda::Value& value);

  void on_portal_record_changed(const Glib::ustring& relationship_name);
  void on_flowtable_related_record_changed(const Glib::ustring& relationship_name);

  /// Remember the layout widget so we can iterate through them later.
  void on_layoutwidget_changed();

  void on_datawidget_layout_item_added(TreeStore_Layout::enumType item_type, DataWidget* pDataWidget);

  class Info
  {
  public:
    Info();

    LayoutItem_Field m_field; //Store the field information so we know the title, ID, and type.
    Glib::ustring m_group;

    Gtk::Alignment* m_first;
    DataWidget* m_second;
    Gtk::CheckButton* m_checkbutton; //Used instead of first and second if it's a bool.
  };

  typedef std::list<Info> type_listFields; //Map of IDs to full info.
  type_listFields m_listFields;

  //Remember the nested FlowTables, so that we can search them for fields too:
  typedef std::list< FlowTableWithFields* > type_sub_flow_tables;
  type_sub_flow_tables m_sub_flow_tables;

  typedef std::list< Box_Data_List_Related* > type_portals;
  type_portals m_portals;

  //Remember the sequence of LayoutWidgetBase widgets, so we can iterate over them later:
  typedef std::list< LayoutWidgetBase* > type_list_layoutwidgets;
  type_list_layoutwidgets m_list_layoutwidgets;

  void add_field_at_position(const LayoutItem_Field& layoutitem_field, const Glib::ustring& table_name, const type_list_layoutwidgets::iterator& add_before);
  void add_layoutwidgetbase(LayoutWidgetBase* layout_widget);
  void add_layoutwidgetbase(LayoutWidgetBase* layout_widget, const type_list_layoutwidgets::iterator& add_before);
  void add_layout_item_at_position(const LayoutItem& item, const type_list_layoutwidgets::iterator& add_before);
  void add_layout_group_at_position(const LayoutGroup& group, const type_list_layoutwidgets::iterator& add_before);

  Glib::ustring m_table_name;

  type_signal_field_edited m_signal_field_edited;

  //type_signal_related_record_added m_signal_related_record_added;
  type_signal_related_record_changed m_signal_related_record_changed;
};


#endif //GLOM_UTILITYWIDGETS_FLOWTABLEWITHFIELDS_H
