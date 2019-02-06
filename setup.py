# -*- coding: utf-8 -*-

from distutils.core import setup, Extension

module = Extension(
    name='dfk._dfk',
    sources=['src/df_wrapper.c', 'src/df.c', 'src/misc.c', 'src/moji_utf8.c'],
    include_dirs=['src/'],
)

setup(name='dfk',
      version='1.0',
      description='The module for computing document frequencies',
      author='Shiori Hironaka',
      author_email='s143369@edu.tut.ac.jp',
      packages=['dfk'],
      ext_modules=[
          module,
      ])
