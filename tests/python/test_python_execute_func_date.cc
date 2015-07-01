//#include <Python.h> //Include it before anything else to avoid "_POSIX_C_SOURCE redefined".
//#if PY_VERSION_HEX >= 0x02040000
//# include <datetime.h> /* From Python */
//#endif


#include <glom/libglom/init.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>

void execute_func_with_date_return_value()
{
  const char* calculation = "import datetime;return datetime.date.today();";
  Glom::type_map_fields field_values;
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Execute a python function:
  Glib::ustring error_message;
  const auto value = Glom::glom_evaluate_python_function_implementation(
    Glom::Field::TYPE_DATE, calculation, field_values,
    0 /* document */, "" /* table name */,
    std::shared_ptr<Glom::Field>(), Gnome::Gda::Value(), // primary key details. Not used in this test.
    connection,
    error_message);

  //std::cout << "type=" << g_type_name(value.get_value_type()) << std::endl;

  //Check that there was no python error:
  g_assert(error_message.empty());
  
  //Check that the return value is of the expected type:
  g_assert(value.get_value_type() == G_TYPE_DATE);

  //Check that the return value is of the expected value:
  Glib::Date date_current;
  date_current.set_time_current();
  const Glib::Date date_result= value.get_date();
  g_assert(date_current == date_result);

  //std::cout << "value=" << value.to_string() << std::endl;
}
                            
void execute_func_with_date_input_value()
{
  const char* calculation = "import datetime\n"
                            "return record[\"test_field\"].year";
  Glom::type_map_fields field_values;
  const auto input_date = Glib::Date(11, Glib::Date::MAY, 1973);
  field_values["test_field"] = Gnome::Gda::Value(input_date);
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Execute a python function:
  Glib::ustring error_message;
  const auto value = Glom::glom_evaluate_python_function_implementation(
    Glom::Field::TYPE_NUMERIC, calculation, field_values,
    0 /* document */, "" /* table name */,
    std::shared_ptr<Glom::Field>(), Gnome::Gda::Value(), // primary key details. Not used in this test.
    connection,
    error_message);

  //std::cout << "type=" << g_type_name(value.get_value_type()) << std::endl;

  //Check that there was no python error:
  g_assert(error_message.empty());
  
  //Check that the return value is of the expected type:
  g_assert(value.get_value_type() == GDA_TYPE_NUMERIC);

  //Check that the return value is of the expected value:
  //std::cout << "GdaNumeric number=" << value.get_numeric()->number << std::endl;
  g_assert(value.get_numeric().get_double() == 1973);

  //std::cout << "value=" << value.to_string() << std::endl;
}

/* This would require the extra dateutil python module.
void execute_func_with_date_input_value_relativedelta()
{
  const char* calculation = "from dateutil.relativedelta import relativedelta\n"
                            "import datetime\n"
                            "today = datetime.date.today()\n"
                            "date_of_birth = record[\"test_field\"]\n"
                            "rd = relativedelta(today, date_of_birth)\n"
                            "return rd.year";
  Glom::type_map_fields field_values;
  const auto input_date = Glib::Date(11, Glib::Date::MAY, 1973);
  field_values["test_field"] = Gnome::Gda::Value(input_date);
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Execute a python function:
  const auto value = Glom::glom_evaluate_python_function_implementation(
    Glom::Field::TYPE_NUMERIC, calculation, field_values,
    0, "",
    std::shared_ptr<Glom::Field>(), Gnome::Gda::Value(), // primary key details. Not used in this test.
    connection);

  //std::cout << "type=" << g_type_name(value.get_value_type()) << std::endl;

  //Check that the return value is of the expected type:
  g_assert(value.get_value_type() == GDA_TYPE_NUMERIC);

  //Check that the return value is of the expected value:
  g_assert(value.get_numeric());
  g_assert(value.get_numeric()->number);
  //std::cout << "GdaNumeric number=" << value.get_numeric()->number << std::endl;
  //g_assert(value.get_numeric()->number == std::string("1973"));

  std::cout << "value=" << value.to_string() << std::endl;
}
*/

int main()
{
  Glom::libglom_init(); //Also initializes python.

  execute_func_with_date_return_value();
  execute_func_with_date_input_value();
  //execute_func_with_date_input_value_relativedelta();

  return EXIT_SUCCESS;
}
