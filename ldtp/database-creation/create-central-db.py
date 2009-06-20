#!/usr/bin/python
from ldtp import *
from ldtputils import *

sys.path = ['..'] + sys.path
import common

import os
import shutil

try:
	xml = LdtpDataFileParser(datafilename)
	backend = xml.gettagvalue('backend')[0]
	button_texts = xml.gettagvalue('button_text')

	common.launch_glom()

	# Read info for database connection
	(hostname, username, password) = common.read_central_info()

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

	# Wait for the file chooser dialog to appear:
	if waittillguiexist('Creating From Example File') == 0:
		raise LdtpExecutionError('File chooser does not show up')

	# Call the new document 'Test':
	settextvalue('Creating From Example File', 'txtName', 'Test');

	# Select the correct backend
	common.select_backend(backend, button_texts)

	# Acknowledge the dialog:
	click('Creating From Example File', 'btnSave')

	# Wait until it asks to enter the credentials
	if waittillguiexist('Connection Details') == 0:
		raise LdtpExecutionError('Connection details dialog does not show up')

	# Set connection details
	settextvalue('Connection Details', 'txtHost', hostname)
	settextvalue('Connection Details', 'txtUser', username)
	settextvalue('Connection Details', 'txtPassword', password)

	# Remember the database glom is going to create on the central server,
	# so we can delete it again later.
	database = getlabel('Connection Details', 'lblDatabase')

	# Acknowledge the dialog
	click('Connection Details', 'btnConnect')

	# Wait until the database has been created:
	common.wait_for_database_open()

	# Everything finished, so check the created database for consistency.
	common.check_small_business_integrity()

	# Exit the application.
	common.exit_glom()

	# Wait until Glom has cleaned up everything, so that removing the 
	# test database is going to work.
	wait(2)

	# Remove the test Glom file
	os.unlink('Test.glom')

except LdtpExecutionError, msg:
	log(msg, 'fail')

	# Remove the test glom file also on error, so that the test
	# does not fail because of the file already existing next time
	try:
		os.unlink('Test.glom')
	# Ignore if this fails, for example if the file does not exist
	except OSError, err:
		pass

        raise LdtpExecutionError (msg)
        # TODO: Terminate the Glom application
	# os.kill(whatpid?, signal.SIGTERM)
