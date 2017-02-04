#include <QDebug>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QRegularExpression>

#include "RuleBasedPhonetiser.h"

namespace Praaline {
namespace ASR {

struct RuleBasedPhonetiserData {
    QList<QPair<QString, QString> > rules;
};

RuleBasedPhonetiser::RuleBasedPhonetiser(QObject *parent) :
    Phonetiser(parent), d(new RuleBasedPhonetiserData)
{
}

RuleBasedPhonetiser::~RuleBasedPhonetiser()
{
    delete d;
}

bool RuleBasedPhonetiser::readRuleFile(const QString &filename)
{
    QFile fileRules(filename);
    if ( !fileRules.open( QIODevice::ReadOnly | QIODevice::Text ) ) return false;
    d->rules.clear();
    QTextStream rules(&fileRules);
    rules.setCodec("UTF-8");
    while (!rules.atEnd()) {
        QString line = rules.readLine().trimmed();
        if (line.isEmpty()) continue;
        if (line.startsWith("#")) continue;
        QStringList parts = line.split("::");
        if (parts.count() != 2) continue;
        QPair<QString, QString> rule;
        rule.first = parts[0].trimmed();
        rule.second = parts[1].trimmed().prepend(" ").append(" ");
        d->rules << rule;
    }
    qDebug() << QString("%1 rules loaded.").arg(d->rules.count());
    fileRules.close();
    return true;
}

QString RuleBasedPhonetiser::phonetise(const QString &input)
{
    QString output(input);

    output = output.replace(QRegularExpression("\\s"), " | ").trimmed();
    output = output.prepend("| ");
    if (!output.endsWith("|")) output = output.append(" |");

//    QFile fileDebug("D:/Aligner_train_tests/debug.txt");
//    if ( !fileDebug.open( QIODevice::WriteOnly | QIODevice::Text ) ) return QString();
//    QTextStream debug(&fileDebug);
//    debug.setCodec("UTF-8");
//    debug.setGenerateByteOrderMark(true);

    QPair<QString, QString> rule;
    foreach (rule, d->rules) {
        if (rule.first.isEmpty()) continue;
        output = output.replace(QRegularExpression(rule.first), rule.second).replace(QRegularExpression("(\\s)+"), " ");
        // debug << rule.first << "\t" << rule.second << "\n" << output << "\n\n";
    }
    //fileDebug.close();

    return output;
}

} // namespace ASR
} // namespace Praaline
