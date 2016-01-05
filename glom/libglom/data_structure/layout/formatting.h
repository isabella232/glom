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

#ifndef GLOM_DATASTRUCTURE_FIELDFORMATTING_H
#define GLOM_DATASTRUCTURE_FIELDFORMATTING_H

#include <libglom/data_structure/layout/usesrelationship.h>
#include <libglom/data_structure/numeric_format.h>
#include <libglom/data_structure/relationship.h>
#include <libglom/data_structure/choicevalue.h>
#include <libgdamm/value.h>
#include <vector>

namespace Glom
{

class LayoutItem_Field;
class LayoutGroup;

/** This specifies how to display data for fields or static text items.
 */
class Formatting : public UsesRelationship //The UsesRelationship base has the relationship for the choices.
{
public:

  Formatting();
  Formatting(const Formatting& src);
  Formatting(Formatting&& src) = delete;
  Formatting& operator=(const Formatting& src);
  Formatting& operator=(Formatting&& src) = delete;

  bool operator==(const Formatting& src) const;

  bool get_has_choices() const;

  bool get_has_related_choices() const;
  bool get_has_related_choices(bool& show_all, bool& with_second) const;
  void set_has_related_choices(bool val = true);

  bool get_has_custom_choices() const;
  void set_has_custom_choices(bool val = true);

  typedef std::vector< std::shared_ptr<ChoiceValue> > type_list_values;
  type_list_values get_choices_custom() const;
  void set_choices_custom(const type_list_values& choices);

  /** Get the original text that corresponds to the translated choice for the 
   * current locale.
   */
  Glib::ustring get_custom_choice_original_for_translated_text(const Glib::ustring& text, const Glib::ustring& locale = Glib::ustring()) const;

  /** Get the translated choice text, for the 
   * current locale, that corresponds to the original text .
   */
  Glib::ustring get_custom_choice_translated(const Glib::ustring& original_text, const Glib::ustring& locale = Glib::ustring()) const;

  typedef std::pair< std::shared_ptr<const LayoutItem_Field>, bool /* is_ascending */> type_pair_sort_field;
  typedef std::vector<type_pair_sort_field> type_list_sort_fields;

  /** Discover whether the entered data should only be one of the available
   * choices.
   * @param [out] as_radio_buttons: Whether the choices should be displayed as
   * radio buttons instead of a combo box.
   */
  bool get_choices_restricted(bool& as_radio_buttons) const;

  /** See get_choices_restricted().
   */
  void set_choices_restricted(bool val = true, bool as_radio_buttons = false);

  //TODO: Add a ChoicesRelated class?

  
  void get_choices_related(std::shared_ptr<const Relationship>& relationship, std::shared_ptr<LayoutItem_Field>& field, std::shared_ptr<LayoutGroup>& extra_layout, type_list_sort_fields& sort_fields, bool& show_all);
  void get_choices_related(std::shared_ptr<const Relationship>& relationship, std::shared_ptr<const LayoutItem_Field>& field, std::shared_ptr<const LayoutGroup>& extra_layout, type_list_sort_fields& sort_fields, bool& show_all) const;
  void set_choices_related(const std::shared_ptr<const Relationship>& relationship_name, const std::shared_ptr<LayoutItem_Field>& field, const std::shared_ptr<LayoutGroup>& extra_layout, const type_list_sort_fields& sort_fields, bool show_all);

  //Just for convenience:
  std::shared_ptr<const Relationship> get_choices_related_relationship(bool& show_all) const;




  /** Get whether the text should be displayed with multiple lines in the
   * details view. Text is displayed with a single line in the list view.
   * @returns whether the text should be displayed with multiple lines
   */
  bool get_text_format_multiline() const;

  /** Set whether the text should be displayed with multiple lines in the
   * details view. Text is displayed with a single line in the list view.
   * @param[in] value whether the text should be displayed with multiple lines
   */
  void set_text_format_multiline(bool value = true);

  /** Get the number of lines of text that should be displayed.
   * @see get_text_format_multiline()
   * @returns the number of lines of text
   */
  guint get_text_format_multiline_height_lines() const;

  /** Get the number of lines of text that should be displayed.
   * @returns the number of lines of text
   */
  void set_text_format_multiline_height_lines(guint value);

  /** Set the font description, as returned from
   * Gtk::FontButton::get_font_name(), which may include the size and style.
   * @param font_desc a Pango font description string
   */
  void set_text_format_font(const Glib::ustring& font_desc);

  /** Get the font description, as returned from
   * Gtk::FontButton::get_font_name(), which may include the size and style.
   * @returns a Pango font description string
   */
  Glib::ustring get_text_format_font() const;

  /** Set the foreground color to use for text when displaying a field value.
   * @param[in] color the text foreground color, in a format recognised by
   * XParseColor
   */
  void set_text_format_color_foreground(const Glib::ustring& color);

  /** Get the foreground color to use for text for the specified value,
   * taking the negative-color into account, if specified.
   * @returns the text foreground color, in a format recognised by XParseColor
   */
  Glib::ustring get_text_format_color_foreground_to_use(const Gnome::Gda::Value& value) const;

  /** Get the foreground color to use for text when displaying a field value.
   * This should be overriden by
   * m_numeric_formatting.m_foreground_color_for_negatives if that is active.
   * @returns the text foreground color, in a format recognised by XParseColor
   */
  Glib::ustring get_text_format_color_foreground() const;

  /** Set the background color to use for text when displaying a field value.
   * @param[in] color a text background color, in a format recognised by
   * XParseColor
   */
  void set_text_format_color_background(const Glib::ustring& color);

  /** Get the background color to use for text when displaying a field value.
   * @returns the text background color, in a format recognised by XParseColor
   */
  Glib::ustring get_text_format_color_background() const;

  enum class HorizontalAlignment
  {
    AUTO, //For instance, RIGHT for numeric fields.
    LEFT,
    RIGHT
  };

  void set_horizontal_alignment(HorizontalAlignment alignment);
  HorizontalAlignment get_horizontal_alignment() const;

  NumericFormat m_numeric_format; //Only used for numeric fields.

  /** Adapt to a change of field name,
   * so this Formatting does not refer to any field that no longer exists.
   *
   * @result true if something was changed.
   */
  bool change_field_item_name(const Glib::ustring& table_name, const Glib::ustring& field_name_old, const Glib::ustring& field_name_new);

private:

  type_list_values m_choices_custom_list; //A drop-down list of possible values for the field.
  bool m_choices_restricted;
  bool m_choices_restricted_as_radio_buttons;
  bool m_choices_custom, m_choices_related;

  bool m_text_format_multiline;
  guint m_text_multiline_height_lines; //The height in number of lines of text.
  //Glib::ustring m_text_multiline_width_example; //An example string from which to calculate the width.
  Glib::ustring m_text_font;
  Glib::ustring m_text_color_foreground, m_text_color_background;
  HorizontalAlignment m_horizontal_alignment;

  std::shared_ptr<LayoutItem_Field> m_choices_related_field;
  std::shared_ptr<LayoutGroup> m_choices_extra_layout_group;
  type_list_sort_fields m_choices_related_sort_fields;
  bool m_choices_related_show_all;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_FIELDFORMATTING_H
