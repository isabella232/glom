//#include <Python.h> //Include it before anything else to avoid "_POSIX_C_SOURCE redefined".
//#if PY_VERSION_HEX >= 0x02040000
//# include <datetime.h> /* From Python */
//#endif


#include <glom/libglom/init.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>

int main()
{
  Glom::libglom_init(); //Also initializes python.

  //Py_Initialize();
  //PyDateTime_IMPORT; //A macro, needed to use PyDate_Check(), PyDateTime_Check(), etc.
  //g_assert(PyDateTimeAPI); //This should have been set by the PyDateTime_IMPORT macro.

  const char* calculation = "import datetime;return datetime.date.today();";
  Glom::type_map_fields field_values;
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Execute a python function:
  const Gnome::Gda::Value value = Glom::glom_evaluate_python_function_implementation(
    Glom::Field::TYPE_DATE, calculation, field_values,
    0 /* document */, "" /* table name */,
    Glom::sharedptr<Glom::Field>(), Gnome::Gda::Value(), // primary key details. Not used in this test.
    connection);

  //std::cout << "type=" << g_type_name(value.get_value_type()) << std::endl;

  //Check that the return value is of the expected type:
  g_assert(value.get_value_type() == G_TYPE_DATE);

  //Check that the return value is of the expected value:
  Glib::Date date_current;
  date_current.set_time_current();
  const Glib::Date date_result= value.get_date();
  g_assert(date_current == date_result);

  //std::cout << "value=" << value.to_string() << std::endl;

  return EXIT_SUCCESS;
}
