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

#ifndef GLOM_DATASTRUCTURE_FIELDFORMATTING_H
#define GLOM_DATASTRUCTURE_FIELDFORMATTING_H

#include <libglom/data_structure/layout/usesrelationship.h>
#include <libglom/data_structure/numeric_format.h>
#include <libglom/data_structure/relationship.h>
#include <libglom/sharedptr.h>
#include <libgdamm.h>

namespace Glom
{

//TODO: This should probably be renamed to Formatting, because it is used for static text items too.
class FieldFormatting : public UsesRelationship //The UsesRelationship base has the relationship for the choices.
{
public:

  FieldFormatting();
  FieldFormatting(const FieldFormatting& src);
  FieldFormatting& operator=(const FieldFormatting& src);
  virtual ~FieldFormatting();

  bool operator==(const FieldFormatting& src) const;

  bool get_has_choices() const;

  bool get_has_related_choices() const;
  void set_has_related_choices(bool val = true);

  bool get_has_custom_choices() const;
  void set_has_custom_choices(bool val = true);

  typedef std::list<Gnome::Gda::Value> type_list_values;
  virtual type_list_values get_choices_custom() const;
  virtual void set_choices_custom(const type_list_values& choices);

  /** Discover whether the entered data should only be one of the available 
   * choices.
   * @param [out] as_radio_buttons: Whether the choices should be displayed as 
   * radio buttons instead of a combo box.
   */
  bool get_choices_restricted(bool& as_radio_buttons) const;

  /** See get_choices_restricted().
   */
  void set_choices_restricted(bool val = true, bool as_radio_buttons = false);

  void get_choices(sharedptr<const Relationship>& relationship_name, Glib::ustring& field, Glib::ustring& field_second, bool& show_all) const;
  void set_choices(const sharedptr<const Relationship>& relationship_name, const Glib::ustring& field, const Glib::ustring& field_second, bool show_all);

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

  enum HorizontalAlignment
  {
    HORIZONTAL_ALIGNMENT_AUTO, //For instance, RIGHT for numeric fields.
    HORIZONTAL_ALIGNMENT_LEFT,
    HORIZONTAL_ALIGNMENT_RIGHT
  };

  void set_horizontal_alignment(HorizontalAlignment alignment);
  HorizontalAlignment get_horizontal_alignment() const;

  void change_field_name(const Glib::ustring& table_name, const Glib::ustring& field_name, const Glib::ustring& field_name_new);

  NumericFormat m_numeric_format; //Only used for numeric fields.

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

  Glib::ustring m_choices_related_field, m_choices_related_field_second;
  bool m_choices_related_show_all;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_FIELDFORMATTING_H



