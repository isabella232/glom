#!/usr/bin/python
from ldtp import *
from ldtputils import *

sys.path = ['..'] + ['.'] + sys.path
import common

import os
import shutil

try:
	# Load data XML from command line when called directly:
	if sys.argv[0].find('ldtprunner') == -1 and len(sys.argv) > 1:
		datafilename = sys.argv[1]

	parser = LdtpDataFileParser(datafilename)
	backend = parser.gettagvalue('backend')

	if len(backend) == 0:
		raise LdtpExecutionError('<backend> tag not set in "' + datafilename + '"')

	backend = backend[0]

	# Create a test database
	common.create_test_database(backend)

	# Launch Glom
	common.launch_glom()

	# Select the Test database file (TestDatabase/Test.glom)
	selectrow(common.initial_dialog, 'ttblOpenExistingDocument', 'Select File')
	click(common.initial_dialog, 'btnSelect')

	filechooser = 'Choose a glom file to open'
	if waittillguiexist(filechooser) == 0:
		raise LdtpExceutionError('The file chooser did not appear.')

	# Descend into the TestDatabase directory:
	doubleclickrow(filechooser, 'tblFiles', 'TestDatabase')

	# Select the .glom file:
	selectrow(filechooser, 'tblFiles', 'Test.glom')

	# Acknowledge the dialog
	click(filechooser, 'btnOpen')
	
	# Enter the connection credentials for the centrally hosted database:
	common.enter_connection_credentials(backend)

	# Wait until the database has been created:
	common.wait_for_database_open()

	# Go in Developer mode:
	selectmenuitem(common.main_window, 'mnuUserLevel;mnuDeveloper')

	# Open the Fields dialog:
	selectmenuitem(common.main_window, 'mnuFields')

	field_definitions = 'Field Definitions'
	if waittillguiexist(field_definitions) == 0:
		raise LdtpExecutionError('The Field Definitions dialog window did not appear.')

	# Add a new field:
	click(field_definitions, 'btnAdd')

	# Give it a name:
	generatekeyevent('test_field<enter>')

	# TODO: Check if an error dialog pops up

	click(field_definitions, 'btnClose')

	# TODO: Check that changing fields and removing fields works as well

	# Exit the application.
	common.exit_glom()

	# Wait a few seconds to give Glom time to close the database connection
	# before attempting to remove it:
	wait(2)

	# Delete the test database
	common.delete_test_database(backend)

except LdtpExecutionError, msg:
	log(msg, 'fail')

	# Remove the created directory also on error, so that the test
	# does not fail because of the database already existing next time
	common.delete_test_database(backend)

	raise LdtpExecutionError (msg)
	# TODO: Terminate the Glom application
	# os.kill(whatpid?, signal.SIGTERM)
