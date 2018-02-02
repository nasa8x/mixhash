#include <Python.h>

#include "Lyra2RE.h"

static PyObject *lyra2re2_getpowhash(PyObject *self, PyObject *args)
{
    char *output;
    PyObject *value;
#if PY_MAJOR_VERSION >= 3
    PyBytesObject *input;
#else
    PyStringObject *input;
#endif
    if (!PyArg_ParseTuple(args, "S", &input))
        return NULL;
    Py_INCREF(input);
    output = PyMem_Malloc(32);

#if PY_MAJOR_VERSION >= 3
    lyra2re2_hash((char *)PyBytes_AsString((PyObject*) input), output);
#else
    lyra2re2_hash((char *)PyString_AsString((PyObject*) input), output);
#endif
    Py_DECREF(input);
#if PY_MAJOR_VERSION >= 3
    value = Py_BuildValue("y#", output, 32);
#else
    value = Py_BuildValue("s#", output, 32);
#endif
    PyMem_Free(output);
    return value;
}

static PyMethodDef Lyra2RE2Methods[] = {
    { "getPoWHash", lyra2re2_getpowhash, METH_VARARGS, "Returns the proof of work hash using Lyra2RE hash" },
    { NULL, NULL, 0, NULL }
};

#if PY_MAJOR_VERSION >= 3
static struct PyModuleDef Lyra2RE2Module = {
    PyModuleDef_HEAD_INIT,
    "lyra2re2_hash",
    "...",
    -1,
    Lyra2RE2Methods
};

PyMODINIT_FUNC PyInit_lyra2re2_hash(void) {
    return PyModule_Create(&Lyra2RE2Module);
}

#else

PyMODINIT_FUNC initlyra2re2_hash(void) {
    (void) Py_InitModule("mixhash_lyra2re2", Lyra2RE2Methods);
}
#endif
