#!/usr/bin/python

import threading
import ldtp
import ldtputils

main_window = '*Glom*'
initial_dialog = 'WelcometoGlom'

error_message = ''
error_happened = threading.Event()
error_happened.clear()

def error_cb():
	global error_message
	if not error_happened.isSet():
		# TODO: Read the actual error message from the dialog window
		error_message = 'Failed to create new database'
		error_happened.set()

def launch_glom():
	# Start glom:
	ldtp.launchapp('glom')

	# Wait for the initial dialog to appear. The argument matches on the
	# Window title, so make sure to set a window title in Glade or in the
	# code for each window.
	# Wildcard (* and ?) can be used.
	if ldtp.waittillguiexist(main_window) == 0:
		raise ldtp.LdtpExecutionError('Glom main window does not show up')
	if ldtp.waittillguiexist(initial_dialog) == 0:
		raise ldtp.LdtpExecutionError('Glom initial dialog does not show up')

def exit_glom():
	ldtp.selectmenuitem(main_window, 'mnuFile;mnuClose')

	if ldtp.waittillguinotexist('Glom-SmallBusinessExample') == 0:
		raise ldtp.LdtpExecutionError('The Glom Window does not disappear after closing the application')

# Reads hostname, username and password to use for access to a
# centrally hosted database server
def read_central_info():
	info = ldtputils.LdtpDataFileParser('central-info.xml')
	ret = [info.gettagvalue('hostname'), info.gettagvalue('username'), info.gettagvalue('password')]
	if len(ret[0]) == 0 or len(ret[1]) == 0 or len(ret[2]) == 0:
		raise ldtp.LdtpExecutionError('Connection details for centrally hosted database not provided. See the README file for how to provide the details.')
	return [ret[0], ret[1], ret[2]]

# Selects one of the backends in button_texts in the database creation dialog
def select_backend(backend_name, button_texts):
	for text in button_texts:
		if ldtp.objectexist('Creating From Example File', 'rbtn' + text):
			ldtp.click('Creating From Example File', 'rbtn' + text)
			break
	else:
		raise ldtp.LdtpExecutionError('Backend ' + backend + ' not supported')

def wait_for_database_open():
	# Be notified when an error dialog pops up
	ldtp.onwindowcreate('Warning', error_cb)
	ldtp.onwindowcreate('Error', error_cb)

	# Maybe one exists already:
	if ldtp.guiexist('Warning') or ldtp.guiexist('Error'):
		error_cb()

	# Wait for the list view to pop up in the main Glom window:
	# Note that the Window title of the Glom Window changes when the file
	# has loaded. If we use wildcards for the Window title (*Glom*) here,
	# then objectexist does not find the notebook_data
	# (ptlListOrDetailsView) widget, even when it has actually appeared.
	# TODO: Maybe we can use setcontext(), to avoid this.
	# TODO: Or maybe this has been fixed in LDTP in the meanwhile,
	# see bug #583021.
	while not ldtp.guiexist('Glom-SmallBusinessExample') or not ldtp.objectexist('Glom-SmallBusinessExample', 'ptlListOrDetailsView'):
		if error_happened.isSet():
			raise ldtp.LdtpExecutionError(error_message)
		# Wait a bit and then try again:
		ldtp.wait()

	# TODO: This doesn't seem to work currently, maybe has something to
	# do with bug #586291.
#	ldtp.removecallback('Warning')
#	ldtp.removecallback('Error')

def check_small_business_integrity():
	if(ldtp.getrowcount(main_window, 'tblTableContent') != 9):
		raise ldtp.LdtpExecutionError("Newly created database does not contain all 8 rows"); # Note there is one placeholder row

	ldtp.selecttab(main_window, 'ptlListOrDetailsView', 'Details')

	# TODO: Check that there is an image present.
	# Accerciser shows the Image Size for the widget which we may use to
	# check this. However, LDTP does not implement this yet.
