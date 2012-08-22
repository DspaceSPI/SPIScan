%module dspace
%{
 extern void scan_start(int n);
 extern void scan_wait();
 extern void scan_cancel();
 extern void startup_dspace();
 extern int gps_locked(void);
 extern double gps_long(void);
 extern double gps_lat(void);
#include "../ArdInterface/ardintf.h"
%}

extern void scan_start(int n);
extern void scan_wait();
extern void scan_cancel();
extern void startup_dspace();
extern int gps_locked(void);
extern double gps_long(void);
extern double gps_lat(void);
#include "../ArdInterface/ardintf.h"
