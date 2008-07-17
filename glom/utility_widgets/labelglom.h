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

#ifndef GLOM_UTILITY_WIDGETS_LABEL_GLOM_H
#define GLOM_UTILITY_WIDGETS_LABEL_GLOM_H

#include <gtkmm.h>
#include "layoutwidgetbase.h"
#include "layoutwidgetutils.h"
#include <glom/libglom/data_structure/layout/layoutitem_button.h>
#include <libglademm.h>

namespace Glom
{

class App_Glom;

class LabelGlom
: public Gtk::EventBox,
  public LayoutWidgetUtils
{
public:
  explicit LabelGlom();
  explicit LabelGlom(const Glib::ustring& label, float xalign, float yalign, bool mnemonic = false);
  virtual ~LabelGlom();

protected:
  void init();

  virtual App_Glom* get_application();
    
  Gtk::Label m_label;
#ifndef GLOM_ENABLE_CLIENT_ONLY    
  virtual bool on_button_press_event(GdkEventButton *event);
  virtual void on_menu_properties_activate();
#endif // !GLOM_ENABLE_CLIENT_ONLY
    
};

} //namespace Glom

#endif //GLOM_UTILITY_WIDGETS_LABEL_GLOM_H

