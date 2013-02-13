from pyramid.response import Response
from pyramid.view import view_config
from time import time, strftime
from sqlalchemy.exc import DBAPIError
from layouts import Layouts
from dummy_data import COMPANY
from dummy_data import PEOPLE
from dummy_data import PROJECTS
from .models import (
    DBSession,
    MyModel,
    )

class ProjectorViews(Layouts):

    def __init__(self, request):
        self.request = request

    @view_config(route_name='home', renderer="templates/spiscansurveyor.pt")
    def index_view(self):
        return {"page_title": "Deploy"}

    @view_config(renderer="templates/spiscansurveyor.pt", name="spiscansurveyor.html")
    def index_view(self):
        return {"page_title": "Deploy"}

    @view_config(renderer="templates/setup.pt", name="setup.html")
    def about_view(self):
        return {"page_title": "System Setup"}

    @view_config(renderer="templates/io.pt", name="io.html")
    def about_view(self):
        return {"page_title": "Input / Output"}

    @view_config(renderer="templates/files.pt", name="files.html")
    def about_view(self):
        return {"page_title": "File Management"}

    @view_config(renderer="templates/manual.pt", name="manual.html")
    def about_view(self):
        return {"page_title": "SPIScan Manual"}

    @view_config(renderer="templates/people.pt", name="people")
    def people_view(self):
        return {"page_title": "People", "people": PEOPLE}

    @view_config(renderer="json", name="updates.json")
    def updates_view(self):
        return [
            strftime("%H:%M:%S"), 
            randint(1,1000)
        ]

#@view_config(route_name='home', renderer='templates/spiscansurveyor.pt')
#def my_view(request):
#    try:
#        one = DBSession.query(MyModel).filter(MyModel.name == 'one').first()
#    except DBAPIError:
#        return Response(conn_err_msg, content_type='text/plain', status_int=500)
#    return {'one': one, 'project': 'surveyorgui'}

#@view_config(route_name='deploy', renderer='templates/spiscansurveyor.pt')
#def my_view(request):
#    try:
#        one = DBSession.query(MyModel).filter(MyModel.name == 'one').first()
#    except DBAPIError:
#        return Response(conn_err_msg, content_type='text/plain', status_int=500)
#    return {'one': one, 'project': 'surveyorgui'}

conn_err_msg = """\
Pyramid is having a problem using your SQL database.  The problem
might be caused by one of the following things:

1.  You may need to run the "initialize_surveyorgui_db" script
    to initialize your database tables.  Check your virtual 
    environment's "bin" directory for this script and try to run it.

2.  Your database server may not be running.  Check that the
    database server referred to by the "sqlalchemy.url" setting in
    your "development.ini" file is running.

After you fix the problem, please restart the Pyramid application to
try it again.
"""
