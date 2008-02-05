by now the documentation creation is not automatized...

# html reference
# requires dot and doxygen

cd doc
ln -s ../src damaris
doxygen Doxyfile
rm damaris

# todo: copy damaris logo

# html wiki export
# requires moinmoin and damaris/data as wikidata

cd doc
# underlay must be writable, so we have to copy it...
cp -r /usr/share/moin/underlay wikiunderlay
python dump_wiki.py
cp -r /usr/share/moin/htdocs/modern tutorial-html
rm -r wikiunderlay wikiconfig.py
