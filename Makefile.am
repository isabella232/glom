## top-level Makefile.am

#Build in these directories:
SUBDIRS = po glom docs macros


#Gnome install information:
GnomeApplicationsdir = $(datadir)/gnome/apps/Applications
GnomeApplications_DATA = glom.desktop

# Install the glom.xml MIME-type registration file, so that the files can be recognised as the correct MIME-type:
MimeDatabasedir = $(datadir)/mime/packages
MimeDatabase_DATA = glom.xml

# Install the application registration, so that the system knows that this application can open files of this MIME-type:
appregdir = $(datadir)/application-registry
appreg_DATA = glom.applications


EXTRA_DIST = $(GnomeApplications_DATA) $(MimeDatabase_DATA)
