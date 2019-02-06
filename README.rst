~~~~~~~~~~~~~~~~~~~
dfk library
~~~~~~~~~~~~~~~~~~~


インストール方法
----------------

:

    pip install .

など。






デバッグ関連
-----------------

:

    head data/D-test-1000.dat | cut -f 1,3 | PYTHONPATH=build/lib.linux-x86_64-3.6/ python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class
    PYTHONPATH=build/lib.linux-x86_64-3.6/ python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class --input <(head data/D-test-1000.dat | cut -f 1,3)
    PYTHONPATH=build/lib.linux-x86_64-3.6/ python -m pdb sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class --input <(head data/D-test-1000.dat | cut -f 1,3)
    PYTHONPATH=build/lib.linux-x86_64-3.6 gdb -ex r --args /home/hironaka15/.pyenv/versions/dfk/bin/python -m pdb sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class --input <(head data/D-test-1000.dat | cut -f 1,3)

    head data/D-test-1000.dat | cut -f 1,3 | sample/Es-bayes/Es-bayes data/D-learn-1000.class data/D-test-1000.class
    head data/D-test-1000.dat | cut -f 1,3 | python sample/smart_es_bayes.py data/D-learn-1000.class data/D-test-1000.class
    head data/D-test-1000.dat | cut -f 1,3 | python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class

    diff <(head data/D-test-1000.dat | cut -f 1,3 | python sample/smart_es_bayes.py data/D-learn-1000.class data/D-test-1000.class) <(head data/D-test-1000.dat | cut -f 1,3 | python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class)
