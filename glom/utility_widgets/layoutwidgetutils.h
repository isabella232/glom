/*
 * glom
 * 
 * glom is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * glom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with glom.  If not, write to:
 *   The Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor
 *   Boston, MA  02110-1301, USA.
 */

#ifndef GLOM_UTILITY_WIDGETS_LAYOUT_WIDGET_UTILS_H
#define GLOM_UTILITY_WIDGETS_LAYOUT_WIDGET_UTILS_H

#include <gtkmm/widget.h>
#include <gtkmm/menu.h>
#include <giomm/simpleactiongroup.h>
#include "layoutwidgetbase.h"

namespace Glom
{

class LayoutWidgetUtils : public LayoutWidgetBase
{
public:
  LayoutWidgetUtils();
  
protected:
  void setup_util_menu(Gtk::Widget* widget);

  Gtk::Menu* m_pPopupMenuUtils;
#ifndef GLOM_ENABLE_CLIENT_ONLY
  virtual void on_menu_properties_activate();

  // This one is implemented here:
  virtual void on_menu_delete_activate();
#endif // !GLOM_ENABLE_CLIENT_ONLY

private:  
  Glib::RefPtr<Gio::SimpleAction> m_refUtilProperties;
  Glib::RefPtr<Gio::SimpleAction> m_refUtilDelete;  
  Glib::RefPtr<Gio::SimpleActionGroup> m_refActionGroup;
};

} // namespace Glom

#endif // GLOM_UTILITY_WIDGETS_LAYOUT_WIDGET_UTILS_H
