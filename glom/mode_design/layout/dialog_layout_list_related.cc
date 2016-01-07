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

#include <glom/mode_design/layout/dialog_layout_list_related.h>
#include <glom/mode_design/layout/dialog_choose_field.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h>
#include <libglom/utils.h> //For bold_message().
#include <libglom/db_utils.h>
#include <glom/utils_ui.h> //For show_ok_dialog().

//#include <libgnome/gnome-i18n.h>
#include <gtkmm/togglebutton.h>
#include <glibmm/i18n.h>

#include <iostream>

namespace Glom
{

const char* Dialog_Layout_List_Related::glade_id("window_data_layout");
const bool Dialog_Layout_List_Related::glade_developer(true);

Dialog_Layout_List_Related::Dialog_Layout_List_Related(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& builder)
: Dialog_Layout_List(cobject, builder),
  m_combo_relationship(nullptr),
  m_checkbutton_show_child_relationships(nullptr),
  m_radio_navigation_automatic(nullptr),
  m_radio_navigation_none(nullptr),
  m_radio_navigation_specify(nullptr),
  m_label_navigation_automatic(nullptr),
  m_combo_navigation_specify(nullptr),
  m_spinbutton_row_line_width(nullptr),
  m_spinbutton_column_line_width(nullptr),
  m_colorbutton_line(nullptr),
  m_for_print_layout(false)
{  
  // Show the appropriate alternate widgets:
  m_box_table_widgets->hide();
  m_box_related_table_widgets->show();
  m_box_related_navigation->show();
  m_hbox_rows_count->show();
  
  m_spinbutton_rows_count_min->set_range(0, 100); //Otherwise only 0 would be allowed.
  m_spinbutton_rows_count_min->set_increments(1, 10); //Otherwise the buttons do nothing.
  m_spinbutton_rows_count_min->signal_value_changed().connect(
    sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_spinbutton_changed));
  m_spinbutton_rows_count_max->set_range(0, 100); //Otherwise only 0 would be allowed.
  m_spinbutton_rows_count_max->set_increments(1, 10); //Otherwise the buttons do nothing.
  m_spinbutton_rows_count_max->signal_value_changed().connect(
    sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_spinbutton_changed));

  builder->get_widget_derived("combo_relationship_name", m_combo_relationship);
  if(m_combo_relationship)
  {
    m_combo_relationship->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_combo_relationship_changed));
  }

  builder->get_widget("checkbutton_show_child_relationships", m_checkbutton_show_child_relationships);
  if(m_checkbutton_show_child_relationships)
  {
    m_checkbutton_show_child_relationships->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_checkbutton_show_child_relationships));
  }


  builder->get_widget("radiobutton_navigation_automatic", m_radio_navigation_automatic);
  builder->get_widget("label_navigation_automatic", m_label_navigation_automatic);
  make_sensitivity_depend_on_toggle_button(*m_radio_navigation_automatic, *m_label_navigation_automatic);

  builder->get_widget("radiobutton_navigation_none", m_radio_navigation_none);

  builder->get_widget("radiobutton_navigation_specify", m_radio_navigation_specify);
  builder->get_widget_derived("combobox_navigation_specify", m_combo_navigation_specify);
  if(m_radio_navigation_specify && m_combo_navigation_specify)
  {
    make_sensitivity_depend_on_toggle_button(*m_radio_navigation_specify, *m_combo_navigation_specify);
    m_combo_navigation_specify->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_combo_navigation_specific_changed));
  }

  builder->get_widget("spinbutton_row_line_width", m_spinbutton_row_line_width);
  if(m_spinbutton_row_line_width)
  {
    m_spinbutton_row_line_width->signal_value_changed().connect(
      sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_spinbutton_changed));
  }

  builder->get_widget("spinbutton_column_line_width", m_spinbutton_column_line_width);
  if(m_spinbutton_column_line_width)
  {
    m_spinbutton_column_line_width->signal_value_changed().connect(
      sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_spinbutton_changed));
  }

  builder->get_widget("colorbutton_line", m_colorbutton_line);
  if(m_colorbutton_line)
  {
    m_colorbutton_line->signal_color_set().connect(
      sigc::mem_fun(*this, &Dialog_Layout_List_Related::on_spinbutton_changed));
  }


  m_modified = false;

  //show_all_children();

  //This entry must be in the Glade file, because it's used by the base class,
  //but we don't want it here, because it is confusing when dealing with relationships:
  if(m_entry_table_title)
    m_entry_table_title->hide(); // We don't use this (it's from the base class).

  if(m_label_table_title)
    m_label_table_title->hide(); // We don't use this (it's from the base class).
}

void Dialog_Layout_List_Related::init_with_portal(const Glib::ustring& layout_name, const Glib::ustring& layout_platform, Document* document, const std::shared_ptr<const LayoutItem_Portal>& portal, const Glib::ustring& from_table, bool for_print_layout)
{
  m_for_print_layout = for_print_layout;

  //Ignore the provided from_table if the portal has one:
  Glib::ustring actual_from_table;
  if(portal)
  {
    const auto portal_from_table = portal->get_from_table();
    if(portal_from_table.empty())
      actual_from_table = portal_from_table;
  }

  if(actual_from_table.empty())
    actual_from_table = from_table;

  if(portal)
    m_portal = glom_sharedptr_clone(portal);
  else
    m_portal = std::make_shared<LayoutItem_Portal>(); //The rest of the class assumes that this is not null.

  gulong rows_count_min = 0;
  gulong rows_count_max = 0;
  m_portal->get_rows_count(rows_count_min, rows_count_max);
  m_spinbutton_rows_count_min->set_value(rows_count_min);
  m_spinbutton_rows_count_max->set_value(rows_count_max);

  type_vecConstLayoutFields empty_fields; //Just to satisfy the base class.
  Dialog_Layout::init(layout_name, layout_platform, document, actual_from_table, empty_fields);
  //m_table_name is now actually the parent_table_name.

  //Hide unwanted widgets and show extra ones:
  if(for_print_layout)
  {
    m_box_related_navigation->hide();
    m_box_frame_lines->show();
    
    m_spinbutton_row_line_width->set_value(
      portal->get_print_layout_row_line_width());
    m_spinbutton_column_line_width->set_value(
      portal->get_print_layout_column_line_width());

    const Gdk::RGBA color( portal->get_print_layout_line_color() );
    m_colorbutton_line->set_rgba(color);

    //Avoid showing formatting options that are about editing:
    m_editable_layout = false;
  }

  update_ui();
}

void Dialog_Layout_List_Related::update_ui(bool including_relationship_list)
{
  if(!m_portal)
    return;

  m_modified = false;

  const auto related_table_name = m_portal->get_table_used(Glib::ustring() /* parent table - not relevant*/);

  //Update the tree models from the document
  auto document = get_document();
  if(document)
  {
    //Fill the relationships combo:
    if(including_relationship_list)
    {
      bool show_child_relationships = m_checkbutton_show_child_relationships->get_active();

      //For the showing of child relationships if necessary:
      if(!show_child_relationships && m_portal->get_related_relationship())
      {
        show_child_relationships = true;
      }

      Glib::ustring from_table;
      if(m_portal->get_has_relationship_name())
        from_table = m_portal->get_relationship()->get_from_table();
      else
        from_table = m_table_name;

      m_combo_relationship->set_relationships(document, from_table, show_child_relationships, false /* don't show parent table */); //We don't show the optional parent table because portal use _only_ relationships, of course.

      if(show_child_relationships != m_checkbutton_show_child_relationships->get_active())
      {
         m_checkbutton_show_child_relationships->set_active(show_child_relationships);
      }
    }

    //Set the table name and title:
    //auto portal_temp = m_portal;
    Document::type_list_layout_groups mapGroups;
    if(m_portal)
    {
      m_combo_relationship->set_selected_relationship(m_portal->get_relationship(), m_portal->get_related_relationship());

      mapGroups.push_back(m_portal);
      document->fill_layout_field_details(related_table_name, mapGroups); //Update with full field information.

      //Show the field layout
      //typedef std::list< Glib::ustring > type_listStrings;
    }

    m_model_items->clear();

    for(const auto& group : mapGroups)
    {
      auto portal = std::dynamic_pointer_cast<const LayoutItem_Portal>(group);
      if(portal)
      {
        for(const auto& item : group->m_list_items)
        {
          auto groupInner = std::dynamic_pointer_cast<const LayoutGroup>(item);

          if(groupInner)
            add_group(Gtk::TreeModel::iterator() /* null == top-level */, groupInner);
          else
          {
            //Add the item to the treeview:
            auto iter = m_model_items->append();
            Gtk::TreeModel::Row row = *iter;
            row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(item);
          }
        }
      }
    }
  }

  //Show the navigation information:
  //const Document::type_vec_relationships vecRelationships = document->get_relationships(m_portal->get_relationship()->get_from_table());
  m_combo_navigation_specify->set_relationships(document, related_table_name, true /* show related relationships */, false /* don't show parent table */); //TODO: Don't show the hidden tables, and don't show relationships that are not used by any fields.
  //m_combo_navigation_specify->set_display_parent_table(""); //This would be superfluous, and a bit confusing.

  if(m_portal->get_navigation_type() == LayoutItem_Portal::navigation_type::SPECIFIC)
  {
    auto navrel = m_portal->get_navigation_relationship_specific();
    //std::cout << "debug navrel=" << navrel->get_relationship()->get_name() << std::endl;
    m_combo_navigation_specify->set_selected_relationship(navrel->get_relationship(), navrel->get_related_relationship());
  }
  else
  {
    std::shared_ptr<const Relationship> none;
    m_combo_navigation_specify->set_selected_relationship(none);
  }

  //Set the appropriate radio button:
  //std::cout << "debug: navrel_type=" << m_portal->get_navigation_relationship_type() << std::endl;
  switch(m_portal->get_navigation_type())
  {
    case LayoutItem_Portal::navigation_type::NONE:
      m_radio_navigation_none->set_active(true);
      break;

    case LayoutItem_Portal::navigation_type::AUTOMATIC:
      m_radio_navigation_automatic->set_active(true);
      break;

    case LayoutItem_Portal::navigation_type::SPECIFIC:
      m_radio_navigation_specify->set_active(true);
      break;
  }

  //Describe the automatic navigation:
  auto relationship_navigation_automatic = 
    m_portal->get_portal_navigation_relationship_automatic(document);
  Glib::ustring automatic_navigation_description = 
    m_portal->get_relationship_name_used(); //TODO: Use get_relationship_display_name() instead?
  if(relationship_navigation_automatic) //This is a relationship in the related table.
  {
    automatic_navigation_description += ("::" + relationship_navigation_automatic->get_relationship_display_name());
  }

  if(automatic_navigation_description.empty())
  {
    automatic_navigation_description = _("None: No visible tables are specified by the fields.");
  }

  m_label_navigation_automatic->set_text(automatic_navigation_description);

  m_modified = false;
}

void Dialog_Layout_List_Related::save_to_document()
{
  Dialog_Layout::save_to_document();

  if(m_modified)
  {
    //Get the data from the TreeView and store it in the document:
    
    //Get the groups and their fields:
    Document::type_list_layout_groups mapGroups;

    //Add the fields to the portal:
    //The code that created this dialog must read m_portal back out again.

    m_portal->remove_all_items();

    guint field_sequence = 1; //0 means no sequence
    for(const auto& row : m_model_items->children())
    {
      std::shared_ptr<LayoutItem> item = row[m_model_items->m_columns.m_col_layout_item];
      const auto field_name = item->get_name();
      if(!field_name.empty())
      {
        m_portal->add_item(item); //Add it to the group:

        ++field_sequence;
      }
    }

    if(m_radio_navigation_specify->get_active())
    {
      std::shared_ptr<Relationship> rel, rel_related;
      rel = m_combo_navigation_specify->get_selected_relationship(rel_related);

      auto uses_rel = std::make_shared<UsesRelationship>();
      uses_rel->set_relationship(rel);
      uses_rel->set_related_relationship(rel_related);

      if(rel || rel_related)
        m_portal->set_navigation_relationship_specific(uses_rel);
      //std::cout << "debug99 main=specify_main" << ", relationship=" << (rel ? rel->get_name() : "none") << std::endl;
    }
    else
    {
      // TODO: Get rid of the else branch. Cleanup code for the relations should
      // go into the Glom::LayoutItem_Portal::set_navigation_relationship_type_* functions.

      //std::cout << "debug: set_navigation_relationship_specific(false, none)" << std::endl;
      std::shared_ptr<UsesRelationship> none;
      m_portal->set_navigation_relationship_specific(none);
    }

    if(m_radio_navigation_automatic->get_active())
      m_portal->set_navigation_type(LayoutItem_Portal::navigation_type::AUTOMATIC);

    if(m_radio_navigation_none->get_active())
    {
      auto uses_rel = std::make_shared<UsesRelationship>();
      uses_rel->set_related_relationship(std::shared_ptr<Relationship>());
      m_portal->set_navigation_type(LayoutItem_Portal::navigation_type::NONE);
    }
    
    m_portal->set_rows_count(
      m_spinbutton_rows_count_min->get_value(),
      m_spinbutton_rows_count_max->get_value());
    
    if(m_for_print_layout)
    {
      m_portal->set_print_layout_row_line_width(
        m_spinbutton_row_line_width->get_value());
      m_portal->set_print_layout_column_line_width(
        m_spinbutton_column_line_width->get_value());      
      m_portal->set_print_layout_line_color(
        m_colorbutton_line->get_rgba().to_string() );
    }
  }
}

void Dialog_Layout_List_Related::on_checkbutton_show_child_relationships()
{
  update_ui();
}

void Dialog_Layout_List_Related::on_combo_relationship_changed()
{
  if(!m_portal)
    return;

  std::shared_ptr<Relationship> relationship_related;
  auto relationship = m_combo_relationship->get_selected_relationship(relationship_related);
  if(!relationship)
    return;

  //Check that the relationship is appropriate for use in a related records portal.
  //The relationship's to field may not be a unique field, because that would
  //prevent the portal from having multiple records.
  auto to_key_field =
    DbUtils::get_fields_for_table_one_field(get_document(), 
      relationship->get_to_table(), relationship->get_to_field());
  bool relationship_invalid = false;
  if(!to_key_field)
  {
    UiUtils::show_ok_dialog(_("Invalid Relationship"),
      _("The relationship may not be used to show related records because the relationship does not specify a field in the related table."),
     *this, Gtk::MESSAGE_ERROR);
    relationship_invalid = true;
  }
  else if(to_key_field->get_primary_key())
  {
    UiUtils::show_ok_dialog(_("Relationship Uses a Related Primary Key"),
      _("The relationship may not be used to show related records because the relationship uses a primary key field in the related table, which must contain unique values. This would prevent the relationship from specifying multiple related records."),
     *this, Gtk::MESSAGE_ERROR);
    relationship_invalid = true;
  }
  else if(to_key_field->get_unique_key())
  {
    UiUtils::show_ok_dialog(_("Relationship Uses a Related Unique Field"),
      _("The relationship may not be used to show related records because the relationship uses a unique-values field in the related table. This would prevent the relationship from specifying multiple related records."),
     *this, Gtk::MESSAGE_ERROR);
    relationship_invalid = true;
  }

  //Reset the previous value if the choice was bad:
  if(relationship_invalid)
  {
    m_combo_relationship->set_selected_relationship(
       m_portal->get_relationship(), m_portal->get_related_relationship());
    return;
  }

  //Clear the list of fields if the relationship has changed,
  //because the fields could not possible be correct for the new table:
  bool relationship_changed = false;
  const auto old_relationship_name = glom_get_sharedptr_name(m_portal->get_relationship());
  const auto old_relationship_related_name = glom_get_sharedptr_name(m_portal->get_related_relationship());
  if( (old_relationship_name != glom_get_sharedptr_name(relationship)) ||
      (old_relationship_related_name != glom_get_sharedptr_name(relationship_related)) )
  {
    relationship_changed = true;
  }


  m_portal->set_relationship(relationship);
  m_portal->set_related_relationship(relationship_related);

  if(relationship_changed)
    m_portal->remove_all_items();

  //Refresh everything for the new relationship:
  update_ui(false /* not including the list of relationships */);

  m_modified = true;
}

void Dialog_Layout_List_Related::on_spinbutton_changed()
{
  m_modified = true;
}

std::shared_ptr<Relationship> Dialog_Layout_List_Related::get_relationship() const
{
  std::cout << "debug: I wonder if this function is used." << std::endl;
  return m_combo_relationship->get_selected_relationship();
}

std::shared_ptr<LayoutItem_Portal> Dialog_Layout_List_Related::get_portal_layout()
{
  return m_portal;
}

void Dialog_Layout_List_Related::on_combo_navigation_specific_changed()
{
  m_modified = true;
}

//Overridden so we can show related fields instead of fields from the parent table:
void Dialog_Layout_List_Related::on_button_add_field()
{
  //Get the chosen field:
  //std::cout << "debug: related relationship=" << glom_get_sharedptr_name(m_portal->get_related_relationship()) << std::endl;
  //std::cout << "debug table used =" << m_portal->get_table_used(m_table_name) << std::endl;

  type_list_field_items fields_list = offer_field_list(m_portal->get_table_used(m_table_name), this);
  for(const auto& field : fields_list)
  {
    if(!field)
      continue;

    //Add the field details to the layout treeview:
    auto iter = m_model_items->append();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_model_items->m_columns.m_col_layout_item] = field;

      //Scroll to, and select, the new row:
      auto refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );
    }
  }
}

void Dialog_Layout_List_Related::on_button_edit()
{
  auto refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    auto iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      std::shared_ptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      auto field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);

      //Get the chosen field:
      auto field_chosen = offer_field_list_select_one_field(field, m_portal->get_table_used(m_table_name), this);
      if(field_chosen)
      {
        //Set the field details in the layout treeview:

        row[m_model_items->m_columns.m_col_layout_item] = field_chosen;

        //Scroll to, and select, the new row:
        /*
        auto refTreeSelection = m_treeview_fields->get_selection();
        if(refTreeSelection)
          refTreeSelection->select(iter);

        m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

        treeview_fill_sequences(m_model_items, m_model_items->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
        */
      }
    }
  }
}

Glib::ustring Dialog_Layout_List_Related::get_fields_table() const
{
  if(!m_portal)
    return Glib::ustring();

  return m_portal->get_table_used(m_table_name);
}

} //namespace Glom
