#include <QString>
#include <QSize>
#include <QImage>
#include <QFileInfo>
#include <QMessageBox>

#include "data/model/Model.h"
#include "data/model/WaveFileModel.h"
#include "framework/Document.h"
#include "view/Pane.h"
#include "view/PaneStack.h"
#include "view/ViewManager.h"

#include <QPdfWriter>
#include <QSvgGenerator>
#include "pngui/epsengine/EpsEngine.h"

#include "ExportVisualisationDialog.h"
#include "ui_exportvisualisationdialog.h"


struct ExportVisualisationDialogData
{
    ExportVisualisationDialogData(Document *d, PaneStack *ps, ViewManager *vm) :
        document(d), paneStack(ps), viewManager(vm)
    {}

    Document    *document;
    PaneStack   *paneStack;
    ViewManager *viewManager;
};

ExportVisualisationDialog::ExportVisualisationDialog(Document *document, PaneStack *paneStack, ViewManager *viewManager,
                                                     QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExportVisualisationDialog),
    d(new ExportVisualisationDialogData(document, paneStack, viewManager))
{
    ui->setupUi(this);
    doExport();
}

ExportVisualisationDialog::~ExportVisualisationDialog()
{
    delete ui;
}

void ExportVisualisationDialog::doExport()
{
    if (!d->viewManager) return;
    Model *model = d->document->getMainModel();
    int zoomLevel = 420;

    sv_frame_t width = 20 * model->getSampleRate();
    sv_frame_t start = model->getStartFrame();
    sv_frame_t end = model->getEndFrame();

    int iter = 1;
    QPdfWriter pdf(QString("export.pdf"));
    pdf.setResolution(200);
    pdf.setPageOrientation(QPageLayout::Landscape);
    QPainter painterPDF;
    painterPDF.begin(&pdf);
    int y = 0;

    for (sv_frame_t f0 = start; f0 < end; f0 += width) {
        sv_frame_t f1 = f0 + width;
        if (f1 > end) f1 = end;

        QList<QImage *> images;
        for (int paneIndex = 0; paneIndex < d->paneStack->getPaneCount(); ++paneIndex) {
            Pane *pane = d->paneStack->getPane(paneIndex);
            if (!pane) continue;

            pane->setZoomLevel(zoomLevel);
            QImage *img = pane->toNewImage(f0, f1);
            images << img;
            // img->save(QString("export_%1_%2.png").arg(iter).arg(paneIndex), "PNG");
        }

        int compositeWidth = 0, compositeHeight = 0;
        foreach (QImage *img, images) {
            if (img->width() > compositeWidth) compositeWidth = img->width();
            compositeHeight = compositeHeight + img->height() + 1;
        }

        // QImage composite(compositeWidth, compositeHeight, QImage::Format_RGB32);

        if (iter > 0) pdf.newPage();
        foreach (QImage *img, images) {
            painterPDF.drawImage(compositeWidth - img->width(), y, *img);
            y = y + img->height() + 1;
        }
        if (iter % 2 == 0) {
            y = 0;
        } else {
            y += 200;
        }


        // composite.save(QString("composite_%1.png").arg(iter), "PNG");
        ++iter;
    }
    painterPDF.end();

}

//Pane *pane = m_paneStack->getCurrentPane();
//if (!pane) return;

//QString path = getSaveFileName(FileFinder::ImageFile);

//if (path == "") return;

//if (QFileInfo(path).suffix() == "") path += ".png";

//bool haveSelection = m_viewManager && !m_viewManager->getSelections().empty();

//QSize total, visible, selected;
//total = pane->getImageSize();
//visible = pane->getImageSize(pane->getFirstVisibleFrame(),
//                             pane->getLastVisibleFrame());

//sv_frame_t sf0 = 0, sf1 = 0;

//if (haveSelection) {
//    MultiSelection::SelectionList selections = m_viewManager->getSelections();
//    sf0 = selections.begin()->getStartFrame();
//    MultiSelection::SelectionList::iterator e = selections.end();
//    --e;
//    sf1 = e->getEndFrame();
//    selected = pane->getImageSize(sf0, sf1);
//}

//QStringList items;
//items << tr("Export the whole pane (%1x%2 pixels)")
//         .arg(total.width()).arg(total.height());
//items << tr("Export the visible area only (%1x%2 pixels)")
//         .arg(visible.width()).arg(visible.height());
//if (haveSelection) {
//    items << tr("Export the selection extent (%1x%2 pixels)")
//             .arg(selected.width()).arg(selected.height());
//} else {
//    items << tr("Export the selection extent");
//}

//QSettings settings;
//settings.beginGroup("VisualiserWidget");
//int deflt = settings.value("lastimageexportregion", 0).toInt();
//if (deflt == 2 && !haveSelection) deflt = 1;
//if (deflt == 0 && total.width() > 32767) deflt = 1;

//ListInputDialog *lid = new ListInputDialog
//        (this, tr("Select region to export"),
//         tr("Which region of the current pane do you want to export as an image?"),
//         items, deflt);

//if (!haveSelection) {
//    lid->setItemAvailability(2, false);
//}
//if (total.width() > 32767) { // appears to be the limit of a QImage
//    lid->setItemAvailability(0, false);
//    lid->setFootnote(tr("Note: the whole pane is too wide to be exported as a single image."));
//}

//bool ok = lid->exec();
//QString item = lid->getCurrentString();
//delete lid;

//if (!ok || item.isEmpty()) return;

//settings.setValue("lastimageexportregion", deflt);

//QImage *image = 0;

//if (item == items[0]) {
//    image = pane->toNewImage();
//} else if (item == items[1]) {
//    image = pane->toNewImage(pane->getFirstVisibleFrame(),
//                             pane->getLastVisibleFrame());
//} else if (haveSelection) {
//    image = pane->toNewImage(sf0, sf1);
//}

//if (!image) return;

//if (!image->save(path, "PNG")) {
//    QMessageBox::critical(this, tr("Failed to save image file"),
//                          tr("Failed to save image file %1").arg(path));
//}

//delete image;
