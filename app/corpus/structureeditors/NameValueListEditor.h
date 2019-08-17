#ifndef NAMEVALUELISTEDITOR_H
#define NAMEVALUELISTEDITOR_H

#include <QWidget>

namespace Praaline {
namespace Core {
class NameValueList;
class NameValueListDatastore;
}
}

struct NameValueListEditorData;

class NameValueListEditor : public QWidget
{
    Q_OBJECT
public:
    explicit NameValueListEditor(QWidget *parent = nullptr);
    ~NameValueListEditor();

signals:

public slots:
    void rebind(Praaline::Core::NameValueListDatastore *datastore);

protected slots:
    void newList();
    void saveList();
    void deleteList();
    void importLists();
    void exportLists();
    void addItem();
    void removeItem();
    void moveItemUp();
    void moveItemDown();

    void selectedListChanged(int);

private:
    NameValueListEditorData *d;

    void setupActions();
    void rebindList(Praaline::Core::NameValueList *list);
};

#endif // NAMEVALUELISTEDITOR_H
