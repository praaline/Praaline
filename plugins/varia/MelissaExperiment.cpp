#include <QString>
#include <QList>
#include <QStringList>
#include "MelissaExperiment.h"

MelissaExperiment::MelissaExperiment()
{

}

QString mergeChocolat(const QString &subjectID, const QString &condition)
{
    QStringList list;
    list << QString("stim/chocolat_%1_A.wav").arg(condition) << QString("rep/%1_chocolat_%2_A.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_B.wav").arg(condition) << QString("rep/%1_chocolat_%2_B.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_C.wav").arg(condition) << QString("rep/%1_chocolat_%2_C.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_D.wav").arg(condition) << QString("rep/%1_chocolat_%2_D.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_E.wav").arg(condition) << QString("rep/%1_chocolat_%2_E.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_F.wav").arg(condition) << QString("rep/%1_chocolat_%2_F.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_G.wav").arg(condition) << QString("rep/%1_chocolat_%2_G.wav").arg(subjectID).arg(condition)
         << QString("stim/chocolat_%1_H.wav").arg(condition) << QString("rep/%1_chocolat_%2_H.wav").arg(subjectID).arg(condition);
    QString sox("sox ");
    foreach (QString filename, list) sox.append(filename).append(" ");
    sox.append(QString("out/%1_chocolat_%2.wav").arg(subjectID).arg(condition));
    return sox;
}

QString mergePlantation(const QString &subjectID, const QString &condition)
{
    QStringList list;
    list << QString("stim/plantation_%1_A.wav").arg(condition) << QString("rep/%1_plantation_%2_A.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_B.wav").arg(condition) << QString("rep/%1_plantation_%2_B.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_C.wav").arg(condition) << QString("rep/%1_plantation_%2_C.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_D.wav").arg(condition) << QString("rep/%1_plantation_%2_D.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_E.wav").arg(condition) << QString("rep/%1_plantation_%2_E.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_F.wav").arg(condition) << QString("rep/%1_plantation_%2_F.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_G.wav").arg(condition) << QString("rep/%1_plantation_%2_G.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_H.wav").arg(condition) << QString("rep/%1_plantation_%2_H.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_I.wav").arg(condition) << QString("rep/%1_plantation_%2_I.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_J.wav").arg(condition) << QString("rep/%1_plantation_%2_J.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_K.wav").arg(condition) << QString("rep/%1_plantation_%2_K.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_L.wav").arg(condition) << QString("rep/%1_plantation_%2_L.wav").arg(subjectID).arg(condition)
         << QString("stim/plantation_%1_M.wav").arg(condition) << QString("rep/%1_plantation_%2_M.wav").arg(subjectID).arg(condition);
    QString sox("sox ");
    foreach (QString filename, list) sox.append(filename).append(" ");
    sox.append(QString("out/%1_plantation_%2.wav").arg(subjectID).arg(condition));
    return sox;
}

QString mergeTabouret(const QString &subjectID, const QString &condition)
{
    QStringList list;
    list << QString("stim/tabouret_%1_A.wav").arg(condition) << QString("rep/%1_tabouret_%2_A.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_B.wav").arg(condition) << QString("rep/%1_tabouret_%2_B.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_C.wav").arg(condition) << QString("rep/%1_tabouret_%2_C.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_D.wav").arg(condition) << QString("rep/%1_tabouret_%2_D.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_E.wav").arg(condition) << QString("rep/%1_tabouret_%2_E.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_F.wav").arg(condition) << QString("rep/%1_tabouret_%2_F.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_G.wav").arg(condition) << QString("rep/%1_tabouret_%2_G.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_H.wav").arg(condition) << QString("rep/%1_tabouret_%2_H.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_I.wav").arg(condition) << QString("rep/%1_tabouret_%2_I.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_J.wav").arg(condition) << QString("rep/%1_tabouret_%2_J.wav").arg(subjectID).arg(condition)
         << QString("stim/tabouret_%1_K.wav").arg(condition) << QString("rep/%1_tabouret_%2_K.wav").arg(subjectID).arg(condition);
    QString sox("sox ");
    foreach (QString filename, list) sox.append(filename).append(" ");
    sox.append(QString("out/%1_tabouret_%2.wav").arg(subjectID).arg(condition));
    return sox;
}

// static
QString MelissaExperiment::multiplex(int participantNo)
{
    QString chocolat, tabouret, plantation;
    QString subjectID = QString("S%1").arg(participantNo);
    if      (participantNo ==  1) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo ==  2) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo ==  3) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo ==  4) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo ==  5) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo ==  6) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo ==  7) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo ==  8) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo ==  9) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 10) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 11) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 12) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 13) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 14) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 15) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 16) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 17) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 18) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 19) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 20) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 21) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 22) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 23) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 24) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }

    else if (participantNo == 31) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 32) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 33) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 34) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 35) { chocolat = mergeChocolat(subjectID, "S");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 36) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 37) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 38) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 39) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 40) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 41) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 42) { chocolat = mergeChocolat(subjectID, "I");    tabouret = mergeTabouret(subjectID, "C");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 43) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 44) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 45) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 46) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 47) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 48) { chocolat = mergeChocolat(subjectID, "C");    tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "Cdc");  }
    else if (participantNo == 49) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 50) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 51) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "S");    }
    else if (participantNo == 52) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "Cdc");  plantation = mergePlantation(subjectID, "I");    }
    else if (participantNo == 53) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "S");    plantation = mergePlantation(subjectID, "C");    }
    else if (participantNo == 54) { chocolat = mergeChocolat(subjectID, "Cdc");  tabouret = mergeTabouret(subjectID, "I");    plantation = mergePlantation(subjectID, "C");    }

    return QString("%1\n%2\n%3").arg(chocolat).arg(tabouret).arg(plantation);
}
