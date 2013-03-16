from sqlalchemy import *
from sqlalchemy.ext.declarative import declarative_base
from sqlalchemy.orm import (
    scoped_session,
    sessionmaker,
    )
from zope.sqlalchemy import ZopeTransactionExtension


engine = create_engine('postgresql://spiscan:spiscan@localhost:5432/spiscan', echo=True) # Establishes SQLAlchemy link to postgresql db
DBSession = scoped_session(sessionmaker(extension=ZopeTransactionExtension()))


class ORMClass(object):                     # Allows later grabs of database values as variables in python
    @classmethod
    def query(class_):
        return DBSession.query(class_)
        
    @classmethod
    def get(class_, spirecord):
        return Session.query(class_).get(spirecord)
        
Base = declarative_base(cls=ORMClass)

class Event(Base):        #Schema description which allows automated table generation if it doesn't already exist
    __tablename__ = 'event'
    spirecord = Column(Integer, Sequence('event_spirecord_seq'), primary_key=True, nullable=False)
    spifileprefix = Column(String)
    spidate = Column(DateTime, nullable=False)
    eventtype = Column(String(3))
    filename = Column(String)
    memo = Column(String)
    project = Column(String)
    extend_existing = True
    
    def __init__(self, spifileprefix, spidate, eventtype, filename, memo, project):  #constructor for new event records
        self.spifileprefix = spifileprefix
        self.spidate = spidate
        self.eventtype = eventtype
        self.filename = filename
        self.memo = memo
        self.project = project
        
    def __repr__(self):
        return "<Event('%s','%s','%s','%s','%s','%s')>" % (self.spifileprefix, self.spidate, self.eventtype, self.filename, self.memo, self.project)
        
class MyModel(Base):  # not used in spiscan at the moment
    __tablename__ = 'models'
    id = Column(Integer, primary_key=True)
    name = Column(Text, unique=True)
    value = Column(Integer)

    def __init__(self, name, value):
        self.name = name
        self.value = value        

Base.metadata.create_all(engine)
