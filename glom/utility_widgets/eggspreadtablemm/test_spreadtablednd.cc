/* Glom
 *
 * Copyright (C) 2010 Openismus GmbH
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

#include <glom/utility_widgets/eggspreadtable/eggspreadtablednd.h>
#include <glom/utility_widgets/eggspreadtablemm/eggspreadtabledndmm.h>
#include <gtkmm/window.h>
#include <gtkmm/entry.h>
#include <gtkmm/label.h>
#include <gtkmm/box.h>
#include <gtkmm/button.h>
#include <gtkmm/spinbutton.h>
#include <gtkmm/expander.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/frame.h>
#include <gtkmm/comboboxtext.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/application.h>

static const guint INITIAL_HSPACING = 2;
static const guint INITIAL_VSPACING = 2;
static const guint INITIAL_LINES = 3;
static const Gtk::Align INITIAL_HALIGN = Gtk::ALIGN_FILL;

static Egg::SpreadTableDnd *paper = 0;
static Gtk::ComboBoxText* combo_halign = 0;
static Gtk::ComboBoxText* combo_orientation = 0;
static Gtk::SpinButton* spinbutton_lines = 0;
static Gtk::Align child_halign = INITIAL_HALIGN;
static bool child_accepts_drops = true;
static bool parent_accepts_drops = true;


static void
populate_spread_table_wrappy(Egg::SpreadTableDnd* spread_table)
{
  const gchar *strings[] = {
    "These are", "some wrappy label", "texts", "of various", "lengths.",
    "They should always be", "shown", "consecutively. Except it's",
    "hard to say", "where exactly the", "label", "will wrap", "and where exactly",
    "the actual", "container", "will wrap.", "This label is really really really long !", 
    "Let's add some more", "labels to the",
    "mix. Just to", "make sure we", "got something to work", "with here."
  };

  /* Remove all children first */
  typedef std::vector<Gtk::Widget*> type_vec_widgets;
  type_vec_widgets children = paper->get_children();
  for(const auto& child : children)
  {
    paper->remove_child(*child);
    delete child;
  }

  for(gsize i = 0; i < G_N_ELEMENTS (strings); ++i)
    {
      Gtk::Label* label = Gtk::manage(new Gtk::Label(strings[i]));
      Gtk::Frame* frame  = Gtk::manage(new Gtk::Frame());
      Gtk::EventBox* eventbox = Gtk::manage(new Gtk::EventBox());
      label->show();
      frame->show();
      eventbox->show();

      frame->add(*label);
      eventbox->add(*frame);

      label->set_line_wrap();
      label->set_line_wrap_mode(Pango::WRAP_WORD);
      label->set_width_chars(10);

      frame->set_halign(child_halign);

      spread_table->insert_child(*eventbox, -1);
    }
}

static void
on_combo_orientation_changed()
{
  Gtk::Orientation orientation = (Gtk::Orientation)combo_orientation->get_active_row_number();

  paper->set_orientation(orientation);
}

static void
on_spinbutton_lines_changed()
{
  const auto lines = spinbutton_lines->get_value_as_int();

  paper->set_lines(lines);
}

static void
on_spinbutton_spacing_changed(Gtk::SpinButton* spinbutton, Gtk::Orientation orientation)
{
  const auto state = spinbutton->get_value_as_int();

  if(orientation == Gtk::ORIENTATION_HORIZONTAL)
    paper->set_horizontal_spacing(state);
  else
    paper->set_vertical_spacing(state);
}

static void
on_combo_halign_changed()
{
  child_halign = (Gtk::Align)combo_halign->get_active_row_number();

  populate_spread_table_wrappy(paper);
}

static bool
on_spreadtable_parent_drop_possible(Gtk::Widget* /* child */, bool& drop_possible)
{
  drop_possible = parent_accepts_drops;

  return TRUE; //Handled, instead of using the default behaviour.
}

static bool
on_inner_spreadtable_drop_possible(Gtk::Widget* /* child */, bool& drop_possible)
{
  drop_possible = child_accepts_drops;

  return TRUE; //Handled, instead of using the default behaviour.
}

static void
on_togglebutton_toggled(Gtk::ToggleButton* togglebutton, bool& value)
{
  value = togglebutton->get_active();
}

static Gtk::Window *
create_window()
{
  Gtk::Window* window = new Gtk::Window();
  Gtk::Box* hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 2));
  hbox->show();
  Gtk::Box* vbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 6));
  vbox->show();

  window->set_border_width (8);

  window->add (*hbox);
  hbox->pack_start(*vbox, false, false, 0);

  Gtk::Frame* frame = Gtk::manage(new Gtk::Frame("SpreadTable"));
  frame->show();
  hbox->pack_start(*frame, true, true, 0);

  Gtk::ScrolledWindow* swindow = Gtk::manage(new Gtk::ScrolledWindow());
  swindow->set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
  
  swindow->show();
  frame->add(*swindow);

  paper = 
    Gtk::manage(new Egg::SpreadTableDnd(
      Gtk::ORIENTATION_VERTICAL,
      INITIAL_LINES));

  paper->set_vertical_spacing (INITIAL_VSPACING);
  paper->set_horizontal_spacing (INITIAL_HSPACING);
  paper->show();

  swindow->add(*paper);

  /* Add SpreadTable test control frame */
  Gtk::Expander* expander = Gtk::manage(new Gtk::Expander("SpreadTable controls"));
  expander->set_expanded();
  Gtk::Box* paper_cntl = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 2));
  paper_cntl->show();;
  expander->show();
  expander->add(*paper_cntl);
  vbox->pack_start(*expander, false, false, 0);

  /* Add Orientation control */
  combo_orientation = Gtk::manage(new Gtk::ComboBoxText());
  combo_orientation->append("Horizontal");
  combo_orientation->append("Vertical");
  combo_orientation->set_active(1);
  combo_orientation->show();

  combo_orientation->set_tooltip_text("Set the spread_table orientation");
  paper_cntl->pack_start(*combo_orientation, false, false, 0);

  combo_orientation->signal_changed().connect(
    sigc::ptr_fun(&on_combo_orientation_changed));

  /* Add horizontal/vertical spacing controls */
  hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 2));
  hbox->show();

  Gtk::Widget* label = Gtk::manage(new Gtk::Label("H Spacing"));
  label->show();
  hbox->pack_start(*label, true, true, 0);

  Gtk::SpinButton* spinbutton = Glib::wrap(GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (0, 30, 1)));
  spinbutton->set_value(INITIAL_HSPACING);
  spinbutton->show();

  spinbutton->set_tooltip_text("Set the horizontal spacing between children");
  hbox->pack_start(*spinbutton, false, false, 0);

  spinbutton->signal_changed().connect(
    sigc::bind(
      sigc::ptr_fun(&on_spinbutton_spacing_changed),
      spinbutton, Gtk::ORIENTATION_HORIZONTAL));
  spinbutton->signal_value_changed().connect(
    sigc::bind(
      sigc::ptr_fun(&on_spinbutton_spacing_changed),
      spinbutton, Gtk::ORIENTATION_HORIZONTAL));

  paper_cntl->pack_start(*hbox, false, false, 0);

  hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 2));
  hbox->show();

  label =  Gtk::manage(new Gtk::Label("V Spacing"));
  label->show();
  hbox->pack_start(*label, true, true, 0);

  spinbutton = Glib::wrap(GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (0, 30, 1)));
  spinbutton->set_value(INITIAL_VSPACING);
  spinbutton->show();

  spinbutton->set_tooltip_text("Set the vertical spacing between children");
  hbox->pack_start(*spinbutton, false, false, 0);

  spinbutton->signal_changed().connect(
    sigc::bind(
      sigc::ptr_fun(&on_spinbutton_spacing_changed),
      spinbutton, Gtk::ORIENTATION_VERTICAL));
  spinbutton->signal_value_changed().connect(
    sigc::bind(
      sigc::ptr_fun(&on_spinbutton_spacing_changed),
      spinbutton, Gtk::ORIENTATION_VERTICAL));

  paper_cntl->pack_start(*hbox, false, false, 0);

  /* Add widget-drop-possible controls */
  Gtk::ToggleButton* togglebutton = Gtk::manage(new Gtk::ToggleButton("parent accept drop"));
  togglebutton->show();
  togglebutton->set_active();
  paper_cntl->pack_start(*togglebutton, false, false, 0);
  togglebutton->signal_toggled().connect(
    sigc::bind(
      sigc::ptr_fun(&on_togglebutton_toggled),
      togglebutton, parent_accepts_drops));

  togglebutton = Gtk::manage(new Gtk::ToggleButton("child accept drop"));
  togglebutton->show();
  togglebutton->set_active();
  paper_cntl->pack_start(*togglebutton, false, false, 0);
  togglebutton->signal_toggled().connect(
    sigc::bind(
      sigc::ptr_fun(&on_togglebutton_toggled),
      togglebutton, child_accepts_drops));

  /* Add lines controls */
  hbox = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_HORIZONTAL, 2));
  hbox->show();

  label = Gtk::manage(new Gtk::Label("Lines"));
  label->show();
  hbox->pack_start(*label, true, true, 0);

  spinbutton_lines = Glib::wrap(GTK_SPIN_BUTTON(gtk_spin_button_new_with_range (1, 30, 1)));
  spinbutton_lines->set_value(INITIAL_LINES);
  spinbutton_lines->show();

  spinbutton_lines->set_tooltip_text("Set the horizontal spacing between children");
  hbox->pack_start(*spinbutton_lines, false, false, 0);

  spinbutton_lines->signal_changed().connect(
    sigc::ptr_fun(&on_spinbutton_lines_changed));
  spinbutton_lines->signal_value_changed().connect(
    sigc::ptr_fun(&on_spinbutton_lines_changed));

  paper_cntl->pack_start(*hbox, false, false, 0);


  /* Add test items control frame */
  expander = Gtk::manage(new Gtk::Expander("Test item controls"));
  expander->set_expanded();
  Gtk::Box* items_cntl = Gtk::manage(new Gtk::Box(Gtk::ORIENTATION_VERTICAL, 2));
  items_cntl->show();
  expander->show();
  expander->add(*items_cntl);
  vbox->pack_start(*expander, false, false, 0);

  /* Add child halign control */
  combo_halign = Gtk::manage(new Gtk::ComboBoxText());
  combo_halign->append("Fill");
  combo_halign->append("Start");
  combo_halign->append("End");
  combo_halign->append("Center");
  combo_halign->set_active(INITIAL_HALIGN);
  combo_halign->show();

  combo_halign->set_tooltip_text("Set the children's halign property");
  items_cntl->pack_start(*combo_halign, false, false, 0);

  combo_halign->signal_changed().connect(
    sigc::ptr_fun(&on_combo_halign_changed));

  populate_spread_table_wrappy(paper);

  /* Embed another dnd spread table */
  Egg::SpreadTableDnd* spreadtable_inner = 
    Gtk::manage(new Egg::SpreadTableDnd(
      Gtk::ORIENTATION_VERTICAL,
      INITIAL_LINES));
  
  spreadtable_inner->set_vertical_spacing(INITIAL_VSPACING);
  spreadtable_inner->set_horizontal_spacing(INITIAL_HSPACING);

  frame = Gtk::manage(new Gtk::Frame());
  spreadtable_inner->show();
  frame->show();
  spreadtable_inner->set_size_request(40, 40);
  frame->add(*spreadtable_inner);

  paper->insert_child(*frame, 5);

  window->set_default_size (500, 400);


  /* Signals to control drop allowed or not */
  paper->signal_widget_drop_possible().connect(
    sigc::ptr_fun(&on_spreadtable_parent_drop_possible));
  spreadtable_inner->signal_widget_drop_possible().connect(
    sigc::ptr_fun(&on_inner_spreadtable_drop_possible));

  return window;
}

int
main(int argc, char *argv[])
{
  Glib::RefPtr<Gtk::Application> app = 
    Gtk::Application::create(argc, argv, "org.glom.test_spreadtablednd");

  Gtk::Window* window = create_window();
  return app->run(*window);
}
