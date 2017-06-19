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

#include "config.h"
#include "datawidget.h"
#include "entry.h"
#include "checkbutton.h"
#include "label.h"
#include <glom/mode_data/datawidget/combo.h>
#include <glom/mode_data/datawidget/combo_as_radio_buttons.h>
#include <glom/mode_data/datawidget/textview.h>
#include <glom/utility_widgets/imageglom.h>
#include <libglom/data_structure/glomconversions.h>
#include <glom/appwindow.h>
#include <glom/mode_design/layout/dialog_choose_field.h>
#include <glom/mode_data/datawidget/dialog_choose_id.h>
#include <glom/mode_data/datawidget/dialog_choose_date.h>
#include <glom/mode_data/datawidget/dialog_new_record.h>
#include <glom/mode_design/layout/layout_item_dialogs/dialog_field_layout.h>
#include <glom/utils_ui.h>
#include <glom/glade_utils.h>
#include <libglom/db_utils.h>
#include <libglom/utils.h>

#include <glibmm/i18n.h>

namespace Glom
{

static DataWidgetChildren::ComboChoices* create_combo_widget_for_field(const std::shared_ptr<LayoutItem_Field>& field)
{
  DataWidgetChildren::ComboChoices* result = nullptr;
  bool as_radio_buttons = false;
  const auto restricted = field->get_formatting_used().get_choices_restricted(as_radio_buttons);
  if(restricted)
  {
    if(as_radio_buttons)
      result = Gtk::manage(new DataWidgetChildren::ComboAsRadioButtons());
    else
      result = Gtk::manage(new DataWidgetChildren::ComboGlom());
  }
  else
    result = Gtk::manage(new DataWidgetChildren::ComboGlom(true /* has_entry */));

  return result;
}

DataWidget::DataWidget(const std::shared_ptr<LayoutItem_Field>& field, const Glib::ustring& table_name, const std::shared_ptr<const Document>& document)
:  m_child(nullptr),
   m_button_go_to_details(nullptr)
{
  const auto glom_type = field->get_glom_type();
  set_layout_item(field, table_name);

  //The GNOME HIG says that labels should have ":" at the end:
  //http://library.gnome.org/devel/hig-book/stable/design-text-labels.html.en
  const auto title = Glib::ustring::compose(_("%1:"), item_get_title_or_name(field));

  m_child = nullptr;
  LayoutWidgetField* pFieldWidget = nullptr;
  if(glom_type == Field::glom_field_type::BOOLEAN)
  {
    auto checkbutton = Gtk::manage( new DataWidgetChildren::CheckButton() );
    checkbutton->show();
    checkbutton->signal_toggled().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

    //TODO: entry->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout );

    m_child = checkbutton;
    pFieldWidget = checkbutton;

    m_label.set_label(title);
    m_label.set_halign(Gtk::Align::START);
    m_label.show();
  }
  else if(glom_type == Field::glom_field_type::IMAGE)
  {
    auto image = Gtk::manage( new ImageGlom() );
    image->set_size_request(200, 200);
    //auto image = Gtk::manage( new Gtk::Image("/home/murrayc/gnome-small.jpg") );
    image->show();
    //TODO: Respond to double-click: checkbutton->signal_toggled().connect( sigc::mem_fun(*this, &DataWidget::on_widget_edited)  );

    //TODO: entry->signal_user_requested_layout().connect( sigc::mem_fun(*this, &DataWidget::on_child_user_requested_layout );

    m_child = image;
    pFieldWidget = image;

    m_label.set_label(title);
    m_label.set_halign(Gtk::Align::START);
    m_label.show();
  }
  else
  {
    m_label.set_label(title);
    m_label.set_halign(Gtk::Align::START);
    m_label.show();

    //Use a Combo if there is a drop-down of choices (A "value list"), else an Entry:
    if(field->get_formatting_used().get_has_choices())
    {
      auto combo = create_combo_widget_for_field(field);

      if(field->get_formatting_used().get_has_custom_choices())
      {
        //set_choices_fixed() needs this, for the numeric layout:
        combo->set_layout_item( get_layout_item(), table_name);

	const Formatting& formatting = field->get_formatting_used();
	bool as_radio_buttons = false; //Ignored;
        combo->set_choices_fixed( formatting.get_choices_custom(), formatting.get_choices_restricted(as_radio_buttons));
      }
      else if(field->get_formatting_used().get_has_related_choices())
      {
        combo = create_combo_widget_for_field(field);
        combo->set_layout_item( get_layout_item(), table_name);

        combo->set_choices_related(document, *field, Gnome::Gda::Value() /* no ID means show all related records */);
      }
      else
      {
        std::cerr << G_STRFUNC << ": Unexpected choice type.\n";
      }

      pFieldWidget = combo;
    }
    else
    {
      if((glom_type == Field::glom_field_type::TEXT) && (field->get_formatting_used().get_text_format_multiline()))
      {
        auto textview = Gtk::manage(new DataWidgetChildren::TextView(glom_type));
        pFieldWidget = textview;
      }
      else  //DATE, NUMBER, etc.
      {
        auto entry = Gtk::manage(new DataWidgetChildren::Entry(glom_type));
        pFieldWidget = entry;
      }

      if(pFieldWidget)
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
    set_child_size_by_field(field);
  }

  if(m_child)
  {
    //Use the text formatting:
    apply_formatting(*m_child, *field);

    bool child_added = false; //Don't use an extra container unless necessary.

    //Update the field details from the document:
    field->set_full_field_details(
      document->get_field(field->get_table_used(table_name), field->get_name()) ); //Otherwise get_primary_key() returns false always.

    std::shared_ptr<Relationship> field_used_in_relationship_to_one;
    const bool add_open_button =
       DbUtils::layout_field_should_have_navigation(table_name, field, document,
         field_used_in_relationship_to_one);

    Gtk::Box* hbox_parent = nullptr; //Only used if there are extra widgets.

    const bool with_extra_widgets = field_used_in_relationship_to_one || add_open_button || (glom_type == Field::glom_field_type::DATE);
    if(with_extra_widgets)
    {
      hbox_parent = Gtk::manage( new Gtk::Box(Gtk::Orientation::HORIZONTAL) ); //We put the child (and any extra stuff) in this:
      hbox_parent->set_spacing(Utils::to_utype(UiUtils::DefaultSpacings::SMALL));

      hbox_parent->pack_start(*m_child);
      hbox_parent->show();
      add(*hbox_parent);

      child_added = true;
    }

    if(glom_type == Field::glom_field_type::DATE)
    {
      //Let the user choose a date from a calendar dialog:
      auto button_date = Gtk::manage(new Gtk::Button(_("..."))); //TODO: A better label/icon for "Choose Date".
      button_date->set_tooltip_text(_("Choose a date from an on-screen calendar."));
      button_date->show();
      hbox_parent->pack_start(*button_date, Gtk::PackOptions::SHRINK);
      button_date->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_choose_date));
    }

    if(hbox_parent && add_open_button)
    {
      //Add a button for related record navigation:
      m_button_go_to_details = Gtk::manage(new Gtk::Button(_("_Open"), true));
      m_button_go_to_details->set_tooltip_text(_("Open the record identified by this ID, in the other table."));
      hbox_parent->pack_start(*m_button_go_to_details, Gtk::PackOptions::SHRINK);
      m_button_go_to_details->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_open_details));

      //Add an additional button to make it easier to choose an ID for this field.
      //Don't add this for simple related primary key fields, because they
      //can generally not be edited via another table's layout.
      if(field_used_in_relationship_to_one)
      {
        auto button_select = Gtk::manage(new Gtk::Button(_("_Find"), true));
        button_select->set_tooltip_text(_("Enter search criteria to identify records in the other table, to choose an ID for this field."));
        hbox_parent->pack_start(*button_select, Gtk::PackOptions::SHRINK);
        button_select->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_select_id));

        auto button_new = Gtk::manage(new Gtk::Button(_("_New"), true));
        button_new->set_tooltip_text(_("Enter details for a new record in the other table, then use its ID for this field."));
        hbox_parent->pack_start(*button_new, Gtk::PackOptions::SHRINK);
        button_new->signal_clicked().connect(sigc::mem_fun(*this, &DataWidget::on_button_new_id));
      }
    }

    if(!child_added)
      add(*m_child);
  }

#ifndef GLOM_ENABLE_CLIENT_ONLY
  setup_menu(this);
#endif // GLOM_ENABLE_CLIENT_ONLY
}

void DataWidget::on_widget_edited()
{
  m_signal_edited.emit(get_value());
  update_go_to_details_button_sensitivity();
}

DataWidget::type_signal_edited DataWidget::signal_edited()
{
  return m_signal_edited;
}

DataWidget::type_signal_choices_changed DataWidget::signal_choices_changed()
{
  return m_signal_choices_changed;
}

void DataWidget::set_value(const Gnome::Gda::Value& value)
{
  auto widget = get_data_child_widget();
  auto generic_field_widget = dynamic_cast<LayoutWidgetField*>(widget);
  if(generic_field_widget)
  {
    //if(generic_field_widget->get_layout_item())
    //  std::cout << "debug: " << G_STRFUNC << ": generic_field_widget->get_layout_item()->get_name()=" << generic_field_widget->get_layout_item()->get_name() << std::endl;

    generic_field_widget->set_value(value);
  }
  else
  {
    auto checkbutton = dynamic_cast<Gtk::CheckButton*>(widget);
    if(checkbutton)
    {
      bool bool_value = false;
      if(!value.is_null() && value.get_value_type() == G_TYPE_BOOLEAN)
        bool_value = value.get_boolean();

      checkbutton->set_active( bool_value );
    }
  }

  update_go_to_details_button_sensitivity();
}

void DataWidget::update_go_to_details_button_sensitivity()
{
  //If there is a Go-To-Details "Open" button, only enable it if there is
  //an ID:
  if(m_button_go_to_details)
  {
    const auto value = get_value();
    const auto enabled = !Conversions::value_is_empty(value);
    m_button_go_to_details->set_sensitive(enabled);
  }
}

Gnome::Gda::Value DataWidget::get_value() const
{
  const auto widget = get_data_child_widget();
  const auto generic_field_widget = dynamic_cast<const LayoutWidgetField*>(widget);
  if(generic_field_widget)
    return generic_field_widget->get_value();
  else
  {
    const auto checkbutton = dynamic_cast<const Gtk::CheckButton*>(widget);
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

void DataWidget::set_child_size_by_field(const std::shared_ptr<const LayoutItem_Field>& field)
{
  const auto glom_type = field->get_glom_type();
  int width = get_suitable_width(field);

  if(glom_type == Field::glom_field_type::IMAGE) //GtkImage widgets default to no size (invisible) if they are empty.
    m_child->set_size_request(width, width);
  else
  {
    int height = -1; //auto.
    if((glom_type == Field::glom_field_type::TEXT) && (field->get_formatting_used().get_text_format_multiline()))
    {
      int example_width = 0;
      int example_height = 0;
      auto refLayout = create_pango_layout("example"); //TODO: Use different text, according to the current locale, or allow the user to choose an example?
      refLayout->get_pixel_size(example_width, example_height);

      if(example_height > 0)
        height = example_height * field->get_formatting_used().get_text_format_multiline_height_lines();
    }

    m_child->set_size_request(width, height);
  }
}

int DataWidget::get_suitable_width(const std::shared_ptr<const LayoutItem_Field>& field_layout)
{
  return UiUtils::get_suitable_field_width_for_widget(*this, field_layout);
}

void DataWidget::set_viewable(bool viewable)
{
  auto child = get_data_child_widget();
  auto entry = dynamic_cast<Gtk::Entry*>(child);
  if(entry)
    entry->set_visibility(viewable); //TODO: This is not an ideal way to show non-viewable fields..
  else
  {
    auto checkbutton = dynamic_cast<Gtk::CheckButton*>(child);
    if(checkbutton)
      checkbutton->property_inconsistent() = !viewable;
  }
}

void DataWidget::set_editable(bool editable)
{
  auto child = get_data_child_widget();
  auto gtkeditable = dynamic_cast<Gtk::Editable*>(child);
  if(gtkeditable)
  {
    gtkeditable->set_editable(editable);
    return;
  }

  //GtkTextView does not implement GtkEditable:
  //See https://bugzilla.gnome.org/show_bug.cgi?id=667008
  //and our TextView class actually derives from ScrolledView anyway,
  //and out LayoutWidgetBase::set_read_only() override takes care of it instead.
  //But let's leave this here just in case:
  auto textview =
    dynamic_cast<Gtk::TextView*>(child);
  if(textview)
  {
    textview->set_editable(editable);
    return;
  }

  auto base = dynamic_cast<LayoutWidgetBase*>(child);
  if(base)
  {
    base->set_read_only(!editable);
    return;
  }

  auto checkbutton = dynamic_cast<Gtk::CheckButton*>(child);
  if(checkbutton)
    checkbutton->set_sensitive(editable);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
bool DataWidget::on_button_press_event(Gdk::EventButton& button_event)
{
  //g_warning("DataWidget::on_button_press_event_popup");

  //Enable/Disable items.
  //We did this earlier, but get_appwindow is more likely to work now:
  auto pApp = get_appwindow();
  if(pApp)
  {
    //TODO: Avoid doing this multiple times:
    pApp->add_developer_action(m_context_layout); //So that it can be disabled when not in developer mode.
    pApp->add_developer_action(m_context_add_field);
    pApp->add_developer_action(m_context_add_related_records);
    pApp->add_developer_action(m_context_add_group);

    pApp->update_userlevel_ui(); //Update our action's sensitivity.

    //Only show this popup in developer mode, so operators still see the default GtkEntry context menu.
    //TODO: It would be better to add it somehow to the standard context menu.
    if(pApp->get_userlevel() == AppState::userlevels::DEVELOPER)
    {
      auto gdkwindow = get_window();
      Gdk::ModifierType mods;
      int x = 0;
      int y = 0;
      gdkwindow->get_device_position(button_event.get_device(), x, y, mods);
      if((mods & Gdk::ModifierType::BUTTON3_MASK) == Gdk::ModifierType::BUTTON3_MASK)
      {
        //Give user choices of actions on this item:
        m_menu_popup->popup(button_event.get_button(), button_event.get_time());
        return true; //We handled this event.
      }
    }
  }

  return Gtk::EventBox::on_button_press_event(button_event);
}

std::shared_ptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name)
{
  return offer_field_list(table_name, std::shared_ptr<LayoutItem_Field>());
}

std::shared_ptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& start_field)
{
  return offer_field_list(table_name, start_field, get_document(), get_appwindow());
}

std::shared_ptr<LayoutItem_Field> DataWidget::offer_field_list(const Glib::ustring& table_name, const std::shared_ptr<const LayoutItem_Field>& start_field, const std::shared_ptr<Document>& document, AppWindow* app)
{
  std::shared_ptr<LayoutItem_Field> result;

  Dialog_ChooseField* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);

  if(dialog)
  {
    dialog->set_document(document, table_name, start_field);
    if(app)
      dialog->set_transient_for(*app);

    const auto response = dialog->run();
    dialog->hide();
    if(response == Gtk::ResponseType::OK)
    {
      //Get the chosen field:
      result = dialog->get_field_chosen();
    }

    delete dialog;
  }

  return result;
}

std::shared_ptr<LayoutItem_Field> DataWidget::offer_field_layout(const std::shared_ptr<const LayoutItem_Field>& start_field)
{
  std::shared_ptr<LayoutItem_Field> result;

  Dialog_FieldLayout* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);
  if(!dialog) //Unlikely and it already warns on stderr.
    return result;

  add_view(dialog); //Give it access to the document.
  dialog->set_field(start_field, m_table_name);

  auto parent = get_appwindow();
  if(parent)
    dialog->set_transient_for(*parent);

  const auto response = dialog->run();
  dialog->hide();
  if(response == Gtk::ResponseType::OK)
  {
    //Get the chosen field:
    result = dialog->get_field_chosen();
  }

  remove_view(dialog);
  delete dialog;

  return result;
}

void DataWidget::on_menupopup_activate_layout()
{
  //finish_editing();

  auto layoutField = std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
  if(layoutField)
  {
    auto itemchosen = offer_field_list(m_table_name, layoutField);
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

  auto layoutField = std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
  if(layoutField)
  {
    auto itemchosen = offer_field_layout(layoutField);
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

AppWindow* DataWidget::get_appwindow() const
{
  auto pWindow = const_cast<Gtk::Container*>(get_toplevel());
  //TODO: This only works when the child widget is already in its parent.

  return dynamic_cast<AppWindow*>(pWindow);
}

#ifndef GLOM_ENABLE_CLIENT_ONLY
void DataWidget::on_child_layout_item_added(LayoutWidgetBase::enumType item_type)
{
  //Tell the parent widget that this widget wants to add an item:
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

void DataWidget::on_button_new_id()
{
  Gnome::Gda::Value chosen_id;
  const auto chosen = offer_related_record_id_new(chosen_id);
  if(!chosen)
    return;

  //Update the choices, if any, to show the new related record:
  m_signal_choices_changed.emit();

  set_value(chosen_id);
  m_signal_edited.emit(chosen_id);
}

void DataWidget::on_button_choose_date()
{
  DataWidgetChildren::Dialog_ChooseDate* dialog = nullptr;
  Utils::get_glade_widget_derived_with_warning(dialog);

  if(dialog)
  {
    auto parent = get_appwindow(); //This doesn't work (and would be wrong) when the widget is in a Field Definitions dialog.
    if(parent)
      dialog->set_transient_for(*parent);
    dialog->set_date_chosen(get_value());

    const auto response = Glom::UiUtils::dialog_run_with_help(dialog);
    dialog->hide();
    if(response == Gtk::ResponseType::OK)
    {
      //Get the chosen date
      const auto value = dialog->get_date_chosen();
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

  DataWidgetChildren::Dialog_ChooseID* dialog = nullptr;
  Glom::Utils::get_glade_widget_derived_with_warning(dialog);

  if(dialog)
  {
    //dialog->set_document(get_document(), table_name, field);
    auto parent = get_appwindow();
    if(parent)
      dialog->set_transient_for(*parent);
    add_view(dialog);

    //Discover the related table, in the relationship that uses this ID field:
    Glib::ustring related_table_name;
    auto layoutField = std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
    if(layoutField)
    {
      auto relationship = get_document()->get_field_used_in_relationship_to_one(m_table_name, layoutField);
      if(relationship)
        related_table_name = relationship->get_to_table();
    }
    else
      g_warning("get_layout_item() was not a LayoutItem_Field");

    dialog->init_db_details(related_table_name, Base_DB::get_active_layout_platform(get_document()));


    const auto response = dialog->run();
    dialog->hide();
    if(response == Gtk::ResponseType::OK)
    {
      //Get the chosen field:
      result = dialog->get_id_chosen(chosen_id);
    }

    remove_view(dialog);
    delete dialog;
  }

  return result;
}


bool DataWidget::offer_related_record_id_new(Gnome::Gda::Value& chosen_id)
{
  bool result = false;

  //Initialize output variable:
  chosen_id = Gnome::Gda::Value();

  DataWidgetChildren::Dialog_NewRecord* dialog = nullptr;
  Glom::Utils::get_glade_widget_derived_with_warning(dialog);

  if(dialog)
  {
    //dialog->set_document(get_document(), table_name, field);
    auto parent = get_appwindow();
    if(parent)
      dialog->set_transient_for(*parent);
    add_view(dialog);

    //Discover the related table, in the relationship that uses this ID field:
    Glib::ustring related_table_name;
    auto layoutField = std::dynamic_pointer_cast<LayoutItem_Field>(get_layout_item());
    if(layoutField)
    {
      auto relationship = get_document()->get_field_used_in_relationship_to_one(m_table_name, layoutField);
      if(relationship)
        related_table_name = relationship->get_to_table();
    }
    else
      g_warning("get_layout_item() was not a LayoutItem_Field");

    dialog->init_db_details(related_table_name, Base_DB::get_active_layout_platform(get_document()));


    const auto response = dialog->run();
    dialog->hide();
    if(response == Gtk::ResponseType::OK)
    {
      //Get the chosen ID:
      result = dialog->get_id_chosen(chosen_id);
    }

    remove_view(dialog);
    delete dialog;
  }

  return result;
}

} //namespace Glom
