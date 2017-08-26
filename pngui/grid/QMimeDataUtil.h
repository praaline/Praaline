#ifndef QMIMEDATAUTIL_H
#define QMIMEDATAUTIL_H

#include <QtCore>
#include <QTableView>
#include <QItemSelectionModel>
#include <QMimeData>
#include <QModelIndex>
#include <QPair>
#include <QItemSelection>

#include "QAIVlib_global.h"
#include "QAdvancedTableView.h"

QAIVLIBSHARED_EXPORT void qMimeDataAddCsv(QMimeData* mimeData, QTableView* view, Qt::ItemDataRole role = Qt::DisplayRole);

QAIVLIBSHARED_EXPORT void qMimeDataAddHtml(QMimeData* mimeData, QTableView* view, Qt::ItemDataRole role = Qt::DisplayRole);

QAIVLIBSHARED_EXPORT void qMimeDataAddPlainText(QMimeData* mimeData, QTableView* view, Qt::ItemDataRole role = Qt::DisplayRole);

QAIVLIBSHARED_EXPORT QPair<QModelIndex, QModelIndex> selectionEdges(QItemSelection selection);

#endif // QMIMEDATAUTIL_H
