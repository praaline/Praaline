#include <QSqlDatabase>
#include <QSqlQuery>

#include "DisMoSerialiserSQL.h"

using namespace DisMoAnnotator;

DisMoSerialiserSql::DisMoSerialiserSql(QObject *parent) :
    QObject(parent)
{
}

// static
void DisMoSerialiserSql::serializeToDatabase(TokenList &TL, QSqlDatabase &db, const QString &annotationID, const QString &speakerID)
{
    if (!db.isOpen())
        db.open();
    QSqlQuery q_min(db), q_mwu(db);

    db.transaction();

    q_min.prepare("DELETE FROM tok_min WHERE annotationID=:annotationID AND speakerID=:speakerID");
    q_min.bindValue(":annotationID", annotationID);
    q_min.bindValue(":speakerID", speakerID);
    q_min.exec();
    q_mwu.prepare("DELETE FROM tok_mwu WHERE annotationID=:annotationID AND speakerID=:speakerID");
    q_mwu.bindValue(":annotationID", annotationID);
    q_mwu.bindValue(":speakerID", speakerID);
    q_mwu.exec();
    q_min.prepare("INSERT INTO tok_min "
                  "(annotationID, speakerID, intervalNo, tMin, tMax, xText, pos_min, disfluency, lemma_min, pos_ext_min, dismoconfidence, dismomethod) VALUES "
                  "(:annotationID, :speakerID, :intervalNo, :tMin, :tMax, :xText, :pos_min, :disfluency, :lemma_min, :pos_ext_min, :dismoconfidence, :dismomethod)");
    q_mwu.prepare("INSERT INTO tok_mwu "
                  "(annotationID, speakerID, intervalNo, tMin, tMax, xText, pos_mwu, discourse, lemma_mwu, pos_ext_mwu, dismoconfidence, dismomethod) VALUES "
                  "(:annotationID, :speakerID, :intervalNo, :tMin, :tMax, :xText, :pos_mwu, :discourse, :lemma_mwu, :pos_ext_mwu, :dismoconfidence, :dismomethod)");
    q_min.bindValue(":annotationID", annotationID);
    q_min.bindValue(":speakerID", speakerID);
    q_mwu.bindValue(":annotationID", annotationID);
    q_mwu.bindValue(":speakerID", speakerID);
    int intervalNo_min = 1; int intervalNo_mwu = 1;
    foreach (Token *tl_tok_mwu, TL) {
        q_mwu.bindValue(":intervalNo", intervalNo_mwu);
        q_mwu.bindValue(":tMin", tl_tok_mwu->tMin().toNanoseconds());
        q_mwu.bindValue(":tMax", tl_tok_mwu->tMax().toNanoseconds());
        q_mwu.bindValue(":xText", tl_tok_mwu->text());
        q_mwu.bindValue(":pos_mwu", tl_tok_mwu->getTagPOS());
        q_mwu.bindValue(":discourse", tl_tok_mwu->getTagDiscourse());
        q_mwu.bindValue(":lemma_mwu", tl_tok_mwu->getLemma());
        q_mwu.bindValue(":pos_ext_mwu", tl_tok_mwu->getTagPOSext());
        q_mwu.bindValue(":dismoconfidence", tl_tok_mwu->getConfidencePOS());
        q_mwu.bindValue(":dismomethod", tl_tok_mwu->getMatchTypePOS());
        q_mwu.exec();
        foreach (TokenUnit *tl_tok_min, tl_tok_mwu->getTokenUnits()) {
            q_min.bindValue(":intervalNo", intervalNo_min);
            q_min.bindValue(":tMin", tl_tok_min->tMin().toNanoseconds());
            q_min.bindValue(":tMax", tl_tok_min->tMax().toNanoseconds());
            q_min.bindValue(":xText", tl_tok_min->text());
            q_min.bindValue(":pos_min", tl_tok_min->getTagPOS());
            q_min.bindValue(":disfluency", tl_tok_min->getTagDisfluency());
            q_min.bindValue(":lemma_min", tl_tok_min->getLemma());
            q_min.bindValue(":pos_ext_min", tl_tok_min->getTagPOSext());
            q_min.bindValue(":dismoconfidence", tl_tok_min->getConfidencePOS());
            q_min.bindValue(":dismomethod", tl_tok_min->getMatchTypePOS());
            q_min.exec();
            intervalNo_min++;
        }
        intervalNo_mwu++;
    }

    db.commit();

}



