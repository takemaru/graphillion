/* Copyright (c) 2015, Red Hat, Inc. and/or its affiliates
 * Licensed under the MIT license; see py3c.h
 */

#ifndef _PY3C_COMPARISON_H_
#define _PY3C_COMPARISON_H_
#include <Python.h>

/* Rich comparisons */

#ifndef Py_RETURN_NOTIMPLEMENTED
#define Py_RETURN_NOTIMPLEMENTED \
    return Py_INCREF(Py_NotImplemented), Py_NotImplemented
#endif

#define PY3C_RICHCMP(val1, val2, op) \
    ((op) == Py_EQ) ? PyBool_FromLong((val1) == (val2)) : \
    ((op) == Py_NE) ? PyBool_FromLong((val1) != (val2)) : \
    ((op) == Py_LT) ? PyBool_FromLong((val1) < (val2)) : \
    ((op) == Py_GT) ? PyBool_FromLong((val1) > (val2)) : \
    ((op) == Py_LE) ? PyBool_FromLong((val1) <= (val2)) : \
    ((op) == Py_GE) ? PyBool_FromLong((val1) >= (val2)) : \
    (Py_INCREF(Py_NotImplemented), Py_NotImplemented)

#endif
