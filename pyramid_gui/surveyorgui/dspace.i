%module dspace

%{
 extern void scan_start(int n);
 extern void scan_wait();
 extern void scan_cancel();
 extern int scan_done();
 extern void startup_dspace();
 extern int gps_locked(void);
 extern double gps_long(void);
 extern double gps_lat(void);
 extern void scan_description(char *v);
 extern void tiff_name(char *v);
#include "../ArdInterface/ardintf.h"
%}

extern void scan_start(int n);
extern void scan_wait();
extern void scan_cancel();
extern void startup_dspace();
extern int gps_locked(void);
extern double gps_long(void);
extern double gps_lat(void);
extern int scan_done();
extern void scan_description(char *v);
extern void tiff_name(char *v);
%include "../ArdInterface/ardintf.h"
