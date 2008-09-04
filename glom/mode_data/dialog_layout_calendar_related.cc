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

#include "dialog_layout_calendar_related.h"
#include "dialog_choose_field.h"
#include "../layout_item_dialogs/dialog_field_layout.h"
#include <bakery/App/App_Gtk.h> //For util_bold_message().

//#include <libgnome/gnome-i18n.h>
#include <gtkmm/togglebutton.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_Layout_Calendar_Related::Dialog_Layout_Calendar_Related(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Dialog_Layout_List(cobject, refGlade),
  m_combo_relationship(0),
  m_checkbutton_show_child_relationships(0),
  m_radio_navigation_automatic(0),
  m_radio_navigation_specify(0),
  m_label_navigation_automatic(0),
  m_combo_navigation_specify(0),
  m_combobox_date_field(0)
{
  // Show the appropriate alternate widgets:
  m_box_table_widgets->hide();
  m_box_related_table_widgets->show();
  m_box_related_navigation->show();

  refGlade->get_widget_derived("combo_relationship_name", m_combo_relationship);
  m_combo_relationship->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Layout_Calendar_Related::on_combo_relationship_changed));

  refGlade->get_widget("checkbutton_show_child_relationships", m_checkbutton_show_child_relationships);
  m_checkbutton_show_child_relationships->signal_toggled().connect(sigc::mem_fun(*this, &Dialog_Layout_Calendar_Related::on_checkbutton_show_child_relationships));

  

  refGlade->get_widget("radiobutton_navigation_automatic", m_radio_navigation_automatic);
  refGlade->get_widget("label_navigation_automatic", m_label_navigation_automatic);
  make_sensitivity_depend_on_toggle_button(*m_radio_navigation_automatic, *m_label_navigation_automatic);

  refGlade->get_widget("radiobutton_navigation_specify", m_radio_navigation_specify);
  refGlade->get_widget_derived("combobox_navigation_specify", m_combo_navigation_specify);
  make_sensitivity_depend_on_toggle_button(*m_radio_navigation_specify, *m_combo_navigation_specify);
  m_combo_navigation_specify->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Layout_Calendar_Related::on_combo_navigation_specific_changed));

  refGlade->get_widget_derived("combobox_date_field", m_combobox_date_field);
  m_combobox_date_field->signal_changed().connect(sigc::mem_fun(*this, &Dialog_Layout_Calendar_Related::on_combo_date_field_changed));

  
  m_modified = false;

  //show_all_children();

  //The base class hid this, but we do want it here:
  //(We share one glade definition for several dialogs.)
  Gtk::Frame* box_calendar = 0;
  refGlade->get_widget("frame_calendar", box_calendar); 
  box_calendar->show();
  
  //This entry must be in the Glade file, because it's used by the base class,
  //but we don't want it here, because it is confusing when dealing with relationships:
  if(m_entry_table_title)
    m_entry_table_title->hide(); // We don't use this (it's from the base class).

  if(m_label_table_title)
    m_label_table_title->hide(); // We don't use this (it's from the base class).
}

Dialog_Layout_Calendar_Related::~Dialog_Layout_Calendar_Related()
{
}


void Dialog_Layout_Calendar_Related::set_document(const Glib::ustring& layout, Document_Glom* document, const sharedptr<const LayoutItem_CalendarPortal>& portal)
{
  m_portal = glom_sharedptr_clone(portal);
  
  Glib::ustring from_table;
  if(portal)
    from_table = portal->get_from_table();

  set_document(layout, document, from_table);
}

void Dialog_Layout_Calendar_Related::set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& from_table)
{
  if(!m_portal)
  {
    m_portal = sharedptr<LayoutItem_CalendarPortal>::create(); //The rest of the class assumes that this is not null.
  }

  type_vecLayoutFields empty_fields; //Just to satisfy the base class.

  
  Dialog_Layout::set_document(layout, document, from_table, empty_fields);
  //m_table_name is now actually the parent_table_name.

  update_ui();
}

void Dialog_Layout_Calendar_Related::update_ui(bool including_relationship_list)
{
  m_modified = false;

  const Glib::ustring related_table_name = m_portal->get_table_used(Glib::ustring() /* parent table - not relevant*/);

  //Update the tree models from the document
  Document_Glom* document = get_document();
  if(document)
  {
    //Fill the relationships combo:
    if(including_relationship_list)
    {
      bool show_child_relationships = m_checkbutton_show_child_relationships->get_active();
        
      //For the showing of child relationships if necessary:
      if(!show_child_relationships && m_portal && m_portal->get_related_relationship())
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
    //sharedptr<LayoutItem_CalendarPortal> portal_temp = m_portal;
    m_combo_relationship->set_selected_relationship(m_portal->get_relationship(), m_portal->get_related_relationship()); 

    Document_Glom::type_list_layout_groups mapGroups;
    if(m_portal)
    {
      mapGroups.push_back(m_portal);
      document->fill_layout_field_details(related_table_name, mapGroups); //Update with full field information.
    }

    //Show the field layout
    //typedef std::list< Glib::ustring > type_listStrings;

    m_model_items->clear();

    for(Document_Glom::type_list_layout_groups::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      sharedptr<const LayoutGroup> group = *iter;
      sharedptr<const LayoutGroup> portal = sharedptr<const LayoutItem_CalendarPortal>::cast_dynamic(group);
      if(portal)
      {
        for(LayoutGroup::type_list_items::const_iterator iterInner = group->m_list_items.begin(); iterInner != group->m_list_items.end(); ++iterInner)
        {
          sharedptr<const LayoutItem> item = *iterInner;
          sharedptr<const LayoutGroup> groupInner = sharedptr<const LayoutGroup>::cast_dynamic(item);

          if(groupInner)
            add_group(Gtk::TreeModel::iterator() /* null == top-level */, groupInner);
          else
          {
            //Add the item to the treeview:
            Gtk::TreeModel::iterator iter = m_model_items->append();
            Gtk::TreeModel::Row row = *iter;
            row[m_model_items->m_columns.m_col_layout_item] = glom_sharedptr_clone(item);
          }
        }
      }
    }
  }

  //Show the navigation information:
  //const Document_Glom::type_vecRelationships vecRelationships = document->get_relationships(m_portal->get_relationship()->get_from_table());
  m_combo_navigation_specify->set_relationships(document, related_table_name, true /* show related relationships */, false /* don't show parent table */); //TODO: Don't show the hidden tables, and don't show relationships that are not used by any fields.
  //m_combo_navigation_specify->set_display_parent_table(""); //This would be superfluous, and a bit confusing.

  bool navigation_is_automatic = false;
  bool navigation_specific_main = false;
  sharedptr<UsesRelationship> navrel = m_portal->get_navigation_relationship_specific(navigation_specific_main);
  if(navigation_specific_main)
    m_combo_navigation_specify->set_selected_parent_table(related_table_name);
  else if(navrel)
  {
    //std::cout << "debug navrel=" << navrel->get_relationship()->get_name() << std::endl;
    m_combo_navigation_specify->set_selected_relationship(navrel->get_relationship(), navrel->get_related_relationship());
  }
  else
  {
    navigation_is_automatic = true;

    sharedptr<const Relationship> none;
    m_combo_navigation_specify->set_selected_relationship(none);
  }

  //Set the appropriate radio button:
  //std::cout << "debug: navigation_is_automatic=" << navigation_is_automatic << std::endl;
  m_radio_navigation_automatic->set_active(navigation_is_automatic); 
  m_radio_navigation_specify->set_active(!navigation_is_automatic); 


  //Describe the automatic navigation:
  sharedptr<const UsesRelationship> relationship_navigation_automatic;
  bool navigation_automatic_main = false;
  relationship_navigation_automatic = get_portal_navigation_relationship_automatic(m_portal, navigation_automatic_main);
  Glib::ustring automatic_navigation_description;
  
  if(navigation_automatic_main)
    automatic_navigation_description = m_portal->get_relationship_name_used();
  else if(relationship_navigation_automatic)
  {
    automatic_navigation_description = m_portal->get_relationship_name_used();

    if(relationship_navigation_automatic->get_has_relationship_name())
      automatic_navigation_description += ("::" + relationship_navigation_automatic->get_relationship_name());

    if(relationship_navigation_automatic->get_has_related_relationship_name())
      automatic_navigation_description += ("::" + relationship_navigation_automatic->get_related_relationship_name());
  }

  if(automatic_navigation_description.empty())
  {
    automatic_navigation_description = _("None: No visible tables are specified by the fields.");
  }

  m_label_navigation_automatic->set_text(automatic_navigation_description);

  sharedptr<Field> debugfield = m_portal->get_date_field();
  if(!debugfield)
    std::cout << "DEBUG: set_document(): date field is NULL" << std::endl;
  else
    std::cout << "DEBUG: set_document(): date field:" << debugfield->get_name() << std::endl;
  
  m_combobox_date_field->set_fields(document, related_table_name, Field::TYPE_DATE);
  m_combobox_date_field->set_selected_field(m_portal->get_date_field());
    
  m_modified = false;
}

void Dialog_Layout_Calendar_Related::save_to_document()
{
  Dialog_Layout::save_to_document();

  if(m_modified)
  {
    //Get the data from the TreeView and store it in the document:

    //Get the groups and their fields:
    Document_Glom::type_list_layout_groups mapGroups;

    //Add the fields to the portal:
    //The code that created this dialog must read m_portal back out again.
    m_portal->remove_all_items();

    guint field_sequence = 1; //0 means no sequence
    for(Gtk::TreeModel::iterator iterFields = m_model_items->children().begin(); iterFields != m_model_items->children().end(); ++iterFields)
    {
      Gtk::TreeModel::Row row = *iterFields;

      sharedptr<LayoutItem> item = row[m_model_items->m_columns.m_col_layout_item];
      const Glib::ustring field_name = item->get_name();
      if(!field_name.empty())
      {
        m_portal->add_item(item); //Add it to the group:

        ++field_sequence;
      }
    }

    if(m_radio_navigation_specify->get_active())
    {
      sharedptr<Relationship> rel, rel_related;
      rel = m_combo_navigation_specify->get_selected_relationship(rel_related);

      bool specify_main = (!rel && !rel_related);
      sharedptr<UsesRelationship> uses_rel = sharedptr<UsesRelationship>::create();
      uses_rel->set_relationship(rel);
      uses_rel->set_related_relationship(rel_related);

      m_portal->set_navigation_relationship_specific(specify_main, uses_rel);
      //std::cout << "debug99 main=specify_main" << ", relationship=" << (rel ? rel->get_name() : "none") << std::endl;
    }
    else
    {
      //std::cout << "debug: set_navigation_relationship_specific(false, none)" << std::endl;
      sharedptr<UsesRelationship> none;
      m_portal->set_navigation_relationship_specific(false, none);
    }
    
    m_portal->set_date_field( m_combobox_date_field->get_selected_field() );
    
    sharedptr<Field> debugfield = m_portal->get_date_field();
    if(!debugfield)
      std::cout << "DEBUG: save_to_document(): date field is NULL" << std::endl;
    else
      std::cout << "DEBUG: save_to_document(): date field:" << debugfield->get_name() << std::endl;
  }
}

void Dialog_Layout_Calendar_Related::on_checkbutton_show_child_relationships()
{
  update_ui();
}

void Dialog_Layout_Calendar_Related::on_combo_relationship_changed()
{
  if(!m_portal)
    return;

  sharedptr<Relationship> relationship_related;
  sharedptr<Relationship> relationship = m_combo_relationship->get_selected_relationship(relationship_related);
  if(relationship)
  {
    //Clear the list of fields if the relationship has changed, because the fields could not possible be correct for the new table:
    bool relationship_changed = false;
    const Glib::ustring old_relationship_name = glom_get_sharedptr_name(m_portal->get_relationship());
    const Glib::ustring old_relationship_related_name = glom_get_sharedptr_name(m_portal->get_related_relationship());
    if( (old_relationship_name != glom_get_sharedptr_name(relationship)) ||
        (old_relationship_related_name != glom_get_sharedptr_name(relationship_related)) )
      relationship_changed = true;

    m_portal->set_relationship(relationship);
    m_portal->set_related_relationship(relationship_related);

    if(relationship_changed)
      m_portal->remove_all_items();

    //Refresh everything for the new relationship:
    update_ui(false /* not including the list of relationships */);

    m_modified = true;
  }
}

sharedptr<Relationship> Dialog_Layout_Calendar_Related::get_relationship() const
{
  std::cout << "debug: I wonder if this function is used." << std::endl;
  return m_combo_relationship->get_selected_relationship();
}

void Dialog_Layout_Calendar_Related::on_combo_navigation_specific_changed()
{
  m_modified = true;
}

void Dialog_Layout_Calendar_Related::on_combo_date_field_changed()
{
  m_modified = true;
}

//Overridden so we can show related fields instead of fields from the parent table:
void Dialog_Layout_Calendar_Related::on_button_field_add()
{
  //Get the chosen field:
  //std::cout << "debug: related relationship=" << glom_get_sharedptr_name(m_portal->get_related_relationship()) << std::endl;
  //std::cout << "debug table used =" << m_portal->get_table_used(m_table_name) << std::endl;

  sharedptr<LayoutItem_Field> field = offer_field_list(m_portal->get_table_used(m_table_name), this);
  if(field)
  {
    //Add the field details to the layout treeview:
    Gtk::TreeModel::iterator iter =  m_model_items->append();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      row[m_model_items->m_columns.m_col_layout_item] = field;

      //Scroll to, and select, the new row:
      Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
      if(refTreeSelection)
        refTreeSelection->select(iter);

      m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );
    }
  }
}

void Dialog_Layout_Calendar_Related::on_button_edit()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
  if(refTreeSelection)
  {
    //TODO: Handle multiple-selection:
    Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
    if(iter)
    {
      Gtk::TreeModel::Row row = *iter;
      sharedptr<LayoutItem> layout_item = row[m_model_items->m_columns.m_col_layout_item];
      sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);

      //Get the chosen field:
      sharedptr<LayoutItem_Field> field_chosen = offer_field_list(field, m_portal->get_table_used(m_table_name), this);
      if(field_chosen)
      {
        //Set the field details in the layout treeview:

        row[m_model_items->m_columns.m_col_layout_item] = field_chosen;

        //Scroll to, and select, the new row:
        /*
        Glib::RefPtr<Gtk::TreeView::Selection> refTreeSelection = m_treeview_fields->get_selection();
        if(refTreeSelection)
          refTreeSelection->select(iter);

        m_treeview_fields->scroll_to_row( Gtk::TreeModel::Path(iter) );

        treeview_fill_sequences(m_model_items, m_model_items->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
        */
      }
    }
  }
}

sharedptr<LayoutItem_CalendarPortal> Dialog_Layout_Calendar_Related::get_portal_layout()
{
  return m_portal;
}


} //namespace Glom

