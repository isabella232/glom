## top-level Makefile.am

#Build in these directories:
SUBDIRS = po glom docs macros


#Gnome install information:
GnomeApplicationsdir = $(datadir)/gnome/apps/Applications
GnomeApplications_DATA = glom.desktop

#Install the glom.xml MIME-type registration file:
MimeDatabasedir = $(datadir)/mime/packages
MimeDatabase_DATA = glom.xml


EXTRA_DIST = $(GnomeApplications_DATA) $(MimeDatabase_DATA)
