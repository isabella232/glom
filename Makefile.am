## top-level Makefile.am

#Build in these directories:
SUBDIRS = po glom docs macros


#Gnome install information:
GnomeApplicationsdir = $(datadir)/gnome/apps/Applications
GnomeApplications_DATA = glom.desktop

# This is the old MIME-type system.
# GNOME 2.6 seems to require that we register with both the old and the new MIME-type systems.
mimeinfodir = $(datadir)/mime-info
mimeinfo_DATA = glom.keys glom.mime

# Install the glom.xml MIME-type registration file, so that the files can be recognised as the $
# correct MIME-type:
MimeDatabasedir = $(datadir)/mime/packages
MimeDatabase_DATA = glom.xml

# Install the application registration, so that the system knows that this application can open files of this MIME-type:
appregdir = $(datadir)/application-registry
appreg_DATA = glom.applications

install-data-hook: 
	-update-mime-database "$(DESTDIR)${datadir}/mime"

EXTRA_DIST = $(GnomeApplications_DATA) $(MimeDatabase_DATA) $(appreg_DATA) $(mimeinfo_DATA)


