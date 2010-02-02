#include <glom/libglom/init.h>
#include <glom/python_embed/glom_python.h>
#include <libglom/data_structure/glomconversions.h>

#include <boost/python.hpp>

int main()
{
  Glom::libglom_init(); //Also initializes python.

  const char* calculation = "count = 0\nfor i in range(0, 100): count += i\nreturn count";
  Glom::type_map_fields field_values;
  Glib::RefPtr<Gnome::Gda::Connection> connection;

  //Execute a python function:
  Gnome::Gda::Value value;
  try
  {
    value = Glom::glom_evaluate_python_function_implementation(
      Glom::Field::TYPE_TEXT, calculation, field_values, 
      0 /* document */, "" /* table name */, connection);
  }
  catch(const std::exception& ex)
  {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
  catch(const boost::python::error_already_set& ex)
  {
    std::cerr << "Exception: boost::python::error_already_set" << std::endl;
    return EXIT_FAILURE;
  }


  //std::cout << "type=" << g_type_name(value.get_value_type()) << std::endl;

  //Check that the return value is of the expected type:
  g_assert(value.get_value_type() == GDA_TYPE_NUMERIC);

  //Check that the return value is of the expected value:
  const double numeric = Glom::Conversions::get_double_for_gda_value_numeric(value);
  g_assert(numeric == 4950.0);

  //std::cout << "value=" << value.to_string() << std::endl;

  return EXIT_SUCCESS;
}
