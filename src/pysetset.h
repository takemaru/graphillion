#ifndef GRAPHILLION_PYSETSET_H_
#define GRAPHILLION_PYSETSET_H_

#include "graphillion/setset.h"

typedef struct {
  PyObject_HEAD
  graphillion::setset* ss;
} PySetsetObject;

PyAPI_DATA(PyTypeObject) PySetset_Type;

#define PySetset_Check(ob)                              \
  (Py_TYPE(ob) == &PySetset_Type ||                     \
   PyType_IsSubtype(Py_TYPE(ob), &PySetset_Type))

#endif  // GRAPHILLION_PYSETSET_H_
