*** Create the xdg-app package like so:

$ cd glom-xdg-app-builder
$ xdg-app-builder --require-changes ../glom-xdgapp manifest.json
$ xdg-app build-export --gpg-sign="murrayc@murrayc.com" /repos/glom ../glom-xdgapp
$ xdg-app repo-update /repos/glom

Then copy all of /repos/glom/* into the repos/ directory that appears
on the website. For glom that is the repos/ directory in its gh_pages
branch on github:
https://github.com/murraycu/glom/tree/gh-pages/repo

Users will need to download the gpg key too, so:
$ gpg --output glom.gpg --export murrayc@murrayc.com
Then copy that into the keys/ directory of the website:
https://github.com/murraycu/glom/tree/gh-pages/keys

** Install the xdg-app package like so:

$ wget http://www.murrayc.com/xdg-app/keys/murrayc.gpg
$ xdg-app add-remote --user --gpg-import=murrayc.gpg glom http://www.murrayc.com/xdg-app/repos/glom/
$ xdg-app install-app --user glom org.glom.Glom

** Run the xdg-app like so:
$ xdg-app run org.glom.Glom

