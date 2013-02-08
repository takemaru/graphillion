from setuptools import setup, find_packages, Extension
import os

extra_include_dirs = ['/opt/local/include']
extra_link_args = ['-L/opt/local/lib']
extra_macros = [('HAVE_LIBGMP', None), ('HAVE_LIBGMPXX', None)]

setup(name='graphillion',
      version='0.1',
      description='Python package for manipulating a set of graphs',
      long_description="""\
""",
      classifiers=[], # Get strings from http://pypi.python.org/pypi?%3Aaction=list_classifiers
      keywords='',
      author='Takeru Inoue',
      author_email='takeru.inoue@gmail.com',
      url='',
      license='MIT',
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
                           os.path.join('src', 'hudd', 'bddc.c'),
                           os.path.join('src', 'hudd', 'BDD.cc'),
                           os.path.join('src', 'hudd', 'ZBDD.cc')],
                  include_dirs=['src'] + extra_include_dirs,
                  libraries=['gmp', 'gmpxx'],
                  define_macros=[('B_64', None)] + extra_macros,
                  extra_link_args=extra_link_args,
                  ),
        ],
      )
