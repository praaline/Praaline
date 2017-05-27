/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

/*
    Sonic Visualiser
    An audio file viewer and annotation editor.
    Centre for Digital Music, Queen Mary, University of London.
    This file copyright 2006 Chris Cannam.
    
    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of the
    License, or (at your option) any later version.  See the file
    COPYING included with this distribution for more information.
*/

#include "Command.h"
#include <QCoreApplication>

UndoableMacroCommand::UndoableMacroCommand(QString name) :
    m_name(name)
{
}

UndoableMacroCommand::~UndoableMacroCommand()
{
    for (size_t i = 0; i < m_commands.size(); ++i) {
	delete m_commands[i];
    }
}

void
UndoableMacroCommand::addCommand(UndoableCommand *command)
{
    m_commands.push_back(command);
}

void
UndoableMacroCommand::deleteCommand(UndoableCommand *command)
{
    for (std::vector<UndoableCommand *>::iterator i = m_commands.begin();
	 i != m_commands.end(); ++i) {

	if (*i == command) {
	    m_commands.erase(i);
	    delete command;
	    return;
	}
    }
}

bool
UndoableMacroCommand::haveCommands() const
{
    return !m_commands.empty();
}

void
UndoableMacroCommand::execute()
{
    for (size_t i = 0; i < m_commands.size(); ++i) {
	m_commands[i]->execute();
    }
}

void
UndoableMacroCommand::unexecute()
{
    for (size_t i = 0; i < m_commands.size(); ++i) {
	m_commands[m_commands.size() - i - 1]->unexecute();
    }
}

QString
UndoableMacroCommand::getName() const
{
    return m_name;
}

void
UndoableMacroCommand::setName(QString name)
{
    m_name = name;
}

BundleCommand::BundleCommand(QString name) :
    UndoableMacroCommand(name)
{
}

BundleCommand::~BundleCommand()
{
}

QString
BundleCommand::getName() const
{
    if (m_commands.size() == 1) return m_name;
    return tr("%1 (%n change(s))", "", int(m_commands.size())).arg(m_name);
}

