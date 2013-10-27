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
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 */

#include "config.h" // For GLOM_ENABLE_CLIENT_ONLY

#include <glom/utility_widgets/adddel/adddel.h>
#include <algorithm> //For std::find.
#include <glibmm/i18n.h>
#include <glom/utility_widgets/cellrendererlist.h>
#include <glom/utility_widgets/adddel/treeviewcolumn_glom.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/dialog_invalid_data.h>
#include <glom/utils_ui.h>
#include <libglom/utils.h>
#include <gtkmm/builder.h>
#include <giomm/menu.h>
//#include <glom/bakery/app_gtk.h>
#include <iostream> //For debug output.

namespace Glom
{

AddDelColumnInfo::AddDelColumnInfo()
: m_style(STYLE_Text),
  m_field_type(Field::TYPE_INVALID),
  m_editable(true),
  m_visible(true),
  m_prevent_duplicates(false)
{
}

AddDelColumnInfo::AddDelColumnInfo(const AddDelColumnInfo& src)
: m_style(src.m_style),
  m_name(src.m_name),
  m_id(src.m_id),
  m_field_type(src.m_field_type),
  m_choices(src.m_choices),
  m_editable(src.m_editable),
  m_visible(src.m_visible),
  m_prevent_duplicates(src.m_prevent_duplicates)
{
}

AddDelColumnInfo& AddDelColumnInfo::operator=(const AddDelColumnInfo& src)
{
  m_style = src.m_style;;
  m_name = src.m_name;
  m_id = src.m_id;
  m_field_type = src.m_field_type;
  m_choices = src.m_choices;
  m_editable = src.m_editable;
  m_visible = src.m_visible;
  m_prevent_duplicates = src.m_prevent_duplicates;

  return *this;
}

AddDel::AddDel()
: Gtk::Box(Gtk::ORIENTATION_VERTICAL),
  m_col_key(0),
  m_pMenuPopup(0),
  m_auto_add(true),
  m_allow_add(true),
  m_allow_delete(true)
{
  init();
}


AddDel::AddDel(BaseObjectType* cobject, const Glib::RefPtr<Gtk::Builder>& /* builder */)
: Gtk::Box(cobject),
  m_col_key(0),
  m_pMenuPopup(0),
  m_auto_add(true),
  m_allow_add(true),
  m_allow_delete(true)
{
  init();
}

void AddDel::init()
{
  set_prevent_user_signals();
  set_ignore_treeview_signals();

  set_spacing(Utils::DEFAULT_SPACING_SMALL);

  m_bAllowUserActions = true;

  //Start with a useful default TreeModel:
  //set_columns_count(1);
  //construct_specified_columns();

  m_ScrolledWindow.add(m_TreeView);
  m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_ScrolledWindow.set_shadow_type(Gtk::SHADOW_IN);
  m_TreeView.show();
  pack_start(m_ScrolledWindow);

  //Make sure that the TreeView doesn't start out only big enough for zero items.
  m_TreeView.set_size_request(-1, 150);

  //Allow the user to change the column order:
  //m_TreeView.set_column_drag_function( sigc::mem_fun(*this, &AddDel::on_treeview_column_drop) );


  //m_TreeView.add_events(Gdk::BUTTON_PRESS_MASK); //Allow us to catch button_press_event and button_release_event
  m_TreeView.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &AddDel::on_treeview_button_press_event) );

  m_TreeView.signal_columns_changed().connect( sigc::mem_fun(*this, &AddDel::on_treeview_columns_changed) );
  //add_blank();

  setup_menu(this);
  signal_button_press_event().connect(sigc::mem_fun(*this, &AddDel::on_button_press_event_Popup));

  set_prevent_user_signals(false);
  set_ignore_treeview_signals(false);

  remove_all_columns(); //set up the default columns.

  show_all_children();
}


AddDel::~AddDel()
{
}

void AddDel::set_treeview_accessible_name(const Glib::ustring& name)
{
#ifdef GTKMM_ATKMM_ENABLED
  m_TreeView.get_accessible()->set_name(name);
#endif
}

void AddDel::warn_about_duplicate()
{
  Glib::ustring message;

  if(m_prevent_duplicates_warning.empty())
    message = _("This item already exists. Please try again.");
  else
    message = m_prevent_duplicates_warning; //Something more specific and helpful.

  Gtk::MessageDialog dialog(Utils::bold_message(_("Duplicate")), true, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK);
  dialog.set_secondary_text(message);

  //TODO: dialog.set_transient_for(get_parent_window());

  dialog.run();
}

void
AddDel::on_MenuPopup_activate_Edit()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_TreeView.get_selection();
  if(refSelection)
  {
    Gtk::TreeModel::iterator iter = refSelection->get_selected();
    if(iter)
    {
      //Discover whether it's the last (empty) row:
      if(get_is_placeholder_row(iter))
      {
        if(row_has_duplicates(iter))
        {
          warn_about_duplicate();
        }
        else
        {
          //This is a new entry:
          signal_user_added()(iter);

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
        }
      }
      else
      {
        //Value changed:
        signal_user_requested_edit()(iter);
      }
    }

  }
}

void AddDel::on_MenuPopup_activate_Delete()
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

void AddDel::setup_menu(Gtk::Widget* /* widget */)
{
  m_refActionGroup = Gio::SimpleActionGroup::create();

  m_refContextEdit = m_refActionGroup->add_action("edit",
   sigc::mem_fun(*this, &AddDel::on_MenuPopup_activate_Edit) );

  if(get_allow_user_actions())
  {
    m_refContextDelete = m_refActionGroup->add_action("delete",
      sigc::mem_fun(*this, &AddDel::on_MenuPopup_activate_Delete) );
  }

  insert_action_group("context", m_refActionGroup);


  //TODO: add_accel_group(builder->get_accel_group());

  Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
  menu->append(_("_Edit"), "context.edit");
  menu->append(_("_Delete"), "context.delete");

  m_pMenuPopup = new Gtk::Menu(menu);
  m_pMenuPopup->attach_to_widget(*this);
}

bool AddDel::on_button_press_event_Popup(GdkEventButton *event)
{
  GdkModifierType mods;
  gdk_window_get_device_position( gtk_widget_get_window (Gtk::Widget::gobj()), event->device, 0, 0, &mods );
  if(mods & GDK_BUTTON3_MASK)
  {
    //Give user choices of actions on this item:
    m_pMenuPopup->popup(event->button, event->time);
  }
  else
  {
    if(event->type == GDK_2BUTTON_PRESS)
    {
      //Double-click means edit.
      //Disable this, because it is confusing when single-click activates editable cells too.
    }
  }

  return true;
}

Gtk::TreeModel::iterator AddDel::get_item_placeholder()
{
  //Get the existing placeholder row, or add one if necessary:
  Gtk::TreeModel::iterator iter = get_last_row();
  if( get_is_placeholder_row(iter) )
  {
    return iter;
  }
  else
  {
    return add_item_placeholder();
  }
}

Gtk::TreeModel::iterator AddDel::add_item_placeholder()
{
  Gtk::TreeModel::iterator iter = m_refListStore->append();
  if(iter)
  {
    iter->set_value(m_col_key, Glib::ustring("")); //Remove temporary key value.
    iter->set_value(m_col_placeholder, true); //This will be unset when set_value() is used to put real data in this column.
  }

  return iter;
}

Gtk::TreeModel::iterator AddDel::add_item(const Glib::ustring& strKey)
{
  Gtk::TreeModel::iterator result = get_next_available_row_with_add_if_necessary();

  if(result)
  {
    Gtk::TreeModel::Row treerow = *result;
    if(treerow)
    {
      result->set_value(m_col_key, strKey);
      result->set_value(m_col_placeholder, false);
    }
  }

  add_blank(); //if necessary

  return result;
}

void AddDel::remove_all()
{
  InnerIgnore innerIgnore(this); //see comments for InnerIgnore class.

  if(m_refListStore)
  {
    Gtk::TreeModel::iterator iter = m_refListStore->children().begin();
    while(iter)
    {
      m_refListStore->erase(iter);
      iter = m_refListStore->children().begin();
    }
  }
}


Glib::ustring AddDel::get_value(const Gtk::TreeModel::iterator& iter, guint col)
{
  Glib::ustring value;

  if(m_refListStore)
  {
    Gtk::TreeModel::Row treerow = *iter;

    if(treerow)
    {
      const guint col_real = col;
      //Get different types of data, depending on the column:
      if(m_ColumnTypes[col_real].m_style == AddDelColumnInfo::STYLE_Boolean)
      {
        bool bValue = false;
        treerow.get_value(col_real, bValue);

        //Create a string representation of the value:
        value = bValue  ? "true" : "false";
      }
      else
      {
        treerow.get_value(col_real, value);
      }
    }
  }

  return value;
}

bool AddDel::get_value_as_bool(const Gtk::TreeModel::iterator& iter, guint col)
{
//TODO: I doubt that this works. It should really get the value from the treeview as a bool. murrayc
  Glib::ustring strValue = get_value(iter, col);
  return (strValue == "true");
}

Glib::ustring AddDel::get_value_key_selected()
{
  return get_value_selected(m_col_key);
}

Glib::ustring AddDel::get_value_selected(guint col)
{
  Glib::ustring strValue = get_value(get_item_selected(), col);
  return strValue;
}


Gtk::TreeModel::iterator AddDel::get_item_selected()
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
  if(refTreeSelection)
  {
     return refTreeSelection->get_selected();
  }

  return m_refListStore->children().end();
}

Gtk::TreeModel::iterator AddDel::get_row(const Glib::ustring& key)
{
  for(Gtk::TreeModel::iterator iter = m_refListStore->children().begin(); iter != m_refListStore->children().end(); ++iter)
  {
    const Glib::ustring strTemp = get_value(iter, m_col_key);
    if(strTemp == key)
    {
      return iter;
    }
  }

  return  m_refListStore->children().end();
}

bool AddDel::select_item(const Gtk::TreeModel::iterator& iter)
{
  guint col_first = 0;
  get_model_column_index(0, col_first);
  return select_item(iter, col_first);
}

bool AddDel::select_item(const Gtk::TreeModel::iterator& iter, guint column, bool start_editing)
{
  if(!m_refListStore)
    return false;

  InnerIgnore innerIgnore(this); //see comments for InnerIgnore class

  bool bResult = false;

  if(iter)
  {
    Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
    if(refTreeSelection)
    {
      refTreeSelection->select(iter);

      Gtk::TreeModel::Path path = m_refListStore->get_path(iter);

      guint view_column_index = 0;
      bool test = get_view_column_index(column, view_column_index);
      if(test)
      {
        Gtk::TreeView::Column* pColumn = m_TreeView.get_column(view_column_index);
        if(pColumn)
          m_TreeView.set_cursor(path, *pColumn, start_editing);
        else
           g_warning("AddDel::select_item:TreeViewColumn not found.");
      }
      else
           g_warning("AddDel::select_item:TreeViewColumn index not found. column=%d", column);
    }

    bResult = true;
  }

  return bResult;
}

bool AddDel::select_item(const Glib::ustring& strItemText, guint column, bool start_editing)
{
  Gtk::TreeModel::iterator iter = get_row(strItemText);
  if(iter)
  {
    return select_item(iter, column, start_editing);
  }

  return false; //failed
}

guint AddDel::get_count() const
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

void AddDel::add_blank()
{
  bool bPreventUserSignals = get_prevent_user_signals();
  set_prevent_user_signals(true);

  bool bAddNewBlank = false;

  if(get_allow_user_actions()) //The extra blank line is only used if the user may add items:
  {
    Gtk::TreeModel::iterator iter = get_last_row();
    if(get_is_placeholder_row(iter))
    {
        bAddNewBlank  = false; //One already exists.
    }
    else
    {
      bAddNewBlank = true; // The last line isn't a placeholder. Add one.
    }
  }

  if(bAddNewBlank)
    add_item_placeholder();

  set_prevent_user_signals(bPreventUserSignals);
}


guint AddDel::get_columns_count() const
{
  return m_TreeView.get_columns().size();
}

/*
void AddDel::set_columns_count(guint count)
{
  m_ColumnTypes.resize(count, STYLE_Text);
  m_vecColumnNames.resize(count);
}
*/

/*
void AddDel::set_column_title(guint col, const Glib::ustring& strText)
{
  bool bPreventUserSignals = get_prevent_user_signals();
  set_prevent_user_signals(true);

  Gtk::TreeViewColumn* pColumn = m_TreeView.get_column(col);
  if(pColumn)
    pColumn->set_title(strText);


  set_prevent_user_signals(bPreventUserSignals);
}
*/

void AddDel::construct_specified_columns()
{
  //TODO_optimisation: This is called many times, just to simplify the API.

  //Delay actual use of set_column_*() stuff until this method is called.

  if(m_ColumnTypes.empty())
    return;

  typedef std::vector< Gtk::TreeModelColumnBase* > type_vecModelColumns;
  type_vecModelColumns vecModelColumns(m_ColumnTypes.size(), 0);

  //Create the Gtk ColumnRecord:

  Gtk::TreeModel::ColumnRecord record;
  {
    type_vecModelColumns::size_type i = 0;
    for(type_ColumnTypes::iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
    {
      Gtk::TreeModelColumnBase* pModelColumn = 0;

      AddDelColumnInfo column_info = *iter;
      switch(column_info.m_style)
      {
        //Create an appropriate type of Model Column:
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          pModelColumn = new Gtk::TreeModelColumn<bool>();
          break;
        }
        case(AddDelColumnInfo::STYLE_Numerical):
        {
          pModelColumn = new Gtk::TreeModelColumn<int>(); //TODO: Actually there are many different numeric types.
          break;
        }
        default:
        {
          pModelColumn = new Gtk::TreeModelColumn<Glib::ustring>();
          break;
        }
      }

      //Store it so we can use it and delete it later:
      vecModelColumns[i] = pModelColumn;

      record.add( *pModelColumn );
      i++;
    }
  }

  //Create the model from the ColumnRecord:
  m_refListStore = Gtk::ListStore::create(record);
  m_TreeView.set_model(m_refListStore);

  //Remove all View columns:
  m_TreeView.remove_all_columns();

  //Add new View Colums:
  int model_column_index = 0;
  int view_column_index = 0;
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
    type_vecModelColumns::value_type& pModelColumn = *iter;

    if(m_ColumnTypes[model_column_index].m_visible)
    {
      const Glib::ustring column_name = m_ColumnTypes[model_column_index].m_name;
      const Glib::ustring column_id = m_ColumnTypes[model_column_index].m_id;

      switch(m_ColumnTypes[model_column_index].m_style)
      {
        case(AddDelColumnInfo::STYLE_Choices):
        {
          //Use a custom CellRenderer:
          CellRendererList* pCellRenderer = Gtk::manage( new CellRendererList() );

          //Add the choices:
          const type_vec_strings vecStrings = m_ColumnTypes[model_column_index].m_choices;
          for(type_vec_strings::const_iterator iter = vecStrings.begin(); iter != vecStrings.end(); ++iter)
          {
            pCellRenderer->append_list_item(*iter);
          }

          // Append the View column.
          // We use a derived Gtk::TreeViewColumn so that we can store extra information in it.
          // This means that we must reimplement the code from the convenience template methods from gtkmm.
          treeview_append_column( Utils::string_escape_underscores(column_name), *pCellRenderer,  *pModelColumn, column_id);

          break;
        }
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          //Use whatever standard CellRenderer gtkmm thinks is appropriate:

          //Cast to the derived type, because append_column<> is templated, and needs the type at compile-time
          //to use the correct specialization:

          Gtk::TreeModelColumn<bool>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<bool>* >(pModelColumn);
          if(pModelColumnDerived)
            treeview_append_column(Utils::string_escape_underscores(column_name), *pModelColumnDerived, column_id);

          break;
        }
        default:
        {
          //Use whatever standard CellRenderer gtkmm thinks is appropriate:

          //Cast to the derived type, because append_column<> is templated, and needs the type at compile-time
          //to use the correct specialization:
          Gtk::TreeModelColumn<Glib::ustring>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<Glib::ustring>* >(pModelColumn);
          if(pModelColumnDerived)
            treeview_append_column(Utils::string_escape_underscores(column_name), *pModelColumnDerived, column_id);

          break;
        }
      } //switch

      if(m_ColumnTypes[model_column_index].m_editable)
      {
        Gtk::CellRendererText* pCellRenderer = dynamic_cast<Gtk::CellRendererText*>(m_TreeView.get_column_cell_renderer(view_column_index));
        if(pCellRenderer)
        {
          //Connect a signal handler:
          if(pCellRenderer)
          {
            //Make it editable:
            pCellRenderer->property_editable() = true;

            //Connect to its signal:
            pCellRenderer->signal_edited().connect(
              sigc::bind( sigc::mem_fun(*this, &AddDel::on_treeview_cell_edited), model_column_index) );
          }
        }
        else
        {
           Gtk::CellRendererToggle* pCellRenderer = dynamic_cast<Gtk::CellRendererToggle*>(m_TreeView.get_column_cell_renderer(view_column_index));
           if(pCellRenderer)
           {
             pCellRenderer->property_activatable() = true;

             //Connect to its signal:
             pCellRenderer->signal_toggled().connect(
               sigc::bind( sigc::mem_fun(*this, &AddDel::on_treeview_cell_edited_bool), model_column_index ) );
           }
        }
      }

      //Connect other signals:
      Gtk::CellRenderer* pCellRenderer = m_TreeView.get_column_cell_renderer(view_column_index);
      if(pCellRenderer)
        pCellRenderer->signal_editing_started().connect(
          sigc::bind( sigc::mem_fun(*this, &AddDel::on_treeview_cell_editing_started), model_column_index) );

       ++view_column_index;
    } //is visible

    ++model_column_index;
  }

  //Delete the vector's items:
  model_column_index = 0;
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
     Gtk::TreeModelColumnBase* pModelColumn = *iter;

     if(model_column_index < (int)m_ColumnTypes.size())
     {
       AddDelColumnInfo::enumStyles style = m_ColumnTypes[model_column_index].m_style;
       switch(style)
       {
        //Cast it to the derived type, so we can delete it properly.
        //This is necessary because TreeModelColumnBase's destructor is not virtual.
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          Gtk::TreeModelColumn<bool>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<bool>* >(pModelColumn);
          delete pModelColumnDerived;
          break;
        }
        case(AddDelColumnInfo::STYLE_Numerical):
        {
          Gtk::TreeModelColumn<int>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<int>* >(pModelColumn);
          delete pModelColumnDerived;
          break;
        }
        default:
        {
          Gtk::TreeModelColumn<Glib::ustring>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<Glib::ustring>* >(pModelColumn);
          delete pModelColumnDerived;
          break;
        }

        *iter = 0;
      }
    }
    else
    {
      std::cerr << G_STRFUNC << ": Leaking a Gtk::TreeModelColumn<>." << std::endl;
    }

    ++model_column_index;
  }

  m_TreeView.columns_autosize();
}

/*
void AddDel::set_value(const Gtk::TreeModel::iterator& iter, guint col, const Gnome::Gda::Value& value)
{
  //Different model columns have different types of data:
  switch(m_ColumnTypes[col].m_style)
  {
    case(AddDelColumnInfo::STYLE_Boolean):
    {
      std::cerr << G_STRFUNC << ": boolean column being set as bool." << std::endl;
      set_value(iter, col, value.get_bool());
      break;
    }
    default:
    {
      set_value( iter, col, Conversions::get_text_for_gda_value(m_ColumnTypes[col].m_field_type, value) );
      break;
    }
  }
}
*/

void AddDel::set_value(const Gtk::TreeModel::iterator& iter, guint col, const Glib::ustring& strValue)
{
  InnerIgnore innerIgnore(this);

  if(!m_refListStore)
    std::cerr << G_STRFUNC << ": No model." << std::endl;
  else
  {
    Gtk::TreeModel::Row treerow = *iter;
    if(treerow)
    {
      //Different model columns have different types of data:
      switch(m_ColumnTypes[col].m_style)
      {
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          bool bValue = (strValue == "true");
          treerow.set_value(col, bValue);
          break;
        }
        default:
        {
          treerow.set_value(col, strValue);
          break;
        }
      }

      //Mark this row as not a placeholder because it has real data now.
      if( (col != m_col_key) && (col != m_col_placeholder) )
      {
        if(!strValue.empty())
        {
           //treerow.set_value(m_col_key, Glib::ustring("placeholder debug value setted"));
           //treerow.set_value(m_col_placeholder, false);
        }
      }
    }

    //Add extra blank if necessary:
    add_blank();
  }
}

void AddDel::set_value(const Gtk::TreeModel::iterator& iter, guint col, unsigned long ulValue)
{
  gchar pchValue[10] = {0};
  sprintf(pchValue, "%d", (guint)ulValue);
  set_value(iter, col, Glib::ustring(pchValue));
}

void AddDel::set_value(const Gtk::TreeModel::iterator& iter, guint col, bool bVal)
{
  InnerIgnore innerIgnore(this);

  if(!m_refListStore)
    std::cerr << G_STRFUNC << ": No model." << std::endl;
  else
  {
    Gtk::TreeModel::Row treerow = *iter;
    if(treerow)
    {
      //Different model columns have different types of data:
      switch(m_ColumnTypes[col].m_style)
      {
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          treerow.set_value(col, bVal);
          break;
        }
        default:
        {
          Glib::ustring strValue = (bVal ? "true" : "false");
          treerow.set_value(col, strValue);
          break;
        }
      }
    }

    //Add extra blank if necessary:
    add_blank();
  }
}

/*
void AddDel::set_value_selected(guint col, const Gnome::Gda::Value& value)
{
  set_value(get_item_selected(), col, value);
}
*/

void AddDel::remove_all_columns()
{
  m_ColumnTypes.clear();

  //Add the hidden key.ID columns
  //Make these visible (with true) if you want to debug problems.
  m_col_key = add_column("Glom Hidden Key", AddDelColumnInfo::STYLE_Text, false /* not editable */, false /* not visible */);
  m_col_placeholder = add_column("Glom Hidden Placeholder", AddDelColumnInfo::STYLE_Boolean, true /* not editable */, false /* not visible */);
}

guint AddDel::add_column(const AddDelColumnInfo& column_info)
{
  m_ColumnTypes.push_back(column_info);

  //Generate appropriate model columns:
  construct_specified_columns();

  //Tell the View to use the model:
  //m_TreeView.set_model(m_refListStore);

  return m_ColumnTypes.size()-1;
}

guint AddDel::add_column(const Glib::ustring& strTitle, AddDelColumnInfo::enumStyles style, bool editable, bool visible)
{
  //Use the title as the column_id:
  return add_column(strTitle, strTitle, style, editable, visible);
}

guint AddDel::add_column(const Glib::ustring& strTitle, const Glib::ustring& column_id, AddDelColumnInfo::enumStyles style, bool editable, bool visible)
{
  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add a new column.

  AddDelColumnInfo column_info;
  column_info.m_name = strTitle;
  column_info.m_style = style;
  column_info.m_id = column_id;
  column_info.m_editable = editable;
  column_info.m_visible = visible;

  return add_column(column_info);
}

Glib::ustring AddDel::get_column_field(guint column_index) const
{
  Glib::ustring result;
  if(column_index < m_ColumnTypes.size())
    result = m_ColumnTypes[column_index].m_id;

  return result;
}

bool AddDel::get_prevent_user_signals() const
{
  return m_bPreventUserSignals;
}

void AddDel::set_prevent_user_signals(bool bVal)
{
  m_bPreventUserSignals = bVal;
}

void AddDel::set_column_choices(guint col, const type_vec_strings& vecStrings)
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
      for(type_vec_strings::const_iterator iter = vecStrings.begin(); iter != vecStrings.end(); ++iter)
      {
        pCellRenderer->append_list_item(*iter);
      }
    }
    else
    {
      //The column does not exist yet, so we must create it:
      construct_specified_columns();
    }
  }
}

void AddDel::set_allow_add(bool val)
{
  m_allow_add = val;
}

void AddDel::set_allow_delete(bool val)
{
  m_allow_delete= val;
}


void AddDel::set_allow_user_actions(bool bVal)
{
  m_bAllowUserActions = bVal;
}

bool AddDel::get_allow_user_actions() const
{
  return m_bAllowUserActions;
}


void AddDel::set_column_width(guint /* col */, guint /*width*/)
{
//  if( col < (guint)m_Sheet.get_columns_count())
//    m_Sheet.set_column_width(col, width);
}

void AddDel::finish_editing()
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

void AddDel::set_ignore_treeview_signals(bool bVal)
{
  m_bIgnoreSheetSignals = bVal;
}

bool AddDel::get_ignore_treeview_signals() const
{
  return m_bIgnoreSheetSignals;
}

/*
void AddDel::reactivate()
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

void AddDel::remove_item(const Gtk::TreeModel::iterator& iter)
{
  if(iter)
    m_refListStore->erase(iter);
}

void AddDel::remove_item_by_key(const Glib::ustring& strKey)
{
  Gtk::TreeModel::iterator iter = get_row(strKey);
  remove_item(iter);
}

AddDel::InnerIgnore::InnerIgnore(AddDel* pOuter)
: m_pOuter(pOuter),
  m_bPreventUserSignals(false),
  m_bIgnoreSheetSignals(false)
{
  if(m_pOuter)
  {
    m_bPreventUserSignals = m_pOuter->get_prevent_user_signals();
    m_pOuter->set_prevent_user_signals();

    m_bIgnoreSheetSignals = m_pOuter->get_ignore_treeview_signals();
    m_pOuter->set_ignore_treeview_signals();
  }
}


AddDel::InnerIgnore::~InnerIgnore()
{
  //Restore values:
  if(m_pOuter)
  {
    m_pOuter->set_prevent_user_signals(m_bPreventUserSignals);
    m_pOuter->set_ignore_treeview_signals(m_bIgnoreSheetSignals);
  }

  m_pOuter = 0;
}

Glib::ustring AddDel::treeview_get_key(const Gtk::TreeModel::iterator& row)
{
  Glib::ustring value;

  if(m_refListStore)
  {
    Gtk::TreeModel::Row treerow = *row;

    if(treerow)
      treerow.get_value(m_col_key, value);
  }

  return value;
}

void AddDel::on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index)
{
  if(path_string.empty())
    return;

  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    bool value_old = false;
    row.get_value(model_column_index, value_old);

    bool value_new = !value_old;
    //Store the user's new value in the model:
    row.set_value(model_column_index, value_new);

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
      row.set_value(model_column_index, value_old);

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

void AddDel::on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index)
{
  if(path_string.empty())
    return;

  Gtk::TreeModel::Path path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter != get_model()->children().end())
  {
    Gtk::TreeModel::Row row = *iter;

    Glib::ustring strTextOld;
    row.get_value(model_column_index, strTextOld);

    //Store the user's new text in the model:
    row.set_value(model_column_index, new_text);

    if(strTextOld == new_text)
      return; //This is not actually an edit.

    //Is it an add or a change?:
    bool bIsAdd = false;
    bool bIsChange = false;

    if(get_allow_user_actions()) //If add is possible:
    {
      if(get_is_placeholder_row(iter))
      {
        bool bPreventUserSignals = get_prevent_user_signals();
        set_prevent_user_signals(true); //Stops extra signal_user_changed.


        if(row_has_duplicates(iter))
        {
          warn_about_duplicate();
          return;
        }
        else
        {
          //Mark this row as no longer a placeholder, because it has data now. The client code must set an actual key for this in the signal_user_added() or m_signal_user_changed signal handlers.
          set_value_key(iter, "glom_unknown");

          add_item_placeholder(); //Add the next blank for the next user add.
        }
        set_prevent_user_signals(bPreventUserSignals);

        bIsAdd = true; //Signal that a new key was added.
      }
    }

    if(!bIsAdd)
      bIsChange = true;

    //Fire appropriate signal:
    if(bIsAdd)
    {
        //Signal that a new key was added"
        m_signal_user_added.emit(row);
    }
    else if(bIsChange)
    {
      //Existing item changed:
      //Check that it has really changed - get the last value.
      if(new_text != strTextOld)
      {
        bool do_signal = true;

        const Field::glom_field_type field_type = m_ColumnTypes[model_column_index].m_field_type;
        if(field_type != Field::TYPE_INVALID) //If a field type was specified for this column.
        {
          //Make sure that the entered data is suitable for this field type:
          bool success = false;
          Glib::ustring text = get_value(row, model_column_index);
          Gnome::Gda::Value value = Conversions::parse_value(field_type, new_text, success);
          if(!success)
          {
             //Tell the user and offer to revert or try again:
             bool revert = glom_show_dialog_invalid_data(field_type);
             if(revert)
             {
               //Revert the data:
               row.set_value(model_column_index, strTextOld);
             }
             else
             {
               //Reactivate the cell so that the data can be corrected.

                Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
                if(refTreeSelection)
                {
                  refTreeSelection->select(row); //TODO: This does not seem to work.

                 Gtk::TreeModel::Path path = m_refListStore->get_path(row);
                 Gtk::TreeView::Column* pColumn = m_TreeView.get_column(model_column_index); //TODO: This might the the view column index, not the model column index.
                 m_TreeView.set_cursor(path, *pColumn, true /* start_editing */); //This highlights the cell, but does not seem to actually start the editing.
               }
             }

             do_signal = false;
          }
          else
          {
            //Actually show the canonical text representation, so that the user sees how his input was interpreted:
            Glib::ustring text_canonical = Conversions::get_text_for_gda_value(field_type, value);
            row.set_value(model_column_index, text_canonical);
          }
        }

        if(do_signal)
          m_signal_user_changed.emit(row, model_column_index);
      }
    }
  }
}


void AddDel::on_treeview_cell_editing_started(Gtk::CellEditable* /* editable */, const Glib::ustring& path_string, int model_column_index)
{
  Gtk::TreeModel::Path path(path_string);

  if(!m_refListStore)
    return;

  Gtk::TreeModel::iterator iterRow = m_refListStore->get_iter(path);
  if(iterRow)
    signal_user_activated().emit(iterRow, model_column_index);
}

AddDel::type_signal_user_added AddDel::signal_user_added()
{
  return m_signal_user_added;
}

AddDel::type_signal_user_changed AddDel::signal_user_changed()
{
  return m_signal_user_changed;
}

AddDel::type_signal_user_requested_delete AddDel::signal_user_requested_delete()
{
  return m_signal_user_requested_delete;
}

AddDel::type_signal_user_requested_edit AddDel::signal_user_requested_edit()
{
  return m_signal_user_requested_edit;
}

AddDel::type_signal_user_requested_edit AddDel::signal_user_requested_extra()
{
  return m_signal_user_requested_extra;
}

AddDel::type_signal_user_requested_add AddDel::signal_user_requested_add()
{
  return m_signal_user_requested_add;
}

AddDel::type_signal_user_activated AddDel::signal_user_activated()
{
  return m_signal_user_activated;
}

AddDel::type_signal_user_reordered_columns AddDel::signal_user_reordered_columns()
{
  return m_signal_user_reordered_columns;
}

void AddDel::on_treeview_button_press_event(GdkEventButton* event)
{
  on_button_press_event_Popup(event);
}

bool AddDel::on_treeview_column_drop(Gtk::TreeView* /* treeview */, Gtk::TreeViewColumn* /* column */, Gtk::TreeViewColumn* /* prev_column */, Gtk::TreeViewColumn* /* next_column */)
{
  return true;
}

guint AddDel::treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, const Gtk::TreeModelColumnBase& model_column, const Glib::ustring& column_id)
{
  TreeViewColumnGlom* pViewColumn = Gtk::manage( new TreeViewColumnGlom(title, cellrenderer) );
  pViewColumn->set_renderer(cellrenderer, model_column); //render it via the default "text" property.
  guint cols_count = m_TreeView.append_column(*pViewColumn);

  //Allow the column to be reordered by dragging and dropping the column header:
  pViewColumn->set_reorderable();

  //Allow the column to be resized:
  pViewColumn->set_resizable();

  //Set a faily sensible default width:
  pViewColumn->set_fixed_width(100); //This is the only way to set the width, so we need to set it as resizable again immediately afterwards.
  pViewColumn->set_resizable();

  //Save the extra ID, using the title if the column_id is empty:
  pViewColumn->set_column_id( (column_id.empty() ? title : column_id) );

  return cols_count;
}

void AddDel::on_treeview_columns_changed()
{
  if(!get_ignore_treeview_signals())
  {
    //Get the new column order, and save it in m_vecColumnIDs:
    m_vecColumnIDs.clear();

    typedef std::vector<Gtk::TreeViewColumn*> type_vecViewColumns;
    type_vecViewColumns vecViewColumns = m_TreeView.get_columns();

    for(type_vecViewColumns::iterator iter = vecViewColumns.begin(); iter != vecViewColumns.end(); ++iter)
    {
      TreeViewColumnGlom* pViewColumn = dynamic_cast<TreeViewColumnGlom*>(*iter);
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

AddDel::type_vec_strings AddDel::get_columns_order() const
{
  //This list is rebuilt in on_treeview_columns_changed, but maybe we could just build it here.
  return m_vecColumnIDs;
}

void AddDel::set_auto_add(bool value)
{
  m_auto_add = value;
}

Glib::RefPtr<Gtk::TreeModel> AddDel::get_model()
{
  return m_refListStore;
}
Glib::RefPtr<const Gtk::TreeModel> AddDel::get_model() const
{
  return m_refListStore;
}

bool AddDel::get_is_first_row(const Gtk::TreeModel::iterator& iter) const
{
  return iter == get_model()->children().begin();
}

bool AddDel::get_is_last_row(const Gtk::TreeModel::iterator& iter) const
{
  return iter == get_last_row();
}

Gtk::TreeModel::iterator AddDel::get_next_available_row_with_add_if_necessary()
{
  Gtk::TreeModel::iterator result;

  if(!m_refListStore)
    return result;

  bool bPreventUserSignals = get_prevent_user_signals();
  set_prevent_user_signals(true);

  if(get_allow_user_actions()) //The extra blank line is only used if the user may add items:
  {
    Gtk::TreeModel::iterator iter = get_last_row();

    if(iter != get_model()->children().end())
    {
      //Look at the last row:
      if( get_is_placeholder_row(iter))
      {
        result = iter;
      }
      else
      {
        // The last line isn't blank, so we cannot use it. Add another one.
        result = m_refListStore->append();
      }
    }
    else
    {
       // This is the first line.
       result = m_refListStore->append();
    }
  }
  else
  {
     result = m_refListStore->append(); //Add a new blank line. There are no blank lines.
  }

  set_prevent_user_signals(bPreventUserSignals);

  return result;
}

Gtk::TreeModel::iterator AddDel::get_last_row() const
{
  //TODO_performance: Hopefully there is a better way to do this.
  Gtk::TreeModel::iterator iter = get_model()->children().begin();
  guint size = get_model()->children().size();
  if(size > 1)
  {
    for(guint i = 0; i < (size -1); ++i)
    {
      ++iter;
    }
  }

  return iter;
}

Gtk::TreeModel::iterator AddDel::get_last_row()
{
  //TODO_performance: Hopefully there is a better way to do this.
  Gtk::TreeModel::iterator iter = get_model()->children().begin();
  guint size = get_model()->children().size();
  if(size > 1)
  {
    for(guint i = 0; i < (size -1); ++i)
    {
      ++iter;
    }
  }

  return iter;
}

Glib::ustring AddDel::get_value_key(const Gtk::TreeModel::iterator& iter)
{
  return get_value(iter, m_col_key);
}


void AddDel::set_value_key(const Gtk::TreeModel::iterator& iter, const Glib::ustring& strValue)
{
  if(!strValue.empty())
  {
    //This is not a placeholder anymore, if it every was:
    iter->set_value(m_col_placeholder, false);
  }

  iter->set_value(m_col_key, strValue);
}

bool AddDel::get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const
{
  if(!iter)
    return false;

  if(!get_is_last_row(iter))
    return false;

  if(iter == m_refListStore->children().end())
    return false;

  bool val = false;
  iter->get_value(m_col_placeholder, val);

  return val;
}

bool AddDel::get_model_column_index(guint view_column_index, guint& model_column_index)
{
  //Initialize output parameter:
  model_column_index = 0;

  const guint hidden = get_count_hidden_system_columns();
  const guint count = m_ColumnTypes.size();
  if(hidden > count)
    return false; //This should never happen.

  if(view_column_index >=  (count - hidden))
    return false;

  model_column_index = view_column_index + hidden; //There are 2 hidden columns at the start.

  return true;
}

bool AddDel::get_view_column_index(guint model_column_index, guint& view_column_index)
{
  //Initialize output parameter:
  view_column_index = 0;

  if(model_column_index >=  m_ColumnTypes.size())
    return false;

  if( !(m_ColumnTypes[model_column_index].m_visible) )
    return false;

  view_column_index = model_column_index - get_count_hidden_system_columns(); //There are 2 hidden columns at the start.

  return true;
}

guint AddDel::get_count_hidden_system_columns()
{
 guint hidden_count = 0;
  if(!(m_ColumnTypes[m_col_key].m_visible))
    ++hidden_count;

  if(!(m_ColumnTypes[m_col_placeholder].m_visible))
    ++hidden_count;

  return hidden_count;
}

void AddDel::prevent_duplicates(guint column_number)
{
  if(column_number < m_ColumnTypes.size())
    m_ColumnTypes[column_number].m_prevent_duplicates = true;
}

void AddDel::set_prevent_duplicates_warning(const Glib::ustring& warning_text)
{
  m_prevent_duplicates_warning = warning_text;
}

bool AddDel::row_has_duplicates(const Gtk::TreeModel::iterator& iter) const
{
  const type_ColumnTypes::size_type cols_count = m_ColumnTypes.size();
  for(type_ColumnTypes::size_type col = 0; col < cols_count; ++col)
  {
    if(m_ColumnTypes[col].m_prevent_duplicates)
    {
      Gtk::TreeModel::Row row = *iter;

      //We can't just use Value, because Gnome::Gda::Value has no operator==, because there is no g_value_equal
      //Gnome::Gda::Value value_this_row;
      //TODO: Actually, Gnome::Gda::Value has an operator==
      //iter->get_value(col, value_this_row);

      Glib::ustring value_text;
      bool value_bool = false;
      int value_int = 0;

      if(m_ColumnTypes[col].m_style == AddDelColumnInfo::STYLE_Text)
        row.get_value(col, value_text);
      else if(m_ColumnTypes[col].m_style == AddDelColumnInfo::STYLE_Boolean)
         row.get_value(col, value_bool);
      else if(m_ColumnTypes[col].m_style == AddDelColumnInfo::STYLE_Numerical)
         row.get_value(col, value_int);

      //std::cout << "value_text=" << value_text << std::endl;

      //Look at each other row to see whether the value exists there already:
      for(Gtk::TreeModel::iterator iterCheck = m_refListStore->children().begin(); iterCheck != m_refListStore->children().end(); ++iterCheck)
      {
        if(iterCheck != iter) //Don't compare the row with itself
        {
          Gtk::TreeModel::Row check_row = *iterCheck;
          ////Gnome::Gda::Value has no operator==, because there is no g_value_equal
          //Gnome::Gda::Value value_check_row;
          //TODO: Actually, Gnome::Gda::Value has an operator==
          //iterCheck->get_value(col, value_check_row);
          //
          //if(value_check_row == value_this_row)
          //  return false; //Duplicate found.

          Glib::ustring check_value_text;
          bool check_value_bool = false;
          int check_value_int = 0;

          if(m_ColumnTypes[col].m_style == AddDelColumnInfo::STYLE_Text)
          {
            check_row.get_value(col, check_value_text);

            //std::cout << "  check_value_text=" << value_text << std::endl;

            if(check_value_text == value_text)
              return true;
          }
          else if(m_ColumnTypes[col].m_style == AddDelColumnInfo::STYLE_Boolean)
          {
            check_row.get_value(col, check_value_bool);
            if(check_value_text == value_text)
              return true;
          }
          else if(m_ColumnTypes[col].m_style == AddDelColumnInfo::STYLE_Numerical)
          {
            check_row.get_value(col, check_value_int);
            if(check_value_text == value_text)
              return true;
          }
        }
      }
    }
  }

  return false;
}

} //namespace Glom
