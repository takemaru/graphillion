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
""",
      classifiers=[], # Get strings from http://pypi.python.org/pypi?%3Aaction=list_classifiers
      keywords='',
      author=release.authors[0][0],
      author_email=release.authors[0][1],
      url='',
      license=release.license,
      packages=find_packages(exclude=['ez_setup', 'examples', 'tests']),
      include_package_data=True,
      zip_safe=False,
      install_requires=[
          # -*- Extra requirements: -*-
      ],
      entry_points="""
      # -*- Entry points: -*-
      """,
      ext_modules=[
        Extension('_graphillion', 
                  sources=[os.path.join('src', 'pysetset.cc'),
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
      )
