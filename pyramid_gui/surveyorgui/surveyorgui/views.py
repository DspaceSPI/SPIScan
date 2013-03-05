from pyramid.response import Response
from pyramid.view import view_config
from random import randint
from .models import Event, DBSession
#from sqlalchemy.exc import DBAPIError
#from .models import (
#    DBSession,
#    MyModel,
#    )
#from pyramid.renderers import get_renderer
import datetime
#from random import randint

class myView(object):

    def __init__(self, request):
        self.request = request

    def __call__(self):
        return {}    

## no home route is defined, I presume that's why going to root produces 404

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
#    	open("/tmp/hit-update","a").write("update hit on %s\n" % datetime.datetime.now().strftime("%Y-%m-%d-%H-%M-%S"))
	open("/tmp/hit-update","a").write("scan on %s\n" % datetime.datetime.utcnow())
        e = Event()
        e.spirecord=None
        e.spifileprefix="modtest"
        e.spidate=datetime.datetime.utcnow()
        e.type="scan"
        DBSession.add(e)
        return [
            randint(0,100),
            randint(0,100),
            randint(0,100),
            randint(0,100),
            888,
        ]
