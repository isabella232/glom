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

#include "adddel.h"
#include <algorithm> //For std::find.
//#include <libgnome/gnome-i18n.h>
#include "cellrendererlist.h"
#include "treeviewcolumn_glom.h"
#include <iostream> //For debug output.

#include "eggcolumnchooser/eggcolumnchooserdialog.h"


AddDelColumnInfo::AddDelColumnInfo()
: m_style(STYLE_Text),
  m_editable(true),
  m_visible(true)
{
}

AddDelColumnInfo::AddDelColumnInfo(const AddDelColumnInfo& src)
: m_style(src.m_style),
  m_name(src.m_name),
  m_id(src.m_id),
  m_choices(src.m_choices),
  m_editable(src.m_editable),
  m_visible(src.m_visible)
{
}

AddDelColumnInfo& AddDelColumnInfo::operator=(const AddDelColumnInfo& src)
{
  m_style = src.m_style;;
  m_name = src.m_name;
  m_id = src.m_id;
  m_choices = src.m_choices;
  m_editable = src.m_editable;
  m_visible = src.m_visible;

  return *this;
}

AddDel::AddDel()
: m_bHasRowTitles(false),
  m_pColumnHeaderPopup(0),
  m_allow_column_chooser(false),
  m_auto_add(true),
  m_allow_add(true)
{
  set_prevent_user_signals();
  set_ignore_treeview_signals();

  set_spacing(6);

  m_bAllowUserActions = true;

  m_strSelectText = gettext("Edit");

  //Start with a useful default TreeModel:
  //set_columns_count(1);
  //construct_specified_columns();

  m_ScrolledWindow.add(m_TreeView);
  m_TreeView.show();
  pack_start(m_ScrolledWindow);

  //Make sure that the TreeView doesn't start out only big enough for zero items.
  m_TreeView.set_size_request(-1, 150);

  //Allow the user to change the column order:
  //m_TreeView.set_column_drag_function( sigc::mem_fun(*this, &AddDel::on_treeview_column_drop) );


  m_TreeView.add_events(Gdk::BUTTON_PRESS_MASK); //Allow us to catch button_press_event and button_release_event
  m_TreeView.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &AddDel::on_treeview_button_press_event) );

  m_TreeView.signal_columns_changed().connect( sigc::mem_fun(*this, &AddDel::on_treeview_columns_changed) );
  //add_blank();

  setup_menu();
  signal_button_press_event().connect(sigc::mem_fun(*this, &AddDel::on_button_press_event_Popup));

  set_prevent_user_signals(false);
  set_ignore_treeview_signals(false);

  show_all_children();
}

AddDel::~AddDel()
{
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
      int row = row_number_from_iterator(iter);

      Glib::ustring strValue_Old = get_value(row);

      finish_editing();

      Glib::ustring strValue = get_value(row);

      if(strValue.size())
      {
        bool bNewEntryPos = row == m_refListStore->children().size() - 1;
        if(strValue_Old.size() && bNewEntryPos)
        {
          //This is a new entry:
          signal_user_added()(row);

          bool bRowAdded = true;

          //The rows might be re-ordered:
          guint rowAdded = row;
          Glib::ustring strValue_Added =  get_value(row);
          if(strValue_Added != strValue)
            bRowAdded = get_row_number(strValue, rowAdded);

          if(bRowAdded)
            signal_user_requested_edit()(rowAdded);
        }
        else
        {
          //Value changed:
          signal_user_requested_edit()(row);
        }

      }

    }

  }
}

void AddDel::on_MenuPopup_activate_ChooseColumns()
{
	GtkDialog* pDialog = GTK_DIALOG( egg_column_chooser_dialog_new(m_TreeView.gobj()) );
	gtk_widget_show_all( GTK_WIDGET(pDialog) );
	gtk_dialog_run(pDialog); //It will delete itself. The C API is funny like that.
	pDialog = 0;
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
      int rowStart = row_number_from_iterator(iter);
      signal_user_requested_delete().emit(rowStart, rowStart);
    }
  }
}

void AddDel::setup_menu()
{
  //Clear existing menu items:
  m_MenuPopup.items().clear();

  //Add new menu items:
  Gtk::Menu_Helpers::MenuElem menuItem_Edit(m_strSelectText);
  m_MenuPopup.items().push_back(menuItem_Edit);
  m_MenuPopup.items().back().signal_activate().connect(sigc::mem_fun(*this, &AddDel::on_MenuPopup_activate_Edit));

  if(get_allow_user_actions())
  {
    Gtk::Menu_Helpers::MenuElem menuItem_Delete(gettext("Delete"));
    m_MenuPopup.items().push_back(menuItem_Delete);
    m_MenuPopup.items().back().signal_activate().connect(sigc::mem_fun(*this, &AddDel::on_MenuPopup_activate_Delete));
  }
  
  if(m_allow_column_chooser)
  {
	  Gtk::Menu_Helpers::MenuElem menuitem_ChooseColumns(gettext("Choose columns"));
	  m_MenuPopup.items().push_back(menuitem_ChooseColumns);
	  m_MenuPopup.items().back().signal_activate().connect(sigc::mem_fun(*this, &AddDel::on_MenuPopup_activate_ChooseColumns));
  }

}

bool AddDel::on_button_press_event_Popup(GdkEventButton *event)
{
  GdkModifierType mods;
  gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
  if(mods & GDK_BUTTON3_MASK)
  {
    //Give user choices of actions on this item:
    m_MenuPopup.popup(event->button, event->time);
  }
  else
  {
    if(event->type == GDK_2BUTTON_PRESS)
    {
      //Double-click means edit.
      on_MenuPopup_activate_Edit();
    }
  }

  return true;
}

guint AddDel::add_item()
{
  return add_item("");
}

int AddDel::get_first_column() const
{
  return (m_bHasRowTitles ? 1 : 0);
}

guint AddDel::add_item(const Glib::ustring& strKey)
{
  if(!m_refListStore)
    return 0;

  //The deactivate signals seems to cause the current cell to revert to it's previsous value.
  //InnerIgnore innerIgnore(this); //see comments for InnerIgnore class.

  //Add a new line:
  //The extra blank is not used if the user may not add items:

  if(get_allow_user_actions())
  {
    if(get_blank_is_used())
    {
      add_blank();
    }
  }
  else
  {
      Gtk::TreeModel::iterator storeIter = m_refListStore->append();  //Just add it:
      //m_refListStore->set_value(jstoreIter, 0, "");
  }

  guint uiRow = m_refListStore->children().size() - 1;  //There must be at least 1 row now.
  Gtk::TreeModel::Row treerow = m_refListStore->children()[uiRow];
  if(treerow)
  {
    treerow.set_value(get_first_column(), strKey);
  }

  bool bExtraBlank = false;
  if(strKey.size())
  {
    //There should always be 1 extra blank.
    //Unless this AddDel does not allow the user to add new items:
    if(get_allow_user_actions())
      bExtraBlank = add_blank();
  }

  //select_item(uiRow);
  //Don't use select_item() because we need to return false from on_sheet_deactivate().


  //if(uiRow < m_Sheet.get_rows_count())
  //  m_Sheet.set_active_cell(uiRow, 0);



  return uiRow;
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

Glib::ustring AddDel::get_value(guint row, guint col)
{
  Glib::ustring value;

  if(m_refListStore)
  {
    Gtk::TreeModel::Row treerow = m_refListStore->children()[row];

    if(treerow)
    {
      const guint col_real = get_first_column() + col;
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

  //TODO: This is a hack. We need to deal with NULL explicitly:
  if(value == "NULL")
    value = "";
    
  return value;
}

bool AddDel::get_value_as_bool(guint row, guint col)
{
  Glib::ustring strValue = get_value(row, col);
  return (strValue == "true");
}

Glib::ustring AddDel::get_value_selected(guint col)
{
  Glib::ustring strValue = get_value(get_item_selected(), col);
  return strValue;
}

guint AddDel::get_item_selected()
{
  gint iRow = 0;
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
  if(refTreeSelection)
  {
     Gtk::TreeModel::iterator iter = refTreeSelection->get_selected();
     if(iter)
       return row_number_from_iterator(iter);
  }

  return 0;
}

bool AddDel::get_row_number(const Glib::ustring& strItemText, guint& row)
{
  row = 0;

  guint rowCount = get_count();

  //Check each item and compare text:
  for(guint i = 0; i < rowCount; i++)
  {
    const Glib::ustring& strTemp = get_value(i, get_first_column());
    if(strTemp == strItemText)
    {
      row = i;
      return true; //found:
    }
  }

  return false; //not found.
}

bool AddDel::select_item(guint row, bool start_editing)
{
  select_item(row, get_first_column(), start_editing);
}

bool AddDel::select_item(guint row, guint column, bool start_editing)
{
  if(!m_refListStore)
    return false;

  InnerIgnore innerIgnore(this); //see comments for InnerIgnore class

  bool bResult = false;

  if(row < m_refListStore->children().size())
  {
    Gtk::TreeModel::Row treerow = m_refListStore->children()[row];
    {
      Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
      if(refTreeSelection)
      {
        refTreeSelection->select(treerow);

        Gtk::TreeModel::Path path = m_refListStore->get_path(treerow);
        Gtk::TreeView::Column* pColumn = m_TreeView.get_column(column);

        m_TreeView.set_cursor(path, *pColumn, start_editing);
      }
    }

    bResult = true;
  }

  return bResult;
}

bool AddDel::select_item(const Glib::ustring& strItemText)
{
  guint row = 0;
  bool bTest = get_row_number(strItemText, row);
  if(bTest)
  {
    select_item(row);
  }

  return bTest;
}

guint AddDel::get_count() const
{
  if(!m_refListStore)
    return 0;

  guint iCount = m_refListStore->children().size();

  //Take account of the extra blank for new entries:
  if(iCount > 0) //Gtk::Extra sheet always has at least 1 row.
  {
    if(get_allow_user_actions()) //If it has the extra row.
    {
      //Look at the last row:
      if(!get_blank_is_used())
           iCount--;
    }
  }

  return iCount;
}

bool AddDel::add_blank()
{
  if(!m_refListStore)
    return false;

  bool bPreventUserSignals = get_prevent_user_signals();
  set_prevent_user_signals(true);

  bool bAddNewBlank = false;

  if(get_allow_user_actions()) //The extra blank line is only used if the user may add items:
  {
    guint rowsCount = m_refListStore->children().size();
    if(rowsCount)
    {
        //Look at the last row:
      Glib::ustring strValue = get_value(rowsCount-1, 0);
      if(strValue.size() == 0)
      {
        bAddNewBlank  = false; //One already exists.
      }
      else
      {
        bAddNewBlank = true; // The last line isn't blank. Add one.
      }
    }
    else
    {
      bAddNewBlank = true; //Add first row.
    }
  }

  if(bAddNewBlank)
    m_refListStore->append();; //Add the next blank for the next user add.

  set_prevent_user_signals(bPreventUserSignals);

  return bAddNewBlank;
}

bool AddDel::get_blank_is_used() const
{
  if(!m_refListStore)
   return false;

  guint iCount = m_refListStore->children().size();

  if(iCount == 0)
  {
    return true; //Shouldn't happen. Force creation of new blank.
  }

  Glib::ustring strValue;

  //Look at the last row:
  Gtk::TreeModel::Row row = m_refListStore->children()[iCount-1];
  if(row)
  {
     row.get_value(get_first_column(), strValue);
  }

  return (strValue.size() > 0);
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
  int iColumn = 0;
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
    type_vecModelColumns::value_type& pModelColumn = *iter;

    if(m_ColumnTypes[iColumn].m_visible)
    {
      const Glib::ustring column_name = m_ColumnTypes[iColumn].m_name;
      const Glib::ustring column_id = m_ColumnTypes[iColumn].m_id;

      int cols_count = 0;
      switch(m_ColumnTypes[iColumn].m_style)
      {
        case(AddDelColumnInfo::STYLE_Choices):
        {
          //Use a custom CellRenderer:
          CellRendererList* pCellRenderer = Gtk::manage( new CellRendererList() );

          //Add the choices:
          const type_vecStrings vecStrings = m_ColumnTypes[iColumn].m_choices;
          for(type_vecStrings::const_iterator iter = vecStrings.begin(); iter != vecStrings.end(); ++iter)
          {
            pCellRenderer->append_list_item(*iter);
          }

          // Append the View column.
          // We use a derived Gtk::TreeViewColumn so that we can store extra information in it.
          // This means that we must reimplement the code from the convenience template methods from gtkmm.
          cols_count = treeview_append_column( string_escape_underscores(column_name), *pCellRenderer,  *pModelColumn, column_id);

          break;
        }
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          //Use whatever standard CellRenderer gtkmm thinks is appropriate:

          //Cast to the derived type, because append_column<> is templated, and needs the type at compile-time
          //to use the correct specialization:

          Gtk::TreeModelColumn<bool>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<bool>* >(pModelColumn);
          if(pModelColumnDerived)
            cols_count = treeview_append_column(string_escape_underscores(column_name), *pModelColumnDerived, column_id);

          break;
        }
        default:
        {
          //Use whatever standard CellRenderer gtkmm thinks is appropriate:

          //Cast to the derived type, because append_column<> is templated, and needs the type at compile-time
          //to use the correct specialization:
          Gtk::TreeModelColumn<Glib::ustring>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<Glib::ustring>* >(pModelColumn);
          if(pModelColumnDerived)
            cols_count = treeview_append_column(string_escape_underscores(column_name), *pModelColumnDerived, column_id);

          break;
        }
      }

      const int model_column_index = cols_count - 1;

      if(m_ColumnTypes[iColumn].m_editable)
      {
        Gtk::CellRendererText* pCellRenderer = dynamic_cast<Gtk::CellRendererText*>(m_TreeView.get_column_cell_renderer(model_column_index));
        if(pCellRenderer)
        {
          //Connect a signal handler:
          if(pCellRenderer)
          {
            //Make it editable:
            pCellRenderer->property_editable() = true;

            //Connect to its signal:
            pCellRenderer->signal_edited().connect(
              sigc::bind( sigc::mem_fun(*this, &AddDel::on_treeview_cell_edited), model_column_index ) );
          }
        }
        else
        {
           Gtk::CellRendererToggle* pCellRenderer = dynamic_cast<Gtk::CellRendererToggle*>(m_TreeView.get_column_cell_renderer(model_column_index));
           if(pCellRenderer)
           {
             pCellRenderer->property_activatable() = true;

             //Connect to its signal:
             pCellRenderer->signal_toggled().connect(
               sigc::bind( sigc::mem_fun(*this, &AddDel::on_treeview_cell_edited_bool), model_column_index ) );
           }
        }
      }


      iColumn++;
    }
  }
  
  //Delete the vector's items:
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
     Gtk::TreeModelColumnBase* pModelColumn = *iter;
     switch(m_ColumnTypes[iColumn].m_style)
     {
       //Cast it to the derived type, so we can delete it properly.
       //This is necessary because TreeModelColumnBase's destructor is not virtual.
       case(AddDelColumnInfo::STYLE_Boolean):
       {
         Gtk::TreeModelColumn<bool>* pModelColumnDerived = static_cast< Gtk::TreeModelColumn<bool>* >(pModelColumn);
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

  m_TreeView.columns_autosize();
}


void AddDel::set_item_title(guint row, const Glib::ustring& strValue)
{
  //Set first column text, if it's used for titles:
  if(m_bHasRowTitles)
  {
    InnerIgnore innerIgnore(this);

    if(!m_refListStore)
      g_warning("AddDel::set_item_title: No model.");
    else
    {
      Gtk::TreeModel::Row treerow = m_refListStore->children()[row];
      if(treerow)
        treerow.set_value(0, strValue);

      //Add extra blank if necessary:
      add_blank();
    }
  }
}

void AddDel::set_value(guint row, guint col, const Glib::ustring& strValue)
{
  InnerIgnore innerIgnore(this);

  if(!m_refListStore)
    g_warning("AddDel::set_value: No model.");
  else
  {
    const guint col_real = get_first_column() + col;
    Gtk::TreeModel::Row treerow = m_refListStore->children()[row];
    if(treerow)
    {
      //Different model columns have different types of data:
      switch(m_ColumnTypes[col_real].m_style)
      {
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          bool bValue = (strValue == "true");
          treerow.set_value(col_real, bValue);
          break;
        }
        default:
        {
          treerow.set_value(col_real, strValue);
          break;
        }
      }
    }

    //Add extra blank if necessary:
    add_blank();
  }
}

void AddDel::set_value(guint row, guint col, unsigned long ulValue)
{
  gchar pchValue[10] = {0};
  sprintf(pchValue, "%d", (guint)ulValue);
  set_value(row, col, Glib::ustring(pchValue));

  /*
  {
  gchar* pchValue = g_strdup_printf("%d", ulValue);
  if(pchValue)
  {
    set_value(row, col, Glib::ustring(pchValue)
    g_free(pchValue);
  }
  }
  */
}

void AddDel::set_value(guint row, guint col, bool bVal)
{
  InnerIgnore innerIgnore(this);

  if(!m_refListStore)
    g_warning("AddDel::set_value: No model.");
  else
  {
    const guint col_real = get_first_column() + col;

    Gtk::TreeModel::Row treerow = m_refListStore->children()[row];
    if(treerow)
    {
      //Different model columns have different types of data:
      switch(m_ColumnTypes[col_real].m_style)
      {
        case(AddDelColumnInfo::STYLE_Boolean):
        {
          treerow.set_value(col_real, bVal);
          break;
        }
        default:
        {
          Glib::ustring strValue = (bVal ? "true" : "false");
          treerow.set_value(col_real, strValue);
          break;
        }
      }
    }

    //Add extra blank if necessary:
    add_blank();
  }
}

void AddDel::remove_all_columns()
{
  m_ColumnTypes.clear();
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

bool AddDel::get_prevent_user_signals() const
{
  return m_bPreventUserSignals;
}

void AddDel::set_prevent_user_signals(bool bVal)
{
  m_bPreventUserSignals = bVal;
}

void AddDel::set_column_choices(guint col, const type_vecStrings& vecStrings)
{
  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add new columns.

  m_ColumnTypes[col].m_choices = vecStrings;

  CellRendererList* pCellRenderer = dynamic_cast<CellRendererList*>( m_TreeView.get_column_cell_renderer(col) );
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
    construct_specified_columns();
  }
}

void AddDel::set_allow_add(bool val)
{
  m_allow_add = val;
}
  
void AddDel::set_allow_user_actions(bool bVal)
{
  m_bAllowUserActions = bVal;
}

bool AddDel::get_allow_user_actions() const
{
  return m_bAllowUserActions;
}

void AddDel::set_select_text(const Glib::ustring& strVal)
{
  m_strSelectText = strVal;
}

Glib::ustring AddDel::get_select_text() const
{
  return m_strSelectText;
}

void AddDel::set_show_row_titles(bool bVal)
{
  m_bHasRowTitles = bVal;

  //TODO_port
  //add_column("", STYLE_Text, false /* not editable */);
}

void AddDel::set_show_column_titles(bool bVal)
{
  m_TreeView.set_headers_visible(bVal);
}


void AddDel::set_column_width(guint col, guint width)
{
//  if( col < (guint)m_Sheet.get_columns_count())
//    m_Sheet.set_column_width(col, width);
}

void AddDel::finish_editing()
{
//  bool bIgnoreSheetSignals = get_ignore_treeview_signals(); //The deactivate signals seems to cause the current cell to revert to it's previsous value.
//  set_ignore_treeview_signals();
//
//  gint row = 0;
//  gint col = 0;
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
//  gint row = 0;
//  gint col = 0;
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
void AddDel::remove_item(guint row)
{
  if(row < get_count())
  {
    Gtk::TreeModel::Row treerow = m_refListStore->children()[row];
    if(treerow)
      m_refListStore->erase(treerow);
  }
}

AddDel::InnerIgnore::InnerIgnore(AddDel* pOuter)
{
  m_pOuter = pOuter;

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

  m_pOuter = false;
}

Glib::ustring AddDel::treeview_get_key(guint row)
{
  Glib::ustring value;

  if(m_refListStore)
  {
    Gtk::TreeModel::Row treerow = m_refListStore->children()[row];

    if(treerow)
      treerow.get_value(get_first_column(), value);
  }

  return value;
}

void AddDel::on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index)
{
  Gtk::TreePath path(path_string);

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

    int row_number = row_number_from_iterator(iter);

    
    //Is this an add or a change?:

    bool bIsAdd = false;
    bool bIsChange = false;

    guint iCount = m_refListStore->children().size();
    if(iCount)
    {
      if(get_allow_user_actions()) //If add is possible:
      {
        if( (model_column_index == get_first_column() ) && (row_number == (iCount - 1)) ) //If it's the last row:
        {
          //We will ignore editing of bool values in the blank row. It seems like a bad way to start a new record.
          //New item in the blank row:
          /*
          Glib::ustring strValue = get_value(row_number);
          if(strValue.size())
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
      //m_signal_user_added.emit(row_number);
    }
    else if(bIsChange)
    {
      //Existing item changed:
      m_signal_user_changed.emit(row_number, model_column_index);
    }
    
  }
}

void AddDel::on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index)
{
  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    Glib::ustring strTextOld;
    row.get_value(model_column_index, strTextOld);

    //Store the user's new text in the model:
    row.set_value(model_column_index, new_text);

    int row_number = row_number_from_iterator(iter);

    //Is it an add or a change?:

    bool bIsAdd = false;
    bool bIsChange = false;

    guint iCount = m_refListStore->children().size();
    if(iCount)
    {
      if(get_allow_user_actions()) //If add is possible:
      {
        if( (model_column_index == get_first_column() ) && (row_number == (iCount - 1)) ) //If it's the last row:
        {
          //New item in the blank row:
          Glib::ustring strValue = get_value(row_number);
          if(strValue.size())
           {
             bool bPreventUserSignals = get_prevent_user_signals();
             set_prevent_user_signals(true); //Stops extra signal_user_changed.
             add_item(); //Add the next blank for the next user add.
             set_prevent_user_signals(bPreventUserSignals);

             bIsAdd = true; //Signal that a new key was added.
          }
        }
      }

      if(!bIsAdd)
        bIsChange = true;
    }

    //Fire appropriate signal:
    if(bIsAdd)
    {
        //Signal that a new key was added:
        m_signal_user_added.emit(row_number);
    }
    else if(bIsChange)
    {
      //Existing item changed:
      //Check that it has really changed - get the last value.
      if(new_text != strTextOld)
      {
          m_signal_user_changed.emit(row_number, model_column_index);
          //new_text = "";
      }
    }
  }
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

int AddDel::row_number_from_iterator(const Gtk::TreeModel::iterator iter)
{
  //This is a bit hacky. We should probably just use iterator in the interface.
  int result;

  Gtk::TreeModel::Path path(iter);
  typedef std::vector<int> type_vecInts;
  type_vecInts vecIndices = path.get_indices();
  if(!vecIndices.empty())
    result = vecIndices[0];

  return result;
}

Glib::ustring AddDel::string_escape_underscores(const Glib::ustring& text)
{
  Glib::ustring result;
  for(Glib::ustring::const_iterator iter = text.begin(); iter != text.end(); ++iter)
  {
    if(*iter == '_')
      result += "__";
    else
      result += *iter;
  }
 
  return result;
}

void AddDel::on_treeview_button_press_event(GdkEventButton* event)
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
    m_TreeView.get_path_at_pos((int)event->x, (int)event->y, path, pColumn, cell_x, cell_y);
    
    //Get the row:
    Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
    int tree_row = row_number_from_iterator(iter);

    //Get the column:
    int tree_col = 0;
    int col_index = 0;

    typedef std::vector<Gtk::TreeView::Column*> type_vecTreeViewColumns;
    type_vecTreeViewColumns vecColumns = m_TreeView.get_columns();
    for(type_vecTreeViewColumns::const_iterator iter = vecColumns.begin(); iter != vecColumns.end(); iter++)
    {
      if(*iter == pColumn)
        tree_col = col_index; //Found.
        
      col_index++;
    }
    
    signal_user_activated().emit(tree_row, tree_col); 
  }

  on_button_press_event_Popup(event);
}

bool AddDel::on_treeview_columnheader_button_press_event(GdkEventButton* event)
{
  //If this is a right-click with the mouse:
  if( (event->type == GDK_BUTTON_PRESS) && (event->button == 3) )
  {
    if(m_pColumnHeaderPopup)
    {
      m_pColumnHeaderPopup->popup(event->button, event->time);
      return true; //It has been handled.
    }
    else
    {
      //Default popup:
      //TODO: We might want to disable this sometimes, because it could be useless sometimes.
      
    }
  }

  return false;
}

bool AddDel::on_treeview_column_drop(Gtk::TreeView* /* treeview */, Gtk::TreeViewColumn* /* column */, Gtk::TreeViewColumn* /* prev_column */, Gtk::TreeViewColumn* /* next_column */)
{
  return true;
}

guint AddDel::treeview_append_column(const Glib::ustring title, Gtk::CellRenderer& cellrenderer, const Gtk::TreeModelColumnBase& model_column, const Glib::ustring& column_id)
{
  TreeViewColumnGlom* pViewColumn = Gtk::manage( new TreeViewColumnGlom(title, cellrenderer) );
  pViewColumn->set_renderer(cellrenderer, model_column); //render it via the default "text" property.
  guint cols_count = m_TreeView.append_column(*pViewColumn);

  //Allow the column to be reordered by dragging and dropping the column header:
  pViewColumn->set_reorderable();

  //Allow the column to be resized:
  pViewColumn->set_resizable();

  //Set a faily sensible default width:
  pViewColumn->set_min_width(100);

  //Save the extra ID, using the title if the column_id is empty:
  pViewColumn->set_column_id( (column_id.empty() ? title : column_id) );

  //TODO pViewColumn->signal_button_press_event().connect( sigc::mem_fun(*this, &AddDel::on_treeview_columnheader_button_press_event) );
  
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

AddDel::type_vecStrings AddDel::get_columns_order() const
{
  //This list is rebuilt in on_treeview_columns_changed, but maybe we could just build it here.
  return m_vecColumnIDs;
}

void AddDel::set_column_header_popup(Gtk::Menu& popup)
{
  m_pColumnHeaderPopup = &popup;
}

 void AddDel::set_allow_column_chooser(bool value)
{
  m_allow_column_chooser = value;
}

void AddDel::set_auto_add(bool value)
{
  m_auto_add = value;
}
