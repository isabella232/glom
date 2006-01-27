web_path_project = /home/murrayc/glom.org/
web_path_docs = $(web_path_project)
rsync_args = -vz --rsh ssh --delete --delete-after --recursive --cvs-exclude

#Don't upload index.html, because it breaks the now wiki-based website.
#But we link to these screenshots from the wiki.
html_docs = screenshots

post-html: 
	rsync $(rsync_args) -r $(html_docs) murrayc@murrayc.com:$(web_path_docs)

