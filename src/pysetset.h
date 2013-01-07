#ifndef ILLION_PYSETSET_H_
#define ILLION_PYSETSET_H_

#include "illion/setset.h"

typedef struct {
  PyObject_HEAD
  illion::setset* ss;
} PySetsetObject;

PyAPI_DATA(PyTypeObject) PySetset_Type;

#define PySetset_Check(ob)                              \
  (Py_TYPE(ob) == &PySetset_Type ||                     \
   PyType_IsSubtype(Py_TYPE(ob), &PySetset_Type))

#endif  // ILLION_PYSETSET_H_
