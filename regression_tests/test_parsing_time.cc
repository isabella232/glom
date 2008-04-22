#include <glom/libglom/data_structure/glomconversions.h>

int main(int argc, char* argv[])
{
  Gnome::Gda::init("test", "1.0", argc, argv);

  const Glib::ustring time_text_input = "01:00 PM";
  std::cout << "time_text_input=" << time_text_input << std::endl;

  bool success = false;
  const Gnome::Gda::Value value = 
    Glom::Conversions::parse_value(Glom::Field::TYPE_TIME, time_text_input, success);

  if(!success)
  {
    std::cerr << "Failed: parse_value() failed." << std::endl;
    return -1; //Failed.
  }

  const Gnome::Gda::Time parsed_time = value.get_time();
  //std::cout << "debug: Parsed Time: hour=" <<  parsed_time.hour << ", minute=" <<  parsed_time.minute << ", second=" <<  parsed_time.second << std::endl;

  if(parsed_time.hour != 13)
  {
    std::cerr << "Failed: The parsed hour was " <<  parsed_time.hour << "instead of 13" << std::endl;
    return -1; //Failed.
  }

  if(parsed_time.minute != 0)
  {
    std::cerr << "Failed: The parsed minute was " <<  parsed_time.minute << "instead of 0" << std::endl;
    return -1; //Failed.
  }

  const Glib::ustring time_text_parsed = 
    Glom::Conversions::get_text_for_gda_value(Glom::Field::TYPE_TIME, value);


  std::cout << "time_text_parsed=" << time_text_parsed << std::endl;
  return 0; //Success.

  //This extra check would fail if :00 seconds are added to the text:
  //if(time_text_input == time_text_parsed)
  //   return 0; //Success.
  //else
  //   return -1; //Failed.
}
