#!/usr/bin/python

import ldtp
import ldtputils

import os
import tempfile
import shutil
import xml.parsers.expat
import gda

main_window = '*Glom*'
initial_dialog = 'WelcometoGlom'

central_info = []

# Radio button texts in the database creation dialog
backend_create_db_button_texts = {
	'PostgresCentral': [
		'CreatedatabaseonanexternalPostgreSQLdatabaseserver,tobespecifiedinthenextstep.',
		'Createdatabaseonanexternaldatabaseserver,tobespecifiedinthenextstep.'
	], 'PostgresSelf': [
		'CreatePostgreSQLdatabaseinitsownfolder,tobehostedbythiscomputer.',
		'Createdatabaseinitsownfolder,tobehostedbythiscomputer.'
	], 'SQLite': [
		'CreateSQLitedatabaseinitsownfolder,tobehostedbythiscomputer.',
		'Createdatabaseinitsownfolder,tobehostedbythiscomputer,usingSQLite.'
	]
}

def launch_glom():
	# Start glom:
	ldtp.launchapp('glom')

	# Wait for the initial dialog to appear. The argument matches on the
	# Window title, so make sure to set a window title in Glade or in the
	# code for each window.
	# Wildcard (* and ?) can be used.
	if ldtp.waittillguiexist(main_window) == 0:
		raise ldtp.LdtpExecutionError('The Glom main window did not appear.')
	if ldtp.waittillguiexist(initial_dialog) == 0:
		raise ldtp.LdtpExecutionError('The Glom initial dialog did not appear.')

def exit_glom():
	ldtp.selectmenuitem(main_window, 'mnuFile;mnuClose')

	if ldtp.waittillguinotexist('*Glom*') == 0:
		raise ldtp.LdtpExecutionError('The Glom Window did not disappear after closing the application.')

# Reads hostname, username and password to use for access to a
# centrally hosted database server
def read_central_info():
	global central_info
	if len(central_info) > 0:
		return central_info

	info = ldtputils.LdtpDataFileParser('central-info.xml')
	ret = [info.gettagvalue('hostname'), info.gettagvalue('username'), info.gettagvalue('password')]

	if len(ret[0]) == 0 or len(ret[1]) == 0 or len(ret[2]) == 0:
		raise ldtp.LdtpExecutionError('Connection details for centrally hosted database not provided. See the README file for how to provide the details.')

	central_info = [ret[0][0], ret[1][0], ret[2][0]]
	return central_info

def enter_connection_credentials(backend_name):
	if backend_name == 'PostgresCentral':
		if ldtp.waittillguiexist('Connection Details') == 0:
			raise ldtp.LdtpExecutionError('The Glom connection details dialog did not appear.')

		(hostname, username, password) = read_central_info()

		# Set connection details
		ldtp.settextvalue('Connection Details', 'txtHost', hostname)
		ldtp.settextvalue('Connection Details', 'txtUser', username)
		ldtp.settextvalue('Connection Details', 'txtPassword', password)

		# Acknowledge the dialog
		ldtp.click('Connection Details', 'btnConnect')

		# Make sure it's gone
		if ldtp.waittillguinotexist('Connection Details') == 0:
			raise ldtp.LdtpExecutionError('The cnnection details dialog did not disappear')

# Selects one of the backends in button_texts in the database creation dialog
def select_backend(dialog_title, backend_name):
	try:
		button_texts = backend_create_db_button_texts[backend_name]
	except KeyError:
		raise ldtp.LdtpExecutionError('Backend "' + backend + '" does not exist')

	for text in button_texts:
		if ldtp.objectexist(dialog_title, 'rbtn' + text):
			ldtp.click(dialog_title, 'rbtn' + text)
			break
	else:
		raise ldtp.LdtpExecutionError('Backend "' + backend + '" not supported')

def wait_for_database_open():
	# Wait for the list view to pop up in the main Glom window:
	# Note that the Window title of the Glom Window changes when the file
	# has loaded. If we use wildcards for the Window title (*Glom*) here,
	# then objectexist does not find the notebook_data
	# (ptlListOrDetailsView) widget, even when it has actually appeared.
	# TODO: Maybe we can use setcontext(), to avoid this.
	# TODO: Or maybe this has been fixed in LDTP in the meanwhile,
	# see bug #583021.
	while not ldtp.guiexist('Glom-Test') or not ldtp.objectexist('Glom-Test', 'ptlListOrDetailsView'):
		# onwindowcreate calls the callback in a new thread, which
		# does not really help us since we don't have a mainloop the
		# callback thread could notify, so we would need to have to
		# poll an event anyway. Instead, we can simply poll directly
		# the existance of an error dialog.
		# Plus, there seems to be a bug in LDTP when running a test
		# sequence of multiple tests using onwindowcreate:
		# http://bugzilla.gnome.org/show_bug.cgi?id=586291.
		if ldtp.guiexist('Warning') or ldtp.guiexist('Error'):
			# TODO: Read error message from error dialog
			raise ldtp.LdtpExecutionError('Failed to create new database')

		# Wait a bit and then try again:
		ldtp.wait()

def check_small_business_integrity():
	if(ldtp.getrowcount(main_window, 'tblTableContent') != 9):
		raise ldtp.LdtpExecutionError("Newly created database does not contain all 8 rows") # Note there is one placeholder row

	ldtp.selecttab(main_window, 'ptlListOrDetailsView', 'Details')

	# TODO: Check that there is an image present.
	# Accerciser shows the Image Size for the widget which we may use to
	# check this. However, LDTP does not implement this yet.

# Replace $hostname and $username with the corresponding values the user
# has set for hostname and username in central-info.
def process_central_file(file):
	(hostname, username, password) = read_central_info()
	output = tempfile.NamedTemporaryFile()

	for line in open(file, 'r'):
		line = line.replace('$hostname', hostname)
		line = line.replace('$username', username)
		output.write(line)
	output.flush()
	shutil.copy(output.name, file)

# Create a new database for the given backend, to test Glom functionality
# with it. The .glom file will end up in TestDatabase/Test.glom
def create_test_database(backend):
	# Copy Database template
	shutil.copytree('database-templates/' + backend, 'TestDatabase')

	if backend == 'PostgresCentral':
		# Replace username and hostname in .glom file and in SQL data
		process_central_file('TestDatabase/data')
		process_central_file('TestDatabase/Test.glom')

		# TODO: Does this require user/password? If so, do this with pygda.
		if os.system('createdb glom_test') != 0:
			raise ldtp.LdtpExecutionError('createdb failed')
		if os.system('cat TestDatabase/data | psql glom_test') != 0:
			raise ldtp.LdtpExecutionError('psql failed')

# Deletes a database created with create_test_database again.
def delete_test_database(backend):
	if backend == 'PostgresCentral':
		# Read glom file to find out database name and port
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
		(hostname, username, password) = read_central_info()

		# Remove the database from the central PostgreSQL server
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
