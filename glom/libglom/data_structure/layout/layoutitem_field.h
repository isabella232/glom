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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H

#include <libglom/data_structure/layout/layoutitem_withformatting.h>
#include <libglom/data_structure/layout/usesrelationship.h>
#include <libglom/data_structure/field.h>
#include <libglom/data_structure/numeric_format.h>
#include <libglom/data_structure/relationship.h>
#include <libglom/data_structure/layout/custom_title.h>

namespace Glom
{

/** A LayoutItem that shows the data from a table field.
 * The field may be in a known table, or in a to table of a relationship
 * or related relatinoship. See UsesRelationship::get_relationship() and
 * UsesRelationship::get_related_relationship() in the base class.
 *
 * get_title() returns either the title of the Field or the CustomTitle.
 * You should not call get/set_title_original() or get/set_title_translation()
 * on items of this type.
 */
class LayoutItem_Field
 : public LayoutItem_WithFormatting,
   public UsesRelationship
{
public:

  LayoutItem_Field();
  LayoutItem_Field(const LayoutItem_Field& src) = default;
  LayoutItem_Field(LayoutItem_Field&& src) = delete;
  LayoutItem_Field& operator=(const LayoutItem_Field& src) = default;
  LayoutItem_Field& operator=(LayoutItem_Field&& src) = delete;

  LayoutItem* clone() const override;

  bool operator==(const LayoutItem_Field& src) const;

  /** Set the non-user-visible name of the field.
   */
  void set_name(const Glib::ustring& name) noexcept override;

  /** Get the non-user-visible name of the field.
   */
  Glib::ustring get_name() const noexcept override; //For use with our std::find_if() lambda.

  /** Get the user-visible title for the field, in the user's current locale.
   * This returns the name if no title is set.
   */
  Glib::ustring get_title(const Glib::ustring& locale) const noexcept override;

  /** Get the user-visible title for the field, in the user's current locale.
   */
  Glib::ustring get_title_or_name(const Glib::ustring& locale) const noexcept override;

  Glib::ustring get_title_or_name_no_custom(const Glib::ustring& locale) const;

  std::shared_ptr<const CustomTitle> get_title_custom() const;
  std::shared_ptr<CustomTitle> get_title_custom();
  void set_title_custom(const std::shared_ptr<CustomTitle>& title);

  //virtual Glib::ustring get_table_name() const;
  //virtual void set_table_name(const Glib::ustring& table_name);

  /** Get a text representation for the field, such as Relationship::FieldName.
   */
  Glib::ustring get_layout_display_name() const override;

  Glib::ustring get_part_type_name() const override;

  Glib::ustring get_report_part_id() const override;

  void set_full_field_details(const std::shared_ptr<const Field>& field);
  std::shared_ptr<const Field> get_full_field_details() const;

  ///Convenience function, to avoid use of get_full_field_details().
  Field::glom_field_type get_glom_type() const;

  //TODO: This might occasionally be different on different layouts: Glib::ustring m_title;


  bool get_editable_and_allowed() const;

  /// For extra fields, needed for SQL queries. The user should never be able to make an item hidden - he can just remove it.
  bool get_hidden() const;
  void set_hidden(bool val = true);

  //Not saved to the document:
  bool m_priv_view;
  bool m_priv_edit;

  /** Discover whether to use the default formatting for this field,
   * instead of some custom per-layout-item field formatting.
   */
  bool get_formatting_use_default() const;

  /** Specify whether to use the default formatting for this field,
   * instead of some custom per-layout-item field formatting.
   */
  void set_formatting_use_default(bool use_default = true);

  /** Get the field formatting used by this layout item, which
   * may be either custom field formatting or the default field formatting.
   */
  const Formatting& get_formatting_used() const override;

  /** Get the alignment for the formatting used (see get_formatting_used()),
   * choosing an appropriate alignment if it is set to HorizontalAlignment::AUTO.
   * Note that this never returns HorizontalAlignment::AUTO.
   *
   * @param for_details_view This can change the effect of HorizontalAlignment::AUTO.
   */
  Formatting::HorizontalAlignment get_formatting_used_horizontal_alignment(bool for_details_view = false) const override;

  /** A convenience method to discover whether the formatting that is used
   * has custom choices with the values restricted to those choices,
   * meaning that those choices could be translated.
   */
  bool get_formatting_used_has_translatable_choices() const;

  /** Compare the name, relationship, and related_relationship.
   */
  bool is_same_field(const LayoutItem_Field& field) const;

private:

  Glib::ustring get_title_no_custom(const Glib::ustring& locale) const;

  Glib::ustring get_title_no_custom_translation(const Glib::ustring& locale, bool fallback = true) const;


  //This is just a cache, filled in by looking at the database structure:
  std::shared_ptr<const Field> m_field;
  bool m_field_cache_valid; //Whetehr m_field is up-to-date.

  bool m_hidden;
  bool m_formatting_use_default;
  std::shared_ptr<CustomTitle> m_title_custom; //translatable.
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_FIELD_H



