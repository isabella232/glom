#include <Python.h>
#if PY_VERSION_HEX >= 0x02040000
# include <datetime.h> /* From Python */
#endif
#include "pygdavalue_conversions.h"

#include <boost/python.hpp>

#include "pygdavalue_conversions.h"
#include <libgdamm/numeric.h>
#include <libgda/gda-blob-op.h>
#include <iostream>

//Python versions before python 2.7 have a compiler error in their PyDateTime_IMPORT macro:
// http://bugs.python.org/issue7463
// so we reimplement the macro for older versions:
#if PY_VERSION_HEX < 0x02070000
#undef PyDateTime_IMPORT
#define PyDateTime_IMPORT \
        PyDateTimeAPI = (PyDateTime_CAPI*) PyCObject_Import((char*)"datetime", \
                                                            (char*)"datetime_CAPI")
#endif //PY_VERSION_HEX

/**
 * pygda_value_from_pyobject:
 * @value: the GValue object to store the converted value in.
 * @obj: the Python object to convert.
 *
 * This function converts a Python object and stores the result in a
 * GValue. If the Python object can't be converted to the
 * type of the GValue, then an error is returned.
 *
 * Returns: true for success.
 */
bool
glom_pygda_value_from_pyobject(GValue* boxed, const boost::python::object& input)
{       
    /* Use an appropriate gda_value_set_*() function.
       We cannot know what GValue type is actually wanted, so
       we must still have the get_*() functions in the python API.
     */

    if(G_IS_VALUE (boxed))
      g_value_unset(boxed);
      
    auto input_c = input.ptr();

    if(input_c == Py_None)
    {
      std::cerr << G_STRFUNC << ": Returning false for Py_None" << std::endl;
      return false;
    }
    
    //We check for bool first, 
    //because bool is derived from int in Python,
    //so PyInt_Check() would also succeed on a bool object.
    //  This comment probably only applies to Python 2,
    //  because Python 3 doesn't seem to have an integer type any more.
    if(PyBool_Check(input_c))
    {
      boost::python::extract<bool> extractor_bool(input);
      if(extractor_bool.check())
      {
        const bool val = extractor_bool;
        g_value_init (boxed, G_TYPE_BOOLEAN);
        g_value_set_boolean (boxed, val);
        return true;
      }
    }
 
#if PY_MAJOR_VERSION < 3
    //Python 3 doesn't seem to have an Integer type.
    if(PyInt_Check(input_c))
    {
      boost::python::extract<int> extractor_int(input);
      if(extractor_int.check())
      {
        const int val = extractor_int;
        g_value_init (boxed, G_TYPE_INT);
        g_value_set_int (boxed, val);
        return true;
      }
    }
#endif

    if(PyLong_Check(input_c))
    {
      boost::python::extract<long> extractor_long(input);
      if(extractor_long.check())
      {
        const long val = extractor_long;
        g_value_init (boxed, G_TYPE_INT);
        g_value_set_int (boxed, val);
        return true;
      }
    }
    
    if(PyFloat_Check(input_c))
    {
      boost::python::extract<double> extractor_double(input);
      if(extractor_double.check())
      {
        const double val = extractor_double;
        g_value_init (boxed, G_TYPE_DOUBLE);
        g_value_set_double (boxed, val);
        return true;
      }
    }
    
    if(PyUnicode_Check(input_c))
    {
      boost::python::extract<std::string> extractor_string(input);
      if(extractor_string.check())
      {
        const std::string str = extractor_string;
        g_value_init(boxed, G_TYPE_STRING);
        g_value_set_string(boxed, str.c_str());
        return true;
      }
    }
    
#if PY_VERSION_HEX >= 0x02040000

    // Note that this sets a local copy of PyDateTimeAPI (in Python's datetime.h
    // header) so it _must_ be repeated and called before any code that use the
    // Python PyDate* macros (!) such as PyDateTime_Check
    PyDateTime_IMPORT; //A macro, needed to use PyDate_Check(), PyDateTime_Check(), etc.
    if(PyDateTimeAPI) //This should have been set but it can fail: https://bugzilla.gnome.org/show_bug.cgi?id=644702
    {
      //TODO: Find some way to do this with boost::python
      if(PyDateTime_Check (input_c))
      {
         GdaTimestamp gda;
         gda.year = PyDateTime_GET_YEAR(input_c);
         gda.month = PyDateTime_GET_MONTH(input_c);
         gda.day = PyDateTime_GET_DAY(input_c);
         gda.hour = PyDateTime_DATE_GET_HOUR(input_c);
         gda.minute = PyDateTime_DATE_GET_MINUTE(input_c);
         gda.second = PyDateTime_DATE_GET_SECOND(input_c);
         gda.timezone = 0;
         gda_value_set_timestamp (boxed, &gda);
         return true;
       } else if(PyDate_Check (input_c))
       {
         GDate *gda = g_date_new_dmy(
           PyDateTime_GET_DAY(input_c),
           (GDateMonth)PyDateTime_GET_MONTH(input_c),
           PyDateTime_GET_YEAR(input_c) );
         g_value_init (boxed, G_TYPE_DATE);
         g_value_set_boxed(boxed, gda);
         g_date_free(gda);
         return true;
       } else if(PyTime_Check (input_c))
       {
         GdaTime gda;
         gda.hour = PyDateTime_TIME_GET_HOUR(input_c);
         gda.minute = PyDateTime_TIME_GET_MINUTE(input_c);
         gda.second = PyDateTime_TIME_GET_SECOND(input_c);
         gda.timezone = 0;
         gda_value_set_time (boxed, &gda);
         return true;
       }
     }
#else
  //std::cout << "DEBUG Dates not supported." << std::endl;
#endif

    PyObject* as_string = PyObject_Repr(input_c);
    boost::python::extract<std::string> extractor_string(as_string);
    const std::string str_as_string = extractor_string;
    Py_XDECREF(as_string);

    std::cerr << G_STRFUNC << ": Unhandled python type: object as string:" <<
      str_as_string << std::endl;
    std::cerr << G_STRFUNC << ": PY_MAJOR_VERSION=" << PY_MAJOR_VERSION << 
      ", PY_VERSION_HEX=" << std::hex << PY_VERSION_HEX << std::endl;
    return false; /* failed. */
}

boost::python::object glom_pygda_value_as_boost_pyobject(const Glib::ValueBase& value)
{
    GValue* boxed = const_cast<GValue*>(value.gobj());
    const auto value_type = G_VALUE_TYPE(boxed);
    boost::python::object ret;

#if PY_VERSION_HEX >= 0x02040000
    if((value_type == G_TYPE_DATE) ||
       (value_type == GDA_TYPE_TIME) ||
       (value_type == GDA_TYPE_TIMESTAMP))
    {
      // Note that this sets a local copy of PyDateTimeAPI (in Python's datetime.h
      // header) so it _must_ be repeated and called before any code that use the
      // Python PyDate* macros (!) such as PyDateTime_Check
      PyDateTime_IMPORT; //A macro, needed to use PyDate_Check(), PyDateTime_Check(), etc.
      g_assert(PyDateTimeAPI); //This should have been set by the PyDateTime_IMPORT macro
    }
#endif

    if(value_type == G_TYPE_INT64) {
        ret = boost::python::object(g_value_get_int64(boxed));
    } else if(value_type == G_TYPE_UINT64) {
        ret = boost::python::object(g_value_get_uint64(boxed));
    } else if(value_type == GDA_TYPE_BINARY) {
        const auto gdabinary = gda_value_get_binary(boxed);
        if(gdabinary)
          ret = boost::python::object((const char*)gdabinary->data); /* TODO: Use the size. TODO: Check for null GdaBinary. */
    } else if(value_type == GDA_TYPE_BLOB) {
        const auto gdablob = gda_value_get_blob (boxed);
        if(gdablob && gdablob->op)
        {
          if(gda_blob_op_read_all(const_cast<GdaBlobOp*>(gdablob->op), const_cast<GdaBlob*>(gdablob)))
          {
            ret = boost::python::object((const char*)gdablob->data.data); /* TODO: Use the size. TODO: Check for null GdaBinary. */
          }
        }
    } else if(value_type == G_TYPE_BOOLEAN) {
        ret = boost::python::object((bool)g_value_get_boolean(boxed));
#if PY_VERSION_HEX >= 0x02040000
    } else if(value_type == G_TYPE_DATE) {

        const auto val = (const GDate*)g_value_get_boxed(boxed);
        if(val)
        {
          //Note that the g_date_get* functions give what we expect, but direct struct field access does not.
          const auto year = g_date_get_year(val);
          const auto month = g_date_get_month(val);
          const auto day = g_date_get_day(val);

          if(!g_date_valid(val))
            std::cerr << G_STRFUNC << ": The GDate is not valid." << std::endl;

          //std::cout << "DEBUG G_TYPE_DATE: year=" << year << ", month=" << month << ", day=" << day << std::endl;
          PyObject* cobject = PyDate_FromDate(year, month, day);
          ret = boost::python::object( (boost::python::handle<>(cobject)) );
        }
#endif
    } else if(value_type == G_TYPE_DOUBLE) {
        ret = boost::python::object(g_value_get_double(boxed));
    } else if(value_type == GDA_TYPE_GEOMETRIC_POINT) {
        const auto val = gda_value_get_geometric_point(boxed);
        if(val)
        {
          PyObject* cobject = Py_BuildValue ("(ii)", val->x, val->y);
          ret = boost::python::object( (boost::python::handle<>(cobject)) );
        }
    } else if(value_type == G_TYPE_INT) {
        ret = boost::python::object(g_value_get_int(boxed));
    } else if(value_type == GDA_TYPE_NUMERIC) {
        const auto val = gda_value_get_numeric(boxed);
        ret = boost::python::object(gda_numeric_get_double((GdaNumeric*)val));
    } else if(value_type == G_TYPE_FLOAT) {
        ret = boost::python::object(g_value_get_float(boxed));
    } else if(value_type == G_TYPE_STRING) {
        const auto val = g_value_get_string(boxed);
        ret = boost::python::object(val);
    } else if(value_type == GDA_TYPE_TIME) {
#if PY_VERSION_HEX >= 0x02040000
        const auto val = gda_value_get_time(boxed);
        if(val)
        {
          PyObject* cobject = PyTime_FromTime(val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GDate::timezone ? */
          ret = boost::python::object( (boost::python::handle<>(cobject)) );
        }
    } else if(value_type == GDA_TYPE_TIMESTAMP) {
        const auto val = gda_value_get_timestamp(boxed);
        if(val)
        {
          PyObject* cobject = PyDateTime_FromDateAndTime(val->year, val->month, val->day, val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GdaTimestamp::timezone ? */
          ret = boost::python::object( (boost::python::handle<>(cobject)) );
        }
#endif
    } else if(value_type == GDA_TYPE_SHORT) {
        ret = boost::python::object(gda_value_get_short(boxed));
    } else if(value_type == GDA_TYPE_USHORT) {
        ret = boost::python::object(gda_value_get_ushort(boxed));
    } else if(value_type == G_TYPE_UINT) {
        ret = boost::python::object(g_value_get_uint(boxed));
    } else if(value_type == GDA_TYPE_NULL) {
        std::cerr << G_STRFUNC << ": Returning Py_None for GDA_TYPE_NULL." << std::endl;
        ret = boost::python::object();
    } else {
      std::cerr << G_STRFUNC << ": Glom: G_VALUE_TYPE() returned unknown type: " << value_type << std::endl;
      std::cerr << G_STRFUNC << "  type name: " << g_type_name(value_type) << std::endl;
    }

    return ret;
}
