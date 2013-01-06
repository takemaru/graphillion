#ifndef ILLION_PYSETS_H_
#define ILLION_PYSETS_H_

#include "Sets.h"

typedef struct {
    PyObject_HEAD
    Sets sets;
} PySetsObject;

PyAPI_DATA(PyTypeObject) PySets_Type;

#define PySets_Check(ob) \
    (Py_TYPE(ob) == &PySets_Type || \
    PyType_IsSubtype(Py_TYPE(ob), &PySets_Type))

#endif // ILLION_PYSETS_H_
