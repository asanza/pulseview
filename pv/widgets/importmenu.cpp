/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2013 Joel Holdsworth <joel@airwebreathe.org.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */

#include <algorithm>
#include <cassert>
#include <string>
#include <utility>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include "importmenu.hpp"

using std::find_if;
using std::map;
using std::pair;
using std::string;
using std::shared_ptr;

using sigrok::Context;
using sigrok::InputFormat;

namespace pv {
namespace widgets {

ImportMenu::ImportMenu(QWidget *parent, shared_ptr<Context> context,
	QAction *open_action) :
	QMenu(parent),
	context_(context),
	mapper_(this)
{
	assert(context);

	if (open_action) {
		addAction(open_action);
		setDefaultAction(open_action);
		addSeparator();
	}

	const map<string, shared_ptr<InputFormat> > formats =
		context->input_formats();

	for (const pair<string, shared_ptr<InputFormat> > &f : formats) {
		assert(f.second);
		QAction *const action =	addAction(tr("Import %1...")
			.arg(QString::fromStdString(f.second->description())));
		action->setData(qVariantFromValue((void*)f.second.get()));
		mapper_.setMapping(action, action);
		connect(action, SIGNAL(triggered()), &mapper_, SLOT(map()));
	}

	connect(&mapper_, SIGNAL(mapped(QObject*)),
		this, SLOT(on_action(QObject*)));
}

void ImportMenu::on_action(QObject *action)
{
	assert(action);

	const map<string, shared_ptr<InputFormat> > formats =
		context_->input_formats();
	const auto iter = find_if(formats.cbegin(), formats.cend(),
		[&](const pair<string, shared_ptr<InputFormat> > &f) {
			return f.second.get() ==
				((QAction*)action)->data().value<void*>(); });
	if (iter == formats.cend())
		return;

	format_selected((*iter).second);
}

} // widgets
} // pv
