web_path_project = /home/murrayc/glom.org/
web_path_docs = $(web_path_project)
rsync_args = -vz --rsh ssh --delete --delete-after --recursive --cvs-exclude


html_docs = index.html screenshots

post-html: 
	rsync $(rsync_args) -r $(html_docs) murrayc@murrayc.com:$(web_path_docs)

