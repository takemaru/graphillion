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
#include <limits>

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

#define RETURN_NEW_OBJECT(py_type, expr)                         \
  do {                                                           \
    PySetsetObject* _ret = reinterpret_cast<PySetsetObject*>(    \
        (py_type)->tp_alloc((py_type), 0));                      \
    if (_ret == NULL) {                                          \
      PyErr_SetString(PyExc_MemoryError, "Failed to "            \
      "allocate memory for new setset object");                  \
      return NULL;                                               \
    }                                                            \
    try {                                                        \
      _ret->ss = new setset(expr);                               \
    } catch (const std::exception& e) {                          \
      Py_DECREF(_ret);                                           \
      Py_TYPE(_ret)->tp_free(reinterpret_cast<PyObject*>(_ret)); \
      PyErr_SetString(PyExc_RuntimeError, e.what());             \
      return NULL;                                               \
    } catch (...) {                                              \
      Py_DECREF(_ret);                                           \
      Py_TYPE(_ret)->tp_free(reinterpret_cast<PyObject*>(_ret)); \
      PyErr_SetString(PyExc_RuntimeError, "Unknown error "       \
        "occured while creating a new setset");                  \
      return NULL;                                               \
    }                                                            \
    return reinterpret_cast<PyObject*>(_ret);                    \
  } while (0);

#define RETURN_NEW_OBJECT2(py_type, other, _other, expr)                \
  do {                                                                  \
    PySetsetObject* _other = reinterpret_cast<PySetsetObject*>(other);  \
    RETURN_NEW_OBJECT((py_type), (expr));                               \
  } while (0);

#define RETURN_NEW_SETSET(expr)  RETURN_NEW_OBJECT(&PySetset_Type, expr)

#define RETURN_NEW_SETSET2(other, _other, expr)  \
    RETURN_NEW_OBJECT2(&PySetset_Type, other, _other, expr)

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
      if (_other == NULL) {                                             \
        Py_DECREF(_result);                                             \
        PyErr_SetString(PyExc_TypeError, "invalid tuple element");      \
        return NULL;                                                    \
      }                                                                 \
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
      if (_other == NULL) {                                            \
        PyErr_SetString(PyExc_TypeError, "invalid tuple element");     \
        return NULL;                                                   \
      }                                                                \
      if (expr(self, _other) == NULL)                                  \
        return NULL;                                                   \
    }                                                                  \
    Py_RETURN_NONE;                                                    \
  } while (0);

static PyObject* setset_build_set(const set<int>& s) {
  PyObject* so = PySet_New(NULL);
  if (so == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Failed to create Python set");
    return NULL;
  }
  for (set<int>::const_iterator e = s.begin(); e != s.end(); ++e) {
    PyObject* eo = PyLong_FromLong(*e);
    if (eo == NULL) {
      PyErr_SetString(PyExc_TypeError, "not int set");
      Py_DECREF(so);
      return NULL;
    }
    if (PySet_Add(so, eo) == -1) {
      PyErr_SetString(PyExc_RuntimeError, "can't add elements to a set");
      Py_DECREF(eo);
      Py_DECREF(so);
      return NULL;
    }
    Py_DECREF(eo);
  }
  return so;
}

static int setset_parse_set(PyObject* so, set<int>* s) {
  assert(s != NULL);
  PyObject* i = PyObject_GetIter(so);
  if (i == NULL) return -1;
  PyObject* eo;
  while ((eo = PyIter_Next(i))) {
    if (!PyLong_Check(eo)) {
      Py_DECREF(eo);
      Py_DECREF(i);
      PyErr_SetString(PyExc_TypeError, "not int set");
      return -1;
    }
    long value = PyLong_AsLong(eo);
    Py_DECREF(eo);
    // error occurs in PyLong_AsLong
    if (PyErr_Occurred()) {
      Py_DECREF(i);
      return -1;
    }
    s->insert(value);
  }
  // error occurs in PyIter_Next
  if (PyErr_Occurred()) {
    Py_DECREF(i);
    return -1;
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
    if (!PyUnicode_Check(key_obj)) {
      PyErr_SetString(PyExc_TypeError, "invalid argument");
      return -1;
    }
    const char* key_cstr = PyUnicode_AsUTF8(key_obj);
    if (key_cstr == NULL) {
      return -1;
    }
    std::string key(key_cstr);
    if (key != "include" && key != "exclude") {
      PyErr_SetString(PyExc_TypeError, "invalid dict key");
      return -1;
    }
    PyObject* i = PyObject_GetIter(lo);
    if (i == NULL) return -1;
    vector<int> v;
    PyObject* eo;
    while ((eo = PyIter_Next(i))) {
      if (!PyLong_Check(eo)) {
        Py_DECREF(eo);
        Py_DECREF(i);
        PyErr_SetString(PyExc_TypeError, "not int");
        return -1;
      }
      long value = PyLong_AsLong(eo);
      Py_DECREF(eo);

      // error occurs in PyLong_AsLong
      if (PyErr_Occurred()) {
        Py_DECREF(i);
        return -1;
      }

      v.push_back(value);
    }
    // error occurs in PyIter_Next
    if (PyErr_Occurred()) {
      Py_DECREF(i);
      return -1;
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
  if (self == NULL) {
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory for PysetsetIterObject");
    return NULL;
  }
  return reinterpret_cast<PyObject*>(self);
}

static void setsetiter_dealloc(PySetsetIterObject* self) {
  delete self->it;
  PyObject_Del(self);
}

static PyObject* setsetiter_next(PySetsetIterObject* self) {
  if (*(self->it) == setset::end()) {
    PyErr_SetNone(PyExc_StopIteration);
    return NULL;
  }
  set<int> s = *(*self->it);
  PyObject* py_set = setset_build_set(s);
  if (py_set == NULL) {
    return NULL;
  }
  ++(*self->it);
  return py_set;
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
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
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
  0, /* tp_free */
  0, /* tp_is_gc */
  0, /* *tp_bases */
  0, /* *tp_mro */
  0, /* *tp_cache */
  0, /* *tp_subclasses */
  0, /* *tp_weaklist */
  0, /* tp_version_tag */
  0, /* tp_finalize */
};

// setset

static PyObject* setset_new(PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/) {
  PySetsetObject* self;
  self = reinterpret_cast<PySetsetObject*>(type->tp_alloc(type, 0));
  if (self == NULL) {
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate memory for PySetsetObject");
    return NULL;
  }
  return reinterpret_cast<PyObject*>(self);
}

static int setset_init(PySetsetObject* self, PyObject* args, PyObject* kwds) {
  int num_elems_a = 0;
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "|Oi", &obj, &num_elems_a))
    return -1;

  try {
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
          Py_DECREF(o);
          Py_DECREF(i);
          PyErr_SetString(PyExc_TypeError, "not set");
          return -1;
        }
        set<int> s;
        if (setset_parse_set(o, &s) == -1) {
          Py_DECREF(o);
          Py_DECREF(i);
          return -1;
        }
        vs.push_back(s);
        Py_DECREF(o);
      }
      Py_DECREF(i);

      // error occurs in PyIter_Next
      if (PyErr_Occurred()) {
        return -1;
      }

      self->ss = new setset(vs);
    } else if (PyDict_Check(obj)) {
      map<string, vector<int> > m;
      if (setset_parse_map(obj, &m) == -1) return -1;
      self->ss = new setset(m, num_elems_a);
    } else {
      PyErr_SetString(PyExc_TypeError, "invalid argument");
      return -1;
    }
  } catch (...) {
    PyErr_SetString(PyExc_RuntimeError, "Unexpected error occurred "
    "during initialization");
  }
  return 0;
}

static void setset_dealloc(PySetsetObject* self) {
  delete self->ss;
  Py_TYPE(self)->tp_free(reinterpret_cast<PyObject*>(self));
}

static PyObject* setset_copy(PySetsetObject* self) {
  RETURN_NEW_OBJECT(Py_TYPE(self), *self->ss);
}

static PyObject* setset_invert(PySetsetObject* self) {
  RETURN_NEW_OBJECT(Py_TYPE(self), ~(*self->ss));
}

static PyObject* setset_complement(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyLong_Check, "int", NULL);
  int num_elems_a = PyLong_AsLong(io);
  if (PyErr_Occurred()) {
    return NULL;
  }
  if (num_elems_a < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->complement(num_elems_a));
}

static PyObject* setset_union(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, (*self->ss) | (*_other->ss));
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
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, (*self->ss) & (*_other->ss));
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
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, (*self->ss) - (*_other->ss));
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
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, (*self->ss) ^ (*_other->ss));
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
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, (*self->ss) / (*_other->ss));
}

static PyObject* setset_quotient_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) /= (*_other->ss));
}

static PyObject* setset_remainder(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, (*self->ss) % (*_other->ss));
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

static int setset_bool(PySetsetObject* self) {
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
  if (!PyArg_ParseTuple(args, "|O", &obj)) {
    PyErr_SetString(PyExc_TypeError, "invalid arguments. expected () or (int)");
    return NULL;
  }
  if (obj == NULL || obj == Py_None) {
    string size = self->ss->size();
    vector<char> buf;
    for (string::const_iterator c = size.begin(); c != size.end(); ++c)
      buf.push_back(*c);
    buf.push_back('\0');
    PyObject* result = PyLong_FromString(buf.data(), NULL, 0);
    if (result == NULL) {
      PyErr_SetString(PyExc_ValueError, "Failed to convert size to integer");
      return NULL;
    }
    return result;
  } else if (PyLong_Check(obj)) {
    int len = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
    RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->set_size(len));
  } else {
    PyErr_SetString(PyExc_TypeError, "not int");
    return NULL;
  }
}

static PyObject* setset_iter(PySetsetObject* self) {
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  try {
    ssi->it = new setset::iterator(self->ss->begin());
  } catch (const std::exception& e) {
    PyObject_Del(ssi);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    PyObject_Del(ssi);
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occured "
      "while creating iterator");
    return NULL;
  }
  if (ssi->it == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_rand_iter(PySetsetObject* self) {
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  ssi->it = NULL;

  try {
    ssi->it = new setset::random_iterator(self->ss->begin_randomly());
  } catch (const std::exception& e) {
    PyObject_Del(ssi);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    PyObject_Del(ssi);
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occured "
      "while creating iterator");
    return NULL;
  }
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
  PyObject* eo = NULL;
  vector<double> w;
  while ((eo = PyIter_Next(i))) {
    if (PyFloat_Check(eo)) {
      w.push_back(PyFloat_AsDouble(eo));
    }
    else if (PyLong_Check(eo)) {
      long value = PyLong_AsLong(eo);
      if (PyErr_Occurred()) {
        return NULL;
      }
      w.push_back(static_cast<double>(value));
    }
    else {
      Py_DECREF(eo);
      Py_DECREF(i);
      PyErr_SetString(PyExc_TypeError, "not a number");
      return NULL;
    }
    Py_DECREF(eo);
  }
  // error occurs in PyIter_Next
  if (PyErr_Occurred()) {
    Py_DECREF(i);
    return NULL;
  }
  Py_DECREF(i);
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == NULL) {
    PyErr_NoMemory();
    return NULL;
  }

  try {
    ssi->it = new setset::weighted_iterator(
        is_maximizing ? self->ss->begin_from_max(w) : self->ss->begin_from_min(w));
  } catch (const std::exception& e) {
    PyObject_Del(ssi);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    PyObject_Del(ssi);
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occured "
      "while creating iterator");
    return NULL;
  }
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
    if (setset_parse_set(obj, &s) == -1) {
      PyErr_SetString(PyExc_TypeError, "Failed to parse the set");
      return -1;
    }
    return self->ss->find(s) != self->ss->end() ? 1 : 0;
  } else if (PyLong_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return -1;
    }
    return self->ss->supersets(e) != setset() ? 1 : 0;
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return -1;
  }
}

static PyObject* setset_add(PySetsetObject* self, PyObject* obj) {
  if (PyAnySet_Check(obj)) {
    set<int> s;
    if (setset_parse_set(obj, &s) == -1) {
      PyErr_SetString(PyExc_TypeError, "Failed to parse the set");
      return NULL;
    }
    self->ss->insert(s);
  } else if (PyLong_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
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
    if (setset_parse_set(obj, &s) == -1) {
      PyErr_SetString(PyExc_TypeError, "Failed to parse the set");
      return NULL;
    }
    if (self->ss->erase(s) == 0) {
      PyErr_SetString(PyExc_KeyError, "not found");
      return NULL;
    }
  } else if (PyLong_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
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
    if (setset_parse_set(obj, &s) == -1) {
      PyErr_SetString(PyExc_TypeError, "Failed to parse the set");
      return NULL;
    }
    self->ss->erase(s);
  } else if (PyLong_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
    self->ss->erase(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not set nor int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_pop(PySetsetObject* self) {
  try {
    setset::iterator i = self->ss->begin();
    if (i == self->ss->end()) {
      PyErr_SetString(PyExc_KeyError, "'pop' from an empty set");
      return NULL;
    }
    set<int> s = *i;
    self->ss->erase(s);
    PyObject* result = setset_build_set(s);
    if (result == NULL) {
      PyErr_SetString(PyExc_RuntimeError, "Failed to build Python set");
      return NULL;
    }
    return result;
  } catch (const std::exception& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occured "
      "in setset_pop");
    return NULL;
  }
}

static PyObject* setset_clear(PySetsetObject* self) {
  self->ss->clear();
  Py_RETURN_NONE;
}

static PyObject* setset_flip(PySetsetObject* self, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
  if (obj == NULL || obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "need arg e");
    return NULL;
  } else if (PyLong_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
    self->ss->flip(e);
  } else {
    PyErr_SetString(PyExc_TypeError, "not int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_flip_all(PySetsetObject* self, PyObject* args) {
  PyObject* obj = NULL;
  if (!PyArg_ParseTuple(args, "O", &obj)) return NULL;
  if (obj == NULL || obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "need arg num_elems");
    return NULL;
  } else if (PyLong_Check(obj)) {
    long num_elems = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
    self->ss->flip_all(static_cast<int>(num_elems));
  } else {
    PyErr_SetString(PyExc_TypeError, "not int");
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_minimal(PySetsetObject* self) {
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->minimal());
}

static PyObject* setset_maximal(PySetsetObject* self) {
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->maximal());
}

static PyObject* setset_hitting(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyLong_Check, "int", NULL);
  int num_elems_a = PyLong_AsLong(io);
  if (PyErr_Occurred()) {
    return NULL;
  }
  if (num_elems_a < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->hitting(num_elems_a));
}

static PyObject* setset_smaller(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyLong_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (PyErr_Occurred()) {
    return NULL;
  }
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->smaller(set_size));
}

static PyObject* setset_larger(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyLong_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (PyErr_Occurred()) {
    return NULL;
  }
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->larger(set_size));
}

static PyObject* setset_set_size(PySetsetObject* self, PyObject* io) {
  CHECK_OR_ERROR(io, PyLong_Check, "int", NULL);
  int set_size = PyLong_AsLong(io);
  if (PyErr_Occurred()) {
    return NULL;
  }
  if (set_size < 0) {
    PyErr_SetString(PyExc_ValueError, "not unsigned int");
    return NULL;
  }
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->set_size(set_size));
}

static PyObject* setset_join(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, self->ss->join(*_other->ss));
}

static PyObject* setset_meet(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, self->ss->meet(*_other->ss));
}

static PyObject* setset_subsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, self->ss->subsets(*_other->ss));
}

static PyObject* setset_supersets(PySetsetObject* self, PyObject* obj) {
  if (PySetset_Check(obj)) {
    RETURN_NEW_OBJECT2(Py_TYPE(self), obj, _obj, self->ss->supersets(*_obj->ss));
  } else if (PyLong_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
    RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->supersets(e));
  } else {
    PyErr_SetString(PyExc_TypeError, "not setset nor int");
    return NULL;
  }
}

static PyObject* setset_non_subsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_OBJECT2(Py_TYPE(self), other, _other, self->ss->non_subsets(*_other->ss));
}

static PyObject* setset_non_supersets(PySetsetObject* self, PyObject* obj) {
  if (PySetset_Check(obj)) {
    RETURN_NEW_OBJECT2(Py_TYPE(self), obj, _obj, self->ss->non_supersets(*_obj->ss));
  } else if (PyLong_Check(obj)) {
    int e = PyLong_AsLong(obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
    RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->non_supersets(e));
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
                                    PyObject* args) {
  int num_elems_a = -1;
  PyObject* probabilities;

  if (!PyArg_ParseTuple(args, "iO", &num_elems_a, &probabilities)) {
    return NULL;
  }

  if (num_elems_a < 0) {
    PyErr_SetString(PyExc_TypeError, "num_elems must be a non-negative integer");
    return NULL;
  }

  if (!PyList_Check(probabilities)) {
    PyErr_SetString(PyExc_TypeError, "Probabilities must be a list");
    return NULL;
  }

  PyObject* i = PyObject_GetIter(probabilities);
  if (i == NULL) return NULL;
  PyObject* eo;
  vector<double> p;
  while ((eo = PyIter_Next(i))) {
    if (PyFloat_Check(eo)) {
      p.push_back(PyFloat_AsDouble(eo));
    }
    else if (PyLong_Check(eo)) {
      long value = PyLong_AsLong(eo);
      if (PyErr_Occurred()) {
        Py_DECREF(eo);
        Py_DECREF(i);
        return NULL;
      }
      p.push_back(static_cast<double>(value));
    }
    else {
      PyErr_SetString(PyExc_TypeError, "not a number");
      Py_DECREF(eo);
      Py_DECREF(i);
      return NULL;
    }
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  return PyFloat_FromDouble(self->ss->probability(p, num_elems_a));
}

static PyObject* setset_dump(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyObject_AsFileDescriptor, "file", NULL);
  int fd = PyObject_AsFileDescriptor(obj);
  if (fd == -1) {
    return NULL;
  }
  FILE* fp = fdopen(dup(fd), "w");
  if (fp == NULL) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  Py_BEGIN_ALLOW_THREADS;
  self->ss->dump(fp);
  Py_END_ALLOW_THREADS;
  if (fclose(fp) != 0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_dumps(PySetsetObject* self) {
  stringstream sstr;
  self->ss->dump(sstr);
  return PyUnicode_FromString(sstr.str().c_str());
}

static PyObject* setset_load(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyObject_AsFileDescriptor, "file", NULL);
  int fd = PyObject_AsFileDescriptor(obj);
  if (fd == -1) {
    return NULL;
  }
  FILE* fp = fdopen(dup(fd), "r");
  if (fp == NULL) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  if (ret == NULL) {
    fclose(fp);
    PyErr_NoMemory();
    return NULL;
  }
  try {
    setset* loaded_ss;
    Py_BEGIN_ALLOW_THREADS;
    loaded_ss = new setset(setset::load(fp));
    Py_END_ALLOW_THREADS;
    ret->ss = loaded_ss;
  } catch (const std::exception& e) {
    Py_DECREF(ret);
    fclose(fp);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    Py_DECREF(ret);
    fclose(fp);
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occurred");
    return NULL;
  }

  if (fclose(fp) != 0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* setset_loads(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyUnicode_Check, "str", NULL);
  const char* utf8_str = PyUnicode_AsUTF8(obj);
  if (utf8_str == NULL) {
    return NULL;
  }
  stringstream sstr(utf8_str);
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  if (ret == NULL) {
    PyErr_NoMemory();
    return NULL;
  }
  try {
    ret->ss = new setset(setset::load(sstr));
  } catch (const std::exception& e) {
    Py_DECREF(ret);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    Py_DECREF(ret);
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occurred");
    return NULL;
  }
  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* setset_enum(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyObject_AsFileDescriptor, "file", NULL);
  int fd = PyObject_AsFileDescriptor(obj);
  if (fd == -1) {
    return NULL;
  }
  FILE* fp = fdopen(fd, "r");
  if (fp == NULL) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  string name = Py_TYPE(self)->tp_name;
  string prefix = name + "([";
  auto outer_pair = std::make_pair(prefix.c_str(), "])");
  auto inner_pair = std::make_pair("set([", "])");
  try {
    Py_BEGIN_ALLOW_THREADS;
    self->ss->_enum(fp, outer_pair, inner_pair);
    Py_END_ALLOW_THREADS;
  } catch (const std::exception& e) {
    fclose(fp);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    fclose(fp);
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occurred");
    return NULL;
  }

  if (fclose(fp) != 0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_enums(PySetsetObject* self) {
  stringstream sstr;
  string name = Py_TYPE(self)->tp_name;
  string prefix = name + "([";
  auto outer_pair = std::make_pair(prefix.c_str(), "])");
  auto inner_pair = std::make_pair("set([", "])");
  try {
    self->ss->_enum(sstr, outer_pair, inner_pair);
  } catch (const std::exception& e) {
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occurred");
    return NULL;
  }
  PyObject* result = PyUnicode_FromString(sstr.str().c_str());
  if (result == NULL) {
    PyErr_SetString(PyExc_RuntimeError, "Failed to create Unicode string");
    return NULL;
  }
  return result;
}

static PyObject* setset_repr(PySetsetObject* self) {
  return PyUnicode_FromFormat("<%s object of %p>", Py_TYPE(self)->tp_name,
                             reinterpret_cast<void*>(self->ss->id()));
}
/*
static long setset_hash(PyObject* self) {
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(self);
  return sso->ss->id();
}
*/

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
      long value = PyLong_AsLong(cost);
      Py_DECREF(cost);
      if (PyErr_Occurred()) {
        Py_DECREF(cost_iter);
        return NULL;
      }
      costs.push_back(static_cast<int>(value));
    } else {
      Py_DECREF(cost);
      Py_DECREF(cost_iter);
      PyErr_SetString(PyExc_TypeError, "not a number");
      return NULL;
    }
  }
  Py_DECREF(cost_iter);

  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->cost_le(costs, cost_bound));
}

static PyObject* setset_remove_some_element(PySetsetObject* self) {
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->remove_some_element());
}

static PyObject* setset_add_some_element(PySetsetObject* self, PyObject* args) {
  int num_variables = 0;
  if (!PyArg_ParseTuple(args, "|i", &num_variables)) {
    return NULL;
  }
  if (num_variables <= 0) {
    PyErr_SetString(PyExc_TypeError, "not a positive number");
    return NULL;
  }
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->add_some_element(setset::max_elem(),
    setset::max_elem() - num_variables + 1));
}

static PyObject* setset_remove_add_some_elements(PySetsetObject* self, PyObject* args) {
  int num_variables = 0;
  if (!PyArg_ParseTuple(args, "|i", &num_variables)) {
    return NULL;
  }
  if (num_variables < 0) {
    PyErr_SetString(PyExc_TypeError, "not a positive number");
    return NULL;
  }
  RETURN_NEW_OBJECT(Py_TYPE(self), self->ss->remove_add_some_elements(setset::max_elem(),
    setset::max_elem() - num_variables + 1));
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
    if (!PyList_Check(edge_obj)) {
      PyErr_SetString(PyExc_TypeError, "Each edge must be a list");
      return empty_edges;
    }
    if (PyList_Size(edge_obj) != 2) {
      PyErr_SetString(PyExc_ValueError, "Each edge must have exactly two elements");
      return empty_edges;
    }
    std::vector<std::string> edge(2);
    for (int j = 0; j < 2; ++j) {
      PyObject *v_obj = PyList_GetItem(edge_obj, j);
      PyObject *v_repr = PyObject_Repr(v_obj);
      if (v_repr == NULL) {
        return empty_edges;
      }
      const char *v = PyUnicode_AsUTF8(v_repr);
      if (v == NULL) {
        Py_DECREF(v_repr);
        return empty_edges;
      }
      edge[j] = std::string(v);
      Py_DECREF(v_repr);
    }
    edges.push_back(std::move(edge));
  }
  return edges;
}

static PyObject* setset_to_vss_convert(PySetsetObject* self, PyObject* args, bool is_evss) {
  std::vector<std::vector<std::string>> edges = parse_args_to_edges(args);
  if (PyErr_Occurred()) {
    return NULL;
  }
  if (edges.size() == 0) {
    PyErr_SetString(PyExc_ValueError, "Graph is empty");
    return NULL;
  }
  PySetsetObject* ret = reinterpret_cast<PySetsetObject*>
      (PySetset_Type.tp_alloc(&PySetset_Type, 0));
  if (ret == NULL) {
    PyErr_SetString(PyExc_MemoryError, "Failed to allocate "
        "memory for PySetsetObject");
    return NULL;
  }
  try {
    if (is_evss) {
      auto ss = self->ss->to_edgevertexsetset_setset(edges);
      ret->ss = new setset(ss);
    } else {
      auto ss = self->ss->to_vertexsetset_setset(edges);
      ret->ss = new setset(ss);
    }
  } catch (const std::exception& e) {
    Py_DECREF(ret);
    PyErr_SetString(PyExc_RuntimeError, e.what());
    return NULL;
  } catch (...) {
    Py_DECREF(ret);
    PyErr_SetString(PyExc_RuntimeError, "Unknown error occurred");
    return NULL;
  }

  return reinterpret_cast<PyObject*>(ret);
}

static PyObject* setset_to_vertexsetset(PySetsetObject* self, PyObject* args) {
  return setset_to_vss_convert(self, args, false);
}

static PyObject* setset_to_edgevertexsetset(PySetsetObject* self, PyObject* args) {
  return setset_to_vss_convert(self, args, true);
}

static PyMemberDef setset_members[] = {
  {NULL}  /* Sentinel */
};

static PyMethodDef setset_methods[] = {
  {"copy", reinterpret_cast<PyCFunction>(setset_copy), METH_NOARGS, ""},
  {"invert", reinterpret_cast<PyCFunction>(setset_invert), METH_NOARGS, ""},
  {"complement", reinterpret_cast<PyCFunction>(setset_complement), METH_O, ""},
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
  {"hitting", reinterpret_cast<PyCFunction>(setset_hitting), METH_O, ""},
  {"smaller", reinterpret_cast<PyCFunction>(setset_smaller), METH_O, ""},
  {"larger", reinterpret_cast<PyCFunction>(setset_larger), METH_O, ""},
  {"set_size", reinterpret_cast<PyCFunction>(setset_set_size), METH_O, ""},
  {"flip", reinterpret_cast<PyCFunction>(setset_flip), METH_VARARGS, ""},
  {"flip_all", reinterpret_cast<PyCFunction>(setset_flip_all), METH_VARARGS, ""},
  {"join", reinterpret_cast<PyCFunction>(setset_join), METH_O, ""},
  {"meet", reinterpret_cast<PyCFunction>(setset_meet), METH_O, ""},
  {"subsets", reinterpret_cast<PyCFunction>(setset_subsets), METH_O, ""},
  {"supersets", reinterpret_cast<PyCFunction>(setset_supersets), METH_O, ""},
  {"non_subsets", reinterpret_cast<PyCFunction>(setset_non_subsets), METH_O, ""},
  {"non_supersets", reinterpret_cast<PyCFunction>(setset_non_supersets), METH_O, ""},
  {"choice", reinterpret_cast<PyCFunction>(setset_choice), METH_NOARGS, ""},
  {"probability", reinterpret_cast<PyCFunction>(setset_probability), METH_VARARGS, ""},
  {"dump", reinterpret_cast<PyCFunction>(setset_dump), METH_O, ""},
  {"dumps", reinterpret_cast<PyCFunction>(setset_dumps), METH_NOARGS, ""},
  {"_enum", reinterpret_cast<PyCFunction>(setset_enum), METH_O, ""},
  {"_enums", reinterpret_cast<PyCFunction>(setset_enums), METH_NOARGS, ""},
  {"cost_le", reinterpret_cast<PyCFunction>(setset_cost_le), METH_VARARGS | METH_KEYWORDS, ""},
  {"remove_some_element", reinterpret_cast<PyCFunction>(setset_remove_some_element), METH_NOARGS, ""},
  {"add_some_element", reinterpret_cast<PyCFunction>(setset_add_some_element), METH_VARARGS, ""},
  {"remove_add_some_elements", reinterpret_cast<PyCFunction>(setset_remove_add_some_elements), METH_VARARGS, ""},
  {"to_vertexsetset", reinterpret_cast<PyCFunction>(setset_to_vertexsetset), METH_VARARGS, ""},
  {"to_edgevertexsetset", reinterpret_cast<PyCFunction>(setset_to_edgevertexsetset), METH_VARARGS, ""},
  {NULL}  /* Sentinel */
};

static PyNumberMethods setset_as_number = {
  0,                                  /*nb_add*/
  reinterpret_cast<binaryfunc>(setset_difference), /*nb_subtract*/
  0,                                  /*nb_multiply*/
  reinterpret_cast<binaryfunc>(setset_remainder), /*nb_remainder*/
  0,                                  /*nb_divmod*/
  0,                                  /*nb_power*/
  0,                                  /*nb_negative*/
  0,                                  /*nb_positive*/
  0,                                  /*nb_absolute*/
  reinterpret_cast<inquiry>(setset_bool), /*nb_nonzero or nb_bool*/
  reinterpret_cast<unaryfunc>(setset_invert), /*nb_invert*/
  0,                                  /*nb_lshift*/
  0,                                  /*nb_rshift*/
  reinterpret_cast<binaryfunc>(setset_intersection), /*nb_and*/
  reinterpret_cast<binaryfunc>(setset_symmetric_difference), /*nb_xor*/
  reinterpret_cast<binaryfunc>(setset_union), /*nb_or*/
  0,                                  /*nb_int*/
  0,                                  /*nb_long or *nb_reserved*/
  0,                                  /*nb_float*/
  0,                                  /*nb_inplace_add*/
  reinterpret_cast<binaryfunc>(setset_difference_update), /*nb_inplace_subtract*/
  0,                                  /*nb_inplace_multiply*/
  reinterpret_cast<binaryfunc>(setset_remainder_update), /*nb_inplace_remainder*/
  0,                                  /*nb_inplace_power*/
  0,                                  /*nb_inplace_lshift*/
  0,                                  /*nb_inplace_rshift*/
  reinterpret_cast<binaryfunc>(setset_intersection_update), /*nb_inplace_and*/
  reinterpret_cast<binaryfunc>(setset_symmetric_difference_update), /*nb_inplace_xor*/
  reinterpret_cast<binaryfunc>(setset_update), /*nb_inplace_or*/
  0, /*nb_floor_divide*/
  reinterpret_cast<binaryfunc>(setset_quotient), /*nb_true_divide*/
  0, /*nb_inplace_floor_divide*/
  reinterpret_cast<binaryfunc>(setset_quotient_update), /*nb_inplace_true_divide*/
  0 /*nb_index*/
// for 3.5?
//  0, /*nb_matrix_multiply*/
//  0 /*nb_inplace_matrix_multiply*/
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
  0,                                  /*tp_reserved at 3.4*/
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
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
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
  0, /* tp_free */
  0, /* tp_is_gc */
  0, /* *tp_bases */
  0, /* *tp_mro */
  0, /* *tp_cache */
  0, /* *tp_subclasses */
  0, /* *tp_weaklist */
  0, /* tp_version_tag */
  0, /* tp_finalize */
};

static PyObject* setset_elem_limit(PyObject*) {
  return PyLong_FromLong(setset::elem_limit());
}

//static PyObject* setset_num_elems(PyObject*, PyObject* args) {
//  PyObject* obj = NULL;
//  if (!PyArg_ParseTuple(args, "|O", &obj)) return NULL;
//  if (obj == NULL) {
//    return PyLong_FromLong(setset::num_elems());
//  } else {
//    setset::num_elems(PyLong_AsLong(obj));
//    Py_RETURN_NONE;
//  }
//}

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
    Py_DECREF(eo);
    if (j == NULL) {
      Py_DECREF(i);
      return false;
    }
    vector<string> e;
    PyObject* vo;
    while ((vo = PyIter_Next(j))) {
      if (!PyBytes_Check(vo) && !PyUnicode_Check(vo)) {
        Py_DECREF(vo);
        Py_DECREF(j);
        Py_DECREF(i);
        PyErr_SetString(PyExc_TypeError, "invalid graph");
        return false;
      }
      string v = PyBytes_Check(vo) ? PyBytes_AsString(vo) : PyUnicode_AsUTF8(vo);
      Py_DECREF(vo);
      if (v.find(',') != string::npos) {
        Py_DECREF(j);
        Py_DECREF(i);
        PyErr_SetString(PyExc_TypeError, "invalid vertex in the graph");
        return false;
      }
      e.push_back(v);
    }
    Py_DECREF(j);

    if (e.size() != 2) {
      Py_DECREF(i);
      PyErr_SetString(PyExc_TypeError, "each edge must have "
        "exactly two vertices");
      return false;
    }
    graph.push_back(make_pair(e[0], e[1]));
  }
  Py_DECREF(i);
  return true;
}

// return false if an error occurs
static bool get_string_from_sequence(PyObject* obj, int index, string* str) {
  PyObject* i = PySequence_GetItem(obj, index);
  if (i == NULL) {
    return false;
  }
  if (!PyBytes_Check(i)) {
    Py_DECREF(i);
    return false;
  }
  const char* s = PyBytes_AsString(i);
  if (s == NULL) {
    Py_DECREF(i);
    return false;
  }
  Py_DECREF(i);
  *str = string(s);
  return true;
}

// return false if an error occurs
static bool get_double_from_sequence(PyObject* obj, int index, double* value) {
  PyObject* i = PySequence_GetItem(obj, index);
  if (i == NULL) {
    return false;
  }
  PyObject* float_obj = PyNumber_Float(i);
  Py_DECREF(i);
  if (float_obj == NULL) {
    return false;
  }
  double v = PyFloat_AsDouble(float_obj);
  Py_DECREF(float_obj);
  if (PyErr_Occurred()) {
    return false;
  }
  *value = v;
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
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
      Py_DECREF(uo);
      if (j == NULL) {
        Py_DECREF(i);
        return NULL;
      }
      vector<string> v;
      PyObject* vo;
      while ((vo = PyIter_Next(j))) {
        if (!PyBytes_Check(vo)) {
          Py_DECREF(vo);
          Py_DECREF(j);
          Py_DECREF(i);
          PyErr_SetString(PyExc_TypeError, "invalid vertex groups");
          return NULL;
        }
        const char* vertex = PyBytes_AsString(vo);
        if (vertex == NULL) {
          Py_DECREF(vo);
          Py_DECREF(j);
          Py_DECREF(i);
          return NULL;
        }
        v.push_back(string(vertex));
        Py_DECREF(vo);
      }
      if (PyErr_Occurred()) { // check the error of PyIter_Next(j)
        Py_DECREF(j);
        Py_DECREF(i);
        return NULL;
      }
      Py_DECREF(j);
      vertex_groups->push_back(v);
    }
    if (PyErr_Occurred()) { // check the error of PyIter_Next(i)
      Py_DECREF(i);
      return NULL;
    }
    Py_DECREF(i);
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
      const char* vertex_p = PyBytes_AsString(vo);
      if (vertex_p == NULL) {
        PyErr_SetString(PyExc_TypeError, "error converting vertex to string");
        return NULL;
      }
      string vertex(vertex_p);
      PyObject* i = PyObject_GetIter(lo);
      if (i == NULL) return NULL;
      vector<int> r;
      PyObject* io;
      while ((io = PyIter_Next(i))) {
        if (!PyLong_Check(io)) {
          Py_DECREF(io);
          Py_DECREF(i);
          PyErr_SetString(PyExc_TypeError, "invalid degree in degree constraints");
          return NULL;
        }
        long value = PyLong_AsLong(io);
        Py_DECREF(io);
        if (PyErr_Occurred()) {
          Py_DECREF(i);
          return NULL;
        }
        r.push_back(value);
      }
      if (PyErr_Occurred()) { // check the error of PyIter_Next(i)
        Py_DECREF(i);
        return NULL;
      }
      Py_DECREF(i);
      if (r.size() < 3) {
        PyErr_SetString(PyExc_ValueError, "degree constraints must have at least 3 values");
        return NULL;
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
    if (i == NULL) return NULL;
    PyObject* io;
    while ((io = PyIter_Next(i))) {
      if (!PyLong_Check(io)) {
        Py_DECREF(io);
        Py_DECREF(i);
        PyErr_SetString(PyExc_TypeError, "invalid number of edges");
        return NULL;
      }
      long value = PyLong_AsLong(io);
      Py_DECREF(io);
      if (PyErr_Occurred()) {
        Py_DECREF(i);
        return NULL;
      }
      r.push_back(value);
    }
    if (PyErr_Occurred()) { // check the error of PyIter_Next(i)
      Py_DECREF(i);
      return NULL;
    }
    Py_DECREF(i);
    if (r.size() < 3) {
      PyErr_SetString(PyExc_ValueError, "num_edges must have at least 3 values");
      return NULL;
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
      if (lo == NULL) {
        Py_DECREF(co);
        Py_DECREF(i);
        return NULL;
      }
      PyObject* j = PyObject_GetIter(lo);
      Py_DECREF(lo);
      if (j == NULL) {
        Py_DECREF(co);
        Py_DECREF(i);
        return NULL;
      }
      PyObject* eo = NULL;
      while ((eo = PyIter_Next(j))) {
        string u, v;
        double w;
        if (!get_string_from_sequence(eo, 0, &u)
            || !get_string_from_sequence(eo, 1, &v)
            || !get_double_from_sequence(eo, 2, &w)) {
          Py_DECREF(eo);
          Py_DECREF(j);
          Py_DECREF(i);
          return NULL;
        }
        /*PyObject* uo = PySequence_GetItem(eo, 0);
        string u = PyBytes_AsString(uo);
        PyObject* vo = PySequence_GetItem(eo, 1);
        if (vo == NULL || !PyBytes_Check(vo)) return NULL;
        string v = PyBytes_AsString(vo);
        PyObject* wo = PySequence_GetItem(eo, 2);
        if (wo == NULL || !PyFloat_Check(wo)) return NULL;
        double w = PyFloat_AsDouble(wo);*/
        expr.push_back(make_pair(make_pair(u, v), w));
      }
      Py_DECREF(j);
      if (PyErr_Occurred()) { // check the error of PyIter_Next(j)
        Py_DECREF(co);
        Py_DECREF(i);
        return NULL;
      }
      PyObject* ro = PySequence_GetItem(co, 1);
      Py_DECREF(co);
      if (ro == NULL) {
        Py_DECREF(i);
        return NULL;
      }
      /*PyObject* r0o = PySequence_GetItem(ro, 0);
      if (r0o == NULL || !PyFloat_Check(r0o)) return NULL;
      range.first = PyFloat_AsDouble(r0o);
      PyObject* r1o = PySequence_GetItem(ro, 1);
      if (r1o == NULL || !PyFloat_Check(r1o)) return NULL;
      range.second = PyFloat_AsDouble(r1o);*/
      if (!get_double_from_sequence(ro, 0, &range.first)
          || !get_double_from_sequence(ro, 1, &range.second)) {
        Py_DECREF(ro);
        Py_DECREF(i);
        return NULL;
      }
      Py_DECREF(ro);
    }
    Py_DECREF(i);
    if (PyErr_Occurred()) { // check the error of PyIter_Next(i)
      return NULL;
    }
  }

  setset ss = SearchGraphs(graph, vertex_groups, degree_constraints, num_edges,
                           num_comps, no_loop, search_space,
                           linear_constraints);

  RETURN_NEW_SETSET(ss);
}

static PyObject* graphset_show_messages(PySetsetObject* self, PyObject* obj) {
  int ret = graphillion::ShowMessages(PyObject_IsTrue(obj));
  if (ret) Py_RETURN_TRUE;
  else Py_RETURN_FALSE;
}

static PyObject* graphset_omp_get_max_threads(PyObject*) {
#ifdef _OPENMP
  return PyLong_FromLong(omp_get_max_threads());
#else
  return PyLong_FromLong(1);
#endif
}

static PyObject* graphset_omp_get_num_threads(PyObject*) {
#ifdef _OPENMP
  return PyLong_FromLong(omp_get_num_threads());
#else
  return PyLong_FromLong(1);
#endif
}

static PyObject* graphset_omp_set_num_threads(PyObject*, PyObject* obj) {
#ifdef _OPENMP
  if (!PyLong_Check(obj)) {
    PyErr_SetString(PyExc_TypeError, "not an integer");
    return NULL;
  }
  long value = PyLong_AsLong(obj);
  omp_set_num_threads(value);
#endif
  Py_RETURN_NONE;
}

static PyObject* graphset_omp_get_num_procs(PyObject*) {
#ifdef _OPENMP
  return PyLong_FromLong(omp_get_num_procs());
#else
  return PyLong_FromLong(1);
#endif
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  if (PyLong_Check(degree_obj)) {
    int d = PyLong_AsLong(degree_obj);
    if (PyErr_Occurred()) {
      return NULL;
    }
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
      if (PyErr_Occurred()) {
        return NULL;
      }
    } else {
      PyErr_SetString(PyExc_TypeError, "degree lower must be integer");
      return NULL;
    }
    PyObject* upper_obj = PyTuple_GetItem(degree_obj, 1);
    if (PyLong_Check(upper_obj)) {
      degree_upper = PyLong_AsLong(upper_obj);
      if (PyErr_Occurred()) {
        return NULL;
      }
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
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  auto ss = graphillion::SearchOddEdgeSubgraphs(graph);
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  if (!PyDict_Check(deg_dist)) {
    PyErr_SetString(PyExc_TypeError, "deg_dist must be a dictionary");
    return NULL;
  }

  PyObject* key, *value;
  Py_ssize_t pos = 0;

  std::vector<int> deg_ranges;
  while (PyDict_Next(deg_dist, &pos, &key, &value)) {
    if (!PyLong_Check(key)) {
      PyErr_SetString(PyExc_TypeError, "key must be an integer.");
      return NULL;
    }
    if (!PyLong_Check(value)) {
      PyErr_SetString(PyExc_TypeError, "Currently, value must be an integer.");
      return NULL;
    }
    int k = PyLong_AsLong(key);
    if (PyErr_Occurred()) {
      return NULL;
    }
    if (k < 0) {
      PyErr_SetString(PyExc_ValueError, "degree distribution keys "
        "must be non-negative");
      return NULL;
    }
    int v = PyLong_AsLong(value);
    if (PyErr_Occurred()) {
      return NULL;
    }
    if (static_cast<int>(deg_ranges.size()) <= k) {
      deg_ranges.resize(k + 1);
    }
    deg_ranges[k] = v;
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
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "not positive integer");
    return NULL;
  }
  if (num_comp_ub < num_comp_lb) {
    PyErr_SetString(PyExc_ValueError, "lower bound is larger than upper bound");
    return NULL;
  }
  if(std::numeric_limits<int16_t>::max() < num_comp_ub){
    PyErr_SetString(PyExc_ValueError, "too many components");
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  auto ss = graphillion::SearchPartitions(graph, num_comp_lb, num_comp_ub);
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "not positive integer");
    return NULL;
  }
  if (upper < lower) {
    PyErr_SetString(PyExc_ValueError, "lower bound is larger than upper bound");
    return NULL;
  }
  if (ratio != 0 && ratio < 1.0) {
    PyErr_SetString(PyExc_ValueError, "ratio is less than 1.0");
    return NULL;
  }
  if (std::numeric_limits<int16_t>::max() < num_comps) {
    PyErr_SetString(PyExc_ValueError, "too many components");
    return NULL;
  }

  vector<pair<string, string> > graph;
  if (!translate_graph(graph_obj, graph)) {
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  map<string, uint32_t> weight_list;
  if (weight_list_obj != NULL && weight_list_obj != Py_None) {
    if (!PyDict_Check(weight_list_obj)) {
      PyErr_SetString(PyExc_TypeError, "weight_list must be a dictionary");
      return NULL;
    }
    PyObject* keyObject;
    PyObject* valObject;
    Py_ssize_t pos = 0;
    while (PyDict_Next(weight_list_obj, &pos, &keyObject, &valObject)) {
      if (!PyBytes_Check(keyObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid vertex in weight list");
        return NULL;
      }
      const char* vertex_p = PyBytes_AsString(keyObject);
      if (vertex_p == NULL) {
        PyErr_SetString(PyExc_ValueError, "invalid vertex in weight list");
        return NULL;
      }
      string vertex(vertex_p);
      if (!PyLong_Check(valObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid weight in weight list");
        return NULL;
      }
      long weight = PyLong_AsLong(valObject);
      if (PyErr_Occurred()) {
        PyErr_SetString(PyExc_ValueError, "invalid weight in weight list");
        return NULL;
      }
      if (weight < 0 || weight > std::numeric_limits<weight_t>::max()) {
        PyErr_SetString(PyExc_ValueError, "Weight value is out of valid range");
        return NULL;
      }
      weight_list[vertex] = static_cast<uint32_t>(weight);
    }
  }

  auto ss = graphillion::SearchBalancedPartitions(graph, weight_list, ratio, lower,
                                             upper, num_comps);
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  auto ss = graphillion::SearchInducedGraphs(graph);
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  std::map<std::string, uint32_t> weight_list;
  if (weight_list_obj != NULL && weight_list_obj != Py_None) {
    if (!PyDict_Check(weight_list_obj)) {
      PyErr_SetString(PyExc_TypeError, "weight_list must be a dictionary");
      return NULL;
    }
    PyObject* keyObject;
    PyObject* valObject;
    Py_ssize_t pos = 0;
    while (PyDict_Next(weight_list_obj, &pos, &keyObject, &valObject)) {
      if (!PyBytes_Check(keyObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid vertex in weight list");
        return NULL;
      }
      const char* vertex_p = PyBytes_AsString(keyObject);
      if (vertex_p == NULL) {
        PyErr_SetString(PyExc_ValueError, "invalid vertex in weight list");
        return NULL;
      }
      string vertex(vertex_p);
      if (!PyLong_Check(valObject)) {
        PyErr_SetString(PyExc_TypeError, "invalid weight in weight list");
        return NULL;
      }
      long weight = PyLong_AsLong(valObject);
      if (PyErr_Occurred()) {
        return NULL;
      }
      if (weight < 0 || weight > std::numeric_limits<weight_t>::max()) {
        PyErr_SetString(PyExc_ValueError, "Weight value is out of valid range");
        return NULL;
      }
      weight_list[vertex] = static_cast<uint32_t>(weight);
    }
  }

  auto ss = graphillion::SearchWeightedInducedGraphs(graph, weight_list, lower,
                                                     upper);
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  if (graphset_obj == NULL || graphset_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "graphset none");
    return NULL;
  }

  if (!PyObject_TypeCheck(graphset_obj, &PySetset_Type)) {
    PyErr_SetString(PyExc_TypeError, "graphset must be a PySetsetObject");
    return NULL;
  }

  auto ss = graphillion::SearchForbiddenInducedSubgraphs(graph,
    reinterpret_cast<PySetsetObject*>(graphset_obj)->ss);
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
    return NULL;
  }

  auto ss = graphillion::SearchChordals(graph);
  RETURN_NEW_SETSET(ss);
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
    PyErr_SetString(PyExc_ValueError, "Failed to translate graph object");
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
        Py_DECREF(i);
        return NULL;
      }
      double prob = PyFloat_AsDouble(p);
      Py_DECREF(p);
      if (PyErr_Occurred()) {
        Py_DECREF(i);
        return NULL;
      }
      probabilities.push_back(prob);
    }
    if (PyErr_Occurred()) {
      Py_DECREF(i);
      return NULL;
    }
    Py_DECREF(i);
  }

  if (terminals_obj == NULL || terminals_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no terminals");
    return NULL;
  }
  if (!PySequence_Check(terminals_obj)) {
    PyErr_SetString(PyExc_TypeError, "terminals must be a sequence of strings");
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
        Py_DECREF(i);
        return NULL;
      }
      terminals.push_back(PyBytes_AsString(term));
      Py_DECREF(term);
    }
    if (PyErr_Occurred()) {
      Py_DECREF(i);
      return NULL;
    }
    Py_DECREF(i);
  }

  auto prob = graphillion::reliability(graph, probabilities, terminals);
  return PyFloat_FromDouble(prob);
}

static PyObject* setset_get_vertices_from_top(PySetsetObject* self, PyObject* args) {
  std::vector<std::vector<std::string>> edges = parse_args_to_edges(args);
  std::vector<std::string> v_order_from_top = VariableConverter::get_vertices_from_top(edges);
  int n = static_cast<int>(v_order_from_top.size());
  PyObject* ret = PyList_New(n);
  if (ret == NULL) {
    return ret;
  }
  for (int i = 0; i < n; ++i) {
    PyObject* v = PyUnicode_FromString(v_order_from_top[i].c_str());
    if (v == NULL) {
      Py_DECREF(ret);
      return NULL;
    }
    if (PyList_SetItem(ret, i, v) != 0) {
      Py_DECREF(v);
      Py_DECREF(ret);
      return NULL;
    }
  }
  return ret;
}

// directed version

bool input_graph(PyObject* graph_obj,
                 std::vector<std::pair<std::string, std::string> >& graph) {
  if (graph_obj == NULL || graph_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no graph");
    return false;
  }
  PyObject* i = PyObject_GetIter(graph_obj);
  if (i == NULL) return false;
  PyObject* eo;
  while ((eo = PyIter_Next(i))) {
    PyObject* j = PyObject_GetIter(eo);
    Py_DECREF(eo);
    if (j == NULL) return false;
    std::vector<std::string> e;
    PyObject* vo;
    while ((vo = PyIter_Next(j))) {
      if (!PyBytes_Check(vo)) {
        Py_DECREF(vo);
        Py_DECREF(j);
        Py_DECREF(i);
        PyErr_SetString(PyExc_TypeError, "invalid graph");
        return false;
      }
      std::string v = PyBytes_AsString(vo);
      Py_DECREF(vo);
      if (v.find(',') != std::string::npos) {
        Py_DECREF(j);
        Py_DECREF(i);
        PyErr_SetString(PyExc_TypeError, "invalid vertex in the graph");
        return false;
      }
      e.push_back(v);
    }
    Py_DECREF(j);
    if (PyErr_Occurred()) {
      return false;
    }
    if (e.size() != 2) {
      Py_DECREF(i);
      PyErr_SetString(PyExc_TypeError, "each edge must have "
        "exactly two vertices");
      return false;
    }
    graph.push_back(make_pair(e[0], e[1]));
  }
  Py_DECREF(i);
  if (PyErr_Occurred()) {
    return false;
  }
  return true;
}

bool input_string_list(PyObject* list_obj, std::vector<std::string>& list) {
  if (list_obj == NULL || list_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no input");
    return false;
  }

  PyObject* i = PyObject_GetIter(list_obj);
  if (i == NULL) return false;
  PyObject* vo;
  while ((vo = PyIter_Next(i))) {
    if (!PyBytes_Check(vo)) {
      PyErr_SetString(PyExc_TypeError, "invalid input");
      Py_DECREF(vo);
      Py_DECREF(i);
      return false;
    }
    std::string v = PyBytes_AsString(vo);
    Py_DECREF(vo);
    if (v.find(',') != std::string::npos) {
      PyErr_SetString(PyExc_TypeError, "invalid vertex in the graph");
      Py_DECREF(i);
      return false;
    }
    list.push_back(v);
  }
  Py_DECREF(i);
  return true;
}

bool input_vertex_to_range_map(
    PyObject* map_obj, std::map<std::string, Range>& mp) {
  PyObject* vo;
  PyObject* lo;
  Py_ssize_t pos = 0;
  while (PyDict_Next(map_obj, &pos, &vo, &lo)) {
    if (!PyBytes_Check(vo)) {
      PyErr_SetString(PyExc_TypeError, "invalid vertex in map object");
      return false;
    }
    std::string vertex = PyBytes_AsString(vo);
    PyObject* i = PyObject_GetIter(lo);
    if (i == NULL) return false;
    std::vector<int> r;
    PyObject* io;
    while ((io = PyIter_Next(i))) {
      if (!PyLong_Check(io)) {
        Py_DECREF(io);
        Py_DECREF(i);
        PyErr_SetString(PyExc_TypeError, "invalid degree in map object");
        return false;
      }
      long value = PyLong_AsLong(io);
      if (PyErr_Occurred()) {
        Py_DECREF(i);
        return false;
      }
      r.push_back(value);
    }
    Py_DECREF(i);
    if (PyErr_Occurred()) {
      return false;
    }
    if (r.size() != 3) {
      PyErr_SetString(PyExc_ValueError, "range must contain exactly 3 integers");
      return false;
    }
    mp[vertex] = Range(r[0], r[1], r[2]);
  }
  return true;
}

static PyObject* graphset_directed_cycles(PyObject*, PyObject* args,
                                          PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "search_space";
  static char* kwlist[3] = {s1, s2, NULL};
  PyObject* graph_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &graph_obj,
                                   &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  graphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None) {
    if (!PyObject_TypeCheck(search_space_obj, &PySetset_Type)) {
      PyErr_SetString(PyExc_TypeError, "search_space must be a setset object");
      return NULL;
    }
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;
  }

  graphillion::setset ss =
      graphillion::SearchDirectedCycles(graph, search_space);

  RETURN_NEW_SETSET(ss);
}

static PyObject* graphset_directed_hamiltonian_cycles(PyObject*, PyObject* args,
                                                      PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "search_space";
  static char* kwlist[3] = {s1, s2, NULL};
  PyObject* graph_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|O", kwlist, &graph_obj,
                                   &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  graphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None) {
    if (!PyObject_TypeCheck(search_space_obj, &PySetset_Type)) {
      PyErr_SetString(PyExc_TypeError, "search_space must be a setset object");
      return NULL;
    }
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;
  }

  graphillion::setset ss =
      graphillion::SearchDirectedHamiltonianCycles(graph, search_space);

  RETURN_NEW_SETSET(ss);
}

static PyObject* graphset_directed_st_path(PyObject*, PyObject* args,
                                           PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "s";
  static char s3[] = "t";
  static char s4[] = "is_hamiltonian";
  static char s5[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, s5, NULL};
  PyObject* graph_obj = NULL;
  int is_hamiltonian = false;
  PyObject* s_obj = NULL;
  PyObject* t_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "OSSp|O", kwlist, &graph_obj,
                                   &s_obj, &t_obj, &is_hamiltonian,
                                   &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::string s, t;
  if (s_obj == NULL || s_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no vertex s");
    return NULL;
  }
  if (!PyBytes_Check(s_obj)) {
    PyErr_SetString(PyExc_TypeError, "invalid vertex s");
    return NULL;
  }
  s = PyBytes_AsString(s_obj);

  if (t_obj == NULL || t_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no vertex t");
    return NULL;
  }
  if (!PyBytes_Check(t_obj)) {
    PyErr_SetString(PyExc_TypeError, "invalid vertex t");
    return NULL;
  }
  t = PyBytes_AsString(t_obj);

  graphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None) {
    if (!PyObject_TypeCheck(search_space_obj, &PySetset_Type)) {
      PyErr_SetString(PyExc_TypeError, "search_space must be a setset object");
      return NULL;
    }
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;
  }

  graphillion::setset ss = graphillion::SearchDirectedSTPath(
      graph, is_hamiltonian, s, t, search_space);

  RETURN_NEW_SETSET(ss);
}

static PyObject* graphset_rooted_forests(PyObject*, PyObject* args,
                                         PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "roots";
  static char s3[] = "is_spanning";
  static char s4[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, NULL};
  PyObject* graph_obj = NULL;
  PyObject* roots_obj = NULL;
  PyObject* search_space_obj = NULL;
  int is_spanning;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "O|OpO", kwlist, &graph_obj,
                                   &roots_obj, &is_spanning, &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::vector<std::string> roots;
  if (roots_obj != NULL && roots_obj != Py_None) {
    if (!input_string_list(roots_obj, roots)) {
      return NULL;
    }
  }

  graphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None) {
    if (!PyObject_TypeCheck(search_space_obj, &PySetset_Type)) {
      PyErr_SetString(PyExc_TypeError, "search_space must be a setset object");
      return NULL;
    }
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;
  }

  graphillion::setset ss = graphillion::SearchDirectedForests(
      graph, roots, is_spanning, search_space);

  RETURN_NEW_SETSET(ss);
}

static PyObject* graphset_rooted_trees(PyObject*, PyObject* args,
                                       PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "root";
  static char s3[] = "is_spanning";
  static char s4[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, NULL};
  PyObject* graph_obj = NULL;
  PyObject* search_space_obj = NULL;
  PyObject* root_obj = NULL;
  int is_spanning = false;
  if (!PyArg_ParseTupleAndKeywords(args, kwds, "OSp|O", kwlist, &graph_obj,
                                   &root_obj, &is_spanning, &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::string root;
  if (root_obj == NULL || root_obj == Py_None) {
    PyErr_SetString(PyExc_TypeError, "no vertex root");
    return NULL;
  }
  if (!PyBytes_Check(root_obj)) {
    PyErr_SetString(PyExc_TypeError, "invalid vertex root");
    return NULL;
  }
  root = PyBytes_AsString(root_obj);

  graphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None) {
    if (!PyObject_TypeCheck(search_space_obj, &PySetset_Type)) {
      PyErr_SetString(PyExc_TypeError, "search_space must be a setset object");
      return NULL;
    }
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;
  }

  graphillion::setset ss =
      graphillion::SearchRootedTrees(graph, root, is_spanning, search_space);

  RETURN_NEW_SETSET(ss);
}

static PyObject* graphset_directed_graphs(PyObject*, PyObject* args,
                                          PyObject* kwds) {
  static char s1[] = "graph";
  static char s2[] = "in_degree_constraints";
  static char s3[] = "out_degree_constraints";
  static char s4[] = "search_space";
  static char* kwlist[] = {s1, s2, s3, s4, NULL};
  PyObject* graph_obj = NULL;
  PyObject* in_degree_constraints_obj = NULL;
  PyObject* out_degree_constraints_obj = NULL;
  PyObject* search_space_obj = NULL;
  if (!PyArg_ParseTupleAndKeywords(
          args, kwds, "O|OOO", kwlist, &graph_obj, &in_degree_constraints_obj,
          &out_degree_constraints_obj, &search_space_obj))
    return NULL;

  std::vector<std::pair<std::string, std::string> > graph;
  if (!input_graph(graph_obj, graph)) {
    return NULL;
  }

  std::map<std::string, Range> in_degree_constraints_entity;
  std::map<std::string, Range>* in_degree_constraints = NULL;
  if (in_degree_constraints_obj != NULL &&
      in_degree_constraints_obj != Py_None) {
    in_degree_constraints = &in_degree_constraints_entity;
    if (!input_vertex_to_range_map(in_degree_constraints_obj,
                                   in_degree_constraints_entity)) {
      return NULL;
    }
  }

  std::map<std::string, Range> out_degree_constrains_entity;
  std::map<std::string, Range>* out_degree_constrains = NULL;
  if (out_degree_constraints_obj != NULL &&
      out_degree_constraints_obj != Py_None) {
    out_degree_constrains = &out_degree_constrains_entity;
    if (!input_vertex_to_range_map(out_degree_constraints_obj,
                                   out_degree_constrains_entity)) {
      return NULL;
    }
  }

  graphillion::setset* search_space = NULL;
  if (search_space_obj != NULL && search_space_obj != Py_None) {
    if (!PyObject_TypeCheck(search_space_obj, &PySetset_Type)) {
      PyErr_SetString(PyExc_TypeError, "search_space must be a setset object");
      return NULL;
    }
    search_space = reinterpret_cast<PySetsetObject*>(search_space_obj)->ss;
  }

  graphillion::setset ss = graphillion::SearchDirectedGraphs(
      graph, in_degree_constraints, out_degree_constrains, search_space);

  RETURN_NEW_SETSET(ss);
}

static PyMethodDef module_methods[] = {
  {"load", reinterpret_cast<PyCFunction>(setset_load), METH_O, ""},
  {"loads", reinterpret_cast<PyCFunction>(setset_loads), METH_O, ""},
  {"_elem_limit", reinterpret_cast<PyCFunction>(setset_elem_limit), METH_NOARGS, ""},
  //{"_num_elems", setset_num_elems, METH_VARARGS, ""},
  {"_graphs", reinterpret_cast<PyCFunction>(graphset_graphs), METH_VARARGS | METH_KEYWORDS, ""},
  {"_show_messages", reinterpret_cast<PyCFunction>(graphset_show_messages), METH_O, ""},
  {"_omp_get_max_threads", reinterpret_cast<PyCFunction>(graphset_omp_get_max_threads), METH_NOARGS, ""},
  {"_omp_get_num_threads", reinterpret_cast<PyCFunction>(graphset_omp_get_num_threads), METH_NOARGS, ""},
  {"_omp_set_num_threads", reinterpret_cast<PyCFunction>(graphset_omp_set_num_threads), METH_O, ""},
  {"_omp_get_num_procs", reinterpret_cast<PyCFunction>(graphset_omp_get_num_procs), METH_NOARGS, ""},
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
  // directed version
  {"_directed_cycles",
    reinterpret_cast<PyCFunction>(graphset_directed_cycles),
    METH_VARARGS | METH_KEYWORDS, ""},
  {"_directed_hamiltonian_cycles",
    reinterpret_cast<PyCFunction>(graphset_directed_hamiltonian_cycles),
    METH_VARARGS | METH_KEYWORDS, ""},
  {"_directed_st_path",
    reinterpret_cast<PyCFunction>(graphset_directed_st_path),
    METH_VARARGS | METH_KEYWORDS, ""},
  {"_rooted_forests", reinterpret_cast<PyCFunction>(graphset_rooted_forests),
    METH_VARARGS | METH_KEYWORDS, ""},
  {"_rooted_trees", reinterpret_cast<PyCFunction>(graphset_rooted_trees),
    METH_VARARGS | METH_KEYWORDS, ""},
  {"_directed_graphs",
    reinterpret_cast<PyCFunction>(graphset_directed_graphs),
    METH_VARARGS | METH_KEYWORDS, ""},
  {NULL}  /* Sentinel */
};

PyDoc_STRVAR(graphillion_doc,
"Hidden module to implement graphillion classes.");

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

PyMODINIT_FUNC PyInit__graphillion(void) {
  PyObject* m;
  if (PyType_Ready(&PySetset_Type) < 0) return NULL;
  if (PyType_Ready(&PySetsetIter_Type) < 0) return NULL;
  m = PyModule_Create(&moduledef);
  if (m == NULL) return NULL;
  Py_INCREF(&PySetset_Type);
  Py_INCREF(&PySetsetIter_Type);
  PyModule_AddObject(m, "setset", reinterpret_cast<PyObject*>(&PySetset_Type));
  PyModule_AddObject(m, "setset_iterator",
                     reinterpret_cast<PyObject*>(&PySetsetIter_Type));
  return m;
}
