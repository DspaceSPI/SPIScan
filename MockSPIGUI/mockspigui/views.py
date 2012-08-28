from pyramid.view import view_config

@view_config(route_name='home', renderer='templates/spiscansurveyor.pt')
def home(request):
    return {'project':'MockSPIGUI'}

@view_config(route_name='deploy', renderer='templates/spiscansurveyor.pt')
def deploy(request):
    return {'project':'MockSPIGUI'}

@view_config(route_name='setup', renderer='templates/setup.pt')
def setup(request):
    return {'project':'MockSPIGUI'}

@view_config(route_name='io', renderer='templates/io.pt')
def io(request):
    return {'project':'MockSPIGUI'}

@view_config(route_name='files', renderer='templates/files.pt')
def files(request):
    return {'project':'MockSPIGUI'}

@view_config(route_name='manual', renderer='templates/manual.pt')
def manual(request):
    return {'project':'MockSPIGUI'}
