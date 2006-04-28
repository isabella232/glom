#include "pygdavalue_conversions.h"
#include <datetime.h> /* From Python */

/**
 * pygda_value_from_pyobject:
 * @value: the GdaValue object to store the converted value in.
 * @obj: the Python object to convert.
 *
 * This function converts a Python object and stores the result in a
 * GdaValue. If the Python object can't be converted to the
 * type of the GdaValue, then an error is returned.
 *
 * Returns: 0 on success, -1 on error.
 */
int
pygda_value_from_pyobject(GdaValue *boxed, PyObject *input)
{
    /* Use an appropriate gda_value_set_*() function.
       We cannot know what GdaValue type is actually wanted, so
       we must still have the get_*() functions in the python API.
     */
    if (PyString_Check (input)) {
      const char* text = PyString_AsString (input);
      gda_value_set_string (boxed, text);
    } else if (PyInt_Check (input)) {
      gda_value_set_integer (boxed, PyInt_AsLong (input));
    } else if (PyLong_Check (input)) {
      gda_value_set_integer (boxed, PyInt_AsLong (input));
    } else if (PyFloat_Check (input)) {
      gda_value_set_double (boxed, PyFloat_AsDouble (input));
    } else if (PyBool_Check (input)) {
      gda_value_set_boolean (boxed, (input == Py_True));
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
         GdaDate gda;
         gda.year = PyDateTime_GET_YEAR(input);
         gda.month = PyDateTime_GET_MONTH(input);
         gda.day = PyDateTime_GET_DAY(input);
         gda_value_set_date(boxed, &gda);
     } else if (PyTime_Check (input)) {
         GdaTime gda;
         gda.hour = PyDateTime_TIME_GET_HOUR(input);
         gda.minute = PyDateTime_TIME_GET_MINUTE(input);
         gda.second = PyDateTime_TIME_GET_SECOND(input);
         gda.timezone = 0;
         gda_value_set_time (boxed, &gda);
    } else {
      g_warning("Unhandled python type.");
      return -1; //failed.
    }

    return 0; //success.
}

/**
 * pygda_value_as_pyobject:
 * @value: the GdaValue object.
 * @copy_boxed: true if boxed values should be copied.
 *
 * This function creates/returns a Python wrapper object that
 * represents the GdaValue passed as an argument.
 *
 * Returns: a PyObject representing the value.
 */
PyObject *
pygda_value_as_pyobject(const GdaValue *boxed, gboolean copy_boxed)
{
    GdaValueType value_type = GDA_VALUE_TYPE_UNKNOWN;
    PyObject* ret = 0;

    value_type = gda_value_get_type ((GdaValue*)boxed);

    PyDateTime_IMPORT; /* So we can use PyDate*() functions. */

    if (value_type == GDA_VALUE_TYPE_NULL) {
      Py_INCREF (Py_None);
      ret = Py_None;
    } else if (value_type == GDA_VALUE_TYPE_BIGINT) {
        ret = PyLong_FromLong (gda_value_get_bigint ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_BIGUINT) {
        ret = PyLong_FromLong (gda_value_get_biguint ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_BINARY) {
        glong size = 0;
        const gchar* val = (const gchar*)gda_value_get_binary ((GdaValue*)boxed, &size);
        ret = PyString_FromString (val); //TODO: Use the size.
    } else if (value_type == GDA_VALUE_TYPE_BLOB) {
        /* const GdaBlob* val = gda_value_get_blob ((GdaValue*)boxed); */
        ret = 0; /* TODO: This thing has a whole read/write API. */
    } else if (value_type == GDA_VALUE_TYPE_BOOLEAN) {
        ret = PyBool_FromLong (gda_value_get_boolean ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_DATE) {
        const GdaDate* val = gda_value_get_date ((GdaValue*)boxed);
        if(val)
          ret = PyDate_FromDate(val->year, val->month, val->day);
    } else if (value_type == GDA_VALUE_TYPE_DOUBLE) {
        ret = PyFloat_FromDouble (gda_value_get_double ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_GEOMETRIC_POINT) {
        const GdaGeometricPoint* val = gda_value_get_geometric_point ((GdaValue*)boxed);
        ret = Py_BuildValue ("(ii)", val->x, val->y);
    } else if (value_type == GDA_VALUE_TYPE_INTEGER) {
        ret = PyInt_FromLong (gda_value_get_integer ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_MONEY) {
        const GdaMoney* val = gda_value_get_money ((GdaValue*)boxed);
        ret = PyFloat_FromDouble(val->amount); /* TODO: We ignore the currency. */
    } else if (value_type == GDA_VALUE_TYPE_NUMERIC) {
        const GdaNumeric* val = gda_value_get_numeric ((GdaValue*)boxed);
        const gchar* number_as_text = val->number; /* Formatted according to the C locale, probably. */
        /* This would need a string _object_: ret = PyFloat_FromString(number_as_text, 0); */
        ret = PyFloat_FromDouble (PyOS_ascii_strtod (number_as_text, 0));
    } else if (value_type == GDA_VALUE_TYPE_SINGLE) {
        ret = PyFloat_FromDouble (gda_value_get_single ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_SMALLINT) {
        ret = PyInt_FromLong (gda_value_get_smallint ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_STRING) {
        const gchar* val = gda_value_get_string ((GdaValue*)boxed);
        ret = PyString_FromString (val);
    } else if (value_type == GDA_VALUE_TYPE_TIME) {
        const GdaTime* val = gda_value_get_time ((GdaValue*)boxed);
        ret = PyTime_FromTime(val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GdaDate::timezone ? */
    } else if (value_type == GDA_VALUE_TYPE_TIMESTAMP) {
        const GdaTimestamp* val = gda_value_get_timestamp ((GdaValue*)boxed);
        ret = PyDateTime_FromDateAndTime(val->year, val->month, val->day, val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GdaTimestamp::timezone ? */
    } else if (value_type == GDA_VALUE_TYPE_TINYINT) {
        ret = PyInt_FromLong (gda_value_get_tinyint ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_TINYUINT) {
        ret = PyInt_FromLong (gda_value_get_tinyuint ((GdaValue*)boxed));
    } else if (value_type == GDA_VALUE_TYPE_UINTEGER) {
        ret = PyInt_FromLong (gda_value_get_uinteger ((GdaValue*)boxed));
    } else {
      g_warning ("gda_value_get_type() returned unknown type %d", value_type);

      Py_INCREF (Py_None);
      ret = Py_None;
    }

    return ret;
}
