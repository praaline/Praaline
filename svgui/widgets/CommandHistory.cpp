/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

/*
   This is a modified version of a source file from the Rosegarden
   MIDI and audio sequencer and notation editor, copyright 2000-2006
   Chris Cannam, distributed under the GNU General Public License.

   This file contains traces of the KCommandHistory class from the KDE
   project, copyright 2000 Werner Trobin and David Faure and
   distributed under the GNU Lesser General Public License.
*/

#include "CommandHistory.h"

#include "base/Command.h"

#include <QRegExp>
#include <QMenu>
#include <QToolBar>
#include <QString>
#include <QTimer>
#include <QAction>

#include <iostream>

#include <typeinfo>

//#define DEBUG_COMMAND_HISTORY 1

CommandHistory *CommandHistory::m_instance = 0;

CommandHistory::CommandHistory() :
    m_undoLimit(50),
    m_redoLimit(50),
    m_menuLimit(15),
    m_savedAt(0),
    m_currentCompound(0),
    m_executeCompound(false),
    m_currentBundle(0),
    m_bundling(false),
    m_bundleTimer(0),
    m_bundleTimeout(3000)
{
    m_undoAction = new QAction(QIcon(":/icons/visualiser/undo.png"), tr("&Undo"), this);
    m_undoAction->setShortcut(tr("Ctrl+Z"));
    m_undoAction->setStatusTip(tr("Undo the last editing operation"));
    connect(m_undoAction, SIGNAL(triggered()), this, SLOT(undo()));
    
    m_undoMenuAction = new QAction(QIcon(":/icons/visualiser/undo.png"), tr("&Undo"), this);
    connect(m_undoMenuAction, SIGNAL(triggered()), this, SLOT(undo()));
    
    m_undoMenu = new QMenu(tr("&Undo"));
    m_undoMenuAction->setMenu(m_undoMenu);
    connect(m_undoMenu, SIGNAL(triggered(QAction *)),
	    this, SLOT(undoActivated(QAction*)));

    m_redoAction = new QAction(QIcon(":/icons/visualiser/redo.png"), tr("Re&do"), this);
    m_redoAction->setShortcut(tr("Ctrl+Shift+Z"));
    m_redoAction->setStatusTip(tr("Redo the last operation that was undone"));
    connect(m_redoAction, SIGNAL(triggered()), this, SLOT(redo()));
    
    m_redoMenuAction = new QAction(QIcon(":/icons/visualiser/redo.png"), tr("Re&do"), this);
    connect(m_redoMenuAction, SIGNAL(triggered()), this, SLOT(redo()));

    m_redoMenu = new QMenu(tr("Re&do"));
    m_redoMenuAction->setMenu(m_redoMenu);
    connect(m_redoMenu, SIGNAL(triggered(QAction *)),
	    this, SLOT(redoActivated(QAction*)));
}

CommandHistory::~CommandHistory()
{
    m_savedAt = -1;
    clearStack(m_undoStack);
    clearStack(m_redoStack);

    delete m_undoMenu;
    delete m_redoMenu;
}

CommandHistory *
CommandHistory::getInstance()
{
    if (!m_instance) m_instance = new CommandHistory();
    return m_instance;
}

void
CommandHistory::clear()
{
#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::clear()" << endl;
#endif
    closeBundle();
    m_savedAt = -1;
    clearStack(m_undoStack);
    clearStack(m_redoStack);
    updateActions();
}

void
CommandHistory::registerMenu(QMenu *menu)
{
    menu->addAction(m_undoAction);
    menu->addAction(m_redoAction);
}

void
CommandHistory::registerToolbar(QToolBar *toolbar)
{
    toolbar->addAction(m_undoMenuAction);
    toolbar->addAction(m_redoMenuAction);
}

void
CommandHistory::addCommand(UndoableCommand *command)
{
    if (!command) return;

    if (m_currentCompound) {
	addToCompound(command, m_executeCompound);
	return;
    }

    addCommand(command, true);
}

void
CommandHistory::addCommand(UndoableCommand *command, bool execute, bool bundle)
{
    if (!command) return;

#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::addCommand: " << command->getName() << " of type " << typeid(*command).name() << " at " << command << ": execute = " << execute << ", bundle = " << bundle << " (m_currentCompound = " << m_currentCompound << ", m_currentBundle = " << m_currentBundle << ")" << endl;
#endif

    if (m_currentCompound) {
	addToCompound(command, execute);
	return;
    }

    if (bundle) {
	addToBundle(command, execute);
	return;
    } else if (m_currentBundle) {
	closeBundle();
    }

#ifdef DEBUG_COMMAND_HISTORY
    if (!m_redoStack.empty()) {
        cerr << "CommandHistory::clearing redo stack" << endl;
    }
#endif

    // We can't redo after adding a command
    clearStack(m_redoStack);

    // can we reach savedAt?
    if ((int)m_undoStack.size() < m_savedAt) m_savedAt = -1; // nope

    m_undoStack.push(command);
    clipCommands();
    
    if (execute) {
	command->execute();
    }

    // Emit even if we aren't executing the command, because
    // someone must have executed it for this to make any sense
    emit commandExecuted();
    emit commandExecuted(command);
    if (!m_bundling) emit activity(command->getName());
    
    updateActions();
}

void
CommandHistory::addToBundle(UndoableCommand *command, bool execute)
{
    if (m_currentBundle) {
	if (!command || (command->getName() != m_currentBundleName)) {
#ifdef DEBUG_COMMAND_HISTORY
            cerr << "CommandHistory::addToBundle: " << command->getName()
                 << ": closing current bundle" << endl;
#endif
	    closeBundle();
	}
    }

    if (!command) return;

    if (!m_currentBundle) {

#ifdef DEBUG_COMMAND_HISTORY
        cerr << "CommandHistory::addToBundle: " << command->getName()
             << ": creating new bundle" << endl;
#endif

	// need to addCommand before setting m_currentBundle, as addCommand
	// with bundle false will reset m_currentBundle to 0
	UndoableMacroCommand *mc = new BundleCommand(command->getName());
        m_bundling = true;
	addCommand(mc, false);
        m_bundling = false;
	m_currentBundle = mc;
	m_currentBundleName = command->getName();
    }

#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::addToBundle: " << command->getName()
         << ": adding to bundle" << endl;
#endif

    if (execute) command->execute();
    m_currentBundle->addCommand(command);

    // Emit even if we aren't executing the command, because
    // someone must have executed it for this to make any sense
    emit commandExecuted();
    emit commandExecuted(command);

    updateActions();

    delete m_bundleTimer;
    m_bundleTimer = new QTimer(this);
    connect(m_bundleTimer, SIGNAL(timeout()), this, SLOT(bundleTimerTimeout()));
    m_bundleTimer->start(m_bundleTimeout);
}

void
CommandHistory::closeBundle()
{
    if (m_currentBundle) {
#ifdef DEBUG_COMMAND_HISTORY
        cerr << "CommandHistory::closeBundle" << endl;
#endif
        emit activity(m_currentBundle->getName());
    }
    m_currentBundle = 0;
    m_currentBundleName = "";
}

void
CommandHistory::bundleTimerTimeout()
{
#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::bundleTimerTimeout: bundle is " << m_currentBundle << endl;
#endif

    closeBundle();
}

void
CommandHistory::addToCompound(UndoableCommand *command, bool execute)
{
    if (!m_currentCompound) {
	cerr << "CommandHistory::addToCompound: ERROR: no compound operation in progress!" << endl;
        return;
    }

#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::addToCompound[" << m_currentCompound->getName() << "]: " << command->getName() << " (exec: " << execute << ")" << endl;
#endif

    if (execute) command->execute();
    m_currentCompound->addCommand(command);
}

void
CommandHistory::startCompoundOperation(QString name, bool execute)
{
    if (m_currentCompound) {
	cerr << "CommandHistory::startCompoundOperation: ERROR: compound operation already in progress!" << endl;
	cerr << "(name is " << m_currentCompound->getName() << ")" << endl;
        return;
    }
 
#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::startCompoundOperation: " << name << " (exec: " << execute << ")" << endl;
#endif
   
    closeBundle();

    m_currentCompound = new UndoableMacroCommand(name);
    m_executeCompound = execute;
}

void
CommandHistory::endCompoundOperation()
{
    if (!m_currentCompound) {
	cerr << "CommandHistory::endCompoundOperation: ERROR: no compound operation in progress!" << endl;
        return;
    }
 
#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::endCompoundOperation: " << m_currentCompound->getName() << endl;
#endif

    UndoableMacroCommand *toAdd = m_currentCompound;
    m_currentCompound = 0;

    if (toAdd->haveCommands()) {

        // We don't execute the macro command here, because we have
        // been executing the individual commands as we went along if
        // m_executeCompound was true.
        addCommand(toAdd, false);
    }
}    

void
CommandHistory::addExecutedCommand(UndoableCommand *command)
{
    addCommand(command, false);
}

void
CommandHistory::addCommandAndExecute(UndoableCommand *command)
{
    addCommand(command, true);
}

void
CommandHistory::undo()
{
    if (m_undoStack.empty()) return;

#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::undo()" << endl;
#endif

    closeBundle();

    UndoableCommand *command = m_undoStack.top();
    command->unexecute();
    emit commandExecuted();
    emit commandUnexecuted(command);
    emit activity(tr("Undo %1").arg(command->getName()));

    m_redoStack.push(command);
    m_undoStack.pop();

    clipCommands();
    updateActions();

    if ((int)m_undoStack.size() == m_savedAt) emit documentRestored();
}

void
CommandHistory::redo()
{
    if (m_redoStack.empty()) return;

#ifdef DEBUG_COMMAND_HISTORY
    cerr << "CommandHistory::redo()" << endl;
#endif

    closeBundle();

    UndoableCommand *command = m_redoStack.top();
    command->execute();
    emit commandExecuted();
    emit commandExecuted(command);
    emit activity(tr("Redo %1").arg(command->getName()));

    m_undoStack.push(command);
    m_redoStack.pop();
    // no need to clip

    updateActions();

    if ((int)m_undoStack.size() == m_savedAt) emit documentRestored();
}

void
CommandHistory::setUndoLimit(int limit)
{
    if (limit > 0 && limit != m_undoLimit) {
        m_undoLimit = limit;
        clipCommands();
    }
}

void
CommandHistory::setRedoLimit(int limit)
{
    if (limit > 0 && limit != m_redoLimit) {
        m_redoLimit = limit;
        clipCommands();
    }
}

void
CommandHistory::setMenuLimit(int limit)
{
    m_menuLimit = limit;
    updateActions();
}

void
CommandHistory::setBundleTimeout(int ms)
{
    m_bundleTimeout = ms;
}

void
CommandHistory::documentSaved()
{
    closeBundle();
    m_savedAt = int(m_undoStack.size());
}

void
CommandHistory::clipCommands()
{
    if (int(m_undoStack.size()) > m_undoLimit) {
	m_savedAt -= (int(m_undoStack.size()) - m_undoLimit);
    }

    clipStack(m_undoStack, m_undoLimit);
    clipStack(m_redoStack, m_redoLimit);
}

void
CommandHistory::clipStack(CommandStack &stack, int limit)
{
    int i;

    if ((int)stack.size() > limit) {

	CommandStack tempStack;

	for (i = 0; i < limit; ++i) {
#ifdef DEBUG_COMMAND_HISTORY
	    Command *command = stack.top();
	    cerr << "CommandHistory::clipStack: Saving recent command: " << command->getName() << " at " << command << endl;
#endif
	    tempStack.push(stack.top());
	    stack.pop();
	}

	clearStack(stack);

	for (i = 0; i < m_undoLimit; ++i) {
	    stack.push(tempStack.top());
	    tempStack.pop();
	}
    }
}

void
CommandHistory::clearStack(CommandStack &stack)
{
    while (!stack.empty()) {
    UndoableCommand *command = stack.top();
	// Not safe to call getName() on a command about to be deleted
#ifdef DEBUG_COMMAND_HISTORY
	cerr << "CommandHistory::clearStack: About to delete command " << command << endl;
#endif
	delete command;
	stack.pop();
    }
}

void
CommandHistory::undoActivated(QAction *action)
{
    int pos = m_actionCounts[action];
    for (int i = 0; i <= pos; ++i) {
	undo();
    }
}

void
CommandHistory::redoActivated(QAction *action)
{
    int pos = m_actionCounts[action];
    for (int i = 0; i <= pos; ++i) {
	redo();
    }
}

void
CommandHistory::updateActions()
{
    m_actionCounts.clear();

    for (int undo = 0; undo <= 1; ++undo) {

	QAction *action(undo ? m_undoAction : m_redoAction);
	QAction *menuAction(undo ? m_undoMenuAction : m_redoMenuAction);
	QMenu *menu(undo ? m_undoMenu : m_redoMenu);
	CommandStack &stack(undo ? m_undoStack : m_redoStack);

	if (stack.empty()) {

	    QString text(undo ? tr("Nothing to undo") : tr("Nothing to redo"));

	    action->setEnabled(false);
	    action->setText(text);

	    menuAction->setEnabled(false);
	    menuAction->setText(text);

	} else {

	    action->setEnabled(true);
	    menuAction->setEnabled(true);

	    QString commandName = stack.top()->getName();
	    commandName.replace(QRegExp("&"), "");

	    QString text = (undo ? tr("&Undo %1") : tr("Re&do %1"))
		.arg(commandName);

	    action->setText(text);
	    menuAction->setText(text);
	}

	menu->clear();

	CommandStack tempStack;
	int j = 0;

	while (j < m_menuLimit && !stack.empty()) {
        UndoableCommand *command = stack.top();
	    tempStack.push(command);
	    stack.pop();

	    QString commandName = command->getName();
	    commandName.replace(QRegExp("&"), "");

	    QString text;
	    if (undo) text = tr("&Undo %1").arg(commandName);
	    else      text = tr("Re&do %1").arg(commandName);
	    
	    QAction *action = menu->addAction(text);
	    m_actionCounts[action] = j++;
	}

	while (!tempStack.empty()) {
	    stack.push(tempStack.top());
	    tempStack.pop();
	}
    }
}

