from setuptools import setup, Extension
import sysconfig

# Get the Python include directory
python_include = sysconfig.get_path('include')

module = Extension('dbms',
                   sources=['wrapper.c', 'CoolDB.h'],
                   include_dirs=[python_include])

setup(name='dbms',
      version='1.0',
      description='Python interface for the simple DBMS implementation',
      ext_modules=[module])
