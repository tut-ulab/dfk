# -*- coding: utf-8 -*-

import dfk._dfk


class DFkData():
    '''DFkを高速に求めるクラス'''

    def __init__(self, ptr):
        self._ptr = ptr

    def dfn(self, n: int, term: str) -> int:
        '''n回以上単語が出現する回数を求める

        Args:
            n: termが何回出現するか
            term: 出現回数を求める文字列
        Returns:
            termがn回以上出現する回数
        '''
        dfk._dfk.use(self._ptr)
        return dfk._dfk.dfn(n, term)

    def cf(self, term: str) -> int:
        '''termの出現回数を求める

        Args:
            term: 出現回数を求める文字列
        Returns:
            termが出現する回数
        '''
        dfk._dfk.use(self._ptr)
        return dfk._dfk.cf(term)

    def total_document(self) -> int:
        '''総ドキュメント数を返す

        Returns:
            総ドキュメント数
        '''
        dfk._dfk.use(self._ptr)
        return dfk._dfk.total_document()


def open(filepath: str):
    '''索引ファイルを開いて返す

    Args:
        filepath of class file
    Returns:
        DFkData
    '''
    #if not os.path.exists(filepath):
    #    pass
    ptr = dfk._dfk.setup(filepath)
    return DFkData(ptr)
