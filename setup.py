import os
from setuptools import setup, find_packages, Extension
import sys

sys.path.insert(0, 'graphillion')
import release

sources_list = [os.path.join('src', 'pygraphillion.cc'),
                os.path.join('src', 'graphillion', 'graphset.cc'),
                os.path.join('src', 'graphillion', 'setset.cc'),
                os.path.join('src', 'graphillion', 'util.cc'),
                os.path.join('src', 'graphillion', 'zdd.cc'),
                os.path.join('src', 'SAPPOROBDD', 'bddc.c'),
                os.path.join('src', 'SAPPOROBDD', 'BDD.cc'),
                os.path.join('src', 'SAPPOROBDD', 'ZBDD.cc')]

if sys.platform == 'win32':
    sources_list.append(os.path.join('src', 'mingw32', 'RpWinResource.c'))
    libraries_list = ['gdi32', 'kernel32', 'user32', 'Psapi']
    extra_link_args_list=['-static']
else:
    libraries_list = []
    extra_link_args_list=[]

setup(name='Graphillion',
      version=release.version,
      description='Fast, lightweight library for a huge number of graphs',
      long_description="""\
Graphillion is a Python library for efficient graphset operations.
Unlike existing graph tools such as NetworkX, which are designed to
manipulate just a single graph at a time, Graphillion handles a large
set of graphs with great efficiency.  Surprisingly, trillions of
trillions of graphs can be processed on a single computer.
""",
      classifiers=[ # http://pypi.python.org/pypi?%3Aaction=list_classifiers
        'Development Status :: 3 - Alpha',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: MIT License',
        'Operating System :: POSIX :: Linux',
        'Operating System :: POSIX :: BSD :: FreeBSD',
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
      url='http://graphillion.org/',
      license=release.license,
      packages = ['graphillion'],
      ext_modules=[
        Extension('_graphillion', 
                  sources=sources_list,
                  include_dirs=['src'],
                  libraries=libraries_list,
                  define_macros=[('B_64', None)],
                  extra_link_args=extra_link_args_list,
                  ),
        ],
      test_suite='graphillion.test',
      )
