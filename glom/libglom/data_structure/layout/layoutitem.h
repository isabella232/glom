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

#ifndef GLOM_DATASTRUCTURE_LAYOUTITEM_H
#define GLOM_DATASTRUCTURE_LAYOUTITEM_H

#include "../translatable_item.h"
#include <glibmm/ustring.h>

namespace Glom
{

class LayoutItem : public TranslatableItem
{
public:

  LayoutItem();
  LayoutItem(const LayoutItem& src);
  LayoutItem& operator=(const LayoutItem& src);
  virtual ~LayoutItem();

  /** Create a new copied instance.
   * This allows us to deep-copy a list of LayoutItems.
   */
  virtual LayoutItem* clone() const = 0;

  bool operator==(const LayoutItem& src) const;

  virtual bool get_editable() const;
  virtual void set_editable(bool val);

  virtual Glib::ustring get_layout_display_name() const;
  virtual Glib::ustring get_part_type_name() const = 0;

  /** Gets the node name to use for the intermediate XML,
   * (and usually, the CSS style class to use for the resulting HTML).
   */
  virtual Glib::ustring get_report_part_id() const;

  bool get_display_width(guint& width) const;
  void set_display_width(guint value);

  /// This is used only for the print layouts.
  void get_print_layout_position(double& x, double& y, double& width, double& height) const;

  /// This is used only for the print layouts.
  void set_print_layout_position(double x, double y, double width, double height);

  /// This is used only for the print layouts.
  void set_print_layout_text_size(double points);

  /// This is used only for the print layouts.
  double get_print_layout_text_size() const;

  guint m_sequence;
  //bool m_hidden;

protected:
  Glib::ustring m_name;
  bool m_editable;

  //Not saved in document:
  guint m_display_width; //In pixels.

  void instantiate_positions() const;

  // These are used only for the print layouts.
  // We put them in a separate member class that's only instantiated when necessary,
  // so that we only waste one pointer instead of several doubles when we don't need them.
  class PrintLayoutPosition
  {
  public:
    PrintLayoutPosition();
    PrintLayoutPosition(const PrintLayoutPosition& src);
    PrintLayoutPosition& operator=(const PrintLayoutPosition& src);

    bool operator==(const PrintLayoutPosition& src) const;

    double m_x;
    double m_y;
    double m_width;
    double m_height;

    //Used only for items that contain text:
    double m_text_size; //In points.
  };
  
  mutable PrintLayoutPosition* m_positions;
};

} //namespace Glom

#endif //GLOM_DATASTRUCTURE_LAYOUTITEM_H



