#include <glom/libglom/init.h>
#include <glom/python_embed/glom_python.h>
#include <iostream>

//Store results from the callbacks and check them later:
Glib::ustring result_table_name_list;
Glib::ustring result_table_name_details;

//This can't be a normal global variable,
//because it would be initialized before we have initialized the glib type system.
static Gnome::Gda::Value& get_result_primary_key_value_details_instance()
{
  static Gnome::Gda::Value result_primary_key_value_details;
  return result_primary_key_value_details;
}

Glib::ustring result_report_name;
bool result_printed_layout = false;
bool result_started_new_record = false;

static void on_script_ui_show_table_list(const Glib::ustring& table_name)
{
  //std::cout << "debug: " << G_STRFUNC << ": table_name=" << table_name << std::endl;
  result_table_name_list = table_name;
}

static void on_script_ui_show_table_details(const Glib::ustring& table_name, const Gnome::Gda::Value& primary_key_value)
{
  //std::cout << "debug: " << G_STRFUNC << ": table_name=" << table_name
  //  << ", primary_key_value=" << primary_key_value.to_string() << std::endl;
  result_table_name_details = table_name;
  get_result_primary_key_value_details_instance() = primary_key_value;
}

static void on_script_ui_print_report(const Glib::ustring& report_name)
{
  result_report_name = report_name;;
}

static void on_script_ui_print_layout()
{
  result_printed_layout = true;
}

static void on_script_ui_start_new_record()
{
  result_started_new_record = true;
}

int main()
{
  Glom::libglom_init(); //Also initializes python.

  const Glib::ustring table_name_input = "sometable";
  const Glib::ustring table_name_details_input = "artists";
  const Gnome::Gda::Value primary_key_value_input(123);
  const Glib::ustring report_name_input = "somereport";

  //Just some code to make sure that the python API exists:
  const Glib::ustring script =
    "table_name = record.table_name;\n"
    "ui.show_table_list(table_name);\n"
    "ui.show_table_details(\"" + table_name_details_input + "\", " + primary_key_value_input.to_string() + ");\n"
    "ui.print_report(\"" + report_name_input + "\");\n"
    "ui.print_layout();\n"
    "ui.start_new_record();\n";
  Glom::type_map_fields field_values;
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  Glom::PythonUICallbacks callbacks;
  callbacks.m_slot_show_table_list =
    &on_script_ui_show_table_list;
  callbacks.m_slot_show_table_details =
    &on_script_ui_show_table_details;
  callbacks.m_slot_print_report =
    &on_script_ui_print_report;
  callbacks.m_slot_print_layout =
    &on_script_ui_print_layout;
  callbacks.m_slot_start_new_record =
    &on_script_ui_start_new_record;

  //Execute a python script:
  Glib::ustring error_message;
  try
  {
    Glom::glom_execute_python_function_implementation(
      script, field_values,
      0 /* document */, table_name_input,
      std::shared_ptr<Glom::Field>(), Gnome::Gda::Value(), // primary key details. Not used in this test.
      connection,
      callbacks,
      error_message);
  }
  catch(const std::exception& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << G_STRFUNC << ": Exception: boost::python::error_already_set\n";
    return EXIT_FAILURE;
  }

  if(!error_message.empty())
  {
    std::cerr << G_STRFUNC << ": Python Error: " << error_message << std::endl;
  }

  g_assert(error_message.empty());

  //Check that the callbacks received the expected values:
  g_assert(result_table_name_list == table_name_input);
  g_assert(result_table_name_details == table_name_details_input);
  g_assert(get_result_primary_key_value_details_instance() == primary_key_value_input);
  g_assert(result_report_name == report_name_input);
  g_assert(result_printed_layout);
  g_assert(result_started_new_record);

  return EXIT_SUCCESS;
}
