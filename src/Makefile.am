bin_PROGRAMS=
cpp_sources=

bin_PROGRAMS+= timestamper
timestamper_SOURCES= timestamper.cpp
cpp_sources+= $(timestamper_SOURCES)

bin_PROGRAMS+= xpipe
xpipe_SOURCES= xpipe.cpp
cpp_sources+= $(xpipe_SOURCES)

bin_PROGRAMS+= paster
paster_SOURCES= paster.cpp
cpp_sources+= $(paster_SOURCES)

lint-local:
	$(top_srcdir)/cpplint.py --extensions=cpp,hpp $(sort $(cpp_sources))
