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
#include "entryglom.h"
#include "../data_structure/field.h"
#include <map>

class FlowTableWithFields : public FlowTable
{
public: 
  FlowTableWithFields();
  virtual ~FlowTableWithFields();

  /** Add a field.
   * @param title The title to show to the left of the entry.
   * @param id The unique identifier for this field, to use with get_field().
   * @param group The title of the group in which this field should be shown, if any.
   */
  virtual void add_field(const Field& field, const Glib::ustring& group = "");
  virtual void remove_field(const Glib::ustring& id); 
  virtual EntryGlom* get_field(const Glib::ustring& id);
  virtual EntryGlom* get_field(const Field& field);
    
  virtual void change_group(const Glib::ustring& id, const Glib::ustring& new_group);

  virtual void remove_all();

  /** For intance,
   * void on_flowtable_field_edited(Glib::ustring id);
   */
  typedef sigc::signal<void, Glib::ustring> type_signal_field_edited;
  type_signal_field_edited signal_field_edited();


protected:

  int get_suitable_width(Field::glom_field_type field_type);
  void on_entry_edited( const Glib::ustring& id);

  class Info
  {
  public:
    Field m_field; //Store the field information so we know the title, ID, and type.
    Glib::ustring m_group;
    Gtk::Alignment* m_first;
    EntryGlom* m_second;
  };

  typedef std::map<Glib::ustring, Info> type_mapFields; //Map of IDs to full info.
  type_mapFields m_mapFields;

  type_signal_field_edited m_signal_field_edited;
};

#endif //GLOM_UTILITYWIDGETS_FLOWTABLEWITHFIELDS_H
