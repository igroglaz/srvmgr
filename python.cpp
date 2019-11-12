#include <Python.h>
#include "python.h"
#include <fstream>
#include "lib\utils.hpp"
#include "srvmgr.h"

PyObject* module_mainServer = NULL;

PyObject* PyLayer_print(PyObject* self, PyObject* args)
{
	const char* strToRun = NULL;

	PyObject* printWhat = PyTuple_GetItem(args, 0);
	
	if(printWhat != NULL && PyString_Check(printWhat))
		Printf("%s", PyString_AsString(printWhat));

	Py_XDECREF(printWhat);

	return Py_BuildValue("");
}

PyObject* PyLayer_print_silent(PyObject* self, PyObject* args)
{
	PyObject* printWhat = PyTuple_GetItem(args, 0);
	
	if(printWhat != NULL && PyString_Check(printWhat))
		log_format("%s\n", PyString_AsString(printWhat));

	Py_XDECREF(printWhat);

	return Py_BuildValue("");
}

#define DECL_CALLBACK(variable, ns, name)   variable += "if(hasattr(sys.modules['__main__'], '"+std::string(name)+"') and hasattr(sys.modules['__main__']."+std::string(name)+", '__call__')):\n"; \
											variable += " sys.modules['"+std::string(ns)+"']."+std::string(name)+" = sys.modules['__main__']."+std::string(name)+";\n"; \
											variable += "else:\n"; \
											variable += " sys.modules['"+std::string(ns)+"']."+std::string(name)+" = None;\n\n";

void Python_Init()
{
	Py_Initialize();

	static PyMethodDef serverMethods[] =
	{
		{"_internal_log", PyLayer_print, METH_VARARGS, NULL},
		{"_internal_log_silent", PyLayer_print_silent, METH_VARARGS, NULL},
		{NULL, NULL, 0, NULL}
	};

	Py_InitModule("server", serverMethods);

	std::string scriptInit = "";
	scriptInit += "import server;\n";
	scriptInit += "def _server_log(*args):\n";
	scriptInit += " server._internal_log(args[0] % args[1:]);\n\n";
	scriptInit += "def _server_log_silent(*args):\n";
	scriptInit += " server._internal_log_silent(args[0] % args[1:]);\n\n";
	scriptInit += "server.log = _server_log;\n";
	scriptInit += "server.log_silent = _server_log_silent;\n";

	// recognized callbacks are
	//  1. on_server_init
	//  2. on_server_quit
	//  3. on_map_init (also server.on_map_init)
	//  4. on_map_quit (also server.on_map_quit)

	PyRun_SimpleString(scriptInit.c_str());

	if(FileExists("maps\\default.py"))
		Python_ExecuteFile("maps\\default.py");

	scriptInit = "import server;\n";
	scriptInit += "import sys;\n";
	DECL_CALLBACK(scriptInit, "server", "on_server_init");
	DECL_CALLBACK(scriptInit, "server", "on_server_quit");
	DECL_CALLBACK(scriptInit, "server", "on_map_init");
	DECL_CALLBACK(scriptInit, "server", "on_map_quit");
	DECL_CALLBACK(scriptInit, "server", "on_character_enter");
	
	PyRun_SimpleString(scriptInit.c_str());
}

void Python_Quit()
{
	Py_Finalize();
}

void Python_ExecuteFile(std::string filename)
{
	std::ifstream ifs(filename.c_str());
	if(!ifs.is_open())
	{
		Printf("Python_ExecuteFile(): Couldn't open file \"%s\".", filename.c_str());
		return;
	}

	std::string script = "";
	std::string line = "";
	while(getline(ifs, line))
		script += line+"\n";

	ifs.close();

	if(PyRun_SimpleString(script.c_str()) != 0)
	{
		Printf("Python_ExecuteFile(): Caught exception while executing \"%s\".", filename.c_str());
		PyObject* ptype = NULL;
		PyObject* pvalue = NULL;
		PyObject* ptraceback = NULL;
		PyObject* pyString = NULL;

		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		PyErr_Clear();

		std::string mMsg = "";

		if(ptype != NULL && (pyString=PyObject_Str(ptype))!=NULL && (PyString_Check(pyString)))
			mMsg = PyString_AsString(pyString);
		else mMsg = "<unknown exception>";
		Py_XDECREF(pyString);

		if(pvalue != NULL && (pyString=PyObject_Str(ptype))!=NULL && (PyString_Check(pyString)))
			mMsg += ": "+std::string(PyString_AsString(pyString));
		Py_XDECREF(pyString);

		Py_XDECREF(ptraceback);
		Py_XDECREF(pvalue);
		Py_XDECREF(ptype);
	}
}

void Python_CallMethod(std::string method)
{
	for(int i = 0; i < method.size(); i++)
	{
		if(!((method[i] >= 'A' && method[i] <= 'Z') ||
			 (method[i] >= 'a' && method[i] <= 'z') ||
			 (method[i] >= '0' && method[i] <= '9') ||
			 (method[i] == '_')))
			return;
	}

	std::string simpleCall = "";
	simpleCall += "import server;\n";
	simpleCall += "import sys;\n";
	simpleCall += "if(hasattr(sys.modules['server'], '"+method+"') and hasattr(sys.modules['server']."+method+", '__call__')):\n";
	simpleCall += " server."+method+"();\n\n";

	PyRun_SimpleString(simpleCall.c_str());
}