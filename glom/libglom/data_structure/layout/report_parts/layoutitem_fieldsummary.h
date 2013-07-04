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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_FIELDSUMMARY_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_FIELDSUMMARY_H

#include <libglom/data_structure/layout/layoutitem_field.h>

namespace Glom
{

class LayoutItem_FieldSummary : public LayoutItem_Field
{
public:

  LayoutItem_FieldSummary();
  LayoutItem_FieldSummary(const LayoutItem_FieldSummary& src);
  LayoutItem_FieldSummary& operator=(const LayoutItem_FieldSummary& src);
  virtual ~LayoutItem_FieldSummary();

  virtual LayoutItem* clone() const;

  bool operator==(const LayoutItem_FieldSummary& src) const;

  virtual Glib::ustring get_part_type_name() const;
  virtual Glib::ustring get_report_part_id() const;

  enum summaryType
  {
    TYPE_INVALID,
    TYPE_SUM,
    TYPE_AVERAGE,
    TYPE_COUNT
  };

  summaryType get_summary_type() const;
  void set_summary_type(summaryType summary_type);

  /// Get the SQL command to use for this summary.
  Glib::ustring get_summary_type_sql() const;

  /// This is used when loading the XML document, because we use get_summary_type_sql() when writing it.
  void set_summary_type_from_sql(const Glib::ustring& summary_type);

  void set_field(const std::shared_ptr<LayoutItem_Field>& field);

  virtual Glib::ustring get_title(const Glib::ustring& locale) const;
  virtual Glib::ustring get_title_or_name(const Glib::ustring& locale) const;

  virtual Glib::ustring get_layout_display_name() const;

  Glib::ustring get_layout_display_name_field() const;

  static Glib::ustring get_summary_type_name(summaryType summary_type);

private:
  summaryType m_summary_type;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_FIELDSUMMARY_H



