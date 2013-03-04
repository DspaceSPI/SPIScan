from sqlalchemy import (
    Column,
    Integer,
    Text,
    Table,
    MetaData,
    )
from sqlalchemy.orm import mapper
from sqlalchemy import create_engine
from sqlalchemy.ext.declarative import declarative_base
#from pyramid.config import Configurator
from sqlalchemy.orm import (
    scoped_session,
    sessionmaker,
    )

from zope.sqlalchemy import ZopeTransactionExtension
from pyramid import config

meta = MetaData()

DBSession = scoped_session(sessionmaker(extension=ZopeTransactionExtension()))
Base = declarative_base()

engine = create_engine('postgresql://spiscan:spiscan@localhost:5432/spiscan')
event_table = Table('event', meta, autoload=True, autoload_with=engine)

class Event(object):
    pass
    
mapper(Event, event_table)

##class MyModel(Base):
##   __tablename__ = 'models'
##    id = Column(Integer, primary_key=True)
##    name = Column(Text, unique=True)
##    value = Column(Integer)

##    def __init__(self, name, value):
##        self.name = name
##        self.value = value
        

