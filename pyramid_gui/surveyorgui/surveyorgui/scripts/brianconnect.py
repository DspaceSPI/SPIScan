#!/usr/bin/python2.4
#
# Small script to show PostgreSQL and Pyscopg together
#

##import psycopg2

##try:
##    conn = psycopg2.connect("dbname='testdb' user='brian' host='localhost' password='bjuk9.0'")
##except:
##    print "I am unable to connect to the database"
    
##cur = conn.cursor()
##cur.execute("""SELECT datname from pg_database""")
##rows = cur.fetchall()
##print "\nShow me the databases:\n"
##for row in rows:
##	print "    ", row[0]
