from pyramid.response import Response
from pyramid.view import view_config
from random import randint
from .models import Event, DBSession, MyModel
from sqlalchemy import *
from sqlalchemy.orm import sessionmaker
import datetime
import subprocess
import time
import pygame
import pygame.camera
import sys 
import dspace
from json import *

def stopcam(self):
    cmdkill = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/killmjpg.sh"
    subprocess.call(cmdkill, shell=True)
    return []

def startcam(self):
    cmdlostart = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/lostart.sh"
    subprocess.call(cmdlostart,shell=True)
    return []

@view_config(renderer='json', name="fieldchange")
def fieldchange_view(request):
    jsondata = request.json_body
    project = jsondata['project']
    spifileprefix = jsondata['spifileprefix']
    memo = jsondata['memo']
    bright = jsondata['bright']
    scanresolution = jsondata['scanresolution']
    with open("/home/brian/spiscanconfig/deploydata.txt", "w") as text_file:
        text_file.write(
        "%s\n" % project + 
        "%s\n" % spifileprefix + 
        "%s\n" % memo + 
        "%s\n" % bright + 
        "%s\n" % scanresolution) 
    return ['OK']

@view_config(renderer='json', name="setupchange")
def setupchange_view(request):
    setupdata = request.json_body
    topmargin = setupdata['topmargin']
    endmargin = setupdata['endmargin']
    startmargin = setupdata['startmargin']
    botmargin = setupdata['botmargin']
    pretopmargin = setupdata['pretopmargin']
    preendmargin = setupdata['preendmargin']
    prestartmargin = setupdata['prestartmargin']
    prebotmargin = setupdata['prebotmargin']
    plcendmargin = setupdata['plcendmargin']
    plcstartmargin = setupdata['plcstartmargin']
    autoscaninterval = setupdata['autoscaninterval']
    overlaytext = setupdata['overlaytext']
    autopiccapture = setupdata['autopiccapture']
    viddev = setupdata['viddev']
    with open("/home/brian/spiscanconfig/currentsetup.txt", "w") as text_file:
        text_file.write(
        "%s\n" % topmargin + 
        "%s\n" % endmargin + 
        "%s\n" % startmargin + 
        "%s\n" % botmargin + 
        "%s\n" % pretopmargin + 
        "%s\n" % preendmargin + 
        "%s\n" % prestartmargin + 
        "%s\n" % prebotmargin + 
        "%s\n" % plcendmargin + 
        "%s\n" % plcstartmargin + 
        "%s\n" % autoscaninterval + 
        "%s\n" % autopiccapture + 
        "%s\n" % overlaytext + 
        "%s\n" % viddev) 
    return ['OK']

@view_config(renderer="json", name="camcapture")
def camcapture(request):
    jsondata = request.json_body
    spifileprefix = jsondata['spifileprefix']
    memo = jsondata['memo']
    project = jsondata['project']
    now = datetime.datetime.utcnow()
    outfile = ("/home/brian/cam/%s" % now.strftime("CAM_%Y%m%d%H%M%S") + ".jpg")
    cam_event = Event(spifileprefix, now, 'CAM', outfile, memo, project)
    DBSession.add(cam_event)
    stopcam(1)
    ##cmdkill = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/killmjpg.sh"
    ##cmdlostart = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/lostart.sh"
    ##cmdhistart = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/histart.sh"
    ##cmdframegrab = "bash /home/brian/DspaceSPI/SPIScan/mjpg-streamer/framegrab.sh"
    ##subprocess.check_call(cmdkill, shell=True)
    pygame.camera.init()
    cam = pygame.camera.Camera("/dev/video1",(1920,1080))
    cam.start()
    img = cam.get_image()
    pygame.image.save(img,"/tmp/capture.jpg")
    cam.stop()
    time.sleep(1)
    ##subprocess.call(cmdlostart,shell=True)
    startcam(1)
    inpfile = "/tmp/capture.jpg"
    tfile = "/home/.spiscan/runtime.conf"
    label = ("SPIScan Surveyor SN7 %s" % now.strftime("%Y%m%d%H%M%S")) 
    args = []
    args += ["-background", "White"]
    args += ["-pointsize","42"]
    args += ["label: "+ label]
    args += ["-gravity", "Center"]
    args += ["-append"]
    subprocess.call(["convert",inpfile] + args + [outfile])
    time.sleep(5)
    imp = []
    imp += ["-dissolve", "45"]
    imp += ["-gravity", "South"]
    subprocess.call(["composite"] + imp + [tfile] + [outfile] + [outfile])
    thmbfile = ("/home/brian/cam/thumbs/%s" % now.strftime("CAM_%Y%m%d%H%M%S") + ".jpg")
    thmb = []
    thmb += ["-resize","80x80"]
    subprocess.call(["convert",outfile] + thmb + [thmbfile])
    return []

@view_config(renderer="json", name="update")
def update_view(request):
    jsondata = request.json_body
    spifileprefix = jsondata['spifileprefix']
    memo = jsondata['memo']
    project = jsondata['project']
    dspace.tiff_name("/tmp/test.tiff")
    dspace.scan_description("test description")
    now = datetime.datetime.utcnow()
    outfile = ("/home/brian/scan/%s" % now.strftime("SPI_%Y%m%d%H%M%S") + ".jpg")
    lastscanfile = ("/home/brian/spiscanconfig/lastscan.jpg")
    scan_event = Event(spifileprefix , now,'SPI',outfile, memo ,project)
    DBSession.add(scan_event)  #adds record of scan event to database but not committed until successful return
    done = 0  # Start actual scan
    dspace.scan_description("test description")
    print "called dspace.scan_description successfully"
    while done == 0:
        dspace.scan_start(2); # 0 is prescan 1 is loscan 2 is hiscan
        while dspace.scan_done()==0 :
                time.sleep(1)
        print "scan done"
        done=1
    inpfile = "/tmp/scan.jpg"
    tfile = "/home/.spiscan/runtime.conf"
    label = ("SPIScan Surveyor SN7 %s" % now.strftime("%Y%m%d%H%M%S")) 
    rotparm = []
    rotparm += ["-rotate", "-90"]
    subprocess.call(["convert",inpfile] + rotparm + [lastscanfile])
    args = []
    args += ["-background", "White"]
    args += ["-pointsize","42"]
    args += ["label: "+ label]
    args += ["-gravity", "Southeast"]
    args += ["-append"]
    subprocess.call(["convert",inpfile] + args + [inpfile])
    time.sleep(1)
    imp = []
    imp += ["-dissolve", "25"]
    imp += ["-gravity", "Southeast"]
    subprocess.call(["composite"] + imp + [tfile] + [inpfile] + [inpfile])
    time.sleep(1)
    rotparm = []
    rotparm += ["-rotate", "-90"]
    subprocess.call(["convert",inpfile] + rotparm + [outfile])
    thmbfile = ("/home/brian/scan/thumbs/%s" % now.strftime("SPI_%Y%m%d%H%M%S") + ".jpg")
    thmb = []
    thmb += ["-resize","80x80"]
    subprocess.call(["convert",outfile] + thmb + [thmbfile])
    return []

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

    @view_config(route_name='lastscan')
    def test_page(request):
        response = Response(content_type='image/jpeg')
        response.app_iter = open('/home/brian/spiscanconfig/lastscan.jpg', 'rb')
        return response

    @view_config(name='scans')
    def scanimages_view(request):
        response = Response(content_type='image/jpeg')
        response.app_iter = open('/home/brian/spiscanconfig/lastscan.jpg', 'rb')
        return response

    @view_config(renderer="templates/spifiles.pt", route_name="spifiles")
    def spifiles(request):
        dbsession = DBSession()
        data = dbsession.query(Event).order_by(Event.project).filter_by(eventtype='SPI').all()
        print data
        return {"page_title":"SPI Files", "data":data}

    @view_config(renderer="templates/camfiles.pt", route_name="camfiles")
    def camfiles(request):
        dbsession = DBSession()
        data = dbsession.query(Event).order_by(Event.project).filter_by(eventtype='CAM').all()
        print data
        return {"page_title":"CAM Files", "data":data}

    @view_config(renderer="templates/logfiles.pt", route_name="logfiles")
    def logfiles(request):
        dbsession = DBSession()
        data = dbsession.query(Event).order_by(Event.project).all()
        print data
        return {"page_title":"LOG Files", "data":data}

    @view_config(renderer='json', name="lastevent")
    def lastevent_view(request):
        levent = request.json_body
        with open("/home/brian/spiscanconfig/levent.txt", "w") as text_file:
            text_file.write("%s" % levent) 
        return ['OK']

    @view_config(renderer="json", name="placement")
    def placement_view(request):
        dspace.tiff_name("/tmp/test.tiff");
        dspace.scan_description("test description")
        now = datetime.datetime.utcnow()
        lastscanfile = ("/home/brian/spiscanconfig/lastscan.jpg")
        done = 0  # Start actual scan
        dspace.scan_description("test description")
        print "called dspace.scan_description successfully"
        while done==0:
            dspace.scan_start(0); # 0 is prescan 1 is loscan 2 is hiscan
            while dspace.scan_done()==0 :
                time.sleep(1)
            print "scan done"
            done=1
        inpfile = "/tmp/scan.jpg"
        rotparm = []
        rotparm += ["-rotate", "-90"]
        subprocess.call(["convert",inpfile] + rotparm + [lastscanfile])
        return []

    @view_config(renderer="json", name="preview")
    def preview_view(request):
        dspace.tiff_name("/tmp/test.tiff");
        dspace.scan_description("test description")
        now = datetime.datetime.utcnow()
        lastscanfile = ("/home/brian/spiscanconfig/lastscan.jpg")
        done = 0  # Start actual scan
        dspace.scan_description("test description")
        print "called dspace.scan_description successfully"
        while done == 0:
            dspace.scan_start(1) # 0 is prescan 1 is loscan 2 is hiscan
            while dspace.scan_done() == 0:
                time.sleep(1)
            print "scan done"
            done=1
        inpfile = "/tmp/scan.jpg"
        rotparm = []
        rotparm += ["-rotate", "-90"]
        subprocess.call(["convert",inpfile] + rotparm + [lastscanfile])
        return []