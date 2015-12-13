
TEMPLATE = subdirs
SUBDIRS = sub_lib sub_tests

sub_lib.file = lib.pro
sub_tests.file = tests/tests.pro
sub_tests.depends = sub_lib

