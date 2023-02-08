import os
import _dfk

'''
PYTHONPATH=build/lib.linux-x86_64-3.6 python try_module.py
のようにして実行する
'''

TESTDATA_DIR = '/home/0/s143369/work/proken2018/repo/data'

SPACEFILE = os.path.join(TESTDATA_DIR, 'D-learn-1000.class')
#DOMAINFILE = os.path.join(TESTDATA_DIR, '')


def show_dfk(term):
    for i in range(5):
        n = _dfk.dfn(i+1, term)
        print('df{k}("{term}"):'.format(k=i+1, term=term), n)
        if n == 0:
            break


if __name__ == '__main__':
    space = _dfk.setup(SPACEFILE)

    show_dfk('gakkai')

    show_dfk('、')

    show_dfk('特性')
