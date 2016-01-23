#ifndef GRAPHILLION_POLLYFILL_H_
#define GRAPHILLION_POLLYFILL_H_

#include "py3c.h"

#ifdef IS_PY3

#define PyString_AsString PyUnicode_AsUTF8

#endif

#endif