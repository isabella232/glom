#!/usr/bin/python

import ldtp

main_window = '*Glom*'
initial_dialog = 'WelcometoGlom'

def launch_glom():
	# Start glom:
	ldtp.launchapp('glom')

	# Wait for the initial dialog to appear. The argument matches on the
	# Window title, so make sure to set a window title in Glade or in the
	# code for each window.
	# Wildcard (* and ?) can be used.
	if ldtp.waittillguiexist(main_window) == 0:
		raise ldtp.LdtpExecutionError('Glom main window does not show up')
	print initial_dialog
	if ldtp.waittillguiexist(initial_dialog) == 0:
		raise ldtp.LdtpExecutionError('Glom initial dialog does not show up')
