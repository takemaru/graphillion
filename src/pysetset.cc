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

using illion::setset;
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
  CHECK_OR_ERROR(obj, PySetset_Check, "setset", nullptr);

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
    if (_ret == nullptr) return nullptr;                                \
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
  PyObject* so = PySet_New(nullptr);
  for (const auto& e : s) {
    PyObject* eo = PyInt_FromLong(e);
    if (eo == nullptr) {
      PyErr_SetString(PyExc_TypeError, "not int set");
      Py_DECREF(eo);
      return nullptr;
    }
    if (PySet_Add(so, eo) == -1) {
      PyErr_SetString(PyExc_RuntimeError, "can't add elements to a set");
      Py_DECREF(eo);
      return nullptr;
    }
    Py_DECREF(eo);
  }
  return so;
}

static int setset_parse_set(PyObject* so, set<int>* s) {
  assert(s != nullptr);
  PyObject* i = PyObject_GetIter(so);
  if (i == nullptr) return -1;
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
  assert(m != nullptr);
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
    if (i == nullptr) return -1;
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
  if (self == nullptr) return nullptr;
  return reinterpret_cast<PyObject*>(self);
}

static void setsetiter_dealloc(PySetsetIterObject* self) {
  delete self->it;
  PyObject_Del(self);
}

static PyObject* setsetiter_next(PySetsetIterObject* self) {
  if (*(self->it) == setset::end())
    return nullptr;
  set<int> s = *(*self->it);
  ++(*self->it);
  return setset_build_set(s);
}

static PyMethodDef setsetiter_methods[] = {
  {nullptr,           nullptr}           /* sentinel */
};

static PyTypeObject PySetsetIter_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "setset_iterator",                             /* tp_name */
  sizeof(PySetsetIterObject),                     /* tp_basicsize */
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
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_HAVE_ITER,                         /* tp_flags */
  0,                                          /* tp_doc */
  0,                                          /* tp_traverse */
  0,                                          /* tp_clear */
  0,                                          /* tp_richcompare */
  0,                                          /* tp_weaklistoffset */
  PyObject_SelfIter,                          /* tp_iter */
  reinterpret_cast<iternextfunc>(setsetiter_next), /* tp_iternext */
  setsetiter_methods,                           /* tp_methods */
  0,              /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  0, /* tp_init */
  PyType_GenericAlloc,       /* tp_alloc */
  setsetiter_new                  /* tp_new */
};

// setset

static PyObject* setset_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
  PySetsetObject* self;
  self = reinterpret_cast<PySetsetObject*>(type->tp_alloc(type, 0));
  if (self == nullptr) return nullptr;
  return reinterpret_cast<PyObject*>(self);
}

static int setset_init(PySetsetObject* self, PyObject* args, PyObject* kwds) {
  PyObject* obj = nullptr;
  if (!PyArg_ParseTuple(args, "|O", &obj))
    return -1;
  if (obj == nullptr) {
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
    if (i == nullptr) return -1;
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

static PyObject* setset_copy(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, *self->ss);
}

static PyObject* setset_complement(PySetsetObject* self) {
  RETURN_NEW_SETSET(self, ~(*self->ss));
}

static PyObject* setset_intersection(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) & (*_other->ss));
}

static PyObject* setset_intersection_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) &= (*_other->ss));
}

static PyObject* setset_union(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) | (*_other->ss));
}

static PyObject* setset_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) |= (*_other->ss));
}

static PyObject* setset_difference(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) - (*_other->ss));
}

static PyObject* setset_difference_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) -= (*_other->ss));
}
/*
static PyObject* setset_product(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) * (*_other->ss));
}

static PyObject* setset_product_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) *= (*_other->ss));
}
*/
static PyObject* setset_symmetric_difference(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, (*self->ss) ^ (*_other->ss));
}

static PyObject* setset_symmetric_difference_update(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_SELF_SETSET(self, other, _other, (*self->ss) ^= (*_other->ss));
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

static Py_ssize_t setset_len(PyObject* obj) {
  PySetsetObject* self = reinterpret_cast<PySetsetObject*>(obj);
  long long int len = strtoll(self->ss->size().c_str(), nullptr, 0);
  if (len != LLONG_MAX) {
    return len;
  } else {
    PyErr_SetString(PyExc_TypeError, "overflow, use setset.len()");
    return -1;
  }
}

static PyObject* setset_long_len(PyObject* obj) {
  PySetsetObject* self = reinterpret_cast<PySetsetObject*>(obj);
  vector<char> buf;
  for (const auto& c : self->ss->size())
    buf.push_back(c);
  buf.push_back('\0');
  return PyLong_FromString(buf.data(), nullptr, 0);
}

static PyObject* setset_rand_iter(PySetsetObject* self) {
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == nullptr) return nullptr;
  ssi->it = new setset::iterator(self->ss->begin());
  if (ssi->it == nullptr) {
    PyErr_NoMemory();
    return nullptr;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

static PyObject* setset_opt_iter(PySetsetObject* self, PyObject* weights) {
  PyObject* i = PyObject_GetIter(weights);
  if (i == nullptr) return nullptr;
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
      Py_DECREF(eo);
      PyErr_SetString(PyExc_TypeError, "not a number");
      return nullptr;
    }
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  PySetsetIterObject* ssi = PyObject_New(PySetsetIterObject, &PySetsetIter_Type);
  if (ssi == nullptr) return nullptr;
  ssi->it = new setset::iterator(self->ss->begin(w));
  if (ssi->it == nullptr) {
    PyErr_NoMemory();
    return nullptr;
  }
  return reinterpret_cast<PyObject*>(ssi);
}

// If an item in o is equal to value, return 1, otherwise return 0. On error, return -1.
static int setset_contains(PySetsetObject* self, PyObject* so) {
  CHECK_OR_ERROR(so, PyAnySet_Check, "set", -1);
  set<int> s;
  if (setset_parse_set(so, &s) == -1) return -1;
  return self->ss->find(s) != setset::end() ? 1 : 0;
}

static PyObject* setset_include(PySetsetObject* self, PyObject* eo) {
  CHECK_OR_ERROR(eo, PyInt_Check, "int", nullptr);
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(
      self->ob_type->tp_alloc(self->ob_type, 0));
  sso->ss = new setset(self->ss->include(PyInt_AsLong(eo)));
  return reinterpret_cast<PyObject*>(sso);
}

static PyObject* setset_exclude(PySetsetObject* self, PyObject* eo) {
  CHECK_OR_ERROR(eo, PyInt_Check, "int", nullptr);
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(
      self->ob_type->tp_alloc(self->ob_type, 0));
  sso->ss = new setset(self->ss->exclude(PyInt_AsLong(eo)));
  return reinterpret_cast<PyObject*>(sso);
}

static PyObject* setset_add(PySetsetObject* self, PyObject* so) {
  CHECK_OR_ERROR(so, PyAnySet_Check, "set", nullptr);
  set<int> s;
  if (setset_parse_set(so, &s) == -1) return nullptr;
  self->ss->insert(s);
  Py_RETURN_NONE;
}

static PyObject* setset_remove(PySetsetObject* self, PyObject* so) {
  CHECK_OR_ERROR(so, PyAnySet_Check, "set", nullptr);
  set<int> s;
  if (setset_parse_set(so, &s) == -1) return nullptr;
  if (self->ss->erase(s) == 0) {
    PyErr_SetString(PyExc_KeyError, "not found");
    return nullptr;
  }
  Py_RETURN_NONE;
}

static PyObject* setset_discard(PySetsetObject* self, PyObject* so) {
  CHECK_OR_ERROR(so, PyAnySet_Check, "set", nullptr);
  set<int> s;
  if (setset_parse_set(so, &s) == -1) return nullptr;
  self->ss->erase(s);
  Py_RETURN_NONE;
}

static PyObject* setset_pop(PySetsetObject* self) {
  setset::iterator i = self->ss->begin();
  if (i == setset::end()) {
    PyErr_SetString(PyExc_KeyError, "not found");
    return nullptr;
  }
  set<int> s = *i;
  self->ss->erase(s);
  return setset_build_set(s);
}

static PyObject* setset_clear(PySetsetObject* self) {
  self->ss->clear();
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

static PyObject* setset_smaller(PySetsetObject* self, PyObject* other) {
  CHECK_OR_ERROR(other, PyInt_Check, "int", nullptr);
  int max_set_size = PyLong_AsLong(other);
  RETURN_NEW_SETSET(self, self->ss->smaller(max_set_size));
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

static PyObject* setset_supersets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->supersets(*_other->ss));
}

static PyObject* setset_nonsubsets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->nonsubsets(*_other->ss));
}

static PyObject* setset_nonsupersets(PySetsetObject* self, PyObject* other) {
  CHECK_SETSET_OR_ERROR(other);
  RETURN_NEW_SETSET2(self, other, _other, self->ss->nonsupersets(*_other->ss));
}

static PyObject* setset_dump(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", nullptr);
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
  Py_BEGIN_ALLOW_THREADS;
  self->ss->dump(fp);
  Py_END_ALLOW_THREADS;
  PyFile_DecUseCount(file);
  Py_RETURN_NONE;
}

static PyObject* setset_dumps(PySetsetObject* self) {
  stringstream sstr;
  self->ss->dump(sstr);
  return PyString_FromString(sstr.str().c_str());
}

static PyObject* setset_load(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", nullptr);
  FILE* fp = PyFile_AsFile(obj);
  PyFileObject* file = reinterpret_cast<PyFileObject*>(obj);
  PyFile_IncUseCount(file);
  Py_BEGIN_ALLOW_THREADS;
  self->ss->load(fp);
  Py_END_ALLOW_THREADS;
  PyFile_DecUseCount(file);
  Py_RETURN_NONE;
}

static PyObject* setset_loads(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyString_Check, "str", nullptr);
  stringstream sstr(PyString_AsString(obj));
  self->ss->load(sstr);
  Py_RETURN_NONE;
}

static PyObject* setset_enum(PySetsetObject* self, PyObject* obj) {
  CHECK_OR_ERROR(obj, PyFile_Check, "file", nullptr);
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

static long setset_hash(PyObject* self) {
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(self);
  return sso->ss->id();
}

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
    return nullptr;
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
  {nullptr}  /* Sentinel */
};

static PyMethodDef setset_methods[] = {
  {"copy", reinterpret_cast<PyCFunction>(setset_copy), METH_NOARGS, ""},
  {"complement", reinterpret_cast<PyCFunction>(setset_complement), METH_NOARGS, ""},
  {"intersection", reinterpret_cast<PyCFunction>(setset_intersection), METH_O, ""},
  {"intersection_update", reinterpret_cast<PyCFunction>(setset_intersection_update), METH_O, ""},
  {"union", reinterpret_cast<PyCFunction>(setset_union), METH_O, ""},
  {"update", reinterpret_cast<PyCFunction>(setset_update), METH_O, ""},
  {"difference", reinterpret_cast<PyCFunction>(setset_difference), METH_O, ""},
  {"difference_update", reinterpret_cast<PyCFunction>(setset_difference_update), METH_O, ""},
//  {"product", reinterpret_cast<PyCFunction>(setset_product), METH_O, ""},
//  {"product_update", reinterpret_cast<PyCFunction>(setset_product_update), METH_O, ""},
  {"symmetric_difference", reinterpret_cast<PyCFunction>(setset_symmetric_difference), METH_O, ""},
  {"symmetric_difference_update", reinterpret_cast<PyCFunction>(setset_symmetric_difference_update), METH_O, ""},
  {"quotient", reinterpret_cast<PyCFunction>(setset_quotient), METH_O, ""},
  {"quotient_update", reinterpret_cast<PyCFunction>(setset_quotient_update), METH_O, ""},
  {"remainder", reinterpret_cast<PyCFunction>(setset_remainder), METH_O, ""},
  {"remainder_update", reinterpret_cast<PyCFunction>(setset_remainder_update), METH_O, ""},
  {"isdisjoint", reinterpret_cast<PyCFunction>(setset_isdisjoint), METH_O, ""},
  {"issubset", reinterpret_cast<PyCFunction>(setset_issubset), METH_O, ""},
  {"issuperset", reinterpret_cast<PyCFunction>(setset_issuperset), METH_O, ""},
  {"len", reinterpret_cast<PyCFunction>(setset_long_len), METH_NOARGS, ""},
  {"rand_iter", reinterpret_cast<PyCFunction>(setset_rand_iter), METH_NOARGS, ""},
  {"opt_iter", reinterpret_cast<PyCFunction>(setset_opt_iter), METH_O, ""},
  {"include", reinterpret_cast<PyCFunction>(setset_include), METH_O, ""},
  {"exclude", reinterpret_cast<PyCFunction>(setset_exclude), METH_O, ""},
  {"add", reinterpret_cast<PyCFunction>(setset_add), METH_O, ""},
  {"remove", reinterpret_cast<PyCFunction>(setset_remove), METH_O, ""},
  {"discard", reinterpret_cast<PyCFunction>(setset_discard), METH_O, ""},
  {"pop", reinterpret_cast<PyCFunction>(setset_pop), METH_NOARGS, ""},
  {"clear", reinterpret_cast<PyCFunction>(setset_clear), METH_NOARGS, ""},
  {"minimal", reinterpret_cast<PyCFunction>(setset_minimal), METH_NOARGS, ""},
  {"maximal", reinterpret_cast<PyCFunction>(setset_maximal), METH_NOARGS, ""},
  {"hitting", reinterpret_cast<PyCFunction>(setset_hitting), METH_NOARGS, ""},
  {"smaller", reinterpret_cast<PyCFunction>(setset_smaller), METH_O, ""},
  {"join", reinterpret_cast<PyCFunction>(setset_join), METH_O, ""},
  {"meet", reinterpret_cast<PyCFunction>(setset_meet), METH_O, ""},
  {"subsets", reinterpret_cast<PyCFunction>(setset_subsets), METH_O, ""},
  {"supersets", reinterpret_cast<PyCFunction>(setset_supersets), METH_O, ""},
  {"nonsubsets", reinterpret_cast<PyCFunction>(setset_nonsubsets), METH_O, ""},
  {"nonsupersets", reinterpret_cast<PyCFunction>(setset_nonsupersets), METH_O, ""},
  {"dump", reinterpret_cast<PyCFunction>(setset_dump), METH_O, ""},
  {"dumps", reinterpret_cast<PyCFunction>(setset_dumps), METH_NOARGS, ""},
  {"load", reinterpret_cast<PyCFunction>(setset_load), METH_O, ""},
  {"loads", reinterpret_cast<PyCFunction>(setset_loads), METH_O, ""},
  {"_enum", reinterpret_cast<PyCFunction>(setset_enum), METH_O, ""},
  {"_enums", reinterpret_cast<PyCFunction>(setset_enums), METH_NOARGS, ""},
  {nullptr}  /* Sentinel */
};

static PyNumberMethods setset_as_number = {
  0,                                  /*nb_add*/
  reinterpret_cast<binaryfunc>(setset_difference), /*nb_subtract*/
  0/*reinterpret_cast<binaryfunc>(setset_product)*/, /*nb_multiply*/
  reinterpret_cast<binaryfunc>(setset_quotient), /*nb_divide*/
  reinterpret_cast<binaryfunc>(setset_remainder), /*nb_remainder*/
  0,                                  /*nb_divmod*/
  0,                                  /*nb_power*/
  0,                                  /*nb_negative*/
  0,                                  /*nb_positive*/
  0,                                  /*nb_absolute*/
  0,                                  /*nb_nonzero*/
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
  0/*reinterpret_cast<binaryfunc>(setset_product_update)*/, /*nb_inplace_multiply*/
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
  setset_len,                           /* sq_length */
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
  0,                         /*ob_size*/
  "_illion.setset",            /*tp_name*/
  sizeof(PySetsetObject),        /*tp_basicsize*/
  0,                         /*tp_itemsize*/
  reinterpret_cast<destructor>(setset_dealloc), /*tp_dealloc*/
  0,                         /*tp_print*/
  0,                         /*tp_getattr*/
  0,                         /*tp_setattr*/
  setset_nocmp,                         /*tp_compare*/
  reinterpret_cast<reprfunc>(setset_repr), /*tp_repr*/
  &setset_as_number,           /*tp_as_number*/
  &setset_as_sequence,         /*tp_as_sequence*/
  0,                         /*tp_as_mapping*/
  setset_hash,                         /*tp_hash */
  0,                         /*tp_call*/
  0,                         /*tp_str*/
  0,                         /*tp_getattro*/
  0,                         /*tp_setattro*/
  0,                         /*tp_as_buffer*/
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE | Py_TPFLAGS_CHECKTYPES, /*tp_flags*/
  "Base class for set of sets", /* tp_doc */
  0,		               /* tp_traverse */
  0,		               /* tp_clear */
  reinterpret_cast<richcmpfunc>(setset_richcompare), /* tp_richcompare */
  0,		               /* tp_weaklistoffset */
  0, /* tp_iter */
  0,                          /* tp_iternext */
  setset_methods,              /* tp_methods */
  setset_members,              /* tp_members */
  0,                         /* tp_getset */
  0,                         /* tp_base */
  0,                         /* tp_dict */
  0,                         /* tp_descr_get */
  0,                         /* tp_descr_set */
  0,                         /* tp_dictoffset */
  reinterpret_cast<initproc>(setset_init), /* tp_init */
  PyType_GenericAlloc,       /* tp_alloc */
  setset_new,                  /* tp_new */
};

static PyObject* setset_universe(PyObject*, PyObject* args) {
  PyObject* obj = nullptr;
  if (!PyArg_ParseTuple(args, "|O", &obj)) return nullptr;
  if (obj == nullptr) {
    vector<int> universe = setset::universe();
    PyObject* lo = PyList_New(universe.size());
    if (lo == nullptr)
      return nullptr;
    for (int i = 0; i < static_cast<int>(universe.size()); ++i) {
      PyObject* eo = PyInt_FromLong(universe[i]);
      if (eo == nullptr) {
        PyErr_SetString(PyExc_TypeError, "not int");
        Py_DECREF(eo);
        return nullptr;
      }
      if (PyList_SetItem(lo, i, eo) == -1) {
        PyErr_SetString(PyExc_RuntimeError, "can't add elements to a list");
        return nullptr;
      }
      Py_DECREF(eo);
    }
    return lo;
  } else {
    PyObject* i = PyObject_GetIter(obj);
    if (i == nullptr) return nullptr;
    vector<int> universe;
    PyObject* eo;
    while ((eo = PyIter_Next(i))) {
      if (!PyInt_Check(eo)) {
        PyErr_SetString(PyExc_TypeError, "not int");
        Py_DECREF(eo);
        return nullptr;
      }
      universe.push_back(PyInt_AsLong(eo));
    }
    Py_DECREF(i);
    setset::universe(universe);
    Py_RETURN_NONE;
  }
}

static PyMethodDef module_methods[] = {
  {"universe", setset_universe, METH_VARARGS, ""},
  {nullptr}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC init_illion(void) {
  PyObject* m;
  if (PyType_Ready(&PySetset_Type) < 0) return;
  if (PyType_Ready(&PySetsetIter_Type) < 0) return;
  m = Py_InitModule3("_illion", module_methods,
                     "Hidden module to implement illion objects.");
  if (m == nullptr) return;
  Py_INCREF(&PySetset_Type);
  Py_INCREF(&PySetsetIter_Type);
  PyModule_AddObject(m, "setset", reinterpret_cast<PyObject*>(&PySetset_Type));
  PyModule_AddObject(m, "setset_iterator",
                     reinterpret_cast<PyObject*>(&PySetsetIter_Type));
}
