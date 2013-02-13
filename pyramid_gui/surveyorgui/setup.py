import os

from setuptools import setup, find_packages

here = os.path.abspath(os.path.dirname(__file__))
README = open(os.path.join(here, 'README.txt')).read()
CHANGES = open(os.path.join(here, 'CHANGES.txt')).read()

requires = [
    'pyramid',
    'SQLAlchemy',
    'transaction',
    'pyramid_tm',
    'pyramid_debugtoolbar',
    'zope.sqlalchemy',
    'waitress',
    ]

setup(name='surveyorgui',
      version='0.0',
      description='interface to SPIScan Surveyor - a sediment profile imager',
      long_description=README + '\n\n' + CHANGES,
      classifiers=[
        "Programming Language :: Python",
        "Framework :: Pyramid",
        "Topic :: Internet :: WWW/HTTP",
        "Topic :: Internet :: WWW/HTTP :: WSGI :: Application",
        ],
      author='Brian Paavo',
      author_email='paavo@benthicscience.com',
      url='http://www.benthicscience.com/',
      keywords='web wsgi bfg pylons pyramid sediment profile imagery',
      packages=find_packages(),
      include_package_data=True,
      zip_safe=False,
      test_suite='surveyorgui',
      install_requires=requires,
      entry_points="""\
      [paste.app_factory]
      main = surveyorgui:main
      [console_scripts]
      initialize_surveyorgui_db = surveyorgui.scripts.initializedb:main
      """,
      )
