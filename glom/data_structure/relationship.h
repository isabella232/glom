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

#ifndef RELATIONSHIP_H
#define RELATIONSHIP_H


#include <glibmm/ustring.h>

class Relationship
{
public: 
  Relationship();
  Relationship(const Relationship& src);
  virtual ~Relationship();

  Relationship& operator=(const Relationship& src);

  bool operator==(const Relationship& src) const;

  virtual bool get_name_not_empty() const; //For performance.
  virtual Glib::ustring get_name() const;
  virtual void set_name(const Glib::ustring& strVal);

  virtual Glib::ustring get_title() const;
  virtual void set_title(const Glib::ustring& strVal);

  Glib::ustring get_title_or_name() const;

  virtual Glib::ustring get_from_table() const;
  virtual Glib::ustring get_from_field() const;
  virtual Glib::ustring get_to_table() const;
  virtual Glib::ustring get_to_field() const;

  virtual void set_from_table(const Glib::ustring& strVal);
  virtual void set_from_field(const Glib::ustring& strVal);
  virtual void set_to_table(const Glib::ustring& strVal);
  virtual void set_to_field(const Glib::ustring& strVal);

  ///Whether related records will be created automatically.
  virtual bool get_auto_create() const;
  virtual void set_auto_create(bool val);

protected:
  Glib::ustring m_strName, m_strTitle;

  Glib::ustring m_strFrom_Table;
  Glib::ustring m_strFrom_Field;
  Glib::ustring m_strTo_Table;
  Glib::ustring m_strTo_Field;
  bool m_auto_create;
};

#endif
