/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef TEST_STRINGBITS_H
#define TEST_STRINGBITS_H

#include "../StringBits.h"

#include <QObject>
#include <QStringList>
#include <QtTest>

#include <iostream>

using namespace std;

class TestStringBits : public QObject
{
    Q_OBJECT

private:
    void testSplitQuoted(QString in, QStringList out) {
        // Only suitable where the output strings do not have
        // consecutive spaces in them
        QCOMPARE(StringBits::splitQuoted(in, ' '), out);
        QString in2(in);
        in2.replace(' ', ',');
        QStringList out2;
        foreach (QString o, out) {
            out2 << o.replace(' ', ',');
        }
        QCOMPARE(StringBits::splitQuoted(in2, ','), out2);
    }

private slots:
    void simple() {
        QString in = "a b c d";
        QStringList out;     
        out << "a" << "b" << "c" << "d";
        testSplitQuoted(in, out);
    }

    void dquoted() {
        QString in = "a \"b c\" d";
        QStringList out;       
        out << "a" << "b c" << "d";
        testSplitQuoted(in, out);
    }

    void drunon() {
        QString in = "a \"b c\"d e";
        QStringList out;       
        out << "a" << "b cd" << "e";
        testSplitQuoted(in, out);
    }

    void squoted() {
        QString in = "a 'b c' d";
        QStringList out;       
        out << "a" << "b c" << "d";
        testSplitQuoted(in, out);
    }

    void srunon() {
        QString in = "a 'b c'd e";
        QStringList out;       
        out << "a" << "b cd" << "e";
        testSplitQuoted(in, out);
    }

    void dempty() {
        QString in = "\"\" \"\" \"\"";
        QStringList out;       
        out << "" << "" << "";
        testSplitQuoted(in, out);
    }

    void sempty() {
        QString in = "'' '' ''";
        QStringList out;       
        out << "" << "" << "";
        testSplitQuoted(in, out);
    }

    void descaped() {
        QString in = "a \"b c\\\" d\"";
        QStringList out;         
        out << "a" << "b c\" d"; 
        testSplitQuoted(in, out);
    }

    void sescaped() {
        QString in = "a 'b c\\' d'";
        QStringList out;         
        out << "a" << "b c' d"; 
        testSplitQuoted(in, out);
    }

    void dnested() {
        QString in = "a \"b c' d\"";
        QStringList out;        
        out << "a" << "b c' d";
        testSplitQuoted(in, out);
    }

    void snested() {
        QString in = "a 'b c\" d'";
        QStringList out;        
        out << "a" << "b c\" d";
        testSplitQuoted(in, out);
    }

    void snested2() {
        QString in = "aa 'bb cc\" dd'";
        QStringList out;            
        out << "aa" << "bb cc\" dd";
        testSplitQuoted(in, out);
    }

    void qquoted() {
        QString in = "a'a 'bb' \\\"cc\" dd\\\"";
        QStringList out;                 
        out << "a'a" << "bb" << "\"cc\"" << "dd\"";  
        testSplitQuoted(in, out);
    }

    void multispace() {
        QString in = "  a'a \\'         'bb'    '      \\\"cc\" ' dd\\\" '";
        QStringList out;                                            
        out << "a'a" << "'" << "bb" << "      \"cc\" " << "dd\"" << "'";
        QCOMPARE(StringBits::splitQuoted(in, ' '), out);

        QString in2 = ",,a'a,\\',,,,,,,,,'bb',,,,',,,,,,\\\"cc\",',dd\\\",'";
        QStringList out2;
        out2 << "" << "" << "a'a" << "'" << "" << "" << "" << "" << "" << ""
             << "" << "" << "bb" << "" << "" << "" << ",,,,,,\"cc\","
             << "dd\"" << "'";
        QCOMPARE(StringBits::splitQuoted(in2, ','), out2);
    }
};

#endif

/* r928
Config: Using QtTest library 5.3.2, Qt 5.3.2
PASS   : TestStringBits::initTestCase()
PASS   : TestStringBits::simple()
PASS   : TestStringBits::dquoted()
PASS   : TestStringBits::squoted()
PASS   : TestStringBits::descaped()
FAIL!  : TestStringBits::sescaped() Compared lists have different sizes.
   Actual   (StringBits::splitQuoted(in, ' ')) size: 3
   Expected (out) size: 2
   Loc: [o/../TestStringBits.h(65)]
PASS   : TestStringBits::dnested()
PASS   : TestStringBits::snested()
PASS   : TestStringBits::snested2()
PASS   : TestStringBits::qquoted()
FAIL!  : TestStringBits::multispace() Compared lists differ at index 1.
   Actual   (StringBits::splitQuoted(in, ' ')): "         "
   Expected (out): "'"
   Loc: [o/../TestStringBits.h(100)]
FAIL!  : TestStringBits::qcommas() Compared lists have different sizes.
   Actual   (StringBits::splitQuoted(in, ',')) size: 4
   Expected (out) size: 3
   Loc: [o/../TestStringBits.h(107)]
PASS   : TestStringBits::cleanupTestCase()
Totals: 10 passed, 3 failed, 0 skipped
*/

/*curr
PASS   : TestStringBits::initTestCase()
PASS   : TestStringBits::simple()
PASS   : TestStringBits::dquoted()
PASS   : TestStringBits::squoted()
PASS   : TestStringBits::descaped()
FAIL!  : TestStringBits::sescaped() Compared lists have different sizes.
   Actual   (StringBits::splitQuoted(in, ' ')) size: 3
   Expected (out) size: 2
   Loc: [o/../TestStringBits.h(65)]
PASS   : TestStringBits::dnested()
PASS   : TestStringBits::snested()
PASS   : TestStringBits::snested2()
PASS   : TestStringBits::qquoted()
FAIL!  : TestStringBits::multispace() Compared lists have different sizes.
   Actual   (StringBits::splitQuoted(in, ' ')) size: 5
   Expected (out) size: 6
   Loc: [o/../TestStringBits.h(100)]
PASS   : TestStringBits::qcommas()
PASS   : TestStringBits::cleanupTestCase()
Totals: 11 passed, 2 failed, 0 skipped
*/
