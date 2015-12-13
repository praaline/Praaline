/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2008 QMUL.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#ifndef _TRANSFORM_FINDER_H_
#define _TRANSFORM_FINDER_H_

#include <QDialog>

#include <vector>

#include "transform/Transform.h"
#include "transform/TransformFactory.h"

class QVBoxLayout;
class QScrollArea;
class QLabel;
class SelectableLabel;
class QWidget;
class QTimer;

class TransformFinder : public QDialog
{
    Q_OBJECT

public:
    TransformFinder(QWidget *parent = 0);
    ~TransformFinder();

    TransformId getTransform() const;

protected slots:
    void searchTextChanged(const QString &);
    void selectedLabelChanged();
    void labelDoubleClicked();
    void timeout();
    void up();
    void down();

protected:
    QLabel *m_infoLabel;
    QLabel *m_beforeSearchLabel;
    QLabel *m_noResultsLabel;

    QScrollArea *m_resultsScroll;
    QWidget *m_resultsFrame;
    QVBoxLayout *m_resultsLayout;
    std::vector<SelectableLabel *> m_labels;
    TransformId m_selectedTransform;
    QTimer *m_timer;

    void setupBeforeSearchLabel();

    QString m_newSearchText;
    typedef std::vector<TextMatcher::Match> SortedResults;
    SortedResults m_sortedResults;
    int m_upToDateCount;
};

#endif

