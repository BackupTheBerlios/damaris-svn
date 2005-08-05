EPYDOC=epydoc

doc:	doc/index.html

doc/index.html: *.py
	$(EPYDOC) --html -o doc *.py

clean:
	rm -f *.pyc *.pyo *~ -r doc/*
