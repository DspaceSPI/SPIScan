#!/usr/bin/python
# -*- coding: utf-8 -*-

import psycopg2
import sys

conn = None

try:
     
    conn = psycopg2.connect('dbname=spiscan user=spiscan')     
    cur = conn.cursor() 
    cur.execute('SELECT * FROM event')    
    col_names = [cn[0] for cn in cur.description]
    rows = cur.fetchall()
    for row in rows:    
        print row
    
except psycopg2.DatabaseError, e:
    print 'Error %s' % e    
    sys.exit(1)
    
    
finally:
    
    if conn:
        conn.close()
