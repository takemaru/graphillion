#include <Python.h>
#include "structmember.h"

#include "pysetset.h"

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

// setset::iterator

typedef struct {
  PyObject_HEAD
  PySetsetObject* ssi_sets; /* Set to NULL when iterator is exhausted */
  Py_ssize_t ssi_pos;
  Py_ssize_t len;
} setsetiterobject;

static void setsetiter_dealloc(setsetiterobject* ssi) {
  Py_XDECREF(ssi->ssi_sets);
  PyObject_Del(ssi);
}

static PyObject* setsetiter_len(setsetiterobject* ssi) {
  return PyInt_FromLong(0);
}

PyDoc_STRVAR(length_hint_doc,
             "Private method returning an estimate of len(list(it)).");

static PyMethodDef setsetiter_methods[] = {
  {"__length_hint__", reinterpret_cast<PyCFunction>(setsetiter_len), METH_NOARGS,
   length_hint_doc},
  {nullptr,           nullptr}           /* sentinel */
};

static PyObject* setsetiter_iternext(setsetiterobject* ssi) {
  PySetsetObject *sso = ssi->ssi_sets;
  if (sso == nullptr) return nullptr;
  assert(PySetset_Check(sso));
  Py_DECREF(sso);
  ssi->ssi_sets = nullptr;
  return nullptr;
}

static PyTypeObject SetsetIter_Type = {
  PyVarObject_HEAD_INIT(&PyType_Type, 0)
  "setsiterator",                             /* tp_name */
  sizeof(setsetiterobject),                     /* tp_basicsize */
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
  Py_TPFLAGS_DEFAULT,                         /* tp_flags */
  0,                                          /* tp_doc */
  0,                                          /* tp_traverse */
  0,                                          /* tp_clear */
  0,                                          /* tp_richcompare */
  0,                                          /* tp_weaklistoffset */
  PyObject_SelfIter,                          /* tp_iter */
  reinterpret_cast<iternextfunc>(setsetiter_iternext), /* tp_iternext */
  setsetiter_methods,                           /* tp_methods */
  0,
};

static PyObject* setset_iter(PySetsetObject* sso) {
  setsetiterobject* ssi = PyObject_New(setsetiterobject, &SetsetIter_Type);
  if (ssi == nullptr) return nullptr;
  Py_INCREF(sso);
  ssi->ssi_sets = sso;
  ssi->ssi_pos = 0;
  ssi->len = 0;
  return reinterpret_cast<PyObject*>(ssi);
}

// setset

static int setset_parse_set(PyObject* so, set<int>* s) {
  assert(s != nullptr);
  PyObject* i = PyObject_GetIter(so);
  if (i == nullptr) return -1;
  PyObject* eo;
  while ((eo = PyIter_Next(i))) {
    if (!PyInt_Check(eo)) {
      Py_DECREF(eo);
      PyErr_SetString(PyExc_TypeError, "can't assign non-integer elements");
      return -1;
    }
    s->insert(PyLong_AsLong(eo));
    Py_DECREF(eo);
  }
  Py_DECREF(i);
  return 0;
}

static int setset_parse_map(PyObject* dict_obj, map<string, set<int> >* m) {
  assert(m != nullptr);
  PyObject* key_obj;
  PyObject* so;
  Py_ssize_t pos = 0;
  while (PyDict_Next(dict_obj, &pos, &key_obj, &so)) {
    if (!PyString_Check(key_obj) || !PyAnySet_Check(so)) {
      PyErr_SetString(PyExc_TypeError, "not list of sets or dicts");
      return -1;
    }
    string key = PyString_AsString(key_obj);
    set<int> s;
    if (setset_parse_set(so, &s) == -1) return -1;
    (*m)[key] = s;
  }
  return 0;
}

static PyObject* setset_new(PyTypeObject* type, PyObject* args, PyObject* kwds) {
  PySetsetObject* self;
  self = reinterpret_cast<PySetsetObject*>(type->tp_alloc(type, 0));
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
    map<string, set<int> > m;
    if (setset_parse_map(obj, &m) == -1) return -1;
    self->ss = new setset(m);
  } else {
    PyObject* i = PyObject_GetIter(obj);
    if (i == nullptr) {
      PyErr_SetString(PyExc_TypeError, "not list of sets or dicts");
      return -1;
    }
    vector<set<int> > vs;
    vector<map<string, set<int> > > vm;
    PyObject* o;
    while ((o = PyIter_Next(i))) {
      if (PyAnySet_Check(o)) {
        set<int> s;
        if (setset_parse_set(o, &s) == -1) return -1;
        vs.push_back(s);
      } else if (PyDict_Check(o)) {
        map<string, set<int> > m;
        if (setset_parse_map(o, &m) == -1) return -1;
        vm.push_back(m);
      }
      Py_DECREF(o);
    }
    Py_DECREF(i);
    self->ss = new setset(vs);
//    self->ss = new setset(vm);  // TODO: merge two setsets
  }
  return 0;
}

static void setset_dealloc(PySetsetObject* self) {
  delete self->ss;
  self->ob_type->tp_free(reinterpret_cast<PyObject*>(self));
}

static PyObject* setset_isdisjoint(PySetsetObject* self, PyObject* other) {
  if (!PySetset_Check(other)) {
    PyErr_SetString(PyExc_TypeError, "not setset");
    return nullptr;
  }
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(other);
  if (self->ss->is_disjoint(*sso->ss))
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyObject* setset_issubset(PySetsetObject* self, PyObject* other) {
  if (!PySetset_Check(other)) {
    PyErr_SetString(PyExc_TypeError, "not setset");
    return nullptr;
  }
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(other);
  if (self->ss->is_subset(*sso->ss))
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyObject* setset_issuperset(PySetsetObject* self, PyObject* other) {
  if (!PySetset_Check(other)) {
    PyErr_SetString(PyExc_TypeError, "not setset");
    return nullptr;
  }
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(other);
  if (self->ss->is_superset(*sso->ss))
    Py_RETURN_TRUE;
  else
    Py_RETURN_FALSE;
}

static PyObject* setset_dump(PySetsetObject* self) {
  self->ss->dump();
  Py_RETURN_NONE;
}

static PyObject* setset_dumps(PySetsetObject* self) {
  stringstream sstr;
  self->ss->dump(sstr);
  return PyString_FromString(sstr.str().c_str());
}

static PyObject* setset_repr(PySetsetObject* self) {
  return PyString_FromFormat("<%s object of %p>", self->ob_type->tp_name,
                             reinterpret_cast<void*>(self->ss->id()));
}

static PyObject* setset_sub(PySetsetObject* self, PyObject* other) {
  PySetsetObject* result = nullptr;
  if (!PySetset_Check(self) || !PySetset_Check(other)) {
    Py_INCREF(Py_NotImplemented);
    return Py_NotImplemented;
  }
  result = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  if (result == nullptr) return nullptr;
  // TODO: add elements to result
  return reinterpret_cast<PyObject*>(result);
}

static PyObject* setset_invert(PySetsetObject* self) {
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(
      PySetset_Type.tp_alloc(&PySetset_Type, 0));
  sso->ss = new setset(~(*self->ss));
  return reinterpret_cast<PyObject*>(sso);
}

// Returns the number of objects in sequence o on success, and -1 on failure.
static Py_ssize_t setset_len(PyObject* self) {
  return 0;
}

// If an item in o is equal to value, return 1, otherwise return 0. On error, return -1.
static int setset_contains(PySetsetObject *self, PyObject *key) {
  return 0;
}

static long setset_hash(PyObject* self) {
  PySetsetObject* sso = reinterpret_cast<PySetsetObject*>(self);
  return sso->ss->id();
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
  {"isdisjoint", reinterpret_cast<PyCFunction>(setset_isdisjoint), METH_O, ""},
  {"issubset", reinterpret_cast<PyCFunction>(setset_issubset), METH_O, ""},
  {"issuperset", reinterpret_cast<PyCFunction>(setset_issuperset), METH_O, ""},
  {"dump", reinterpret_cast<PyCFunction>(setset_dump), METH_NOARGS, ""},
  {"dumps", reinterpret_cast<PyCFunction>(setset_dumps), METH_NOARGS, ""},
  {nullptr}  /* Sentinel */
};

static PyNumberMethods setset_as_number = {
  0,                                  /*nb_add*/
  reinterpret_cast<binaryfunc>(setset_sub), /*nb_subtract*/
  0,                                  /*nb_multiply*/
  0,                                  /*nb_divide*/
  0,                                  /*nb_remainder*/
  0,                                  /*nb_divmod*/
  0,                                  /*nb_power*/
  0,                                  /*nb_negative*/
  0,                                  /*nb_positive*/
  0,                                  /*nb_absolute*/
  0,                                  /*nb_nonzero*/
  reinterpret_cast<unaryfunc>(setset_invert), /*nb_invert*/
  0,                                  /*nb_lshift*/
  0,                                  /*nb_rshift*/
//    (binaryfunc)setset_and,                /*nb_and*/
//    (binaryfunc)setset_xor,                /*nb_xor*/
//    (binaryfunc)setset_or,                 /*nb_or*/
  0,                                  /*nb_coerce*/
  0,                                  /*nb_int*/
  0,                                  /*nb_long*/
  0,                                  /*nb_float*/
  0,                                  /*nb_oct*/
  0,                                  /*nb_hex*/
  0,                                  /*nb_inplace_add*/
//    (binaryfunc)setset_isub,               /*nb_inplace_subtract*/
  0,                                  /*nb_inplace_multiply*/
  0,                                  /*nb_inplace_divide*/
  0,                                  /*nb_inplace_remainder*/
  0,                                  /*nb_inplace_power*/
  0,                                  /*nb_inplace_lshift*/
  0,                                  /*nb_inplace_rshift*/
//    (binaryfunc)setset_iand,               /*nb_inplace_and*/
//    (binaryfunc)setset_ixor,               /*nb_inplace_xor*/
//    (binaryfunc)setset_ior,                /*nb_inplace_or*/
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
  0,                         /*tp_compare*/
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
  Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
  "Base class for set of sets", /* tp_doc */
  0,		               /* tp_traverse */
  0,		               /* tp_clear */
  reinterpret_cast<richcmpfunc>(setset_richcompare), /* tp_richcompare */
  0,		               /* tp_weaklistoffset */
  reinterpret_cast<getiterfunc>(setset_iter), /* tp_iter */
  0,		               /* tp_iternext */
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

static PyMethodDef module_methods[] = {
  {nullptr}  /* Sentinel */
};

#ifndef PyMODINIT_FUNC  /* declarations for DLL import/export */
#define PyMODINIT_FUNC void
#endif
PyMODINIT_FUNC init_illion(void) {
  PyObject* m;
  if (PyType_Ready(&PySetset_Type) < 0) return;
  m = Py_InitModule3("_illion", module_methods,
                     "Hidden module to implement illion objects.");
  if (m == nullptr) return;
  Py_INCREF(&PySetset_Type);
  PyModule_AddObject(m, "setset", reinterpret_cast<PyObject*>(&PySetset_Type));
}
