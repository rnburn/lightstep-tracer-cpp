#include <Python.h>

#include "lightstep/tracer.h"
#include "opentracing/dynamic_load.h"
#include "python_bridge_tracer/module.h"

namespace lightstep {
//--------------------------------------------------------------------------------------------------
// MakeTracer
//--------------------------------------------------------------------------------------------------
static PyObject* MakeTracer(PyObject* /*self*/, PyObject* args, PyObject* keywords) noexcept try {
  static char* keyword_names[] = {const_cast<char*>("config"), nullptr};
  char* config;
  if (!PyArg_ParseTupleAndKeywords(args, keywords, "s:Tracer", keyword_names,
        &config)) {
    return nullptr;
  }
  auto tracer = MakeLightStepTracer(config);
  return python_bridge_tracer::makeTracer(std::move(tracer));
} catch (const std::exception& e) {
  // TODO: exception
  Py_RETURN_NONE;
}

//--------------------------------------------------------------------------------------------------
// ModuleMethods
//--------------------------------------------------------------------------------------------------
static PyMethodDef ModuleMethods[] = {
    {"Tracer", reinterpret_cast<PyCFunction>(MakeTracer),
     METH_VARARGS | METH_KEYWORDS, PyDoc_STR("Constructs a lightstep tracer")},
    {nullptr, nullptr}};

//--------------------------------------------------------------------------------------------------
// ModuleDefinition
//--------------------------------------------------------------------------------------------------
static PyModuleDef ModuleDefinition = {PyModuleDef_HEAD_INIT, "lightstep",
                                       "doc string", -1, ModuleMethods};

} // namespace lightstep

//--------------------------------------------------------------------------------------------------
// PyInit_lightstep
//--------------------------------------------------------------------------------------------------
extern "C" {
PyMODINIT_FUNC PyInit_lightstep() noexcept {
  using namespace lightstep;
  auto module = PyModule_Create(&ModuleDefinition);
  if (module == nullptr) {
    return nullptr;
  }
  if (!python_bridge_tracer::setupClasses(module)) {
    return nullptr;
  }
  return module;
}
} // extern "C"
