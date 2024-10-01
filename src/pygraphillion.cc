/*********************************************************************
Copyright 2013  JST ERATO Minato project and other contributors
http://www-erato.ist.hokudai.ac.jp/?language=en

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
**********************************************************************/

#include <Python.h>
#include "pollyfill.h"

#include "structmember.h"

#include "graphillion/partition/Partition.h"
#include "graphillion/partition/BalancedPartition.h"
#include "pygraphillion.h"

#include <cstdlib>
#include <climits>

#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <sstream>
#include <vector>

#include "graphillion/graphset.h"
#include "graphillion/reliability/reliability.h"
#include "graphillion/regular/RegularGraphs.h"
#include "graphillion/induced_graphs/InducedGraphs.h"
#include "graphillion/induced_graphs/WeightedInducedGraphs.h"
#include "graphillion/chordal/chordal.h"
#include "graphillion/forbidden_induced/ForbiddenInducedSubgraphs.h"

#include "graphillion/odd_edges_subgraphs/OddEdgeSubgraphs.h"
#include "graphillion/degree_distribution/DegreeDistributionGraphs.h"

using graphillion::setset;
using graphillion::Range;
using graphillion::vertex_t;
using graphillion::edge_t;
using graphillion::weighted_edge_t;
using graphillion::linear_constraint_t;
using std::map;
using std::pair;
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
        Py_TYPE(self)->tp_alloc(Py_TYPE(self), 0));           \
    _ret->ss = new setset(expr);                              \
    return reinterpret_cast<PyObject*>(_ret);                 \
  } while (0);


#define RETURN_NEW_SETSET2(self, other, _other, expr)                   \
  do {                                                                  \
    PySetsetObject* _other = reinterpret_cast<PySetsetObject*>(other); \
    PySetsetObject* _ret = reinterpret_cast<PySetsetObject*>(           \
        Py_TYPE(self)->tp_alloc(Py_TYPE(self), 0));                     \
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
    PySetsetObject* _other = reinterpret_cast<PySetsetObject*>(other); \
    if (expr) Py_RETURN_TRUE;                                           \
    else      Py_RETURN_FALSE;                                          \
  } while (0);

#define DO_FOR_MULTI(self, others, expr)                                \
  do {                                                                  \
    PyObject* _result = reinterpret_cast<PyObject*>(self);              \
    if (PyTuple_GET_SIZE(others) == 0)                                  \
      return setset_copy(self);                                         \
    Py_INCREF(self);                                                    \
    for (Py_ssize_t _i = 0; _i < PyTuple_GET_SIZE(others); ++_i) {      \
      PyObject* _other = PyTuple_GET_ITEM(others, _i);                  \
      PyObject* _newresult                                              \
          = expr(reinterpret_cast<PySetsetObject*>(_result), _other);   \
      if (_newresult == NULL) {                                         \
        Py_DECREF(_result);                                             \
        return NULL;                                                    \
      }                                                                 \
      Py_DECREF(_result);                                               \
      _result = _newresult;                                             \
    }                                                                   \
    return _result;                                                     \
  } while (0);

#define UPDATE_FOR_MULTI(self, others, expr)                           \
  do {                                                                 \
    for (Py_ssize_t _i = 0; _i < PyTuple_GET_SIZE(others); ++_i) {     \
      PyObject* _other = PyTuple_GET_ITEM(others, _i);                 \
      if (expr(self, _other) == NULL)                                  \
        return NULL;                                                   \
    }                                                                  \
    Py_RETURN_NONE;                                                    \
  } while (0);

#if IS_PY3 == 1

#define PyString_AsString PyUnicode_AsUTF8

int PyFile_Check(PyObject *obj) {
  return PyObject_AsFileDescriptor(obj);
}

#endif

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
    Py_DECREF(eo);  // TODO: is Py_DECREF() required for PyInt_FromLong object?
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

static vector<int> intersection(const map<string, vector<int> >& m,
                                const string& key1, const string& key2) {
  map<string, vector<int> >::const_iterator in_i = m.find(key1);
  map<string, vector<int> >::const_iterator ex_i = m.find(key2);
  vector<int> in_v = in_i != m.end() ? in_i->second : vector<int>();
  vector<int> ex_v = ex_i != m.end() ? ex_i->second : vector<int>();
  std::sort(in_v.begin(), in_v.end());
  std::sort(ex_v.begin(), ex_v.end());
  vector<int> v(std::max(in_v.size(), ex_v.size()));
  vector<int>::const_iterator end
      = std::set_intersection(in_v.begin(), in_v.end(), ex_v.begin(), ex_v.end(),
                              v.begin());
  v.resize(end - v.begin());
  return v;
}

static int setset_parse_map(PyObject* dict_obj, map<string, vector<int> >* m) {
  assert(m != NULL);
  PyObject* key_obj;
  PyObject* lo;
  Py_ssize_t pos = 0;
  while (PyDict_Next(dict_obj, &pos, &key_obj, &lo)) {
    if (!PyStr_Check(key_obj)) {
      PyErr_SetString(PyExc_TypeError, "invalid argument");
      return -1;
    }
    string key = PyString_AsString(key_obj);
    if (key != "include" && key != "exclude") {
      PyErr_SetString(PyExc_TypeError, "invalid dict key");
      return -1;
    }
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
  if (!intersection(*m, "include", "exclude").empty()) {
    PyErr_SetString(PyExc_TypeError, "inconsistent constraints");
    return -1;
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
  0,                                          /* tp_compare or *tp_reserved */
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
  setsetiter_new,                             /* tp_new */
#if IS_PY3 == 1
  0, /* tp_free */
  0, /* tp_is_gc */
  0, /* *tp_bases */
  0, /* *tp_mro */
  0, /* *tp_cache */
  0, /* *tp_subclasses */
  0, /* *tp_weaklist */
  0, /* tp_version_tag */
  0, /* tp_finalize */
#endif
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
  } else if (PyDict_Check(obj)) {
    map<string, vector<int> > m;
    if (setset_parse_map(obj, &m) == -1) return -1;
    self->ss = new setset(m);
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
  Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

static PyObject* setset_copy(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, *self->ss);
}

static PyObject* setset_invert(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, ~(*self->ss));
}

static PyObject* setset_union(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) | (*_other->ss));
}

static PyObject* setset_union_multi(PySetsetObject* self, PyObject* others) {
  DO_FOR_MULTI(self, others, setset_union);
}

static PyObject* setset_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) |= (*_other->ss));
}

static PyObject* setset_update_multi(PySetsetObject* self, PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_update);
}

static PyObject* setset_intersection(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) & (*_other->ss));
}

static PyObject* setset_intersection_multi(PySetsetObject* self, PyObject* others) {
  DO_FOR_MULTI(self, others, setset_intersection);
}

static PyObject* setset_intersection_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) &= (*_other->ss));
}

static PyObject* setset_intersection_update_multi(PySetsetObject* self, PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_intersection_update);
}

static PyObject* setset_difference(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) - (*_other->ss));
}

static PyObject* setset_difference_multi(PySetsetObject* self, PyObject* others) {
  DO_FOR_MULTI(self, others, setset_difference);
}

static PyObject* setset_difference_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) -= (*_other->ss));
}

static PyObject* setset_difference_update_multi(PySetsetObject* self, PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_difference_update);
}

static PyObject* setset_symmetric_difference(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) ^ (*_other->ss));
}

static PyObject* setset_symmetric_difference_multi(PySetsetObject* self, PyObject* others) {
  DO_FOR_MULTI(self, others, setset_symmetric_difference);
}

static PyObject* setset_symmetric_difference_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) ^= (*_other->ss));
}

static PyObject* setset_symmetric_difference_update_multi(PySetsetObject* self, PyObject* others) {
  UPDATE_FOR_MULTI(self, others, setset_symmetric_difference_update);
}

static PyObject* setset_quotient(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) / (*_other->ss));
}

static PyObject* setset_quotient_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) /= (*_other->ss));
}

static PyObject* setset_remainder(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) % (*_other->ss));
}

static PyObject* setset_remainder_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) %= (*_other->ss));
}

static PyObject* setset_isdisjoint(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_disjoint(*_other->ss));
}

static PyObject* setset_issubset(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_TRUE_IF(self, other, _other, self->ss->is_subset(*_other->ss));
}

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
    PyErr_SetString(PyExc_OverflowError, "overflow, use obj.len()");
    return -1;
  }
}

static PyObject* setset_len2(PySetsetObject* self, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return NULL;
  if (obj == NULL || obj == Py_None) {
    string size = self->ss->size();
    vector<char> buf;
    for (string::const_iterator c = size.begin(); c != size.end(); ++c)
      buf.push_back(*c);
    buf.push_back('\0');
    return PyLong_FromString(buf.data(), NULL, 0);
  } else if (PyInt_Check(obj)) {
    int len = PyLong_AsLong(obj);
    RETURN_NEW_SETSET(self, self->ss->set_size(len));
  } else {
    PyErr_SetString(PyExc_TypeError, "not int");
    return NULL;
  }
}

static PyObject* setset_iter(PySetsetObject* self) {
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) return NULL;
  ssi->it = new setset::iterator(self->ss->begin());
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_rand_iter(PySetsetObject* self) {
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) return NULL;
  ssi->it = new setset::random_iterator(self->ss->begin_randomly());
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
  ssi->it = new setset::weighted_iterator(
      is_maximizing ? self->ss->begin_from_max(w) : self->ss->begin_from_min(w));
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_max_iter(PySetsetObject* self, PyObject* weights) {
  return setset_optimize(self, weights, true);
}

static PyObject* setset_min_iter(PySetsetObject* self, PyObject* weights) {
  return setset_optimize(self, weights, false);
}

// If an item in o is equal to value, return 1, otherwise return 0. On error, return -1.
static int setset_contains(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    set<int> s;
    if (setset_parse_set(obj, &s) == -1) return -1;
    return self->ss->find(s) != self->ss->end() ? 1 : 0;
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    return self->ss->supersets(e) != setset() ? 1 : 0;
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return -1;
  }
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
    if (self->ss->supersets(e).empty()) {
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
  if (i == self->ss->end()) {
    PyErr_SetString(PyExc_KeyError, "'pop' from an empty set");
    return NULL;
  }
  set<int> s = *i;
  self->ss->erase(s);
  return setset_build_set(s);
}

static PyObject* setset_clear(PySetsetObject* self) {
  self->ss->clear();
  Py_RETURN_NONE;
}

static PyObject* setset_flip(PySetsetObject* self, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return NULL;
  if (obj == NULL || obj == Py_None) {
    self->ss->flip();
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    self->ss->flip(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_minimal(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->minimal());
}

static PyObject* setset_maximal(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->maximal());
}

static PyObject* setset_hitting(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->hitting());
}

static PyObject* setset_smaller(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->smaller(set_size));
}

static PyObject* setset_larger(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->larger(set_size));
}

static PyObject* setset_set_size(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyInt_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_SETSET(self, self->ss->set_size(set_size));
}

static PyObject* setset_join(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->join(*_other->ss));
}

static PyObject* setset_meet(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->meet(*_other->ss));
}

static PyObject* setset_subsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->subsets(*_other->ss));
}

static PyObject* setset_supersets(PySetsetObject* self, PyObject* obj) {
  if (PySetset_Check(obj)) {
    RETURN_NEW_SETSET2(self, obj, _obj, self->ss->supersets(*_obj->ss));
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    RETURN_NEW_SETSET(self, self->ss->supersets(e));
  } else {
    PyErr_SetString(PyExc_TypeError, "not setset nor int");
    return NULL;
  }
}

static PyObject* setset_non_subsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->non_subsets(*_other->ss));
}

static PyObject* setset_non_supersets(PySetsetObject* self, PyObject* obj) {
  if (PySetset_Check(obj)) {
    RETURN_NEW_SETSET2(self, obj, _obj, self->ss->non_supersets(*_obj->ss));
  } else if (PyInt_Check(obj)) {
    int e = PyLong_AsLong(obj);
    RETURN_NEW_SETSET(self, self->ss->non_supersets(e));
  } else {
    PyErr_SetString(PyExc_TypeError, "not setset nor int");
    return NULL;
  }
}

static PyObject* setset_choice(PySetsetObject* self) {
  setset::iterator i = self->ss->begin();
  if (i == self->ss->end()) {
    PyErr_SetString(PyExc_KeyError, "'choice' from an empty set");
    return NULL;
  }
  set<int> s = *i;
  return setset_build_set(s);
}

static PyObject* setset_probability(PySetsetObject* self,
                                    PyObject* probabilities) {
  PyObject* i = PyObject_GetIter(probabilities);
  if (i == NULL) return NULL;
  PyObject* eo;
  vector<double> p;
  while ((eo = PyIter_Next(i))) {
    if (PyFloat_Check(eo)) {
      p.push_back(PyFloat_AsDouble(eo));
    }
    else if (PyLong_Check(eo)) {
      p.push_back(static_cast<double>(PyLong_AsLong(eo)));
    }
    else if (PyInt_Check(eo)) {
      p.push_back(static_cast<double>(PyInt_AsLong(eo)));
    }
    else {
      PyErr_SetString(PyExc_TypeError, "not a number");
      Py_DECREF(eo);
      return NULL;
    }
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  return PyFloat_FromDouble(self->ss->probability(p));
}

static PyObject* setset_dump(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
#if IS_PY3 == 1
  int fd = PyObject_AsFileDescriptor(obj);
  FILE* fp = fdopen(dup(fd), "w");
#else
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
#endif
  Py_BEGIN_ALLOW_THREADS;
  self->ss->dump(fp);
  Py_END_ALLOW_THREADS;
#if IS_PY3 == 1
  fclose(fp);
#else
  PyFile_DecUseCount(file);
#endif
  Py_RETURN_NONE;
}

static PyObject* setset_dumps(PySetsetObject* self) {
  stringstream sstr;
  self->ss->dump(sstr);
  return PyStr_FromString(sstr.str().c_str());
}

static PyObject* setset_load(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
#if IS_PY3 == 1
  int fd = PyObject_AsFileDescriptor(obj);
  FILE* fp = fdopen(dup(fd), "r");
#else
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
#endif
  PySetsetObject* ret;
  Py_BEGIN_ALLOW_THREADS;
  ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(setset::load(fp));
  Py_END_ALLOW_THREADS;
#if IS_PY3 == 1
  fclose(fp);
#else
  PyFile_DecUseCount(file);
#endif
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* setset_loads(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyStr_Check, "str", NULL);
  stringstream sstr(PyStr_AsString(obj));
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(setset::load(sstr));
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* setset_enum(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", NULL);
#if IS_PY3 == 1
  int fd = PyObject_AsFileDescriptor(obj);
  FILE* fp = fdopen(fd, "r");
#else
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
#endif
  Py_BEGIN_ALLOW_THREADS;
  string name = Py_TYPE(self)->tp_name;
  self->ss->_enum(fp, std::make_pair((name + "([").c_str(), "])"),
                  std::make_pair("set([", "])"));
  Py_END_ALLOW_THREADS;
#if IS_PY3 == 0
  PyFile_DecUseCount(file);
#endif
  Py_RETURN_NONE;
}

static PyObject* setset_enums(PySetsetObject* self) {
  stringstream sstr;
  string name = Py_TYPE(self)->tp_name;
  self->ss->_enum(sstr, std::make_pair((name + "([").c_str(), "])"),
                  std::make_pair("set([", "])"));
  return PyStr_FromString(sstr.str().c_str());
}

static PyObject* setset_repr(PySetsetObject* self) {
  return PyStr_FromFormat("<%s object of %p>", Py_TYPE(self)->tp_name,
                             reinterpret_cast<void*>(self->ss->id()));
}
/*
static long setset_hash(PyObject* self) {
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(self);
  return sso->ss->id();
}
*/
#if IS_PY3 == 0
static int setset_nocmp(PyObject* self, PyObject* other) {
  PyErr_SetString(PyExc_TypeError, "cannot compare using cmp()");
  return -1;
}
#endif

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

static PyObject* setset_cost_le(PySetsetObject* self, PyObject* args, PyObject* kwds) {
  static char s1[] = "costs";
  static char s2[] = "cost_bound";
  static char* kwlist[3] = {s1, s2, NULL};
  PyObject* costs_obj = NULL;
  bddcost cost_bound = 0;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "Oi", kwlist, &costs_obj, &cost_bound))
    return NULL;
  if (costs_obj == NULL || costs_obj == Py_None) {
    PyErr_SetString(PyExc_ValueError, "no costs");
    return NULL;
  }
  //if (cost_bound == 0) {
  //  PyErr_SetString(PyExc_ValueError, "no cost bound");
  //  return NULL;
  //}

  PyObject* cost_iter = PyObject_GetIter(costs_obj);
  if (cost_iter == NULL) return NULL;
  vector<bddcost> costs;
  PyObject* cost;
  while ((cost = PyIter_Next(cost_iter))) {
    if (PyLong_Check(cost)) {
      costs.push_back(static_cast<int>(PyLong_AsLong(cost)));
    } else {
      PyErr_SetString(PyExc_TypeError, "not a number");
      Py_DECREF(cost);
      return NULL;
    }
  }
  Py_DECREF(cost_iter);

  RETURN_NEW_SETSET(self, self->ss->cost_le(costs, cost_bound));
}

static PyObject* setset_remove_some_element(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, self->ss->remove_some_element());
}

static PyObject* setset_add_some_element(PySetsetObject* self, PyObject* args) {
  int num_variables = 0;
  if (!PyArg_ParseTuple(args, "|i", &num_variables)) {
    return NULL;
  }
  if (num_variables < 0) {
    PyErr_SetString(PyExc_TypeError, "not a positive number");
    return NULL;
  } else if (num_variables == 0) {
    num_variables = setset::num_elems();
  }
  //RETURN_NEW_SETSET(self, self->ss->add_some_element(setset::max_elem(), setset::max_elem() - setset::num_elems() + 1));
  RETURN_NEW_SETSET(self, self->ss->add_some_element(setset::max_elem() - setset::num_elems() + num_variables,
    setset::max_elem() - setset::num_elems() + 1));
}

static PyObject* setset_remove_add_some_elements(PySetsetObject* self, PyObject* args) {
  int num_variables = 0;
  if (!PyArg_ParseTuple(args, "|i", &num_variables)) {
    return NULL;
  }
  if (num_variables < 0) {
    PyErr_SetString(PyExc_TypeError, "not a positive number");
    return NULL;
  } else if (num_variables == 0) {
    num_variables = setset::num_elems();
  }
  //RETURN_NEW_SETSET(self, self->ss->remove_add_some_elements(setset::max_elem(), setset::max_elem() - setset::num_elems() + 1));
  RETURN_NEW_SETSET(self, self->ss->remove_add_some_elements(setset::max_elem() - setset::num_elems() + num_variables,
    setset::max_elem() - setset::num_elems() + 1));
}

std::vector<std::vector<std::string>> parse_args_to_edges(PyObject* args) {
  std::vector<std::vector<std::string>> edges;
  assert(edges.size() == 0);
  PyObject* edges_obj = NULL;
  std::vector<std::vector<std::string>> empty_edges;
  if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &edges_obj))
    return empty_edges;
  if (edges_obj == NULL || edges_obj == Py_None) {
    PyErr_SetString(PyExc_ValueError, "no edges");
    return empty_edges;
  }

  int edge_num = PyList_Size(edges_obj);
  edges.reserve(edge_num);
  for (int i = 0; i < edge_num; ++i) {
    PyObject *edge_obj = PyList_GetItem(edges_obj, i);
    if (!PyList_Check(edge_obj) || PyList_Size(edge_obj) != 2) {
      PyErr_SetString(PyExc_ValueError, "invalid edge");
      return empty_edges;
    }
    std::vector<std::string> edge(2);
    for (int j = 0; j < 2; ++j) {
      PyObject *v_obj = PyList_GetItem(edge_obj, j);
      PyObject *v_repr = PyObject_Repr(v_obj);
      const char *v = PyString_AsString(v_repr);
      edge[j] = std::string(v);
    }
    edges.push_back(std::move(edge));
  }
  return edges;
}

static PyObject* setset_to_vertexsetset(PySetsetObject* self, PyObject* args) {
  std::vector<std::vector<std::string>> edges = parse_args_to_edges(args);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  auto ss = self->ss->to_vertexsetset_setset(edges);
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyMemberDef setset_members[] = {
  {NULL}  /* Sentinel */
};

static PyMethodDef setset_methods[] = {
  {"copy", reinterpret_cast<PyCFunction>(setset_copy), METH_NOARGS, ""},
  {"invert", reinterpret_cast<PyCFunction>(setset_invert), METH_NOARGS, ""},
  {"union", reinterpret_cast<PyCFunction>(setset_union_multi), METH_VARARGS, ""},
  {"update", reinterpret_cast<PyCFunction>(setset_update_multi), METH_VARARGS, ""},
  {"intersection", reinterpret_cast<PyCFunction>(setset_intersection_multi), METH_VARARGS, ""},
  {"intersection_update", reinterpret_cast<PyCFunction>(setset_intersection_update_multi), METH_VARARGS, ""},
  {"difference", reinterpret_cast<PyCFunction>(setset_difference_multi), METH_VARARGS, ""},
  {"difference_update", reinterpret_cast<PyCFunction>(setset_difference_update_multi), METH_VARARGS, ""},
  {"symmetric_difference", reinterpret_cast<PyCFunction>(setset_symmetric_difference_multi), METH_VARARGS, ""},
  {"symmetric_difference_update", reinterpret_cast<PyCFunction>(setset_symmetric_difference_update_multi), METH_VARARGS, ""},
  {"quotient", reinterpret_cast<PyCFunction>(setset_quotient), METH_O, ""},
  {"quotient_update", reinterpret_cast<PyCFunction>(setset_quotient_update), METH_O, ""},
  {"remainder", reinterpret_cast<PyCFunction>(setset_remainder), METH_O, ""},
  {"remainder_update", reinterpret_cast<PyCFunction>(setset_remainder_update), METH_O, ""},
  {"isdisjoint", reinterpret_cast<PyCFunction>(setset_isdisjoint), METH_O, ""},
  {"issubset", reinterpret_cast<PyCFunction>(setset_issubset), METH_O, ""},
  {"issuperset", reinterpret_cast<PyCFunction>(setset_issuperset), METH_O, ""},
  {"len", reinterpret_cast<PyCFunction>(setset_len2), METH_VARARGS, ""},
  {"iter", reinterpret_cast<PyCFunction>(setset_iter), METH_NOARGS, ""},
  {"rand_iter", reinterpret_cast<PyCFunction>(setset_rand_iter), METH_NOARGS, ""},
  {"max_iter", reinterpret_cast<PyCFunction>(setset_max_iter), METH_O, ""},
  {"min_iter", reinterpret_cast<PyCFunction>(setset_min_iter), METH_O, ""},
  {"add", reinterpret_cast<PyCFunction>(setset_add), METH_O, ""},
  {"remove", reinterpret_cast<PyCFunction>(setset_remove), METH_O, ""},
  {"discard", reinterpret_cast<PyCFunction>(setset_discard), METH_O, ""},
  {"pop", reinterpret_cast<PyCFunction>(setset_pop), METH_NOARGS, ""},
  {"clear", reinterpret_cast<PyCFunction>(setset_clear), METH_NOARGS, ""},
  {"minimal", reinterpret_cast<PyCFunction>(setset_minimal), METH_NOARGS, ""},
  {"maximal", reinterpret_cast<PyCFunction>(setset_maximal), METH_NOARGS, ""},
  {"hitting", reinterpret_cast<PyCFunction>(setset_hitting), METH_NOARGS, ""},
  {"smaller", reinterpret_cast<PyCFunction>(setset_smaller), METH_O, ""},
  {"larger", reinterpret_cast<PyCFunction>(setset_larger), METH_O, ""},
  {"set_size", reinterpret_cast<PyCFunction>(setset_set_size), METH_O, ""},
  {"flip", reinterpret_cast<PyCFunction>(setset_flip), METH_VARARGS, ""},
  {"join", reinterpret_cast<PyCFunction>(setset_join), METH_O, ""},
  {"meet", reinterpret_cast<PyCFunction>(setset_meet), METH_O, ""},
  {"subsets", reinterpret_cast<PyCFunction>(setset_subsets), METH_O, ""},
  {"supersets", reinterpret_cast<PyCFunction>(setset_supersets), METH_O, ""},
  {"non_subsets", reinterpret_cast<PyCFunction>(setset_non_subsets), METH_O, ""},
  {"non_supersets", reinterpret_cast<PyCFunction>(setset_non_supersets), METH_O, ""},
  {"choice", reinterpret_cast<PyCFunction>(setset_choice), METH_NOARGS, ""},
  {"probability", reinterpret_cast<PyCFunction>(setset_probability), METH_O, ""},
  {"dump", reinterpret_cast<PyCFunction>(setset_dump), METH_O, ""},
  {"dumps", reinterpret_cast<PyCFunction>(setset_dumps), METH_NOARGS, ""},
  {"_enum", reinterpret_cast<PyCFunction>(setset_enum), METH_O, ""},
  {"_enums", reinterpret_cast<PyCFunction>(setset_enums), METH_NOARGS, ""},
  {"cost_le", reinterpret_cast<PyCFunction>(setset_cost_le), METH_VARARGS | METH_KEYWORDS, ""},
  {"remove_some_element", reinterpret_cast<PyCFunction>(setset_remove_some_element), METH_NOARGS, ""},
  {"add_some_element", reinterpret_cast<PyCFunction>(setset_add_some_element), METH_VARARGS, ""},
  {"remove_add_some_elements", reinterpret_cast<PyCFunction>(setset_remove_add_some_elements), METH_VARARGS, ""},
  {"to_vertexsetset", reinterpret_cast<PyCFunction>(setset_to_vertexsetset), METH_VARARGS, ""},
  {NULL}  /* Sentinel */
};

static PyNumberMethods setset_as_number = {
  0,                                  /*nb_add*/
  reinterpret_cast<binaryfunc>(setset_difference), /*nb_subtract*/
  0,                                  /*nb_multiply*/
#if IS_PY3 == 0
  reinterpret_cast<binaryfunc>(setset_quotient), /*nb_divide*/
#endif
  reinterpret_cast<binaryfunc>(setset_remainder), /*nb_remainder*/
  0,                                  /*nb_divmod*/
  0,                                  /*nb_power*/
  0,                                  /*nb_negative*/
  0,                                  /*nb_positive*/
  0,                                  /*nb_absolute*/
  reinterpret_cast<inquiry>(setset_nonzero), /*nb_nonzero or nb_bool*/
  reinterpret_cast<unaryfunc>(setset_invert), /*nb_invert*/
  0,                                  /*nb_lshift*/
  0,                                  /*nb_rshift*/
  reinterpret_cast<binaryfunc>(setset_intersection), /*nb_and*/
  reinterpret_cast<binaryfunc>(setset_symmetric_difference), /*nb_xor*/
  reinterpret_cast<binaryfunc>(setset_union), /*nb_or*/
#if IS_PY3 == 0
  0/*reinterpret_cast<coercion>(Py_TPFLAGS_CHECKTYPES)*/, /*nb_coerce*/
#endif
  0,                                  /*nb_int*/
  0,                                  /*nb_long or *nb_reserved*/
  0,                                  /*nb_float*/
#if IS_PY3 == 0
  0,                                  /*nb_oct*/
  0,                                  /*nb_hex*/
#endif
  0,                                  /*nb_inplace_add*/
  reinterpret_cast<binaryfunc>(setset_difference_update), /*nb_inplace_subtract*/
  0,                                  /*nb_inplace_multiply*/
#if IS_PY3 == 0
  reinterpret_cast<binaryfunc>(setset_quotient_update), /*nb_inplace_divide*/
#endif
  reinterpret_cast<binaryfunc>(setset_remainder_update), /*nb_inplace_remainder*/
  0,                                  /*nb_inplace_power*/
  0,                                  /*nb_inplace_lshift*/
  0,                                  /*nb_inplace_rshift*/
  reinterpret_cast<binaryfunc>(setset_intersection_update), /*nb_inplace_and*/
  reinterpret_cast<binaryfunc>(setset_symmetric_difference_update), /*nb_inplace_xor*/
  reinterpret_cast<binaryfunc>(setset_update), /*nb_inplace_or*/
#if IS_PY3 == 1
  0, /*nb_floor_divide*/
  reinterpret_cast<binaryfunc>(setset_quotient), /*nb_true_divide*/
  0, /*nb_inplace_floor_divide*/
  reinterpret_cast<binaryfunc>(setset_quotient_update), /*nb_inplace_true_divide*/
  0 /*nb_index*/
// for 3.5?
//  0, /*nb_matrix_multiply*/
//  0 /*nb_inplace_matrix_multiply*/
#endif
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

PyDoc_STRVAR(setset_doc,
"Hidden class to implement graphillion classes.\n\
\n\
A setset object stores a set of sets.  A set element must be a\n\
positive number.");

#ifdef WIN32
__declspec(dllexport)
#endif
PyTypeObject PySetset_Type = {
  PyVarObject_HEAD_INIT(NULL, 0)
  "_graphillion.setset",              /*tp_name*/
  sizeof(PySetsetObject),             /*tp_basicsize*/
  0,                                  /*tp_itemsize*/
  reinterpret_cast<destructor>(setset_dealloc), /*tp_dealloc*/
  0,                                  /*tp_print*/
  0,                                  /*tp_getattr*/
  0,                                  /*tp_setattr*/
#if IS_PY3 == 1
  0,                                  /*tp_reserved at 3.4*/
#else
  setset_nocmp,                       /*tp_compare*/
#endif
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
  setset_doc,                         /* tp_doc */
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
#ifdef IS_PY3
  0, /* tp_free */
  0, /* tp_is_gc */
  0, /* *tp_bases */
  0, /* *tp_mro */
  0, /* *tp_cache */
  0, /* *tp_subclasses */
  0, /* *tp_weaklist */
  0, /* tp_version_tag */
  0, /* tp_finalize */
#endif
};

static PyObject* setset_elem_limit(PyObject*) {
  return PyInt_FromLong(setset::elem_limit());
}

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

// return true if success
static bool translate_graph(PyObject* graph_obj,
                            vector<pair<string, string> >& graph) {
  if (graph_obj == NULL || graph_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no graph");
    return false;
  }
  PyObject* i = PyObject_GetIter(graph_obj);
  if (i == NULL) return false;
  PyObject* eo;
  while ((eo = PyIter_Next(i))) {
    PyObject* j = PyObject_GetIter(eo);
    if (j == NULL) return false;
    vector<string> e;
    PyObject* vo;
    while ((vo = PyIter_Next(j))) {
      if (!PyBytes_Check(vo)) {
        PyErr_SetString(PyExc_TypeError, "invalid graph");
        return false;
      }
      string v = PyBytes_AsString(vo);
      if (v.find(',') != string::npos) {
        PyErr_SetString(PyExc_TypeError, "invalid vertex in the graph");
        return false;
      }
      e.push_back(v);
    }
    assert(e.size() == 2);
    graph.push_back(make_pair(e[0], e[1]));
  }
  return true;
}

static PyObject* graphset_graphs(PyObject*, PyObject* args, PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "vertex_groups";
  static char s3[] = "degree_constraints";
  static char s4[] = "num_edges";
  static char s5[] = "num_comps";
  static char s6[] = "no_loop";
  static char s7[] = "search_space";
  static char s8[] = "linear_constraints";
  static char* kwlist[9] = {s1, s2, s3, s4, s5, s6, s7, s8, NULL};
  PyObject* graph_obj = NULL;
  PyObject* vertex_groups_obj = NULL;
  PyObject* degree_constraints_obj = NULL;
  PyObject* num_edges_obj = NULL;
  int num_comps = -1, no_loop = 0;
  PyObject* search_space_obj = NULL;
  PyObject* linear_constraints_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OOOiiOO", kwlist, &graph_obj,
                                   &vertex_groups_obj, &degree_constraints_obj,
                                   &num_edges_obj, &num_comps, &no_loop,
                                   &search_space_obj, &linear_constraints_obj))
    return NULL;

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  vector<vector<string> > vertex_groups_entity;
  vector<vector<string> >* vertex_groups = NULL;
  if (vertex_groups_obj != NULL && vertex_groups_obj != Py_None) {
    vertex_groups = &vertex_groups_entity;
    PyObject* i = PyObject_GetIter(vertex_groups_obj);
    if (i == NULL) return NULL;
    PyObject* uo;
    while ((uo = PyIter_Next(i))) {
      PyObject* j = PyObject_GetIter(uo);
      if (j == NULL) return NULL;
      vector<string> v;
      PyObject* vo;
      while ((vo = PyIter_Next(j))) {
        if (!PyBytes_Check(vo)) {
          PyErr_SetString(PyExc_TypeError, "invalid vertex groups");
          return NULL;
        }
        string vertex = PyBytes_AsString(vo);
        v.push_back(vertex);
      }
      vertex_groups->push_back(v);
    }
  }

  map<string, Range> degree_constraints_entity;
  map<string, Range>* degree_constraints = NULL;
  if (degree_constraints_obj != NULL && degree_constraints_obj != Py_None) {
    degree_constraints = &degree_constraints_entity;
    PyObject* vo;
    PyObject* lo;
    Py_ssize_t pos = 0;
    while (PyDict_Next(degree_constraints_obj, &pos, &vo, &lo)) {
      if (!PyBytes_Check(vo)) {
        PyErr_SetString(PyExc_TypeError, "invalid vertex in degree constraints");
        return NULL;
      }
      string vertex = PyBytes_AsString(vo);
      PyObject* i = PyObject_GetIter(lo);
      if (i == NULL) return NULL;
      vector<int> r;
      PyObject* io;
      while ((io = PyIter_Next(i))) {
        if (!PyInt_Check(io)) {
          Py_DECREF(io);
          PyErr_SetString(PyExc_TypeError, "invalid degree in degree constraints");
          return NULL;
        }
        r.push_back(PyInt_AsLong(io));
      }
      (*degree_constraints)[vertex] = Range(r[0], r[1], r[2]);
    }
  }

  Range num_edges_entity;
  Range* num_edges = NULL;
  if (num_edges_obj != NULL && num_edges_obj != Py_None) {
    num_edges = &num_edges_entity;
    vector<int> r;
    PyObject* i = PyObject_GetIter(num_edges_obj);
    PyObject* io;
    while ((io = PyIter_Next(i))) {
      if (!PyInt_Check(io)) {
        Py_DECREF(io);
        PyErr_SetString(PyExc_TypeError, "invalid number of edges");
        return NULL;
      }
      r.push_back(PyInt_AsLong(io));
    }
    *num_edges = Range(r[0], r[1], r[2]);
  }

  setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None)
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;

  vector<linear_constraint_t> linear_constraints_entity;
  vector<linear_constraint_t>* linear_constraints = NULL;
  if (linear_constraints_obj != NULL && linear_constraints_obj != Py_None) {
    linear_constraints = &linear_constraints_entity;
    PyObject* i = PyObject_GetIter(linear_constraints_obj);
    if (i == NULL) return NULL;
    PyObject* co;
    while ((co = PyIter_Next(i))) {
      linear_constraints->push_back(linear_constraint_t());
      linear_constraint_t& c = linear_constraints->back();
      vector<weighted_edge_t>& expr = c.first;
      pair<double,double>& range = c.second;
      PyObject* lo = PySequence_GetItem(co, 0);
      if (lo == NULL) return NULL;
      PyObject* j = PyObject_GetIter(lo);
      if (j == NULL) return NULL;
      PyObject* eo;
      while ((eo = PyIter_Next(j))) {
        PyObject* uo = PySequence_GetItem(eo, 0);
        if (uo == NULL || !PyBytes_Check(uo)) return NULL;
        string u = PyBytes_AsString(uo);
        PyObject* vo = PySequence_GetItem(eo, 1);
        if (vo == NULL || !PyBytes_Check(vo)) return NULL;
        string v = PyBytes_AsString(vo);
        PyObject* wo = PySequence_GetItem(eo, 2);
        if (wo == NULL || !PyFloat_Check(wo)) return NULL;
        double w = PyFloat_AsDouble(wo);
        expr.push_back(make_pair(make_pair(u, v), w));
      }
      PyObject* ro = PySequence_GetItem(co, 1);
      if (ro == NULL) return NULL;
      PyObject* r0o = PySequence_GetItem(ro, 0);
      if (r0o == NULL || !PyFloat_Check(r0o)) return NULL;
      range.first = PyFloat_AsDouble(r0o);
      PyObject* r1o = PySequence_GetItem(ro, 1);
      if (r1o == NULL || !PyFloat_Check(r1o)) return NULL;
      range.second = PyFloat_AsDouble(r1o);
    }
  }

  setset ss = SearchGraphs(graph, vertex_groups, degree_constraints, num_edges,
                           num_comps, no_loop, search_space,
                           linear_constraints);

  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graphset_show_messages(PySetsetObject* self, PyObject* obj) {
  int ret = graphillion::ShowMessages(PyObject_IsTrue(obj));
  if (ret) Py_RETURN_TRUE;
  else Py_RETURN_FALSE;
}

static PyObject* regular_graphs(PyObject*, PyObject* args, PyObject* kwds){
  static char s1[] = "graph";
  static char s2[] = "degree";
  static char s3[] = "is_connected";
  static char s4[] = "graphset";
  static char* kwlist[5] = {s1, s2, s3, s4, NULL};
  PyObject* graph_obj = NULL;
  PyObject* degree_obj = NULL;
  PyObject* is_connected_obj = NULL;
  PyObject* graphset_obj = NULL;
  int degree_lower = 0;
  int degree_upper = INT_MAX;
  if(!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|O", kwlist, &graph_obj, &degree_obj, &is_connected_obj, &graphset_obj)){
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  if (PyInt_Check(degree_obj)) {
    int d = PyLong_AsLong(degree_obj);
    degree_lower = d;
    degree_upper = d;
  } else if (PyTuple_Check(degree_obj)) {
    Py_ssize_t tuple_size = PyTuple_Size(degree_obj);
    if (tuple_size != 2) {
      PyErr_SetString(PyExc_TypeError, "tuple size must be 2");
      return NULL;
    }
    PyObject* lower_obj = PyTuple_GetItem(degree_obj, 0);
    if (PyLong_Check(lower_obj)) {
      degree_lower = PyLong_AsLong(lower_obj);
    } else {
      PyErr_SetString(PyExc_TypeError, "degree lower must be integer");
      return NULL;
    }
    PyObject* upper_obj = PyTuple_GetItem(degree_obj, 1);
    if (PyLong_Check(upper_obj)) {
      degree_upper = PyLong_AsLong(upper_obj);
    } else {
      PyErr_SetString(PyExc_TypeError, "degree upper must be an integer");
      return NULL;
    }
  } else {
    PyErr_SetString(PyExc_TypeError, "degree must be an integer or a tuple");
    return NULL;
  }

  if (!PyBool_Check(is_connected_obj)) {
    PyErr_SetString(PyExc_TypeError, "is_connected is not bool");
    return NULL;
  }
  bool is_connected = (is_connected_obj != Py_False);

  setset* search_space = NULL;
  if (graphset_obj != NULL && graphset_obj != Py_None) {
    search_space = reinterpret_cast<PySetsetObject*>(graphset_obj)->ss;
  }

  auto ss = graphillion::SearchRegularGraphs(graph,
              degree_lower, degree_upper,
              is_connected, search_space);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* odd_edges_subgraphs(PyObject*, PyObject* args, PyObject* kwds) {
  static char s1[] = "graph";
  static char* kwlist[2] = {s1, NULL};
  PyObject* graph_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &graph_obj)) {
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  auto ss = graphillion::SearchOddEdgeSubgraphs(graph);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* degree_distribution_graphs(PyObject*, PyObject* args, PyObject* kwds) {

  static char s1[] = "graph";
  static char s2[] = "deg_dist";
  static char s3[] = "is_connected";
  static char s4[] = "graphset";
  static char* kwlist[5] = {s1, s2, s3, s4, NULL};

  PyObject* graph_obj = NULL;
  PyObject* deg_dist = NULL;
  PyObject* is_connected = NULL;
  PyObject* graphset_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "OOO|O", kwlist, &graph_obj,
                                   &deg_dist, &is_connected, &graphset_obj)) {
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  PyObject* key, *value;
  Py_ssize_t pos = 0;

  std::vector<int> deg_ranges;
  while (PyDict_Next(deg_dist, &pos, &key, &value)) {
    if (!PyInt_Check(key)) {
      PyErr_SetString(PyExc_TypeError, "key must be an integer.");
      return NULL;
    }
    if (PyInt_Check(value)) {
      int k = PyInt_AsLong(key);
      int v = PyInt_AsLong(value);
      if (static_cast<int>(deg_ranges.size()) <= k) {
        deg_ranges.resize(k + 1);
      }
      deg_ranges[k] = v;
    } else {
      PyErr_SetString(PyExc_TypeError, "Currently, value must be an integer.");
      return NULL;
    }
  }

  if (!PyBool_Check(is_connected)) {
    PyErr_SetString(PyExc_TypeError, "not bool");
    return NULL;
  }

  setset* search_space = NULL;
  if (graphset_obj != NULL && graphset_obj != Py_None) {
    search_space = reinterpret_cast<PySetsetObject*>(graphset_obj)->ss;
  }

  auto ss = graphillion::SearchDegreeDistributionGraphs(graph, deg_ranges,
    is_connected != Py_False, search_space);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* graph_partitions(PyObject*, PyObject* args, PyObject* kwds){
  static char s1[] = "graph";
  static char s2[] = "num_comp_lb";
  static char s3[] = "num_comp_ub";
  static char* kwlist[4] = {s1, s2, s3, NULL};
  PyObject* graph_obj = NULL;
  int num_comp_lb = 1;
  int num_comp_ub = std::numeric_limits<int16_t>::max();
  if(!PyArg_ParseTupleAndKeywords(args, kwds, "O|ii", kwlist, &graph_obj, &num_comp_lb, &num_comp_ub)){
    return NULL;
  }
  if (num_comp_lb < 1){
    PyErr_SetString(PyExc_TypeError, "not positive integer");
    return NULL;
  }
  if (num_comp_ub < num_comp_lb) {
    PyErr_SetString(PyExc_TypeError, "lower bound is larger than upper bound");
    return NULL;
  }
  if(std::numeric_limits<int16_t>::max() < num_comp_ub){
    PyErr_SetString(PyExc_TypeError, "too many components");
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  auto ss = graphillion::SearchPartitions(graph, num_comp_lb, num_comp_ub);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* balanced_partitions(PyObject*, PyObject* args, PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "weight_list";
  static char s3[] = "ratio";
  static char s4[] = "lower";
  static char s5[] = "upper";
  static char s6[] = "num_comps";
  static char* kwlist[7] = {s1, s2, s3, s4, s5, s6, NULL};

  PyObject* graph_obj = NULL;
  PyObject* weight_list_obj = NULL;
  double ratio = 0;
  uint32_t lower = 0, upper = std::numeric_limits<weight_t>::max() / 4;
  int num_comps = -1;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OdIIi", kwlist, &graph_obj,
                                   &weight_list_obj, &ratio, &lower, &upper,
                                   &num_comps)) {
    return NULL;
  }
  if (num_comps != -1 && num_comps < 1) {
    PyErr_SetString(PyExc_TypeError, "not positive integer");
    return NULL;
  }
  if (upper < lower) {
    PyErr_SetString(PyExc_TypeError, "lower bound is larger than upper bound");
    return NULL;
  }
  if (ratio != 0 && ratio < 1.0) {
    PyErr_SetString(PyExc_TypeError, "ratio is less than 1.0");
    return NULL;
  }
  if (std::numeric_limits<int16_t>::max() < num_comps) {
    PyErr_SetString(PyExc_TypeError, "too many components");
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  map<string, uint32_t> weight_list;
  if (weight_list_obj != NULL && weight_list_obj != Py_None) {
    PyObject* keyObject;
    PyObject* valObject;
    Py_ssize_t pos = 0;
    while (PyDict_Next(weight_list_obj, &pos, &keyObject, &valObject)) {
      if (!PyBytes_Check(keyObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid vertex in weight list");
        return NULL;
      }
      string vertex = PyBytes_AsString(keyObject);
      if (!PyInt_Check(valObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid weight in weight list");
        return NULL;
      }
      uint32_t weight = PyInt_AsLong(valObject);
      weight_list[vertex] = weight;
    }
  }

  auto ss = graphillion::SearchBalancedPartitions(graph, weight_list, ratio, lower,
                                             upper, num_comps);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* induced_graphs(PyObject*, PyObject* args, PyObject* kwds){
  static char s1[] = "graph";
  static char* kwlist[2] = {s1, NULL};

  PyObject* graph_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &graph_obj)) {
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  auto ss = graphillion::SearchInducedGraphs(graph);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* weighted_induced_graphs(PyObject*, PyObject* args,
                                         PyObject* kwds){
  static char s1[] = "graph";
  static char s2[] = "weight_list";
  static char s3[] = "lower";
  static char s4[] = "upper";
  static char* kwlist[5] = {s1, s2, s3, s4, NULL};

  PyObject* graph_obj = NULL;
  PyObject* weight_list_obj = NULL;
  uint32_t lower = 0, upper = std::numeric_limits<uint32_t>::max() / 2;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OII", kwlist, &graph_obj,
                                   &weight_list_obj, &lower, &upper)) {
    return NULL;
  }
  if (upper < lower) {
    PyErr_SetString(PyExc_TypeError, "lower bound is larger than upper bound");
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  std::map<std::string, uint32_t> weight_list;
  if (weight_list_obj != NULL && weight_list_obj != Py_None) {
    PyObject* keyObject;
    PyObject* valObject;
    Py_ssize_t pos = 0;
    while (PyDict_Next(weight_list_obj, &pos, &keyObject, &valObject)) {
      if (!PyBytes_Check(keyObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid vertex in weight list");
        return NULL;
      }
      string vertex = PyBytes_AsString(keyObject);
      if (!PyInt_Check(valObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid weight in weight list");
        return NULL;
      }
      uint32_t weight = PyInt_AsLong(valObject);
      weight_list[vertex] = weight;
    }
  }

  auto ss = graphillion::SearchWeightedInducedGraphs(graph, weight_list, lower,
                                                     upper);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* forbidden_induced_subgraphs(PyObject*, PyObject* args, PyObject* kwds){
  static char s1[] = "graph";
  static char s2[] = "graphset";
  static char* kwlist[3] = {s1, s2, NULL};

  PyObject* graph_obj = NULL;
  PyObject* graphset_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "OO", kwlist, &graph_obj, &graphset_obj)) {
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  if (graphset_obj == NULL || graphset_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "graphset none");
    return NULL;
  }

  auto ss = graphillion::SearchForbiddenInducedSubgraphs(graph,
    reinterpret_cast<PySetsetObject*>(graphset_obj)->ss);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* chordal_graphs(PyObject*, PyObject* args, PyObject* kwds){
  static char s1[] = "graph";
  static char* kwlist[2] = {s1, NULL};

  PyObject* graph_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O", kwlist, &graph_obj)) {
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  auto ss = graphillion::SearchChordals(graph);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  ret->ss = new setset(ss);
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* reliability(PyObject*, PyObject* args, PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "probabilities";
  static char s3[] = "terminals";
  static char* kwlist[4] = {s1, s2, s3, NULL};

  PyObject* graph_obj = NULL;
  PyObject* prob_list_obj = NULL;
  PyObject* terminals_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OO", kwlist, &graph_obj,
                                   &prob_list_obj, &terminals_obj)) {
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    return NULL;
  }

  vector<double> probabilities;
  if (prob_list_obj == NULL || prob_list_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no prob_list");
    return NULL;
  }
  {
    PyObject* i = PyObject_GetIter(prob_list_obj);
    if (i == NULL) return NULL;
    PyObject* p;
    while ((p = PyIter_Next(i))) {
      if (!PyFloat_Check(p)) {
        PyErr_SetString(PyExc_TypeError, "invalid probability");
        Py_DECREF(p);
        return NULL;
      }
      probabilities.push_back(PyFloat_AsDouble(p));
      Py_DECREF(p);
    }
    Py_DECREF(i);
  }

  if (terminals_obj == NULL || terminals_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no terminals");
    return NULL;
  }
  vector<string> terminals;
  {
    PyObject* i = PyObject_GetIter(terminals_obj);
    if (i == NULL) return NULL;
    PyObject* term;
    while ((term = PyIter_Next(i))) {
      if (!PyBytes_Check(term)) {
        PyErr_SetString(PyExc_TypeError, "invalid terminals");
        Py_DECREF(term);
        return NULL;
      }
      terminals.push_back(PyBytes_AsString(term));
      Py_DECREF(term);
    }
    Py_DECREF(i);
  }

  auto prob = graphillion::reliability(graph, probabilities, terminals);
  return PyFloat_FromDouble(prob);
}

static PyObject* setset_get_vertices_from_top(PySetsetObject* self, PyObject* args) {
  std::vector<std::vector<std::string>> edges = parse_args_to_edges(args);
  std::vector<std::string> v_order_from_top = VariableConverter::get_vertices_from_top(edges);
  int n = v_order_from_top.size();
  PyObject* ret = PyList_New(n);
  for (int i = 0; i < n; ++i) {
    PyObject* v = PyStr_FromString(v_order_from_top[i].c_str());
    PyList_SetItem(ret, i, v);
  }
  return ret;
}

static PyMethodDef module_methods[] = {
  {"load", reinterpret_cast<PyCFunction>(setset_load), METH_O, ""},
  {"loads", reinterpret_cast<PyCFunction>(setset_loads), METH_O, ""},
  {"_elem_limit", reinterpret_cast<PyCFunction>(setset_elem_limit), METH_NOARGS, ""},
  {"_num_elems", setset_num_elems, METH_VARARGS, ""},
  {"_graphs", reinterpret_cast<PyCFunction>(graphset_graphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_show_messages", reinterpret_cast<PyCFunction>(graphset_show_messages), METH_O, ""},
  {"_regular_graphs", reinterpret_cast<PyCFunction>(regular_graphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_odd_edges_subgraphs", reinterpret_cast<PyCFunction>(odd_edges_subgraphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_degree_distribution_graphs", reinterpret_cast<PyCFunction>(degree_distribution_graphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_partitions", reinterpret_cast<PyCFunction>(graph_partitions), METH_VARARGS | METH_KEYWORDS, ""},
  {"_balanced_partitions", reinterpret_cast<PyCFunction>(balanced_partitions), METH_VARARGS | METH_KEYWORDS, ""},
  {"_induced_graphs", reinterpret_cast<PyCFunction>(induced_graphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_weighted_induced_graphs", reinterpret_cast<PyCFunction>(weighted_induced_graphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_forbidden_induced_subgraphs", reinterpret_cast<PyCFunction>(forbidden_induced_subgraphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_chordal_graphs", reinterpret_cast<PyCFunction>(chordal_graphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_reliability", reinterpret_cast<PyCFunction>(reliability), METH_VARARGS | METH_KEYWORDS, ""},
  {"_get_vertices_from_top", reinterpret_cast<PyCFunction>(setset_get_vertices_from_top), METH_VARARGS, ""},
  {NULL}  /* Sentinel */
};

PyDoc_STRVAR(graphillion_doc,
"Hidden module to implement graphillion classes.");

#if IS_PY3 == 1
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT,
    "_graphillion",      /* m_name */
    graphillion_doc,     /* m_doc */
    -1,                  /* m_size */
    module_methods,      /* m_methods */
    NULL,                /* m_reload */
    NULL,                /* m_traverse */
    NULL,                /* m_clear */
    NULL,                /* m_free */
};
#endif
MODULE_INIT_FUNC(_graphillion) {
  PyObject* m;
  if (PyType_Ready(&PySetset_Type) < 0) return NULL;
  if (PyType_Ready(&PySetsetIter_Type) < 0) return NULL;
#if IS_PY3 == 1
  m = PyModule_Create(&moduledef);
#else
  m = Py_InitModule3("_graphillion", module_methods, graphillion_doc);
#endif
  if (m == NULL) return NULL;
  Py_INCREF(&PySetset_Type);
  Py_INCREF(&PySetsetIter_Type);
  PyModule_AddObject(m, "setset", reinterpret_cast<PyObject*>(&PySetset_Type));
  PyModule_AddObject(m, "setset_iterator",
                     reinterpret_cast<PyObject*>(&PySetsetIter_Type));
  return m;
}
