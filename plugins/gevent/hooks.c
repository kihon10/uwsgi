#include "gevent.h"

extern struct uwsgi_server uwsgi;
extern struct uwsgi_gevent ugevent;


int uwsgi_gevent_wait_write_hook(struct wsgi_request *wsgi_req) {

        PyObject *ret = NULL;

        /// create a watcher for writes
        PyObject *watcher = PyObject_CallMethod(ugevent.hub_loop, "io", "ii", wsgi_req->poll.fd, 2);
        if (!watcher) return -1;

        PyObject *timer = PyObject_CallMethod(ugevent.hub_loop, "timer", "i", uwsgi.shared->options[UWSGI_OPTION_SOCKET_TIMEOUT]);
        if (!timer) {
                Py_DECREF(watcher);
		return -1;
        }

        PyObject *current_greenlet = GET_CURRENT_GREENLET;
        PyObject *current = PyObject_GetAttrString(current_greenlet, "switch");

        ret = PyObject_CallMethod(watcher, "start", "OO", current, watcher);
        if (!ret) {
                stop_the_watchers_and_clear
		return -1;
        }
        Py_DECREF(ret);

        ret = PyObject_CallMethod(timer, "start", "OO", current, timer);
        if (!ret) {
                stop_the_watchers_and_clear
		return -1;
        }
        Py_DECREF(ret);

        ret = PyObject_CallMethod(ugevent.hub, "switch", NULL);
        wsgi_req->switches++;
        if (!ret) {
                stop_the_watchers_and_clear
		return -1;
        }
        Py_DECREF(ret);

        if (ret == timer) {
		stop_the_watchers_and_clear;
                return -1;
        }

        stop_the_watchers_and_clear;
	return 0;
}

