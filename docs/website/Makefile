web_path_project = /home/groups/g/gl/glom/htdocs/
web_path_docs = $(web_path_project)
rsync_args = -vz --rsh ssh --delete --delete-after --recursive --cvs-exclude


html_docs = index.html screenshots

post-html: 
	rsync $(rsync_args) -r $(html_docs) $$USER@shell.sourceforge.net:$(web_path_docs)

