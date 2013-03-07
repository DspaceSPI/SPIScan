from pyramid.response import Response
from pyramid.view import view_config
from random import randint
from .models import Event, DBSession
#from sqlalchemy.exc import DBAPIError
#from .models import (
#    DBSession,
#    mymodel,
#    )
#from pyramid.renderers import get_renderer
import datetime
import subprocess
import time
import pygame
import pygame.camera


class myView(object):

    def __init__(self, request):
        self.request = request

    def __call__(self):
        return {}    

    @view_config(renderer="templates/spiscansurveyor.pt", route_name="home")
    def home_view(request):
        return {}

    @view_config(renderer="templates/spiscansurveyor.pt", name="spiscansurveyor.html")
    def index_view(request):
        return {}

    @view_config(renderer="templates/setup.pt", name="setup.html")
    def setup_view(request):
        return {}

    @view_config(renderer="templates/io.pt", name="io.html")
    def io_view(request):
        return {}

    @view_config(renderer="templates/files.pt", name="files.html")
    def files_view(request):
        return {}

    @view_config(renderer="templates/manual.pt", name="manual.html")
    def manual_view(request):
        return {}        
            
    @view_config(renderer="json", name="update")
    def update_view(self):
    	now = datetime.datetime.utcnow()
    	outfile = ("/home/spiscan/scan/%s" % now.strftime("SPI_%Y%m%d%H%M%S") + ".jpg")
#    	open("/tmp/hit-update","a").write("update hit on %s\n" % datetime.datetime.now().strftime("%Y-%m-%d-%H-%M-%S"))
#	open("/tmp/hit-update","a").write("scan on %s\n" % datetime.datetime.utcnow())
        e = Event()
        e.spirecord=None
        e.spifileprefix="modtest"
        e.spidate=now
        e.type="SPI"
        e.filename= outfile
        DBSession.add(e)
#	 comtest=subprocess.Popen('', bufsize=0, executable=none, stdin=None, stdout=None, stderr=None, preexec_fn=None, close_fds=False, cwd=None, env=None, universal_newlines=False, startupinfo=None, creationflags=0)
 	return [
 	     randint(0,100),
 	     randint(0,100),
 	     randint(0,100),
 	     randint(0,100),
 	     888,
 	]
 	
    @view_config(renderer="json", name="stopcam")
    def stopcam_view(self):
	cmdkill = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/killmjpg.sh"
	subprocess.call(cmdkill, shell=True)
 	return []
 	
    @view_config(renderer="json", name="startcam")
    def startcam_view(self):
	cmdlostart = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/lostart.sh"
	subprocess.call(cmdlostart,shell=True)
 	return []
        
    @view_config(renderer="json", name="camcapture")
    def camcapture_view(self):
	now = datetime.datetime.utcnow()
	outfile = ("/home/brian/cam/%s" % now.strftime("CAM_%Y%m%d%H%M%S") + ".jpg")
	e = Event()
        e.spirecord=None
        e.spifileprefix="desktest"
        e.spidate= now
#       e.spidate= now.strftime("Y%m%d%H%M%S")
        e.type="CAM"
        e.filename=outfile 
        DBSession.add(e)
#	cmdkill = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/killmjpg.sh"
#	cmdlostart = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/lostart.sh"
#	cmdhistart = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/histart.sh"
#	cmdframegrab = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/framegrab.sh"
#	subprocess.check_call(cmdkill, shell=True)
#       time.sleep(2)
	pygame.camera.init()
	cam = pygame.camera.Camera("/dev/video1",(1920,1080))
	cam.start()
	img = cam.get_image()
	pygame.image.save(img,"/tmp/capture.jpg")
#	time.sleep(2)
	cam.stop()
#	time.sleep(2)
#	subprocess.call(cmdlostart,shell=True)
#	open("/tmp/capture","a").write("frame grab on " + now)
        inpfile = "/tmp/capture.jpg"
        tfile = "/home/.spiscan/runtime.conf"
        label = ("SPIScan Surveyor SN7 %s" % now.strftime("%Y%m%d%H%M%S")) 
#        time.sleep(2)
        args = []
        args += ["-background", "White"]
  	args += ["-pointsize","42"]
        args += ["label: "+ label]
        args += ["-gravity", "Center"]
        args += ["-append"]
        subprocess.call(["convert",inpfile] + args + [outfile])
	time.sleep(5)
	imp = []
	imp += ["-dissolve", "25"]
	imp += ["-gravity", "South"]
	subprocess.call(["composite"] + imp + [tfile] + [outfile] + [outfile])
	return []
