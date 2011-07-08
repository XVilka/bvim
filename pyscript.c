#include <Python.h>
#include "bvi.h"
#include "set.h"

/* Save current buffer into file */
static PyObject* bvi_save()
{
	save(filename, start, end, flags);
}

/* Load file into current buffer */
static PyObject* bvi_load()
{
	if (mode == MODE_ADD)
	{
		addfile(filename);
	}
	else
	{
		load(filename);
	}
}

/* Get file pointer for current buffer */
static PyObject* bvi_file()
{}

/* Execute bvi cmd */
static PyObject* bvi_exec()
{
	docmdline(cmdline);
}

static PyObject* bvi_display_error()
{
	emsg(message);
}

static PyObject* bvi_status_line_msg()
{
	msg(message);
}

/* Undo */
static PyObject* bvi_undo()
{}

/* Redo */
static PyObject* bvi_redo()
{}

/* Set any bvi parameter (analog of :set param cmd) */
static PyObject* bvi_set_param()
{}

static PyObject* bvi_scrolldown()
{
	scrolldown(lines);
}

static PyObject* bvi_scrollup()
{
	scrollup(lines);
}

/* Insert count of bytes at position */
static PyObject* bvi_insert()
{}

/* Overwrite count of bytes with custom data */
static PyObject* bvi_overwrite()
{}

/* Remove count of bytes from position */
static PyObject* bvi_remove()
{}

/* Get current cursor position */
static PyObject* bvi_cursor()
{}

/* Display arbitrary address on the screen */
static PyObject* bvi_setpage()
{
	setpage(address);
}

/* -------------------------------------------------------------------------------------
 *                                 EXPORTED METHODS
 * -------------------------------------------------------------------------------------
 */

static PyMethodDef BviMethods[] = {
	{},
	{ NULL, NULL, 0, NULL }
}

/* -------------------------------------------------------------------------------------
 *
 *  Initialization of Python scripting support and loading plugins ...
 *
 * -------------------------------------------------------------------------------------
 */

static bvi_python_init()
{
	Py_Initialize();
	Py_InitModule("bvi", BviMethods);
}

static bvi_python_finish()
{
	Py_Finalize();
}
