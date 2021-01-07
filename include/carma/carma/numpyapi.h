/*  carma/numpyapi.h: Wrapper around Numpy's API
 *
 *  Adapated from:
 *
 *      pybind11/numpy.h: Basic NumPy support, vectorize() wrapper
 *      Copyright (c) 2016 Wenzel Jakob <wenzel.jakob@epfl.ch>
 *      All rights reserved. Use of this source code is governed by a
 *      BSD-style license that can be found in the LICENSE file.
 *
 *  Copyright (c) 2021 Ralph Urlus <rurlus.dev@gmail.com>
 *  All rights reserved. Use of this source code is governed by a
 *  Apache-2.0 license that can be found in the LICENSE file.
 */

#ifndef INCLUDE_CARMA_CARMA_NUMPYAPI_H_
#define INCLUDE_CARMA_CARMA_NUMPYAPI_H_

#define NPY_NO_DEPRECATED_API NPY_1_14_API_VERSION
/* C headers */
#include <Python.h>
#include <pymem.h>
#include <numpy/arrayobject.h>
#include <numpy/ndarraytypes.h>

#include <pybind11/pybind11.h>  // NOLINT

namespace py = pybind11;

namespace carma {
namespace api {

inline void * NPy_Malloc(size_t size) {
    return PyMem_RawMalloc(size);
}

inline void NPy_free(void *ptr) {
    PyMem_RawFree(ptr);
}

inline npy_intp* NPyDimMem_NEW(int nd) {
    return static_cast<npy_intp*>(PyMem_RawMalloc(nd * sizeof(npy_intp)));
}

struct npy_api {
    typedef struct {
        Py_intptr_t *ptr;
        int len;
    } PyArray_Dims;

    static npy_api& get() {
        static npy_api api = lookup();
        return api;
    }

    PyObject *(*PyArray_NewCopy_)(PyObject *, int);
    void (*PyArray_Free_)(PyObject *, void* ptr);
    void *(*PyDataMem_NEW_)(size_t nbytes);

 private:
    enum functions {
        API_PyArray_NewCopy = 85,
        API_PyArray_Free = 165,
        API_PyDataMem_NEW = 288,
    };

    static npy_api lookup() {
        py::module m = py::module::import("numpy.core.multiarray");
        auto c = m.attr("_ARRAY_API");
#if PY_MAJOR_VERSION >= 3
        void **api_ptr = reinterpret_cast<void **>(PyCapsule_GetPointer(c.ptr(), NULL));
#else
        void **api_ptr = reinterpret_cast<void **>(PyCObject_AsVoidPtr(c.ptr()));
#endif
        npy_api api;
#define DECL_NPY_API(Func) api.Func##_ = (decltype(api.Func##_)) api_ptr[API_##Func];
        DECL_NPY_API(PyArray_NewCopy);
        DECL_NPY_API(PyArray_Free);
        DECL_NPY_API(PyDataMem_NEW);
#undef DECL_NPY_API
        return api;
    }
};

}  // namespace api
}  // namespace carma

#endif  // INCLUDE_CARMA_CARMA_NUMPYAPI_H_
