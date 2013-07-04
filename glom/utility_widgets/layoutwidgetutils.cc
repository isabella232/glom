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
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "layoutwidgetutils.h"
#include <gtkmm/builder.h>
#include <giomm/menu.h>
#include <glibmm/i18n.h>
#include <iostream>

namespace Glom
{
  
LayoutWidgetUtils::LayoutWidgetUtils() :
  m_pPopupMenuUtils(0)
{
  //Derived class's constructors must call this:
  //setup_util_menu(this);
}

LayoutWidgetUtils::~LayoutWidgetUtils()
{
	
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void LayoutWidgetUtils::setup_util_menu(Gtk::Widget* widget)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refUtilProperties = m_refActionGroup->add_action("properties",
    sigc::mem_fun(*this, &LayoutWidgetUtils::on_menu_properties_activate) );
  m_refUtilDelete = m_refActionGroup->add_action("delete",
    sigc::mem_fun(*this, &LayoutWidgetUtils::on_menu_delete_activate) );
  
  widget->insert_action_group("utility", m_refActionGroup);

  Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
  menu->append(_("Properties"), "context.properties");
  menu->append(_("_Delete"), "context.delete");

  m_pPopupMenuUtils = new Gtk::Menu(menu);
  m_pPopupMenuUtils->attach_to_widget(*widget);
#endif
}

void LayoutWidgetUtils::on_menu_delete_activate()
{
  Gtk::Widget* parent = dynamic_cast<Gtk::Widget*>(this);
  if(!parent)
  {
    // Should never happen!
    std::cerr << G_STRFUNC << ": LayoutWidgetUtils is no Gtk::Widget" << std::endl;
    return;
  }

  LayoutWidgetBase* base = 0;
  do
  {
    parent = parent->get_parent();
    base = dynamic_cast<LayoutWidgetBase*>(parent);
    if(base)
    {
      break;
    }
  } while (parent);

  if(base)
  {
    std::shared_ptr<LayoutGroup> group = 
      std::dynamic_pointer_cast<LayoutGroup>(base->get_layout_item());
    if(!group)
      return;

    group->remove_item(get_layout_item());
    signal_layout_changed().emit();
  }
}

void LayoutWidgetUtils::on_menu_properties_activate()
{
  //This is not pure virtual, so we can easily use this base class in unit tests.
  std::cerr << G_STRFUNC << ": Not imlemented. Derived classes should override this." << std::endl;
}

#endif // !GLOM_ENABLE_CLIENT_ONLY

} // namespace Glom

