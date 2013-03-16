
int StartSaveTIFF(const char * name, long w, long h);
int LineSaveTIFF(unsigned char *pdst);
int EndSaveTIFF();
void setResolution(double dpi);
void setLatLon(double latitude, double longitude);
void setScanner(char * manufact, char * model);
void setDate(char * date);
void setDescription(char * desc);
