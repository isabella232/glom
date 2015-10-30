/* Glom
 *
 * Copyright (C) 2001-2010 Murray Cumming
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

#include "db_adddel.h"
#include <algorithm> //For std::find.
#include <glibmm/i18n.h>
#include <glom/utility_widgets/cellrendererlist.h>
#include <glom/mode_data/datawidget/cellrenderer_buttonimage.h>
#include <glom/mode_data/datawidget/cellrenderer_buttontext.h>
#include <glom/mode_data/datawidget/cellrenderer_dblist.h>
#include "db_treeviewcolumn_glom.h"
#include <libglom/data_structure/glomconversions.h>
#include <glom/dialog_invalid_data.h>
#include <glom/appwindow.h>
#include <glom/utils_ui.h> //For UiUtils::image_scale_keeping_ratio().
#include <glom/mode_data/datawidget/cellcreation.h>
#include <glibmm/main.h>
#include <giomm/menu.h>
#include <libglom/db_utils.h>

#include <iostream> //For debug output.


namespace Glom
{

DbAddDel::DbAddDel()
: Gtk::Box(Gtk::ORIENTATION_VERTICAL),
  m_column_is_sorted(false),
  m_column_sorted_direction(false),
  m_column_sorted(0),
  m_pMenuPopup(nullptr),
  m_bAllowUserActions(true),
  m_bPreventUserSignals(false),
  m_bIgnoreTreeViewSignals(false),
  m_allow_add(true),
  m_allow_delete(true),
  m_find_mode(false),
  m_allow_only_one_related_record(false),
  m_validation_retry(false),
  m_allow_view(true),
  m_allow_view_details(false),
  m_treeviewcolumn_button(nullptr),
  m_fixed_cell_height(0),
  m_rows_count_min(0),
  m_rows_count_max(0)
{
  set_prevent_user_signals();
  set_ignore_treeview_signals(true);

  set_spacing(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));

  //Start with a useful default TreeModel:
  //set_columns_count(1);
  //construct_specified_columns();

  // Give the TreeView an accessible name, to access it in LDTP
  // TODO: Maybe this should be a constructor parameter, so that multiple
  // DbAddDels in a single Window can be addressed separately.
#ifdef GTKMM_ATKMM_ENABLED
  m_TreeView.get_accessible()->set_name(_("Table Content"));
#endif

  m_ScrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  m_ScrolledWindow.set_shadow_type(Gtk::SHADOW_IN);
  m_TreeView.set_fixed_height_mode(); //This allows some optimizations.
  m_ScrolledWindow.add(m_TreeView);
  pack_start(m_ScrolledWindow);

  m_TreeView.show();

  //Make sure that the TreeView doesn't start out only big enough for zero items.
  set_height_rows(6, 6);

  m_TreeView.add_events(Gdk::BUTTON_PRESS_MASK); //Allow us to catch button_press_event and button_release_event
  m_TreeView.signal_button_press_event().connect_notify( sigc::mem_fun(*this, &DbAddDel::on_treeview_button_press_event) );
  m_TreeView.signal_columns_changed().connect( sigc::mem_fun(*this, &DbAddDel::on_treeview_columns_changed) );
  //signal_button_press_event().connect(sigc::mem_fun(*this, &DbAddDel::on_button_press_event_Popup));
  //add_blank();

  setup_menu(this);


  set_prevent_user_signals(false);
  set_ignore_treeview_signals(false);

  remove_all_columns(); //set up the default columns.

  show_all_children();

#ifdef GLOM_ENABLE_CLIENT_ONLY //Actually this has only been necessary for Maemo.
  // Adjust sizing when style changed

  signal_style_changed().connect(sigc::mem_fun(*this, &DbAddDel::on_self_style_changed));
#endif // !GLOM_ENABLE_CLIENT_ONLY

  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_TreeView.get_selection();
  if(refSelection)
  {
    refSelection->signal_changed().connect(
      sigc::mem_fun(*this, &DbAddDel::on_treeview_selection_changed));
  }
}

DbAddDel::~DbAddDel()
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  auto pApp = get_appwindow();
  if(pApp)
  {
    pApp->remove_developer_action(m_refContextLayout);
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

void DbAddDel::set_height_rows(gulong rows_count_min, gulong rows_count_max)
{
  m_rows_count_min = rows_count_min;
  m_rows_count_max = rows_count_max;

  set_height_rows_actual(m_rows_count_min);
}

void DbAddDel::set_height_rows_actual(gulong rows_count)
{
  //TODO: File a bug about API for this in GtkTreeView.
  const guint height_for_rows = rows_count * get_fixed_cell_height();
  //std::cout << "debug: height_for_rows = " << height_for_rows << std::endl;
  const guint extra_for_treeview = 50; //TODO: Find some way to guess this.
  m_ScrolledWindow.set_min_content_height(height_for_rows + extra_for_treeview);
}

void DbAddDel::do_user_requested_edit()
{
  auto iter = get_item_selected();
  if(iter)
  {
    signal_user_requested_edit()(iter);
  }
  else
    std::cerr << G_STRFUNC << ": No item was selected." << std::endl;
}

void DbAddDel::on_idle_row_edit()
{
  on_MenuPopup_activate_Edit();
}

void DbAddDel::on_cell_button_clicked(const Gtk::TreeModel::Path& path)
{
  if(!m_refListStore)
    return;

  auto iter = m_refListStore->get_iter(path);
  if(iter)
  {
    select_item(iter, false /* start_editing */);
  }

  //This delayed action avoids a warning about a NULL GtkAdjustment.
  //It's fairly understandable that GtkTreeView doesn't like to be destroyed 
  //as a side-effect of a click on one of its GtkCellRenderers.
  //That's unlikely to be fixed properly until GtkTreeView supports a real 
  //button cell-renderer.
  Glib::signal_idle().connect_once(
    sigc::mem_fun(*this, &DbAddDel::on_idle_row_edit));
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
    auto iter = refSelection->get_selected();
    if(iter && !get_is_placeholder_row(iter))
    {
      //TODO: We can't handle multiple-selections yet.
      user_requested_delete(iter, iter);
    }
  }
}

void DbAddDel::on_MenuPopup_activate_layout()
{
  finish_editing();
  signal_user_requested_layout().emit();
}

void DbAddDel::setup_menu(Gtk::Widget* /* widget */)
{
  m_refActionGroup = Gio::SimpleActionGroup::create();

  const Glib::ustring edit_title =
    (m_open_button_title.empty() ? _("_Edit") : m_open_button_title); //TODO: Use this?

  m_refContextEdit = m_refActionGroup->add_action("edit",
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Edit) );

  m_refContextDelete = m_refActionGroup->add_action("delete",
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Delete) );

  m_refContextAdd = m_refActionGroup->add_action("add",
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_Add) );
  m_refContextAdd->set_enabled(m_allow_add);

#ifndef GLOM_ENABLE_CLIENT_ONLY
  // Don't add ContextLayout in client only mode because it would never
  // be sensitive anyway
  m_refContextLayout =  m_refActionGroup->add_action("layout",
    sigc::mem_fun(*this, &DbAddDel::on_MenuPopup_activate_layout) );

  //TODO: This does not work until this widget is in a container in the window:
  auto pApp = get_appwindow();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->update_userlevel_ui(); //Update our action's sensitivity.
  }
#endif // !GLOM_ENABLE_CLIENT_ONLY

  insert_action_group("context", m_refActionGroup);

  //TODO: add_accel_group(builder->get_accel_group());

  Glib::RefPtr<Gio::Menu> menu = Gio::Menu::create();
  menu->append(_("_Edit"), "context.edit");
  menu->append(_("_Add"), "context.add");
  menu->append(_("_Delete"), "context.delete");
#ifndef GLOM_ENABLE_CLIENT_ONLY
  menu->append(_("_Layout"), "context.layout");
#endif

  m_pMenuPopup = new Gtk::Menu(menu);
  m_pMenuPopup->attach_to_widget(*this);

  if(get_allow_user_actions())
  {
    m_refContextEdit->set_enabled();
    m_refContextDelete->set_enabled();
  }
  else
  {
    m_refContextEdit->set_enabled(false);
    m_refContextDelete->set_enabled(false);
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  if(pApp)
    m_refContextLayout->set_enabled(pApp->get_userlevel() == AppState::userlevels::DEVELOPER);
#endif // !GLOM_ENABLE_CLIENT_ONLY
}

bool DbAddDel::on_button_press_event_Popup(GdkEventButton *button_event)
{
#ifndef GLOM_ENABLE_CLIENT_ONLY
  //Enable/Disable items.
  //We did this earlier, but get_appwindow is more likely to work now:
  auto pApp = get_appwindow();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->update_userlevel_ui(); //Update our action's sensitivity.
  }
#endif

  GdkModifierType mods;
  gdk_window_get_device_position( gtk_widget_get_window(Gtk::Widget::gobj()), button_event->device, 0, 0, &mods );
  if(mods & GDK_BUTTON3_MASK)
  {
    //Give user choices of actions on this item:
    m_pMenuPopup->popup(button_event->button, button_event->time);
    return true; //handled.
  }
  else
  {
    if(button_event->type == GDK_2BUTTON_PRESS)
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
  if(m_refListStore)
    return m_refListStore->get_placeholder_row();
  else
   return Gtk::TreeModel::iterator();
}

Gnome::Gda::Value DbAddDel::get_value(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem_Field>& layout_item) const
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
        type_list_indexes::const_iterator iter_begin = list_indexes.begin(); //Just get the first displayed instance of this field->

        const guint col_real = *iter_begin + get_count_hidden_system_columns();
        treerow.get_value(col_real, value);
      }
    }
  }

  return value;
}

Gnome::Gda::Value DbAddDel::get_value_key_selected() const
{
  auto iter = get_item_selected();
  if(iter)
  {
    return get_value_key(iter);
  }
  else
    return Gnome::Gda::Value();
}

Gnome::Gda::Value DbAddDel::get_value_selected(const std::shared_ptr<const LayoutItem_Field>& layout_item) const
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

  if(m_refListStore)
    return m_refListStore->children().end();
  else
    return Gtk::TreeModel::iterator();
}

Gtk::TreeModel::iterator DbAddDel::get_item_selected() const
{
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
}


Gtk::TreeModel::iterator DbAddDel::get_row(const Gnome::Gda::Value& key)
{
  if(!m_refListStore)
    return Gtk::TreeModel::iterator();

  for(auto iter = m_refListStore->children().begin(); iter != m_refListStore->children().end(); ++iter)
  {
    //Gtk::TreeModel::Row row = *iter;
    const auto valTemp = get_value_key(iter);
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
  std::shared_ptr<const LayoutItem> layout_item;
  for(const auto& item : m_column_items)
  {
    layout_item = item;
    if(layout_item)
      break;
  }

  return select_item(iter, layout_item, start_editing);
}

bool DbAddDel::select_item(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem>& layout_item, bool start_editing)
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
    g_assert(refTreeSelection);
    refTreeSelection->select(iter);

    Gtk::TreeModel::Path path = m_refListStore->get_path(iter);

    guint view_column_index = 0;
    const auto test = get_view_column_index(treemodel_col, view_column_index);
    if(test)
    {
      auto pColumn = m_TreeView.get_column(view_column_index);
      if(pColumn)
      {
        if(pColumn != m_treeviewcolumn_button) //This would activate the button. Let's avoid this, though it should never happen.
        {
          m_TreeView.set_cursor(path, *pColumn, start_editing);
        }
      }
      else
       g_warning("DbAddDel::select_item:TreeViewColumn not found.");
    }
    else
       g_warning("DbAddDel::select_item:TreeViewColumn index not found. column=%d", treemodel_col);

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
    --iCount;
  }

  return iCount;
}

guint DbAddDel::get_columns_count() const
{
  return m_TreeView.get_columns().size();
}

guint DbAddDel::get_fixed_cell_height()
{
  if(m_fixed_cell_height <= 0)
  {
    // Discover a suitable height, and cache it,
    // by looking at the heights of all columns:
    // Note that this is usually calculated during construct_specified_columns(),
    // when all columns are known.

    //Get a default:
    Glib::RefPtr<Pango::Layout> refLayoutDefault = m_TreeView.create_pango_layout("ExampleEg");
    int width_default = 0;
    int height_default = 0;
    refLayoutDefault->get_pixel_size(width_default, height_default);
    m_fixed_cell_height = height_default;

    //Look at each column:
    for(auto iter = m_column_items.begin(); iter != m_column_items.end(); ++iter)
    {
      Glib::ustring font_name;

      std::shared_ptr<const LayoutItem_WithFormatting> item_withformatting = std::dynamic_pointer_cast<const LayoutItem_WithFormatting>(*iter);
      if(item_withformatting)
      {
         const auto formatting = item_withformatting->get_formatting_used();
         font_name = formatting.get_text_format_font();
      }

      if(font_name.empty())
        continue;

      // Translators: This is just some example text used to discover an appropriate height for user-entered text in the UI. This text itself is never shown to the user.
      Glib::RefPtr<Pango::Layout> refLayout = m_TreeView.create_pango_layout(_("ExampleEg"));
      const Pango::FontDescription font(font_name);
      refLayout->set_font_description(font);
      int width = 0;
      int height = 0;
      refLayout->get_pixel_size(width, height);

      if(height > (int)m_fixed_cell_height)
        m_fixed_cell_height = (guint)height;
    }
  }
  
  //We add extra spacing, because otherwise the bottom of letters such as "g" get cut off.
  //We get this style property, which might be causing it. murrayc 
  //TODO: Find out if this is reallyt the right way to calculate the correct height:
  int extra_height = 0;
  gtk_widget_style_get(GTK_WIDGET(m_TreeView.gobj()), "vertical-separator", &extra_height, (void*)0);
  //std::cout << "debug: extra_height=" << extra_height << std::endl;

  return m_fixed_cell_height + extra_height;
}


Gtk::CellRenderer* DbAddDel::construct_specified_columns_cellrenderer(const std::shared_ptr<LayoutItem>& layout_item, int model_column_index, int data_model_column_index)
{
  InnerIgnore innerIgnore(this); //see comments for InnerIgnore class

  auto pCellRenderer = create_cell(layout_item, m_table_name, get_document(), get_fixed_cell_height());

  std::shared_ptr<const LayoutItem_Field> item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);

  //Set extra cellrenderer attributes, depending on the type used,
  //to support editing:

  auto pCellRendererText = dynamic_cast<Gtk::CellRendererText*>(pCellRenderer);
  if(pCellRendererText)
  {
    //Connect to edited signal:
    if(item_field) //Only fields can be edited:
    {
      pCellRendererText->signal_editing_started().connect(sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_editing_started));

      //Make it editable:
      pCellRendererText->property_editable() = true;

      //Connect to its signal:
      pCellRendererText->signal_edited().connect(
        sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited), model_column_index, data_model_column_index) );
    }
  }
  else
  {
    auto pCellRendererToggle = dynamic_cast<Gtk::CellRendererToggle*>(pCellRenderer);
    if(pCellRendererToggle)
    {
      pCellRendererToggle->property_activatable() = true;

      if(item_field) //Only fields can be edited:
      {
        //Connect to its signal:
        pCellRendererToggle->signal_toggled().connect(
          sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_cell_edited_bool), model_column_index, data_model_column_index ) );
      }
    }
    else
    {
      auto pCellRendererPixbuf = dynamic_cast<Gtk::CellRendererPixbuf*>(pCellRenderer);
      if(pCellRendererPixbuf)
      {
        //TODO: Do something when it's clicked, such as show the big image in a window or tooltip?
      }
    }
  }

  auto pCellButton = Gtk::manage( new GlomCellRenderer_ButtonText() );
  if(pCellButton)
  {
    std::shared_ptr<const LayoutItem_Button> item_button = std::dynamic_pointer_cast<const LayoutItem_Button>(layout_item);
    if(item_button)
    {
      pCellButton->signal_clicked().connect(
        sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_cell_layout_button_clicked), model_column_index) );
    }
  }

  return pCellRenderer;
}

void DbAddDel::construct_specified_columns()
{
  InnerIgnore innerIgnore(this);

  //TODO_optimisation: This is called many times, just to simplify the API.

  //Delay actual use of set_column_*() stuff until this method is called.

  if(m_column_items.empty())
  {
    //std::cout << "debug: " << G_STRFUNC << ": showing hint model: m_find_mode=" << m_find_mode << std::endl;

    m_refListStore.reset();
    if(m_table_name.empty())
    {
      m_TreeView.set_model(m_refListStore); // clear old model from treeview
    }
    else
      show_hint_model();
    return;
  }

  m_refListStore = DbTreeModel::create(m_found_set, m_column_items, m_allow_view, m_find_mode, m_FieldsShown);
  //m_FieldsShown is needed by Base_DB_Table_Data::record_new().

  m_TreeView.set_model(m_refListStore); // clear old model from treeview

  //Remove all View columns:
  treeview_delete_all_columns();


  //Add new View Colums:
  int model_column_index = 0; //Not including the hidden internal columns.
  int view_column_index = 0;

  {
    auto pCellButton = Gtk::manage(new GlomCellRenderer_ButtonImage());

    pCellButton->signal_clicked().connect(sigc::mem_fun(*this, &DbAddDel::on_cell_button_clicked));

    m_treeviewcolumn_button = Gtk::manage(new Gtk::TreeViewColumn());
    m_treeviewcolumn_button->pack_start(*pCellButton);


    Gtk::Requisition requistion_min, requistion_natural; //TODO: Really support natural size.
    pCellButton->get_preferred_size(m_TreeView, requistion_min, requistion_natural);

    m_treeviewcolumn_button->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED); //Needed by fixed-height mode.

    // TODO: I am not sure whether this is always correct. Perhaps, we also
    // have to take into account the xpad property of the cell renderer and
    // the spacing property of the treeviewcolumn.
    int horizontal_separator = 0;
    m_TreeView.get_style_property("horizontal-separator", horizontal_separator);
    const int button_width = requistion_min.width + horizontal_separator*2;
    if(button_width > 0) //Otherwise an assertion fails.
      m_treeviewcolumn_button->set_fixed_width(button_width);

    m_treeviewcolumn_button->set_visible(m_allow_view_details);

    m_TreeView.append_column(*m_treeviewcolumn_button);

    ++view_column_index;
  }

  bool no_columns_used = true;
  int data_model_column_index = 0; //-1 means undefined index.

  guint column_to_expand = 0;
  const auto has_expandable_column = get_column_to_expand(column_to_expand);
  //std::cout << "DEBUG: column_to_expand=" << column_to_expand  << ", has=" << has_expandable_column << std::endl;

  for(auto iter = m_column_items.begin(); iter != m_column_items.end(); ++iter)
  {
    const auto layout_item = m_column_items[model_column_index]; //TODO: Inefficient.
    if(layout_item) //column_info.m_visible)
    {
      no_columns_used = false;

      const auto column_name = item_get_title_or_name(layout_item);
      const auto column_id = layout_item->get_name();

      // Whenever we are dealing with real database fields,
      // we need to know the index of the field in the query:
      int item_data_model_column_index = -1;
      std::shared_ptr<const LayoutItem_Field> item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);
      if(item_field)
      {
        item_data_model_column_index = data_model_column_index;
        ++data_model_column_index;
      }

      //Add the ViewColumn
      auto pCellRenderer = construct_specified_columns_cellrenderer(layout_item, model_column_index, item_data_model_column_index);
      if(pCellRenderer)
      {
        //Get the index of the field in the query, if it is a field:
        //std::cout << "debug: model_column_index=" << model_column_index << ", item_data_model_column_index=" << item_data_model_column_index << std::endl;
        const bool expand = has_expandable_column && ((int)column_to_expand == model_column_index);
        treeview_append_column(column_name,
          *pCellRenderer,
          model_column_index, item_data_model_column_index,
          expand);

        /* TODO:
        if(column_info.m_editable)
        {

        }
        */

        ++view_column_index;
      }

    } //is visible

    ++model_column_index;
  } //for


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

  //Adjust the number of rows to show.
  //This can change the amount of height requested for the widget.
  //Show as many rows as needed, but not more than the maximum:
  gulong total = 0; //ignored
  gulong db_rows_count_found = 0;
  Glib::RefPtr<DbTreeModel> refModelDerived = Glib::RefPtr<DbTreeModel>::cast_dynamic(m_refListStore);
  if(refModelDerived)
    refModelDerived->get_record_counts(total, db_rows_count_found);
  
  //+1 for the empty row:
  gulong rows_count = std::min(m_rows_count_max,  db_rows_count_found + 1);
  //Do not use less than the minimum:
  rows_count = std::max(rows_count, m_rows_count_min);
  set_height_rows_actual(rows_count);
}

bool DbAddDel::refresh_from_database()
{
  construct_specified_columns();
  return true;

  /*
  if(m_refListStore)
  {
    //Glib::RefPtr<Gtk::TreeModel> refNull;
    const auto result = m_refListStore->refresh_from_database(m_found_set);
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

void DbAddDel::set_value(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value)
{
  set_value(iter, layout_item, value, true /* including the specified field */);
}

void DbAddDel::set_value(const Gtk::TreeModel::iterator& iter, const std::shared_ptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value, bool set_specified_field_layout)
{
  //g_warning("DbAddDel::set_value begin");

  InnerIgnore innerIgnore(this);

  if(!m_refListStore)
  {
    std::cerr << G_STRFUNC << ": No model." << std::endl;
    return;
  }

  //Show the value in any columns:
  Gtk::TreeModel::Row treerow = *iter;
  if(treerow)
  {
    const auto list_indexes = get_data_model_column_index(layout_item, set_specified_field_layout);
    for(const auto& item : list_indexes)
    {
      const guint treemodel_col = item + get_count_hidden_system_columns();
      treerow.set_value(treemodel_col, value);
    }
  }

  /// Get indexes of any columns with choices with !show_all relationships that have @a from_key as the from_key.
  const auto list_choice_cells = get_choice_index(layout_item /* from_key field name */);
  for(const auto& model_index : list_choice_cells)
  {
    refresh_cell_choices_data_from_database_with_foreign_key(model_index, value /* foreign key value */);
  }

  //Add extra blank if necessary:
  //add_blank();

  //g_warning("DbAddDel::set_value end");
}

void DbAddDel::set_value_selected(const std::shared_ptr<const LayoutItem_Field>& layout_item, const Gnome::Gda::Value& value)
{
  set_value(get_item_selected(), layout_item, value);
}

void DbAddDel::refresh_cell_choices_data_from_database_with_foreign_key(guint model_index, const Gnome::Gda::Value& foreign_key_value)
{
  if(m_column_items.size() <= model_index)
  {
    std::cerr << G_STRFUNC << ": model_index is out of range: model_index=" << model_index << ", size=" << m_column_items.size() << std::endl;
    return;
  }

  std::shared_ptr<const LayoutItem> item = m_column_items[model_index];
  std::shared_ptr<const LayoutItem_Field> layout_field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
  if(!layout_field)
  {
    std::cerr << G_STRFUNC << ": The layout item was not a LayoutItem_Field." << std::endl;
    return;
  }

  guint view_column_index = 0;
  const auto test = get_view_column_index(model_index, view_column_index);
  if(!test)
  {
    std::cerr << G_STRFUNC << ": view column not found for model_column=" << model_index << std::endl;
    return;
  }

  auto cell =
    dynamic_cast<CellRendererDbList*>( m_TreeView.get_column_cell_renderer(view_column_index) );
  if(!cell)
  {
    std::cerr << G_STRFUNC << ": cell renderer not found for model_column=" << model_index << std::endl;
    return;
  }

  cell->set_choices_related(get_document(), layout_field, foreign_key_value);
}

void DbAddDel::remove_all_columns()
{
  m_column_items.clear();

  m_fixed_cell_height = 0; //Force it to be recalculated.
}

void DbAddDel::set_table_name(const Glib::ustring& table_name)
{
  m_found_set.m_table_name = table_name;
  Base_DB_Table::m_table_name = table_name;
}

void DbAddDel::set_columns(const LayoutGroup::type_list_items& layout_items)
{
  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add a new column.

  for(const auto& layout_item : layout_items)
  {
    if(!layout_item)
      continue; //TODO: Do something more sensible.

    //Make it non-editable if it is auto-generated:
    //TODO: Actually use this bool:
    /*
    std::shared_ptr<const LayoutItem_Field> field = std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);
    if(field)
    {
      std::shared_ptr<const Field> field_full = field->get_full_field_details();
      if(field_full && field_full->get_auto_increment())
        column_info.m_editable = false;
      else
        column_info.m_editable = field->get_editable_and_allowed();
    }
    */

    m_column_items.push_back(layout_item);
  }

  //Generate appropriate model columns:
  construct_specified_columns();
}

void DbAddDel::set_found_set(const FoundSet& found_set)
{
  m_found_set = found_set;
}

FoundSet DbAddDel::get_found_set() const
{
  return m_found_set;
}

DbAddDel::type_list_indexes DbAddDel::get_data_model_column_index(const std::shared_ptr<const LayoutItem_Field>& layout_item_field, bool including_specified_field_layout) const
{
  //TODO_Performance: Replace all this looping by a cache/map:

  type_list_indexes list_indexes;

  if(!layout_item_field)
    return list_indexes;

  guint data_model_column_index = 0;
  for(const auto& item : m_column_items)
  {
    std::shared_ptr<const LayoutItem_Field> field = std::dynamic_pointer_cast<const LayoutItem_Field>(item); //TODO_Performance: This would be unnecessary if !layout_item_field
    if(field)
    {
      if(field->is_same_field(layout_item_field)
        && (including_specified_field_layout || field != layout_item_field))
      {
        list_indexes.push_back(data_model_column_index);
      }

      ++data_model_column_index;
    }
  }

  return list_indexes;
}

DbAddDel::type_list_indexes DbAddDel::get_column_index(const std::shared_ptr<const LayoutItem>& layout_item) const
{
  //TODO_Performance: Replace all this looping by a cache/map:

  type_list_indexes list_indexes;

  if(!layout_item)
  {
    std::cerr << G_STRFUNC << ": layout_item was null." << std::endl;
    return list_indexes;
  }

  std::shared_ptr<const LayoutItem_Field> layout_item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);

  guint i = 0;
  for(const auto& item : m_column_items)
  {
    const std::shared_ptr<const LayoutItem_Field> field = std::dynamic_pointer_cast<const LayoutItem_Field>(item); //TODO_Performance: This would be unnecessary if !layout_item_field
    if(field && layout_item_field && field->is_same_field(layout_item_field))
    {
      list_indexes.push_back(i);
    }
    else if(*(item) == *(layout_item))
    {
      list_indexes.push_back(i);
    }

    ++i;
  }

  return list_indexes;
}

DbAddDel::type_list_indexes DbAddDel::get_choice_index(const std::shared_ptr<const LayoutItem_Field>& from_key)
{
  type_list_indexes result;

  if(!from_key)
    return result;

  const auto from_key_name = from_key->get_name();

  guint index = 0;
  for(const auto& item : m_column_items)
  {
    std::shared_ptr<const LayoutItem_Field> field = std::dynamic_pointer_cast<const LayoutItem_Field>(item);
    if(!field)
       continue;

    const auto format = field->get_formatting_used();

    bool choice_show_all = false;
    const std::shared_ptr<const Relationship> choice_relationship =
      format.get_choices_related_relationship(choice_show_all);
    if(choice_relationship && !choice_show_all) //"Show All" choices don't use the ID field values.
    {
      if(choice_relationship->get_from_field() == from_key_name)
        result.push_back(index);
    }

    index++;
  }

  return result;
}


std::shared_ptr<const LayoutItem_Field> DbAddDel::get_column_field(guint column_index) const
{
  if(column_index < m_column_items.size())
  {
    std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>( m_column_items[column_index] );
    if(field)
      return field;
  }

  return std::shared_ptr<const LayoutItem_Field>();
}

bool DbAddDel::get_prevent_user_signals() const
{
  return m_bPreventUserSignals;
}

void DbAddDel::set_prevent_user_signals(bool bVal)
{
  m_bPreventUserSignals = bVal;
}

/*
//This is generally used for non-database-data lists.
void DbAddDel::set_column_choices(guint col, const type_vec_strings& vecStrings)
{
  InnerIgnore innerIgnore(this); //Stop on_treeview_columns_changed() from doing anything when it is called just because we add new columns.

  m_column_items[col].m_choices = vecStrings;

  guint view_column_index = 0;
  const auto test = get_view_column_index(col, view_column_index);
  if(test)
  {
    auto pCellRenderer =
      dynamic_cast<CellRendererDbList*>( m_TreeView.get_column_cell_renderer(view_column_index) );
    if(pCellRenderer)
    {
      //Add the choices:
      pCellRenderer->remove_all_list_items();
      for(const auto& item : vecStrings)
      {
        pCellRenderer->append_list_item(item);
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
*/

void DbAddDel::set_allow_add(bool val)
{
  m_allow_add = val;

  m_refContextAdd->set_enabled(val);
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

  m_pOuter = 0;
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

DbAddDel::type_signal_record_selection_changed DbAddDel::signal_record_selection_changed()
{
  return m_signal_record_selection_changed;
}

void DbAddDel::on_cell_layout_button_clicked(const Gtk::TreeModel::Path& path, int model_column_index)
{
  if(!m_refListStore)
    return;

  auto iter = m_refListStore->get_iter(path);
  if(iter)
  {
    std::shared_ptr<const LayoutItem> layout_item = m_column_items[model_column_index];
    std::shared_ptr<const LayoutItem_Button> item_button = std::dynamic_pointer_cast<const LayoutItem_Button>(layout_item);
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
  auto iter = m_refListStore->get_iter(path);
  if(iter)
  {
    Gtk::TreeModel::Row row = *iter;

    const int tree_model_column_index = data_model_column_index + get_count_hidden_system_columns();

    Gnome::Gda::Value value_old;
    row.get_value(tree_model_column_index, value_old);

    const auto bValueOld = (value_old.get_value_type() == G_TYPE_BOOLEAN) && value_old.get_boolean();
    const bool bValueNew = !bValueOld;
    Gnome::Gda::Value value_new;
    value_new.set(bValueNew);
    //Store the user's new value in the model:
    row.set_value(tree_model_column_index, value_new);

    //TODO: Did it really change?

    //Is this an add or a change?:

    bool bIsAdd = false;
    bool bIsChange = false;

    const auto iCount = m_refListStore->children().size();
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

void DbAddDel::on_idle_treeview_cell_edited_revert(const Gtk::TreeModel::Row& row, guint model_column_index)
{
  Glib::RefPtr<Gtk::TreeSelection> refTreeSelection = m_TreeView.get_selection();
  if(!refTreeSelection)
    return;
    
  refTreeSelection->select(row); //TODO: This does not seem to work.
  
  guint view_column_index = 0;
  get_view_column_index(model_column_index, view_column_index);
  auto pColumn = m_TreeView.get_column(view_column_index);
  if(!pColumn)
  {
    std::cerr << G_STRFUNC << ": pColumn is null." << std::endl;
    return;
  }
  
  auto pCell = dynamic_cast<Gtk::CellRendererText*>(pColumn->get_first_cell());
  if(!pCell)
  {
    std::cerr << G_STRFUNC << ": pCell is null." << std::endl;
    return;
  }
    
  const auto path = get_model()->get_path(row);
  
  //Highlights the cell, and start the editing:
  m_TreeView.set_cursor(path, *pColumn, *pCell, true /* start_editing */);
}

void DbAddDel::on_treeview_cell_edited(const Glib::ustring& path_string, const Glib::ustring& new_text, int model_column_index, int data_model_column_index)
{
  //Note:: model_column_index is actually the AddDel column index, not the TreeModel column index.
  if(path_string.empty())
    return;

  if(!m_refListStore)
    return;

  const Gtk::TreeModel::Path path(path_string);
 
  if(path.empty())
  {
    std::cerr << G_STRFUNC << ": path is empty." << std::endl;
    return;
  }

  //Get the row from the path:
  auto iter = m_refListStore->get_iter(path);
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
        const auto bPreventUserSignals = get_prevent_user_signals();
        set_prevent_user_signals(true); //Stops extra signal_user_changed.

        //Don't add a new row if nothing was entered into the placeholder.
        if(new_text.empty())
          return;

        //Mark this row as no longer a placeholder, because it has data now. The client code must set an actual key for this in the signal_user_added() or m_signal_user_changed signal handlers.
        m_refListStore->set_is_not_placeholder(iter);
        //Don't mark this as not a placeholder, because it's still a placeholder until it has a key value.

        set_prevent_user_signals(bPreventUserSignals);

        bIsAdd = true; //Signal that a new key was added.
      }
    }


    std::shared_ptr<LayoutItem_Field> item_field = std::dynamic_pointer_cast<LayoutItem_Field>(m_column_items[model_column_index]);
    if(!item_field)
      return;

    const auto field_type = item_field->get_glom_type();
    if(field_type != Field::glom_field_type::INVALID) //If a field type was specified for this column.
    {
      //Make sure that the entered data is suitable for this field type:
      bool success = false;

      Glib::ustring new_text_to_save = new_text;

      //If this layout field uses a translatable set of custom choices,
      //then make sure that we only write the original to the database, though we display the translated version:
      if(item_field->get_formatting_used_has_translatable_choices())
      {
        const auto formatting = item_field->get_formatting_used();
        new_text_to_save = formatting.get_custom_choice_original_for_translated_text(new_text);

        //If somehow (though this should be impossible), the user entered a 
        //translated value with no corresponding original text, then
        //store the translated version rather than losing data.
        if(new_text_to_save.empty())
          new_text_to_save = new_text;
      }

      const auto value = Conversions::parse_value(field_type, new_text_to_save, item_field->get_formatting_used().m_numeric_format, success);
      if(!success)
      {  
          //Tell the user and offer to revert or try again:
          const auto revert = glom_show_dialog_invalid_data(field_type);
          if(revert)
          {
            //Revert the data:
            row.set_value(treemodel_column_index, valOld);
          }
          else
          {
            //Reactivate the cell so that the data can be corrected.
            
            //Set the text to be used in the start_editing signal handler:
            m_validation_invalid_text_for_retry = new_text;
            m_validation_retry = true;

            //But do this in an idle timout, so that the TreeView doesn't get 
            //confused by us changing editing state in this signal handler.
            Glib::signal_idle().connect_once(
              sigc::bind(
               sigc::mem_fun(*this, &DbAddDel::on_idle_treeview_cell_edited_revert),
               row, model_column_index));
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

void DbAddDel::on_treeview_button_press_event(GdkEventButton* button_event)
{
  on_button_press_event_Popup(button_event);
}

/* We do not let the developer resize the columns directly in the treeview
 * because we cannot easily avoid this signal handler from being called just during the 
 * intial size allocation.
 * Anyway, this would be rather implicit anyway - people might not know that they are changing it in the document.
 * The size can still be specified in the layout dialog.
 */
/*
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
  std::vector<Gtk::TreeView::Column*> columns = m_TreeView.get_columns();
  const auto n_view_columns = columns.size();
  if(n_view_columns && (view_column == m_TreeView.get_column(n_view_columns -1)))
    return;

  const std::shared_ptr<LayoutItem>& layout_item = m_column_items[model_column_index];

  const auto width = view_column->get_width();
  //std::cout << "debug: " << G_STRFUNC << ": width=" << width << std::endl;

  if(width == -1) //Means automatic.
    return;

  if(layout_item)
    layout_item->set_display_width(width);
}
*/

void DbAddDel::on_treeview_column_clicked(int model_column_index)
{
  BusyCursor busy_cursor(get_appwindow());

  if(model_column_index >= (int)m_column_items.size())
    return;

  std::shared_ptr<const LayoutItem_Field> layout_item = std::dynamic_pointer_cast<const LayoutItem_Field>(m_column_items[model_column_index]); //We can only sort on fields, not on other layout item.
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

    for(auto iter = vecViewColumns.begin(); iter != vecViewColumns.end(); ++iter)
    {
      auto pViewColumn = dynamic_cast<DbTreeViewColumnGlom*>(*iter);
      if(pViewColumn)
      {
        const auto column_id = pViewColumn->get_column_id();
        m_vecColumnIDs.push_back(column_id);

      }
    }

    //Tell other code that something has changed, so the new column order can be serialized.
    //TODO: If this is ever wanted: m_signal_user_reordered_columns.emit();
  }
}

bool DbAddDel::get_column_to_expand(guint& column_to_expand) const
{
  //Initialize output parameter:
  column_to_expand = 0;
  bool result = false;

  //Discover the right-most text column:
  guint i = 0;
  for(const auto& layout_item : m_column_items)
  {
    std::shared_ptr<LayoutItem_Field> layout_item_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
    if(layout_item_field)
    {
      //Only text columns should expand.
      //Number fields are right-aligned, so expansion is annoying.
      //Time and date fields don't vary their width much.
      if(layout_item_field->get_glom_type() == Field::glom_field_type::TEXT)
      {
        //Check that no specific width has been specified:
        const auto column_width = layout_item_field->get_display_width();
        if(!column_width)
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

  auto pViewColumn = Gtk::manage( new DbTreeViewColumnGlom(Utils::string_escape_underscores(title), cellrenderer) );

  //This is needed by fixed-height mode. We get critical warnings otherwise.
  //But we must call set_fixed_width() later or we will have a zero-width column.
  pViewColumn->set_sizing(Gtk::TREE_VIEW_COLUMN_FIXED);

  guint cols_count = m_TreeView.append_column(*pViewColumn);

  std::shared_ptr<const LayoutItem> layout_item = m_column_items[model_column_index];
  std::shared_ptr<const LayoutItem_Field> layout_item_field = std::dynamic_pointer_cast<const LayoutItem_Field>(layout_item);

  //Tell the Treeview.how to render the Gnome::Gda::Values:
  if(layout_item_field)
  {
    pViewColumn->set_cell_data_func(cellrenderer,
      sigc::bind( sigc::mem_fun(*this, &DbAddDel::treeviewcolumn_on_cell_data), model_column_index, data_model_column_index) );
  }

  //Allow the column to be reordered by dragging and dropping the column header:
  pViewColumn->set_reorderable();

  //Allow the column to be resized:
  pViewColumn->set_resizable();

  //GtkTreeView's fixed-height-mode does not allow us to have anything but
  //the last column as expandable.
  //TODO: Can we get the total size and calculate a starting size instead?
  expand = false;

  int column_width = -1; //Means expand.
  if(!expand)
  {
    column_width = layout_item->get_display_width();
    if(!(column_width > 0))
    {
      //TODO: Choose a width based on the first 100 values.
      if(layout_item_field)
      {
       column_width = UiUtils::get_suitable_field_width_for_widget(*this, layout_item_field, true /* or_title */, true /* for treeview */);
       column_width = column_width + 8; //Some extra for the GtkTreeView's padding.
       //std::cout << "DEBUG: column_width=" << column_width << std::endl;
      }
      else
        column_width = 100; //TODO: Don't save this default in the document.
    }
  }

  if(column_width > 0) //Otherwise there's an assertion fails.
    pViewColumn->set_fixed_width(column_width); //This is the only way to set the width, so we need to set it as resizable again immediately afterwards.

  pViewColumn->set_resizable();
  //This property is read only: pViewColumn->property_width() = column_width;

  //Save the extra ID, using the title if the column_id is empty:
  const auto column_id = m_column_items[model_column_index]->get_name();
  pViewColumn->set_column_id( (column_id.empty() ? title : column_id) );

  //Let the user click on the column header to sort.
  pViewColumn->set_clickable();
  pViewColumn->signal_clicked().connect(
    sigc::bind( sigc::mem_fun(*this, &DbAddDel::on_treeview_column_clicked), model_column_index) );

  // See the comment on on_treeview_column_resized():
  //pViewColumn->connect_property_changed("width", sigc::bind(sigc::mem_fun(*this, &DbAddDel::on_treeview_column_resized), model_column_index, pViewColumn) );

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

  if(model_column_index >=  m_column_items.size())
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

guint DbAddDel::get_count_hidden_system_columns() const
{
  return 0; //The key now has explicit API in the model.
  //return 1; //The key.
  //return 2; //The key and the placeholder boolean.
}

std::shared_ptr<Field> DbAddDel::get_key_field() const
{
  return m_key_field;
}

void DbAddDel::set_key_field(const std::shared_ptr<Field>& field)
{
  m_key_field = field;
}

void DbAddDel::treeviewcolumn_on_cell_data(Gtk::CellRenderer* renderer, const Gtk::TreeModel::iterator& iter, int model_column_index, int data_model_column_index)
{
  //std::cout << "debug: DbAddDel::treeviewcolumn_on_cell_data()" << std::endl;

  if(iter)
  {
    const std::shared_ptr<LayoutItem>& layout_item = m_column_items[model_column_index];

    std::shared_ptr<LayoutItem_Field> field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
    if(field)
    {
      const guint col_real = data_model_column_index + get_count_hidden_system_columns();
      Gtk::TreeModel::Row treerow = *iter;
      Gnome::Gda::Value value;
      treerow->get_value(col_real, value);

      /*
      GType debug_type = value.get_value_type();
      std::cout << "debug: " << G_STRFUNC << ": GType=" << debug_type << std::endl;
      if(debug_type)
         std::cout << "    GType name=\"" << g_type_name(debug_type) << "\"" << std::endl;
      */

      const auto type = field->get_glom_type();
      switch(type)
      {
        case(Field::glom_field_type::BOOLEAN):
        {
          auto pDerived = dynamic_cast<Gtk::CellRendererToggle*>(renderer);
          if(pDerived)
            pDerived->set_active( (value.get_value_type() == G_TYPE_BOOLEAN) && value.get_boolean() );

          break;
        }
        case(Field::glom_field_type::IMAGE):
        {
          auto pDerived = dynamic_cast<Gtk::CellRendererPixbuf*>(renderer);
          if(pDerived)
          {
            Glib::RefPtr<Gdk::Pixbuf> pixbuf = UiUtils::get_pixbuf_for_gda_value(value);

            //Scale it down to a sensible size.
            if(pixbuf)
              pixbuf = UiUtils::image_scale_keeping_ratio(pixbuf,  get_fixed_cell_height(), pixbuf->get_width());

            pDerived->property_pixbuf() = pixbuf;
          }
          else
            std::cerr << G_STRFUNC << ": glom_type is enumType::IMAGE but gda type is not VALUE_TYPE_BINARY" << std::endl;

          break;
        }
        default:
        {
          //TODO: Maybe we should have custom cellrenderers for time, date, and numbers.
          auto pDerived = dynamic_cast<Gtk::CellRendererText*>(renderer);
          if(pDerived)
          {
            //std::cout << "debug: " << G_STRFUNC << ": field name=" << field->get_name() << ", glom type=" << field->get_glom_type() << std::endl;
            Glib::ustring text = Conversions::get_text_for_gda_value(field->get_glom_type(), value, field->get_formatting_used().m_numeric_format);
            //g_assert(text != "NULL");

            //If this layout field uses a translatable set of custom choices,
            //then make sure that we show the translated version, never showing the original text from the database:
            if(field->get_formatting_used_has_translatable_choices())
            {
              const auto formatting = field->get_formatting_used();
              const auto text_to_show = formatting.get_custom_choice_translated(text);

              //Use the translation if there is one.
              //Otherwise, show the original data rather than showing nothing:
              if(!text_to_show.empty())
               text = text_to_show;
            }

            pDerived->property_text() = text;

            //Show a different color if the value is numeric, if that's specified:
            if(type == Field::glom_field_type::NUMERIC)
            {
               const Glib::ustring fg_color =
                 field->get_formatting_used().get_text_format_color_foreground_to_use(value);
               if(!fg_color.empty())
                   pDerived->property_foreground() = fg_color;
               else
                   pDerived->property_foreground().reset_value();
            }
          }

          break;
        }
      }
    }
  }
}

AppWindow* DbAddDel::get_appwindow()
{
  auto pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

void DbAddDel::on_treeview_cell_editing_started(Gtk::CellEditable* cell_editable, const Glib::ustring& /* path */)
{
  //Start editing with previously-entered (but invalid) text, 
  //if we are allowing the user to correct some invalid data. 
  if(m_validation_retry)
  {
    //This is the CellEditable inside the CellRenderer. 
    auto celleditable_validated = cell_editable;

    //It's usually an Entry, at least for a CellRendererText:
    auto pEntry = dynamic_cast<Gtk::Entry*>(celleditable_validated);
    if(pEntry)
    {
      pEntry->set_text(m_validation_invalid_text_for_retry);
      m_validation_retry = false;
      m_validation_invalid_text_for_retry.clear();
    }
  }
}

void DbAddDel::set_allow_view(bool val)
{
  m_allow_view = val;
}

void DbAddDel::set_allow_view_details(bool val)
{
  m_allow_view_details = val;

  //Hide it if it was visible, if it exists,
  //otherwise do that later after creating it:
  if(m_treeviewcolumn_button)
    m_treeviewcolumn_button->set_visible(val);
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
  treeview_delete_all_columns();
  m_treeviewcolumn_button = nullptr; //When we removed the view columns, this was deleted because it's manage()ed.

  m_model_hint = Gtk::ListStore::create(m_columns_hint);
  auto iter = m_model_hint->append();
  (*iter)[m_columns_hint.m_col_hint] = _("Right-click to layout, to specify the related fields.");

  m_TreeView.set_model(m_model_hint);
  m_TreeView.set_fixed_height_mode(false); //fixed_height mode is incompatible with the default append_column() helper method.
  m_TreeView.append_column("", m_columns_hint.m_col_hint);
}

bool DbAddDel::start_new_record()
{
  auto iter = get_item_placeholder();
  if(!iter)
    return false;

  std::shared_ptr<LayoutItem_Field> fieldToEdit;

  //Start editing in the primary key or the first cell if the primary key is auto-incremented (because there is no point in editing an auto-generated value)
  //guint index_primary_key = 0;
  const bool bPresent = true; //get_field_primary_key_index(index_primary_key); //If there is no primary key then the default of 0 is OK.
  if(!bPresent)
    return false;

  std::shared_ptr<Field> fieldPrimaryKey = get_key_field();
  if(fieldPrimaryKey && fieldPrimaryKey->get_auto_increment())
  {
    //Start editing in the first cell that is not auto_increment:
    for(const auto& layout_item : m_column_items)
    {
      std::shared_ptr<LayoutItem_Field> layout_item_field = std::dynamic_pointer_cast<LayoutItem_Field>(layout_item);
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
    fieldToEdit = std::make_shared<LayoutItem_Field>();
    fieldToEdit->set_full_field_details(fieldPrimaryKey);
  }

  //std::cout << "debug: index_field_to_edit=" << index_field_to_edit << std::endl;

  if(fieldToEdit)
  {
    select_item(iter, fieldToEdit, true /* start_editing */);
  }
  else
  {
    //The only keys are non-editable, so just add a row:
    select_item(iter); //without start_editing.
    //g_warning("start_new_record(): index_field_to_edit does not exist: %d", index_field_to_edit);
  }

  return true;
}

void DbAddDel::user_changed(const Gtk::TreeModel::iterator& row, guint col)
{
  const auto parent_primary_key_value = get_value_key(row);
  //std::cout << "debug: " << G_STRFUNC << ": parent_primary_key_value=" << parent_primary_key_value.to_string() << std::endl;

  std::shared_ptr<const LayoutItem_Field> layout_field = get_column_field(col);

  if(!Conversions::value_is_empty(parent_primary_key_value)) //If the record's primary key is filled in:
  {
    Glib::ustring table_name = m_found_set.m_table_name;
    std::shared_ptr<Field> primary_key_field;
    Gnome::Gda::Value primary_key_value;
    auto window = get_appwindow();

    //Just update the record:
    try
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
        const auto relationship_name = layout_field->get_relationship_name();

        auto document = dynamic_cast<Document*>(get_document());

        std::shared_ptr<Relationship> relationship = document->get_relationship(m_found_set.m_table_name, relationship_name);
        if(relationship)
        {
          table_name = relationship->get_to_table();
          const auto to_field_name = relationship->get_to_field();
          //Get the key field in the other table (the table that we will change)
          primary_key_field = DbUtils::get_fields_for_table_one_field(document, 
            table_name, to_field_name); //TODO_Performance.
          if(primary_key_field)
          {
            //Get the value of the corresponding key in the current table (that identifies the record in the table that we will change)
            std::shared_ptr<LayoutItem_Field> layout_item = std::make_shared<LayoutItem_Field>();
            layout_item->set_full_field_details( document->get_field(relationship->get_from_table(), relationship->get_from_field()) );

            primary_key_value = get_value_selected(layout_item);

            //Note: This just uses an existing record if one already exists:
            Gnome::Gda::Value primary_key_value_used;
            if(!m_find_mode)
            {
              const auto test = add_related_record_for_field(layout_field, relationship, primary_key_field, primary_key_value, primary_key_value_used);
              if(!test)
                return;
            }

            //Get the new primary_key_value if it has been created:
            primary_key_value = primary_key_value_used;

            //Now that the related record exists, the following code to set the value of the other field in the related field can succeed.
          }
          else
          {
            std::cerr << G_STRFUNC << ": key not found for edited related field." << std::endl;
          }
        }
      }

      //Update the field in the record (the record with this primary key):
      const auto field_value = get_value(row, layout_field);
      //std::cout << "debug: " << G_STRFUNC << ": field_value = " << field_value.to_string() << std::endl;
      //const std::shared_ptr<const Field>& field = layout_field->m_field;
      //const Glib::ustring strFieldName = layout_field->get_name();

      LayoutFieldInRecord field_in_record(layout_field, m_found_set.m_table_name /* parent */, primary_key_field, primary_key_value);

      //Check whether the value meets uniqueness constraints:
      if(!check_entered_value_for_uniqueness(m_found_set.m_table_name, row, layout_field, field_value, window))
      {
        //Revert to the value in the database:
        const auto value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);

        return; //The value has been reverted to the value in the database.
      }


      const auto bTest = set_field_value_in_database(field_in_record, row, field_value, false /* don't use current calculations */, window);
      if(!bTest)
      {
        //Update failed.
        //Replace with correct values.
        const auto value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);
      }
      else
      {
        //Display the same value in other instances of the same field:
        set_value(row, layout_field, field_value, false /* don't set the actually-edited cell */);

        signal_record_changed().emit();
      }
    }
    catch(const Glib::Exception& ex)
    {
      handle_error(ex, get_app_window());

      //Replace with correct values.
      if(primary_key_field)
      {
        LayoutFieldInRecord field_in_record(layout_field, m_found_set.m_table_name /* parent */, primary_key_field, primary_key_value);
        const auto value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);
      }
    }
    catch(const std::exception& ex)
    {
      handle_error(ex, get_app_window());

      //Replace with correct values.
      if(primary_key_field)
      {
        LayoutFieldInRecord field_in_record(layout_field, m_found_set.m_table_name /* parent */, primary_key_field, primary_key_value);
        const auto value_old = get_field_value_in_database(field_in_record, window);
        set_entered_field_data(row, layout_field, value_old);
      }
    }
  }
  else
  {
    //This record probably doesn't exist yet.
    //Add new record, which will generate the primary key:
    user_added(row);

    const auto primaryKeyValue = get_value_key(row); //TODO_Value
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
      std::cout << G_STRFUNC << ": debug: record not yet added." << std::endl;
    }
  }
}


void DbAddDel::user_added(const Gtk::TreeModel::iterator& row)
{
  //Prevent impossible multiple related records:
  //The developer-mode UI now prevents the developer from using such a relationship anyway.
  if(m_allow_only_one_related_record && (get_count() > 0))
  {
    //Tell user that they can't do that:
    Gtk::MessageDialog dialog(UiUtils::bold_message(_("Extra Related Records Not Possible")), true, Gtk::MESSAGE_WARNING);
    dialog.set_secondary_text(_("You attempted to add a new related record, but there can only be one related record, because the relationship uses a unique key.")),
    dialog.set_transient_for(*AppWindow::get_appwindow());
    dialog.run();

    return;
  }

  //std::cout << "DbAddDel::on_adddel_user_added" << std::endl;

  Gnome::Gda::Value primary_key_value;

  std::shared_ptr<const Field> primary_key_field = get_key_field();

  //Get the new primary key value, if one is available now:
  if(primary_key_field->get_auto_increment())
  {
    //Auto-increment is awkward (we can't get the last-generated ID) with postgres, so we auto-generate it ourselves;
    const auto strPrimaryKeyName = primary_key_field->get_name();
    primary_key_value = DbUtils::get_next_auto_increment_value(m_found_set.m_table_name, strPrimaryKeyName);  //TODO: return a Gnome::Gda::Value of an appropriate type.
  }
  else
  {
    //Use the user-entered primary key value:

    //This only works when the primary key is already stored: primary_key_value = get_value_key(row);
    //primary_key_value = get_value_key_selected();

    std::shared_ptr<LayoutItem_Field> layout_field = std::make_shared<LayoutItem_Field>();
    layout_field->set_full_field_details(primary_key_field);
    primary_key_value = get_value_selected(layout_field);
    std::cout << "DEBUG: get_value_key_selected(): " << primary_key_value.to_string() << std::endl;
  }

  //If no primary key value is available yet, then don't add the record yet:
  if(Conversions::value_is_empty(primary_key_value))
    return;

  std::shared_ptr<SharedConnection> sharedconnection = connect_to_server(get_appwindow()); //Keep it alive while we need the data_model.
  if(!sharedconnection)
  {
    //Add Record failed.
    //Replace with correct values:
    fill_from_database();
    return;
  }


  std::shared_ptr<LayoutItem_Field> layout_field = std::make_shared<LayoutItem_Field>();
  layout_field->set_full_field_details(primary_key_field);
  if(!check_entered_value_for_uniqueness(m_found_set.m_table_name, layout_field, primary_key_value, get_appwindow()))
  {
    //Revert to a blank value.
    primary_key_value = Conversions::get_empty_value(layout_field->get_full_field_details()->get_glom_type());
    set_entered_field_data(row, layout_field, primary_key_value);
    return;
  }

  if(m_find_mode)
    return;

  const auto added = record_new(true /* use entered field data*/, primary_key_value);
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
    std::shared_ptr<LayoutItem_Field> layout_item = std::make_shared<LayoutItem_Field>();
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
      const auto primary_key_value = get_primary_key_value(rowStart);
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
void DbAddDel::set_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  return set_value_selected(field, value);
}

//An override of the Base_DB method:
void DbAddDel::set_entered_field_data(const Gtk::TreeModel::iterator& row, const std::shared_ptr<const LayoutItem_Field>& field, const Gnome::Gda::Value& value)
{
  return set_value(row, field, value);
}

Gnome::Gda::Value DbAddDel::get_entered_field_data(const std::shared_ptr<const LayoutItem_Field>& field) const
{
  return get_value_selected(field);
}

std::shared_ptr<Field> DbAddDel::get_field_primary_key() const
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

void DbAddDel::on_treeview_selection_changed()
{
  Glib::RefPtr<Gtk::TreeView::Selection> refSelection = m_TreeView.get_selection();
  if(!refSelection)
    return;

  const auto one_selected = (refSelection->count_selected_rows() > 0);
  on_selection_changed(one_selected);
}

void DbAddDel::on_selection_changed(bool selection)
{
  m_refContextDelete->set_enabled(selection);
  m_refContextAdd->set_enabled(selection);
  
  m_signal_record_selection_changed.emit();
}

void DbAddDel::treeview_delete_all_columns()
{
  UiUtils::treeview_delete_all_columns(&m_TreeView);

  //Reset this too, because we must have just deleted it:
  m_treeviewcolumn_button = nullptr;
}

const Gtk::Window* DbAddDel::get_app_window() const
{
  auto nonconst = const_cast<DbAddDel*>(this);
  return nonconst->get_app_window();
}
  
Gtk::Window* DbAddDel::get_app_window()
{
  return dynamic_cast<Gtk::Window*>(get_toplevel());
}


} //namespace Glom
