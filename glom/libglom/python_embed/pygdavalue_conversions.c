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
pygda_value_from_pyobject(GValue *boxed, PyObject *input)
{
    /* Use an appropriate gda_value_set_*() function.
       We can not know what GValue type is actually wanted, so
       we must still have the get_*() functions in the python API.
     */

    if (G_IS_VALUE (boxed)) g_value_unset (boxed);
    if (PyString_Check (input)) {
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
         GDate gda;
         gda.year = PyDateTime_GET_YEAR(input);
         gda.month = PyDateTime_GET_MONTH(input);
         gda.day = PyDateTime_GET_DAY(input);
         g_value_init (boxed, G_TYPE_DATE);
         g_value_set_boxed(boxed, &gda);
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

/**
 * pygda_value_as_pyobject:
 * @value: the GValue object.
 * @copy_boxed: true if boxed values should be copied.
 *
 * This function creates/returns a Python wrapper object that
 * represents the GValue passed as an argument.
 *
 * Returns: a PyObject representing the value.
 */
PyObject *
pygda_value_as_pyobject(const GValue *boxed, gboolean copy_boxed)
{
    GType value_type = GDA_TYPE_NULL;
    PyObject* ret = 0;

    value_type = G_VALUE_TYPE ((GValue*)boxed);

#if PY_VERSION_HEX >= 0x02040000
    PyDateTime_IMPORT; /* So we can use PyDate*() functions. */
#endif

    if (value_type == GDA_TYPE_NULL) {
      Py_INCREF (Py_None);
      ret = Py_None;
    } else if (value_type == G_TYPE_INT64) {
        ret = PyLong_FromLong (g_value_get_int64 ((GValue*)boxed));
    } else if (value_type == G_TYPE_UINT64) {
        ret = PyLong_FromLong (g_value_get_uint64 ((GValue*)boxed));
    } else if (value_type == GDA_TYPE_BINARY) {
        const GdaBinary* gdabinary = gda_value_get_binary((GValue*)boxed);
        ret = PyString_FromString ((const char*)gdabinary->data); /* TODO: Use the size. TODO: Check for null GdaBinary. */
    } else if (value_type == GDA_TYPE_BLOB) {
        /* const GdaBlob* val = gda_value_get_blob ((GValue*)boxed); */
        ret = 0; /* TODO: This thing has a whole read/write API. */
    } else if (value_type == G_TYPE_BOOLEAN) {
        ret = PyBool_FromLong (g_value_get_boolean ((GValue*)boxed));
#if PY_VERSION_HEX >= 0x02040000
    } else if (value_type == G_TYPE_DATE) {
        const GDate* val = (const GDate*)g_value_get_boxed ((GValue*)boxed);
        if(val)
          ret = PyDate_FromDate(val->year, val->month, val->day);
#endif
    } else if (value_type == G_TYPE_DOUBLE) {
        ret = PyFloat_FromDouble (g_value_get_double ((GValue*)boxed));
    } else if (value_type == GDA_TYPE_GEOMETRIC_POINT) {
        const GdaGeometricPoint* val = gda_value_get_geometric_point ((GValue*)boxed);
        ret = Py_BuildValue ("(ii)", val->x, val->y);
    } else if (value_type == G_TYPE_INT) {
        ret = PyInt_FromLong (g_value_get_int ((GValue*)boxed));
    } else if (value_type == GDA_TYPE_NUMERIC) {
        const GdaNumeric* val = gda_value_get_numeric ((GValue*)boxed);
        const gchar* number_as_text = val->number; /* Formatted according to the C locale, probably. */
        /* This would need a string _object_: ret = PyFloat_FromString(number_as_text, 0); */
        ret = PyFloat_FromDouble (g_ascii_strtod (number_as_text, NULL));
    } else if (value_type == G_TYPE_FLOAT) {
        ret = PyFloat_FromDouble (g_value_get_float ((GValue*)boxed));
    } else if (value_type == GDA_TYPE_SHORT) {
        ret = PyInt_FromLong (gda_value_get_short ((GValue*)boxed));
    } else if (value_type == G_TYPE_STRING) {
        const gchar* val = g_value_get_string ((GValue*)boxed);
        ret = PyString_FromString (val);
    } else if (value_type == GDA_TYPE_TIME) {
#if PY_VERSION_HEX >= 0x02040000
        const GdaTime* val = gda_value_get_time ((GValue*)boxed);
        ret = PyTime_FromTime(val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GDate::timezone ? */
    } else if (value_type == GDA_TYPE_TIMESTAMP) {
        const GdaTimestamp* val = gda_value_get_timestamp ((GValue*)boxed);
        ret = PyDateTime_FromDateAndTime(val->year, val->month, val->day, val->hour, val->minute, val->second, 0); /* TODO: Should we ignore GdaTimestamp::timezone ? */
#endif
    } else if (value_type == GDA_TYPE_SHORT) {
        ret = PyInt_FromLong (gda_value_get_short ((GValue*)boxed));
    } else if (value_type == GDA_TYPE_USHORT) {
        ret = PyInt_FromLong (gda_value_get_ushort ((GValue*)boxed));
    } else if (value_type == G_TYPE_UINT) {
        ret = PyInt_FromLong (gda_value_get_uint ((GValue*)boxed));
    } else {
      g_warning ("G_VALUE_TYPE() returned unknown type %d", value_type);

      Py_INCREF (Py_None);
      ret = Py_None;
    }

    return ret;
}
