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

#include "dialog_user.h"
#include <glom/frame_glom.h> //For Frame_Glom::show_ok_dialog().
//#include <libgnome/gnome-i18n.h>
#include <glibmm/i18n.h>

namespace Glom
{

Dialog_User::Dialog_User(BaseObjectType* cobject, const Glib::RefPtr<Gnome::Glade::Xml>& refGlade)
: Gtk::Dialog(cobject)
{
  //refGlade->get_widget("label_table_name", m_label_table_name);

  refGlade->get_widget("entry_user", m_entry_user);
  refGlade->get_widget_derived("combobox_group", m_combo_group);
  refGlade->get_widget("entry_password", m_entry_password);
  refGlade->get_widget("entry_password_confirm", m_entry_password_confirm);

  show_all_children();
}

Dialog_User::~Dialog_User()
{
}

bool Dialog_User::check_password()
{
  if(m_entry_password->get_text() != m_entry_password_confirm->get_text())
  {
    Frame_Glom::show_ok_dialog(_("Passwords Do Not Match"), _("The entered password does not match the entered password confirmation. Please try again."), *this, Gtk::MESSAGE_ERROR);
    return false;
  }
  else if(m_entry_password->get_text().empty())
  {
    Frame_Glom::show_ok_dialog(_("Password Is Empty"), _("Please enter a password this for this user."), *this, Gtk::MESSAGE_ERROR);
    return false;
  }
  else
    return true;
}

/*
void Dialog_User::set_document(const Glib::ustring& layout, Document_Glom* document, const Glib::ustring& table_name, const type_vecLayoutFields& table_fields)
{
  m_modified = false;

  Dialog_Layout::set_document(layout, document, table_name, table_fields);


  //Update the tree models from the document
  if(document)
  {
    //Set the table name and title:
    m_label_table_name->set_text(table_name);
    m_entry_table_title->set_text( document->get_table_title(table_name) );

    Document_Glom::type_mapLayoutGroupSequence mapGroups = document->get_data_layout_groups_plus_new_fields(layout, m_table_name);

    //If no information is stored in the document, then start with something:

    if(mapGroups.empty())
    {
      LayoutGroup group;
      group.set_name("main");
      group.m_columns_count = 2;

      guint field_sequence = 1; //0 means no sequence
      for(type_vecLayoutFields::const_iterator iter = table_fields.begin(); iter != table_fields.end(); ++iter)
      {
        LayoutItem_Field item = *iter;
        item.m_sequence = field_sequence;

        group.add_item(item, field_sequence);

        ++field_sequence;
      }

      mapGroups[1] = group;
    }

    //Show the field layout
    typedef std::list< Glib::ustring > type_listStrings;

    m_model_items->clear();

    //guint field_sequence = 1; //0 means no sequence
    //guint group_sequence = 1; //0 means no sequence
    for(Document_Glom::type_mapLayoutGroupSequence::const_iterator iter = mapGroups.begin(); iter != mapGroups.end(); ++iter)
    {
      sharedptr<const LayoutGroup> group = iter->second;

      add_group(Gtk::TreeModel::iterator(), group);
    }

    //treeview_fill_sequences(m_model_items, m_model_items->m_columns.m_col_sequence); //The document should have checked this already, but it does not hurt to check again.
  }

  //Open all the groups:
  m_treeview_users->expand_all();

  m_modified = false;

}
*/

} //namespace Glom











