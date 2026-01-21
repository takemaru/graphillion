from setuptools import setup, Extension
import os
import platform
import subprocess
import shutil
import sys
import sysconfig
import tempfile

sys.path.insert(0, 'graphillion')
import release

# Check if external SAPPOROBDD (pysapporobdd) should be used
USE_EXTERNAL_SAPPOROBDD = os.environ.get('USE_EXTERNAL_SAPPOROBDD', '').lower() in ('1', 'true', 'yes')


def check_for_openmp():
    """Check  whether the default compiler supports OpenMP.
    This routine is adapted from yt, thanks to Nathan
    Goldbaum. See https://github.com/pynbody/pynbody/issues/124"""

    # Create a temporary directory
    tmpdir = tempfile.mkdtemp()
    curdir = os.getcwd()
    os.chdir(tmpdir)

    # Get compiler invocation
    compiler = os.environ.get('CC',
                              sysconfig.get_config_var('CC') or 'gcc')

    # make sure to use just the compiler name without flags
    compiler = compiler.split()[0]

    # Attempt to compile a test script.
    # See http://openmp.org/wp/openmp-compilers/
    if compiler == 'g++':
        filename = r'test.cpp'
    else:
        filename = r'test.c'
    with open(filename,'w') as f :
        f.write("""#include <omp.h>
#include <stdio.h>
int main() {
#pragma omp parallel
printf(\"Hello from thread %d, nthreads %d\\n\", omp_get_thread_num(), omp_get_num_threads());
}
""")
    try:
        with open(os.devnull, 'w') as fnull:
            exit_code = subprocess.call([compiler, '-fopenmp', filename],
                                        stdout=fnull, stderr=fnull)
    except OSError :
        exit_code = 1

    # Clean up
    os.chdir(curdir)
    shutil.rmtree(tmpdir)

    if exit_code == 0:
        return True
    else:
        import multiprocessing
        cpus = multiprocessing.cpu_count()
        if cpus>1:
            print ("""WARNING:
OpenMP support is not available with your C compiler, even though your
machine has more than one core available.  Although Graphillion
supports parallel computation using OpenMP, it will only run on one
core with your current configuration.

Continuing your build without OpenMP...
""")
        return False


# Common source files (graphillion core)
graphillion_sources = [os.path.join('src', 'pygraphillion.cc'),
                       os.path.join('src', 'graphillion', 'graphset.cc'),
                       os.path.join('src', 'graphillion', 'setset.cc'),
                       os.path.join('src', 'graphillion', 'util.cc'),
                       os.path.join('src', 'graphillion', 'zdd.cc'),
                       os.path.join('src', 'graphillion', 'reconf.cc'),
                       os.path.join('src', 'graphillion', 'regular', 'RegularGraphs.cc'),
                       os.path.join('src', 'graphillion', 'partition', 'Partition.cc'),
                       os.path.join('src', 'graphillion', 'partition', 'BalancedPartition.cc'),
                       os.path.join('src', 'graphillion', 'reliability', 'reliability.cc'),
                       os.path.join('src', 'graphillion', 'induced_graphs', 'InducedGraphs.cc'),
                       os.path.join('src', 'graphillion', 'induced_graphs', 'WeightedInducedGraphs.cc'),
                       os.path.join('src', 'graphillion', 'chordal', 'chordal.cc'),
                       os.path.join('src', 'graphillion', 'forbidden_induced', 'ForbiddenInducedSubgraphs.cc'),
                       os.path.join('src', 'graphillion', 'odd_edges_subgraphs', 'OddEdgeSubgraphs.cc'),
                       os.path.join('src', 'graphillion', 'degree_distribution', 'DegreeDistributionGraphs.cc'),
                       os.path.join('src', 'graphillion', 'variable_converter', 'variable_converter.cc')]

# SAPPOROBDD source files (only needed when not using external library)
sapporobdd_sources = [os.path.join('src', 'SAPPOROBDD', 'bddc.cc'),
                      os.path.join('src', 'SAPPOROBDD', 'BDD.cc'),
                      os.path.join('src', 'SAPPOROBDD', 'ZBDD.cc'),
                      os.path.join('src', 'SAPPOROBDD', 'BDDCT.cc')]

if USE_EXTERNAL_SAPPOROBDD:
    # External SAPPOROBDD mode: link against pysapporobdd's libsapporobdd.so
    try:
        import pysapporobdd
        pysapporobdd_path = os.path.dirname(pysapporobdd.__file__)
        sapporobdd_lib_path = pysapporobdd_path
        sapporobdd_include = os.path.join(pysapporobdd_path, 'include')
        if not os.path.exists(sapporobdd_include):
            raise RuntimeError(
                f"pysapporobdd include directory not found: {sapporobdd_include}\n"
                "Please reinstall pysapporobdd or check your installation."
            )
    except ImportError:
        raise RuntimeError(
            "USE_EXTERNAL_SAPPOROBDD=1 requires pysapporobdd to be installed.\n"
            "Please run: pip install pysapporobdd"
        )
    sources_list = graphillion_sources
else:
    # Default mode: compile SAPPOROBDD sources directly (statically linked)
    sources_list = graphillion_sources + sapporobdd_sources
    sapporobdd_lib_path = None
    sapporobdd_include = None

if sys.platform == 'win32':
    sources_list.append(os.path.join('src', 'mingw32', 'RpWinResource.c'))
    libraries_list = ['gdi32', 'kernel32', 'user32', 'Psapi']
    extra_compile_args_list = []
    extra_link_args_list = ['-static']
else:
    libraries_list = []
    if check_for_openmp():
        extra_compile_args_list = ['-fopenmp']
        extra_link_args_list = ['-fopenmp']
    else:
        extra_compile_args_list = []
        extra_link_args_list = []


# We add this option to suppress warning when compiling bddc.c
# in SAPPOROBDD library. It is no problem because
# the variables that the compiler warns are actually used.
if platform.system() == 'Darwin': # macOS
    extra_compile_args_list.append('-std=c++11')
elif platform.system() == 'Windows':
    pass
else:
    extra_compile_args_list.append('-Wno-maybe-uninitialized')

#extra_compile_args_list.append('-UNDEBUG')

# Configure include directories, libraries, and macros based on build mode
if USE_EXTERNAL_SAPPOROBDD:
    # External SAPPOROBDD mode
    include_dirs_list = ['src', sapporobdd_include]
    libraries_list.append('sapporobdd')
    library_dirs_list = [sapporobdd_lib_path]
    runtime_library_dirs_list = [sapporobdd_lib_path]
    define_macros_list = [('B_64', None), ('USE_EXTERNAL_SAPPOROBDD', None)]
else:
    # Default mode (statically linked SAPPOROBDD)
    include_dirs_list = ['src', 'src/SAPPOROBDD']
    library_dirs_list = []
    runtime_library_dirs_list = []
    define_macros_list = [('B_64', None)]

setup(name='graphillion',
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
        'Development Status :: 4 - Beta',
        'Intended Audience :: Developers',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: MIT License',
        'Operating System :: POSIX :: Linux',
        'Operating System :: POSIX :: BSD :: FreeBSD',
        'Operating System :: MacOS :: MacOS X',
        'Programming Language :: C',
        'Programming Language :: C++',
#        'Programming Language :: Python :: 2.7',
#        'Programming Language :: Python :: 2',
#        'Programming Language :: Python :: 3.4',
#        'Programming Language :: Python :: 3.5',
        'Programming Language :: Python :: 3.6',
        'Programming Language :: Python :: 3.7',
        'Programming Language :: Python :: 3.8',
        'Programming Language :: Python :: 3.9',
        'Programming Language :: Python :: 3.10',
        'Programming Language :: Python :: 3.11',
        'Programming Language :: Python :: 3.12',
        'Programming Language :: Python :: 3',
        'Topic :: Software Development :: Libraries :: Python Modules',
        'Topic :: Scientific/Engineering :: Information Analysis',
        'Topic :: Scientific/Engineering :: Mathematics',
        ],
      keywords=['graph', 'set', 'math', 'network'],
      author=release.authors[0][0],
      author_email=release.authors[0][1],
      url='http://graphillion.org/',
#      license=release.license,
      packages=['graphillion', 'graphillion.test'],
#      install_requires=['future'],
      ext_modules=[
        Extension('_graphillion',
                  sources=sources_list,
                  include_dirs=include_dirs_list,
                  library_dirs=library_dirs_list,
                  runtime_library_dirs=runtime_library_dirs_list,
                  libraries=libraries_list,
                  define_macros=define_macros_list,
                  extra_compile_args=extra_compile_args_list,
                  extra_link_args=extra_link_args_list,
                  ),
        ],
#      test_suite='graphillion.test',
      )
