from pyramid.config import Configurator
from sqlalchemy import engine_from_config
from .models import (
    DBSession,
    Base,
    Event,
    )

def main(global_config, **settings):
    """ This function returns a Pyramid WSGI application.
    """
    engine = engine_from_config(settings, 'sqlalchemy.')
    DBSession.configure(bind=engine)
    Base.metadata.bind = engine
    config = Configurator(settings=settings)
    config.add_static_view('static', 'static', cache_max_age=3600)
    config.add_route('home', '/')
    config.add_route('lastscan', '/lastscan.jpg')  # added bp for serving dynamically from filesys
    config.add_route('scanslist','/scanslist.html')
    config.add_route('spifiles', '/spifiles')
    config.add_route('camfiles', '/camfiles')
    config.add_route('logfiles', '/logfiles')
    config.scan()
#    print dir(config)  // prints all object to console
    return config.make_wsgi_app()
