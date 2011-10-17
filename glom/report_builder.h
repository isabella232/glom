/* Glom
 *
 * Copyright (C) 2001-2006 Murray Cumming
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

#ifndef GLOM_REPORT_BUILDER_H
#define GLOM_REPORT_BUILDER_H

#include <glom/base_db.h>
#include <libglom/data_structure/report.h>
#include <libglom/sharedptr.h>

namespace Glom
{

class ReportBuilder : public Base_DB
{
public:
  ReportBuilder();
  virtual ~ReportBuilder();

  static sharedptr<Report> create_standard_list_report(const Document* document, const Glib::ustring& table_name);

  //void set_report(const Glib::ustring& table_name, const sharedptr<const Report>& report);
  //sharedptr<Report> get_report();

  /**
   * @result The filepath of the generated HTML file.
   */
  std::string report_build(const FoundSet& found_set, const sharedptr<const Report>& report);
 
private:

  void report_build_groupby(const FoundSet& found_set_parent, xmlpp::Element& parent_node, const sharedptr<LayoutItem_GroupBy>& group_by);
  void report_build_groupby_children(const FoundSet& found_set, xmlpp::Element& nodeGroupBy, const sharedptr<LayoutItem_GroupBy>& group_by);
  void report_build_summary(const FoundSet& found_set_parent, xmlpp::Element& parent_node, const sharedptr<LayoutItem_Summary>& summary);
  void report_build_headerfooter(const FoundSet& found_set, xmlpp::Element& parent_node, const sharedptr<LayoutGroup>& group);

  typedef std::vector< sharedptr<LayoutItem> > type_vecLayoutItems;

  void report_build_records(const FoundSet& found_set, xmlpp::Element& parent_node, const type_vecLayoutItems& items, bool one_record_only = false);
  void report_build_records_get_fields(const FoundSet& found_set, const sharedptr<LayoutGroup>& group, type_vecLayoutFields& items);
  void report_build_records_field(const FoundSet& found_set, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Field>& field, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& colField, bool vertical = false);
  void report_build_records_text(const FoundSet& found_set, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Text>& textobject, bool vertical = false);
  void report_build_records_image(const FoundSet& found_set, xmlpp::Element& nodeParent, const sharedptr<const LayoutItem_Image>& imageobject, bool vertical = false);
  void report_build_records_vertical_group(const FoundSet& found_set, xmlpp::Element& vertical_group_node, const sharedptr<LayoutItem_VerticalGroup>& group, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& field_index);
};

} //namespace Glom

#endif // GLOM_REPORT_BUILDER_H
