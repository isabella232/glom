/* Glom
 *
 * Copyright (C) 2001-2005 Murray Cumming
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

#ifndef GLOM_MODE_DATA_LAYOUT_WIDGET_FIELD_H
#define GLOM_MODE_DATA_LAYOUT_WIDGET_FIELD_H

#include "layoutwidgetbase.h"

namespace Glom
{

class LayoutWidgetField : public LayoutWidgetBase
{
public: 
  LayoutWidgetField();
  virtual ~LayoutWidgetField();

  virtual void set_value(const Gnome::Gda::Value& value) = 0;

  virtual Gnome::Gda::Value get_value() const = 0;

  /**Whether this widget still has the original entered data, instead of just a representation.
   * For intance, and image widget might only store a preview.
   */
  virtual bool get_has_original_data() const;

  typedef sigc::signal<void> type_signal_edited;
  type_signal_edited signal_edited();

protected:
  type_signal_edited m_signal_edited;
  bool m_entered_data_stored;
};

} //namespace Glom

#endif //GLOM_MODE_DATA_LAYOUT_WIDGET_FIELD_H
