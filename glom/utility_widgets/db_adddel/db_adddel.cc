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
#include <glom/libglom/data_structure/glomconversions.h>
#include "../../dialog_invalid_data.h"
#include "../../application.h"
#include <glom/libglom/utils.h>
#include "cellrenderer_buttonimage.h"
#include "cellrenderer_buttontext.h"
#include <glom/utility_widgets/imageglom.h> //For ImageGlom::scale_keeping_ratio().
//#include "../cellrendererlist.h"
#include <iostream> //For debug output.
#include <gtk/gtktreeview.h>
#include <gtk/gtkstock.h>

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
  m_pMenuPopup(0),
  m_bAllowUserActions(true),
  m_bPreventUserSignals(false),
  m_bIgnoreTreeViewSignals(false),
  m_auto_add(true),
  m_allow_add(true),
  m_allow_delete(true),
  m_columns_ready(false),
  m_allow_view(true),
  m_allow_view_details(false),
  m_treeviewcolumn_button(0),
  m_fixed_cell_height(0)
{
  set_prevent_user_signals();
  set_ignore_treeview_signals(true);

  set_spacing(Utils::DEFAULT_SPACING_SMALL);

  //Start with a useful default TreeModel:
  //set_columns_count(1);
  //construct_specified_columns();
  

  m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_ScrolledWindow.add(m_TreeView);
  m_ScrolledWindow.set_shadow_type(Gtk::SHADOW_IN);

  m_TreeView.set_fixed_height_mode(); //This allows some optimizations.
  m_TreeView.show();

  pack_start(m_ScrolledWindow);

  //Make sure that the TreeView doesn't start out only big enough for zero items.
  m_TreeView.set_size_request(-1, 150);

  //Allow the user to change the column order:
  //m_TreeView.set_column_drag_function( sigc::mem_fun(*this, &DbAddDel::on_treeview_column_drop) );


  m_TreeView.add_events(Gdk::BUTTON_PRESS_MASK); //Allow us to catch button_press_event and button_release_event
  m_TreeView.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &DbAddDel::on_treeview_button_press_event) );

  m_TreeView.signal_columns_changed().connect( sigc::mem_fun(*this, &DbAddDel::on_treeview_columns_changed) );
  //add_blank();

  setup_menu();
  signal_button_press_event().connect(sigc::mem_fun(*this, &DbAddDel::on_button_press_event_Popup));

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
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->remove_developer_action(m_refContextLayout);
  } 
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

void
DbAddDel::on_MenuPopup_activate_Edit()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_TreeView.get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      //Discover whether it's the last (empty) row:
      //if(get_is_placeholder_row(iter))
     // {
        //This is a new entry:
       // if(m_allow_add)
       //   signal_user_added().emit(iter, 0);

        /*
        bool bRowAdded = true;

        //The rows might be re-ordered:
        Gtk::TreeModel::iterator rowAdded = iter;
        Glib::ustring strValue_Added =  get_value_key(iter);
        if(strValue_Added != strValue)
          rowAdded = get_row(strValue);

        if(bRowAdded)
          signal_user_requested_edit()(rowAdded);
        */
     // }
     // else
     // {
        //Value changed:
        signal_user_requested_edit()(iter);
     // }
    }

  }
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void DbAddDel::on_MenuPopup_activate_layout()
{
  finish_editing();
  signal_user_requested_layout().emit();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

void DbAddDel::on_MenuPopup_activate_Add()
{
  if(m_auto_add)
  {
    Gtk::TreeModel::iterator iter = get_item_placeholder();
    if(iter)
    {
      select_item(iter, true /* start_editing */);
    }
  }
  else
  {
    signal_user_requested_add().emit(); //Let the client code add the row explicitly, if it wants.
  }
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
      signal_user_requested_delete().emit(iter, iter);
    }
  }
}

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

  //TODO: This does not work until this widget is in a container in the window:s
  App_Glom* pApp = get_application();
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
  if(error.get() != NULL)
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
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }
#endif

  GdkModifierType mods;
  gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
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

Gtk::TreeModel::iterator DbAddDel::get_item_placeholder()
{
  //Get the existing placeholder row, or add one if necessary:
  return m_refListStore->get_placeholder_row();
}

Gnome::Gda::Value DbAddDel::get_value(const Gtk::TreeModel::iterator& iter, const sharedptr<const LayoutItem_Field>& layout_item)
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

Gnome::Gda::Value DbAddDel::get_value_key_selected()
{
  Gtk::TreeModel::iterator iter = get_item_selected();
  if(iter)
  {
    return get_value_key(iter);
  }
  else
    return Gnome::Gda::Value();
}

Gnome::Gda::Value DbAddDel::get_value_selected(const sharedptr<const LayoutItem_Field>& layout_item)
{
  return get_value(get_item_selected(), layout_item);
}

Gtk::TreeModel::iterator DbAddDel::get_item_selected()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
  if(refTreeSelection)
  {
     return refTreeSelection->get_selected();
  }

  return m_refListStore->children().end();
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

    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
    if(refTreeSelection)
    {
      refTreeSelection->select(iter);

      Gtk::TreeModel::Path path = m_refListStore->get_path(iter);

      guint view_column_index = 0;
      const bool test = get_view_column_index(treemodel_col, view_column_index);
      if(test)
      {
        Gtk::TreeView::Column* pColumn = m_TreeView.get_column(view_column_index);
        if(pColumn)
        {
          if(pColumn != m_treeviewcolumn_button) //This would activate the button. Let's avoid this, though it should never happen.
            m_TreeView.set_cursor(path, *pColumn, start_editing);
        }
        else
           g_warning("DbAddDel::select_item:TreeViewColumn not found.");
      }
      else
           g_warning("DbAddDel::select_item:TreeViewColumn index not found. column=%d", treemodel_col);
    }

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

guint DbAddDel::get_columns_count() const
{
  return m_TreeView.get_columns().size();
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
  if(m_fixed_cell_height > 0)
    return m_fixed_cell_height;
  else
  {
    //Discover a suitable height:
    Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout("example");
    int width = 0;
    int height = 0;
    refLayout->get_pixel_size(width, height);

    m_fixed_cell_height = height;
    return m_fixed_cell_height;
  }
}

void DbAddDel::on_cell_layout_button_clicked(const Gtk::TreeModel::Path& path, int model_column_index)
{
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

Gtk::CellRenderer* DbAddDel::construct_specified_columns_cellrenderer(const sharedptr<LayoutItem>& layout_item, int model_column_index, int data_model_column_index)
{
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
          rendererList->set_restrict_values_to_list(item_field->get_formatting_used().get_choices_restricted());

          pCellRenderer = rendererList;
        }
        else
          pCellRenderer = Gtk::manage( new Gtk::CellRendererText() );

        break;
      }
    } //switch

    //Set font and colors:
    const FieldFormatting& formatting = item_field->get_formatting_used();
    if(pCellRenderer)
      apply_formatting(pCellRenderer, formatting);
  }
  else
  {
     //Non-fields:

     sharedptr<LayoutItem_Image> item_image = sharedptr<LayoutItem_Image>::cast_dynamic(layout_item);
     if(item_image)
     {
       Gtk::CellRendererPixbuf* pixbuf_renderer = Gtk::manage( new Gtk::CellRendererPixbuf() );

       Glib::RefPtr<Gdk::Pixbuf> pixbuf = item_image->get_image_as_pixbuf();
       if(pixbuf)
         pixbuf_renderer->set_property("pixbuf", pixbuf);
       else
         pixbuf_renderer->set_property("stock-id", Gtk::Stock::MISSING_IMAGE);

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

           pCellButton->signal_clicked().connect(
             sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_cell_layout_button_clicked), model_column_index) );

           pCellRenderer = pCellButton;
         }
       }
     }
  }

  
  //Set extra cellrenderer attributes, depending on the type used:
  Gtk::CellRendererText* pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(pCellRenderer);
  if(pCellRendererText)
  {
    //Use an ellipze to indicate excessive text, 
    //so that similar values do not look equal, 
    //and to avoid multi-line comments. TODO: Is there a better way to restrict the height? This doesn't actually truncate multilines anyway.
    g_object_set(pCellRendererText->gobj(), "ellipsize", PANGO_ELLIPSIZE_END, (gpointer)NULL);

    //Restrict the height, to prevent multiline text cells,
    //and to allow TreeView performance optimisation:
    pCellRendererText->set_fixed_size(-1, get_fixed_cell_height() );

    //Connect to edited signal:
    if(item_field) //Only fields can be edited:
    {
      //Make it editable:
      pCellRendererText->set_property("editable", true);

      //Align numbers to the right: //TODO: Avoid this for ID keys.
      if(item_field->get_glom_type() == Field::TYPE_NUMERIC )
        pCellRendererText->set_property("xalign", 1.0);

      //Connect to its signal:
      pCellRendererText->signal_edited().connect(
        sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited), model_column_index, data_model_column_index) );

    }

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
            Document_Glom* document = get_document();
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
        //Connect to its signal:
        pCellRendererToggle->signal_toggled().connect(
        sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited_bool), model_column_index, data_model_column_index ) );
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

void DbAddDel::apply_formatting(Gtk::CellRenderer* renderer, const FieldFormatting& formatting)
{
  Gtk::CellRendererText* text_renderer = dynamic_cast<Gtk::CellRendererText*>(renderer);
  if(!text_renderer)
    return;

  //Use the text formatting:
  const Glib::ustring font_desc = formatting.get_text_format_font();
  if(!font_desc.empty())
    text_renderer->property_font() = font_desc;

  const Glib::ustring fg = formatting.get_text_format_color_foreground();
  if(!fg.empty())
    text_renderer->property_foreground() = fg;

  const Glib::ustring bg = formatting.get_text_format_color_background();
  if(!bg.empty())
    text_renderer->property_background() = bg;
}

void DbAddDel::construct_specified_columns()
{
  //std::cout << "debug: DbAddDel::construct_specified_columns" << std::endl;

  InnerIgnore innerIgnore(this);

  //TODO_optimisation: This is called many times, just to simplify the API.

  //Delay actual use of set_column_*() stuff until this method is called.

  if(m_ColumnTypes.empty())
  {
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
    m_refListStore = type_model_store::create(record, m_found_set, fields, column_index_key, m_allow_view);
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

  m_TreeView.set_model(m_refListStore);


  //Remove all View columns:
  m_TreeView.remove_all_columns();

  //Add new View Colums:
  int model_column_index = 0; //Not including the hidden internal columns.
  int view_column_index = 0;
  {
    GlomCellRenderer_ButtonImage* pCellButton = Gtk::manage(new GlomCellRenderer_ButtonImage());
    pCellButton->signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel::on_cell_button_clicked));


    m_treeviewcolumn_button = Gtk::manage(new Gtk::TreeViewColumn());
    m_treeviewcolumn_button->pack_start(*pCellButton);


    int x_offset, y_offset, width, height;
    pCellButton->get_size(m_TreeView, x_offset, y_offset, width, height);

    m_treeviewcolumn_button->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED); //Need by fixed-height mode.

    // TODO: I am not sure whether this is always correct. Perhaps, we also
    // have to take into account the xpad property of the cell renderer and
    // the spacing property of the treeviewcolumn.
    int horizontal_separator;
    m_TreeView.get_style_property("horizontal-separator", horizontal_separator);
    m_treeviewcolumn_button->set_fixed_width(width + horizontal_separator*2);

    m_treeviewcolumn_button->set_property("visible", true);

    m_TreeView.append_column(*m_treeviewcolumn_button);

    ++view_column_index;
  }

  bool no_columns_used = true;
  int data_model_column_index = 0;
  
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
        treeview_append_column(column_name, *pCellRenderer, model_column_index, item_data_model_column_index);

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
    //We must set this each time, because show_hint_model() might unset it:
    m_TreeView.set_fixed_height_mode(); //This allows some optimizations.
  }

  m_TreeView.columns_autosize();

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
        guint treemodel_col = *iter + get_count_hidden_system_columns();
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

  m_columns_ready = false;
}

void DbAddDel::set_table_name(const Glib::ustring& table_name)
{
  m_found_set.m_table_name = table_name;
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

void DbAddDel::set_column_choices(guint col, const type_vecStrings& vecStrings)
{
  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add new columns.

  m_ColumnTypes[col].m_choices = vecStrings;

  guint view_column_index = 0;
  bool test = get_view_column_index(col, view_column_index);
  if(test)
  { 
    CellRendererList* pCellRenderer = dynamic_cast<CellRendererList*>( m_TreeView.get_column_cell_renderer(view_column_index) );
    if(pCellRenderer)
    {
      //Add the choices:
      pCellRenderer->remove_all_list_items();
      for(type_vecStrings::const_iterator iter = vecStrings.begin(); iter != vecStrings.end(); ++iter)
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
  m_refContextAdd->set_sensitive(val);
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

void DbAddDel::set_show_column_titles(bool bVal)
{
  m_TreeView.set_headers_visible(bVal);
}


void DbAddDel::set_column_width(guint /* col */, guint /*width*/)
{
//  if( col < (guint)m_Sheet.get_columns_count())
//    m_Sheet.set_column_width(col, width);
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
  if(iter)
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
{
  m_pOuter = pOuter;

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

Gnome::Gda::Value DbAddDel::treeview_get_key(const Gtk::TreeModel::iterator& row)
{
  Gnome::Gda::Value value;

  if(m_refListStore)
  {
    return m_refListStore->get_key_value(row);
  }

  return value;
}

void DbAddDel::on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index, int data_model_column_index)
{
  //Note:: model_column_index is actually the AddDel column index, not the TreeModel column index.

  if(path_string.empty())
    return;

  const Gtk::TreePath path(path_string);

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

    int iCount = m_refListStore->children().size();
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
      //m_signal_user_added.emit(row);
    }
    else if(bIsChange)
    {
      //Existing item changed:

      m_signal_user_changed.emit(row, model_column_index);
    }
  }
}

void DbAddDel::on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index, int data_model_column_index)
{
  //Note:: model_column_index is actually the AddDel column index, not the TreeModel column index.
  if(path_string.empty())
    return;

  const Gtk::TreePath path(path_string);

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
    bool do_signal = true;

    if(get_allow_user_actions()) //If add is possible:
    {
      if(get_is_placeholder_row(iter))
      {
        bool bPreventUserSignals = get_prevent_user_signals();
        set_prevent_user_signals(true); //Stops extra signal_user_changed.

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

          do_signal = false;
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
          m_signal_user_added.emit(row, model_column_index);
      }
      else if(bIsChange)
      {
        //Existing item changed:
        //Check that it has really changed - get the last value.
        if(value != valOld)
        {
          if(do_signal)
            m_signal_user_changed.emit(row, model_column_index);
        }
      }
    }
  }
}

DbAddDel::type_signal_user_added DbAddDel::signal_user_added()
{
  return m_signal_user_added;
}

DbAddDel::type_signal_user_changed DbAddDel::signal_user_changed()
{
  return m_signal_user_changed;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
DbAddDel::type_signal_user_requested_layout DbAddDel::signal_user_requested_layout()
{
  return m_signal_user_requested_layout;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

DbAddDel::type_signal_user_requested_delete DbAddDel::signal_user_requested_delete()
{
  return m_signal_user_requested_delete;
}

DbAddDel::type_signal_user_requested_edit DbAddDel::signal_user_requested_edit()
{
  return m_signal_user_requested_edit;
}

DbAddDel::type_signal_user_requested_add DbAddDel::signal_user_requested_add()
{
  return m_signal_user_requested_add;
}

DbAddDel::type_signal_user_activated DbAddDel::signal_user_activated()
{
  return m_signal_user_activated;
}

DbAddDel::type_signal_user_reordered_columns DbAddDel::signal_user_reordered_columns()
{
  return m_signal_user_reordered_columns;
}

DbAddDel::type_signal_script_button_clicked DbAddDel::signal_script_button_clicked()
{
  return m_signal_script_button_clicked;
}

void DbAddDel::on_treeview_button_press_event(GdkEventButton* event)
{
  if(event->type == GDK_BUTTON_PRESS) //Whatever would cause cellrenderer activation.
  {
    //This is really horrible code:
    //Maybe we can improve the gtkmm API for this:

    //Get the row and column:
    Gtk::TreeModel::Path path;
    Gtk::TreeView::Column* pColumn = 0;
    int cell_x = 0;
    int cell_y = 0;  

    // Make sure to use the non-deprecated const version:
    bool row_exists = static_cast<const Gtk::TreeView&>(m_TreeView).get_path_at_pos((int)event->x, (int)event->y, path, pColumn, cell_x, cell_y);

    //Get the row:
    if(row_exists && m_refListStore)
    {
      Gtk::TreeModel::iterator iterRow = m_refListStore->get_iter(path);
      if(iterRow)
      {
        //Get the column:
        int tree_col = 0;
        int col_index = get_count_hidden_system_columns();

        typedef std::vector<Gtk::TreeView::Column*> type_vecTreeViewColumns;
        type_vecTreeViewColumns vecColumns = m_TreeView.get_columns();
        for(type_vecTreeViewColumns::const_iterator iter = vecColumns.begin(); iter != vecColumns.end(); iter++)
        {
          if(*iter == pColumn)
            tree_col = col_index; //Found.

          col_index++;
        }

        signal_user_activated().emit(iterRow, tree_col);
      }
    }
  }

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

guint DbAddDel::treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, int model_column_index, int data_model_column_index)
{
  DbTreeViewColumnGlom* pViewColumn = Gtk::manage( new DbTreeViewColumnGlom(Utils::string_escape_underscores(title), cellrenderer) );
  pViewColumn->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED); //Need by fixed-height mode.

  guint cols_count = m_TreeView.append_column(*pViewColumn);

  sharedptr<const LayoutItem> layout_item = m_ColumnTypes[model_column_index].m_item;
  sharedptr<const LayoutItem_Field> layout_item_field = sharedptr<const LayoutItem_Field>::cast_dynamic(layout_item);

  //Tell the TreeView how to render the Gnome::Gda::Values:
  if(layout_item_field)
  {
    pViewColumn->set_cell_data_func(cellrenderer, 
      sigc::bind( sigc::mem_fun(*this, &DbAddDel::treeviewcolumn_on_cell_data), model_column_index, data_model_column_index) );
  }

  //Allow the column to be reordered by dragging and dropping the column header:
  pViewColumn->set_reorderable();

  //Allow the column to be resized:
  pViewColumn->set_resizable();
  
  guint column_width = 0;
  if(!layout_item->get_display_width(column_width)) //Not saved in document, but remembered when the column is resized.
     column_width = 100; //Fairly sensible default. TODO: Choose a width based on the first 100 values.

  pViewColumn->set_fixed_width((int)column_width); //This is the only way to set the width, so we need to set it as resizable again immediately afterwards.
  pViewColumn->set_resizable();
  //This property is read only: pViewColumn->property_width() = (int)column_width;

  //Save the extra ID, using the title if the column_id is empty:
  Glib::ustring column_id = m_ColumnTypes[model_column_index].m_item->get_name();
  pViewColumn->set_column_id( (column_id.empty() ? title : column_id) );

  //TODO pViewColumn->signal_button_press_event().connect( sigc::mem_fun(*this, &DbAddDel::on_treeview_columnheader_button_press_event) );

  //Let the user click on the column header to sort.
  pViewColumn->set_clickable();
  pViewColumn->signal_clicked().connect(
    sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_column_clicked), model_column_index) );

  pViewColumn->connect_property_changed("width", sigc::bind(sigc::mem_fun(*this, &DbAddDel::on_treeview_column_resized), model_column_index, pViewColumn) );

  return cols_count;
}


void DbAddDel::on_treeview_column_resized(int model_column_index, DbTreeViewColumnGlom* view_column)
{
  if(!view_column)
    return;

  DbAddDelColumnInfo& column_info = m_ColumnTypes[model_column_index];

  guint width = (guint)view_column->get_width();
  //std::cout << "  DbAddDel::on_treeview_column_resized(): width=" << width << std::endl;

  if(column_info.m_item)
      column_info.m_item->set_display_width(width);
}

void DbAddDel::on_treeview_column_clicked(int model_column_index)
{
  Bakery::BusyCursor busy_cursor(get_application());

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
    m_signal_user_reordered_columns.emit();
  }
}

DbAddDel::type_vecStrings DbAddDel::get_columns_order() const
{
  //This list is rebuilt in on_treeview_columns_changed, but maybe we could just build it here.
  return m_vecColumnIDs;
}

void DbAddDel::set_auto_add(bool value)
{
  m_auto_add = value;
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
  return m_refListStore->get_last_row();
}

Gtk::TreeModel::iterator DbAddDel::get_last_row()
{
  return m_refListStore->get_last_row();
}

Gnome::Gda::Value DbAddDel::get_value_key(const Gtk::TreeModel::iterator& iter)
{
  return treeview_get_key(iter);
}


void DbAddDel::set_value_key(const Gtk::TreeModel::iterator& iter, const Gnome::Gda::Value& value)
{
  if(iter)
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

bool DbAddDel::get_view_column_index(guint model_column_index, guint& view_column_index)
{
  //Initialize output parameter:
  view_column_index = 0;

  if(model_column_index >=  m_ColumnTypes.size())
    return false;

  if( !(m_ColumnTypes[model_column_index].m_visible) )
    return false;

  view_column_index = model_column_index;
  if(m_treeviewcolumn_button)
  {
    ++view_column_index;
  }
  else
    std::cout << "m_treeviewcolumn_button is null." << std::endl;

  return true;
}

guint DbAddDel::get_count_hidden_system_columns()
{
  return 0; //The key now has explicit API in the model.
  //return 1; //The key.
  //return 2; //The key and the placeholder boolean.
}

void DbAddDel::set_rules_hint(bool val)
{
  m_TreeView.set_rules_hint(val);
}

sharedptr<Field> DbAddDel::get_key_field() const
{
  return m_key_field;
}

void DbAddDel::set_key_field(const sharedptr<Field>& field)
{
  m_key_field = field;
}

void DbAddDel::treeviewcolumn_on_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, int model_column_index, int data_model_column_index)
{
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

      switch(field->get_glom_type())
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
            Glib::RefPtr<Gdk::Pixbuf> pixbuf = Conversions::get_pixbuf_for_gda_value(value);
         
            //Scale it down to a sensible size.
            if(pixbuf)
              pixbuf = ImageGlom::scale_keeping_ratio(pixbuf,  get_fixed_cell_height(), pixbuf->get_width());

            g_object_set(pDerived->gobj(), "pixbuf", pixbuf ? pixbuf->gobj() : NULL, (gpointer)NULL);
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
            g_object_set(pDerived->gobj(), "text", text.c_str(), (gpointer)NULL);
          }

          break;
        } 
      }
    }
  }
}

App_Glom* DbAddDel::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

void DbAddDel::set_allow_view(bool val)
{
  m_allow_view = val;
}

void DbAddDel::set_allow_view_details(bool val)
{
  m_allow_view_details = val;

  //Hide it if it was visible:
  if(m_treeviewcolumn_button)
    g_object_set(m_treeviewcolumn_button->gobj(), "visible", get_allow_view_details() ? TRUE : FALSE, (gpointer)NULL);
}

bool DbAddDel::get_allow_view_details() const
{
  return m_allow_view_details;
}

void DbAddDel::on_cell_button_clicked(const Gtk::TreeModel::Path& path)
{
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter)
  {
    select_item(iter, false /* start_editing */);
  }

  on_MenuPopup_activate_Edit();
}

#ifdef GLOM_ENABLE_CLIENT_ONLY 
void DbAddDel::on_self_style_changed(const Glib::RefPtr<Gtk::Style>& style)
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
  m_TreeView.remove_all_columns();
  m_treeviewcolumn_button = 0; //When we removed the view columns, this was deleted because it's manage()ed.

  m_model_hint = Gtk::ListStore::create(m_columns_hint);
  Gtk::TreeModel::iterator iter = m_model_hint->append();
  (*iter)[m_columns_hint.m_col_hint] = _("Right-click to layout, to specify the related fields.");

  m_TreeView.set_model(m_model_hint);

  m_TreeView.set_fixed_height_mode(false); //fixed_height mode is incompatible with the default append_column() helper method.
  m_TreeView.append_column("", m_columns_hint.m_col_hint);
}

} //namespace Glom


