#include <Python.h>
#include "df.h"

// Implementation
static PyObject *
_dfk_setup(PyObject *self, PyObject *args)
{
    const char *filename;
    if (!PyArg_ParseTuple(args, "s", &filename))
        return NULL;

    void *space = df_setup(filename);
    return PyLong_FromVoidPtr(space);
}

static PyObject *
_dfk_use(PyObject *self, PyObject *args)
{
    void *space;
    if (!PyArg_ParseTuple(args, "I", &space))
        return NULL;

    df_use((void *)space);
    Py_RETURN_NONE;
}

static PyObject *
_dfk_cf(PyObject *self, PyObject *args)
{
    const char *str;
    if (!PyArg_ParseTuple(args, "s", &str))
        return NULL;

    const long count = cf(str);
    return PyLong_FromLong(count);
}

static PyObject *
_dfk_dfn(PyObject *self, PyObject *args)
{
    const long n;
    const char *str;
    if (!PyArg_ParseTuple(args, "Is", &n, &str))
        return NULL;

    const long count = dfn(n, str);
    return PyLong_FromLong(count);
}

static PyObject *
_dfk_total_document(PyObject *self, PyObject *args)
{
    const long count = df_total_document();
    return PyLong_FromLong(count);
}

// MethodDef
static PyMethodDef _dfkMethods[] = {
    {"setup", _dfk_setup, METH_VARARGS, "Setup space"},
    {"use", _dfk_use, METH_VARARGS, "Select space to use"},
    {"cf", _dfk_cf, METH_VARARGS, "Count term frequency"},
    {"dfn", _dfk_dfn, METH_VARARGS, "Count document frequency"},
    {"df_total_document", _dfk_total_document, METH_VARARGS, "Returns number of current documents"},
    {NULL, NULL, 0, NULL}
};

// ModuleDef
static struct PyModuleDef _dfkmodule = {
    PyModuleDef_HEAD_INIT,
    "_dfk",
    "The module for counting term and document frequencies",
    -1,
    _dfkMethods
};

// Module Init
PyMODINIT_FUNC
PyInit__dfk(void)
{
    PyObject *m;
    m = PyModule_Create(&_dfkmodule);
    if (m == NULL)
        return NULL;
    return m;
}
