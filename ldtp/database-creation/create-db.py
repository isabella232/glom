#!/usr/bin/python
from ldtp import *
from ldtputils import *

sys.path = ['..'] + sys.path
import common

import os
import shutil
import threading

#class BackendUnavailableError (Exception):
#	def __str__ (self):
#		return 'Backend not available'

# TODO: Avoid these being global variables:
errorMessage = ''
errorHappened = threading.Event()
errorHappened.clear()

def error_cb():
	global errorMessage
	# We already stop at the first error that occurs
	if not errorHappened.isSet():
		# TODO: Read the actual error message from the error
		# dialog window:
		errorMessage = 'Failed to create new database'
		errorHappened.set()

try:
	xml = LdtpDataFileParser(datafilename)
	backend = xml.gettagvalue('backend')[0]
	button_texts = xml.gettagvalue('button_text')

	common.launch_glom()

	# Create a new document from the Small Business Example:
	# The notebook and treeview widgets are accessed via their
	# "accessible name". It can be set in Glade on the Accessibility tab,
	# or in code via widget->get_accessible()->set_name("").
	# It is meant to be a human-readable, translatable string describing
	# the widget. To access the widget in LDTP, all spaces are removed
	# from that string, so "Create New Document" becomes
	# "CreateNewDocument". Each widget type also has an (optional, I
	# believe) prefix, for example notebooks are prefixed by 'ptl' and
	# treeviews are prefixed by 'ttbl'.
	selecttab(common.initial_dialog, 'ptlOpenorcreateDocument', 'Create New Document')
	selectrow(common.initial_dialog, 'ttblCreateNewDocument', 'Small Business Example')

	# Buttons (prefix: 'btn') are addressed via their label text:
	click(common.initial_dialog, 'btnSelect')

	# Create a directory into which to create the new Glom document:
	if not os.path.exists('TestDatabase'):
		os.mkdir('TestDatabase')

	# Wait for the file chooser dialog to appear:
	if waittillguiexist('Creating From Example File') == 0:
		raise LdtpExecutionError('File chooser does not show up')

	# Navigate into the newly created folder:
	doubleclickrow('Creating From Example File', 'tblFiles', 'TestDatabase')

	# Call the new document 'Test':
	settextvalue('Creating From Example File', 'txtName', 'Test');

	# Make sure we use the self-hosted postgresql backend:
	for text in button_texts:
		if objectexist('Creating From Example File', 'rbtn' + text):
			click('Creating From Example File', 'rbtn' + text)
			break
	else:
		raise LdtpExecutionError('Backend ' + backend + ' not supported')

	# Make sure the Glom main window still exists
	if not guiexist(common.main_window):
		raise LdtpExecutionError('The Glom main window does not exist anymore')

	# Be notified when error dialogs pop up during database creation
	onwindowcreate('Warning', error_cb)
	onwindowcreate('Error', error_cb)

	# Acknowledge the dialog:
	click('Creating From Example File', 'btnSave')

	# Wait for the list view to pop up in the main Glom window:
	# Note that the Window title of the Glom Window changes when the file
	# has loaded. If we use wildcards for the Window title (*Glom*) here,
	# then objectexist does not find the notebook_data
	# (ptlListOrDetailsView) widget, even when it has actually appeared.
	# TODO: Maybe we can use setcontext(), to avoid this.
	# TODO: Or maybe this has been fixed in LDTP in the meanwhile,
	# see bug #583021.
	while not guiexist('Glom-SmallBusinessExample') or not objectexist('Glom-SmallBusinessExample', 'ptlListOrDetailsView'):
		if errorHappened.isSet():
			raise LdtpExecutionError(errorMessage)
		# Wait a bit and then try again:
		wait()

	# Everything finished, so check the created database for consistency.
	if(getrowcount(common.main_window, 'tblTableContent') != 9):
		raise LdtpExecutionError("Newly created database does not contain all 8 rows"); # Note there is one placeholder row

	selecttab(common.main_window, 'ptlListOrDetailsView', 'Details')

	# TODO: Check that there is an image present.
	# Accerciser shows the Image Size for the widget which we may use to
	# check this. However, LDTP does not implement this yet.

	# Exit the application. Here, the wildcard *Glom* works again. I
	# suppose this is because the menu already existed when we used the
	# Wildcard for the first time, in contrast to the notebook_data.
	selectmenuitem(common.main_window, 'mnuFile;mnuClose')

	if waittillguinotexist('Glom-SmallBusinessExample') == 0:
		raise LdtpExecutionError('The Glom Window does not disappear after closing the application')

	# Wait until Glom has cleaned up everything, so that removing the 
	# test database is going to work.
	wait(2)

	# Remove the test database again
       	shutil.rmtree('TestDatabase')

except LdtpExecutionError, msg:
	log(msg, 'fail')

	# Remove the created directory also on error, so that the test
	# does not fail because of the database already existing next time
	shutil.rmtree('TestDatabase')

        raise LdtpExecutionError (msg)
        # TODO: Terminate the Glom application
	# os.kill(whatpid?, signal.SIGTERM)
