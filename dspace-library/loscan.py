import dspace
import time
done = 0
dspace.scan_description("test description");
while done==0:
	dspace.scan_start(1);
        while dspace.scan_done()==0 :
                time.sleep(1)
        print "scan done"
	done=1

