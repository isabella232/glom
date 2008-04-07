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
#include <glibmm/i18n.h>
#include <iostream>

namespace Glom
{
  
LayoutWidgetUtils::LayoutWidgetUtils() :
  m_pPopupMenuUtils(0)
{
  m_refActionGroup = Gtk::ActionGroup::create();

  m_refActionGroup->add(Gtk::Action::create("UtilMenu", "Utility Menu") );
  m_refUtilProperties = Gtk::Action::create("UtilProperties", _("Properties"));
  m_refUtilDetails = Gtk::Action::create("UtilDetails", _("Details"));
  setup_util_menu();
}

LayoutWidgetUtils::~LayoutWidgetUtils()
{
	
}

void LayoutWidgetUtils::setup_util_menu()
{
  m_refUIManager = Gtk::UIManager::create();
	
  m_refActionGroup->add(m_refUtilProperties,
    sigc::mem_fun(*this, &LayoutWidgetUtils::on_menu_properties_activate) );
  m_refActionGroup->add(m_refUtilDetails,
    sigc::mem_fun(*this, &LayoutWidgetUtils::on_menu_details_activate) );
    
  m_refUIManager->insert_action_group(m_refActionGroup);

  try
  {
    Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='UtilMenu'>"
        "    <menuitem action='UtilProperties'/>"
        "    <menuitem action='UtilDetails'/>"
        "  </popup>"
        "</ui>";

    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_pPopupMenuUtils = dynamic_cast<Gtk::Menu*>( m_refUIManager->get_widget("/UtilMenu") ); 
  if(!m_pPopupMenuUtils)
    g_warning("menu not found");
}

} // namespace Glom

