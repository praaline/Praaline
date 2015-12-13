
Dataquay
========

Dataquay is a free open source library that provides a friendly C++
API for an RDF data store using Qt4 classes and containers.

  http://breakfastquay.com/dataquay/

This is version 0.9 of Dataquay.  Note that this is a pre-1.0 release
and the API is still subject to change.

Dataquay is simple to use and easy to integrate. It is principally
intended for use in Qt-based applications that would like to use an
RDF datastore as backing for in-memory project data, to avoid having
to provide application data-specific file formats and to make it easy
to augment the data with descriptive metadata pulled in from external
sources. Dataquay is also intended to be useful for applications whose
primary purpose is not related to RDF but that have ad-hoc RDF needs
for metadata management.

Dataquay does not include the datastore implementation itself; instead
it is a wrapper around either Redland (http://librdf.org) or Sord
(http://drobilla.net/software/sord/).

Dataquay provides these features:

 * Conversion between arbitrary data types and RDF nodes using the
   Node class and QVariant types. Data are converted to XSD datatypes
   where possible, using an easily extended mechanism.

 * Simple and flexible storage, query, and file I/O (natively using
   the Turtle format) functions provided by the Store and BasicStore
   classes.

 * Straightforward transactional interface via TransactionalStore and
   Transaction. Transactions are atomic at the library level, and are
   isolated from any non-transactional queries occurring at the same
   time. The transactional implementation is designed to be simple
   rather than scalable, intended for use in encapsulating single-user
   editing operations: it is probably not wise to use Dataquay as a
   store for server applications, although it might work.

 * Optional ODBC-style transactional Connection interface.

 * ObjectMapper, a facility which can take care of a complete object
   hierarchy, map it to the RDF store, and synchronise in both
   directions any changes to the hierarchy or the store. Arbitrary
   mappings between C++ class and property names and RDF URIs can be
   specified.  The mapping is flexible enough to allow you to load
   many publicly generated Linked Data sources directly into object
   class hierarchies, if you wish (although this is not usually an
   ideal way to handle unpredictable data sources).

 * API documentation, thread safety, documentation of threading
   requirements, readable code, and basic unit tests included.

 * BSD licensing. You can use Dataquay cost-free in commercial or open
   source applications and modify it as you like with no particular
   requirements except acknowledgement in your copyright notes. We do
   ask that you let us know of any bugs, fixes and enhancements you
   might find -- particularly for serious bugs -- but you have no
   obligation to do so.


Choice of datastore
-------------------

Dataquay can be built against either Redland (http://librdf.org) or
Sord (http://drobilla.net/software/sord/).

You will need to have the Raptor, Rasqal and Redland libraries
installed in order to build and use Dataquay with Redland, or the Sord
and Serd libraries installed in order to use Sord.

To use Redland, ensure USE_REDLAND is defined in config.pri; to use
Sord, ensure USE_SORD is defined.  The choice is made at compile time:
Dataquay does not have any module or plugin system.

Which to choose?  Sord is smaller and simpler, Redland more complete.
SPARQL queries and data loading from a remote (e.g. HTTP) resource are
only available when using Redland.  For this reason, anyone packaging
Dataquay for general use (e.g. in a Linux distribution) is advised to
build with Redland.


Compiling Dataquay
------------------

Dataquay requires Qt 4.5 or newer with qmake.

Make sure your preferred datastore (see above) is selected in the
config.pri file.

Run "qmake dataquay.pro", then make.

This should compile the library and unit tests and automatically run
the tests. If compilation succeeds but any unit tests fail, please do
not use the library -- report the problem to me instead.

Linux users can "make install" after building if desired.

A debug build will print a lot of information to standard error during
run time; this can be very helpful for debugging your application, but
it can quickly become excessive.  Perform a release build to eliminate
this output.


Chris Cannam
chris.cannam@breakfastquay.com
