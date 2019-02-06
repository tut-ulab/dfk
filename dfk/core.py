# -*- coding: utf-8 -*-

import dfk._dfk


class _DFkData():
    def __init__(self, ptr):
        self._ptr = ptr

    def dfn(self, n, term):
        dfk._dfk.use(self._ptr)
        return dfk._dfk.dfn(n, term)

    def cf(self, term):
        dfk._dfk.use(self._ptr)
        return dfk._dfk.cf(term)

    def total_document(self):
        dfk._dfk.use(self._ptr)
        return dfk._dfk.total_document()


def open(filepath):
    #if not os.path.exists(filepath):
    #    pass
    ptr = dfk._dfk.setup(filepath)
    return _DFkData(ptr)
