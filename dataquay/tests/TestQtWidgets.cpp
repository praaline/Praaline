
#include "dataquay/BasicStore.h"
#include "dataquay/PropertyObject.h"
#include "dataquay/TransactionalStore.h"
#include "dataquay/Connection.h"
#include "dataquay/RDFException.h"
#include "dataquay/objectmapper/ObjectStorer.h"
#include "dataquay/objectmapper/ObjectLoader.h"
#include "dataquay/objectmapper/ObjectBuilder.h"
#include "dataquay/objectmapper/TypeMapping.h"
#include "../src/Debug.h"

#include <QStringList>
#include <QMutex>
#include <QMutexLocker>
#include <QWidget>
#include <iostream>

#include <QMainWindow>
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFrame>
#include <QMenuBar>
#include <QMenu>
#include <QAction>

#include <cassert>

using std::cerr;
using std::endl;

#include <QMetaObject>
#include <QMetaProperty>

#include <QApplication>

namespace Dataquay
{
namespace Test
{

static Uri qtypePrefix("http://breakfastquay.com/rdf/dataquay/qtype/");
static Uri dqPrefix("http://breakfastquay.com/rdf/dataquay/common/"); //???

struct LayoutLoader : public ObjectLoader::LoadCallback {

    void loaded(ObjectLoader *m, ObjectLoader::NodeObjectMap &map, Node node, QObject *o)
    {
	cerr << "LayoutLoader::loaded: uri " << node.value.toStdString() << ", object type " << o->metaObject()->className() << endl;

	Store *s = m->getStore();
	PropertyObject pod(s, dqPrefix.toString(), node); //!!! was cacheing, but that doesn't support this interface yet

	QObject *parent = o->parent();
	if (dynamic_cast<QMainWindow *>(parent) &&
	    dynamic_cast<QMenu *>(o)) {
	    dynamic_cast<QMainWindow *>(parent)->menuBar()->addMenu
		(dynamic_cast<QMenu *>(o));
	}
	if (dynamic_cast<QMenu *>(parent) &&
	    dynamic_cast<QAction *>(o)) {
	    dynamic_cast<QMenu *>(parent)->addAction
		(dynamic_cast<QAction *>(o));
	}

	QObject *layout = 0;  
	QObject *layoutOf = 0;
	QObject *centralOf = 0;

	if (pod.hasProperty("layout")) {
	    layout = map.value(Uri(pod.getProperty("layout").value<Uri>()));
	    std::cerr << "have layout property: in map is " << layout << std::endl;
	}
	if (pod.hasProperty("layout_of")) {
	    layoutOf = map.value(Uri(pod.getProperty("layout_of").value<Uri>()));
	    std::cerr << "have layout_of property: in map is " << layoutOf << std::endl;
	}
	if (pod.hasProperty("central_widget_of")) {
	    centralOf = map.value(Uri(pod.getProperty("central_widget_of").value<Uri>()));
	    std::cerr << "have central_widget_of property: in map is " << centralOf << std::endl;
	}

	if (centralOf) {
	    QMainWindow *m = dynamic_cast<QMainWindow *>(centralOf);
	    QWidget *w = dynamic_cast<QWidget *>(o);
	    if (m && w) {
		m->setCentralWidget(w);
		std::cerr << "added central widget" << std::endl;
	    }
	}

	if (layoutOf) {
	    QWidget *w = dynamic_cast<QWidget *>(layoutOf);
	    QLayout *l = dynamic_cast<QLayout *>(o);
	    if (w && l) {
		w->setLayout(l);
		std::cerr << "added layout to widget" << std::endl;
	    }
	}
	
	if (layout) {
	    QLayout *cl = dynamic_cast<QLayout *>(layout);
	    if (cl) {
		QWidget *w = dynamic_cast<QWidget *>(o);
		if (w) cl->addWidget(w);
		std::cerr << "added widget to layout" << std::endl;
	    }
	}
    }
};

struct LayoutStorer : public ObjectStorer::StoreCallback {

    void stored(ObjectStorer *m, ObjectStorer::ObjectNodeMap &map, QObject *o, Node node)
    {
	std::cerr << "LayoutStorer: stored: " << node << std::endl;

	PropertyObject pod(m->getStore(), dqPrefix.toString(), node);

	//!!! not right -- these calls back to store() cause an
	//!!! infinite loop -- we really need to store only if not
	//!!! stored already, or modify ObjectStorer so as to call
	//!!! callbacks only after all objects have been stored (as
	//!!! ObjectLoader does already)

	QLayout *layout = dynamic_cast<QLayout *>(o);
	if (layout) {
	    pod.setProperty(0, "layout_of", m->store(o->parent(), map));
	}
	QWidget *widget = dynamic_cast<QWidget *>(o);
	if (widget) {
	    if (widget->layout()) {
		pod.setProperty(0, "layout", m->store(widget->layout(), map));
	    }
	}
    }
};

extern int
testQtWidgets(int argc, char **argv)
{
    QApplication app(argc, argv);

    QString infile = "file:test-qt-widgets.ttl";
    if (argc > 1) {
	infile = QString("file:%1").arg(argv[1]);
    }

    BasicStore store;
    store.import(infile, BasicStore::ImportIgnoreDuplicates);

    ObjectBuilder *b = ObjectBuilder::getInstance();
    b->registerClass<QMainWindow>();
    b->registerClass<QFrame, QWidget>();
    b->registerClass<QLabel, QWidget>();
    b->registerClass<QGridLayout, QWidget>();
    b->registerClass<QVBoxLayout, QWidget>();
    b->registerClass<QMenu, QWidget>();
    b->registerClass<QMenuBar, QWidget>();
    b->registerClass<QAction, QObject>();

    TypeMapping mapping;
    mapping.setObjectTypePrefix(qtypePrefix);
    mapping.setPropertyPrefix(qtypePrefix);
    mapping.setRelationshipPrefix(dqPrefix);

    ObjectLoader oloader(&store);
    oloader.setTypeMapping(mapping);
    LayoutLoader loader;
    oloader.addLoadCallback(&loader, ObjectLoader::FinalCallback);

    std::cerr << "about to load all objects..." << std::endl;

    QObjectList objects = oloader.loadAll();
    
    QMainWindow *mw = 0;
    if (!mw) {
	foreach (QObject *o, objects) {
	    std::cerr << "child: " << o->metaObject()->className() << std::endl;
	    QMainWindow *hmw = qobject_cast<QMainWindow *>(o);
	    if (hmw) {
		mw = hmw;
		std::cerr << "showing main window (it is " << hmw << ", object name \"" << hmw->objectName().toStdString() << "\")" << std::endl;
	    }
        }
    }
    if (mw) mw->show();
    else {
	std::cerr << "no main window! nothing will appear to happen!" << std::endl;
    }

    BasicStore store2;
    store2.setBaseUri(store.getBaseUri());
    store2.addPrefix("dq", dqPrefix);
    store2.addPrefix("qtype", qtypePrefix);
    ObjectStorer ostorer(&store2);
    ostorer.setTypeMapping(mapping);
    ostorer.setPropertyStorePolicy(ObjectStorer::StoreIfChanged);
    LayoutStorer storer;
    ostorer.addStoreCallback(&storer);
    ostorer.setFollowPolicy(ObjectStorer::FollowObjectProperties |
			    ObjectStorer::FollowSiblings |
			    ObjectStorer::FollowParent |
			    ObjectStorer::FollowChildren);

    std::cerr << "about to store all objects..." << std::endl;

    ostorer.store(objects);

    std::cerr << "about to save resulting store..." << std::endl;

    store2.save("test-qt-widgets-out.ttl");

    return app.exec();
}

}
}

int
main(int argc, char **argv)
{
    if (!Dataquay::Test::testQtWidgets(argc, argv)) return 1;

    std::cerr << "testQtWidgets successfully completed" << std::endl;
    return 0;
}
