~~~~~~~~~~~~~~~~~~~
dfk library
~~~~~~~~~~~~~~~~~~~

インストール方法
----------------

::

   pip install .

など。



テスト実行の方法
-------------------

::

   ( cd make-index && make )  # 分析の前処理プログラムのコンパイル
   ( cd data && make )  # テストデータに対する分析の前処理の実行
   head data/D-test-1000.dat | cut -f 1,3 | python sample/smart_es_bayes.py data/D-learn-1000.class data/D-test-1000.class

dfkライブラリ自体の利用例を知りたい場合、 ``sample/smart_es_bayes.py`` を読むと良い。


デバッグ関連のコマンドメモ
--------------------------------

::

   python setup.py build_ext --inplace  # コンパイルだけする
   head data/D-test-1000.dat | cut -f 1,3 | PYTHONPATH=build/lib.linux-x86_64-3.6/ python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class
   PYTHONPATH=build/lib.linux-x86_64-3.6/ python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class --input <(head data/D-test-1000.dat | cut -f 1,3)
   PYTHONPATH=build/lib.linux-x86_64-3.6/ python -m pdb sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class --input <(head data/D-test-1000.dat | cut -f 1,3)
   PYTHONPATH=build/lib.linux-x86_64-3.6 gdb -ex r --args /home/hironaka15/.pyenv/versions/dfk/bin/python -m pdb sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class --input <(head data/D-test-1000.dat | cut -f 1,3)

   head data/D-test-1000.dat | cut -f 1,3 | sample/Es-bayes/Es-bayes data/D-learn-1000.class data/D-test-1000.class
   head data/D-test-1000.dat | cut -f 1,3 | python sample/smart_es_bayes.py data/D-learn-1000.class data/D-test-1000.class
   head data/D-test-1000.dat | cut -f 1,3 | python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class

   diff <(head data/D-test-1000.dat | cut -f 1,3 | python sample/smart_es_bayes.py data/D-learn-1000.class data/D-test-1000.class) <(head data/D-test-1000.dat | cut -f 1,3 | python sample/es_bayes.py data/D-learn-1000.class data/D-test-1000.class)

