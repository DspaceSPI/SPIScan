//
//  (C) Paul Campbell paul@taniwha.com 2012
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sane/sane.h>
#include <pthread.h>
#include <jpeglib.h>
#include "savetiff.h"

#define TMP_FILE "/tmp/scan.jpg"
static char tiff_file[512] = "/tmp/scan.tiff";


typedef struct scan_type {
	long 	dpi;	
	double	length;	// high
} scan_type;

//  4800|2400|1200|600|300|200|150|100|75dpi supported by 700F
#define CIRC 100			// circumference in mm
#define SCAN_TYPE(dpi, degrees) {dpi, CIRC*(double)degrees/360.0}

static const scan_type st[] = {
	SCAN_TYPE(75, 75),		// preview
	SCAN_TYPE(300, 360),		// scan
	SCAN_TYPE(600, 360),		// HQ scan
};

static pthread_mutex_t scan_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t scan_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t scan_cond_done = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t image_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t image_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t image_cond_done = PTHREAD_COND_INITIALIZER;
static unsigned char scan_request=0;
static unsigned char scan_running=0;
static unsigned char scan_failed=0;
static unsigned char req_scan_cancel=0;
static long jpeg_size, last_jpeg_size = 0;
static void *jpeg_thread(void*x);
static unsigned char last_scan_valid = 0;
static unsigned long last_scan_size = 0;
static unsigned char *last_scan = 0;
static unsigned char *next_scan = 0;
static unsigned long scan_size = 0;
static unsigned char this_scan_type = 0;
static unsigned char jpeg_active=0, jpeg_start=0, jpeg_cancel=0, jpeg_done = 0;
static long scan_width, scan_height;
static unsigned char saving_tiff=0;

void
tiff_name(char *v)
{
        strncpy(tiff_file, v, sizeof(tiff_file));
        tiff_file[sizeof(tiff_file)-1] = 0;
}

void *
scan_thread(void *v)
{
	const SANE_Device **list;
	SANE_Status res;
	unsigned char initted = 0, opened=0, not_retried=0, retried, initted_already;
	int resolution_option = -1;
	int br_y_option = -1;
	SANE_Handle handle;
	SANE_Parameters p;
	int i;
	long err, len, l, off;
	unsigned char done=0;
	int num_dev_options;

	pthread_detach(pthread_self());
	pthread_mutex_lock(&scan_mutex);
	for (;;) {
		while (!scan_request)
			pthread_cond_wait(&scan_cond, &scan_mutex);
		scan_request = 0;
		scan_failed = 0;
		scan_running = 1;
		retried = 0;
		req_scan_cancel = 0;
		pthread_mutex_unlock(&scan_mutex);
printf("s1\n");
		if (!initted) {
			initted = 1;
			not_retried = 1;
retry:		
printf("s2\n");
			if (initted_already) {
				if (opened)
					sane_close(handle);
				sane_exit();
				opened=0;
				initted_already=0;
				resolution_option = -1;
				br_y_option = -1;
			}
printf("s3\n");
			if (!not_retried) {
				initted = 0;
				goto done;
			}
			not_retried = 0;
printf("s4\n");

			sane_init(0,0);
			initted_already=1;
			res = sane_get_devices(&list, 1);
			if (!list || !list[0]) {
				fprintf(stderr, "no scanners found '%s'\n", sane_strstatus(res));
				goto retry;
			}
printf("s5\n");
			for (i = 0; ; i++) {
				if (!list[i]) {
					fprintf(stderr, "SANE can't find a genesys scanner\n");
					goto retry;
				}
				if (strstr(list[i]->name, "genesys") != 0)
					break;
			}
printf("opening '%s'\n", list[i]->name);
			if ((res = sane_open(list[i]->name, &handle)) != SANE_STATUS_GOOD) {
				fprintf(stderr, "SANE open '%s' error '%s'\n", list[i]->name, sane_strstatus(res));
				goto retry;
			}
			opened=1;
printf("searching for options\n");
			res = sane_control_option(handle, 0, SANE_ACTION_GET_VALUE, &num_dev_options, 0);
  			if (res != SANE_STATUS_GOOD) {
      				fprintf (stderr, "Could not get value for option 0: %s\n", sane_strstatus(res));
      				goto retry;
    			}
//printf("got %d\n",num_dev_options);

			for (i = 1; i < num_dev_options ; i++) {
				const SANE_Option_Descriptor *o = sane_get_option_descriptor(handle, i);
				if (!o)
					continue;
			// --mode=Color --depth=8 --resolution=600 --format=tiff 
printf("%d: option name='%s'\n", i, o->name);
printf("%d: option unit=%d\n", i, o->unit);
//printf("%d: option title='%s'\n", i, o->title);
//printf("%d: option desc='%s'\n", i, o->desc);
printf("%d: option type=%d\n", i, o->type);
printf("%d: option size=%d\n", i, o->size);
//printf("%d: option ct=%d\n", i, o->constraint_type);
//printf("%d: option range=0x%lx\n", i, (long)o->constraint.range);
fflush(stdout);
				if (!SANE_OPTION_IS_SETTABLE (o->cap))
					continue;
				if (strcmp(o->name, "mode") == 0) {
					sane_control_option(handle, i, SANE_ACTION_SET_VALUE, (char*)"Color", 0);
				} else
				if (strcmp(o->name, "depth") == 0) {
					SANE_Word val = 8;
					sane_control_option(handle, i, SANE_ACTION_SET_VALUE, &val, 0);
				} else
				if (strcmp(o->name, "br-y") == 0) {
					br_y_option = i;
				} else
				if (strcmp(o->name, "resolution") == 0) {
					resolution_option = i;
				}
			}
		}
printf("setting options handle=%lx\n", (long)handle);
		if (resolution_option < 0) {
			fprintf(stderr, "no SANE resolution parameter found\n");
		} else {
			SANE_Word val = st[this_scan_type].dpi;
			res = sane_control_option(handle, resolution_option, SANE_ACTION_SET_VALUE, &val, 0);
			if (res != SANE_STATUS_GOOD)
				fprintf(stderr, "SANE set resolution=%d failed '%s'\n", val,  sane_strstatus(res));
		}
		if (br_y_option < 0) {
			fprintf(stderr, "no SANE br-y parameter found\n");
		} else {
			SANE_Fixed val = SANE_FIX(st[this_scan_type].length);
			//if (o->unit != SANE_UNIT_MM)
			//	val = val*st[this_scan_type].dpi/25.4;
			res = sane_control_option(handle, i, SANE_ACTION_SET_VALUE, &val, 0);
			if (res != SANE_STATUS_GOOD)
				fprintf(stderr, "SANE set length %d failed '%s'\n", val,  sane_strstatus(res));
		}
		setScanner((char*)"Canon", (char*)"700F");
printf("start\n");
		if (sane_start(handle) != SANE_STATUS_GOOD)
			goto retry;
printf("start done\n");
		if ((res = sane_get_parameters(handle, &p)) != SANE_STATUS_GOOD) {
			fprintf(stderr, "SANE get parameters failed '%s'\n", sane_strstatus(res));
			sane_cancel(handle);
			goto cls;
		}
		len = p.bytes_per_line*p.lines;
		pthread_mutex_lock(&image_mutex);
		l = p.bytes_per_line*8;
		if (l > scan_size) {
			if (last_scan) {
				free(last_scan);
				last_scan = 0;
			}
			if (next_scan) {
				free(next_scan);
				next_scan = 0;
			}
		}
		last_scan_valid = 0;
		scan_size = l;
		if (!last_scan)
			last_scan = (unsigned char *)malloc(l);
		if (!next_scan)
			next_scan = (unsigned char *)malloc(l);
		if (!last_scan || !next_scan) {
			fprintf(stderr, "SANE memory allocation failed\n");
			sane_cancel(handle);
			goto cls;
		}
		done = 0;
		off = 0;
		scan_width = p.pixels_per_line;
		scan_height = p.lines;
		jpeg_start = 1;
		jpeg_cancel = 0;
		jpeg_done = 0;
		pthread_cond_broadcast(&image_cond);
		pthread_mutex_unlock(&image_mutex);
		for (;;) {
			SANE_Int l=0;
			if (req_scan_cancel) {
				sane_cancel(handle);
				break;
			}
			res = sane_read(handle, &next_scan[0], 8*p.bytes_per_line, &l);
			off += l;
			if (l) {
				unsigned char *t;
				pthread_mutex_lock(&image_mutex);
				while (last_scan_valid) 
					pthread_cond_wait(&image_cond_done, &image_mutex);
				t = last_scan;
				last_scan = next_scan;
				next_scan = t;
				last_scan_valid = 1;
				last_scan_size = l;
				pthread_cond_broadcast(&image_cond);
				pthread_mutex_unlock(&image_mutex);
			}
			if (res != SANE_STATUS_GOOD || off >= len) {
				if (res == SANE_STATUS_EOF || (res == SANE_STATUS_GOOD && off == len)) {
					done = 1;
					break;
				}
				fprintf(stderr, "SANE read failed '%s' count = %ld/%ld\n", sane_strstatus(res), off, len);
				break;
			}
		}
		pthread_mutex_lock(&image_mutex);
		if (done && !req_scan_cancel) {
			jpeg_done = 1;
			err = 0;
		} else {
			// jpeg cancel
			jpeg_cancel = 1;
			err = 1;
		}
		pthread_cond_broadcast(&image_cond);
		while (jpeg_active)
			pthread_cond_wait(&image_cond_done, &image_mutex);
		pthread_mutex_unlock(&image_mutex);
cls:
		sane_cancel(handle);
done:
		pthread_mutex_lock(&scan_mutex);
		scan_running = 0;
			
		pthread_cond_broadcast(&scan_cond_done);
	}
	pthread_mutex_unlock(&scan_mutex);
	sane_exit();
	return 0;
}

void
startup_scan()
{
	static unsigned char thread_running=0;

	pthread_mutex_lock(&scan_mutex);
	if (!thread_running) {
		pthread_t tid;
		thread_running = 1;
		pthread_create(&tid, 0, scan_thread, 0);
		pthread_create(&tid, 0, jpeg_thread, 0);
	}
	pthread_mutex_unlock(&scan_mutex);
}

void
scan_start(int type)
{

	if (type < 0 || type  > 2)
		type = 0;
	startup_scan();
	pthread_mutex_lock(&scan_mutex);
	while (scan_running) 
		pthread_cond_wait(&scan_cond_done, &scan_mutex);
	scan_request = 1;
	this_scan_type = type;
	req_scan_cancel = 0;
	saving_tiff = (type == 2);
	pthread_cond_broadcast(&scan_cond);
	pthread_mutex_unlock(&scan_mutex);
}
void
scan_cancel(void)
{
	pthread_mutex_lock(&scan_mutex);
	if (scan_running)
		req_scan_cancel = 1;
	pthread_mutex_unlock(&scan_mutex);
}

void
scan_wait(void)
{
	pthread_mutex_lock(&scan_mutex);
	while (scan_running || scan_request) 
		pthread_cond_wait(&scan_cond_done, &scan_mutex);
	pthread_mutex_unlock(&scan_mutex);
}

int
scan_done(void)
{
        int res;
	pthread_mutex_lock(&scan_mutex);
        res = (scan_running || scan_request ? 0:1);
	pthread_mutex_unlock(&scan_mutex);
        return res;
}

static void *
jpeg_thread(void*x)
{
	FILE *outfile = 0;
	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr jerr;
	JSAMPROW row_pointer[8];
	int i, stride;
	unsigned char *off;

	pthread_mutex_lock(&image_mutex);
	for (;;) {
		while (!jpeg_start && !(jpeg_cancel&&jpeg_active) && !jpeg_done && !last_scan_valid)
			pthread_cond_wait(&image_cond, &image_mutex);
		if (jpeg_cancel && jpeg_active) {
printf("jpeg cancel\n");
			pthread_mutex_unlock(&image_mutex);
			if (jpeg_active) {
				pthread_mutex_unlock(&image_mutex);
				fclose(outfile);
				jpeg_destroy_compress(&cinfo);
				if (saving_tiff)
					EndSaveTIFF();
			}
			pthread_mutex_lock(&image_mutex);
			jpeg_done = 0;
			jpeg_active = 0;
			pthread_cond_broadcast(&image_cond_done);
		} else
		if (jpeg_start) {
printf("jpeg start\n");
			jpeg_active = 1;
			jpeg_start = 0;
			pthread_mutex_unlock(&image_mutex);
			cinfo.err = jpeg_std_error(&jerr);
printf("jpeg create compress\n");
			jpeg_create_compress(&cinfo);
			outfile = fopen(TMP_FILE, "w");
			if (!outfile) {
printf("open failed\n");
				jpeg_active = 0;
				jpeg_destroy_compress(&cinfo);
			} else {
				jpeg_stdio_dest(&cinfo, outfile);
				cinfo.image_width = scan_width;
				cinfo.image_height = scan_height;
				cinfo.input_components = 3;
				cinfo.in_color_space = JCS_RGB;
printf("set defaults\n");
				jpeg_set_defaults(&cinfo);
printf("set quality\n");
				jpeg_set_quality(&cinfo, 100, TRUE /* limit to baseline-JPEG values */);
printf("start compress\n");
				jpeg_start_compress(&cinfo, TRUE);
				stride = 3*scan_width;
				if (saving_tiff && tiff_file[0]) {
					setResolution(st[this_scan_type].dpi);
					StartSaveTIFF(tiff_file, scan_width, scan_height);
				}
			}
printf("jpeg start done\n");
			pthread_mutex_lock(&image_mutex);
		} else
		if (last_scan_valid) {
			int count;
			pthread_mutex_unlock(&image_mutex);
			if (jpeg_active) {
				count = 0;
				for (i = 0; i < last_scan_size; i += stride) {
					row_pointer[count] = &last_scan[i];
					if (saving_tiff) 
						LineSaveTIFF(row_pointer[count]);
					count++;
				}
				jpeg_write_scanlines(&cinfo, &row_pointer[0], count);
			}
			pthread_mutex_lock(&image_mutex);
			last_scan_valid = 0;
			pthread_cond_broadcast(&image_cond_done);
		} else
		if (jpeg_done) {
printf("jpeg done\n");
			pthread_mutex_unlock(&image_mutex);
			if (jpeg_active) {
				jpeg_finish_compress(&cinfo);
				fclose(outfile);
				jpeg_destroy_compress(&cinfo);
				if (saving_tiff)
					EndSaveTIFF();
			}
			pthread_mutex_lock(&image_mutex);
			jpeg_done = 0;
			jpeg_active = 0;
			pthread_cond_broadcast(&image_cond_done);
		}
	}
	pthread_mutex_unlock(&image_mutex);
	return 0;
}

void scan_description(char *v) { extern void setDescription(char *); setDescription(v); }
