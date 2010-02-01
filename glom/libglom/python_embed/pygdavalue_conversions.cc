#include <Python.h>
#if PY_VERSION_HEX >= 0x02040000
# include <datetime.h> /* From Python */
#endif
#include "pygdavalue_conversions.h"


/**
 * pygda_value_from_pyobject:
 * @value: the GValue object to store the converted value in.
 * @obj: the Python object to convert.
 *
 * This function converts a Python object and stores the result in a
 * GValue. If the Python object can't be converted to the
 * type of the GValue, then an error is returned.
 *
 * Returns: 0 on success, -1 on error.
 */
int
glom_pygda_value_from_pyobject(GValue *boxed, PyObject *input)
{
    // We shouldn't need to call PyDateTime_IMPORT again, 
    // after already doing it in libglom_init(),
    // but PyDate_Check crashes (with valgrind warnings) if we don't.
    PyDateTime_IMPORT; //A macro, needed to use PyDate_Check(), PyDateTime_Check(), etc.
    g_assert(PyDateTimeAPI); //This should have been set by the PyDateTime_IMPORT macro.

    /* Use an appropriate gda_value_set_*() function.
       We can not know what GValue type is actually wanted, so
       we must still have the get_*() functions in the python API.
     */

    if (G_IS_VALUE (boxed)) g_value_unset(boxed);
    if (input == Py_None) {
      /* Leave value uninitialized to represent NULL value */
    } else if (PyString_Check (input)) {
      const char* text = PyString_AsString (input);
      g_value_init (boxed, G_TYPE_STRING);
      g_value_set_string (boxed, text);
    } else if (PyInt_Check (input)) {
      g_value_init (boxed, G_TYPE_INT);
      g_value_set_int (boxed, PyInt_AsLong (input));
    } else if (PyLong_Check (input)) {
      g_value_init (boxed, G_TYPE_INT);
      g_value_set_int (boxed, PyInt_AsLong (input));
    } else if (PyFloat_Check (input)) {
      g_value_init (boxed, G_TYPE_DOUBLE);
      g_value_set_double (boxed, PyFloat_AsDouble (input));
    } else if (PyBool_Check (input)) {
      g_value_init (boxed, G_TYPE_BOOLEAN);
      g_value_set_boolean (boxed, (input == Py_True));
#if PY_VERSION_HEX >= 0x02040000
    } else if (PyDateTime_Check (input)) {
         GdaTimestamp gda;
         gda.year = PyDateTime_GET_YEAR(input);
         gda.month = PyDateTime_GET_MONTH(input);
         gda.day = PyDateTime_GET_DAY(input);
         gda.hour = PyDateTime_DATE_GET_HOUR(input);
         gda.minute = PyDateTime_DATE_GET_MINUTE(input);
         gda.second = PyDateTime_DATE_GET_SECOND(input);
         gda.timezone = 0;
         gda_value_set_timestamp (boxed, &gda);
     } else if (PyDate_Check (input)) {
         GDate *gda = g_date_new_dmy(
           PyDateTime_GET_DAY(input),
           (GDateMonth)PyDateTime_GET_MONTH(input),
           PyDateTime_GET_YEAR(input) );
         g_value_init (boxed, G_TYPE_DATE);
         g_value_set_boxed(boxed, gda);
         g_date_free(gda);
     } else if (PyTime_Check (input)) {
         GdaTime gda;
         gda.hour = PyDateTime_TIME_GET_HOUR(input);
         gda.minute = PyDateTime_TIME_GET_MINUTE(input);
         gda.second = PyDateTime_TIME_GET_SECOND(input);
         gda.timezone = 0;
         gda_value_set_time (boxed, &gda);
#endif
    } else {
      g_warning("Unhandled python type.");
      return -1; /* failed. */
    }

    return 0; /* success. */
}

boost::python::object glom_pygda_value_as_boost_pyobject(const Glib::ValueBase& value)
{
    GValue* boxed = const_cast<GValue*>(value.gobj());
    const GType value_type = G_VALUE_TYPE(boxed);
    boost::python::object ret;

#if PY_VERSION_HEX >= 0x02040000
    PyDateTime_IMPORT; /* So we can use PyDate*() functions. */
#endif

    if (value_type == G_TYPE_INT64) {
        ret = boost::python::object(g_value_get_int64(boxed));
    } else if (value_type == G_TYPE_UINT64) {
        ret = boost::python::object(g_value_get_uint64(boxed));
    } else if (value_type == GDA_TYPE_BINARY) {
        const GdaBinary* gdabinary = gda_value_get_binary(boxed);
        ret = boost::python::object((const char*)gdabinary->data); /* TODO: Use the size. TODO: Check for null GdaBinary. */
    } else if (value_type == GDA_TYPE_BLOB) {
        /* const GdaBlob* val = gda_value_get_blob (boxed; */
        /* TODO: This thing has a whole read/write API. */
    } else if (value_type == G_TYPE_BOOLEAN) {
        ret = boost::python::object((bool)g_value_get_boolean(boxed));
#if PY_VERSION_HEX >= 0x02040000
    } else if (value_type == G_TYPE_DATE) {
        /* TODO: 
        const GDate* val = (const GDate*)g_value_get_boxed(boxed);
        if(val)
          ret = PyDate_FromDate(val->year, val->month, val->day);
        */
#endif
    } else if (value_type == G_TYPE_DOUBLE) {
        ret = boost::python::object(g_value_get_double(boxed));
    } else if (value_type == GDA_TYPE_GEOMETRIC_POINT) {
    /*
        const GdaGeometricPoint* val = gda_value_get_geometric_point(boxed;
        ret = Py_BuildValue ("(ii)", val->x, val->y);
    */
    } else if (value_type == G_TYPE_INT) {
        ret = boost::python::object(g_value_get_int(boxed));
    } else if (value_type == GDA_TYPE_NUMERIC) {
        const GdaNumeric* val = gda_value_get_numeric(boxed);
        const gchar* number_as_text = val->number; /* Formatted according to the C locale, probably. */
        /* This would need a string _object_: ret = PyFloat_FromString(number_as_text, 0); */
        ret = boost::python::object(g_ascii_strtod(number_as_text, 0));
    } else if (value_type == G_TYPE_FLOAT) {
        ret = boost::python::object(g_value_get_float(boxed));
    } else if (value_type == GDA_TYPE_SHORT) {
        ret = boost::python::object(gda_value_get_short(boxed));
    } else if (value_type == G_TYPE_STRING) {
        const gchar* val = g_value_get_string(boxed);
        ret = boost::python::object(val);
    } else if (value_type == GDA_TYPE_TIME) {
#if PY_VERSION_HEX >= 0x02040000
        //const GdaTime* val = gda_value_get_time(boxed)
        //ret = PyTime_FromTime(val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GDate::timezone ? */
    } else if (value_type == GDA_TYPE_TIMESTAMP) {
        //const GdaTimestamp* val = gda_value_get_timestamp (boxed;
        //ret = PyDateTime_FromDateAndTime(val->year, val->month, val->day, val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GdaTimestamp::timezone ? */
#endif
    } else if (value_type == GDA_TYPE_SHORT) {
        ret = boost::python::object(gda_value_get_short(boxed));
    } else if (value_type == GDA_TYPE_USHORT) {
        ret = boost::python::object(gda_value_get_ushort(boxed));
    } else if (value_type == G_TYPE_UINT) {
        ret = boost::python::object(g_value_get_uint(boxed));
    } else {
      g_warning ("G_VALUE_TYPE() returned unknown type %" G_GSIZE_FORMAT, value_type);
    }

    return ret;
}
