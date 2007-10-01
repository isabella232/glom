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
#include <glom/libglom/data_structure/layout/layoutgroup.h>
#include <glom/libglom/data_structure/layout/layoutitem_field.h>
#include <glom/libglom/data_structure/layout/layoutitem_notebook.h>
#include <glom/libglom/data_structure/layout/layoutitem_portal.h>
#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <glom/libglom/data_structure/layout/layoutitem_text.h>
#include <glom/libglom/data_structure/field.h>
#include <glom/libglom/document/document_glom.h>
#include "../mode_data/box_data_list_related.h"
#include "layoutwidgetbase.h"
#include "../mode_data/treestore_layout.h" //Forthe enum.
#include <map>
#include <list>

namespace Glom
{

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
  virtual void add_field(const sharedptr<LayoutItem_Field>& layoutitem_field, const Glib::ustring& table_name);
  virtual void remove_field(const Glib::ustring& id);

  typedef std::map<int, Field> type_map_field_sequence;
  //virtual void add_group(const Glib::ustring& group_name, const Glib::ustring& group_title, const type_map_field_sequence& fields);

  virtual void add_layout_item(const sharedptr<LayoutItem>& item);
  virtual void add_layout_group(const sharedptr<LayoutGroup>& group);

  virtual void set_field_editable(const sharedptr<const LayoutItem_Field>& field, bool editable = true);

  virtual Gnome::Gda::Value get_field_value(const sharedptr<const LayoutItem_Field>& field) const;
  //virtual Gnome::Gda::Value get_field_value(const Glib::ustring& id) const;
  virtual void set_field_value(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
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
  //void get_layout_groups(Document_Glom::type_mapLayoutGroupSequence& groups);
  //sharedptr<LayoutGroup> get_layout_group();

  /** For instance,
   * void on_flowtable_field_edited(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
   */
  typedef sigc::signal<void, const sharedptr<const LayoutItem_Field>&, const Gnome::Gda::Value&> type_signal_field_edited;
  type_signal_field_edited signal_field_edited();
  
  /** For instance,
   * void on_flowtable_field_open_details_requested(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
   */
  typedef sigc::signal<void, const sharedptr<const LayoutItem_Field>&, const Gnome::Gda::Value&> type_signal_field_open_details_requested;
  type_signal_field_open_details_requested signal_field_open_details_requested();

  /** For instance,
   * void on_related_record_changed(const Glib::ustring& relationship_name);
   */
  typedef sigc::signal<void, const Glib::ustring&> type_signal_related_record_changed;
  type_signal_related_record_changed signal_related_record_changed();

  /** For instance,
   * void on_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);
   */
  typedef sigc::signal<void, const Glib::ustring&, Gnome::Gda::Value> type_signal_requested_related_details;
  type_signal_requested_related_details signal_requested_related_details();

 /** For instance,
   * void on_script_button_clicked(const sharedptr<LayoutItem_Button>& layout_item>);
   */
  typedef sigc::signal<void, const sharedptr<LayoutItem_Button>&> type_signal_script_button_clicked;
  type_signal_script_button_clicked signal_script_button_clicked();

protected:

  virtual type_list_widgets get_field(const sharedptr<const LayoutItem_Field>& field);
  virtual type_list_const_widgets get_field(const sharedptr<const LayoutItem_Field>& field) const;

  ///Get portals whose relationships have @a from_key as the from_key.
  virtual type_list_widgets get_portals(const sharedptr<const LayoutItem_Field>& from_key);

  //int get_suitable_width(Field::glom_field_type field_type);

  void on_entry_edited(const Gnome::Gda::Value& value, sharedptr<const LayoutItem_Field> field);
  void on_entry_open_details_requested(const Gnome::Gda::Value& value, sharedptr<const LayoutItem_Field> field);
  void on_flowtable_entry_edited(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
  void on_flowtable_entry_open_details_requested(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value); 

  void on_portal_record_changed(const Glib::ustring& relationship_name);
  void on_flowtable_related_record_changed(const Glib::ustring& relationship_name);
  void on_flowtable_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);

  void on_script_button_clicked(const sharedptr<LayoutItem_Button>& layout_item);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  /// Remember the layout widget so we can iterate through them later.
  void on_layoutwidget_changed();
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_datawidget_layout_item_added(LayoutWidgetBase::enumType item_type, DataWidget* pDataWidget);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void on_portal_user_requested_details(Gnome::Gda::Value primary_key_value, Box_Data_List_Related* portal_box);

  class Info
  {
  public:
    Info();

    sharedptr<const LayoutItem_Field> m_field; //Store the field information so we know the title, ID, and type.
    Glib::ustring m_group;

    Gtk::Label* m_first;
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

  void add_field_at_position(const sharedptr<LayoutItem_Field>& layoutitem_field, const Glib::ustring& table_name, const type_list_layoutwidgets::iterator& add_before);
  void add_button_at_position(const sharedptr<LayoutItem_Button>& layoutitem_button, const Glib::ustring& table_name, const type_list_layoutwidgets::iterator& add_before);
  void add_textobject_at_position(const sharedptr<LayoutItem_Text>& layoutitem_text, const Glib::ustring& table_name, const type_list_layoutwidgets::iterator& add_before);
  void add_imageobject_at_position(const sharedptr<LayoutItem_Image>& layoutitem_image, const Glib::ustring& table_name, const type_list_layoutwidgets::iterator& add_before);

  void add_layoutwidgetbase(LayoutWidgetBase* layout_widget);
  void add_layoutwidgetbase(LayoutWidgetBase* layout_widget, const type_list_layoutwidgets::iterator& add_before);
  void add_layout_item_at_position(const sharedptr<LayoutItem>& item, const type_list_layoutwidgets::iterator& add_before);
  void add_layout_group_at_position(const sharedptr<LayoutGroup>& group, const type_list_layoutwidgets::iterator& add_before);
  void add_layout_notebook_at_position(const sharedptr<LayoutItem_Notebook>& notebook, const type_list_layoutwidgets::iterator& add_before);
  void add_layout_related_at_position(const sharedptr<LayoutItem_Portal>& portal, const type_list_layoutwidgets::iterator& add_before);

  Box_Data_List_Related* create_related(const sharedptr<LayoutItem_Portal>& portal, bool show_title = true);

  Glib::ustring m_table_name;

  type_signal_field_edited m_signal_field_edited;
  type_signal_field_open_details_requested m_signal_field_open_details_requested;

  //type_signal_related_record_added m_signal_related_record_added;
  type_signal_related_record_changed m_signal_related_record_changed;
  type_signal_requested_related_details m_signal_requested_related_details;

  type_signal_script_button_clicked m_signal_script_button_clicked;
};

} //namespace Glom

#endif //GLOM_UTILITYWIDGETS_FLOWTABLEWITHFIELDS_H
