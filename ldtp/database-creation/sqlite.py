#!/usr/bin/python
import create_db

try:
	create_db.test_create_db([
		'CreateSQLitedatabaseinitsownfolder,tobehostedbythiscomputer.',
		'Createdatabaseinitsownfolder,tobehostedbythiscomputer,usingSQLite.'
	])
except create_db.BackendUnavailableError:
	raise LdtpExecutionError('Self-hosted postgresql backend not available')



