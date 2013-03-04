#!/usr/bin/python
# -*- coding: utf-8 -*-

import psycopg2
import sys


con = None

try:
     
    con = psycopg2.connect("dbname='testdb' user='janbodnar'") 
    
    cur = con.cursor() 
    cur.execute('SELECT * FROM cars')    
    col_names = [cn[0] for cn in cur.description]
    
    rows = cur.fetchall()
    
    print "%s %-10s %s" % (col_names[0], col_names[1], col_names[2])

    for row in rows:    
        print "%2s %-10s %s" % row
    
   

except psycopg2.DatabaseError, e:
    print 'Error %s' % e    
    sys.exit(1)
    
    
finally:
    
    if con:
        con.close()
