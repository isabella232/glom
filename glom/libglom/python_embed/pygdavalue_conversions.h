#include <pygobject.h>
#include <libgda/libgda.h>

#ifndef GLOM_PYGDA_VALUE_CONVERSIONS_H
#define GLOM_PYGDA_VALUE_CONVERSIONS_H

G_BEGIN_DECLS

int
glom_pygda_value_from_pyobject(GValue *boxed, PyObject *input);

PyObject *
glom_pygda_value_as_pyobject(const GValue *value, gboolean copy_boxed);

G_END_DECLS

#endif //GLOM_PYGDA_VALUE_CONVERSIONS_H
