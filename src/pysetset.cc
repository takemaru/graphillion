#include <Python.h>
#include "structmember.h"

#include "pysetset.h"

#include <cstdlib>
#include <climits>

#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

using graphillion::setset;
using std::map;
using std::set;
using std::string;
using std::stringstream;
using std::vector;

#define CHECK_OR_ERROR(obj, check, name, ret)         \
  do {                                                \
    if (!check(obj)) {                                \
      PyErr_SetString(PyExc_TypeError, "not " name);  \
      return (ret);                                   \
    }                                                 \
  } while (0) ;

#define CHECK_SETSET_OR_ERROR(obj)                              \
  CHECK_OR_ERROR(obj, PySetset_Check, "setset", NULL);

#define RETURN_NEW_SETSET(self, expr)                         \
  do {                                                        \
    PySetsetObject* _ret = reinterpret_cast<PySetsetObject*>( \
        (self)->ob_type->tp_alloc((self)->ob_type, 0));       \
    _ret->ss = new setset(expr);                              \
    return reinterpret_cast<PyObject*>(_ret);                 \
  } while (0);


#define RETURN_NEW_SETSET2(self, other, _other, expr)                   \
  do {                                                                  \
    PySetsetObject* (_other) = reinterpret_cast<PySetsetObject*>(other); \
    PySetsetObject* _ret = reinterpret_cast<PySetsetObject*>(           \
        (self)->ob_type->tp_alloc((self)->ob_type, 0));                 \
    if (_ret == NULL) return NULL;                                      \
    _ret->ss = new setset(expr);                                        \
    return reinterpret_cast<PyObject*>(_ret);                           \
  } while (0);

#define RETURN_SELF_SETSET(self, other, _other, expr)                  \
  do {                                                                 \
    PySetsetObject* _other = reinterpret_cast<PySetsetObject*>(other); \
    (expr);                                                            \
    Py_INCREF(self);                                                   \
    return reinterpret_cast<PyObject*>(self);                          \
  } while (0);

#define RETURN_TRUE_IF(self, other, _other, expr)                       \
  do {                                                                  \
    PySetsetObject* (_other) = reinterpret_cast<PySetsetObject*>(other); \
    if (expr) Py_RETURN_TRUE;                                           \
    else      Py_RETURN_FALSE;                                          \
  } while (0);

static PyObject* setset_build_set(const set<int>& s) {
  PyObject* so = PySet_New(NULL);
  for (set<int>::const_iterator e = s.begin(); e != s.end(); ++e) {
    PyObject* eo = PyInt_FromLong(*e);
    if (eo == NULL) {
      PyErr_SetString(PyExc_TypeError, "not int set");
      Py_DECREF(eo);
      return NULL;
    }
    if (PySet_Add(so, eo) == -1) {
      PyErr_SetString(PyExc_RuntimeError, "can't add elements to a set");
      Py_DECREF(eo);
      return NULL;
    }
    Py_DECREF(eo); // TODO: no Py_DECREF required to obj of PyInt_FromLong?
  }
  return so;
}

static int setset_parse_set(PyObject* so, set<int>* s) {
  assert(s != NULL);
  PyObject* i = PyObject_GetIter(so);
  if (i == NULL) return -1;
  PyObject* eo;
  while ((eo = PyIter_Next(i))) {
    if (!PyInt_Check(eo)) {
      Py_DECREF(eo);
      PyErr_SetString(PyExc_TypeError, "not int set");
      return -1;
    }
    s->insert(PyInt_AsLong(eo));
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  return 0;
}

static int setset_parse_map(PyObject* dict_obj, map<string, vector<int> >* m) {
  assert(m != NULL);
  PyObject* key_obj;
  PyObject* lo;
  Py_ssize_t pos = 0;
  while (PyDict_Next(dict_obj, &pos, &key_obj, &lo)) {
    if (!PyString_Check(key_obj)) {
      PyErr_SetString(PyExc_TypeError, "invalid argument");
      return -1;
    }
    string key = PyString_AsString(key_obj);
    PyObject* i = PyObject_GetIter(lo);
    if (i == NULL) return -1;
    vector<int> v;
    PyObject* eo;
    while ((eo = PyIter_Next(i))) {
      if (!PyInt_Check(eo)) {
        Py_DECREF(eo);
        PyErr_SetString(PyExc_TypeError, "not int");
        return -1;
      }
      v.push_back(PyInt_AsLong(eo));
      Py_DECREF(eo);
    }
    Py_DECREF(i);
    (*m)[key] = v;
  }
  return 0;
}

// setset::iterator

typedef struct {
  PyObject_HEAD
  setset::iterator* it;
} PySetsetIterObject;

static PyObject* setsetiter_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
  PySetsetIterObject* self;
  self = reinterpret_cast<PySetsetIterObject*>(type->tp_alloc(type, 0));
  if (self == NULL) return NULL;
  return reinterpret_cast<PyObject*>(self);
}

static void setsetiter_dealloc(PySetsetIterObject* self) {
  delete self->it;
  PyObject_Del(self);
}

static PyObject* setsetiter_next(PySetsetIterObject* self) {
  if (*(self->it) == setset::end())
    return NULL;
  set<int> s = *(*self->it);
  ++(*self->it);
  return setset_build_set(s);
}

static PyMethodDef setsetiter_methods[] = {
  {NULL,           NULL}           /* sentinel */
};

static PyTypeObject PySetsetIter_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "setset_iterator",                          /* tp_name */
  sizeof(PySetsetIterObject),                 /* tp_basicsize */
  0,                                          /* tp_itemsize */
  /* methods */
  reinterpret_cast<destructor>(setsetiter_dealloc), /* tp_dealloc */
  0,                                          /* tp_print */
  0,                                          /* tp_getattr */
  0,                                          /* tp_setattr */
  0,                                          /* tp_compare */
  0,                                          /* tp_repr */
  0,                                          /* tp_as_number */
  0,                                          /* tp_as_sequence */
  0,                                          /* tp_as_mapping */
  0,                                          /* tp_hash */
  0,                                          /* tp_call */
  0,                                          /* tp_str */
  PyObject_GenericGetAttr,                    /* tp_getattro */
  0,                                          /* tp_setattro */
  0,                                          /* tp_as_buffer */
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_ITER, /* tp_flags */
  0,                                          /* tp_doc */
  0,                                          /* tp_traverse */
  0,                                          /* tp_clear */
  0,                                          /* tp_richcompare */
  0,                                          /* tp_weaklistoffset */
  PyObject_SelfIter,                          /* tp_iter */
  reinterpret_cast<iternextfunc>(setsetiter_next), /* tp_iternext */
  setsetiter_methods,                         /* tp_methods */
  0,                                          /* tp_members */
  0,                                          /* tp_getset */
  0,                                          /* tp_base */
  0,                                          /* tp_dict */
  0,                                          /* tp_descr_get */
  0,                                          /* tp_descr_set */
  0,                                          /* tp_dictoffset */
  0,                                          /* tp_init */
  PyType_GenericAlloc,                        /* tp_alloc */
  setsetiter_new                              /* tp_new */
};

// setset

static PyObject* setset_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
  PySetsetObject* self;
  self = reinterpret_cast<PySetsetObject*>(type->tp_alloc(type, 0));
  if (self == NULL) return NULL;
  return reinterpret_cast<PyObject*>(self);
}

static int setset_init(PySetsetObject* self, PyObject* args, PyObject* kwds) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj))
    return -1;
  if (obj == NULL || obj == Py_None) {
    self->ss = new setset();
  } else if (PySetset_Check(obj)) {
    PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(obj);
    self->ss = new setset(*(sso->ss));
  } else if (PyAnySet_Check(obj)) {
    set<int> s;
    if (setset_parse_set(obj, &s) == -1) return -1;
    self->ss = new setset(s);
  } else if (PyDict_Check(obj)) {
    map<string, vector<int> > m;
    if (setset_parse_map(obj, &m) == -1) return -1;
    self->ss = new setset(m);
  } else if (PyList_Check(obj)) {
    PyObject* i = PyObject_GetIter(obj);
    if (i == NULL) return -1;
    vector<set<int> > vs;
    PyObject* o;
    while ((o = PyIter_Next(i))) {
      if (!PyAnySet_Check(o)) {
        PyErr_SetString(PyExc_TypeError, "not set");
        return -1;
      }
      set<int> s;
      if (setset_parse_set(o, &s) == -1) return -1;
      vs.push_back(s);
      Py_DECREF(o);
    }
    Py_DECREF(i);
    self->ss = new setset(vs);
  } else {
    PyErr_SetString(PyExc_TypeError, "invalid argumet");
    return -1;
  }
  if (PyErr_Occurred())
    return -1;
  return 0;
}

static void setset_dealloc(PySetsetObject* self) {
  delete self->ss;
  self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

PyDoc_STRVAR(copy_doc,
"Returns a new setset with a shallow copy of `self`.\n\
\n\
Examples:\n\
  >>> ss2 = ss1.copy()\n\
  >>> ss1 -= ss2\n\
  >>> ss1 == ss2\n\
  False\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __init__()");

static PyObject* setset_copy(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, *self->ss);
}

PyDoc_STRVAR(complement_doc,
"Returns a new setset with the complement set of `self`.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2])])\n\
  >>> ss = ~ss\n\
  >>> ss\n\
  setset([set([]), set([2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __invert__()");

static PyObject* setset_complement(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, ~(*self->ss));
}

PyDoc_STRVAR(intersection_doc,
"Returns a new setset with sets common to `self` and all others.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss = ss1 & ss2\n\
  >>> ss\n\
  setset([set([1])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __and__(), intersection_update(), union(), difference(),\n\
  symmetric_difference()");

static PyObject* setset_intersection(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) & (*_other->ss));
}

PyDoc_STRVAR(intersection_update_doc,
"Updates `self`, keeping only sets found in it and all others.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss1 &= ss2\n\
  >>> ss1\n\
  setset([set([1])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __iand__(), intersection(), update(), difference_update(),\n\
  symmetric_difference_update()");

static PyObject* setset_intersection_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) &= (*_other->ss));
}

PyDoc_STRVAR(union_doc,
"Returns a new setset with sets from `self` and all others.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss = ss1 | ss2\n\
  >>> ss\n\
  setset([set([]), set([1]), set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __or__(), update(), intersection(), difference(),\n\
  symmetric_difference()");

static PyObject* setset_union(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) | (*_other->ss));
}

PyDoc_STRVAR(union_update_doc,
"Updates `self`, adding sets from all others.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss1 |= ss2\n\
  >>> ss1\n\
  setset([set([]), set([1]), set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __ior__(), union(), intersection_update(), difference_update(),\n\
  symmetric_difference_update()");

static PyObject* setset_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) |= (*_other->ss));
}

PyDoc_STRVAR(difference_doc,
"Returns a new setset with sets in `self` that are not in the others.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss = ss1 - ss2\n\
  >>> ss\n\
  setset([set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __sub__(), difference_update(), symmetric_difference(), union(),\n\
  intersection()");

static PyObject* setset_difference(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) - (*_other->ss));
}

PyDoc_STRVAR(difference_update_doc,
"Update `self`, removing sets found in others.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss1 -= ss2\n\
  >>> ss1\n\
  setset([set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __isub__(), difference(), symmetric_difference_update(), update()\n\
  intersection_update()");

static PyObject* setset_difference_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) -= (*_other->ss));
}

PyDoc_STRVAR(symmetric_difference_doc,
"Returns a new setset with sets in either `self` or `other` but not both.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss = ss1 ^ ss2\n\
  >>> ss\n\
  setset([set([]), set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __xor__(), symmetric_difference_update(), difference(), union()\n\
  intersection()");

static PyObject* setset_symmetric_difference(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) ^ (*_other->ss));
}

PyDoc_STRVAR(symmetric_difference_update_doc,
"Update `self`, keeping only sets in either setset, but not in both.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([1])])\n\
  >>> ss1 ^= ss2\n\
  >>> ss1\n\
  setset([set([]), set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __ixor__(), symmetric_difference(), difference_update(), update(),\n\
  intersection_update");

static PyObject* setset_symmetric_difference_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) ^= (*_other->ss));
}

PyDoc_STRVAR(quotient_doc,
"Returns a new setset of quotient.\n\
\n\
The quotient is defined by,\n\
  f / g = {a | a \\cup b \\in f and a \\cap b = \\empty, \\forall b \\in g}.\n\
D. Knuth, Exercise 204, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1,2]), set([3,4])])\n\
  >>> ss = ss / setset([set([2])])\n\
  >>> ss\n\
  setset([set([1])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __div__(), quotient_update(), remainder()");

static PyObject* setset_quotient(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) / (*_other->ss));
}

PyDoc_STRVAR(quotient_update_doc,
"Updates `self` by the quotient.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1,2]), set([3,4])])\n\
  >>> ss /= setset([set([2])])\n\
  >>> ss\n\
  setset([set([1])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __idiv__(), quotient(), remainder_update()");

static PyObject* setset_quotient_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) /= (*_other->ss));
}

PyDoc_STRVAR(remainder_doc,
"Returns a new setset of remainder.\n\
\n\
The remainder is defined by,\n\
  f % g = f - (f \\sqcup (f / g)).\n\
D. Knuth, Exercise 204, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1,2]), set([3,4])])\n\
  >>> ss = ss % setset([set([2])])\n\
  >>> ss\n\
  setset([set([3, 4])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __mod__(), remainder_update(), quotient()");

static PyObject* setset_remainder(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) % (*_other->ss));
}

PyDoc_STRVAR(remainder_update_doc,
"Updates `self` by the remainder.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1,2]), set([3,4])])\n\
  >>> ss %= setset([set([2])])\n\
  >>> ss\n\
  setset([set([3, 4])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  __imod__(), remainder(), quotient_update()");

static PyObject* setset_remainder_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) %= (*_other->ss));
}

PyDoc_STRVAR(isdisjoint_doc,
"Returns True if `self` has no sets in common with `other`.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([]), set([2])])\n\
  >>> ss1.disjoint(ss2)\n\
  True\n\
\n\
Returns:\n\
  True or False.\n\
\n\
See Also:\n\
  issubset(), issuperset()");

static PyObject* setset_isdisjoint(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_disjoint(*_other->ss));
}

PyDoc_STRVAR(issubset_doc,
"Tests if every set in `self` is in `other`.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([1]), set([1,2]), set([2])])\n\
  >>> ss1 <= (ss2)\n\
  True\n\
\n\
Returns:\n\
  True or False.\n\
\n\
See Also:\n\
  __le__(), __lt__(), issuperset(), isdisjoint()");

static PyObject* setset_issubset(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_subset(*_other->ss));
}

PyDoc_STRVAR(issuperset_doc,
"Tests if every set in `other` is in `self`.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2]), set([2])])\n\
  >>> ss2 = setset([set([1]), set([1,2])])\n\
  >>> ss1 >= (ss2)\n\
  True\n\
\n\
Returns:\n\
  True or False.\n\
\n\
See Also:\n\
  __ge__(), __gt__(), issubset(), isdisjoint()");

static PyObject* setset_issuperset(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_superset(*_other->ss));
}

static int setset_nonzero(PySetsetObject* self) {
  return !self->ss->empty();
}

static Py_ssize_t setset_len(PyObject* obj) {
  PySetsetObject* self = reinterpret_cast<PySetsetObject*>(obj);
  long long int len = strtoll(self->ss->size().c_str(), NULL, 0);
  if (len != LLONG_MAX) {
    return len;
  } else {
    PyErr_SetString(PyExc_OverflowError, "overflow, use setset.len()");
    return -1;
  }
}

PyDoc_STRVAR(long_len_doc,
"Returns the number of sets in `self`.\n\
\n\
This method never throws OverflowError unlike `len(setset)`.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2])])\n\
  >>> ss.len()\n\
  2\n\
\n\
Returns:\n\
  The number of sets.\n\
\n\
See Also:\n\
  __len__()");

static PyObject* setset_long_len(PyObject* obj) {
  PySetsetObject* self = reinterpret_cast<PySetsetObject*>(obj);
  vector<char> buf;
  string size = self->ss->size();
  for (string::const_iterator c = size.begin(); c != size.end(); ++c)
    buf.push_back(*c);
  buf.push_back('\0');
#ifdef HAVE_LIBGMPXX
  return PyLong_FromString(buf.data(), NULL, 0);
#else
  return PyLong_FromDouble(strtod(buf.data(), NULL));
#endif
}

static PyObject* setset_randomize(PySetsetObject* self) {
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) return NULL;
  ssi->it = new setset::iterator(self->ss->begin());
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_optimize(PySetsetObject* self, PyObject* weights,
                                 bool is_maximizing) {
  PyObject* i = PyObject_GetIter(weights);
  if (i == NULL) return NULL;
  PyObject* eo;
  vector<double> w;
  while ((eo = PyIter_Next(i))) {
    if (PyFloat_Check(eo)) {
      w.push_back(PyFloat_AsDouble(eo));
    }
    else if (PyLong_Check(eo)) {
      w.push_back(static_cast<double>(PyLong_AsLong(eo)));
    }
    else if (PyInt_Check(eo)) {
      w.push_back(static_cast<double>(PyInt_AsLong(eo)));
    }
    else {
      PyErr_SetString(PyExc_TypeError, "not a number");
      Py_DECREF(eo);
      return NULL;
    }
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) return NULL;
  ssi->it = new setset::iterator(
      is_maximizing ? self->ss->maximize(w) : self->ss->minimize(w));
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_maximize(PySetsetObject* self, PyObject* weights) {
  return setset_optimize(self, weights, true);
}

static PyObject* setset_minimize(PySetsetObject* self, PyObject* weights) {
  return setset_optimize(self, weights, false);
}

// If an item in o is equal to value, return 1, otherwise return 0. On error, return -1.
static int setset_contains(PySetsetObject* self, PyObject* so) {
  CHECK_OR_ERROR(so, PyAnySet_Check, "set", -1);
  set<int> s;
  if (setset_parse_set(so, &s) == -1) return -1;
  return self->ss->find(s) != setset::end() ? 1 : 0;
}

static PyObject* setset_include(PySetsetObject* self, PyObject* eo) {
  CHECK_OR_ERROR(eo, PyInt_Check, "int", NULL);
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(
      self->ob_type->tp_alloc(self->ob_type, 0));
  sso->ss = new setset(self->ss->include(PyInt_AsLong(eo)));
  return reinterpret_cast<PyObject*>(sso);
}

static PyObject* setset_exclude(PySetsetObject* self, PyObject* eo) {
  CHECK_OR_ERROR(eo, PyInt_Check, "int", NULL);
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(
      self->ob_type->tp_alloc(self->ob_type, 0));
  sso->ss = new setset(self->ss->exclude(PyInt_AsLong(eo)));
  return reinterpret_cast<PyObject*>(sso);
}

static PyObject* setset_add(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    set<int> s;
    if (setset_parse_set(obj, &s) == -1) return NULL;
    self->ss->insert(s);
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    self->ss->insert(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_remove(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    set<int> s;
    if (setset_parse_set(obj, &s) == -1) return NULL;
    if (self->ss->erase(s) == 0) {
      PyErr_SetString(PyExc_KeyError, "not found");
      return NULL;
    }
    self->ss->erase(s);
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (self->ss->include(e).empty()) {
      PyErr_SetString(PyExc_KeyError, "not found");
      return NULL;
    }
    self->ss->erase(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_discard(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    set<int> s;
    if (setset_parse_set(obj, &s) == -1) return NULL;
    self->ss->erase(s);
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    self->ss->erase(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_pop(PySetsetObject* self) {
  setset::iterator i = self->ss->begin();
  if (i == setset::end()) {
    PyErr_SetString(PyExc_KeyError, "not found");
    return NULL;
  }
  set<int> s = *i;
  self->ss->erase(s);
  return setset_build_set(s);
}

PyDoc_STRVAR(clear_doc,
"Removes all sets from `self`.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2])])\n\
  >>> ss.clear()\n\
  >>> ss\n\
  setset([])");

static PyObject* setset_clear(PySetsetObject* self) {
  self->ss->clear();
  Py_RETURN_NONE;
}

PyDoc_STRVAR(minimal_doc,
"Returns a new setset of minimal sets.\n\
\n\
The minimal sets are defined by,\n\
  f.minimal() = {a \\in f | b \\in f and a \\subseteq -> a = b}.\n\
D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2]), set([2,3])])\n\
  >>> ss = ss.minimal()\n\
  >>> ss\n\
  setset([set([1]), set([2, 3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  maximal(), hitting()");

static PyObject* setset_minimal(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->minimal());
}

PyDoc_STRVAR(maximal_doc,
"Returns a new setset of maximal sets.\n\
\n\
The maximal sets are defined by,\n\
  f.maximal() = {a \\in f | b \\in f and a \\superseteq -> a = b}.\n\
D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2]), set([2,3])])\n\
  >>> ss = ss.maximal()\n\
  >>> ss\n\
  setset([set([1, 2]), set([2, 3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  minimal()");

static PyObject* setset_maximal(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->maximal());
}

PyDoc_STRVAR(hitting_doc,
"Returns a new setset of hitting sets.\n\
\n\
The hitting sets are normally used as minimal hitting sets.\n\
\n\
The hitting sets are defined by,\n\
  f.hitting() = {a | b \\in f -> a \\cap b \\neq \\empty}.\n\
T. Toda, Hypergraph Dualization Algorithm Based on Binary Decision\n\
Diagrams.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2]), set([2,3])])\n\
  >>> ss = ss.hitting().minimal()\n\
  >>> ss\n\
  setset([set([1, 2]), set([2, 3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  minimal()");

static PyObject* setset_hitting(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->hitting());
}

PyDoc_STRVAR(smaller_doc,
"Returns a new setset with sets smaller than `n`.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2]), set([1,2,3])])\n\
  >>> ss = ss.smaller(2)\n\
  >>> ss\n\
  setset([set([1])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  larger(), equal()");

static PyObject* setset_smaller(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->smaller(set_size));
}

PyDoc_STRVAR(larger_doc,
"Returns a new setset with sets larger than `n`.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2]), set([1,2,3])])\n\
  >>> ss = ss.larger(2)\n\
  >>> ss\n\
  setset([set([1, 2, 3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  smaller(), equal()");

static PyObject* setset_larger(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->larger(set_size));
}

PyDoc_STRVAR(equal_doc,
"Returns a new setset with sets whose sizes are equal to `n`.\n\
\n\
Examples:\n\
  >>> ss = setset([set([1]), set([1,2]), set([1,2,3])])\n\
  >>> ss = ss.equal(2)\n\
  >>> ss\n\
  setset([set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  smaller(), larger()");

static PyObject* setset_equal(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->equal(set_size));
}

static PyObject* setset_invert(PySetsetObject* self, PyObject* eo) {
  CHECK_OR_ERROR(eo, PyInt_Check, "int", NULL);
  int e = PyLong_AsLong(eo);
  RETURN_NEW_SETSET(self, self->ss->invert(e));
}

PyDoc_STRVAR(join_doc,
"Returns a new setset of join between `self` and `other`.\n\
\n\
The join operation is defined by,\n\
  f \\sqcup g = {a \\cup b | a \\in f and b \\in g}.\n\
D. Knuth, Exercise 203, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([3])])\n\
  >>> ss = ss1.join(ss2)\n\
  >>> ss\n\
  setset([set([1, 3]), set([1, 2, 3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  meet()");

static PyObject* setset_join(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->join(*_other->ss));
}

PyDoc_STRVAR(meet_doc,
"Returns a new setset of meet between `self` and `other`.\n\
\n\
The meet operation is defined by,\n\
  f \\sqcap g = {a \\cap b | a \\in f and b \\in g}.\n\
D. Knuth, Exercise 203, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1,2]), set([1,3])])\n\
  >>> ss2 = setset([set([2,3])])\n\
  >>> ss = ss1.meet(ss2)\n\
  >>> ss\n\
  setset([set([2]), set([3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  join()");

static PyObject* setset_meet(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->meet(*_other->ss));
}

PyDoc_STRVAR(subsets_doc,
"Returns a new setset with sets that are subsets of a set in `other`.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([1,3]), set([2,3])])\n\
  >>> ss = ss1.subsets(ss2)\n\
  >>> ss\n\
  setset([set([1])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  supersets(), nonsubsets()");

static PyObject* setset_subsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->subsets(*_other->ss));
}

PyDoc_STRVAR(supersets_doc,
"Returns a new setset with sets that are supersets of a set in `other`.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1,3]), set([2,3])])\n\
  >>> ss2 = setset([set([1]), set([1,2])])\n\
  >>> ss = ss1.supersets(ss2)\n\
  >>> ss\n\
  setset([set([1, 3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  subsets(), nonsupersets()");

static PyObject* setset_supersets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->supersets(*_other->ss));
}

PyDoc_STRVAR(nonsubsets_doc,
"Returns a new setset with sets that aren't subsets of any set in `other`.\n\
\n\
The nonsubsets are defined by,\n\
  f.nonsubsets(g) = {a \\in f | b \\in g -> a \\not\\subseteq b}.\n\
D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1]), set([1,2])])\n\
  >>> ss2 = setset([set([1,3]), set([2,3])])\n\
  >>> ss = ss1.nonsubsets(ss2)\n\
  >>> ss\n\
  setset([set([1, 2])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  nonsupersets(), subsets()");

static PyObject* setset_nonsubsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->nonsubsets(*_other->ss));
}

PyDoc_STRVAR(nonsupersets_doc,
"Returns a new setset with sets that aren't supersets of any set in `other`.\n\
\n\
The nonsupersets are defined by,\n\
  f.nonsupersets(g) = {a \\in f | b \\in g -> a \\not\\superseteq b}.\n\
D. Knuth, Exercise 236, The art of computer programming, Sect.7.1.4.\n\
\n\
Examples:\n\
  >>> ss1 = setset([set([1,3]), set([2,3])])\n\
  >>> ss2 = setset([set([1]), set([1,2])])\n\
  >>> ss = ss1.nonsupersets(ss2)\n\
  >>> ss\n\
  setset([set([2, 3])])\n\
\n\
Returns:\n\
  A new setset object.\n\
\n\
See Also:\n\
  nonsubsets(), supersets()");

static PyObject* setset_nonsupersets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->nonsupersets(*_other->ss));
}

PyDoc_STRVAR(dump_doc,
"Serialize `self` to a file `fp`.\n\
\n\
This method does not serialize the universe, which should be saved\n\
separately by pickle.\n\
\n\
Args:\n\
  fp: A write-supporting file-like object.\n\
\n\
Examples:\n\
  >>> f = open('/path/to/file', 'w')\n\
  >>> ss.dump(f)\n\
\n\
See Also:\n\
  dumps(), load()");

static PyObject* setset_dump(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
  Py_BEGIN_ALLOW_THREADS;
  self->ss->dump(fp);
  Py_END_ALLOW_THREADS;
  PyFile_DecUseCount(file);
  Py_RETURN_NONE;
}

PyDoc_STRVAR(dumps_doc,
"Returns a serialized `self`.\n\
\n\
This method does not serialize the universe, which should be saved\n\
separately by pickle.\n\
\n\
Examples:\n\
  >>> str = ss.dumps()\n\
\n\
See Also:\n\
  dump(), loads()");

static PyObject* setset_dumps(PySetsetObject* self) {
  stringstream sstr;
  self->ss->dump(sstr);
  return PyString_FromString(sstr.str().c_str());
}

PyDoc_STRVAR(load_doc,
"Deserialize a file `fp` to `self`.\n\
\n\
This method does not deserialize the universe, which should be\n\
loaded separately by pickle.\n\
\n\
Args:\n\
  fp: A read-supporting file-like object.\n\
\n\
Examples:\n\
  >>> f = open('/path/to/file')\n\
  >>> ss.load(f)\n\
\n\
See Also:\n\
  loads(), dump()");

static PyObject* setset_load(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
  Py_BEGIN_ALLOW_THREADS;
  self->ss->load(fp);
  Py_END_ALLOW_THREADS;
  PyFile_DecUseCount(file);
  Py_RETURN_NONE;
}

PyDoc_STRVAR(loads_doc,
"Deserialize `str` to `self`.\n\
\n\
This method does not deserialize the universe, which should be\n\
loaded separately by pickle.\n\
\n\
Args:\n\
  str: A str instance.\n\
\n\
Examples:\n\
  >>> ss.load(str)\n\
\n\
See Also:\n\
  load(), dumps()");

static PyObject* setset_loads(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyString_Check, "str", NULL);
  stringstream sstr(PyString_AsString(obj));
  self->ss->load(sstr);
  Py_RETURN_NONE;
}

static PyObject* setset_enum(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
  Py_BEGIN_ALLOW_THREADS;
  string name = self->ob_type->tp_name;
  self->ss->_enum(fp, std::make_pair((name + "([").c_str(), "])"),
                  std::make_pair("set([", "])"));
  Py_END_ALLOW_THREADS;
  PyFile_DecUseCount(file);
  Py_RETURN_NONE;
}

static PyObject* setset_enums(PySetsetObject* self) {
  stringstream sstr;
  string name = self->ob_type->tp_name;
  self->ss->_enum(sstr, std::make_pair((name + "([").c_str(), "])"),
                  std::make_pair("set([", "])"));
  return PyString_FromString(sstr.str().c_str());
}

static PyObject* setset_repr(PySetsetObject* self) {
  return PyString_FromFormat("<%s object of %p>", self->ob_type->tp_name,
                             reinterpret_cast<void*>(self->ss->id()));
}
/*
static long setset_hash(PyObject* self) {
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(self);
  return sso->ss->id();
}
*/
static int setset_nocmp(PyObject* self, PyObject* other) {
  PyErr_SetString(PyExc_TypeError, "cannot compare using cmp()");
  return -1;
}

static PyObject* setset_richcompare(PySetsetObject* self, PyObject* obj, int op) {
  PySetsetObject* sso;
  if(!PySetset_Check(obj)) {
    if (op == Py_EQ) Py_RETURN_FALSE;
    if (op == Py_NE) Py_RETURN_TRUE;
    PyErr_SetString(PyExc_TypeError, "can only compare to set of sets");
    return NULL;
  }
  sso = reinterpret_cast<PySetsetObject*>(obj);
  switch (op) {
    case Py_EQ:
      if (*self->ss == *sso->ss) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    case Py_NE:
      if (*self->ss != *sso->ss) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    case Py_LE:
      if (*self->ss <= *sso->ss) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    case Py_GE:
      if (*self->ss >= *sso->ss) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    case Py_LT:
      if (*self->ss < *sso->ss) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
    case Py_GT:
      if (*self->ss > *sso->ss) Py_RETURN_TRUE;
      else Py_RETURN_FALSE;
  }
  Py_INCREF(Py_NotImplemented);
  return Py_NotImplemented;
}

static PyMemberDef setset_members[] = {
  {NULL}  /* Sentinel */
};

static PyMethodDef setset_methods[] = {
  {"copy", reinterpret_cast<PyCFunction>(setset_copy), METH_NOARGS, copy_doc},
  {"complement", reinterpret_cast<PyCFunction>(setset_complement), METH_NOARGS, complement_doc},
  {"intersection", reinterpret_cast<PyCFunction>(setset_intersection), METH_O, intersection_doc},
  {"intersection_update", reinterpret_cast<PyCFunction>(setset_intersection_update), METH_O, intersection_update_doc},
  {"union", reinterpret_cast<PyCFunction>(setset_union), METH_O, union_doc},
  {"update", reinterpret_cast<PyCFunction>(setset_update), METH_O, union_update_doc},
  {"difference", reinterpret_cast<PyCFunction>(setset_difference), METH_O, difference_doc},
  {"difference_update", reinterpret_cast<PyCFunction>(setset_difference_update), METH_O, difference_update_doc},
  {"symmetric_difference", reinterpret_cast<PyCFunction>(setset_symmetric_difference), METH_O, symmetric_difference_doc},
  {"symmetric_difference_update", reinterpret_cast<PyCFunction>(setset_symmetric_difference_update), METH_O, symmetric_difference_update_doc},
  {"quotient", reinterpret_cast<PyCFunction>(setset_quotient), METH_O, quotient_doc},
  {"quotient_update", reinterpret_cast<PyCFunction>(setset_quotient_update), METH_O, quotient_update_doc},
  {"remainder", reinterpret_cast<PyCFunction>(setset_remainder), METH_O, remainder_doc},
  {"remainder_update", reinterpret_cast<PyCFunction>(setset_remainder_update), METH_O, remainder_update_doc},
  {"isdisjoint", reinterpret_cast<PyCFunction>(setset_isdisjoint), METH_O, isdisjoint_doc},
  {"issubset", reinterpret_cast<PyCFunction>(setset_issubset), METH_O, issubset_doc},
  {"issuperset", reinterpret_cast<PyCFunction>(setset_issuperset), METH_O, issuperset_doc},
  {"len", reinterpret_cast<PyCFunction>(setset_long_len), METH_NOARGS, long_len_doc},
  {"randomize", reinterpret_cast<PyCFunction>(setset_randomize), METH_NOARGS, ""},
  {"maximize", reinterpret_cast<PyCFunction>(setset_maximize), METH_O, ""},
  {"minimize", reinterpret_cast<PyCFunction>(setset_minimize), METH_O, ""},
  {"include", reinterpret_cast<PyCFunction>(setset_include), METH_O, ""},
  {"exclude", reinterpret_cast<PyCFunction>(setset_exclude), METH_O, ""},
  {"add", reinterpret_cast<PyCFunction>(setset_add), METH_O, ""},
  {"remove", reinterpret_cast<PyCFunction>(setset_remove), METH_O, ""},
  {"discard", reinterpret_cast<PyCFunction>(setset_discard), METH_O, ""},
  {"pop", reinterpret_cast<PyCFunction>(setset_pop), METH_NOARGS, ""},
  {"clear", reinterpret_cast<PyCFunction>(setset_clear), METH_NOARGS, clear_doc},
  {"minimal", reinterpret_cast<PyCFunction>(setset_minimal), METH_NOARGS, minimal_doc},
  {"maximal", reinterpret_cast<PyCFunction>(setset_maximal), METH_NOARGS, maximal_doc},
  {"hitting", reinterpret_cast<PyCFunction>(setset_hitting), METH_NOARGS, hitting_doc},
  {"smaller", reinterpret_cast<PyCFunction>(setset_smaller), METH_O, smaller_doc},
  {"larger", reinterpret_cast<PyCFunction>(setset_larger), METH_O, larger_doc},
  {"equal", reinterpret_cast<PyCFunction>(setset_equal), METH_O, equal_doc},
  {"invert", reinterpret_cast<PyCFunction>(setset_invert), METH_O, ""},
  {"join", reinterpret_cast<PyCFunction>(setset_join), METH_O, join_doc},
  {"meet", reinterpret_cast<PyCFunction>(setset_meet), METH_O, meet_doc},
  {"subsets", reinterpret_cast<PyCFunction>(setset_subsets), METH_O, subsets_doc},
  {"supersets", reinterpret_cast<PyCFunction>(setset_supersets), METH_O, supersets_doc},
  {"nonsubsets", reinterpret_cast<PyCFunction>(setset_nonsubsets), METH_O, nonsubsets_doc},
  {"nonsupersets", reinterpret_cast<PyCFunction>(setset_nonsupersets), METH_O, nonsupersets_doc},
  {"dump", reinterpret_cast<PyCFunction>(setset_dump), METH_O, dump_doc},
  {"dumps", reinterpret_cast<PyCFunction>(setset_dumps), METH_NOARGS, dumps_doc},
  {"load", reinterpret_cast<PyCFunction>(setset_load), METH_O, load_doc},
  {"loads", reinterpret_cast<PyCFunction>(setset_loads), METH_O, loads_doc},
  {"_enum", reinterpret_cast<PyCFunction>(setset_enum), METH_O, ""},
  {"_enums", reinterpret_cast<PyCFunction>(setset_enums), METH_NOARGS, ""},
  {NULL}  /* Sentinel */
};

static PyNumberMethods setset_as_number = {
  0,                                  /*nb_add*/
  reinterpret_cast<binaryfunc>(setset_difference), /*nb_subtract*/
  0,                                  /*nb_multiply*/
  reinterpret_cast<binaryfunc>(setset_quotient), /*nb_divide*/
  reinterpret_cast<binaryfunc>(setset_remainder), /*nb_remainder*/
  0,                                  /*nb_divmod*/
  0,                                  /*nb_power*/
  0,                                  /*nb_negative*/
  0,                                  /*nb_positive*/
  0,                                  /*nb_absolute*/
  reinterpret_cast<inquiry>(setset_nonzero), /*nb_nonzero*/
  reinterpret_cast<unaryfunc>(setset_complement), /*nb_invert*/
  0,                                  /*nb_lshift*/
  0,                                  /*nb_rshift*/
  reinterpret_cast<binaryfunc>(setset_intersection), /*nb_and*/
  reinterpret_cast<binaryfunc>(setset_symmetric_difference), /*nb_xor*/
  reinterpret_cast<binaryfunc>(setset_union), /*nb_or*/
  0/*reinterpret_cast<coercion>(Py_TPFLAGS_CHECKTYPES)*/, /*nb_coerce*/
  0,                                  /*nb_int*/
  0,                                  /*nb_long*/
  0,                                  /*nb_float*/
  0,                                  /*nb_oct*/
  0,                                  /*nb_hex*/
  0,                                  /*nb_inplace_add*/
  reinterpret_cast<binaryfunc>(setset_difference_update), /*nb_inplace_subtract*/
  0,                                  /*nb_inplace_multiply*/
  reinterpret_cast<binaryfunc>(setset_quotient_update), /*nb_inplace_divide*/
  reinterpret_cast<binaryfunc>(setset_remainder_update), /*nb_inplace_remainder*/
  0,                                  /*nb_inplace_power*/
  0,                                  /*nb_inplace_lshift*/
  0,                                  /*nb_inplace_rshift*/
  reinterpret_cast<binaryfunc>(setset_intersection_update), /*nb_inplace_and*/
  reinterpret_cast<binaryfunc>(setset_symmetric_difference_update), /*nb_inplace_xor*/
  reinterpret_cast<binaryfunc>(setset_update), /*nb_inplace_or*/
};

static PySequenceMethods setset_as_sequence = {
  setset_len,                         /* sq_length */
  0,                                  /* sq_concat */
  0,                                  /* sq_repeat */
  0,                                  /* sq_item */
  0,                                  /* sq_slice */
  0,                                  /* sq_ass_item */
  0,                                  /* sq_ass_slice */
  reinterpret_cast<objobjproc>(setset_contains), /* sq_contains */
};

PyTypeObject PySetset_Type = {
  PyObject_HEAD_INIT(NULL)
  0,                                  /*ob_size*/
  "_graphillion.setset",              /*tp_name*/
  sizeof(PySetsetObject),             /*tp_basicsize*/
  0,                                  /*tp_itemsize*/
  reinterpret_cast<destructor>(setset_dealloc), /*tp_dealloc*/
  0,                                  /*tp_print*/
  0,                                  /*tp_getattr*/
  0,                                  /*tp_setattr*/
  setset_nocmp,                       /*tp_compare*/
  reinterpret_cast<reprfunc>(setset_repr), /*tp_repr*/
  &setset_as_number,                  /*tp_as_number*/
  &setset_as_sequence,                /*tp_as_sequence*/
  0,                                  /*tp_as_mapping*/
  0,                                  /*tp_hash */
  0,                                  /*tp_call*/
  0,                                  /*tp_str*/
  0,                                  /*tp_getattro*/
  0,                                  /*tp_setattro*/
  0,                                  /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES, /*tp_flags*/
  "Base class for set of sets",       /* tp_doc */
  0,		                      /* tp_traverse */
  0,		                      /* tp_clear */
  reinterpret_cast<richcmpfunc>(setset_richcompare), /* tp_richcompare */
  0,		                      /* tp_weaklistoffset */
  0,                                  /* tp_iter */
  0,                                  /* tp_iternext */
  setset_methods,                     /* tp_methods */
  setset_members,                     /* tp_members */
  0,                                  /* tp_getset */
  0,                                  /* tp_base */
  0,                                  /* tp_dict */
  0,                                  /* tp_descr_get */
  0,                                  /* tp_descr_set */
  0,                                  /* tp_dictoffset */
  reinterpret_cast<initproc>(setset_init), /* tp_init */
  PyType_GenericAlloc,                /* tp_alloc */
  setset_new,                         /* tp_new */
};

static PyObject* setset_num_elems(PyObject*, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return NULL;
  if (obj == NULL) {
    return PyInt_FromLong(setset::num_elems());
  } else {
    setset::num_elems(PyInt_AsLong(obj));
    Py_RETURN_NONE;
  }
}

static PyMethodDef module_methods[] = {
  {"num_elems", setset_num_elems, METH_VARARGS, ""},
  {NULL}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC init_graphillion(void) {
  PyObject* m;
  if (PyType_Ready(&PySetset_Type) < 0) return;
  if (PyType_Ready(&PySetsetIter_Type) < 0) return;
  m = Py_InitModule3("_graphillion", module_methods,
                     "Hidden module to implement graphillion objects.");
  if (m == NULL) return;
  Py_INCREF(&PySetset_Type);
  Py_INCREF(&PySetsetIter_Type);
  PyModule_AddObject(m, "setset", reinterpret_cast<PyObject*>(&PySetset_Type));
  PyModule_AddObject(m, "setset_iterator",
                     reinterpret_cast<PyObject*>(&PySetsetIter_Type));
}
