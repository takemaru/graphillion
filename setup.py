import os
from setuptools import setup, find_packages, Extension
import sys

sys.path.insert(0, 'graphillion')
import release

prefixes = [os.path.join('/usr'), os.path.join('/usr', 'local'),
            os.path.join('/usr', 'share'), os.path.join('/opt', 'local'),
            os.path.join('/sw', 'local')]

include_dirs = ['src']
link_args = []
macros = []
libraries = []

for prefix in prefixes:
    path = os.path.join(prefix, 'include', 'gmpxx.h')
    if os.path.isfile(path):
        include_dirs.extend([os.path.join(prefix, 'include')])
        link_args.extend(['-L' + os.path.join(prefix, 'lib')])
        macros.extend([('HAVE_LIBGMP', None), ('HAVE_LIBGMPXX', None)])
        libraries.extend(['gmp', 'gmpxx'])
        break

setup(name='graphillion',
      version=release.version,
      description='A fast, lightweight graphset operation library',
      long_description="""\
Graphillion is a Python library for efficient graphset operations.
Unlike existing graph tools such as NetworkX, which are designed to
process just a single graph at a time, Graphilion manipulates a large
set of graphs with great efficiency.  Surprisingly, trillions of
trillions of graphs can be handled in a single computer.
""",
      classifiers=[ # http://pypi.python.org/pypi?%3Aaction=list_classifiers
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: MIT License',
        'Operating System :: POSIX :: Linux',
        'Operating System :: MacOS :: MacOS X',
        'Programming Language :: C',
        'Programming Language :: C++',
        'Programming Language :: Python :: 2.6',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 2 :: Only',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Scientific/Engineering :: Information Analysis',
        'Topic :: Scientific/Engineering :: Mathematics',
        ], 
      keywords=['graph', 'set', 'math', 'network'],
      author=release.authors[0][0],
      author_email=release.authors[0][1],
      url='https://github.com/takemaru/graphillion',
      license=release.license,
      zip_safe=False,
      ext_modules=[
        Extension('_graphillion', 
                  sources=[os.path.join('src', 'pygraphillion.cc'),
                           os.path.join('src', 'graphillion', 'graphset.cc'),
                           os.path.join('src', 'graphillion', 'setset.cc'),
                           os.path.join('src', 'graphillion', 'util.cc'),
                           os.path.join('src', 'graphillion', 'zdd.cc'),
                           os.path.join('src', 'SAPPOROBDD', 'bddc.c'),
                           os.path.join('src', 'SAPPOROBDD', 'BDD.cc'),
                           os.path.join('src', 'SAPPOROBDD', 'ZBDD.cc')],
                  include_dirs=include_dirs,
                  extra_link_args=link_args,
                  define_macros=macros,
                  libraries=libraries,
                  ),
        ],
      test_suite='graphillion.test',
      )
