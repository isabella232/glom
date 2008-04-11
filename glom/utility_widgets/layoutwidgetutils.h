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

#ifndef _GLOM_LAYOUT_WIDGET_UTILS_H_
#define _GLOM_LAYOUT_WIDGET_UTILS_H_

#include <gtkmm.h>
#include "layoutwidgetbase.h"

namespace Glom
{

class LayoutWidgetUtils : public LayoutWidgetBase
{
public:
  LayoutWidgetUtils();
  virtual ~LayoutWidgetUtils();
  
protected:
  void setup_util_menu();
  Gtk::Menu* m_pPopupMenuUtils;
  
  virtual void on_menu_properties_activate() = 0;

  // This one is implemented here
  virtual void on_menu_delete_activate();
  
  Glib::RefPtr<Gtk::Action> m_refUtilProperties;
  Glib::RefPtr<Gtk::Action> m_refUtilDelete;  
  Glib::RefPtr<Gtk::ActionGroup> m_refActionGroup;
  Glib::RefPtr<Gtk::UIManager> m_refUIManager;
};

} // namespace Glom

#endif // _GLOM_LAYOUT_WIDGET_UTILS_H_
