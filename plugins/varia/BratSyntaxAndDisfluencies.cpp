#include <QList>
#include <QMap>

#include "pncore/annotation/IntervalTier.h"
#include "pncore/annotation/SequenceTier.h"
using namespace Praaline::Core;

#include "BratSyntaxAndDisfluencies.h"

BratSyntaxAndDisfluencies::BratSyntaxAndDisfluencies()
{

}

QString htmlHead(const QString &pageTitle)
{
    QString html;
    html.append("<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
    html.append("<html lang=\"fr-FR\" xml:lang=\"fr-FR\" xmlns=\"http://www.w3.org/1999/xhtml\">\n");
    html.append("<head>\n");
    html.append("<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\"/>\n");
    html.append(QString("<title>%1</title>\n").arg(pageTitle));
    html.append("<link rel=\"stylesheet\" type=\"text/css\" href=\"style-vis.css\"/>\n");
    html.append("<script type=\"text/javascript\" src=\"client/lib/head.load.min.js\"></script>\n");
    html.append("</head>\n");
    return html;
}

QString scriptInit()
{
    QString s;
    s.append("var bratLocation = '';\n");
    s.append("head.js(");
    // External libraries
    s.append("    bratLocation + 'client/lib/jquery.min.js',\n");
    s.append("    bratLocation + 'client/lib/jquery.svg.min.js',\n");
    s.append("    bratLocation + 'client/lib/jquery.svgdom.min.js',\n");
    // brat helper modules
    s.append("    bratLocation + 'client/src/configuration.js',\n");
    s.append("    bratLocation + 'client/src/util.js',\n");
    s.append("    bratLocation + 'client/src/annotation_log.js',\n");
    s.append("    bratLocation + 'client/lib/webfont.js',\n");
    // brat modules
    s.append("    bratLocation + 'client/src/dispatcher.js',\n");
    s.append("    bratLocation + 'client/src/url_monitor.js',\n");
    s.append("    bratLocation + 'client/src/visualizer.js'\n");
    s.append(");\n");
    s.append("var webFontURLs = [\n");
    s.append("    bratLocation + 'static/fonts/Astloch-Bold.ttf',\n");
    s.append("    bratLocation + 'static/fonts/PT_Sans-Caption-Web-Regular.ttf',\n");
    s.append("    bratLocation + 'static/fonts/Liberation_Sans-Regular.ttf'\n");
    s.append("];\n");
    return s;
}

QString BratSyntaxAndDisfluencies::getHTML(QPointer<Praaline::Core::CorpusCommunication> com)
{
    QString html;


    return html;
}

//For each interval in response
//text of the interval --> name, condition

//get syntactic sequences >> create list of opening parentheses and closing parentheses

//get intvFrom, intvTo for tok_mwu
//get tok_mwus in a list
//foreach tok_mwu
//if in opening-paretheses :: add it
//type tok_mwu text
//if in closing parentheses :: add it

void test()
{
    QMap<int, int> tok_min_charStart;
    QMap<int, int> tok_min_charEnd;
    QList<Sequence *> sequencesSyntax;
    QMap<int, QString> tok_mwu_textOpening;
    QMap<int, QString> tok_mwu_textClosing;


}

