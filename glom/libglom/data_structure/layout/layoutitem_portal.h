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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_PORTAL_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_PORTAL_H

#include <libglom/data_structure/layout/layoutgroup.h>
#include <libglom/data_structure/field.h>
#include <libglom/data_structure/relationship.h>
//#include <libglom/data_structure/has_title_singular.h>

namespace Glom
{

class Document; //For the utility functions.

/**
 * get_title() returns either the title of the Field or the CustomTitle.
 * You should not call get/set_title_original() or get/set_title_translation()
 * on items of this type.
 */
class LayoutItem_Portal
: public LayoutGroup,
  public UsesRelationship
  
  //TODO: Allow portals to have custom titles that override the relationship titles?
  //public HasTitleSingular
{
public:

  LayoutItem_Portal();
  LayoutItem_Portal(const LayoutItem_Portal& src);
  LayoutItem_Portal(LayoutItem_Portal&& src) = delete;
  LayoutItem_Portal& operator=(const LayoutItem_Portal& src);
  LayoutItem_Portal& operator=(LayoutItem_Portal&& src) = delete;
  virtual ~LayoutItem_Portal();

  LayoutItem* clone() const override;

  Glib::ustring get_title(const Glib::ustring& locale) const noexcept override;
  Glib::ustring get_title_or_name(const Glib::ustring& locale) const noexcept override;

  //We do not implement this, because we only ever want to use the method
  //in the derived class.
  //Glib::ustring get_part_type_name() const override;

  void change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new) override;
  void change_related_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new) override;

  ///A helper method to avoid extra ifs to avoid null dereferencing.
  Glib::ustring get_from_table() const;

  //virtual void debug(guint level = 0) const;

  /** Gets the relationship to use for navigation if get_navigation_type() is 
   * NAVIGATION_NONE.
   */
  std::shared_ptr<UsesRelationship> get_navigation_relationship_specific();

  /** Get the @a relationship to use for navigation if get_navigation_type() is 
   * NAVIGATION_NONE.
   */
  std::shared_ptr<const UsesRelationship> get_navigation_relationship_specific() const;

  /** Set the @a relationship to use for navigation if get_navigation_type() is 
   * NAVIGATION_NONE.
   */
  void set_navigation_relationship_specific(const std::shared_ptr<UsesRelationship>& relationship);

  void reset_navigation_relationship();

  /** The navigation (if any) that should be used when the user 
   * activates a related record row.
   */
  enum class navigation_type
  {
    NONE, /**< No navigation will be offered. */
    AUTOMATIC, /**< The destination related table will be chosen automatically based on the relationship and the visible fields. */
    SPECIFIC /**< The destination related table will be determined by a specified relationship. */
  };

  /** Discover what @a type (if any) navigation should be used when the user 
   * activates a related record row.
   */
  navigation_type get_navigation_type() const;

  /** Set what @a type (if any) navigation should be used when the user 
   * activates a related record row.
   */
  void set_navigation_type(navigation_type type);
  
  /** Discover what table to show when clicking on a related record.
   * This table will not necessarily just be the directly related table.
   * The caller should check, in the document, that the returned @a table_name is not hidden.
   *
   * @param table_name The table that should be shown.
   * @param relationship The relationship in the directly related table that should be used to get to that table. If this is empty then we should just show the table directly.
   */
  void get_suitable_table_to_view_details(Glib::ustring& table_name, std::shared_ptr<const UsesRelationship>& relationship, const Document* document) const;

  /** Get the relationship (from the related table) into which the row button should navigate,
   * or none if it should use the portal's directly related table itself.
   * (If that should be chosen automatically, by looking at the fields in the portal.)
   */
  std::shared_ptr<const UsesRelationship> get_portal_navigation_relationship_automatic(const Document* document) const;

  /// This is used only for the print layouts.
  double get_print_layout_row_height() const;

  /// This is used only for the print layouts.
  void set_print_layout_row_height(double row_height);


  /// This is used only for the print layouts.
  double get_print_layout_row_line_width() const;
    
  /// This is used only for the print layouts.
  void set_print_layout_row_line_width(double width);
  
  /// This is used only for the print layouts.
  double get_print_layout_column_line_width() const;
  
  /// This is used only for the print layouts.
  void set_print_layout_column_line_width(double width);
  
  /// This is used only for the print layouts.
  Glib::ustring get_print_layout_line_color() const;
  
  /// This is used only for the print layouts.
  void set_print_layout_line_color(const Glib::ustring& color);
  
  /** Get the number of rows that should be displayed.
   */
  void get_rows_count(gulong& rows_count_min, gulong& rows_count_max) const;
  
  /** Set the number of rows that should be displayed.
   */
  void set_rows_count(gulong rows_count_min, gulong rows_count_max);


private:

  std::shared_ptr<const LayoutItem_Field> get_field_is_from_non_hidden_related_record(const Document* document) const;
  std::shared_ptr<const LayoutItem_Field> get_field_identifies_non_hidden_related_record(std::shared_ptr<const Relationship>& used_in_relationship, const Document* document) const;


  std::shared_ptr<UsesRelationship> m_navigation_relationship_specific;

  // This is used only for the print layouts.
  double m_print_layout_row_height;
  double m_print_layout_row_line_width, m_print_layout_column_line_width;
  Glib::ustring m_print_layout_line_color; //TODO: Patch GooCanvasTable to allow different colors for rows and columns?

  //If no navigation relationship has been specified then it will be automatically chosen or navigation will be disabled:
  navigation_type m_navigation_type;
  
  gulong m_rows_count_min, m_rows_count_max;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_PORTAL_H

