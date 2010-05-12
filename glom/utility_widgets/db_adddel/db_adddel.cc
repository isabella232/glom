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

#include "db_adddel.h"
#include <algorithm> //For std::find.
#include <glibmm/i18n.h>
#include "../cellrendererlist/cellrendererlist.h"
#include "db_treeviewcolumn_glom.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/dialog_invalid_data.h>
#include <glom/application.h>
#include <glom/utils_ui.h>
#include "cellrenderer_buttonimage.h"
#include "cellrenderer_buttontext.h"
#include <glom/utils_ui.h> //For Utils::image_scale_keeping_ratio().
#include <libglom/db_utils.h>

#include <iostream> //For debug output.
#include <gtk/gtktreeview.h>
#include <gtk/gtkstock.h>

#ifdef GLOM_ENABLE_MAEMO
//TODO: Remove this when we don't need to call C hildon functions:
#include <hildon/hildon.h>
#endif

namespace Glom
{

DbAddDelColumnInfo::DbAddDelColumnInfo()
:
  m_editable(true),
  m_visible(true)
{
}

DbAddDelColumnInfo::DbAddDelColumnInfo(const DbAddDelColumnInfo& src)
: m_item(src.m_item),
  m_choices(src.m_choices),
  m_editable(src.m_editable),
  m_visible(src.m_visible)
{
}

DbAddDelColumnInfo& DbAddDelColumnInfo::operator=(const DbAddDelColumnInfo& src)
{
  m_item = src.m_item;
  m_choices = src.m_choices;
  m_editable = src.m_editable;
  m_visible = src.m_visible;

  return *this;
}

DbAddDel::DbAddDel()
: m_column_is_sorted(false),
  m_column_sorted_direction(false),
  m_column_sorted(0),
#ifndef GLOM_ENABLE_MAEMO
  m_pMenuPopup(0),
#endif //GLOM_ENABLE_MAEMO
  m_bAllowUserActions(true),
  m_bPreventUserSignals(false),
  m_bIgnoreTreeViewSignals(false),
  m_allow_add(true),
  m_allow_delete(true),
  m_find_mode(false),
  m_allow_only_one_related_record(false),
  m_columns_ready(false),
  m_allow_view(true),
  m_allow_view_details(false),
#ifndef GLOM_ENABLE_MAEMO
  m_treeviewcolumn_button(0),
#endif
  m_fixed_cell_height(0)
{
  set_prevent_user_signals();
  set_ignore_treeview_signals(true);

  set_spacing(Utils::DEFAULT_SPACING_SMALL);

  //Start with a useful default TreeModel:
  //set_columns_count(1);
  //construct_specified_columns();
  
  // Give the TreeView an accessible name, to access it in LDTP
  // TODO: Maybe this should be a constructor parameter, so that multiple
  // DbAddDels in a single Window can be addressed separately.
#ifdef GTKMM_ATKMM_ENABLED
  m_TreeView.get_accessible()->set_name(_("Table Content"));
#endif  

  #ifndef GLOM_ENABLE_MAEMO
  m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_ScrolledWindow.set_shadow_type(Gtk::SHADOW_IN);
  m_TreeView.set_fixed_height_mode(); //This allows some optimizations.
  m_TreeView.set_rules_hint();
  m_ScrolledWindow.add(m_TreeView);
  pack_start(m_ScrolledWindow);
  #else
  //Do not let the treeview emit activated as soon as a row is pressed.
  //TODO: Allow this default maemo behaviour?
  g_object_set(m_TreeView.gobj(), "hildon-ui-mode", HILDON_UI_MODE_NORMAL, (void*)0);
  
  //Allow get_selected() and get_active() to work:
  m_TreeView.set_column_selection_mode(Hildon::TOUCH_SELECTOR_SELECTION_MODE_SINGLE);
  pack_start(m_TreeView);
  
  m_TreeView.signal_changed().connect(sigc::mem_fun(*this, &DbAddDel::on_maemo_touchselector_changed));
  #endif //GLOM_ENABLE_MAEMO

  m_TreeView.show();

  //Make sure that the TreeView doesn't start out only big enough for zero items.
  m_TreeView.set_size_request(-1, 150);

  //Allow the user to change the column order:
  //m_TreeView.set_column_drag_function( sigc::mem_fun(*this, &DbAddDel::on_treeview_column_drop) );

  #ifndef GLOM_ENABLE_MAEMO
  m_TreeView.add_events(Gdk::BUTTON_PRESS_MASK); //Allow us to catch button_press_event and button_release_event
  m_TreeView.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &DbAddDel::on_treeview_button_press_event) );
  m_TreeView.signal_columns_changed().connect( sigc::mem_fun(*this, &DbAddDel::on_treeview_columns_changed) );
  signal_button_press_event().connect(sigc::mem_fun(*this, &DbAddDel::on_button_press_event_Popup));
  #endif //GLOM_ENABLE_MAEMO
  //add_blank();

  setup_menu();


  set_prevent_user_signals(false);
  set_ignore_treeview_signals(false);

  remove_all_columns(); //set up the default columns.

  show_all_children();

#ifdef GLOM_ENABLE_CLIENT_ONLY //Actually this has only been necessary for Maemo.
  // Adjust sizing when style changed
  // TODO_Maemo: This calls construct_specified_columns(), which runs the SQL query again.
  //       Try to change the row and column sizes without doing that.
  
  signal_style_changed().connect(sigc::mem_fun(*this, &DbAddDel::on_self_style_changed));
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

DbAddDel::~DbAddDel()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  Application* pApp = get_application();
  if(pApp)
  {
    pApp->remove_developer_action(m_refContextLayout);
  } 
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

void DbAddDel::do_user_requested_edit()
{
  Gtk::TreeModel::iterator iter = get_item_selected();
  if(iter)
  {
    signal_user_requested_edit()(iter);
  }
  else
    std::cerr << "DbAddDel::do_user_requested_edit(): No item was selected." << std::endl;
}

#ifndef GLOM_ENABLE_MAEMO

void DbAddDel::on_cell_button_clicked(const Gtk::TreeModel::Path& path)
{
  if(!m_refListStore)
    return;

  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter)
  {
    select_item(iter, false /* start_editing */);
  }

  on_MenuPopup_activate_Edit();
}

void DbAddDel::on_MenuPopup_activate_Edit()
{
  do_user_requested_edit();
}

void DbAddDel::on_MenuPopup_activate_Add()
{
  //Create a new record in the database:
  start_new_record();
}

void DbAddDel::on_MenuPopup_activate_Delete()
{
  finish_editing();

  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_TreeView.get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      //TODO: We can't handle multiple-selections yet.
      user_requested_delete(iter, iter);
    }
  }
}
#endif //GLOM_ENABLE_MAEMO

#ifndef GLOM_ENABLE_CLIENT_ONLY
void DbAddDel::on_MenuPopup_activate_layout()
{
  finish_editing();
  signal_user_requested_layout().emit();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

#ifdef GLOM_ENABLE_MAEMO
void DbAddDel::on_maemo_touchselector_changed(int /* column */)
{
  if(!m_bIgnoreTreeViewSignals)
    do_user_requested_edit();
}
#endif //GLOM_ENABLE_MAEMO

#ifdef GLOM_ENABLE_MAEMO
void DbAddDel::setup_menu()
{
}
#else
void DbAddDel::setup_menu()
{
  m_refActionGroup = Gtk::ActionGroup::create();

  m_refActionGroup->add(Gtk::Action::create("ContextMenu", "Context Menu") );

  if(m_open_button_title.empty())
    m_refContextEdit =  Gtk::Action::create("ContextEdit", Gtk::Stock::EDIT);
  else
    m_refContextEdit =  Gtk::Action::create("ContextEdit", m_open_button_title);

  m_refActionGroup->add(m_refContextEdit,
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Edit) );

  m_refContextDelete =  Gtk::Action::create("ContextDelete", Gtk::Stock::DELETE);
  m_refActionGroup->add(m_refContextDelete,
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Delete) );

  m_refContextAdd =  Gtk::Action::create("ContextAdd", Gtk::Stock::ADD);
  m_refActionGroup->add(m_refContextAdd,
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Add) );
  m_refContextAdd->set_sensitive(m_allow_add);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  // Don't add ContextLayout in client only mode because it would never
  // be sensitive anyway
  m_refContextLayout =  Gtk::Action::create("ContextLayout", _("Layout"));
  m_refActionGroup->add(m_refContextLayout,
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_layout) );

  //TODO: This does not work until this widget is in a container in the window:
  Application* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY

  m_refUIManager = Gtk::UIManager::create();

  m_refUIManager->insert_action_group(m_refActionGroup);

  //TODO: add_accel_group(m_refUIManager->get_accel_group());

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
#endif
    Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='ContextMenu'>"
        "    <menuitem action='ContextEdit'/>"
        "    <menuitem action='ContextAdd'/>"
        "    <menuitem action='ContextDelete'/>"
#ifndef GLOM_ENABLE_CLIENT_ONLY
        "    <menuitem action='ContextLayout'/>"
#endif
        "  </popup>"
        "</ui>";

#ifdef GLIBMM_EXCEPTIONS_ENABLED
    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }
#else
  std::auto_ptr<Glib::Error> error;
  m_refUIManager->add_ui_from_string(ui_info, error);
  if(error.get())
  {
    std::cerr << "building menus failed: " << error->what();
  }
#endif //GLIBMM_EXCEPTIONS_ENABLED

  //Get the menu:
  m_pMenuPopup = dynamic_cast<Gtk::Menu*>( m_refUIManager->get_widget("/ContextMenu") ); 
  if(!m_pMenuPopup)
    g_warning("menu not found");


  if(get_allow_user_actions())
  {
    m_refContextEdit->set_sensitive();
    m_refContextDelete->set_sensitive();
  }
  else
  {
    m_refContextEdit->set_sensitive(false);
    m_refContextDelete->set_sensitive(false);
  }
 
#ifndef GLOM_ENABLE_CLIENT_ONLY
  if(pApp)
    m_refContextLayout->set_sensitive(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER);
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

bool DbAddDel::on_button_press_event_Popup(GdkEventButton *event)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  Application* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }
#endif

  GdkModifierType mods;
  gdk_window_get_pointer( gtk_widget_get_window (Gtk::Widget::gobj()), 0, 0, &mods );
  if(mods & GDK_BUTTON3_MASK)
  {
    //Give user choices of actions on this item:
    m_pMenuPopup->popup(event->button, event->time);
    return true; //handled.
  }
  else
  {
    if(event->type == GDK_2BUTTON_PRESS)
    {
      //Double-click means edit.
      //Don't do this usually, because users sometimes double-click by accident when they just want to edit a cell.

      //TODO: If the cell is not editable, handle the double-click as an edit/selection.
      //on_MenuPopup_activate_Edit();
      return false; //Not handled.
    }
  }

  return  false; //Not handled. TODO: Call base class?
}
#endif //GLOM_ENABLE_MAEMO

Gtk::TreeModel::iterator DbAddDel::get_item_placeholder()
{
  //Get the existing placeholder row, or add one if necessary:
  if(m_refListStore)
    return m_refListStore->get_placeholder_row();
  else
   return Gtk::TreeModel::iterator();
}

Gnome::Gda::Value DbAddDel::get_value(const Gtk::TreeModel::iterator& iter, const sharedptr<const LayoutItem_Field>& layout_item) const
{
  Gnome::Gda::Value value;

  if(m_refListStore)
  {
    Gtk::TreeModel::Row treerow = *iter;

    if(treerow)
    {
      type_list_indexes list_indexes = get_data_model_column_index(layout_item);
      if(!list_indexes.empty())
      {
        type_list_indexes::const_iterator iter = list_indexes.begin(); //Just get the first displayed instance of this field->

        const guint col_real = *iter + get_count_hidden_system_columns();
        treerow.get_value(col_real, value);
      }
    }
  }

  return value;
}

Gnome::Gda::Value DbAddDel::get_value_key_selected() const
{
  Gtk::TreeModel::iterator iter = get_item_selected();
  if(iter)
  {
    return get_value_key(iter);
  }
  else
    return Gnome::Gda::Value();
}

Gnome::Gda::Value DbAddDel::get_value_selected(const sharedptr<const LayoutItem_Field>& layout_item) const
{
  return get_value(get_item_selected(), layout_item);
}

Gtk::TreeModel::iterator DbAddDel::get_item_selected()
{
  #ifdef GLOM_ENABLE_MAEMO
  //TODO: See bug https://bugs.maemo.org/show_bug.cgi?id=4640
  //about the get_selected()/get_active() confusion.
  return m_TreeView.get_selected(0);
  //This doesn't seem to work for anything but the first item: return m_TreeView.get_active();
  #else
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
  if(refTreeSelection)
  {
     return refTreeSelection->get_selected();
  }

  if(m_refListStore)
    return m_refListStore->children().end();
  else
    return Gtk::TreeModel::iterator();
  #endif //GLOM_ENABLE_MAEMO
}

Gtk::TreeModel::iterator DbAddDel::get_item_selected() const
{
  #ifdef GLOM_ENABLE_MAEMO
  Hildon::TouchSelector& unconst = const_cast<Hildon::TouchSelector&>(m_TreeView);
  return unconst.get_selected(0);
  
  //TODO: What would this mean?
  //See https://bugs.maemo.org/show_bug.cgi?id=4641
  // return m_TreeView.get_active();
  #else
  Glib::RefPtr<const Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
  if(refTreeSelection)
  {
     Glib::RefPtr<Gtk::TreeSelection> unconst = Glib::RefPtr<Gtk::TreeSelection>::cast_const(refTreeSelection);
     return unconst->get_selected();
  }

  if(m_refListStore)
    return m_refListStore->children().end();
  else
    return Gtk::TreeModel::iterator();
  #endif //GLOM_ENABLE_MAEMO
}


Gtk::TreeModel::iterator DbAddDel::get_row(const Gnome::Gda::Value& key)
{
  if(!m_refListStore)
    return Gtk::TreeModel::iterator();

  for(Gtk::TreeModel::iterator iter = m_refListStore->children().begin(); iter != m_refListStore->children().end(); ++iter)
  {
    //Gtk::TreeModel::Row row = *iter;
    const Gnome::Gda::Value& valTemp = get_value_key(iter);
    if(valTemp == key)
    {
      return iter;
    }
  }

  return  m_refListStore->children().end();
}

bool DbAddDel::select_item(const Gtk::TreeModel::iterator& iter, bool start_editing)
{
  //Find the first column with a layout_item:
  sharedptr<const LayoutItem> layout_item;
  for(type_ColumnTypes::iterator iter_columns = m_ColumnTypes.begin(); iter_columns != m_ColumnTypes.end(); ++iter_columns)
  {
    const DbAddDelColumnInfo& column_info = *iter_columns;
    if(column_info.m_item)
    {
      layout_item = column_info.m_item;
      break;
    }
  }

  return select_item(iter, layout_item, start_editing);
}

bool DbAddDel::select_item(const Gtk::TreeModel::iterator& iter, const sharedptr<const LayoutItem>& layout_item, bool start_editing)
{
  if(!m_refListStore)
    return false;

  InnerIgnore innerIgnore(this); //see comments for InnerIgnore class

  bool bResult = false;

  if(iter)
  {
    //Get the model column:
    guint treemodel_col = 0;
    type_list_indexes list_indexes = get_column_index(layout_item);
    if(list_indexes.empty())
      return false;
    else
      treemodel_col = *(list_indexes.begin());

    treemodel_col += get_count_hidden_system_columns();

    #ifdef GLOM_ENABLE_MAEMO
    m_TreeView.select_iter(0, iter, true);
    #else
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
    g_assert(refTreeSelection);
    refTreeSelection->select(iter);
    
    Gtk::TreeModel::Path path = m_refListStore->get_path(iter);

    guint view_column_index = 0;
    const bool test = get_view_column_index(treemodel_col, view_column_index);
    if(test)
    {
      Gtk::TreeView::Column* pColumn = m_TreeView.get_column(view_column_index);
      if(pColumn)
      {
        #ifndef GLOM_ENABLE_MAEMO
        if(pColumn != m_treeviewcolumn_button) //This would activate the button. Let's avoid this, though it should never happen.
        #endif //GLOM_ENABLE_MAEMO
        {
          m_TreeView.set_cursor(path, *pColumn, start_editing);
        }
      }
      else
       g_warning("DbAddDel::select_item:TreeViewColumn not found.");
    }
    else
       g_warning("DbAddDel::select_item:TreeViewColumn index not found. column=%d", treemodel_col);
    #endif //GLOM_ENABLE_MAEMO

    bResult = true;
  }

  return bResult;
}

guint DbAddDel::get_count() const
{
  if(!m_refListStore)
    return 0;

  guint iCount = m_refListStore->children().size();

  //Take account of the extra blank for new entries:
  if(get_allow_user_actions()) //If it has the extra row.
  {
    if(get_is_placeholder_row(get_last_row()))
      --iCount;
  }

  return iCount;
}

#ifdef GLOM_ENABLE_MAEMO
Glib::RefPtr<Hildon::TouchSelectorColumn> DbAddDel::touch_selector_get_column()
{
  if(m_TreeView.get_num_columns() == 0)
  {
    //TODO: Needs a newer hildonmm: m_TreeView.append_column(m_refListStore);
    hildon_touch_selector_append_column(m_TreeView.gobj(), m_refListStore->gobj(), 0, static_cast<char*>(0), true);
  }

  return m_TreeView.get_column(0);
}

Glib::RefPtr<const Hildon::TouchSelectorColumn> DbAddDel::touch_selector_get_column() const
{
  if(m_TreeView.get_num_columns() == 0)
    return Glib::RefPtr<const Hildon::TouchSelectorColumn>();

  return m_TreeView.get_column(0);
}

#endif //GLOM_ENABLE_MAEMO

guint DbAddDel::get_columns_count() const
{
  #ifdef GLOM_ENABLE_MAEMO
  Glib::RefPtr<const Hildon::TouchSelectorColumn> column = touch_selector_get_column();
  if(!column)
    return 0;

  std::list<const Gtk::CellRenderer*> cells = column->get_cells();
  return cells.size();
  #else
  return m_TreeView.get_columns().size();
  #endif //GLOM_ENABLE_MAEMO
}

/*
void DbAddDel::set_columns_count(guint count)
{
  m_ColumnTypes.resize(count, STYLE_Text);
  m_vecColumnNames.resize(count);
}
*/

/*
void DbAddDel::set_column_title(guint col, const Glib::ustring& strText)
{
  bool bPreventUserSignals = get_prevent_user_signals();
  set_prevent_user_signals(true);

  Gtk::TreeViewColumn* pColumn = m_TreeView.get_column(col);
  if(pColumn)
    pColumn->set_title(strText);


  set_prevent_user_signals(bPreventUserSignals);
}
*/

int DbAddDel::get_fixed_cell_height()
{
  if(m_fixed_cell_height <= 0)
  {
    // Discover a suitable height, and cache it,
    // by looking at the heights of all columns:
    // Note that this is usually calculated during construct_specified_columns(), 
    // when all columns are known.
    
    //Get a default:
    Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout("example");
    int width = 0;
    int height = 0;
    refLayout->get_pixel_size(width, height);
    m_fixed_cell_height = height;

    //Look at each column:
    for(type_ColumnTypes::iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
    {
      Glib::ustring font_name;

      sharedptr<LayoutItem_WithFormatting> item_withformatting = sharedptr<LayoutItem_WithFormatting>::cast_dynamic(iter->m_item);
      if(item_withformatting)
      {
         const FieldFormatting& formatting = item_withformatting->get_formatting_used();
         font_name = formatting.get_text_format_font();
      }

      if(font_name.empty())
        continue;

      // Translators: This is just some example text used to discover an appropriate height for user-entered text in the UI. This text itself is never shown to the user.
      Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout(_("Example"));
      const Pango::FontDescription font(font_name);
      refLayout->set_font_description(font);
      int width = 0;
      int height = 0;
      refLayout->get_pixel_size(width, height);

      if(height > m_fixed_cell_height)
        m_fixed_cell_height = height;
    }
  }

  return m_fixed_cell_height;
}

Gtk::CellRenderer* DbAddDel::construct_specified_columns_cellrenderer(const sharedptr<LayoutItem>& layout_item, int model_column_index, int data_model_column_index)
{
  InnerIgnore innerIgnore(this); //see comments for InnerIgnore class

  Gtk::CellRenderer* pCellRenderer = 0;

  //Create the appropriate cellrenderer type:
  sharedptr<LayoutItem_Field> item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
  if(item_field)
  {
    if(item_field->get_hidden())
      return 0;

    switch(item_field->get_glom_type())
    {
      case(Field::TYPE_BOOLEAN):
      {
         pCellRenderer = Gtk::manage( new Gtk::CellRendererToggle() );

          break;
      }
      case(Field::TYPE_IMAGE):
      {
        pCellRenderer = Gtk::manage( new Gtk::CellRendererPixbuf() );

        break;
      }
      default:
      {
        if(item_field->get_formatting_used().get_has_choices())
        {
          CellRendererList* rendererList = Gtk::manage( new CellRendererList() );
          bool as_radio_buttons = false; //Can't really be done in a list, so we ignore it.
          rendererList->set_restrict_values_to_list(
            item_field->get_formatting_used().get_choices_restricted(as_radio_buttons));

          pCellRenderer = rendererList;
        }
        else
          pCellRenderer = Gtk::manage( new Gtk::CellRendererText() );

        break;
      }
    } //switch
  }
  else
  {
     //Non-fields:

     sharedptr<LayoutItem_Image> item_image = sharedptr<LayoutItem_Image>::cast_dynamic(layout_item);
     if(item_image)
     {
       Gtk::CellRendererPixbuf* pixbuf_renderer = Gtk::manage( new Gtk::CellRendererPixbuf() );

       Glib::RefPtr<Gdk::Pixbuf> pixbuf = Utils::get_pixbuf_for_gda_value(item_image->m_image);
       if(pixbuf)
         pixbuf_renderer->set_property("pixbuf", pixbuf);
       else
         pixbuf_renderer->set_property("stock-id", Gtk::StockID(Gtk::Stock::MISSING_IMAGE));

       pCellRenderer = pixbuf_renderer;
     }
     else
     {
       sharedptr<LayoutItem_Text> item_text = sharedptr<LayoutItem_Text>::cast_dynamic(layout_item);
       if(item_text)
       {
         Gtk::CellRendererText* pCellText = Gtk::manage( new Gtk::CellRendererText() );
         pCellText->set_property("text", item_text->get_text());

         pCellRenderer = pCellText;
       }
       else
       {
         sharedptr<LayoutItem_Button> item_button = sharedptr<LayoutItem_Button>::cast_dynamic(layout_item);
         if(item_button)
         {
           GlomCellRenderer_ButtonText* pCellButton = Gtk::manage( new GlomCellRenderer_ButtonText() );
           pCellButton->set_property("text", item_button->get_title_or_name());
           //pCellButton->set_fixed_width(50); //Otherwise it doesn't show up. TODO: Discover the width of the contents.

           #ifndef GLOM_ENABLE_MAEMO
           pCellButton->signal_clicked().connect(
             sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_cell_layout_button_clicked), model_column_index) );
           #endif //GLOM_ENABLE_MAEMO

           pCellRenderer = pCellButton;
         }
       }
     }
  }

  //Use formatting:
  sharedptr<LayoutItem_WithFormatting> item_withformatting = sharedptr<LayoutItem_WithFormatting>::cast_dynamic(layout_item);
  if(item_withformatting && pCellRenderer)
  {
    apply_formatting(pCellRenderer, item_withformatting);
  }
  
  //Set extra cellrenderer attributes, depending on the type used:
  Gtk::CellRendererText* pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(pCellRenderer);
  if(pCellRendererText)
  {
    //Use an ellipze to indicate excessive text, 
    //so that similar values do not look equal, 
    //and to avoid multi-line comments. TODO: Is there a better way to restrict the height? This doesn't actually truncate multilines anyway.
    g_object_set(pCellRendererText->gobj(), "ellipsize", PANGO_ELLIPSIZE_END, (gpointer)0);

    //Restrict the height, to prevent multiline text cells,
    //and to allow TreeView performance optimisation:
    //TODO: Avoid specifying a width for the last column?
    int suitable_width = 0;
    pCellRendererText->get_property("width", suitable_width);
    pCellRendererText->set_fixed_size(suitable_width, get_fixed_cell_height() );

    #ifndef GLOM_ENABLE_MAEMO //List views are non-editable on Maemo.
    //Connect to edited signal:
    if(item_field) //Only fields can be edited:
    {
      //Make it editable:
      pCellRendererText->set_property("editable", true);

      //Connect to its signal:
      pCellRendererText->signal_edited().connect(
        sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited), model_column_index, data_model_column_index) );
    }
    #endif //GLOM_ENABLE_MAEMO

    //Choices:
    CellRendererList* pCellRendererCombo = dynamic_cast<CellRendererList*>(pCellRenderer);
    if(pCellRendererCombo)
    {
      pCellRendererCombo->remove_all_list_items();

      if(item_field && item_field->get_formatting_used().get_has_custom_choices())
      {
        pCellRendererCombo->set_use_second(false); //Custom choices have only one column.

        //set_choices() needs this, for the numeric layout:
        //pCellRendererCombo->set_layout_item(get_layout_item()->clone(), table_name); //TODO_Performance: We only need this for the numerical format.
        const FieldFormatting::type_list_values list_values = item_field->get_formatting_used().get_choices_custom();
        for(FieldFormatting::type_list_values::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
        {
          pCellRendererCombo->append_list_item( Conversions::get_text_for_gda_value(item_field->get_glom_type(), *iter, item_field->get_formatting_used().m_numeric_format) );
        }
      }
      else if(item_field && item_field->get_formatting_used().get_has_related_choices())
      {
        sharedptr<Relationship> choice_relationship;
        Glib::ustring choice_field, choice_second;
        item_field->get_formatting_used().get_choices(choice_relationship, choice_field, choice_second);

        if(choice_relationship && !choice_field.empty())
        {
          const Glib::ustring to_table = choice_relationship->get_to_table();

          const bool use_second = !choice_second.empty();
          pCellRendererCombo->set_use_second(use_second);

          const sharedptr<LayoutItem_Field> layout_field_second = sharedptr<LayoutItem_Field>::create();
          if(use_second)
          {
            Document* document = get_document();
            if(document)
            {
              sharedptr<Field> field_second = document->get_field(to_table, choice_second); //TODO: Actually show this in the combo:
              layout_field_second->set_full_field_details(field_second);

              //We use the default formatting for this field->
            }
          }

          Utils::type_list_values_with_second list_values = Utils::get_choice_values(item_field);
          for(Utils::type_list_values_with_second::const_iterator iter = list_values.begin(); iter != list_values.end(); ++iter)
          {
            const Glib::ustring first = Conversions::get_text_for_gda_value(item_field->get_glom_type(), iter->first, item_field->get_formatting_used().m_numeric_format);

            Glib::ustring second;
            if(use_second)
              second = Conversions::get_text_for_gda_value(layout_field_second->get_glom_type(), iter->second, layout_field_second->get_formatting_used().m_numeric_format);

              pCellRendererCombo->append_list_item(first, second);
          }
        }
      }
    }
  }
  else
  {
    Gtk::CellRendererToggle* pCellRendererToggle = dynamic_cast<Gtk::CellRendererToggle*>(pCellRenderer);
    if(pCellRendererToggle)
    {
      pCellRendererToggle->set_property("activatable", true);

      if(item_field) //Only fields can be edited:
      {
        #ifndef GLOM_ENABLE_MAEMO //There's no direct editing via the list view on Maemo.
        //Connect to its signal:
        pCellRendererToggle->signal_toggled().connect(
        sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited_bool), model_column_index, data_model_column_index ) );
        #endif //GLOM_ENABLE_MAEMO
      }
    }
    else
    {
      Gtk::CellRendererPixbuf* pCellRendererPixbuf = dynamic_cast<Gtk::CellRendererPixbuf*>(pCellRenderer);
      if(pCellRendererPixbuf)
      {
        //TODO: Do something when it's clicked, such as show the big image in a window or tooltip?
      }
    }
  }

  return pCellRenderer;
}

void DbAddDel::apply_formatting(Gtk::CellRenderer* renderer, const sharedptr<const LayoutItem_WithFormatting>& layout_item)
{
  Gtk::CellRendererText* text_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!text_renderer)
    return;

  //Use the text formatting:

  //Horizontal alignment:
  const FieldFormatting::HorizontalAlignment alignment =
    layout_item->get_formatting_used_horizontal_alignment();
  const float x_align = (alignment == FieldFormatting::HORIZONTAL_ALIGNMENT_LEFT ? 0.0 : 1.0);
#ifdef GLIBMM_PROPERTIES_ENABLED  
      text_renderer->property_xalign() = x_align;
#else    
      text_renderer->set_property("xalign", alignment);
#endif

  const FieldFormatting& formatting = layout_item->get_formatting_used();

  const Glib::ustring font_desc = formatting.get_text_format_font();
  if(!font_desc.empty())
#ifdef GLIBMM_PROPERTIES_ENABLED  
    text_renderer->property_font() = font_desc;
#else
    text_renderer->set_property("font", font_desc);
#endif        

  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
#ifdef GLIBMM_PROPERTIES_ENABLED  
    text_renderer->property_foreground() = fg;
#else    
    text_renderer->set_property("foreground", fg);
#endif
    
  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
#ifdef GLIBMM_PROPERTIES_ENABLED
    text_renderer->property_background() = bg;
#else    
    text_renderer->set_property("background", bg);
#endif
}

void DbAddDel::construct_specified_columns()
{
  InnerIgnore innerIgnore(this);

  //TODO_optimisation: This is called many times, just to simplify the API.

  //Delay actual use of set_column_*() stuff until this method is called.

  if(m_ColumnTypes.empty())
  {
    //std::cout << "debug: DbAddDel::construct_specified_columns(): showing hint model: m_find_mode=" << m_find_mode << std::endl;

    m_refListStore.reset();
    if(m_table_name.empty())
    {
      #ifdef GLOM_ENABLE_MAEMO
      //TODO: Needs a newer hildonmm: m_TreeView.append_column(m_refListStore);
      //TODO: Remove all previous columns?
      hildon_touch_selector_append_column(m_TreeView.gobj(), m_refListStore->gobj(), 0, static_cast<char*>(0), true);
      #else
      m_TreeView.set_model(m_refListStore); // clear old model from treeview
      #endif
    }
    else
      show_hint_model();
    return;
  }

  typedef Gtk::TreeModelColumn<Gnome::Gda::Value> type_modelcolumn_value;
  typedef std::vector< type_modelcolumn_value* > type_vecModelColumns;
  type_vecModelColumns vecModelColumns(m_ColumnTypes.size(), 0);

  //Create the Gtk ColumnRecord:

  Gtk::TreeModel::ColumnRecord record;
    
  //Database columns:
  type_model_store::type_vec_fields fields;
  {
    type_vecModelColumns::size_type i = 0;
    for(type_ColumnTypes::iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
    {
      sharedptr<LayoutItem_Field> item_field = sharedptr<LayoutItem_Field>::cast_dynamic(iter->m_item);
      if(item_field)
      {
        type_modelcolumn_value* pModelColumn = new type_modelcolumn_value;

        //Store it so we can use it and delete it later:
        vecModelColumns[i] = pModelColumn;

        record.add( *pModelColumn );

        //if(iter->m_item->get_has_relationship_name())
        //  std::cout << "  DEBUG: relationship=" << iter->m_item->get_relationship()->get_name() << std::endl;

        fields.push_back(item_field);

        i++;
      }
    }
  }
  
  m_FieldsShown = fields; //Needed by Base_DB_Table_Data::record_new().

  {
    //Find the primary key:
    int column_index_key = 0;
    bool key_found = false;
    for(type_model_store::type_vec_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
    {
      sharedptr<LayoutItem_Field> layout_item = *iter;
      if( !(layout_item->get_has_relationship_name()) )
      {
        sharedptr<const Field> field_full = layout_item->get_full_field_details();
        if(field_full && field_full->get_primary_key() )
        {
          key_found = true;
          break;
        }
      }

      ++column_index_key;
    }

    if(key_found)
    {
      //Create the model from the ColumnRecord:
      //Note that the model will use a dummy Gda DataModel if m_find_mode is true.
      //std::cout << "debug: Creating new type_model_store() for table=" << m_found_set.m_table_name << std::endl;
      m_refListStore = type_model_store::create(record, m_found_set, fields, column_index_key, m_allow_view, m_find_mode);
    }
    else
    {
      g_warning("%s: no primary key field found.", __FUNCTION__);
      //for(type_model_store::type_vec_fields::const_iterator iter = fields.begin(); iter != fields.end(); ++iter)
      //{
      //  g_warning("  field: %s", (iter->get_name().c_str());
      //}

      m_refListStore = Glib::RefPtr<type_model_store>();
    }
  }
 
  #ifdef GLOM_ENABLE_MAEMO
  //Remove all View columns:
  while(m_TreeView.get_num_columns())
  {
    m_TreeView.remove_column(0);
  }
  #else
  m_TreeView.set_model(m_refListStore); // clear old model from treeview

  //Remove all View columns:
  m_TreeView.remove_all_columns();
  #endif


  //Add new View Colums:
  int model_column_index = 0; //Not including the hidden internal columns.
  int view_column_index = 0;

  #ifndef GLOM_ENABLE_MAEMO
  {
    GlomCellRenderer_ButtonImage* pCellButton = Gtk::manage(new GlomCellRenderer_ButtonImage());

    pCellButton->signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel::on_cell_button_clicked));

    m_treeviewcolumn_button = Gtk::manage(new Gtk::TreeViewColumn());
    m_treeviewcolumn_button->pack_start(*pCellButton);

    int x_offset = 0;
    int y_offset = 0;
    int width = 0;
    int height = 0;
    pCellButton->get_size(m_TreeView, x_offset, y_offset, width, height);

    m_treeviewcolumn_button->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED); //Needed by fixed-height mode.

    // TODO: I am not sure whether this is always correct. Perhaps, we also
    // have to take into account the xpad property of the cell renderer and
    // the spacing property of the treeviewcolumn.
    int horizontal_separator = 0;
    m_TreeView.get_style_property("horizontal-separator", horizontal_separator);
    const int button_width = width + horizontal_separator*2;
    if(button_width > 0) //Otherwise an assertion fails.
      m_treeviewcolumn_button->set_fixed_width(button_width);

    m_treeviewcolumn_button->set_visible(m_allow_view_details);

    m_TreeView.append_column(*m_treeviewcolumn_button);

    ++view_column_index;
  }
  #endif //GLOM_ENABLE_MAEMO

  bool no_columns_used = true;
  int data_model_column_index = 0; //-1 means undefined index.
  
  guint column_to_expand = 0;
  const bool has_expandable_column = get_column_to_expand(column_to_expand);
  //std::cout << "DEBUG: column_to_expand=" << column_to_expand  << ", has=" << has_expandable_column << std::endl;
  
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
    const DbAddDelColumnInfo& column_info = m_ColumnTypes[model_column_index];
    if(column_info.m_visible)
    {
      no_columns_used = false;

      const Glib::ustring column_name = column_info.m_item->get_title_or_name();
      const Glib::ustring column_id = column_info.m_item->get_name();

      // Whenever we are dealing with real database fields, 
      // we need to know the index of the field in the query:
      int item_data_model_column_index = -1;
      sharedptr<const LayoutItem> layout_item = m_ColumnTypes[model_column_index].m_item;
      sharedptr<const LayoutItem_Field> item_field = sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);
      if(item_field)
      {
        item_data_model_column_index = data_model_column_index;
        ++data_model_column_index;
      }

      //Add the ViewColumn
      Gtk::CellRenderer* pCellRenderer = construct_specified_columns_cellrenderer(column_info.m_item, model_column_index, item_data_model_column_index);
      if(pCellRenderer)
      {
        //Get the index of the field in the query, if it is a field:
        //std::cout << "debug: model_column_index=" << model_column_index << ", item_data_model_column_index=" << item_data_model_column_index << std::endl;
        const bool expand = has_expandable_column && ((int)column_to_expand == model_column_index);
        treeview_append_column(column_name, 
          *pCellRenderer, 
          model_column_index, item_data_model_column_index, 
          expand);

        if(column_info.m_editable)
        {
       
        }

        ++view_column_index;
      }

    } //is visible

    ++model_column_index;
  } //for

  //Delete the vector's items:
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
     type_modelcolumn_value* pModelColumn = *iter;
     if(pModelColumn)
       delete pModelColumn;
  }

  
  if(no_columns_used)
  {
    show_hint_model();
  }
  else
  {
    #ifndef GLOM_ENABLE_MAEMO
    //We must set this each time, because show_hint_model() might unset it:
    m_TreeView.set_fixed_height_mode(); //This allows some optimizations.
    #endif //GLOM_ENABLE_MAEMO
  }

  

  #ifndef GLOM_ENABLE_MAEMO
  m_TreeView.columns_autosize();
  #endif

  //Make sure there's a blank row after the database rows that have just been added.
  //add_blank();
}

bool DbAddDel::refresh_from_database()
{
  construct_specified_columns();
  return true;

  /*
  if(m_refListStore)
  {
    //Glib::RefPtr<Gtk::TreeModel> refNull;
    const bool result = m_refListStore->refresh_from_database(m_found_set);
    //m_TreeView.set_model(refNull); //TODO: This causes a g_warning(): gtk_tree_view_unref_tree_helper: assertion `node != NULL' failed
    if(m_TreeView.get_model())
      gtk_tree_view_set_model(m_TreeView.gobj(), 0); //This gives the same warning.

    m_TreeView.set_model(m_refListStore);
    return result;
  }
  else
    return false;
  */
}

bool DbAddDel::refresh_from_database_blank()
{
  if(m_find_mode)
    return refresh_from_database();

  if(m_refListStore)
  {
    m_refListStore->clear(); //Remove all rows.
  }

  return true;
}

void DbAddDel::set_value(const Gtk::TreeModel::iterator& iter, const sharedptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value)
{
  //g_warning("DbAddDel::set_value begin");

  InnerIgnore innerIgnore(this);

  if(!m_refListStore)
    g_warning("DbAddDel::set_value: No model.");
  else
  {
    Gtk::TreeModel::Row treerow = *iter;
    if(treerow)
    {
      type_list_indexes list_indexes = get_data_model_column_index(layout_item);
      for(type_list_indexes::const_iterator iter = list_indexes.begin(); iter != list_indexes.end(); ++iter)
      {
        const guint treemodel_col = *iter + get_count_hidden_system_columns();
        treerow.set_value(treemodel_col, value);

        //Mark this row as not a placeholder because it has real data now.
        if(!(Conversions::value_is_empty(value)))
        {
          //treerow.set_value(m_col_key, Glib::ustring("placeholder debug value setted"));
          //treerow.set_value(m_col_placeholder, false);
        }
      }
    }

    //Add extra blank if necessary:
    //add_blank();
  }

  //g_warning("DbAddDel::set_value end");
}

void DbAddDel::set_value_selected(const sharedptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value)
{
  set_value(get_item_selected(), layout_item, value);
}

void DbAddDel::remove_all_columns()
{
  m_ColumnTypes.clear();

  m_fixed_cell_height = 0; //Force it to be recalculated.
  m_columns_ready = false;
}

void DbAddDel::set_table_name(const Glib::ustring& table_name)
{
  m_found_set.m_table_name = table_name;
  Base_DB_Table::m_table_name = table_name;
}

guint DbAddDel::add_column(const sharedptr<LayoutItem>& layout_item)
{
  if(!layout_item)
    return 0; //TODO: Do something more sensible.

  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add a new column.
  
  DbAddDelColumnInfo column_info;
  column_info.m_item = layout_item;
  //column_info.m_editable = editable;
  //column_info.m_visible = visible;

  //Make it non-editable if it is auto-generated:

  sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
  if(field)
  {
    sharedptr<const Field> field_full = field->get_full_field_details();
    if(field_full && field_full->get_auto_increment())
      column_info.m_editable = false;
    else
      column_info.m_editable = field->get_editable_and_allowed();
  }

  m_ColumnTypes.push_back(column_info);

  //Generate appropriate model columns:
  if(m_columns_ready)
    construct_specified_columns();

  //Tell the View to use the model:
  //m_TreeView.set_model(m_refListStore);

  return m_ColumnTypes.size() - 1;
}

void DbAddDel::set_found_set(const FoundSet& found_set)
{
  m_found_set = found_set;
}

FoundSet DbAddDel::get_found_set() const
{
  return m_found_set;
}

void DbAddDel::set_columns_ready()
{
  m_columns_ready = true;
  construct_specified_columns();
}

DbAddDel::type_list_indexes DbAddDel::get_data_model_column_index(const sharedptr<const LayoutItem_Field>& layout_item_field) const
{
  //TODO_Performance: Replace all this looping by a cache/map:

  type_list_indexes list_indexes;

  if(!layout_item_field)
    return list_indexes;

  guint data_model_column_index = 0;
  for(type_ColumnTypes::const_iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(iter->m_item); //TODO_Performance: This would be unnecessary if !layout_item_field
    if(field)
    {
      if(field && field->is_same_field(layout_item_field))
      {
        list_indexes.push_back(data_model_column_index);
      }

      ++data_model_column_index;
    }
  }

  return list_indexes;
}

DbAddDel::type_list_indexes DbAddDel::get_column_index(const sharedptr<const LayoutItem>& layout_item) const
{
  //TODO_Performance: Replace all this looping by a cache/map:

  type_list_indexes list_indexes;

  sharedptr<const LayoutItem_Field> layout_item_field = sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);

  guint i = 0;
  for(type_ColumnTypes::const_iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
  {
    sharedptr<const LayoutItem_Field> field = sharedptr<const LayoutItem_Field>::cast_dynamic(iter->m_item); //TODO_Performance: This would be unnecessary if !layout_item_field
    if(field && layout_item_field && field->is_same_field(layout_item_field))
    {
      list_indexes.push_back(i);
    }
    else if(*(iter->m_item) == *(layout_item))
    {
      list_indexes.push_back(i);
    }

    ++i;
  }

  return list_indexes;
}

sharedptr<const LayoutItem_Field> DbAddDel::get_column_field(guint column_index) const
{
  if(column_index < m_ColumnTypes.size())
  {
    sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic( m_ColumnTypes[column_index].m_item );
    if(field)
      return field;
  }

  return sharedptr<const LayoutItem_Field>();
}

bool DbAddDel::get_prevent_user_signals() const
{
  return m_bPreventUserSignals;
}

void DbAddDel::set_prevent_user_signals(bool bVal)
{
  m_bPreventUserSignals = bVal;
}

void DbAddDel::set_column_choices(guint col, const type_vec_strings& vecStrings)
{
  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add new columns.

  m_ColumnTypes[col].m_choices = vecStrings;

  guint view_column_index = 0;
  const bool test = get_view_column_index(col, view_column_index);
  if(test)
  { 
    #ifdef GLOM_ENABLE_MAEMO
    Glib::RefPtr<Hildon::TouchSelectorColumn> column = touch_selector_get_column();
    g_assert(column);
    std::vector<Gtk::CellRenderer*> list_renderers = column->get_cells();
    g_assert(!list_renderers.empty());
    CellRendererList* pCellRenderer = dynamic_cast<CellRendererList*>(list_renderers[0]);
    #else
    CellRendererList* pCellRenderer = 
      dynamic_cast<CellRendererList*>( m_TreeView.get_column_cell_renderer(view_column_index) );
    #endif //GLOM_ENABLE_MAEMO
    if(pCellRenderer)
    {
      //Add the choices:
      pCellRenderer->remove_all_list_items();
      for(type_vec_strings::const_iterator iter = vecStrings.begin(); iter != vecStrings.end(); ++iter)
      {
        pCellRenderer->append_list_item(*iter);
      }
    }
    else
    {
      //The column does not exist yet, so we must create it:
      if(m_columns_ready)
        construct_specified_columns();
    }
  }
}

void DbAddDel::set_allow_add(bool val)
{
  m_allow_add = val;

  #ifndef GLOM_ENABLE_MAEMO
  m_refContextAdd->set_sensitive(val);
  #endif //GLOM_ENABLE_MAEMO
}

void DbAddDel::set_allow_delete(bool val)
{
  m_allow_delete = val;
}

void DbAddDel::set_allow_user_actions(bool bVal)
{
  m_bAllowUserActions = bVal;
}

bool DbAddDel::get_allow_user_actions() const
{
  return m_bAllowUserActions;
}

void DbAddDel::set_find_mode(bool val)
{
  const bool current = m_find_mode;
  m_find_mode = val;

  //Recreate the model if necessary:
  if( (current != m_find_mode) &&
      m_refListStore)
  {
    construct_specified_columns();
  }
}
  
void DbAddDel::set_allow_only_one_related_record(bool val)
{
  m_allow_only_one_related_record = val;
}

void DbAddDel::finish_editing()
{
//  bool bIgnoreSheetSignals = get_ignore_treeview_signals(); //The deactivate signals seems to cause the current cell to revert to it's previsous value.
//  set_ignore_treeview_signals();
//
//  int row = 0;
//  int col = 0;
//  m_Sheet.get_active_cell(row, col);
//  m_Sheet.set_active_cell(row, col);
//
//  set_ignore_treeview_signals(bIgnoreSheetSignals);
}

/*
void DbAddDel::reactivate()
{
//  //The sheet does not seem to get updated until one of its cells is activated:
//
//  int row = 0;
//  int col = 0;
//  m_Sheet.get_active_cell(row, col);
//
//  //Activate 0,0 if none is currently active.
//  if( (row == -1) && (col == -1) )
//  {
//    row = 0;
//    col = 0;
//  }
//
//  m_Sheet.set_active_cell(row, col);
}
*/

void DbAddDel::remove_item(const Gtk::TreeModel::iterator& iter)
{
  if(iter && m_refListStore)
    m_refListStore->erase(iter);
}

bool DbAddDel::get_ignore_treeview_signals() const
{
  return m_bIgnoreTreeViewSignals;
}

void DbAddDel::set_ignore_treeview_signals(bool ignore)
{
  m_bIgnoreTreeViewSignals = ignore;
}

DbAddDel::InnerIgnore::InnerIgnore(DbAddDel* pOuter)
: m_pOuter(pOuter),
  m_bPreventUserSignals(false),
  m_bIgnoreTreeViewSignals(false)
{
  if(m_pOuter)
  {
    m_bPreventUserSignals = m_pOuter->get_prevent_user_signals();
    m_pOuter->set_prevent_user_signals();

    m_bIgnoreTreeViewSignals = m_pOuter->get_ignore_treeview_signals();
    m_pOuter->set_ignore_treeview_signals();
  }
}


DbAddDel::InnerIgnore::~InnerIgnore()
{
  //Restore values:
  if(m_pOuter)
  {
    m_pOuter->set_prevent_user_signals(m_bPreventUserSignals);
    m_pOuter->set_ignore_treeview_signals(m_bIgnoreTreeViewSignals);
  }

  m_pOuter = false;
}

Gnome::Gda::Value DbAddDel::treeview_get_key(const Gtk::TreeModel::iterator& row) const
{
  Gnome::Gda::Value value;

  if(m_refListStore)
  {
    return m_refListStore->get_key_value(row);
  }

  return value;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
DbAddDel::type_signal_user_requested_layout DbAddDel::signal_user_requested_layout()
{
  return m_signal_user_requested_layout;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

DbAddDel::type_signal_user_requested_edit DbAddDel::signal_user_requested_edit()
{
  return m_signal_user_requested_edit;
}


DbAddDel::type_signal_script_button_clicked DbAddDel::signal_script_button_clicked()
{
  return m_signal_script_button_clicked;
}

DbAddDel::type_signal_record_added DbAddDel::signal_record_added()
{
  return m_signal_record_added;
}

DbAddDel::type_signal_sort_clause_changed DbAddDel::signal_sort_clause_changed()
{
  return m_signal_sort_clause_changed;
}

#ifndef GLOM_ENABLE_MAEMO
void DbAddDel::on_cell_layout_button_clicked(const Gtk::TreeModel::Path& path, int model_column_index)
{
  if(!m_refListStore)
    return;

  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter)
  {
    sharedptr<const LayoutItem> layout_item = m_ColumnTypes[model_column_index].m_item;
    sharedptr<const LayoutItem_Button> item_button = sharedptr<const LayoutItem_Button>::cast_dynamic(layout_item);
    if(item_button)
    {
      m_signal_script_button_clicked.emit(item_button, iter);
    }
  }
}

void DbAddDel::on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index, int data_model_column_index)
{
  //Note:: model_column_index is actually the AddDel column index, not the TreeModel column index.

  if(path_string.empty())
    return;

  if(!m_refListStore)
    return;

  const Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    const int tree_model_column_index = data_model_column_index + get_count_hidden_system_columns();

    Gnome::Gda::Value value_old;
    row.get_value(tree_model_column_index, value_old);

    const bool bValueOld = (value_old.get_value_type() == G_TYPE_BOOLEAN) && value_old.get_boolean();
    const bool bValueNew = !bValueOld;
    Gnome::Gda::Value value_new;
    value_new.set(bValueNew);
    //Store the user's new value in the model:
    row.set_value(tree_model_column_index, value_new);

    //TODO: Did it really change?

    //Is this an add or a change?:

    bool bIsAdd = false;
    bool bIsChange = false;

    const int iCount = m_refListStore->children().size();
    if(iCount)
    {
      if(get_allow_user_actions()) //If add is possible:
      {
        if( get_is_placeholder_row(iter) ) //If it's the last row:
        {
          //We will ignore editing of bool values in the blank row. It seems like a bad way to start a new record.
          //New item in the blank row:
          /*
          Glib::ustring strValue = get_value(row);
          if(!strValue.empty())
          {
            bool bPreventUserSignals = get_prevent_user_signals();
            set_prevent_user_signals(true); //Stops extra signal_user_changed.
            add_item(); //Add the next blank for the next user add.
            set_prevent_user_signals(bPreventUserSignals);
         */

          bIsAdd = true; //Signal that a new key was added.
          //}
        }
      }

      if(!bIsAdd)
        bIsChange = true;
    }

    //Fire appropriate signal:
    if(bIsAdd)
    {
      //Change it back, so that we ignore it:
      row.set_value(tree_model_column_index, value_old);
         
      //Signal that a new key was added:
      //We will ignore editing of bool values in the blank row. It seems like a bad way to start a new record.
      //user_added(row);
    }
    else if(bIsChange)
    {
      //Existing item changed:

      user_changed(row, model_column_index);
    }
  }
}

void DbAddDel::on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index, int data_model_column_index)
{
  //Note:: model_column_index is actually the AddDel column index, not the TreeModel column index.
  if(path_string.empty())
    return;

  if(!m_refListStore)
    return;

  const Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter != get_model()->children().end())
  {
    Gtk::TreeModel::Row row = *iter;

    const int treemodel_column_index = data_model_column_index + get_count_hidden_system_columns();

    Gnome::Gda::Value valOld;
    row.get_value(treemodel_column_index, valOld);
    //std::cout << "debug: valOld type=" << valOld.get_value_type() << std::endl;

    //Store the user's new text in the model:
    //row.set_value(treemodel_column_index, new_text);

    //Is it an add or a change?:
    bool bIsAdd = false;
    bool bIsChange = false;
    bool do_change = true;

    if(get_allow_user_actions()) //If add is possible:
    {
      if(get_is_placeholder_row(iter))
      {
        const bool bPreventUserSignals = get_prevent_user_signals();
        set_prevent_user_signals(true); //Stops extra signal_user_changed.

        //Don't add a new row if nothing was entered into the placeholder.
        if (new_text.empty())
          return;

        //Mark this row as no longer a placeholder, because it has data now. The client code must set an actual key for this in the signal_user_added() or m_signal_user_changed signal handlers.
        m_refListStore->set_is_not_placeholder(iter);
        //Don't mark this as not a placeholder, because it's still a placeholder until it has a key value.

        set_prevent_user_signals(bPreventUserSignals);

        bIsAdd = true; //Signal that a new key was added.
      }
    }


    sharedptr<LayoutItem_Field> item_field = sharedptr<LayoutItem_Field>::cast_dynamic(m_ColumnTypes[model_column_index].m_item);
    if(!item_field)
      return;

    const Field::glom_field_type field_type = item_field->get_glom_type();
    if(field_type != Field::TYPE_INVALID) //If a field type was specified for this column.
    {
      //Make sure that the entered data is suitable for this field type:
      bool success = false;

      const Gnome::Gda::Value value = Conversions::parse_value(field_type, new_text, item_field->get_formatting_used().m_numeric_format, success);
      if(!success)
      {
          //Tell the user and offer to revert or try again:
          bool revert = glom_show_dialog_invalid_data(field_type);
          if(revert)
          {
            //Revert the data:
            row.set_value(treemodel_column_index, valOld);
          }
          else
          {
            //Reactivate the cell so that the data can be corrected.

            Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
            if(refTreeSelection)
            {
              refTreeSelection->select(row); //TODO: This does not seem to work.

              if(!path.empty())
              {
                Gtk::TreeView::Column* pColumn = m_TreeView.get_column(model_column_index);
                if(pColumn)
                {
                  Gtk::CellRendererText* pCell = dynamic_cast<Gtk::CellRendererText*>(pColumn->get_first_cell_renderer());
                  if(pCell)
                  {
                    //TreeView::set_cursor(), or start_editing() would get the old value back from the model again
                    //so we do something similar without getting the old value:
                    m_TreeView.set_cursor(path, *pColumn, *pCell, true /* start_editing */); //This highlights the cell, and starts the editing.

                    //This is based on gtk_tree_view_start_editing():
                    //TODO: This does not actually work. I emailed gtk-list about how to do this.
                    /*
                    pCell->stop_editing();
                    pCell->property_text() = "test"; //new_text; //Allow the user to start with the bad text that he entered so far.

                    Gdk::Rectangle background_area;
                    m_TreeView.get_background_area(path, *pColumn, background_area);

                    Gdk::Rectangle cell_area;
                    m_TreeView.get_cell_area(path, *pColumn, background_area); 
                    */
                  }
                }
              }
              else
              {
                g_warning("DbAddDel::on_treeview_cell_edited(): path is invalid.");
              }
            }
          }

          do_change = false;
      }
      else
      {
        //Store the value in the model:
        //std::cout << "debug: setting value: column=" << treemodel_column_index << ", type=" << value.get_value_type() << std::endl;
        row.set_value(treemodel_column_index, value);
        //std::cout << "debug: after setting value" << std::endl;
      }

      if(!bIsAdd)
        bIsChange = true;

      //Fire appropriate signal:
      if(bIsAdd)
      {
        //Signal that a new key was added:
        if(m_allow_add)
          user_added(row);
      }
      else if(bIsChange)
      {
        //Existing item changed:
        //Check that it has really changed - get the last value.
        if(value != valOld)
        {
          if(do_change)
            user_changed(row, model_column_index);
        }
      }
    }
  }
}

void DbAddDel::on_treeview_button_press_event(GdkEventButton* event)
{
  on_button_press_event_Popup(event);
}

bool DbAddDel::on_treeview_columnheader_button_press_event(GdkEventButton* event)
{
  //If this is a right-click with the mouse:
  if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
  {
    
    
  }

  return false;
}

bool DbAddDel::on_treeview_column_drop(Gtk::TreeView* /* treeview */, Gtk::TreeViewColumn* /* column */, Gtk::TreeViewColumn* /* prev_column */, Gtk::TreeViewColumn* /* next_column */)
{
  return true;
}

void DbAddDel::on_treeview_column_resized(int model_column_index, DbTreeViewColumnGlom* view_column)
{
  if(!view_column)
    return;
  
  //Ignore this property change signal handler if we are setting the size in code:
  if(m_bIgnoreTreeViewSignals)
    return;

  //We do not save the column width if this is the last column, 
  //because that must always be automatic, 
  //because it must resize when the whole column resizes.
  std::list<const Gtk::TreeView::Column*> columns = m_TreeView.get_columns();
  const int n_view_columns = columns.size();
  if(n_view_columns && (view_column == m_TreeView.get_column(n_view_columns -1)))
    return;

  DbAddDelColumnInfo& column_info = m_ColumnTypes[model_column_index];

  const int width = view_column->get_width();
  //std::cout << "  DbAddDel::on_treeview_column_resized(): width=" << width << std::endl;

  if(width == -1) //Means automatic.
    return;
    
  if(column_info.m_item)
      column_info.m_item->set_display_width(width);
}

void DbAddDel::on_treeview_column_clicked(int model_column_index)
{
  BusyCursor busy_cursor(get_application());

  if(model_column_index >= (int)m_ColumnTypes.size())
    return;

  sharedptr<const LayoutItem_Field> layout_item = sharedptr<const LayoutItem_Field>::cast_dynamic(m_ColumnTypes[model_column_index].m_item); //We can only sort on fields, not on other layout item.
  if(layout_item && layout_item->get_name_not_empty())
  {
    bool ascending = true;
    if(m_column_is_sorted && ((int)m_column_sorted == model_column_index))
    {
      //Reverse the existing direction:
      ascending = !m_column_sorted_direction;
    }

    //Remember the user's chosen sort, so we can reverse it if he clicks again:
    m_column_is_sorted = true;
    m_column_sorted_direction = ascending;
    m_column_sorted = model_column_index;

    //Set the sort clause to be used by refresh_from_database():
    m_found_set.m_sort_clause.clear();
    m_found_set.m_sort_clause.push_back( type_pair_sort_field(layout_item, ascending) );
  }

  refresh_from_database();

  m_signal_sort_clause_changed.emit();
}

void DbAddDel::on_treeview_columns_changed()
{
  if(!m_bIgnoreTreeViewSignals)
  {
    //Get the new column order, and save it in m_vecColumnIDs:
    m_vecColumnIDs.clear();

    typedef std::vector<Gtk::TreeViewColumn*> type_vecViewColumns;
    type_vecViewColumns vecViewColumns = m_TreeView.get_columns();

    for(type_vecViewColumns::iterator iter = vecViewColumns.begin(); iter != vecViewColumns.end(); ++iter)
    {
      DbTreeViewColumnGlom* pViewColumn = dynamic_cast<DbTreeViewColumnGlom*>(*iter);
      if(pViewColumn)
      {
        const Glib::ustring column_id = pViewColumn->get_column_id();
        m_vecColumnIDs.push_back(column_id);

      }
    }

    //Tell other code that something has changed, so the new column order can be serialized.
    //TODO: If this is ever wanted: m_signal_user_reordered_columns.emit();
  }
}
#endif //GLOM_ENABLE_MAEMO

bool DbAddDel::get_column_to_expand(guint& column_to_expand) const
{
  //Initialize output parameter:
  column_to_expand = 0;
  bool result = false;
  
  //Discover the right-most text column: 
  guint i = 0;
  for(type_ColumnTypes::const_iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
  {
    sharedptr<LayoutItem> layout_item = iter->m_item;
           
    sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
    if(layout_item_field)
    {  
      //Only text columns should expand.
      //Number fields are right-aligned, so expansion is annoying.
      //Time and date fields don't vary their width much.
      if(layout_item_field->get_glom_type() == Field::TYPE_TEXT)
      {
        //Check that no specific width has been specified:
        const guint column_width = layout_item_field->get_display_width();
        if(!column_width) //TODO: Ignore these on maemo?
        {
          column_to_expand = i;
          result = true;
        }
        
      }
    }
    
    ++i;
  }
  
  return result;
}

guint DbAddDel::treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, int model_column_index, int data_model_column_index, bool expand)
{
  InnerIgnore innerIgnore(this); //see comments for InnerIgnore class

  #ifndef GLOM_ENABLE_MAEMO 
  DbTreeViewColumnGlom* pViewColumn = Gtk::manage( new DbTreeViewColumnGlom(Utils::string_escape_underscores(title), cellrenderer) );

  //This is needed by fixed-height mode. We get critical warnings otherwise.
  //But we must call set_fixed_width() later or we will have a zero-width column.
  pViewColumn->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);

  guint cols_count = m_TreeView.append_column(*pViewColumn);
  #else
  //Mathias Hasselmann says that this is required for the Maemo 5 style, 
  //though we don't know yet where that is documented. murrayc.
  cellrenderer.set_property("xpad", HILDON_MARGIN_DEFAULT);

  Glib::RefPtr<Hildon::TouchSelectorColumn> pViewColumn = touch_selector_get_column();
  pViewColumn->pack_start(cellrenderer, expand);
  g_assert(pViewColumn);
  guint cols_count = get_columns_count();
  #endif //GLOM_ENABLE_MAEMO

  sharedptr<const LayoutItem> layout_item = m_ColumnTypes[model_column_index].m_item;
  sharedptr<const LayoutItem_Field> layout_item_field = sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);

  //Tell the Treeview.how to render the Gnome::Gda::Values:
  if(layout_item_field)
  {
    pViewColumn->set_cell_data_func(cellrenderer, 
      sigc::bind( sigc::mem_fun(*this, &DbAddDel::treeviewcolumn_on_cell_data), model_column_index, data_model_column_index) );
  }

  #ifndef GLOM_ENABLE_MAEMO
  //Allow the column to be reordered by dragging and dropping the column header:
  pViewColumn->set_reorderable();

  //Allow the column to be resized:
  pViewColumn->set_resizable();
  #endif //GLOM_ENABLE_MAEMO
  
  #ifndef GLOM_ENABLE_MAEMO
  //GtkTreeView's fixed-height-mode does not allow us to have anything but 
  //the last column as expandable.
  //TODO: Can we get the total size and calculate a starting size instead?
  expand = false;
  #endif

  int column_width = -1; //Means expand.
  if(!expand)
  {
    column_width = layout_item->get_display_width();
    if(!(column_width > 0))
    {
      //TODO: Choose a width based on the first 100 values.
      if(layout_item_field)
      {
       column_width = Utils::get_suitable_field_width_for_widget(*this, layout_item_field, true /* or_title */);
       column_width = column_width / 3;
       //std::cout << "DEBUG: column_width=" << column_width << std::endl;
      }
      else
        column_width = 100; //TODO: Don't save this default in the document.
    }
  }

  #ifdef GLOM_ENABLE_MAEMO
  cellrenderer.set_property("width", column_width);
  #else
  if(column_width > 0) //Otherwise there's an assertion fails.
    pViewColumn->set_fixed_width(column_width); //This is the only way to set the width, so we need to set it as resizable again immediately afterwards.
    
  pViewColumn->set_resizable();
  //This property is read only: pViewColumn->property_width() = column_width;

  //Save the extra ID, using the title if the column_id is empty:
  const Glib::ustring column_id = m_ColumnTypes[model_column_index].m_item->get_name();
  pViewColumn->set_column_id( (column_id.empty() ? title : column_id) );

  //TODO pViewColumn->signal_button_press_event().connect( sigc::mem_fun(*this, &DbAddDel::on_treeview_columnheader_button_press_event) );

  //Let the user click on the column header to sort.
  pViewColumn->set_clickable();
  pViewColumn->signal_clicked().connect(
    sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_column_clicked), model_column_index) );

  pViewColumn->connect_property_changed("width", sigc::bind(sigc::mem_fun(*this, &DbAddDel::on_treeview_column_resized), model_column_index, pViewColumn) );
  #endif //GLOM_ENABLE_MAEMO

  return cols_count;
}


DbAddDel::type_vec_strings DbAddDel::get_columns_order() const
{
  //This list is rebuilt in on_treeview_columns_changed, but maybe we could just build it here.
  return m_vecColumnIDs;
}

Glib::RefPtr<Gtk::TreeModel> DbAddDel::get_model()
{
  return m_refListStore;
}
Glib::RefPtr<const Gtk::TreeModel> DbAddDel::get_model() const
{
  return m_refListStore;
}

bool DbAddDel::get_is_first_row(const Gtk::TreeModel::iterator& iter) const
{
  if(iter)
    return iter == get_model()->children().begin();
  else
    return false;
}

bool DbAddDel::get_is_last_row(const Gtk::TreeModel::iterator& iter) const
{
  if(iter)
  {
    //TODO: Avoid this. iter::operator() might not work properly with our custom tree model.
    return iter == get_last_row();
  }
  else
    return false;
}

Gtk::TreeModel::iterator DbAddDel::get_last_row() const
{
  if(m_refListStore)
    return m_refListStore->get_last_row();
  else
    return Gtk::TreeModel::iterator();
}

Gtk::TreeModel::iterator DbAddDel::get_last_row()
{
  if(m_refListStore)
    return m_refListStore->get_last_row();
  else
    return Gtk::TreeModel::iterator();
}

Gnome::Gda::Value DbAddDel::get_value_key(const Gtk::TreeModel::iterator& iter) const
{
  return treeview_get_key(iter);
}


void DbAddDel::set_value_key(const Gtk::TreeModel::iterator& iter, const Gnome::Gda::Value& value)
{
  if(iter && m_refListStore)
  {
    Gtk::TreeModel::Row row = *iter;

    if(!(Conversions::value_is_empty(value)))
    {
      //This is not a placeholder anymore, if it every was:
      m_refListStore->set_is_not_placeholder(iter);
      //row[*m_modelcolumn_placeholder] = false;
    }

    m_refListStore->set_key_value(iter, value);
  }
}

bool DbAddDel::get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const
{
  //g_warning("DbAddDel::get_is_placeholder_row()");

  if(!get_is_last_row(iter))
  {
    return false;
  }

  if(!iter)
    return false;

  if(!m_refListStore)
    return false;

  if(iter == m_refListStore->children().end())
  {
    return false;
  }

  return  m_refListStore->get_is_placeholder(iter);
  //Gtk::TreeModel::Row row = *iter;
  //return row[*m_modelcolumn_placeholder];
}

bool DbAddDel::get_model_column_index(guint view_column_index, guint& model_column_index)
{
//TODO: Remove this function. We should never seem to expose the underlying TreeModel modelcolumn index anyway.
  model_column_index = view_column_index;

  return true;
}

bool DbAddDel::get_view_column_index(guint model_column_index, guint& view_column_index) const
{
  //Initialize output parameter:
  view_column_index = 0;

  if(model_column_index >=  m_ColumnTypes.size())
    return false;

  if( !(m_ColumnTypes[model_column_index].m_visible) )
    return false;

  view_column_index = model_column_index;

  #ifndef GLOM_ENABLE_MAEMO
  if(m_treeviewcolumn_button)
  {
    ++view_column_index;
  }
  else
    std::cout << "m_treeviewcolumn_button is null." << std::endl;
  #endif //GLOM_ENABLE_MAEMO

  return true;
}

guint DbAddDel::get_count_hidden_system_columns() const
{
  return 0; //The key now has explicit API in the model.
  //return 1; //The key.
  //return 2; //The key and the placeholder boolean.
}

sharedptr<Field> DbAddDel::get_key_field() const
{
  return m_key_field;
}

void DbAddDel::set_key_field(const sharedptr<Field>& field)
{
  m_key_field = field;
}

#ifdef GLOM_ENABLE_MAEMO
void DbAddDel::treeviewcolumn_on_cell_data(const Gtk::TreeModel::iterator& iter, int model_column_index, int data_model_column_index)
#else
void DbAddDel::treeviewcolumn_on_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, int model_column_index, int data_model_column_index)
#endif
{
#ifdef GLOM_ENABLE_MAEMO
  Glib::RefPtr<Hildon::TouchSelectorColumn> column = touch_selector_get_column();
  g_assert(column);
  std::vector<Gtk::CellRenderer*> cells = column->get_cells();
  Gtk::CellRenderer* renderer = cells[model_column_index];
#endif
  
  //std::cout << "debug: DbAddDel::treeviewcolumn_on_cell_data()" << std::endl; 

  if(iter)
  {
    const DbAddDelColumnInfo& column_info = m_ColumnTypes[model_column_index];

    sharedptr<LayoutItem_Field> field = sharedptr<LayoutItem_Field>::cast_dynamic(column_info.m_item);
    if(field)
    {
      const guint col_real = data_model_column_index + get_count_hidden_system_columns();
      Gtk::TreeModel::Row treerow = *iter;
      Gnome::Gda::Value value;
      treerow->get_value(col_real, value);

      /*
      GType debug_type = value.get_value_type();
      std::cout << "  debug: DbAddDel::treeviewcolumn_on_cell_data(): GdaValue from TreeModel::get_value(): GType=" << debug_type << std::endl;
      if(debug_type)
         std::cout << "    GType name=\"" << g_type_name(debug_type) << "\"" << std::endl; 
      */

      const Field::glom_field_type type = field->get_glom_type();
      switch(type)
      {
        case(Field::TYPE_BOOLEAN):
        {
          Gtk::CellRendererToggle* pDerived = dynamic_cast<Gtk::CellRendererToggle*>(renderer);
          if(pDerived)
            pDerived->set_active( (value.get_value_type() == G_TYPE_BOOLEAN) && value.get_boolean() ); 

          break;
        }
        case(Field::TYPE_IMAGE):
        {
          Gtk::CellRendererPixbuf* pDerived = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
          if(pDerived)
          {
            Glib::RefPtr<Gdk::Pixbuf> pixbuf = Utils::get_pixbuf_for_gda_value(value);
         
            //Scale it down to a sensible size.
            if(pixbuf)
              pixbuf = Utils::image_scale_keeping_ratio(pixbuf,  get_fixed_cell_height(), pixbuf->get_width());

            g_object_set(pDerived->gobj(), "pixbuf", pixbuf ? pixbuf->gobj() : 0, (gpointer)0);
          }
          else
            g_warning("Field::sql(): glom_type is TYPE_IMAGE but gda type is not VALUE_TYPE_BINARY");

          break;
        }
        default:
        {
          //TODO: Maybe we should have custom cellrenderers for time, date, and numbers.
          Gtk::CellRendererText* pDerived = dynamic_cast<Gtk::CellRendererText*>(renderer);
          if(pDerived)
          {
            //std::cout << "  debug: DbAddDel::treeviewcolumn_on_cell_data(): field name=" << field->get_name() << ", glom type=" << field->get_glom_type() << std::endl;
            const Glib::ustring text = Conversions::get_text_for_gda_value(field->get_glom_type(), value, field->get_formatting_used().m_numeric_format);
            //g_assert(text != "NULL");
            g_object_set(pDerived->gobj(), "text", text.c_str(), (gpointer)0);
          }

          //Show a different color if the value is numeric, if that's specified:
          if(type == Field::TYPE_NUMERIC)
          {
             const Glib::ustring fg_color = 
               field->get_formatting_used().get_text_format_color_foreground_to_use(value);
             if(!fg_color.empty())
                 g_object_set(pDerived->gobj(), "foreground", fg_color.c_str(), (gpointer)0);
             else
                 g_object_set(pDerived->gobj(), "foreground", (const char*)0, (gpointer)0);
          }

          break;
        } 
      }
    }
  }
}

Application* DbAddDel::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<Application*>(pWindow);
}

void DbAddDel::set_allow_view(bool val)
{
  m_allow_view = val;
}

void DbAddDel::set_allow_view_details(bool val)
{
  m_allow_view_details = val;

  #ifndef GLOM_ENABLE_MAEMO
  //Hide it if it was visible, if it exists,
  //otherwise do that later after creating it:
  if(m_treeviewcolumn_button)
    m_treeviewcolumn_button->set_visible(val);
  #endif //GLOM_ENABLE_MAEMO
}

bool DbAddDel::get_allow_view_details() const
{
  return m_allow_view_details;
}

#ifdef GLOM_ENABLE_CLIENT_ONLY 
void DbAddDel::on_self_style_changed(const Glib::RefPtr<Gtk::Style>& /* style */)
{
  // Reset fixed cell height because the font might have changed due to the new style:
  m_fixed_cell_height = 0;

  // Reconstruct columns because sizes might have changed:
  // (TODO: But don't get the data again because that would be inefficient).
  construct_specified_columns();
}
#endif

void DbAddDel::set_open_button_title(const Glib::ustring& title)
{
  m_open_button_title = title;
}

void DbAddDel::show_hint_model()
{
  #ifdef GLOM_ENABLE_MAEMO
  Glib::RefPtr<Hildon::TouchSelectorColumn> column = touch_selector_get_column();
  g_assert(column);
  column->clear();
  #else
  m_TreeView.remove_all_columns();
  m_treeviewcolumn_button = 0; //When we removed the view columns, this was deleted because it's manage()ed.
  #endif //GLOM_ENABLE_MAEMO

  m_model_hint = Gtk::ListStore::create(m_columns_hint);
  Gtk::TreeModel::iterator iter = m_model_hint->append();
  (*iter)[m_columns_hint.m_col_hint] = _("Right-click to layout, to specify the related fields.");

  #ifdef GLOM_ENABLE_MAEMO
  column = touch_selector_get_column();
  g_assert(column);
  column->pack_start(m_columns_hint.m_col_hint);
  #else
  m_TreeView.set_model(m_model_hint);
  m_TreeView.set_fixed_height_mode(false); //fixed_height mode is incompatible with the default append_column() helper method.
  m_TreeView.append_column("", m_columns_hint.m_col_hint);
  #endif
}

bool DbAddDel::start_new_record()
{
  Gtk::TreeModel::iterator iter = get_item_placeholder();
  if(!iter)
    return false;
  
  sharedptr<LayoutItem_Field> fieldToEdit;

  //Start editing in the primary key or the first cell if the primary key is auto-incremented (because there is no point in editing an auto-generated value)
  //guint index_primary_key = 0;
  const bool bPresent = true; //get_field_primary_key_index(index_primary_key); //If there is no primary key then the default of 0 is OK.
  if(!bPresent)
    return false;
   
  sharedptr<Field> fieldPrimaryKey = get_key_field();
  if(fieldPrimaryKey && fieldPrimaryKey->get_auto_increment())
  {
    //Start editing in the first cell that is not auto_increment:
    for(type_ColumnTypes::iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
    {
      sharedptr<LayoutItem> layout_item = iter->m_item;
      sharedptr<LayoutItem_Field> layout_item_field = sharedptr<LayoutItem_Field>::cast_dynamic(layout_item);
      if(!(layout_item_field->get_full_field_details()->get_auto_increment()))
      {
        fieldToEdit = layout_item_field;
        break;
      }
    }
  }
  else
  {
    //The primary key is not auto-increment, so start by editing it:
    fieldToEdit = sharedptr<LayoutItem_Field>::create();
    fieldToEdit->set_full_field_details(fieldPrimaryKey);
  }

  //std::cout << "debug: index_field_to_edit=" << index_field_to_edit << std::endl;

  if(fieldToEdit)
  {
    select_item(iter, fieldToEdit, true /* start_editing */);
  }
  else
  {
    std::cout << "start_new_record(): no editable rows." << std::endl;
    //The only keys are non-editable, so just add a row:
    select_item(iter); //without start_editing.
    //g_warning("start_new_record(): index_field_to_edit does not exist: %d", index_field_to_edit);
  }
  
  return true;
}

void DbAddDel::user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  const Gnome::Gda::Value parent_primary_key_value = get_value_key(row);
  //std::cout << "DbAddDel::user_changed(): parent_primary_key_value=" << parent_primary_key_value.to_string() << std::endl;
 
  sharedptr<const LayoutItem_Field> layout_field = get_column_field(col);

  if(!Conversions::value_is_empty(parent_primary_key_value)) //If the record's primary key is filled in:
  {
    Glib::ustring table_name = m_found_set.m_table_name;
    sharedptr<Field> primary_key_field;
    Gnome::Gda::Value primary_key_value;
    Gtk::Window* window = get_application();

    //Just update the record:
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    try
#endif // GLIBMM_EXCEPTIONS_ENABLED
    {
      if(!layout_field->get_has_relationship_name())
      {
        table_name = m_found_set.m_table_name;
        primary_key_field = get_key_field();
        primary_key_value = parent_primary_key_value;
      }
      else
      {
        //If it's a related field then discover the actual table that it's in,
        //plus how to identify the record in that table.
        const Glib::ustring relationship_name = layout_field->get_relationship_name();

        Document* document = dynamic_cast<Document*>(get_document());

        sharedptr<Relationship> relationship = document->get_relationship(m_found_set.m_table_name, relationship_name);
        if(relationship)
        {
          table_name = relationship->get_to_table();
          const Glib::ustring to_field_name = relationship->get_to_field();
          //Get the key field in the other table (the table that we will change)
          primary_key_field = get_fields_for_table_one_field(table_name, to_field_name); //TODO_Performance.
          if(primary_key_field)
          {
            //Get the value of the corresponding key in the current table (that identifies the record in the table that we will change)
            sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
            layout_item->set_full_field_details( document->get_field(relationship->get_from_table(), relationship->get_from_field()) );

            primary_key_value = get_value_selected(layout_item);

            //Note: This just uses an existing record if one already exists:
            Gnome::Gda::Value primary_key_value_used;
            if(!m_find_mode)
            {
              const bool test = add_related_record_for_field(layout_field, relationship, primary_key_field, primary_key_value, primary_key_value_used);
              if(!test)
                return;
            }

            //Get the new primary_key_value if it has been created:
            primary_key_value = primary_key_value_used;

            //Now that the related record exists, the following code to set the value of the other field in the related field can succeed.
          }
          else
          {
            g_warning("Box_Data_List::on_flowtable_field_edited(): key not found for edited related field.");
          }
        }
      }

      //Update the field in the record (the record with this primary key):
      const Gnome::Gda::Value field_value = get_value(row, layout_field);
      //std::cout << "Box_Data_List::on_adddel_user_changed(): field_value = " << field_value.to_string() << std::endl;
      //const sharedptr<const Field>& field = layout_field->m_field;
      //const Glib::ustring strFieldName = layout_field->get_name();

      LayoutFieldInRecord field_in_record(layout_field, m_found_set.m_table_name /* parent */, primary_key_field, primary_key_value);

      //Check whether the value meets uniqueness constraints:
      if(!check_entered_value_for_uniqueness(m_found_set.m_table_name, row, layout_field, field_value, window))
      {
        //Revert to the value in the database:
        const Gnome::Gda::Value value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);

        return; //The value has been reverted to the value in the database.
      }


      const bool bTest = set_field_value_in_database(field_in_record, row, field_value, false /* don't use current calculations */, window);
      if(!bTest)
      {
        //Update failed.
        //Replace with correct values.
        const Gnome::Gda::Value value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);
      }
      else
        signal_record_changed().emit();
    }
#ifdef GLIBMM_EXCEPTIONS_ENABLED
    catch(const Glib::Exception& ex)
    {
      handle_error(ex);

      //Replace with correct values.
      if(primary_key_field)
      {
        LayoutFieldInRecord field_in_record(layout_field, m_found_set.m_table_name /* parent */, primary_key_field, primary_key_value);
        const Gnome::Gda::Value value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);
      }
    }
    catch(const std::exception& ex)
    {
      handle_error(ex);

      //Replace with correct values.
      if(primary_key_field)
      {
        LayoutFieldInRecord field_in_record(layout_field, m_found_set.m_table_name /* parent */, primary_key_field, primary_key_value);
        const Gnome::Gda::Value value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);
      }
    }
#endif // GLIBMM_EXCEPTIONS_ENABLED
  }
  else
  {
    //This record probably doesn't exist yet.
    //Add new record, which will generate the primary key:
    user_added(row);
    
    const Gnome::Gda::Value primaryKeyValue = get_value_key(row); //TODO_Value
    if(!(Conversions::value_is_empty(primaryKeyValue))) //If the Add succeeeded:
    {
      if(!(layout_field->get_full_field_details()->get_primary_key())) //Don't try to re-set the primary key field, because we just inserted the record with it.
      {
        user_changed(row, col); //Change this field in the new record.
      }
    }
    else
    {
      //A field value was entered, but the record has not been added yet, because not enough information exists yet.
       g_warning("Box_Data_List::on_adddel_user_changed(): debug: record not yet added.");
    }
  }
}


void DbAddDel::user_added(const Gtk::TreeModel::iterator& row)
{
  std::cout << "DEBUG: DbAddDel::user_added()" << std::endl;

  //Prevent impossible multiple related records:
  //The developer-mode UI now prevents the developer from using such a relationship anyway.
  if(m_allow_only_one_related_record && (get_count() > 0))
  {
    //Tell user that they can't do that:
    Gtk::MessageDialog dialog(Utils::bold_message(_("Extra Related Records Not Possible")), true, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text(_("You attempted to add a new related record, but there can only be one related record, because the relationship uses a unique key.")),
    dialog.set_transient_for(*Application::get_application());
    dialog.run();
    
    return;
  }
  
  //std::cout << "DbAddDel::on_adddel_user_added" << std::endl;

  Gnome::Gda::Value primary_key_value;

  sharedptr<const Field> primary_key_field = get_key_field();

  //Get the new primary key value, if one is available now:
  if(primary_key_field->get_auto_increment())
  {
    //Auto-increment is awkward (we can't get the last-generated ID) with postgres, so we auto-generate it ourselves;
    const Glib::ustring strPrimaryKeyName = primary_key_field->get_name();
    primary_key_value = DbUtils::get_next_auto_increment_value(m_found_set.m_table_name, strPrimaryKeyName);  //TODO: return a Gnome::Gda::Value of an appropriate type.
  }
  else
  {
    //Use the user-entered primary key value:

    //This only works when the primary key is already stored: primary_key_value = get_value_key(row);
    //sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
    //layout_item->set_full_field_details(field);
    
    primary_key_value = get_value_key_selected();
  }

  //If no primary key value is available yet, then don't add the record yet:
  if(Conversions::value_is_empty(primary_key_value))
    return;
    
  #ifdef GLIBMM_EXCEPTIONS_ENABLED
  sharedptr<SharedConnection> sharedconnection = connect_to_server(get_application()); //Keep it alive while we need the data_model.
  #else
  std::auto_ptr<ExceptionConnection> error;
  sharedptr<SharedConnection> sharedconnection = connect_to_server(get_application(), error); //Keep it alive while we need the data_model.
  // Ignore error - sharedconnection is checked for NULL instead:
  #endif
  if(!sharedconnection)
  {
    //Add Record failed.
    //Replace with correct values:
    fill_from_database();
    return;
  }
  

  sharedptr<LayoutItem_Field> layout_field = sharedptr<LayoutItem_Field>::create();
  layout_field->set_full_field_details(primary_key_field);
  if(!check_entered_value_for_uniqueness(m_found_set.m_table_name, layout_field, primary_key_value, get_application()))
  {
    //Revert to a blank value.
    primary_key_value = Conversions::get_empty_value(layout_field->get_full_field_details()->get_glom_type());
    set_entered_field_data(row, layout_field, primary_key_value);
    return;
  }

  if(m_find_mode)
    return;

  const bool added = record_new(true /* use entered field data*/, primary_key_value);
  if(!added)
  {
    handle_error();
    return;
  }
  
  //Save the primary key value for later use:
  set_value_key(row, primary_key_value);

  //Show the primary key in the row, if the primary key is visible:

  //If it's an auto-increment, then get the value and show it:
  if(primary_key_field->get_auto_increment())
  {
    sharedptr<LayoutItem_Field> layout_item = sharedptr<LayoutItem_Field>::create();
    layout_item->set_full_field_details(primary_key_field);
    set_value(row, layout_item, primary_key_value);
  }

  //Allow a parent widget to link the new record by setting the foreign key:
  signal_record_added().emit(row, primary_key_value);
}

void DbAddDel::user_requested_delete(const Gtk::TreeModel::iterator& rowStart, const Gtk::TreeModel::iterator&  /* rowEnd TODO */)
{
  if(rowStart)
  {
    if(confirm_delete_record())
    {
      const Gnome::Gda::Value primary_key_value = get_primary_key_value(rowStart);
      if(!m_find_mode)
        record_delete(primary_key_value);

      //Remove the row:
      remove_item(rowStart);

      //TODO_refactor: Just emit signal_record_changed() directly instead?
      on_record_deleted(primary_key_value);
    }
  }
}

//An override of the Base_DB method:
void DbAddDel::set_entered_field_data(const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  return set_value_selected(field, value);
}

//An override of the Base_DB method:
void DbAddDel::set_entered_field_data(const Gtk::TreeModel::iterator& row, const sharedptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  return set_value(row, field, value);
}

Gnome::Gda::Value DbAddDel::get_entered_field_data(const sharedptr<const LayoutItem_Field>& field) const
{
  return get_value_selected(field);
}

sharedptr<Field> DbAddDel::get_field_primary_key() const
{
  return get_key_field();
}

Gnome::Gda::Value DbAddDel::get_primary_key_value_selected() const
{
  return get_value_key_selected();
}

void DbAddDel::set_primary_key_value(const Gtk::TreeModel::iterator& row, const Gnome::Gda::Value& value)
{
  set_value_key(row, value);
}

Gnome::Gda::Value DbAddDel::get_primary_key_value(const Gtk::TreeModel::iterator& row) const
{
  return get_value_key(row);
}

Gtk::TreeModel::iterator DbAddDel::get_row_selected()
{
  return get_item_selected();
}

} //namespace Glom


