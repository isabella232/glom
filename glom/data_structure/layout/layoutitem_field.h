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
 
#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H

#include "layoutitem.h"
#include "../field.h"

class LayoutItem_Field : public LayoutItem
{
public:

  LayoutItem_Field();
  LayoutItem_Field(const LayoutItem_Field& src);
  LayoutItem_Field& operator=(const LayoutItem_Field& src);
  virtual ~LayoutItem_Field();

  virtual LayoutItem* clone() const;

  //virtual Glib::ustring get_table_name() const;
  //virtual void set_table_name(const Glib::ustring& table_name);

  virtual bool get_has_relationship_name() const;
  virtual Glib::ustring get_relationship_name() const;
  virtual void set_relationship_name(const Glib::ustring& relationship_name);

  //This is filled in by looking at the database structure:
  Field m_field;
  //TODO: This might occasionally be different on different layouts: Glib::ustring m_title;

  bool get_editable_and_allowed() const;

  //Not saved to the document:
  bool m_priv_view;
  bool m_priv_edit;

protected:
  //Glib::ustring m_table_name; //If m_relationship_name is not used.
  Glib::ustring m_relationship_name;
};

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H



