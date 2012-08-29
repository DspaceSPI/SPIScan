import dspace

dspace.scan_description("test description");
while 1:
	dspace.scan_start(2);
	dspace.scan_wait();

