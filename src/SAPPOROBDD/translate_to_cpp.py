#!/usr/bin/python3
# -*- coding: utf-8 -*-

# usage: Put translate_to_cpp.py, bddc.h, and bddc.c files in the same directory,
#        and run `python translate_to_cpp.py`.
#        It generates new bddc.h and bddc.cc files.

import os
import re
import sys

def convert_code(c_code):
    # rename the "export" function because it is a C++ keyword.
    ts1 = '!!!tempstring1!!!'
    ts2 = '!!!tempstring2!!!'
    c_code = c_code.replace('bddexport', ts1)
    c_code = c_code.replace('bddvexport', ts2)
    c_code = c_code.replace('export', 'export_static')
    c_code = c_code.replace(ts1, 'bddexport')
    c_code = c_code.replace(ts2, 'bddvexport')

    c_code = c_code.replace('static int  err B_ARG((char *msg, bddp num));',
                            'static int  err B_ARG((const char *msg, bddp num));')
    c_code = c_code.replace('char *msg;', 'const char *msg;')

    # change the function call style from the old style to ANSI style.
    # an example of the old style:
    # int function(a, b)
    # int a;
    # short b;
    # {...}

    # detect the head of functions
    # for example, it matches "int func(a, b)"
    pattern = re.compile(r'\n(static\s+)?(\w+\s+\*?\w+)\(([^)]*?)\)')
    matches = pattern.finditer(c_code)
    if not matches:
        return "No valid function definition found."

    # add the positions of matched functions
    function_positions = []
    for m in matches:
        function_positions.append(m.span())
    function_positions.append((len(c_code), -1))

    # add strings before the first function
    # +1 for the head '\n' in the above regex
    new_c_code = c_code[0 : function_positions[0][0] + 1]

    # for each functions
    for i in range(len(function_positions) - 1):
        function_head = c_code[ function_positions[i][0] + 1 : function_positions[i][1] ]
        paren_head = function_head.index('(')
        function_type_and_name = function_head[0 : paren_head]
        function_code = c_code[ function_positions[i][1] : function_positions[i + 1][0] + 1]
        function_lines = function_code.split('\n')
        if function_lines[0].strip().endswith('}'): # one line function
            new_c_code += function_head + function_code # output as is
            continue
        comments = []
        arguments = [] # list of tuples (type, varname)
        body = ""
        bodymode = False
        for line in function_lines:
            if bodymode:
                body += line + '\n'
            elif line.startswith('/*'): # comment line
                if line.endswith('*/'):
                    comments.append(line)
                else:
                    print('not support comment line', file = sys.stderr)
            elif line.endswith(';'): # argument variable declaration
                s = line[0 : -1] # remove last ';'
                # for example, parse "char *a, b, c;"
                vars = s.split(',')
                pattern = re.compile(r'[a-zA-Z0-9_]+$')
                # find the start of the first variable name
                match = re.search(pattern, vars[0])
                if match:
                    # typename such as "char *"
                    typename = vars[0][0 : match.start()]
                    typename = typename.replace('\t', '').strip()
                    argname = vars[0][match.start() : ]
                    if (typename.endswith('*')): # the type is a pointer
                        # need not a space between the type and name
                        arguments.append(typename + argname)
                    else:
                        arguments.append(typename + ' ' + argname)
                    for v in vars[1:]:
                        if (typename.endswith('*')): # the type is a pointer
                            # need not a space between the type and name
                            arguments.append(typename + v.strip())
                        else:
                            arguments.append(typename + ' ' + v.strip())
                else:
                    print('format error: ' + line.strip(), file = sys.stderr)
                    exit(1)
            elif line.startswith('{'):
                bodymode = True
                body = line + '\n'

        new_c_code += function_type_and_name + '(' + ', '.join(arguments) + ')\n'
        for com in comments:
            new_c_code += com + '\n'
        new_c_code += body[0 : -1] # remove last redundant '\n'

    return new_c_code


def main():
    input_h_file = 'bddc.h'
    output_h_file = 'bddc.temp.h'
    backup_h_file = 'bddc.h.old'
    input_c_file = 'bddc.c'
    output_cc_file = 'bddc.cc'
    backup_c_file = 'bddc.c.old'

    if not os.path.exists(input_h_file):
        print('Input file {} does not exist.'.format(input_h_file))
        exit(1)

    if os.path.exists(output_h_file):
        print('Output file {} already exists.'.format(output_h_file))
        exit(1)

    if os.path.exists(backup_h_file):
        print('Backup file {} already exists.'.format(backup_h_file))
        exit(1)

    with open(input_h_file, 'r', encoding='utf-8') as f:
        h_code = f.read()

    h_code = h_code.replace('#ifdef BDD_CPP\n  extern "C" {\n#endif /* BDD_CPP */', '')
    h_code = h_code.replace('#ifdef BDD_CPP\n  }\n#endif /* BDD_CPP */', '')

    with open(output_h_file, 'w', encoding='utf-8') as f:
        f.write(h_code)

    os.rename(input_h_file, backup_h_file)
    os.rename(output_h_file, input_h_file)

    if os.path.exists(backup_c_file):
        print('Backup file {} already exists.'.format(backup_c_file))
        exit(1)

    if not os.path.exists(input_c_file):
        print('Input file {} does not exist.'.format(input_c_file))
        exit(1)

    if os.path.exists(output_cc_file):
        print('Output file {} already exists.'.format(output_cc_file))
        exit(1)

    if os.path.exists(backup_c_file):
        print('Backup file {} already exists.'.format(backup_c_file))
        exit(1)

    with open(input_c_file, 'r', encoding='utf-8') as f:
        c_code = f.read()

    new_c_code = convert_code(c_code)

    with open(output_cc_file, 'w', encoding='utf-8') as f:
        f.write(new_c_code)

    os.rename(input_c_file, backup_c_file)


if __name__ == '__main__':
    main()
