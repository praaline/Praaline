#include <QObject>
#include <QItemDelegate>
#include <QComboBox>
#include <QCheckBox>
#include <QApplication>
#include <QAbstractItemView>
#include <QStylePainter>
#include "CheckBoxList.h"

// internal private delegate
class CheckBoxListDelegate : public QItemDelegate
{
public:

    CheckBoxListDelegate(QObject *parent) : QItemDelegate(parent)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const
    {
        if (!index.isValid()) return;
        //Get item data
        bool value = index.data(Qt::UserRole).toBool();
        QString text = index.data(Qt::DisplayRole).toString();
        // fill style options with item data
        const QStyle *style = QApplication::style();
        QStyleOptionButton opt;
        opt.state |= value ? QStyle::State_On : QStyle::State_Off;
        opt.state |= QStyle::State_Enabled;
        opt.text = text;
        opt.rect = option.rect;
        // draw item data as CheckBox
        style->drawControl(QStyle::CE_CheckBox,&opt,painter);
    }

    QWidget *createEditor(QWidget *parent,
                          const QStyleOptionViewItem & option ,
                          const QModelIndex & index ) const
    {
        Q_UNUSED(option)
        Q_UNUSED(index)
        // create check box as our editor
        QCheckBox *editor = new QCheckBox(parent);
        return editor;
    }

    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        if (!index.isValid()) return;
        //set editor data
        QCheckBox *myEditor = static_cast<QCheckBox*>(editor);
        if (!myEditor) return;
        myEditor->setText(index.data(Qt::DisplayRole).toString());
        myEditor->setChecked(index.data(Qt::UserRole).toBool());
    }

    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
    {
        if (!index.isValid()) return;
        //get the value from the editor (CheckBox)
        QCheckBox *myEditor = static_cast<QCheckBox*>(editor);
        if (!myEditor) return;
        bool value = myEditor->isChecked();
        //set model data
        QMap<int,QVariant> data;
        data.insert(Qt::DisplayRole, myEditor->text());
        data.insert(Qt::UserRole, value);
        model->setItemData(index, data);
    }

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        if (!index.isValid()) return;
        if (!editor) return;
        editor->setGeometry(option.rect);
    }
};

//min-width:10em;
CheckBoxList::CheckBoxList(QWidget *parent)
    : QComboBox(parent), m_DisplayText("")
{
    // set delegate items view
    view()->setItemDelegate(new CheckBoxListDelegate(this));
    //view()->setStyleSheet("  padding: 15px; ");
    // Enable editing on items view
    view()->setEditTriggers(QAbstractItemView::CurrentChanged);
    // set "CheckBoxList::eventFilter" as event filter for items view
    view()->viewport()->installEventFilter(this);
    // default
    view()->setAlternatingRowColors(true);
}

CheckBoxList::~CheckBoxList()
{
}

bool CheckBoxList::eventFilter(QObject *object, QEvent *event)
{
    // don't close items view after we release the mouse button
    // by simple eating MouseButtonRelease in viewport of items view
    if(event->type() == QEvent::MouseButtonRelease && object==view()->viewport()) {
        return true;
    }
    return QComboBox::eventFilter(object,event);
}
void CheckBoxList::paintEvent(QPaintEvent *)
{
    QStylePainter painter(this);
    painter.setPen(palette().color(QPalette::Text));

    // draw the combobox frame, focusrect and selected etc.
    QStyleOptionComboBox opt;
    initStyleOption(&opt);

    // if no display text been set , use "..." as default
    if(m_DisplayText.isEmpty()) {
        QString text;
        for (int i = 0; i < count(); ++i) {
            if (itemData(i).toBool())
                text.append(itemText(i)).append(", ");
        }
        if (text.length() > 2) text.chop(2);
        opt.currentText = text;
    }
    else {
        opt.currentText = m_DisplayText;
    }
    painter.drawComplexControl(QStyle::CC_ComboBox, opt);

    // draw the icon and text
    painter.drawControl(QStyle::CE_ComboBoxLabel, opt);
}

void CheckBoxList::setDisplayText(QString text)
{
    m_DisplayText = text;
}

QString CheckBoxList::getDisplayText() const
{
    return m_DisplayText;
}

