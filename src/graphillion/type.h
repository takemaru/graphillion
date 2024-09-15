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

#ifndef GRAPHILLION_TYPE_H_
#define GRAPHILLION_TYPE_H_

#include <stdint.h>
#include <vector>

#include "SAPPOROBDD/ZBDD.h"

namespace graphillion {

typedef ZBDD zdd_t;
typedef bddword word_t;
typedef int32_t elem_t;  // bddvar

typedef std::string vertex_t;
typedef std::pair<vertex_t, vertex_t> edge_t;
typedef std::pair<edge_t,double> weighted_edge_t;
typedef std::pair<std::vector<weighted_edge_t>,std::pair<double,double> > linear_constraint_t;

}  // namespace graphillion

#endif  // GRAPHILLION_TYPE_H_
