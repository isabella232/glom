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

#include "datawidget.h"
#include "entryglom.h"
#include "labelglom.h"
#include "comboentryglom.h"
#include "comboglom.h"
#include "textviewglom.h"
#include "imageglom.h"
#include <glom/libglom/data_structure/glomconversions.h>
#include "../application.h"
#include "../mode_data/dialog_choose_field.h"
#include "dialog_choose_id.h"
#include "dialog_choose_date.h"
#include "../layout_item_dialogs/dialog_field_layout.h"
#include <glom/libglom/utils.h>
#include <glibmm/i18n.h>

namespace Glom
{

/*
DataWidget::DataWidget(Field::glom_field_type glom_type, const Glib::ustring& title)
: m_glom_type(glom_type),
  m_pMenuPopup(0)
*/

DataWidget::DataWidget(const sharedptr<LayoutItem_Field>& field, const Glib::ustring& table_name, const Document_Glom* document)
{
  const Field::glom_field_type glom_type = field->get_glom_type();
  set_layout_item(field, table_name);

  m_child = 0;
  LayoutWidgetField* pFieldWidget = 0;
  const Glib::ustring title = field->get_title_or_name();
  if(glom_type == Field::TYPE_BOOLEAN)
  {
    Gtk::CheckButton* checkbutton = Gtk::manage( new Gtk::CheckButton( title ) );
    checkbutton->show();
    checkbutton->signal_toggled().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

    //TODO: entry->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout );

    m_child = checkbutton;

    m_label.set_text( Glib::ustring() ); //It is not used.
  }
  else if(glom_type == Field::TYPE_IMAGE)
  {
    ImageGlom* image = Gtk::manage( new ImageGlom() );
    image->set_size_request(200, 200);
    //Gtk::Image* image = Gtk::manage( new Gtk::Image("/home/murrayc/gnome-small.jpg") );
    image->show();
    //TODO: Respond to double-click: checkbutton->signal_toggled().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

    //TODO: entry->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout );

    m_child = image;
    pFieldWidget = image;

    m_label.set_label(title);
    m_label.set_alignment(0);
    m_label.show();
  }
  else
  {
    m_label.set_label(title);
    m_label.set_alignment(0);
    m_label.show();

    //Use a Combo if there is a drop-down of choices (A "value list"), else an Entry:
    if(field->get_formatting_used().get_has_choices())
    {
      ComboGlomChoicesBase* combo = 0; //Gtk::manage(new ComboEntryGlom());

      if(field->get_formatting_used().get_has_custom_choices())
      {
        if(field->get_formatting_used().get_choices_restricted())
          combo = Gtk::manage(new ComboGlom());
        else
          combo = Gtk::manage(new ComboEntryGlom());

        //set_choices() needs this, for the numeric layout:
        combo->set_layout_item( get_layout_item(), table_name);

        combo->set_choices( field->get_formatting_used().get_choices_custom() );
      }
      else if(field->get_formatting_used().get_has_related_choices())
      {
        sharedptr<Relationship> choice_relationship;
        Glib::ustring choice_field, choice_second;
        field->get_formatting_used().get_choices(choice_relationship, choice_field, choice_second);
        if(choice_relationship && !choice_field.empty())
        {
          const Glib::ustring to_table = choice_relationship->get_to_table();

          const bool with_second = !choice_second.empty();

          if(with_second && document)
          {
            sharedptr<Field> field_second = document->get_field(to_table, choice_second);

            sharedptr<LayoutItem_Field> layout_field_second = sharedptr<LayoutItem_Field>::create();
            layout_field_second->set_full_field_details(field_second);
            //We use the default formatting for this field->

            if(field->get_formatting_used().get_choices_restricted())
              combo = Gtk::manage(new ComboGlom(layout_field_second));
            else
              combo = Gtk::manage(new ComboEntryGlom(layout_field_second));
          }
          else
          {
            if(field->get_formatting_used().get_choices_restricted())
              combo = Gtk::manage(new ComboGlom());
            else
              combo = Gtk::manage(new ComboEntryGlom());
          }

          //set_choices() needs this, for the numeric layout:
          combo->set_layout_item( get_layout_item(), table_name);

          combo->set_choices_with_second( Utils::get_choice_values(field) );
        }
      }
      else
      {
        g_warning("DataWidget::DataWidget(): Unexpected choice type.");
      }

      pFieldWidget = combo;
    }
    else
    {
      if((glom_type == Field::TYPE_TEXT) && (field->get_formatting_used().get_text_format_multiline()))
      {
        TextViewGlom* textview = Gtk::manage(new TextViewGlom(glom_type));
        pFieldWidget = textview;
      }
      else
      {
        EntryGlom* entry = Gtk::manage(new EntryGlom(glom_type));
        pFieldWidget = entry;
      }

      pFieldWidget->set_layout_item( get_layout_item(), table_name); //TODO_Performance: We only need this for the numerical format.
    }
  }

  if(pFieldWidget)
  {
    pFieldWidget->signal_edited().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

#ifndef GLOM_ENABLE_CLIENT_ONLY
    pFieldWidget->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout) );
    pFieldWidget->signal_user_requested_layout_properties().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout_properties) );
    pFieldWidget->signal_layout_item_added().connect( sigc::mem_fun(*this, &DataWidget::on_child_layout_item_added) );
#endif // !GLOM_ENABLE_CLIENT_ONLY


    m_child = dynamic_cast<Gtk::Widget*>(pFieldWidget);
    const int width = get_suitable_width(field);

    if(glom_type == Field::TYPE_IMAGE) //GtkImage widgets default to no size (invisible) if they are empty.
      m_child->set_size_request(width, width);
    else
    {
      int height = -1; //auto.
      if((glom_type == Field::TYPE_TEXT) && (field->get_formatting_used().get_text_format_multiline()))
      {
        int example_width = 0;
        int example_height = 0;
        Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout("example"); //TODO: Use different text, according to the current locale, or allow the user to choose an example?
        refLayout->get_pixel_size(example_width, example_height);
        if(example_height > 0)
          height = example_height * field->get_formatting_used().get_text_format_multiline_height_lines();
      }

      m_child->set_size_request(width, height);
    }

    m_child->show_all();
  }

  if(m_child)
  {
    bool child_added = false; //Don't use an extra container unless necessary.
    const bool field_used_in_relationship_to_one = document->get_field_used_in_relationship_to_one(table_name, field->get_name());

    Gtk::HBox* hbox_parent = 0; //Only used if there are extra widgets.
    const bool with_extra_widgets = field_used_in_relationship_to_one || (glom_type == Field::TYPE_DATE);
    if(with_extra_widgets)
    {
      hbox_parent = Gtk::manage( new Gtk::HBox() ); //We put the child (and any extra stuff) in this:
      hbox_parent->set_spacing(6);

      hbox_parent->pack_start(*m_child);
      add(*hbox_parent);

      child_added = true;
    }

    if(glom_type == Field::TYPE_DATE)
    {
      //Let the user choose a date from a calendar dialog:
      Gtk::Button* button_date = Gtk::manage(new Gtk::Button(_("..."))); //TODO: A better label/icon for "Choose Date". 
      button_date->show();
      hbox_parent->pack_start(*button_date);
      button_date->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_choose_date));
    }

    if(field_used_in_relationship_to_one && hbox_parent)
    {
      //Add buttons for related record navigation:
      Gtk::Button* button_go_to_details = Gtk::manage(new Gtk::Button(Gtk::Stock::OPEN));
      hbox_parent->pack_start(*button_go_to_details);
      button_go_to_details->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_open_details));

      Gtk::Button* button_select = Gtk::manage(new Gtk::Button(Gtk::Stock::FIND));
      hbox_parent->pack_start(*button_select);
      button_select->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_select_id));
    }

    if(!child_added)
      add(*m_child);
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu();
#endif // GLOM_ENABLE_CLIENT_ONLY

  set_events(Gdk::BUTTON_PRESS_MASK);
}

DataWidget::~DataWidget()
{
}

void DataWidget::on_widget_edited()
{
  m_signal_edited.emit(get_value());
}

DataWidget::type_signal_edited DataWidget::signal_edited()
{
  return m_signal_edited;
}


void DataWidget::set_value(const Gnome::Gda::Value& value)
{
  Gtk::Widget* widget = get_data_child_widget();
  LayoutWidgetField* generic_field_widget = dynamic_cast<LayoutWidgetField*>(widget);
  if(generic_field_widget)
  {
    //if(generic_field_widget->get_layout_item())
    //  std::cout << "DataWidget::set_value(): generic_field_widget->get_layout_item()->get_name()=" << generic_field_widget->get_layout_item()->get_name() << std::endl;

    generic_field_widget->set_value(value);
  }
  else
  {
    Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(widget);
    if(checkbutton)
    {
      bool bValue = false;
      if(!value.is_null() && value.get_value_type() == G_TYPE_BOOLEAN)
        bValue = value.get_boolean();

      checkbutton->set_active( bValue );
    }
  }
}

Gnome::Gda::Value DataWidget::get_value() const
{
  const Gtk::Widget* widget = get_data_child_widget();
  const LayoutWidgetField* generic_field_widget = dynamic_cast<const LayoutWidgetField*>(widget);
  if(generic_field_widget)
    return generic_field_widget->get_value();
  else
  {
    const Gtk::CheckButton* checkbutton = dynamic_cast<const Gtk::CheckButton*>(widget);
    if(checkbutton)
    {
      return Gnome::Gda::Value(checkbutton->get_active());
    }
    else
      return Gnome::Gda::Value(); //null.
  }
}

Gtk::Label* DataWidget::get_label()
{
  return &m_label;
}

const Gtk::Label* DataWidget::get_label() const
{
  return &m_label;
}

int DataWidget::get_suitable_width(const sharedptr<const LayoutItem_Field>& field_layout)
{
  int result = 150;

  const Field::glom_field_type field_type = field_layout->get_glom_type();

  Glib::ustring example_text;
  switch(field_type)
  {
    case(Field::TYPE_DATE):
    {
      Glib::Date date(31, Glib::Date::Month(12), 2000);
      example_text = Conversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(date));
      break;
    }
    case(Field::TYPE_TIME):
    {
      Gnome::Gda::Time time = {0, 0, 0, 0};
      time.hour = 24;
      time.minute = 59;
      time.second = 59;
      example_text = Conversions::get_text_for_gda_value(field_type, Gnome::Gda::Value(time));
      break;
    }
    case(Field::TYPE_NUMERIC):
    {
      example_text = "EUR 9999999999";
      break;
    }
    case(Field::TYPE_TEXT):
    case(Field::TYPE_IMAGE): //Give images the same width as text fields, so they will often line up.
    {
      //if(!field_layout->get_text_format_multiline()) //Use the full width for multi-line text.
        example_text = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";
      break;
    }
    default:
    {
      break;
    }
  }


  if(!example_text.empty())
  {
    //Get the width required for this string in the current font:
    Glib::RefPtr<Pango::Layout> refLayout = create_pango_layout(example_text);
    int width = 0;
    int height = 0;
    refLayout->get_pixel_size(width, height);
    result = width;

    //Add a bit more:
    result += 10;
  }

  return result;
}

void DataWidget::set_viewable(bool viewable)
{
  Gtk::Widget* child = get_data_child_widget();
  Gtk::Entry* entry = dynamic_cast<Gtk::Entry*>(child);
  if(entry)
    entry->set_visibility(viewable); //TODO: This is not an ideal way to show non-viewable fields..
  else
  {
    Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(child);
    if(checkbutton)
      checkbutton->set_property("inconsistent", !viewable);
  }
}

void DataWidget::set_editable(bool editable)
{
  Gtk::Widget* child = get_data_child_widget();
  Gtk::Entry* entry = dynamic_cast<Gtk::Entry*>(child);
  if(entry)
    entry->set_editable(editable);
  else
  {
    LayoutWidgetBase* base = dynamic_cast<LayoutWidgetBase*>(child);
    if(base)
      base->set_read_only(!editable);
    else
    {
      Gtk::CheckButton* checkbutton = dynamic_cast<Gtk::CheckButton*>(child);
      if(checkbutton)
        checkbutton->set_sensitive(editable);
    }
  }
}

/*
void DataWidget::setup_menu()
{
  m_refActionGroup->add(m_refContextLayout,
    sigc::mem_fun(*this, &DataWidget::on_menupopup_activate_layout) );

  m_refActionGroup->add(m_refContextLayoutProperties,
    sigc::mem_fun(*this, &DataWidget::on_menupopup_activate_layout_properties) );

  m_refActionGroup->add(m_refContextAddField,
    sigc::bind( sigc::mem_fun(*this, &DataWidget::on_menupopup_add_item), LayoutWidgetBase::TYPE_FIELD ) );

  m_refActionGroup->add(m_refContextAddRelatedRecords,
    sigc::bind( sigc::mem_fun(*this, &DataWidget::on_menupopup_add_item), LayoutWidgetBase::TYPE_PORTAL ) );

  m_refActionGroup->add(m_refContextAddGroup,
    sigc::bind( sigc::mem_fun(*this, &DataWidget::on_menupopup_add_item), LayoutWidgetBase::TYPE_GROUP ) );

  //TODO: This does not work until this widget is in a container in the window:s
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextLayoutProperties); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 
  }

  m_refUIManager = Gtk::UIManager::create();

  m_refUIManager->insert_action_group(m_refActionGroup);

  //TODO: add_accel_group(m_refUIManager->get_accel_group());

  try
  {
    Glib::ustring ui_info = 
        "<ui>"
        "  <popup name='ContextMenu'>"
        "    <menuitem action='ContextLayout'/>"
        "    <menuitem action='ContextLayoutProperties'/>"
        "    <menuitem action='ContextAddField'/>"
        "    <menuitem action='ContextAddRelatedRecords'/>"
        "    <menuitem action='ContextAddGroup'/>"
        "  </popup>"
        "</ui>";

    m_refUIManager->add_ui_from_string(ui_info);
  }
  catch(const Glib::Error& ex)
  {
    std::cerr << "building menus failed: " <<  ex.what();
  }

  //Get the menu:
  m_pMenuPopup = dynamic_cast<Gtk::Menu*>( m_refUIManager->get_widget("/ContextMenu") ); 
  if(!m_pMenuPopup)
    g_warning("menu not found");


  if(pApp)
  {
    const bool sensitive = pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER;
    m_refContextLayout->set_sensitive(sensitive);
    m_refContextAddField->set_sensitive(sensitive);
    m_refContextAddRelatedRecords->set_sensitive(sensitive);
    m_refContextAddGroup->set_sensitive(sensitive);
  }
}
*/

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool DataWidget::on_button_press_event(GdkEventButton *event)
{
  //g_warning("DataWidget::on_button_press_event_popup");

  //Enable/Disable items.
  //We did this earlier, but get_application is more likely to work now:
  App_Glom* pApp = get_application();
  if(pApp)
  {
    pApp->add_developer_action(m_refContextLayout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_refContextAddField);
    pApp->add_developer_action(m_refContextAddRelatedRecords);
    pApp->add_developer_action(m_refContextAddGroup);

    pApp->update_userlevel_ui(); //Update our action's sensitivity. 

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::USERLEVEL_DEVELOPER)
    {
      GdkModifierType mods;
      gdk_window_get_pointer( Gtk::Widget::gobj()->window, 0, 0, &mods );
      if(mods & GDK_BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_pMenuPopup->popup(event->button, event->time);
        return true; //We handled this event.
      }
    }
  }

  return Gtk::EventBox::on_button_press_event(event);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

sharedptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name)
{
  return offer_field_list(table_name, sharedptr<LayoutItem_Field>());
}

sharedptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name, const sharedptr<const LayoutItem_Field>& start_field)
{
  sharedptr<LayoutItem_Field> result;

  Glib::RefPtr<Gnome::Glade::Xml> refXml;

#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_field");
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
    return result;
  }
#else
  std::auto_ptr<Gnome::Glade::XmlError> error;
  refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_field", "", error);
  if(error.get())
  {
    std::cerr << error->what() << std::endl;
    return result;
  }
#endif
  
  Dialog_ChooseField* dialog = 0;
  refXml->get_widget_derived("dialog_choose_field", dialog);

  if(dialog)
  {
    dialog->set_document(get_document(), table_name, start_field);
    dialog->set_transient_for(*get_application());
    int response = dialog->run();
    dialog->hide();
    if(response == Gtk::RESPONSE_OK)
    {
      //Get the chosen field:
      result = dialog->get_field_chosen();
    }

    delete dialog;
  }

  return result;
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
sharedptr<LayoutItem_Field> DataWidget::offer_field_layout(const sharedptr<const LayoutItem_Field>& start_field)
{
  sharedptr<LayoutItem_Field> result;

  try
  {
    Glib::RefPtr<Gnome::Glade::Xml> refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_layout_field_properties");

    Dialog_FieldLayout* dialog = 0;
    refXml->get_widget_derived("dialog_layout_field_properties", dialog);

    if(dialog)
    {
      add_view(dialog); //Give it access to the document.
      dialog->set_field(start_field, m_table_name);
      dialog->set_transient_for(*get_application());
      const int response = dialog->run();
      dialog->hide();
      if(response == Gtk::RESPONSE_OK)
      {
        //Get the chosen field:
        result = dialog->get_field_chosen();
      }

      remove_view(dialog);
      delete dialog;
    }
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
  }

  return result;
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

/*
void DataWidget::on_menupopup_add_item(LayoutWidgetBase::enumType item)
{
  signal_layout_item_added().emit(item);
}
*/

#ifndef GLOM_ENABLE_CLIENT_ONLY
void DataWidget::on_menupopup_activate_layout()
{
  //finish_editing();

  sharedptr<LayoutItem_Field> layoutField = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(layoutField)
  {
    sharedptr<LayoutItem_Field> itemchosen = offer_field_list(m_table_name, layoutField);
    if(itemchosen)
    {
      *layoutField = *itemchosen;
      signal_layout_changed().emit();
    }
  }
}

void DataWidget::on_menupopup_activate_layout_properties()
{
  //finish_editing();

  sharedptr<LayoutItem_Field> layoutField = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
  if(layoutField)
  {
    sharedptr<LayoutItem_Field> itemchosen = offer_field_layout(layoutField);
    if(itemchosen)
    {
      *layoutField = *itemchosen;
      //set_layout_item(itemchosen, m_table_name);

      signal_layout_changed().emit();
    }
  }
}

void DataWidget::on_child_user_requested_layout_properties()
{
  on_menupopup_activate_layout_properties();
}

void DataWidget::on_child_user_requested_layout()
{
  on_menupopup_activate_layout();
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

App_Glom* DataWidget::get_application()
{
  Gtk::Container* pWindow = get_toplevel();
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<App_Glom*>(pWindow);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void DataWidget::on_child_layout_item_added(LayoutWidgetBase::enumType item_type)
{
  signal_layout_item_added().emit(item_type);
}
#endif // !GLOM_ENABLE_CLIENT_ONLY

Gtk::Widget* DataWidget::get_data_child_widget()
{
  return m_child;
}

const Gtk::Widget* DataWidget::get_data_child_widget() const
{
  return m_child;
}
 
 DataWidget::type_signal_open_details_requested DataWidget::signal_open_details_requested()
 {
   return m_signal_open_details_requested;
 }

 void DataWidget::on_button_open_details()
 {
   signal_open_details_requested().emit(get_value());
 }

 void DataWidget::on_button_select_id()
 {
   Gnome::Gda::Value chosen_id;
   bool chosen = offer_related_record_id_find(chosen_id);
   if(chosen)
   {
     set_value(chosen_id);
     m_signal_edited.emit(chosen_id);
   }
 }

void DataWidget::on_button_choose_date()
{
  Glib::RefPtr<Gnome::Glade::Xml> refXml;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_date");
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
    return;
  }
#else
  std::auto_ptr<Gnome::Glade::XmlError> error;
  refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_choose_date", "", error);
  if(error.get())
  {
    std::cerr << error->what() << std::endl;
    return;
  }
#endif

  Dialog_ChooseDate* dialog = 0;
  refXml->get_widget_derived("dialog_choose_date", dialog);

  if(dialog)
  {
    dialog->set_transient_for(*get_application());
    dialog->set_date_chosen(get_value());

    const int response = Glom::Utils::dialog_run_with_help(dialog, "dialog_choose_date");
    dialog->hide();
    if(response == Gtk::RESPONSE_OK)
    {
      //Get the chosen date
      const Gnome::Gda::Value value = dialog->get_date_chosen();
      set_value(value);

      m_signal_edited.emit(value);
    }

    delete dialog;
  }
}

bool DataWidget::offer_related_record_id_find(Gnome::Gda::Value& chosen_id)
{
  bool result = false;

  //Initialize output variable:
  chosen_id = Gnome::Gda::Value();

  Glib::RefPtr<Gnome::Glade::Xml> refXml;
#ifdef GLIBMM_EXCEPTIONS_ENABLED
  try
  {
    refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_find_id");
  }
  catch(const Gnome::Glade::XmlError& ex)
  {
    std::cerr << ex.what() << std::endl;
    return result;
  }
#else
  std::auto_ptr<Gnome::Glade::XmlError> error;
  refXml = Gnome::Glade::Xml::create(GLOM_GLADEDIR "glom.glade", "dialog_find_id", "", error);
  if(error.get())
  {
    std::cerr << error->what() << std::endl;
    return result;
  }
#endif

  Dialog_ChooseID* dialog = 0;
  refXml->get_widget_derived("dialog_find_id", dialog);

  if(dialog)
  {
    //dialog->set_document(get_document(), table_name, field);
    dialog->set_transient_for(*get_application());
    add_view(dialog);

    //Discover the related table, in the relationship that uses this ID field:
    Glib::ustring related_table_name;
    sharedptr<LayoutItem_Field> layoutField = sharedptr<LayoutItem_Field>::cast_dynamic(get_layout_item());
    if(layoutField)
    {
      sharedptr<Relationship> relationship = get_document()->get_field_used_in_relationship_to_one(m_table_name, layoutField->get_name());
      if(relationship)
        related_table_name = relationship->get_to_table();
    }
    else
      g_warning("get_layout_item() was not a LayoutItem_Field");

    dialog->init_db_details(related_table_name);


    int response = dialog->run();
    dialog->hide();
    if(response == Gtk::RESPONSE_OK)
    {
      //Get the chosen field:
      result = dialog->get_id_chosen(chosen_id);
    }

    remove_view(dialog);
    delete dialog;
  }

  return result;
}

} //namespace Glom
