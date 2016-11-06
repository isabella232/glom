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

#ifndef GLOM_MODE_DATA_FLOWTABLEWITHFIELDS_H
#define GLOM_MODE_DATA_FLOWTABLEWITHFIELDS_H

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/utility_widgets/flowtable.h>
#include <libglom/data_structure/layout/layoutgroup.h>
#include <libglom/data_structure/layout/layoutitem_field.h>
#include <libglom/data_structure/layout/layoutitem_notebook.h>
#include <libglom/data_structure/layout/layoutitem_portal.h>
#include <libglom/data_structure/layout/layoutitem_calendarportal.h>
#include <libglom/data_structure/layout/layoutitem_button.h>
#include <libglom/data_structure/layout/layoutitem_text.h>
#include <libglom/data_structure/field.h>
#include <libglom/document/document.h>
#include <glom/utility_widgets/layoutwidgetbase.h>
#include <glom/utility_widgets/layoutwidgetutils.h>
#include <glom/mode_data/box_data_list_related.h>
#include <glom/mode_data/datawidget/combochoices.h>
#include "box_data_calendar_related.h"
#include <glom/mode_design/layout/treestore_layout.h> //Forthe enum.
#include <gtkmm/checkbutton.h>
#include <gtkmm/sizegroup.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/box.h>
#include <unordered_map>
#include <list>

namespace Glom
{

class DataWidget;

class FlowTableWithFields
  :
    public FlowTable,
#ifndef GLOM_ENABLE_CLIENT_ONLY
    public LayoutWidgetUtils,
#endif
    public View_Composite_Glom
{
public:
  explicit FlowTableWithFields(const Glib::ustring& table_name = Glib::ustring());
  virtual ~FlowTableWithFields();

  ///The table name is needed to discover details of relationships.
  void set_table(const Glib::ustring& table_name);

  /** Prevent any attempts to change actual records,
   * if the widget is just being used to enter find critera,
   * and prevents any need for data retrieval from the database, because
   * no data will be displayed.
   *
   * @param val True if find mode should be used.
   */
  void set_find_mode(bool val = true);

  /** Add a field.
   * @param layoutitem_field The layout item that describes this field,
   * @param table_name The table on which this layout appears.
   */
  void add_field(const std::shared_ptr<LayoutItem_Field>& layoutitem_field, const Glib::ustring& table_name);

  void remove_field(const Glib::ustring& id);

  void add_layout_item(const std::shared_ptr<LayoutItem>& item);

    /**
   * @param with_indent Pass true for top-level groups, to avoid wasting extra space with an unnecessary indent.
   */
  void add_layout_group_or_derived(const std::shared_ptr<LayoutGroup>& group, bool with_indent = true);

  void set_field_editable(const LayoutItem_Field& field, bool editable = true);

  Gnome::Gda::Value get_field_value(const LayoutItem_Field& field) const;

  /** Set the displayed @a value in any instances of the specified @a field.
   */
  void set_field_value(const LayoutItem_Field& field, const Gnome::Gda::Value& value);

  /** Set the displayed @a value in any instances of the field other than the specified @a layout_field.
   */
  void set_other_field_value(const LayoutItem_Field& layout_field, const Gnome::Gda::Value& value);

  /** Refresh the list of related records in choice combo boxes,
   * in any instance of the specified field.
   */
  void update_choices(const LayoutItem_Field& field);

  typedef std::list<Gtk::Widget*> type_list_widgets;
  typedef std::list<const Gtk::Widget*> type_list_const_widgets;

  void set_design_mode(bool value = true) override;

  void remove_all();

  typedef std::vector< Glib::RefPtr<Gtk::SizeGroup> > type_vec_sizegroups;

  /** Apply the size groups to all field labels.
   * By calling this method on multiple FlowTables, the field widgets in
   * different groups can then align.
   * @param size_groups A vector containing a size group for each possible column.
   */
  void apply_size_groups_to_labels(const type_vec_sizegroups& size_group);

  /** Create a size group and make all the labels in child flowtables use it,
   * making them align.
   */
  void align_child_group_labels();

  /** Get the layout structure, which might have changed in the child widgets since
   * the whole widget structure was built.
   * for instance, if the user chose a new field for a DataWidget,
   * or a new relationship for a portal.
   */
  void get_layout_groups(Document::type_list_layout_groups& groups);
  std::shared_ptr<LayoutGroup> get_layout_group();

  void set_enable_drag_and_drop(bool enabled = true);

  /** For instance,
   * void on_flowtable_field_edited(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
   */
  typedef sigc::signal<void(const std::shared_ptr<const LayoutItem_Field>&, const Gnome::Gda::Value&)> type_signal_field_edited;
  type_signal_field_edited signal_field_edited();

  /** For instance,
   * void on_flowtable_field_choices_changed(const std::shared_ptr<const LayoutItem_Field>& field);
   */
  typedef sigc::signal<void(const std::shared_ptr<const LayoutItem_Field>&)> type_signal_field_choices_changed;
  type_signal_field_choices_changed signal_field_choices_changed();


  /** For instance,
   * void on_flowtable_field_open_details_requested(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value);
   */
  typedef sigc::signal<void(const std::shared_ptr<const LayoutItem_Field>&, const Gnome::Gda::Value&)> type_signal_field_open_details_requested;
  type_signal_field_open_details_requested signal_field_open_details_requested();

  /** For instance,
   * void on_related_record_changed(const Glib::ustring& relationship_name);
   */
  typedef sigc::signal<void(const Glib::ustring&)> type_signal_related_record_changed;
  type_signal_related_record_changed signal_related_record_changed();

  /** For instance,
   * void on_requested_related_details(const Glib::ustring& table_name, Gnome::Gda::Value primary_key_value);
   */
  typedef sigc::signal<void(const Glib::ustring&, Gnome::Gda::Value)> type_signal_requested_related_details;
  type_signal_requested_related_details signal_requested_related_details();

 /** For instance,
   * void on_script_button_clicked(const std::weak_ptr<LayoutItem_Button>& layout_item_weak>);
   */
  typedef sigc::signal<void(const std::shared_ptr<LayoutItem_Button>&)> type_signal_script_button_clicked;
  type_signal_script_button_clicked signal_script_button_clicked();

private:

  void set_field_value(const LayoutItem_Field& field, const Gnome::Gda::Value& value, bool set_specified_field_layout);

  // If include_item is set, then the output list will contain field's widget,
  // otherwise not.
  type_list_widgets get_field(const LayoutItem_Field& field, bool include_item);
  type_list_const_widgets get_field(const LayoutItem_Field& field, bool include_item) const;

  typedef std::list<Box_Data_Portal*> type_portals;

  /// Get portals whose relationships have @a from_key as the from_key.
  type_portals get_portals(const LayoutItem_Field& from_key);


  typedef std::list<DataWidgetChildren::ComboChoices*> type_choice_widgets;

  /// Get choice widgets with !show_all relationships that have @a from_key as the from_key.
  type_choice_widgets get_choice_widgets(const LayoutItem_Field& from_key);

  /** Examine this flow table and all child flow tables, discovering which
   * has the most columns.
   */
  guint get_sub_flowtables_max_columns() const;

  //int get_suitable_width(Field::glom_field_type field_type);

  void on_entry_edited(const Gnome::Gda::Value& value, const std::weak_ptr<const LayoutItem_Field>& field);
  void on_entry_choices_changed(const std::weak_ptr<const LayoutItem_Field>& field);
  void on_entry_open_details_requested(const Gnome::Gda::Value& value, const std::weak_ptr<const LayoutItem_Field>& field);

  void on_script_button_clicked(const std::weak_ptr<LayoutItem_Button>& layout_item_weak);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_datawidget_layout_item_added(LayoutWidgetBase::enumType item_type, DataWidget* pDataWidget);
#endif // !GLOM_ENABLE_CLIENT_ONLY

  void on_portal_user_requested_details(Gnome::Gda::Value primary_key_value, Box_Data_Portal* portal_box);
  class Info
  {
  public:
    Info();

    std::shared_ptr<const LayoutItem_Field> m_field; //Store the field information so we know the title, ID, and type.

    Gtk::Widget* m_first;
    Gtk::EventBox* m_first_eventbox; //The label is often inside an eventbox.
    Glib::RefPtr<Gtk::SizeGroup> m_first_in_sizegroup; //Just to avoid a warning when removing a widget not in a group.

    DataWidget* m_second;
    Gtk::CheckButton* m_checkbutton; //Used instead of first and second if it's a bool.
  };

  //Map of IDs to full info.
  std::list<Info> m_listFields;

  //Remember the nested FlowTables, so that we can search them for fields too:
  std::list< FlowTableWithFields* > m_sub_flow_tables;

  type_portals m_portals;

  //Remember the sequence of LayoutWidgetBase widgets, so we can iterate over them later:
  std::list<LayoutWidgetBase*> m_list_layoutwidgets;

  void add_button(const std::shared_ptr<LayoutItem_Button>& layoutitem_button, const Glib::ustring& table_name);
  void add_textobject(const std::shared_ptr<LayoutItem_Text>& layoutitem_text, const Glib::ustring& table_name);
  void add_imageobject(const std::shared_ptr<LayoutItem_Image>& layoutitem_image, const Glib::ustring& table_name);

  void add_layoutwidgetbase(LayoutWidgetBase* layout_widget);

  /** Add a normal group, not a notebook or portal.
   * See also add_layout_group_or_derived().
   *
   * @param with_indent Pass true for top-level groups, to avoid wasting extra space with an unnecessary indent.
   */
  void add_layout_group(const std::shared_ptr<LayoutGroup>& group, bool with_indent = true);

  void add_layout_notebook(const std::shared_ptr<LayoutItem_Notebook>& notebook);
  void add_layout_portal(const std::shared_ptr<LayoutItem_Portal>& portal);

#ifndef GLOM_ENABLE_CLIENT_ONLY

  std::shared_ptr<LayoutItem_Portal> get_portal_relationship();

#endif // !GLOM_ENABLE_CLIENT_ONLY

  Box_Data_List_Related* create_related(const std::shared_ptr<LayoutItem_Portal>& portal, bool show_title = true);
  Box_Data_Calendar_Related* create_related_calendar(const std::shared_ptr<LayoutItem_CalendarPortal>& portal, bool show_title = true);

  Glib::ustring m_table_name;
  bool m_find_mode;

  //Size groups shared by this widget's sibling FlowTables,
  //with one group for each column.
  type_vec_sizegroups m_vec_size_groups;

  type_signal_field_edited m_signal_field_edited;
  type_signal_field_choices_changed m_signal_field_choices_changed;
  type_signal_field_open_details_requested m_signal_field_open_details_requested;

  //type_signal_related_record_added m_signal_related_record_added;
  type_signal_related_record_changed m_signal_related_record_changed;
  type_signal_requested_related_details m_signal_requested_related_details;
  type_signal_script_button_clicked m_signal_script_button_clicked;

  //menu
#ifndef GLOM_ENABLE_CLIENT_ONLY
  void on_menu_properties_activate() override;
  void on_menu_delete_activate() override; // override this to add a dialog box
  bool on_button_press_event(GdkEventButton *event) override;
#endif // !GLOM_ENABLE_CLIENT_ONLY
};

} //namespace Glom

#endif // GLOM_MODE_DATA_FLOWTABLEWITHFIELDS_H
