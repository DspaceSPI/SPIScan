#!/usr/bin/python
# -*- coding: utf-8 -*-

import psycopg2
import sys


con = None

try:
     
    con = psycopg2.connect(database='spiscan', user='spiscan') 
    
    cur = con.cursor() 
    cur.execute("""SELECT table_name FROM information_schema.tables 
       WHERE table_schema = 'public'""")    
        
    rows = cur.fetchall()

    for row in rows:
        print row[0]
    
   
except psycopg2.DatabaseError, e:
    print 'Error %s' % e    
    sys.exit(1)
    
    
finally:
    
    if con:
        con.close()
