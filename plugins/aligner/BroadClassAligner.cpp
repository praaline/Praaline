#include <QString>
#include <QList>
#include "pncore/annotation/intervaltier.h"

#include "BroadClassAligner.h"

BroadClassAligner::BroadClassAligner()
{
    m_defaultBroadClass = "UNK";
}

void BroadClassAligner::addBroadPhoneticClass(const QString &name, const QList<QString> &phonemes)
{
    m_broadPhoneticClasses << name;
    foreach (QString ph, phonemes) {
        m_correspondances.insert(ph, name);
    }
}

void BroadClassAligner::resetClasses()
{
    m_broadPhoneticClasses.clear();
    m_correspondances.clear();
}

void BroadClassAligner::initialiseFR()
{
    resetClasses();
    addBroadPhoneticClass("PP", QList<QString>() << "p" << "b" << "t" << "d" << "k" << "g");      // plosives
    addBroadPhoneticClass("FF", QList<QString>() << "f" << "v" << "s" << "z" << "S" << "Z");      // fricatives
    addBroadPhoneticClass("NN", QList<QString>() << "m" << "n");                                  // nasals
    addBroadPhoneticClass("LL", QList<QString>() << "l" << "R");                                  // liquids
    addBroadPhoneticClass("GG", QList<QString>() << "w" << "H" << "j");                           // glides
    addBroadPhoneticClass("VV", QList<QString>() << "i" << "e" << "E" << "a" << "A" <<
                            "o" << "O" << "y" << "2" << "9");                                       // vowels
    addBroadPhoneticClass("VN", QList<QString>() << "e~" << "a~" << "o~" << "9~");                // nasal vowels
    addBroadPhoneticClass("EE", QList<QString>() << "@");                                         // schwa
    addBroadPhoneticClass("SIL", QList<QString>() << "_");                                        // silence
}


void BroadClassAligner::initialiseCV()
{
    resetClasses();
    // vowels
    addBroadPhoneticClass("V", QList<QString>() << "i" << "e" << "E" << "a" << "A" <<
                            "o" << "O" << "y" << "2" << "9" << "e~" << "a~" << "o~" << "9~" << "@");
    // consonants
    addBroadPhoneticClass("C", QList<QString>() << "p" << "b" << "t" << "d" << "k" << "g" <<
                            "f" << "v" << "s" << "z" << "S" << "Z" << "m" << "n" << "l" << "R" <<
                            "w" << "H" << "j");
    // silence
    addBroadPhoneticClass("SIL", QList<QString>() << "_");
}

QString BroadClassAligner::phonemeToBroadClass(const QString &phoneme) const
{
    return m_correspondances.value(phoneme.trimmed(), m_defaultBroadClass);
}

void BroadClassAligner::updatePhoneTierWithBroadClasses(IntervalTier *tier, const QString &attributeBroadClass) const
{
    if (!tier) return;
    foreach (Interval *phone, tier->intervals()) {
        phone->setAttribute(attributeBroadClass, phonemeToBroadClass(phone->text()));
    }
}

