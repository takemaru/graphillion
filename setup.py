from setuptools import setup, find_packages, Extension
import sys, os

setup(name='illion',
      version='0.1',
      description='Python package for manipulating x-illions of sets and graphs',
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
        Extension('_illion', 
                  sources=[os.path.join('src', 'pysetset.cc'),
                           os.path.join('src', 'illion', 'setset.cc'),
                           os.path.join('src', 'illion', 'util.cc'),
                           os.path.join('src', 'illion', 'zdd.cc'),
                           os.path.join('src', 'hudd', 'bddc.c'),
                           os.path.join('src', 'hudd', 'BDD.cc'),
                           os.path.join('src', 'hudd', 'ZBDD.cc'),
                           ],
                  include_dirs=['src'],
                  libraries=['gmp', 'gmpxx'],
                  define_macros=[('B_64', None),
                                 ('HAVE_LIBGMP', None),
                                 ('HAVE_LIBGMPXX', None)],
                  extra_compile_args=['-std=c++11'],
                  ),
        ],
      )
