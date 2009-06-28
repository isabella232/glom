#!/usr/bin/python
from ldtp import *
from ldtputils import *

sys.path = ['..'] + ['.'] + sys.path
import common

import os
import shutil

import xml.parsers.expat
import gda

def delete_database(backend):
	if backend == 'PostgresCentral':
		# Read the glom file to find out the database name and port:
		xml_info = []

		def start_element(name, attrs):
			if name == 'connection':
				xml_info.append(attrs['database'])
				xml_info.append(attrs['port'])

		file = open('TestDatabase/Test.glom', 'r')
		content = file.read(1024)

		parser = xml.parsers.expat.ParserCreate()
		parser.StartElementHandler = start_element
		parser.Parse(content)

		if len(xml_info) < 2:
			raise LdtpExecutionError('Glom file does not contain port or database of centrally hosted database')

		(database, port) = (xml_info[0], xml_info[1])
		(hostname, username, password) = common.read_central_info()

		# Remove the database from the central PostgreSQL server:
		op = gda.gda_prepare_drop_database('PostgreSQL', database)
		op.set_value_at('/SERVER_CNX_P/HOST', hostname)
		op.set_value_at('/SERVER_CNX_P/PORT', port)
		op.set_value_at('/SERVER_CNX_P/ADM_LOGIN', username)
		op.set_value_at('/SERVER_CNX_P/ADM_PASSWORD', password)
		gda.gda_perform_drop_database('PostgreSQL', op)

	try:
	       	shutil.rmtree('TestDatabase')
	except OSError:
		pass

try:
	# Load data XML from command line when called directly:
	if sys.argv[0].find('ldtprunner') == -1 and len(sys.argv) > 1:
		datafilename = sys.argv[1]

	parser = LdtpDataFileParser(datafilename)
	backend = parser.gettagvalue('backend')
	example = parser.gettagvalue('example')

	if len(backend) == 0 or len(example) == 0:
		raise LdtpExecutionError('<backend> or <example> tag not set in "' + datafilename + '"')

	backend = backend[0]
	example = int(example[0])

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
	if example:
		selectrow(common.initial_dialog, 'ttblCreateNewDocument', 'Small Business Example')
	else:
		selectrow(common.initial_dialog, 'ttblCreateNewDocument', 'New Empty Document')

	# Buttons (prefix: 'btn') are addressed via their label text:
	click(common.initial_dialog, 'btnSelect')

	# Create a directory into which to create the new Glom document:
	if not os.path.exists('TestDatabase'):
		os.mkdir('TestDatabase')

	# The file chooser dialog has a different title depending on whether
	# we create an empty document, or an example document.
	creation_dialog = ''
	if example:
		creation_dialog = 'Creating From Example File'
	else:
		creation_dialog = 'Save Document'

	# Wait for the file chooser dialog to appear:
	if waittillguiexist(creation_dialog) == 0:
		raise LdtpExecutionError('The file chooser dialog does not appear.')

	# Navigate into the newly created folder:
	doubleclickrow(creation_dialog, 'tblFiles', 'TestDatabase')

	# Call the new document 'Test', and save it as 'Test.glom':
	settextvalue(creation_dialog, 'txtName', 'Test');
	settextvalue(creation_dialog, 'txtTitle', 'Test');

	# Make sure we use the correct backend:
	common.select_backend(creation_dialog, backend)

	# Acknowledge the dialog:
	click(creation_dialog, 'btnSave')

	# Enter the connection credentials for the centrally hosted database:
	common.enter_connection_credentials(backend)

	if not example:
		# Wait for the Tables dialog to appear
		if waittillguiexist('Tables') == 0:
			raise LdtpExecutionError('Tables dialog does not appear')

		# Create an initial table
		click('Tables', 'Add')
		# This does not seem to work here, I'm not sure why.
		# getcellvalue works, though:
		#setcellvalue('Tables', 'tblTables', 0, 0, 'TestTable')
		# Use this as a workaround:
		generatekeyevent('test_table<enter>')
		click('Tables', 'Close')

		common.wait_for_database_open()
	else:
		# Wait until the database has been created:
		common.wait_for_database_open()
		# Check that the example database has been loaded correctly:
		common.check_small_business_integrity()

	# Exit the application.
	common.exit_glom()

	# Now, open the newly created database, making sure the document was
	# saved correctly and we can open it:
	common.launch_glom()

	# It should be in the Recent document list in the recently
	# used documents:
	selectrow(common.initial_dialog, 'ttblOpenExistingDocument', 'Test.glom')

	# Buttons (prefix: 'btn') are addressed via their label text:
	click(common.initial_dialog, 'btnSelect')

	# Enter the connection credentials for the centrally hosted database:
	common.enter_connection_credentials(backend)

	# Wait until the database has been opened:
	common.wait_for_database_open()

	# Everything finished, check that no information has been lost in
	# the database:
	if example:
		common.check_small_business_integrity()

	# Exit the application.
	common.exit_glom()

	# Wait until Glom has cleaned up everything, so that removing the
	# test database is going to work:
	wait(2)

	# Remove the test database again:
	delete_database(backend)

except LdtpExecutionError, msg:
	log(msg, 'fail')

	# Print the exception to stdout because it does not seem to appear 
	# in the log file: TODO: Fix that. murrayc.
	print "LdtpExecutionError:"
	print msg
	print "\n"

	# Remove the created directory also on error, so that the test
	# does not fail because of the database already existing next time:
	delete_database(backend)

	raise LdtpExecutionError (msg)
	# TODO: Terminate the Glom application
	# os.kill(whatpid?, signal.SIGTERM)
