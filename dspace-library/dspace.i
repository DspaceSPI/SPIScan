%module dspace
%{
 extern void scan_start(int n);
 extern void scan_wait();
 extern void scan_cancel();
 extern void startup_dspace();
#include "../ArdInterface/ardintf.h"
%}

extern void scan_start(int n);
extern void scan_wait();
extern void scan_cancel();
extern void startup_dspace();
#include "../ArdInterface/ardintf.h"
