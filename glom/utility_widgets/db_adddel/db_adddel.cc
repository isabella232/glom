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
#include <libintl.h>
#include "../adddel/cellrendererlist.h"
#include "db_treeviewcolumn_glom.h"
#include "../../data_structure/glomconversions.h"
#include "../../dialog_invalid_data.h"
#include <iostream> //For debug output.

DbAddDelColumnInfo::DbAddDelColumnInfo()
:
  m_editable(true),
  m_visible(true)
{
}

DbAddDelColumnInfo::DbAddDelColumnInfo(const DbAddDelColumnInfo& src)
: m_field(src.m_field),
  m_choices(src.m_choices),
  m_editable(src.m_editable),
  m_visible(src.m_visible)
{
}

DbAddDelColumnInfo& DbAddDelColumnInfo::operator=(const DbAddDelColumnInfo& src)
{
  m_field = src.m_field;
  m_choices = src.m_choices;
  m_editable = src.m_editable;
  m_visible = src.m_visible;

  return *this;
}

DbAddDel::DbAddDel()
: m_modelcolumn_key(0),
  m_pColumnHeaderPopup(0),
  m_allow_column_chooser(false),
  m_auto_add(true),
  m_allow_add(true),
  m_allow_delete(true)
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
}

DbAddDel::~DbAddDel()
{
  if(m_modelcolumn_key)
    delete m_modelcolumn_key;
  
  if(m_modelcolumn_placeholder)
    delete m_modelcolumn_placeholder;
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
      if(get_is_placeholder_row(iter))
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
      else
      {
        //Value changed:
        signal_user_requested_edit()(iter);
      }
    }

  }
}

void DbAddDel::on_MenuPopup_activate_ChooseColumns()
{
/*
  GtkDialog* pDialog = GTK_DIALOG( egg_column_chooser_dialog_new(m_TreeView.gobj()) );
  gtk_widget_show_all( GTK_WIDGET(pDialog) );
  gtk_dialog_run(pDialog); //It will delete itself. The C API is funny like that.
  pDialog = 0;
*/
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
  //Clear existing menu items:
  m_MenuPopup.items().clear();

  //Add new menu items:
  Gtk::Menu_Helpers::MenuElem menuItem_Edit(m_strSelectText);
  m_MenuPopup.items().push_back(menuItem_Edit);
  m_MenuPopup.items().back().signal_activate().connect(sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Edit));

  if(get_allow_user_actions())
  {
    Gtk::Menu_Helpers::MenuElem menuItem_Delete(gettext("Delete"));
    m_MenuPopup.items().push_back(menuItem_Delete);
    m_MenuPopup.items().back().signal_activate().connect(sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Delete));
  }
  
  if(m_allow_column_chooser)
  {
    Gtk::Menu_Helpers::MenuElem menuitem_ChooseColumns(gettext("Choose columns"));
    m_MenuPopup.items().push_back(menuitem_ChooseColumns);
    m_MenuPopup.items().back().signal_activate().connect(sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_ChooseColumns));
  }

}

bool DbAddDel::on_button_press_event_Popup(GdkEventButton *event)
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

Gtk::TreeModel::iterator DbAddDel::get_item_placeholder()
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
  
Gtk::TreeModel::iterator DbAddDel::add_item_placeholder()
{
  Gtk::TreeModel::iterator iter = m_refListStore->append();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    row[*m_modelcolumn_key] = Gnome::Gda::Value(); //Remove temporary key value.
    row[*m_modelcolumn_placeholder] =  true; //This will be unset when set_value() is used to put real data in this column.
  }

  return iter;
}

Gtk::TreeModel::iterator DbAddDel::add_item(const Gnome::Gda::Value& valKey)
{
  if(!(get_model()))
    return Gtk::TreeModel::iterator();
    
  Gtk::TreeModel::iterator result = get_next_available_row_with_add_if_necessary();

  if(result)
  {
    Gtk::TreeModel::Row treerow = *result;
    if(treerow)
    {
      treerow[*m_modelcolumn_key] = valKey;
      treerow[*m_modelcolumn_placeholder] =  false; 
    }
  }

  add_blank(); //if necessary

  return result;
}

void DbAddDel::remove_all()
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


Gnome::Gda::Value DbAddDel::get_value_as_value(const Gtk::TreeModel::iterator& iter, guint col)
{
  Gnome::Gda::Value value;

  if(m_refListStore)
  {
    Gtk::TreeModel::Row treerow = *iter;

    if(treerow)
    {
      const guint col_real = col + get_count_hidden_system_columns();
      treerow.get_value(col_real, value);
    }
  }

  return value;
}

Gnome::Gda::Value DbAddDel::get_value_key_selected_as_value()
{
  Gtk::TreeModel::iterator iter = get_item_selected();
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
    return row[*m_modelcolumn_key];
  }
  else
    return Gnome::Gda::Value();
}

Gnome::Gda::Value DbAddDel::get_value_selected_as_value(guint col)
{
  return get_value_as_value(get_item_selected(), col);
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
  for(Gtk::TreeModel::iterator iter = m_refListStore->children().begin(); iter != m_refListStore->children().end(); ++iter)
  {
    Gtk::TreeModel::Row row = *iter;
    const Gnome::Gda::Value& valTemp = row[*m_modelcolumn_key];
    if(valTemp == key)
    {
      return iter;
    }
  }

  return  m_refListStore->children().end();
}

bool DbAddDel::select_item(const Gtk::TreeModel::iterator& iter)
{
  guint col_first = 0;
  get_model_column_index(0, col_first);
  return select_item(iter, col_first);
}

bool DbAddDel::select_item(const Gtk::TreeModel::iterator& iter, guint column, bool start_editing)
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
           g_warning("DbAddDel::select_item:TreeViewColumn not found.");
      }
      else
           g_warning("DbAddDel::select_item:TreeViewColumn index not found. column=%d", column);
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

void DbAddDel::add_blank()
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

void DbAddDel::construct_specified_columns()
{
  //TODO_optimisation: This is called many times, just to simplify the API.

  //Delay actual use of set_column_*() stuff until this method is called.

  if(m_ColumnTypes.empty())
    return;

  typedef Gtk::TreeModelColumn<Gnome::Gda::Value> type_modelcolumn_value;
  typedef std::vector< type_modelcolumn_value* > type_vecModelColumns;
  type_vecModelColumns vecModelColumns(m_ColumnTypes.size(), 0);

  //Create the Gtk ColumnRecord:

  Gtk::TreeModel::ColumnRecord record;
  
  //Hidden internal columns:
  
  //We must recreate these standard modelcolumns, because we can not reuse them
  if(m_modelcolumn_key)
    delete m_modelcolumn_key; 
  m_modelcolumn_key = new Gtk::TreeModelColumn<Gnome::Gda::Value>;
  
  if(m_modelcolumn_placeholder)
    delete m_modelcolumn_placeholder;
  m_modelcolumn_placeholder = new Gtk::TreeModelColumn<bool>;
    
  record.add(*m_modelcolumn_key);
  record.add(*m_modelcolumn_placeholder);
    
  //Database columns:
  {       
    type_vecModelColumns::size_type i = 0;
    for(type_ColumnTypes::iterator iter = m_ColumnTypes.begin(); iter != m_ColumnTypes.end(); ++iter)
    {
      type_modelcolumn_value* pModelColumn = new type_modelcolumn_value;

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
  int model_column_index = 0; //Not including the hidden internal columns.
  int view_column_index = 0;
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
    DbAddDelColumnInfo& column_info = m_ColumnTypes[model_column_index];
    if(column_info.m_visible)
    {
      const Glib::ustring column_name = column_info.m_field.get_title_or_name();
      const Glib::ustring column_id = column_info.m_field.get_name();

      int cols_count = 0;
      Gtk::CellRenderer* pCellRenderer = 0;
      switch(column_info.m_field.get_glom_type())
      {
  /*
        case(DbAddDelColumnInfo::STYLE_Choices):
        {
          //Use a custom CellRenderer:
          CellRendererList* pCellRendererDerived = Gtk::manage( new CellRendererList() );

          //Add the choices:
          const type_vecStrings vecStrings = column_info.m_choices;
          for(type_vecStrings::const_iterator iter = vecStrings.begin(); iter != vecStrings.end(); ++iter)
          {
            pCellRendererDerived->append_list_item(*iter);
          }
          
          pCellRenderer = pCellRendererDerived;

          break;
        }
        
   */
   
    /* Possible types:
    TYPE_INVALID,
    TYPE_NUMERIC,
    TYPE_TEXT,
    TYPE_DATE,
    TYPE_TIME,
    TYPE_BOOLEAN
    */
  
        case(Field::TYPE_BOOLEAN):
        {
          pCellRenderer = Gtk::manage( new Gtk::CellRendererToggle() );

          break;
        }
        default:
        {
          pCellRenderer = Gtk::manage( new Gtk::CellRendererText() );
          
          break;
        }
      } //switch
      
      ++cols_count;
      
      //Add the ViewColumn
      treeview_append_column(column_name, *pCellRenderer, model_column_index);
     
      if(column_info.m_editable)
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
              sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited), model_column_index) );
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
               sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited_bool), model_column_index ) );
           }
        }
      }

    ++view_column_index;
    
    } //is visible

    ++model_column_index;
  }
  
  //Delete the vector's items:
  for(type_vecModelColumns::iterator iter = vecModelColumns.begin(); iter != vecModelColumns.end(); ++iter)
  {
     type_modelcolumn_value* pModelColumn = *iter;
     delete pModelColumn;
  }

  m_TreeView.columns_autosize();
}

void DbAddDel::set_value(const Gtk::TreeModel::iterator& iter, guint col, const Gnome::Gda::Value& value)
{
  InnerIgnore innerIgnore(this);

  if(!m_refListStore)
    g_warning("DbAddDel::set_value: No model.");
  else
  {
    Gtk::TreeModel::Row treerow = *iter;
    if(treerow)
    {
      guint treemodel_col = col + get_count_hidden_system_columns();
      treerow.set_value(treemodel_col, value);

      //Mark this row as not a placeholder because it has real data now.
      if(!(GlomConversions::value_is_empty(value)))
      {
         //treerow.set_value(m_col_key, Glib::ustring("placeholder debug value setted"));
         //treerow.set_value(m_col_placeholder, false);
      }
    }

    //Add extra blank if necessary:
    add_blank();
  }
}

void DbAddDel::set_value_selected(guint col, const Gnome::Gda::Value& value)
{
  set_value(get_item_selected(), col, value);
}

void DbAddDel::remove_all_columns()
{
  m_ColumnTypes.clear();
}

guint DbAddDel::add_column(const Field& field, bool editable, bool visible)
{
  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add a new column.

  DbAddDelColumnInfo column_info;
  column_info.m_field = field;
  column_info.m_editable = editable;
  column_info.m_visible = visible;

  //Make it non-editable if it is auto-generated:
  if(field.get_field_info().get_auto_increment())
    column_info.m_editable = false;

  m_ColumnTypes.push_back(column_info);

  //Generate appropriate model columns:
  construct_specified_columns();

  //Tell the View to use the model:
  //m_TreeView.set_model(m_refListStore);

  return m_ColumnTypes.size() - 1;
  
}

Glib::ustring DbAddDel::get_column_field(guint column_index) const
{
  Glib::ustring result;
  if(column_index < m_ColumnTypes.size())
    result = m_ColumnTypes[column_index].m_field.get_name();

  return result;
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
      construct_specified_columns();
    }
  }
}

void DbAddDel::set_allow_add(bool val)
{
  m_allow_add = val;
}

void DbAddDel::set_allow_delete(bool val)
{
  m_allow_delete= val;
}
  
  
void DbAddDel::set_allow_user_actions(bool bVal)
{
  m_bAllowUserActions = bVal;
}

bool DbAddDel::get_allow_user_actions() const
{
  return m_bAllowUserActions;
}

void DbAddDel::set_select_text(const Glib::ustring& strVal)
{
  m_strSelectText = strVal;
}

Glib::ustring DbAddDel::get_select_text() const
{
  return m_strSelectText;
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

void DbAddDel::set_ignore_treeview_signals(bool bVal)
{
  m_bIgnoreSheetSignals = bVal;
}

bool DbAddDel::get_ignore_treeview_signals() const
{
  return m_bIgnoreSheetSignals;
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

DbAddDel::InnerIgnore::InnerIgnore(DbAddDel* pOuter)
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


DbAddDel::InnerIgnore::~InnerIgnore()
{
  //Restore values:
  if(m_pOuter)
  {
    m_pOuter->set_prevent_user_signals(m_bPreventUserSignals);
    m_pOuter->set_ignore_treeview_signals(m_bIgnoreSheetSignals);
  }

  m_pOuter = false;
}

Gnome::Gda::Value DbAddDel::treeview_get_key(const Gtk::TreeModel::iterator& row)
{
  Gnome::Gda::Value value;

  if(m_refListStore)
  {
    Gtk::TreeModel::Row treerow = *row;

    if(treerow)
    {
      return treerow[*m_modelcolumn_key];
    }
  }

  return value;
}

void DbAddDel::on_treeview_cell_edited_bool(const Glib::ustring& path_string, int model_column_index)
{
  //Note:: model_column_index is actually the AddDel column index, not the TreeModel column index.
  
  if(path_string.empty())
    return;
    
  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    int tree_model_column_index = model_column_index + get_count_hidden_system_columns();
    
    Gnome::Gda::Value value_old;
    row.get_value(tree_model_column_index, value_old);

    bool bValueNew = !(value_old.get_bool());
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

void DbAddDel::on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index)
{
  //Note:: model_column_index is actually the AddDel column index, not the TreeModel column index.
  
  if(path_string.empty())
    return;
  
  Gtk::TreePath path(path_string);

  //Get the row from the path:
  Gtk::TreeModel::iterator iter = m_refListStore->get_iter(path);
  if(iter != get_model()->children().end())
  {
    Gtk::TreeModel::Row row = *iter;

    const int treemodel_column_index = model_column_index + get_count_hidden_system_columns();
    
    Gnome::Gda::Value valOld;
    row.get_value(treemodel_column_index, valOld);
    
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
        set_value_key(iter, Gnome::Gda::Value("glom_unknown"));
        
        add_item_placeholder(); //Add the next blank for the next user add.
        set_prevent_user_signals(bPreventUserSignals);

        bIsAdd = true; //Signal that a new key was added.
      }
    }

    const Field::glom_field_type field_type = m_ColumnTypes[model_column_index].m_field.get_glom_type();
    if(field_type != Field::TYPE_INVALID) //If a field type was specified for this column.
    {
      //Make sure that the entered data is suitable for this field type:
      bool success = false;
      Gnome::Gda::Value value = GlomConversions::parse_value(field_type, new_text, success);
      if(!success)
      {
          //Tell the user and offer to revert or try again:
          bool revert = glom_show_dialog_invalid_date(field_type);
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
        row.set_value(treemodel_column_index, value);
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

Glib::ustring DbAddDel::string_escape_underscores(const Glib::ustring& text)
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
    m_TreeView.get_path_at_pos((int)event->x, (int)event->y, path, pColumn, cell_x, cell_y);
    
    //Get the row:
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

  on_button_press_event_Popup(event);
}

bool DbAddDel::on_treeview_columnheader_button_press_event(GdkEventButton* event)
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

bool DbAddDel::on_treeview_column_drop(Gtk::TreeView* /* treeview */, Gtk::TreeViewColumn* /* column */, Gtk::TreeViewColumn* /* prev_column */, Gtk::TreeViewColumn* /* next_column */)
{
  return true;
}

guint DbAddDel::treeview_append_column(const Glib::ustring& title, Gtk::CellRenderer& cellrenderer, int model_column_index)
{
  DbTreeViewColumnGlom* pViewColumn = Gtk::manage( new DbTreeViewColumnGlom(title, cellrenderer) );
  guint cols_count = m_TreeView.append_column(*pViewColumn);

  //Tell the TreeView how to render the Gnome::Gda::Values:
  pViewColumn->set_cell_data_func(cellrenderer, 
    sigc::bind( sigc::mem_fun(*this, &DbAddDel::treeviewcolumn_on_cell_data), model_column_index) );

  //Allow the column to be reordered by dragging and dropping the column header:
  pViewColumn->set_reorderable();

  //Allow the column to be resized:
  pViewColumn->set_resizable();

  //Set a faily sensible default width:
  pViewColumn->set_min_width(100);

  //Save the extra ID, using the title if the column_id is empty:
  Glib::ustring column_id = m_ColumnTypes[model_column_index].m_field.get_name();
  pViewColumn->set_column_id( (column_id.empty() ? title : column_id) );

  //TODO pViewColumn->signal_button_press_event().connect( sigc::mem_fun(*this, &DbAddDel::on_treeview_columnheader_button_press_event) );
  
  return cols_count;
}

void DbAddDel::on_treeview_columns_changed()
{
  if(!get_ignore_treeview_signals())
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

void DbAddDel::set_column_header_popup(Gtk::Menu& popup)
{
  m_pColumnHeaderPopup = &popup;
}

 void DbAddDel::set_allow_column_chooser(bool value)
{
  m_allow_column_chooser = value;
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
  return iter == get_model()->children().begin();
}

bool DbAddDel::get_is_last_row(const Gtk::TreeModel::iterator& iter) const
{
  return iter == get_last_row();
}

Gtk::TreeModel::iterator DbAddDel::get_next_available_row_with_add_if_necessary()
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
        // The last line isn't blank, so we can not use it. Add another one.
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

Gtk::TreeModel::iterator DbAddDel::get_last_row() const
{
  Glib::RefPtr<const Gtk::TreeModel> refModel = get_model();
  if(!refModel)
    return Gtk::TreeModel::iterator();
  else
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
}

Gtk::TreeModel::iterator DbAddDel::get_last_row()
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

Gnome::Gda::Value DbAddDel::get_value_key_as_value(const Gtk::TreeModel::iterator& iter)
{
  return treeview_get_key(iter);
}


void DbAddDel::set_value_key(const Gtk::TreeModel::iterator& iter, const Gnome::Gda::Value& value)
{
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;
  
    if(!(GlomConversions::value_is_empty(value)))
    {
      //This is not a placeholder anymore, if it every was:
      row[*m_modelcolumn_placeholder] = false;
    }
  
    row[*m_modelcolumn_key] = value;
  }
}

bool DbAddDel::get_is_placeholder_row(const Gtk::TreeModel::iterator& iter) const
{
  if(!get_is_last_row(iter))
    return false;

  if(iter == m_refListStore->children().end())
    return false;

  Gtk::TreeModel::Row row = *iter;
  return row[*m_modelcolumn_placeholder];
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

  return true;
}

guint DbAddDel::get_count_hidden_system_columns()
{
  return 2; //The key and the placeholder boolean.
}

void DbAddDel::set_rules_hint(bool val)
{
  m_TreeView.set_rules_hint(val);
}

void DbAddDel::set_key_type(const Field& field)
{
  m_key_field = field;
}

void DbAddDel::treeviewcolumn_on_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, int model_column_index)
{
  Gnome::Gda::Value value = get_value_as_value(iter, model_column_index);
  
  DbAddDelColumnInfo& column_info = m_ColumnTypes[model_column_index];
  switch(column_info.m_field.get_glom_type())
  {
    case(Field::TYPE_BOOLEAN):
    {
      Gtk::CellRendererToggle* pDerived = dynamic_cast<Gtk::CellRendererToggle*>(renderer);
      pDerived->set_active( value.get_bool() ); 
  
      break;
    }
    default:
    {
      //TODO: Maybe we should have custom cellrenderers for time, date, and numbers.
      Gtk::CellRendererText* pDerived = dynamic_cast<Gtk::CellRendererText*>(renderer);
      pDerived->property_text() = GlomConversions::get_text_for_gda_value(column_info.m_field.get_glom_type(), value);
  
      break;
    } 
  }
}
  
      

