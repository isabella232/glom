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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#ifndef GLOM_REPORT_BUILDER_H
#define GLOM_REPORT_BUILDER_H

#include <libglom/data_structure/report.h>
#include <libglom/document/document.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_summary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_fieldsummary.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_verticalgroup.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_header.h>
#include <libglom/data_structure/layout/report_parts/layoutitem_footer.h>

namespace Glom
{

class ReportBuilder
{
public:
  explicit ReportBuilder(const std::locale& locale);

  virtual ~ReportBuilder();

  static std::shared_ptr<Report> create_standard_list_report(const Document* document, const Glib::ustring& table_name);

  //TODO: Remove set_document() and get_document()?
  void set_document(Document* document);

  //void set_report(const Glib::ustring& table_name, const std::shared_ptr<const Report>& report);
  //std::shared_ptr<Report> get_report();

  /**
   * @result The HTML of the generated report.
   */
  Glib::ustring report_build(const FoundSet& found_set, const std::shared_ptr<const Report>& report);

  /**
   * @result The filepath of a temporary file containing the generated HTML file.
   */
  std::string report_build_and_save(const FoundSet& found_set, const std::shared_ptr<const Report>& report);
 
 
private:

  bool report_build_groupby(const FoundSet& found_set_parent, xmlpp::Element& parent_node, const std::shared_ptr<LayoutItem_GroupBy>& group_by);
  bool report_build_groupby_children(const FoundSet& found_set, xmlpp::Element& nodeGroupBy, const std::shared_ptr<LayoutItem_GroupBy>& group_by);
  bool report_build_summary(const FoundSet& found_set_parent, xmlpp::Element& parent_node, const std::shared_ptr<LayoutItem_Summary>& summary);
  bool report_build_headerfooter(const FoundSet& found_set, xmlpp::Element& parent_node, const std::shared_ptr<LayoutGroup>& group);

  typedef std::vector< std::shared_ptr<LayoutItem> > type_vecLayoutItems;
  typedef std::vector< std::shared_ptr<LayoutItem_Field> > type_vecLayoutFields;

  bool report_build_records(const FoundSet& found_set, xmlpp::Element& parent_node, const type_vecLayoutItems& items, bool one_record_only = false);
  bool report_build_records_get_fields(const FoundSet& found_set, const std::shared_ptr<LayoutGroup>& group, type_vecLayoutFields& items);
  bool report_build_records_field(const FoundSet& found_set, xmlpp::Element& nodeParent, const std::shared_ptr<const LayoutItem_Field>& field, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& colField, bool vertical = false);
  bool report_build_records_text(const FoundSet& found_set, xmlpp::Element& nodeParent, const std::shared_ptr<const LayoutItem_Text>& textobject, bool vertical = false);
  bool report_build_records_image(const FoundSet& found_set, xmlpp::Element& nodeParent, const std::shared_ptr<const LayoutItem_Image>& imageobject, bool vertical = false);
  bool report_build_records_vertical_group(const FoundSet& found_set, xmlpp::Element& vertical_group_node, const std::shared_ptr<LayoutItem_VerticalGroup>& group, const Glib::RefPtr<Gnome::Gda::DataModel>& datamodel, guint row, guint& field_index);

  Document* get_document();

  Document* m_document;

  std::locale m_locale; //For use with GlomConversions
  Glib::ustring m_locale_id; //To get the appropriate translations.
};

} //namespace Glom

#endif // GLOM_REPORT_BUILDER_H
