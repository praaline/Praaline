/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Dataquay

    A C++/Qt library for simple RDF datastore management.
    Copyright 2009-2012 Chris Cannam.
  
    Permission is hereby granted, free of charge, to any person
    obtaining a copy of this software and associated documentation
    files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy,
    modify, merge, publish, distribute, sublicense, and/or sell copies
    of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be
    included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR
    ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
    CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
    WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

    Except as contained in this notice, the name of Chris Cannam
    shall not be used in advertising or otherwise to promote the sale,
    use or other dealings in this Software without prior written
    authorization.
*/

#include "TestBasicStore.h"
#include "TestDatatypes.h"
#include "TestTransactionalStore.h"
#include "TestImportOptions.h"
#include "TestObjectMapper.h"
#include <QtTest>

int main(int argc, char *argv[])
{
    int good = 0, bad = 0;

    QCoreApplication app(argc, argv);

    Dataquay::TestBasicStore tbs;
    if (QTest::qExec(&tbs, argc, argv) == 0) ++good;
    else ++bad;

    Dataquay::TestDatatypes td;
    if (QTest::qExec(&td, argc, argv) == 0) ++good;
    else ++bad;

    Dataquay::TestTransactionalStore tts;
    if (QTest::qExec(&tts, argc, argv) == 0) ++good;
    else ++bad;

    Dataquay::TestImportOptions ti;
    if (QTest::qExec(&ti, argc, argv) == 0) ++good;
    else ++bad;

    Dataquay::TestObjectMapper tom;
    if (QTest::qExec(&tom, argc, argv) == 0) ++good;
    else ++bad;

    if (bad > 0) {
	std::cerr << "\n********* " << bad << " test suite(s) failed!\n" << std::endl;
	return 1;
    } else {
        std::cerr << "All tests passed" << std::endl;
        return 0;
    }
}

