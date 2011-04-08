#!/usr/bin/env python

#
# Copyright (c) 2006-11 Openismus GmbH
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this file; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# The actual .glom file that uses this is at
# http://gitorious.org/debian_repository_analyzer

from gi.repository import Gtk
from gi.repository import Gda

import apt # For apt.Cache.
import apt_pkg # For apt_pkg.GetPkgSourceList
import sys
import os
import difflib

class DebugWindow:
    debug_window = None
    debug_textview = None
    debug_button_close = None

    def __init__(self):
        #print "debug: Creating debug window."
        self.debug_window = Gtk.Window()
        vbox = Gtk.VBox()
        vbox.set_spacing(6)
        vbox.show()
        self.debug_window.add(vbox)

        scrolledwindow = Gtk.ScrolledWindow()
        scrolledwindow.set_policy(Gtk.PolicyType.AUTOMATIC, Gtk.PolicyType.AUTOMATIC)
        scrolledwindow.show()
        vbox.pack_start(scrolledwindow, True, True, 0)

        self.debug_textview = Gtk.TextView()
        self.debug_textview.show()
        scrolledwindow.add(self.debug_textview) #We don't need to use add_with_viewport() with a TextView.

        hbox = Gtk.HBox()
        hbox.show()
        vbox.pack_start(hbox, False, False, 6)

        self.debug_button_close = Gtk.Button(stock=Gtk.STOCK_CLOSE)
        self.debug_button_close.set_sensitive(False) #Only let the user close this window when we are finished.
        self.debug_button_close.show()
        hbox.pack_end(self.debug_button_close, False, False, 6)

        self.debug_button_close.connect("clicked", self.on_debug_button_clicked, None)

        self.debug_window.set_default_size(400, 400) #Make it big enough.
        self.debug_window.set_title(u"Repository Analyzer debug output")

    def append_text(self, text):
        #Add the text to the window:
        textbuffer = self.debug_textview.get_buffer()
        enditer = textbuffer.get_end_iter()
        textbuffer.insert(enditer, text + "\n")

        #Make sure the the last message is always visible:
        enditer = textbuffer.get_end_iter()
        self.debug_textview.scroll_to_iter(enditer, 0, False, 0, 0)

        #Update the UI even during intensive processing:
        self.refresh_ui()

    def refresh_ui(self):
        while Gtk.events_pending():
            Gtk.main_iteration()

    def on_debug_button_clicked(self, *args):
        self.debug_window.destroy()



debugwindow = DebugWindow()

def print_debug(text):

    debugwindow.debug_window.show() #Only show it when we first use it. TODO: How will it be destroyed if the button is never clicked, because it is never shown?
    debugwindow.append_text(text)

    #print(text)

class StandardLicenses(object):

    def __init__(self):

        self.license_dict = {}

        license_name_gpl1 = u"GPL1"
        license_text_gpl1 = u"GNU GENERAL PUBLIC LICENSE\n       Version 1, February 1989"  #We stop the extract here, because there are versions with different FSF addresses.
        self.add_license(license_name_gpl1, license_text_gpl1)

        license_name_gpl2 = u"GPL2"
        license_text_gpl2 = u"GNU GENERAL PUBLIC LICENSE\n		       Version 2, June 1991"  #We stop the extract here, because there are versions with different FSF addresses.
        self.add_license(license_name_gpl2, license_text_gpl2)

        license_name_lgpl2 = u"LGPL2"
        license_text_lgpl2 = u"GNU LIBRARY GENERAL PUBLIC LICENSE\n		       Version 2, June 1991"  #We stop the extract here, because there are versions with different FSF addresses.
        self.add_license(license_name_lgpl2, license_text_lgpl2)

        license_name_lgpl2p1 = u"LGPL2.1"
        license_text_lgpl2p1 = u"GNU LESSER GENERAL PUBLIC LICENSE\n		       Version 2.1, February 1999" #We stop the extract here, because there are versions with different FSF addresses.
        self.add_license(license_name_lgpl2p1, license_text_lgpl2p1)

        license_name_mpl = u"MPL"
        license_text_mpl = u"MOZILLA PUBLIC LICENSE"
        self.add_license(license_name_mpl, license_text_mpl)

        license_name_boost = u"Boost"
        license_text_boost = u"Boost Software License - Version 1.0 - August 17th, 2003"
        self.add_license(license_name_boost, license_text_boost)

        license_name_mit = u"MIT"
        license_text_mit = u"""Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the \"Software\"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions"""
        self.add_license(license_name_mit, license_text_mit)

        license_name_x11 = u"X11" #Seems to be different to MIT. Sometimes mentions Keith Packard explicitly.
        license_text_x11 = u"""Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation""" #Stop here, before an additional explicit name mention, forbidding use of the author's name, usually Keith Packard.
        self.add_license(license_name_x11, license_text_x11)

        license_name_bsd = u"BSD"
        license_text_bsd = u"""Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.""" #Stop here, before mention of a specific organisation name, usually forbidding use of that name.
        self.add_license(license_name_bsd, license_text_bsd)

        license_name_bsd_alt = u"BSD_alternate" #Almost identical - just uses - instead of 1. and 2.
        license_text_bsd_alt = u"""Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

- Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.

- Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.""" #Stop here, before mention of a specific organisation name, usually forbidding use of that name.
        self.add_license(license_name_bsd_alt, license_text_bsd_alt)

    def add_license(self, name, text):
        self.license_dict[name] = self.remove_excess_whitespace(text)


    def remove_excess_whitespace(self, text):
        return u" ".join(text.split()) #remove multiple white spaces and new lines, tabs etc. and replace by a single whitespace

    def get_is_standard_license(self, text):
        #Look for each of the licenses texts (part of them, anyway) in the given text,
        #and return the name of any standard license found.
        for key in self.license_dict.keys():
            standard_license_text = self.license_dict[key]
            pos = self.remove_excess_whitespace(text).find(standard_license_text)
            if((pos != -1) and (pos < 200)): #If the starting text appeared much later, it would be very odd.
                #print_debug( "debug: get_is_standard_license(): found: %s" % key )
                return key

        #print_debug( "debug: get_is_standard_license(): text not identified\n %s" % text )
        return u""

    def get_are_same_standard_license(self, texta, textb, license_name_out):
        standard_name_a = self.get_is_standard_license(texta)
        if(len(standard_name_a) == 0):
            return False

        standard_name_b = self.get_is_standard_license(textb)
        if(standard_name_a == standard_name_b):
            license_name_out = standard_name_a
            return True
        else:
            license_name_out = u""
            return False





#### From James Henstridge's httpcache.py, with ugly changes by me (Murray Cumming:

'''Very simple minded class that can be used to maintain a cache of files
downloaded from web servers.  It is designed to reduce load on web servers,
and draws ideas from feedparser.py.  Strategies include:
    - If a resource has been checked in the last 6 hours, consider it current.
    - support gzip transfer encoding.
    - send If-Modified-Since and If-None-Match headers when validating a
      resource to reduce downloads when the file has not changed.
    - honour Expires headers returned by server.  If no expiry time is
      given, it defaults to 6 hours.
'''

import os
import urllib2
import urlparse
import time
import rfc822
import StringIO
try:
    import gzip
except ImportError:
    gzip = None

try:
    import xml.dom.minidom
except ImportError:
    raise SystemExit, 'Python xml packages are required but could not be found'

class HttpCache:

    def __init__(self):
        self._cache = HttpCache.Cache()





    class Cache:

        class CacheEntry:
            def __init__(self, uri, local, modified, etag, expires=0):
                self.uri = uri
                self.local = local
                self.modified = modified
                self.etag = etag
                self.expires = expires

        cachedir = os.path.join(os.environ['HOME'], '.jhbuild', 'cache')
        # default to a 6 hour expiry time.
        default_age = 6 * 60 * 60

        def __init__(self, cachedir=None):
            if cachedir:
                self.cachedir = cachedir
            if not os.path.exists(self.cachedir):
                os.makedirs(self.cachedir)
            self.entries = {}

        def read_cache(self):
            self.entries = {}
            cindex = os.path.join(self.cachedir, 'index.xml')
            try:
                document = xml.dom.minidom.parse(cindex)
            except:
                return # treat like an empty cache
            if document.documentElement.nodeName != 'cache':
                document.unlink()
                return # doesn't look like a cache

            for node in document.documentElement.childNodes:
                if node.nodeType != node.ELEMENT_NODE: continue
                if node.nodeName != 'entry': continue
                uri = node.getAttribute('uri')
                local = str(node.getAttribute('local'))
                if node.hasAttribute('modified'):
                    modified = node.getAttribute('modified')
                else:
                    modified = None
                if node.hasAttribute('etag'):
                    etag = node.getAttribute('etag')
                else:
                    etag = None
                expires = self._parse_isotime(node.getAttribute('expires'))
                # only add to cache list if file actually exists.
                if os.path.exists(os.path.join(self.cachedir, local)):
                    self.entries[uri] = HttpCache.Cache.CacheEntry(uri, local, modified,
                                                   etag, expires)
            document.unlink()

        def _parse_isotime(self, string):
            if string[-1] != 'Z':
                return time.mktime(time.strptime(string, '%Y-%m-%dT%H:%M:%S'))
            tm = time.strptime(string, '%Y-%m-%dT%H:%M:%SZ')
            return time.mktime(tm[:8] + (0,)) - time.timezone

        def _format_isotime(self, tm):
            return time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime(tm))

        def write_cache(self):
            cindex = os.path.join(self.cachedir, 'index.xml')


            document = xml.dom.minidom.Document()
            document.appendChild(document.createElement('cache'))
            node = document.createTextNode('\n')
            document.documentElement.appendChild(node)
            for uri in self.entries.keys():
                entry = self.entries[uri]
                node = document.createElement('entry')
                node.setAttribute('uri', entry.uri)
                node.setAttribute('local', entry.local)
                if entry.modified:
                    node.setAttribute('modified', entry.modified)
                if entry.etag:
                    node.setAttribute('etag', entry.etag)
                node.setAttribute('expires', self._format_isotime(entry.expires))
                document.documentElement.appendChild(node)

                node = document.createTextNode('\n')
                document.documentElement.appendChild(node)

            document.writexml(open(cindex, 'w'))
            document.unlink()

        def _make_filename(self, uri):
            '''picks a unique name for a new entry in the cache.
            Very simplistic.'''
            # get the basename from the URI
            parts = urlparse.urlparse(uri, allow_fragments=False)
            base = parts[2].split('/')[-1]
            if not base: base = 'index.html'

            is_unique = False
            while not is_unique:
                is_unique = True
                for uri in self.entries.keys():
                    if self.entries[uri].local == base:
                        is_unique = False
                        break
                if not is_unique:
                    base = base + '-'
            return base


        def _parse_date(self, date):
            tm = rfc822.parsedate_tz(date)
            if tm:
                return rfc822.mktime_tz(tm)
            return 0


        def load(self, uri, nonetwork=False):
            '''Downloads the file associated with the URI, and returns a local
            file name for contents.'''
            # pass file URIs straight through -- no need to cache them
            parts = urlparse.urlparse(uri)
            if parts[0] in ('', 'file'):
                return parts[2]

            now = time.time()

            # is the file cached and not expired?
            self.read_cache()
            entry = self.entries.get(uri)
            if entry:
                if (nonetwork or now <= entry.expires):
                    return os.path.join(self.cachedir, entry.local)

            if nonetwork:
                raise RuntimeError('file not in cache, but not allowed '
                               'to check network')

            request = urllib2.Request(uri)
            if gzip:
                request.add_header('Accept-encoding', 'gzip')
            if entry:
                if entry.modified:
                    request.add_header('If-Modified-Since', entry.modified)
                if entry.etag:
                    request.add_header('If-None-Match', entry.etag)

            try:
                response = urllib2.urlopen(request)

                # get data, and gunzip it if it is encoded
                data = response.read()
                ## if gzip and response.headers.get('Content-Encoding', '') == 'gzip':
                ##    try:
                ##        data = gzip.GzipFile(fileobj=StringIO.StringIO(data)).read()
                ##    except:
                ##        data = ''

                expires = response.headers.get('Expires')

                # add new content to cache
                entry = HttpCache.Cache.CacheEntry(uri, self._make_filename(uri),
                                   response.headers.get('Last-Modified'),
                                   response.headers.get('ETag'))
                filename = os.path.join(self.cachedir, entry.local)
                open(filename, 'wb').write(data)
            except urllib2.HTTPError, e:
                if e.code == 304: # not modified. update validated
                    expires = e.hdrs.get('Expires')
                    filename = os.path.join(self.cachedir, entry.local)
                else:
                    raise

            # set expiry date
            entry.expires = self._parse_date(expires)
            if entry.expires <= now: # ignore expiry times that have already passed
                entry.expires = now + self.default_age

            # save cache
            self.entries[uri] = entry
            self.write_cache()
            return filename

    _cache = None
    def load(self, uri, nonetwork=False):
        '''Downloads the file associated with the URI, and returns a local
        file name for contents.'''
        if not self._cache: self._cache = Cache()
        return self._cache.load(uri, nonetwork=nonetwork)







#### From package_data.py:

import apt # For apt.Cache.
import apt_pkg # For apt_pkg.GetPkgSourceList

import sys
import os
import dircache

import urllib2 #Library to do downloads.
import os.path #For split()
import gzip
import tarfile
import StringIO

#We create globals object because Python makes it so difficult to have static class methods.
#(A patch would be welcome if you disagree.) murrayc.
standard_licenses = StandardLicenses()
the_httpcache = HttpCache()

class PackageData:
    def __init__(self, apt_cache, apt_srcrecords, candver):

        # Save the name and version:
        self.name = candver.ParentPkg.Name
        self.version = candver.VerStr

        # Get the description and tarball URI:
        package = apt_cache[self.name]
        self.description = package.description

        #print_debug( "  Name %s" % package.Name )
        self.source_package_name = package.sourcePackageName

        #Get the path of the source tarball, using the apt-pkg API:
        srcrec = apt_srcrecords.Lookup(self.source_package_name)
        if(srcrec == None):
            apt_srcrecords.Restart() #Michael Vogt said this might stop Lookup() from failing sometimes.
            srcrec = apt_srcrecords.Lookup(self.source_package_name) #Try again

        if srcrec:
            # TODO: Is this really the only way to get the path?
            # It's ugly.
            #
            for (the_md5hash, the_size, the_path, the_type) in apt_srcrecords.Files:
                if(the_type == "tar"): #Possible types seems to be "tar", "dsc", and "diff"
                    self.tarball_uri = the_path
                    # TODO: In Ubuntu Edgy, we should be able to get the full URI by doing this,
                    # according to Michael Vogt:
                    # self.tarball_uri = srcrec.Index.ArchiveURI(self.tarball_uri)
                elif(the_type == "diff"):
                    self.diff_uri = the_path
        else:
            print_debug( "  debug: apt_srcrecords.Lookup() failed for package %s. " % self.source_package_name )


        # Note: This is broken when Dir::State::status is not set: It returns huge false, irrelevant dependencies.
        # Get the dependencies of this package:
        # TODO: Get direct dependencies only?
        dependslist = candver.DependsList

        #Look at each dependency:
        self.dependencies = set() #If we don't do this then it seems to use the list from the last instantiation of this class, so the list keeps getting bigger.
        for dep in dependslist.keys():

            # get the list of each dependency object
            for depVerList in dependslist[dep]:

                for z in depVerList:

                    # get all TargetVersions of
                    # the dependency object
                    for tpkg in z.AllTargets():
                        self.dependencies.add(tpkg.ParentPkg.Name)
                        #TODO: This does not always seem to be the same name that we get for the parent package.
                        #       Sometimes there is no package with this exact name.
                        #       Or maybe that's a problem with the repository.

        #print_debug( "  debug: dependencies of package %s: %s" % (self.name, self.dependencies) )


        self.license_text = ""
        license_found = False
        #repository_base_uri = "http://archive.ubuntu.com/ubuntu/" # TODO: Get this from python-apt somehow.
        repository_base_uri = "http://repository.maemo.org/" # Remove this hack when we have ArchiveURI() in python-apt in Ubuntu Edgy.

        if(self.diff_uri):
            full_diff_uri = repository_base_uri + self.diff_uri
            self.diff_uri = full_diff_uri #Store the full URI so we can use it in scripts in the Glom database if we like.

        #Try to get the license text by looking at the source tarball:
        if(self.tarball_uri):

             # Michael Vogt says that full_uri = srcrec.Index.ArchiveURI(tarball_uri) gives the whole thing, but that will only work in Ubuntu Edgy (not Dapper)

            full_uri = repository_base_uri + self.tarball_uri
            self.tarball_uri = full_uri #Store the full URI so we can use it in scripts in the Glom database if we like.

            license_found = self.get_license_from_tarball(full_uri)
            if(license_found == False):
                print_debug( "debug: package: %s, license file not found in tarball: %s" % (self.name, full_uri) )

        #Try to get the license from the .diff instead:
        if(license_found == False and self.diff_uri):

            #print_debug( "  debug: Trying the .diff instead: URI=%s" % self.diff_uri )
            license_found = self.get_license_from_diffgz(self.diff_uri)
            if(license_found == False):
                print_debug( "    debug: package: %s, license file not found in diff: %s" % (self.name, self.diff_uri) )

        self.license_found = license_found


    def get_license_from_file(self, file_object):
        license_found = False

        data = file_object.read()
        if((data == None) or (len(data) == 0)):
            text = u"unknown (empty)"
        else:
            text = self.convert_to_unicode_with_fallbacks(data)
            if(len(text) == 0):
                text = u"unknown (encoding error)"
            else:
                license_found = True

        self.license_text = text

        #Remove any unnecessary leading or trailing whitespace:
        self.license_text = self.license_text.strip()

        print_debug( "  debug: is standard license?: %s" % standard_licenses.get_is_standard_license(self.license_text) )

        return license_found


    def convert_to_unicode_with_fallbacks(self, data, encoding=None):
        try:
            if(encoding == None):
                result = unicode(data)
            else:
                result = unicode(data, encoding)

            return result
        except UnicodeError, ex:
            result = ""
            print_debug( "  debug: encoding error: encoding=%s" % str(encoding) )

        #Decoding failed if we got this far:

        #Fall back to other encodings:
        if(encoding == None):
            possible_encodings = ("utf-8", "ascii", "latin_1", "utf_16", "iso8859_2")
            for enc in possible_encodings:
                result = self.convert_to_unicode_with_fallbacks(data, enc)
                if(len(result) != 0):
                    print_debug( "fallback encoding successful: %s" % enc )
                    return result

            print_debug( "debug: encoding error: All fallback codecs failed." )

        return u""

    def get_license_from_tarball_tarfile(self, the_tarfile):
        license_found = False

        try:
            for tarinfo in the_tarfile:
                if(tarinfo.isreg()):
                    filepath = tarinfo.name
                    (directory, filename) = os.path.split(filepath)

                    # Find a license file and read its contents:

                    previously_found_pathdir = ""
                    if(filename in self.possible_license_files): #TODO: Make this case-insensitive somehow?
                        print_debug( "debug: package: %s, license file found in tarball. Attempting read: %s" % (self.name, filename) )

                        fileobject = the_tarfile.extractfile(tarinfo)

                        previous_found_license = self.license_text
                        is_higher_than_previous = ( len(previously_found_pathdir) == 0 or (len(directory) < len(previously_found_pathdir)) )

                        license_found_this = self.get_license_from_file(fileobject)
                        if(license_found_this):
                            license_found = True
                            #print_debug( "  debug: read successful." )

                            if(is_higher_than_previous):
                                previously_found_pathdir = directory
                            else:
                                # Restore the one we found earlier
                                # (it was at a higher directory level, so it's more likely to be what we want.
                                self.license_text = previous_found_license
                                #print_debug( "  debug1: restoring previous found text" )
                        else:
                            print_debug( "  debug: read failed. Error, if any: %s" %  self.license_text  )
                            #Restore the last text if any, including the last "unknown" error text,
                            #so we prefer previous found license text, or errors from higher-up files:
                            if( license_found or ((is_higher_than_previous == False) and len(previous_found_license)) ):
                                self.license_text = previous_found_license
                                #print_debug( "  debug2: restoring previous found text" )



        except tarfile.TarError, ex:
            print_debug( "Error while extracting tarball: %s" % str(ex) )

        #print_debug( "debug: File found in tarball?: %s" % str(license_found) )

        #If no license was found, try looking for nested tarballs.
        #This seems to happen quite often - the tarball contains a tarball.
        if(license_found == False):
            try:
                for tarinfo in the_tarfile:
                    if(tarinfo.isreg()):
                        filepath = tarinfo.name
                        (directory, filename) = os.path.split(filepath)

                        # Find a .tar.gz file inside the tarball:
                        file_extensions = (".tar.gz", ".tgz", ".bz2")

                        for ext in file_extensions:
                            if(filepath.endswith(ext)):
                                #print_debug( "debug: package: %s, examining nested tarball: %s" % (self.name, filename) )

                                file_object = the_tarfile.extractfile(tarinfo)

                                try:
                                    #This doesn't work - we get this error:
                                    # " 'ExFileObject' object has no attribute 'rfind' "
                                    #
                                    #tar = tarfile.open(file_object)

                                    #So we do this instead:
                                    gzipped_data = file_object.read() #Or bzipped.
                                    filename_local = "/tmp/repository_analyzer_nested_temp.tar" + ext
                                    localfile = open(filename_local, 'wb')
                                    localfile.write(gzipped_data)
                                    localfile.close()

                                except tarfile.TarError, ex:
                                    #raise
                                    print_debug( "tarfile.open() of nested tarball failed: for package: %s, for file: %s: %s" % (self.name, filename, ex) )
                                    self.license_text = "unknown (error extracting nested tarball)"
                                    license_found = False

                                try:
                                    license_found = self.get_license_from_tarball(filename_local)
                                except  tarfile.TarError, ex:
                                    #raise
                                    print_debug( "tarfile.open() of nested tarball failed: for package: %s, for file: %s: %s" % (self.name, filename, ex) )
                                    self.license_text = "unknown (error opening extracted nested tarball)"
                                    license_found = False

            except tarfile.TarError, ex:
                print_debug( "Error while extracting tarball: %s" % str(ex) )

        return license_found

    # This seems to be the only way to delete a directory without getting exceptions about it not being empty:
    # I took this from some forum somewhere.
    # I am surprised that there isn't an easier way. murrayc.
    def recursive_delete(self, dirname):
        #print_debug( "debug: recursive_delete(%s)" % dirname )
        dircache.reset() #Seems to be necessary. Otherwise it sometimes reports incorrect filenames.
        files = dircache.listdir(dirname)
        for thefile in files:
            #print_debug( "  debug: thefile=%s" % thefile )
            path = os.path.join (dirname, thefile)
            if os.path.isdir(path):
                self.recursive_delete(path)
            else:
                #print_debug( 'Removing file: "%s"' % path )
                try:
                    retval = os.unlink(path)
                except OSError, ex:
                    print_debug( "  debug: exception in recursive_delete(self, %s), while unlink path %s" % (dirname, path) )
                    raise

        #print_debug( 'Removing directory:', dirname )
        os.rmdir(dirname)

    def get_license_from_diffgz_gzipfile(self, the_gzipfile):
        license_found = False

        # A .gz file can contain only one file (A .tar.gz can contain multiple files.)
        try:
            data = the_gzipfile.read()
            filename = "temp.diff"
            filename_local_parent = "/tmp/repository_diff/"

            #Remove any previous temporary files there, and recreate the directory:
            try:
                if(os.path.isdir(filename_local_parent)):
                    self.recursive_delete(filename_local_parent)
            except IOError, e:
                print_debug( "Error during removedirs: %s" % str(e) )

            safe_mkdir(filename_local_parent)

            filename_local = filename_local_parent + filename
            localfile = open(filename_local, 'wb')
            localfile.write(data)
            localfile.close()

            #Expand the directory:
            # -f makes it ignore warnings, so we only see new files.
            # -s makes it silent (apart from errors).
            # &> /dev/null hides errors from stdout, because strange encodings can confuse the terminal, making it show nonsense.
            command = "cd " + filename_local_parent +"; patch -p0 -f -s < " + filename_local + " &> /dev/null"
            #print_debug( "debug: command=%s" % command )
            os.system(command)


            #Examine the files that were created by patch:
            for root, dirs, files in os.walk(filename_local_parent):
                for the_file in files:
                    if(the_file in self.possible_license_files):
                        the_file_fullpath = os.path.join(root, the_file)
                        print_debug( "debug: license file found in diff: %s" % the_file_fullpath )

                        try:
                            fileobject = open(the_file_fullpath)
                            if(fileobject):
                                license_found = self.get_license_from_file(fileobject)
                        except IOError, e:
                            print_debug( "Could not open file: %s" % the_file_fullpath )
                            print_debug( "  Exception: %s" % str(e) )

        except IOError, e:
            print_debug( "Could not read gzipfile %s" % str(e) )

        return license_found



    def open_tarball(self, uri):
        if (uri == ""):
            self.license_text = "unknown (empty source tarball file URI)"
            return

        #print_debug( "debug: open_tarball(): uri=%s" % uri )

        # Get the tar.gz tarball data:
        # Use the httpcache to avoid repeated downloads of the same tarball:
        try:
            local_filename = the_httpcache.load(uri)
        except IOError, e:
            print_debug( "Could not download %s: %s" % (uri, str(e)) )
            self.license_text = "unknown (error downloading source tarball file)"
            return

        #print_debug( "debug: local_filename=%s" % local_filename )

        #Non-httpcache version:
        ## request = urllib2.Request(uri)
        ## request.add_header('Accept-encoding', 'gzip')
        ##
        ## try:
        ##     response = urllib2.urlopen(request)

        ##     # get data, and gunzip it if it is encoded
        ##     gzipped_data = response.read()
        ##     ##print_debug( "data read" )
        ##     ##print_debug( "response.headers=%s" % response.headers )
        ##     ##if 1: #response.headers.get('Content-Encoding', '') == 'gzip':
        ##     ##   try:
        ##     ##       data = gzip.GzipFile(fileobj=StringIO.StringIO(gzipped_data)).read()
        ##     ##    except:
        ##     ##       print_debug( "GzipFile failed." )
        ##     ##        data = ''
        ##
        ## except urllib2.HTTPError, e:
        ##      print_debug( "error: Could not open url: %s" % uri )
        ##         raise

        # Save the gzipped data to a local file:
        # TODO: Maybe tarfile.open() can open the URI directly, or can directly access the data?
        # The documentation hints at this, but I can't get it to work. murrayc.
        #filename_local = "/tmp/repository_analyzer_temp.tar.gz"
        #open(filename_local, 'wb').write(gzipped_data)

        # Open the local file as a tarfile,
        # so we can examine its contents:

        # Try opening the file as a .gz or .bz2:
        # Actually, they always seem to be .tar.gz files.
        could_open_file = False
        for open_mode in ("r:gz", "r:bz2"):
            #print_debug( "trying mode=%s for local_filename=%s" % (open_mode, local_filename) )
            try:
                tar = tarfile.open(local_filename, open_mode)

                if(tar):
                    return tar
            except Exception, ex:
                print_debug( "debug: tarfile open failed with mode=%s for filename=%s" % (open_mode, local_filename) )
                print_debug( "  ex=%s" % ex )
                continue

        print_debug( "package=%s Could not open source URI as .gz or .bz2: %s" % (self.name, local_filename) )
        return

    def open_gzipfile(self, uri):
        if (uri == ""):
            return

        # Get the tar.gz tarball data:
        # Use the httpcache to avoid repeated downloads of the same tarball:
        try:
            local_filename = the_httpcache.load(uri)
        except Exception, e:
            print_debug( "Could not download %s: %s" % (uri, str(e)) )
            self.license_text = "unknown (error downloading tarball)"
            return

        # Open the local file as a gzipfile,
        # so we can examine its contents:

        # Try opening the file as a .gz or .bz2:
        # Actually, they always seem to be .tar.gz files.
        try:
            gzipped_file = gzip.open(local_filename)
            if(gzipped_file):
                return gzipped_file
        except Exception, ex:
            print_debug( "debug: gzipfile open failed for filename=%s" % local_filename )
            print_debug( "  ex=%s" % ex )

        print_debug( "package=%s Could not open diff.gz URI as .gz: %s" % (self.name, local_filename) )
        return




    def get_license_from_tarball(self, uri):
        # print_debug( "debug: Trying tarball: %s" % uri )
        # Based on code in jhbuild's httpcache.py:

        self.license_text = "unknown" #The default

        tar = self.open_tarball(uri)
        if(tar == None):
            if(len(self.license_text) == 0):
                self.license_text = u"unknown (error opening source tarball file)"
            return False

        license_found = self.get_license_from_tarball_tarfile(tar)
        tar.close()

        return license_found


    def get_license_from_diffgz(self, uri):
        # Based on code in jhbuild's httpcache.py:

        self.license_text = u"unknown" #The default

        gzipped_file = self.open_gzipfile(uri)
        if(gzipped_file == None):
            self.license_text = u"unknown (error opening diff.gz file)"
            print_debug( "debug: open_tarball(%s) failed." % uri )
            return False

        license_found = self.get_license_from_diffgz_gzipfile(gzipped_file)
        gzipped_file.close()

        if(license_found == False):
            if(len(self.license_text) == 0): #TODO: Is there an empty() for efficiency?
                self.license_text = u"unknown (error reading from source tarball file)"

        return license_found



    name = ""
    source_package_name = ""
    tarball_uri = ""
    diff_uri = ""
    version = ""
    dependencies = set()
    license_found = False
    license_text = ""
    license_text_simplified = False # Whether we extracted a common part of the license, ignoring a unique part.

    # TODO: Be more clever when more than one of these files is present, or at least prioritize them:
    # In some cases there are multiple licenses because different executables in the tarball are
    # under different licenses. For instance, flac. In that case, a human must decide.
    possible_license_files = ("COPYING", "COPYING.LIB", "LICENSE", "LICENCE", "license", "licence", "COPYING.Xiph", "LEGAL.NOTICE", "COPYRIGHT", "COPYRIGHT.TXT", "license.terms", "licence.terms", "Copying", "copying", "copying.lib", "copyright", "LICENSE_1_0.txt", "copyright.in")



#### From safe_mkdir.py:

# I took this from some forum somewhere.
# I am surprised that there isn't an easier way. murrayc.

def safe_mkdir(newdir):
    """works the way a good mkdir should :)
        - already exists, silently complete
        - regular file in the way, raise an exception
        - parent directory(ies) does not exist, make them as well
    """
    if os.path.isdir(newdir):
        pass
    elif os.path.isfile(newdir):
        raise OSError("a file with the same name as the desired " \
                      "dir, '%s', already exists." % newdir)
    else:
        head, tail = os.path.split(newdir)
        if head and not os.path.isdir(head):
            safe_mkdir(head)
        #print_debug( "_mkdir %s" % repr(newdir) )
        if tail:
            os.mkdir(newdir)



#### From py:

def escape_text_for_sql(text):
    # Note that this probably requires Glom 1.0.5, because previous versions can't handle newlines in text fields for example_rows.
    # Use this as a workaround when using Glom < 1.0.5:
    # placeholder_newline = "NEWLINE"

    text = text.replace("\n", "\\n")

    #Workaround for the postgres errors about unterminated quoted strings:
    #text = text.replace("\'", "XsinglequoteX")
    #text = text.replace("`", "XbackquoteX")
    #text = text.replace("\"", "XdoublequoteX")
    #text = text.replace(",", "XcommaX")

    #Escape other characters that must be escaped for postgres:
    #Single quotes must be quoted as '', but backquotes and double quotes must not.
    #Not that this text will be further quoted (for XML) when it is written as XML.
    text = text.replace(u"\'", u"\'\'")
    text = text.replace(u";",u"\\073")
    return text

def boolean_for_sql(val):
    if(val):
        return "TRUE"
    else:
        return "FALSE"

def quote_for_sql(text):
    return "'" + text + "'"



def get_licenses_map(packages_dict):
    #Build a map of licenses, so we can list each unique license only once:
    licenses_map = {}

    for package_name in packages_dict.keys():
        package_data = packages_dict[package_name]
        if(package_data.license_text and len(package_data.license_text)): #TODO: Is there an empty() method to save time?
            # Create a list for this key, if necessary:
            if(licenses_map.has_key(package_data.license_text) == False):
                licenses_map[package_data.license_text] = [] # An empty list.

            # Associate the package name with this license text,
            # along with any other package names already associated with the same license text:
            licenses_map[package_data.license_text].append(package_data.name)

    return licenses_map

def get_licenses_map_with_matching(out_licenses_map, packages_dict):
    #For quick debugging of the previous steps:
    #return False

    #Examine each license and try to extract common parts,
    #ignoring small differences such as "XYZ is license under the GPL. Here is the license."

    match_found = False
    dict_matching_packages = {}
    for license_text in out_licenses_map.keys():

        do_exact_match = False #Otherwise, do an approximate match.

        #Don't roughly compare the special "unknown" error license texts.
        #Compare these exactly instead.
        package_name_list = out_licenses_map[license_text]
        package_name_first = package_name_list[0]
        if(packages_dict[package_name_first].license_found == False):
            do_exact_match = True

        for other_license_text in out_licenses_map.keys():

            debugwindow.refresh_ui() #Make sure it still reacts.

            #Don't compare to itself:
            if(other_license_text == license_text):
                break

            other_do_exact_match = do_exact_match #Otherwise, do an approximate match.

            #Don't roughly compare the special "unknown" error license texts.
            #Compare these exactly instead.
            if(other_do_exact_match == False):
                package_name_list = out_licenses_map[other_license_text]
                package_name_first = package_name_list[0]
                if(packages_dict[package_name_first].license_found == False):
                    other_do_exact_match = True

            this_match_found = False
            common_text = ""
            license_name = ""

            if(other_do_exact_match):
                #Exact comparison:
                if(license_text == other_license_text):
                    this_match_found = True
                    common_text = license_text
            else:

                #Standard-licenses comparison:
                license_name = ""
                if(standard_licenses.get_are_same_standard_license(license_text, other_license_text, license_name)):
                    this_match_found = True
                    common_text = license_text #TODO: Use the standard text somehow?.
                    #print_debug( "  debug: standard license texts found." )
                else:
                    #Rough comparison:
                    matcher = difflib.SequenceMatcher(None, license_text, other_license_text)
                    if(matcher.ratio() >= 0.95): #quick_ratio() is significantly faster than ratio().
                        this_match_found = True
                        common_text = ""
                        for block in matcher.get_matching_blocks():
                            (start_a, start_b, count) = block
                            common_text += license_text[start_a:(start_a + count)]

            if(this_match_found):
                match_found = True
                print_debug( "match found for packages %s and packages %s" % (out_licenses_map[license_text], out_licenses_map[other_license_text]) )
                #print_debug( "match found: text=%s" % (common_text) )

                for package_name in out_licenses_map[license_text]:
                    dict_matching_packages[package_name] = common_text

                for package_name in out_licenses_map[other_license_text]:
                    dict_matching_packages[package_name] = common_text


    #Use the matches that were found:
    for package_name in dict_matching_packages.keys():
        license_text_common = dict_matching_packages[package_name]

        #Store the simplified license text instaed of the original:
        packages_dict[package_name].license_text = license_text_common
        packages_dict[package_name].license_text_simplified = True #Mark that we did this.

    #Change the contents of the output variable:
    new_licenses_map = get_licenses_map(packages_dict)
    out_licenses_map.clear()
    out_licenses_map.update(new_licenses_map)

    return match_found


def get_package_data_list(out_licenses_map):

    #Start with the default config, probably from /etc/apt/sources.list:
    apt_pkg.InitConfig()
    apt_pkg.InitSystem()

    #Use a temporary config:
    config_dir_cache_archives = "./tmp_apt_archives"
    config_dir_state = "./tmp_apt_varlibapt"  #usually /var/lib/apt
    config_dir_state_lists =  config_dir_state + "/lists"
    config_dir_state_status = "./tmp_apt_varlibdpkg" #usually /var/lib/dpkg/
    config_file_state_status = "./tmp_apt_varlibdpkg/status" #usually /var/lib/dpkg/status

    # Create the temporary directories, because python-apt does not seem to
    # create them automatically, and throws exceptions.
    safe_mkdir(config_dir_cache_archives)
    safe_mkdir(config_dir_cache_archives + "/partial")
    safe_mkdir(config_dir_state)
    safe_mkdir(config_dir_state_lists)
    safe_mkdir(config_dir_state_lists + "/partial")
    safe_mkdir(config_dir_state_status)
    tempfile = open(config_file_state_status, 'w') #Touch the file so that it exists, so that libapt does not complain.
    tempfile.close()

    #Write a temporary sources.list file, for python-apt to use:
    #TODO: Get this from a separate python script, when glom supports libraries of reusable python scripts.
    #Or maybe put it in a database table.
    temp_sourceslist_path = "/tmp/repository_analyzer_sources.list"
    output = open(temp_sourceslist_path, 'w')
    sources_list = "deb http://repository.maemo.org/ mistral free non-free\ndeb-src http://repository.maemo.org/ mistral free non-free"

    output.write(sources_list)
    output.close()

    # Tell apt what sources.list to use.
    # (See "apt-config dump" for the list of all config keys.)
    apt_pkg.Config.Set("Dir::Etc::sourcelist", temp_sourceslist_path)
    apt_pkg.Config.Set("Dir::Cache::archives", config_dir_cache_archives)
    apt_pkg.Config.Set("Dir::State", config_dir_state)
    apt_pkg.Config.Set("Dir::State::Lists", config_dir_state_lists)
    apt_pkg.Config.Set("Dir::State::status", config_file_state_status) #If we don't set this then we will pick up packages from the local system, from the default status file.


    #Initialize the return value and the output parameter:
    packages_dict = {} # A dict.
    out_licenses_map.clear() #A dict of license text (big) to a list of package names. Note that = {} would assign a new dict, so the caller wouldn't get the contents.

    # Get the list of all packages in the repository.
    # This depends on your /etc/apt/sources.list
    cache = apt.Cache()

    print_debug( "Updating apt cache ..." )
    cache.update() #We could pass a progress object to show feedback.
    cache.open(apt.progress.OpProgress()) #Shouldn't python-apt's cache.update() do this? Michael Vogt thinks so.
    print_debug( ".. finished updating apt cache\n" )

    srcrecords = apt_pkg.GetPkgSrcRecords()
    srcrecords.Restart()

    print_debug( "Number of packages in cache: %d" % len(cache) )

    # Look at each package:
    i = 0
    for pkg in cache:

        candver = cache._depcache.GetCandidateVer(pkg._pkg)

        # Ignore packages with no candidate version:
        if candver == None:
            continue

        package_data = PackageData(cache, srcrecords, candver) # Retrieves the data from apt.
        if(package_data.tarball_uri):
            packages_dict[package_data.name] = package_data
            print_debug( "Processed package %d: %s" % (i, package_data.name) )
        #else:
            # TODO: The if(package_data.tarball_uri) is an old workaround, which it doesn't hurt to keep.
            # python-apt seems to list files not in the specified sources.list, if Dir::State::status is not set.
            # and we detect these because there is no source url for them.
            #print_debug( "Abandoned package %d: %s" % (i, package_data.name) )

        i += 1

    print_debug( "Number of packages used: %d" % len(packages_dict) )

    #for package_data in packages_dict:
    #   print_debug( package_data.name )

    out_licenses_map_temp = get_licenses_map(packages_dict)

    previous_licenses_count = len(out_licenses_map_temp.keys())
    initial_previous_licenses_count = previous_licenses_count
    print_debug( "Number of unique license texts found: %d" % previous_licenses_count )
    print_debug( "Searching for similar license texts (this can take a long time) ..." )

    #Note that this changes packages_dict and out_licenses_map_temp:
    matches_found = True
    while(matches_found):
        matches_found = get_licenses_map_with_matching(out_licenses_map_temp, packages_dict)
        if(matches_found):
            licenses_count = len(out_licenses_map_temp.keys())
            print_debug( "... Matching again after reducing %d unique licenses to %d ... " % (previous_licenses_count, licenses_count) )
            previous_licenses_count = licenses_count

    print_debug( "... Finished search for similar license texts: Reduced initial %d licenses to %d." % (initial_previous_licenses_count, previous_licenses_count) )

    #Fill the output parameter:
    out_licenses_map.clear()
    out_licenses_map.update(out_licenses_map_temp)

    return packages_dict


def debug_create_connection_record():
    #For debugging, outside of Glom:
    data_source_name = "datasource_glomtest"

    data_source = Gda.config_find_data_source(data_source_name)
    if not data_source:
        print_debug( "debug: Creating the DataSource, because it does not exist yet." )

        # Create it if it does not exist already:
        data_source = Gda.DataSourceInfo()
        data_source.name = data_source_name
        data_source.username = "murrayc"
        data_source.password = "luftballons"
        data_source.description = "Test."
        data_source.provider = "PostgreSQL"
        # You must specify a database when using PostgreSQL, even when you want to create a database.
        # template1 always exists.
        # data_source.cnc_string = "DATABASE=template1"
        data_source.cnc_string = "DATABASE=glom_repositoryanalyzer28162;HOST=localhost"

        # TODO: Add save_data_source(data_source_info)
        Gda.config_save_data_source(data_source.name, data_source.provider, data_source.cnc_string, data_source.description, data_source.username, data_source.password)

    cnc_string = "HOST=localhost;PORT=5434;DB_NAME=glom_example_smallbusiness_v2"
    auth_string = "USERNAME=glom_default_developer_user;PASSWORD=glom_default_developer_password"
    gda_connection = Gda.Connection.open_from_string("PostgreSQL", cnc_string, auth_string)


    class TestRecord:
        connection = None

    record = TestRecord()
    record.connection = gda_connection
    return record


def execute_sql_non_select_query(query_text):
    #We use encode() here because, when running inside Glom, gda.Command() somehow expects an ascii string and tries to convert the unicode string to ascii, causing exceptions because the conversion does not default to 'replace'.
    #TODO: Find out why it acts differently inside Glom. This is not a problem when running normally as a standalone script.
    command = query_text.encode('ascii', 'replace')
    return record.connection.execute_non_select_command(command)

def execute_sql_select_query(query_text):
    #We use encode() here because, when running inside Glom, gda.Command() somehow expects an ascii string and tries to convert the unicode string to ascii, causing exceptions because the conversion does not default to 'replace'.
    #TODO: Find out why it acts differently inside Glom. This is not a problem when running normally as a standalone script.
    command = query_text.encode('ascii', 'replace')
    return record.connection.execute_select_command(command)


def is_first_scan():
    query = "SELECT license_id FROM licenses"
    datamodel = execute_sql_select_query(query)
    if(datamodel and (datamodel.get_n_rows() > 0)):
        return False
    else:
        return True

def main():

    if(is_first_scan() == False):
        dlg = Gtk.MessageDialog(None, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, u"Scan Already Done")
        dlg.format_secondary_text("License records already exist, suggesting that a previous scan has already been done.")
        dlg.run()
        dlg.destroy()
        return

    #For debugging, outside of Glom:
    #record = debug_create_connection_record()



    # Rows of SQL-formatted, comma-separated (fields), newline-separated (rows) example data,
    # as Glom expects to find in .glom files:
    #rows_packages = ""
    #rows_package_dependencies = ""
    #rows_licenses = ""

    licenses_map = {} # a new dict.
    packages_dict = get_package_data_list(licenses_map)


    empty_text = quote_for_sql("")

    #Look at each key (the license texts):
    licenses_map_ids = {} # a new dict, of text to ids.
    license_id = 0
    for license_text in licenses_map.keys():

        licenses_map_ids[license_text] = license_id #So we can get the license ID later.

        license_description = standard_licenses.get_is_standard_license(license_text) #Returns "" if it is not recognised as a standard license.
        #If the license text is short (usually an error),
        #use it for the description to make things clearer:
        if((len(license_description) == 0) and len(license_text) < 50):
            license_description = license_text

        # This will look like this:
        # 'id','description',opensource,'license_text',false,false,false,'creditrequired'
        license_row = u"%d,%s,false,%s,false,false,false,%s" % (license_id, quote_for_sql(license_description), quote_for_sql(escape_text_for_sql(license_text)),  empty_text)
        license_id += 1

        #Add the row to the database:
        query = u"INSERT INTO \"licenses\" (\"license_id\",\"description\",\"open_source\",\"license_text\",\"dynamic_linking_requires_source\",\"static_linking_requires_source\",\"modifications_must_be_released\",\"credit_required\") VALUES (%s)" % license_row

        #dlg = Gtk.MessageDialog(None, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK)
        #dlg.format_secondary_text(query)
        #ret = dlg.run()
        #dlg.destroy()

        execute_sql_non_select_query(query)

        #rows_licenses += license_row + placeholder_newline #Use a placeholder that we can later convert to an escaped newline, because minidom doesn't do this for us, though it escapes other things.

    print_debug( "count of licenses=%d" % license_id )

    #Look at each package:
    dependency_id = 0
    for package_name in packages_dict.keys():
        #print_debug( "used: ", package_data.name )


        license_id = "NULL" #empty integer value.

        package_data = packages_dict[package_name]
        if(package_data.license_text):
            try:
                license_id = licenses_map_ids[package_data.license_text]
            except:
                print_debug( "Error while determining license_id." )

        # This will look like this:
        # 'name','comments','description','license_id',,'version','parent_package','tarball_uri'
        package_row = u"%s,%s,%s,%s,%s,%s,%s,%s,%s" % ( quote_for_sql(package_data.name), empty_text, quote_for_sql(escape_text_for_sql(package_data.description)), license_id, quote_for_sql(package_data.version), quote_for_sql(package_data.source_package_name), quote_for_sql(package_data.tarball_uri), quote_for_sql(package_data.diff_uri), boolean_for_sql(package_data.license_text_simplified) )

        if(package_row):
            #Add the row to the database:
            query = u"INSERT INTO \"packages\" (\"name\",\"comments\",\"description\",\"license_id\",\"version\",\"parent_package\",\"tarball_uri\",\"diff_uri\",\"licensed_simplified\") VALUES (%s)" % package_row
            execute_sql_non_select_query(query)


            #rows_packages += package_row + placeholder_newline #Use a placeholder that we can later convert to an escaped newline, because minidom doesn't do this for us, though it escapes other things.

        # Dependencies:

        if(package_data.dependencies):
            for dependency in package_data.dependencies:

                # This will look like this:
                # 'dependency_id', 'dependency_name', 'package_name',
                dependency_row = u"%d, %s, %s" % (dependency_id, quote_for_sql(dependency), quote_for_sql(package_data.name))
                dependency_id += 1
                #rows_package_dependencies += dependency_row + placeholder_newline

                #Add the row to the database:
                query = u"INSERT INTO \"package_dependencies\" (\"package_dependencies_id\",\"package_name\",\"parent_package_name\") VALUES (%s)" % dependency_row
                execute_sql_non_select_query(query)


    debugwindow.debug_button_close.set_sensitive(True) #Let the user close the window. Don't close it automatically, so that they can read it and close when ready.

if __name__ == "__main__":
    main()
