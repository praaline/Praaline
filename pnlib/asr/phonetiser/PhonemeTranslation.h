#ifndef PHONEMETRANSLATION_H
#define PHONEMETRANSLATION_H

#include <QString>

namespace Praaline {
namespace ASR {

struct PhonemeTranslationData;

class PhonemeTranslation
{
public:
    PhonemeTranslation();
    PhonemeTranslation(const QString &nameSource, const QString &nameTarget);
    ~PhonemeTranslation();

    QString translate(const QString &phoneme) const;
    QString backtranslate(const QString &phoneme) const;

    QString nameSource() const;
    void setNameSource(const QString &name);
    QString nameTarget() const;
    void setNameTarget(const QString &name);

    void clear();
    void insertTranslation(const QString &phonemeSource, const QString &phonemeTarget);
    void insertBacktranslation(const QString &phonemeTarget, const QString &phonemeSource);
    void insertCommutative(const QString &phonemeA, const QString &phonemeB);

    bool read(const QString &filename);
    bool write(const QString &filename) const;

private:
    PhonemeTranslationData *d;
};

} // namespace ASR
} // namespace Praaline

#endif // PHONEMETRANSLATION_H
