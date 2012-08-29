//#include "stdafx.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cmath"
#include "math.h"
//#include <java/lang/OutOfMemoryError.h>
//#include <java/lang/Exception.h>

#include <tiff.h>
#include <tiffio.h>
#define TIFF_GAMMA  2.2


static char *m_strTIFFmodel;
static char *m_strTIFFmake;
static char *m_strTIFFsoftware;
static char m_strDescription[256];
static bool m_hasGeoInformation;
static double m_latitude;
static double m_longitude;
static float m_fXres;
static float m_fYres;
static TIFF * tif=0;
static int row = 0;

int
StartSaveTIFF(const char * name, long w, long h)
{

	const char *output_file = name;


	int result = 0;

	//unsigned long total_width = GetTotalWidth();
	//unsigned long bitcount = GetBitCount();
	//unsigned long bytecount = bitcount / 8;

	double	image_gamma = TIFF_GAMMA;


	if ((tif = TIFFOpen(output_file, "w")) == NULL)
	{
		return result;
	}

	uint32 rc = 0;
	rc = TIFFCreateDirectory(tif);
		  

	if (m_strTIFFmake) {
		TIFFSetField(tif, TIFFTAG_MAKE, m_strTIFFmake);
	}
	if (m_strTIFFmodel) {
		TIFFSetField(tif, TIFFTAG_MODEL, m_strTIFFmodel);
	}
	time_t t = time(0);
	struct tm tm;
	char tb[100];
	gmtime_r(&t, &tm);
	snprintf(tb, sizeof(tb), "%04d:%02d:%02d %02d:%02d:%02d",
		tm.tm_year+1900, tm.tm_mon+1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	TIFFSetField(tif, TIFFTAG_DATETIME, &tb[0]);
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_INCH);
	if (m_fXres > 0) {
		TIFFSetField(tif, TIFFTAG_XRESOLUTION, m_fXres);
	}
	if (m_fYres > 0) {
		TIFFSetField(tif, TIFFTAG_YRESOLUTION, m_fYres);
	}
	if (m_strDescription) {
		TIFFSetField(tif, TIFFTAG_IMAGEDESCRIPTION, m_strDescription);
	}
	if (m_strTIFFsoftware) {
		TIFFSetField(tif, TIFFTAG_SOFTWARE, m_strTIFFsoftware);
	}
	TIFFSetField(tif, TIFFTAG_IMAGEWIDTH, w);
	TIFFSetField(tif, TIFFTAG_IMAGELENGTH, h);
	TIFFSetField(tif, TIFFTAG_BITSPERSAMPLE, 8);
	TIFFSetField(tif, TIFFTAG_COMPRESSION, COMPRESSION_PACKBITS);
	TIFFSetField(tif, TIFFTAG_PHOTOMETRIC, PHOTOMETRIC_RGB);
	TIFFSetField(tif, TIFFTAG_SAMPLESPERPIXEL, 3);
	TIFFSetField(tif, TIFFTAG_ROWSPERSTRIP, 1);
	TIFFSetField(tif, TIFFTAG_PLANARCONFIG, PLANARCONFIG_CONTIG);
	TIFFSetField(tif, TIFFTAG_RESOLUTIONUNIT, RESUNIT_NONE);
	TIFFSetField(tif, TIFFTAG_ORIENTATION, ORIENTATION_TOPLEFT);
	uint32 GPSIFDOffset = 0;



#define TIFFTAG_GEOPIXELSCALE	33550
#define TIFFTAG_GEOTRANSMATRIX	34264
#define TIFFTAG_GEOTIEPOINTS	33922
#define TIFFTAG_GEOKEYDIRECTORY	34735
#define TIFFTAG_GEODOUBLEPARAMS	34736
#define TIFFTAG_GEOASCIIPARAMS	34737

	static const TIFFFieldInfo xtiffFieldInfo[] = {
		{ TIFFTAG_GEOPIXELSCALE,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
			  1,	1,	(char *)"GeoPixelScale" },
		{ TIFFTAG_GEOTRANSMATRIX,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
			  1,	1,	(char *)"GeoTransformationMatrix" },
		{ TIFFTAG_GEOTIEPOINTS,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
			  1,	1,	(char *)"GeoTiePoints" },
		{ TIFFTAG_GEOKEYDIRECTORY, -1,-1, TIFF_SHORT,	FIELD_CUSTOM,
			  1,	1,	(char *)"GeoKeyDirectory" },
		{ TIFFTAG_GEODOUBLEPARAMS,	-1,-1, TIFF_DOUBLE,	FIELD_CUSTOM,
			  1,	1,	(char *)"GeoDoubleParams" },
		{ TIFFTAG_GEOASCIIPARAMS,	-1,-1, TIFF_ASCII,	FIELD_CUSTOM,
			  1,	0,	(char *)"GeoASCIIParams" }
	};

	TIFFMergeFieldInfo(tif, xtiffFieldInfo, sizeof (xtiffFieldInfo) / sizeof (xtiffFieldInfo[0]));


	const uint16 KeyDirectoryVersion	= 1;
	const uint16 KeyRevision			= 1;
	const uint16 MinorRevision			= 0;

	const uint16 GTModelTypeGeoKey		= 1024;
	const uint16 GTRasterTypeGeoKey		= 1025;
	const uint16 GeographicTypeGeoKey	= 2048;

	const uint16 ModelTypeGeographic	= 2;
	const uint16 RasterPixelIsArea		= 1;
	const uint16 GCS_WGS_84			= 4326;

		
	const uint16 geoKeys = 3;

	uint16 geoKeyDirectoryTag[] = {
		KeyDirectoryVersion,	KeyRevision,	MinorRevision,	geoKeys,
            	GTModelTypeGeoKey,		0,	1,	ModelTypeGeographic,
           	GTRasterTypeGeoKey,		0,	1,	RasterPixelIsArea,
           	GeographicTypeGeoKey,		0,	1,	GCS_WGS_84
	};


	uint16 tiepointTags = 1;

	double modelTiepointTag[] = { 0, 0, 0, m_longitude, m_latitude, 0 };


	if (m_hasGeoInformation) {
		TIFFSetField(tif, TIFFTAG_GEOTIEPOINTS, 6*tiepointTags, modelTiepointTag);
		TIFFSetField(tif, TIFFTAG_GEOKEYDIRECTORY, 4*(geoKeys + 1), geoKeyDirectoryTag);
	}

#ifdef NOTDEF
	TIFFFieldInfo tfi[] = {
		{0, 4, 4, TIFF_BYTE, FIELD_CUSTOM, 0, 0, "GPSVersionID"},
		{1, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, 0, "GPSLatitudeRef"},
		{2, 3, 3, TIFF_RATIONAL, FIELD_CUSTOM, 0, 0, "GPSLatitude"},
		{3, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, 0, "GPSLongitudeRef"},
		{4, 3, 3, TIFF_BYTE, FIELD_CUSTOM, 0, 0, "GPSLongitude"},
		{5, 1, 1, TIFF_BYTE, FIELD_CUSTOM, 0, 0, "GPSAltitudeRef"},
		{6, 1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, 0, "GPSAltitude"},
		{7, 3, 3, TIFF_RATIONAL, FIELD_CUSTOM, 0, 0, "GPSTimeStamp"},
		{8, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSSatellites"},
		{9, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSStatus"},
		{10, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSMeasureMode"},
		{11, 1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSDOP"},
		{12, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSSpeedRef"},
		{13, 1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSSpeed"},
		{14, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSTrackRef"},
		{15, 1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSTrack"},
		{16, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSImgDirectionRef"},
		{17, 1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSImgDirection"},
		{18, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSMapDatum"},
		{19, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSDestLatitudeRef"},
		{20, 3, 3, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSDestLatitude"},
		{21, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSDestLongitudeRef"},
		{22, 3, 3, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSDestLongitude"},
		{23, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSDestBearingRef"},
		{24, 1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSDestBearing"},
		{25, 2, 2, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSDestDistanceRef"},
		{26, 1, 1, TIFF_RATIONAL, FIELD_CUSTOM, 0, FALSE, "GPSDestDistance"},
		{27, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_UNDEFINED, FIELD_CUSTOM, 0, FALSE, ""},
		{28, TIFF_VARIABLE, TIFF_VARIABLE, TIFF_UNDEFINED, FIELD_CUSTOM, 0, FALSE, ""},
		{29, 11, 11, TIFF_ASCII, FIELD_CUSTOM, 0, FALSE, "GPSDateStamp"},
		{30, 1, 1, TIFF_SHORT, FIELD_CUSTOM, 0, FALSE, "GPSDifferential"}
	};
#endif
	row = 0;
	return result;
}



#ifdef NOTDEF



	unsigned char * psrc = (unsigned char *)0;//GetBits();
	unsigned char * pdst = new unsigned char[(w * 3)];
		
		
	unsigned long src_index;
	unsigned long dst_index;

		  
	for (int row = 0; row < h; row++ ){

			  
		memset(pdst,0,(size_t)(w * 3));

			  
			  
		for (int col = 0; col < w; col++){
			src_index = (h - row - 1) * total_width * bytecount + col * bytecount;
			dst_index = col * 3;
			pdst[dst_index++] = psrc[src_index+2];
			pdst[dst_index++] = psrc[src_index+1];
			pdst[dst_index] = psrc[src_index];
			result++;
		}

			  
		TIFFWriteScanline(tif, pdst, row, 0);
	}
#endif


int 
LineSaveTIFF(unsigned char *pdst)
{
	if (!tif)
		return 0;
	TIFFWriteScanline(tif, pdst, row, 0);
	row++;
	return 1;
}

int
EndSaveTIFF()
{
	int rc = TIFFWriteDirectory(tif);

	TIFFClose(tif);
	tif = 0;
	return 0;


}

void setDescription(char * desc)
{
	strncpy(m_strDescription, desc, sizeof(m_strDescription));
	m_strDescription[sizeof(m_strDescription)-1] = 0;
}


void setLatLon(double latitude, double longitude) {
	m_latitude = latitude;
	m_longitude = longitude;
	m_hasGeoInformation = true;
}


void setScanner(char * manufact, char * model)
{
	m_strTIFFmake = manufact;
	m_strTIFFmodel = model;
}

void setResolution(double dpi)
{
	m_fXres = dpi;
	m_fYres = dpi;
}





