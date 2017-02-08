/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012-2013 Joel Holdsworth <joel@airwebreathe.org.uk>
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
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#include <cassert>

#include <libsigrokcxx/libsigrokcxx.hpp>

#include <QLabel>

#include "connect.hpp"

#include <pv/devicemanager.hpp>
#include <pv/devices/hardwaredevice.hpp>

using std::list;
using std::map;
using std::shared_ptr;
using std::string;

using Glib::ustring;
using Glib::Variant;
using Glib::VariantBase;

using sigrok::ConfigKey;
using sigrok::Driver;
using sigrok::Error;

using pv::devices::HardwareDevice;

namespace pv {
namespace dialogs {

Connect::Connect(QWidget *parent, pv::DeviceManager &device_manager) :
	QDialog(parent),
	tcp_endpoint_(&form_),
	tcp_endpoint_layout_(&tcp_endpoint_),
	tcp_host_(&tcp_endpoint_),
	tcp_port_(&tcp_endpoint_),
	device_manager_(device_manager),
	layout_(this),
	form_(this),
	form_layout_(&form_),
	drivers_(&form_),
	serial_devices_(&form_),
	scan_button_(tr("&Scan for devices using driver above"), this),
	device_list_(this),
	button_box_(QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
		Qt::Horizontal, this)
{
	setWindowTitle(tr("Connect to Device"));

	connect(&button_box_, SIGNAL(accepted()), this, SLOT(accept()));
	connect(&button_box_, SIGNAL(rejected()), this, SLOT(reject()));

	populate_drivers();
	connect(&drivers_, SIGNAL(activated(int)),
		this, SLOT(device_selected(int)));

	form_.setLayout(&form_layout_);
	form_layout_.addRow(tr("&Driver"), &drivers_);

	form_layout_.addRow(tr("Serial &Port"), &serial_devices_);
	serial_devices_.setEditable(true);

	tcp_host_.setPlaceholderText("192.168.1.100");
	tcp_endpoint_layout_.addWidget(&tcp_host_);
	tcp_endpoint_layout_.addWidget(new QLabel(":"));
	tcp_port_.setRange(1, 65535);
	tcp_port_.setValue(5555);
	tcp_endpoint_layout_.addWidget(&tcp_port_);
	tcp_endpoint_layout_.setContentsMargins(0, 0, 0, 0);
	form_layout_.addRow(tr("TCP &Endpoint"), &tcp_endpoint_);

	unset_connection();

	connect(&scan_button_, SIGNAL(pressed()),
		this, SLOT(scan_pressed()));

	setLayout(&layout_);
	layout_.addWidget(&form_);
	layout_.addWidget(&scan_button_);
	layout_.addWidget(&device_list_);
	layout_.addWidget(&button_box_);
}

shared_ptr<HardwareDevice> Connect::get_selected_device() const
{
	const QListWidgetItem *const item = device_list_.currentItem();
	if (!item)
		return shared_ptr<HardwareDevice>();

	return item->data(Qt::UserRole).value<shared_ptr<HardwareDevice>>();
}

void Connect::populate_drivers()
{
	for (auto entry : device_manager_.context()->drivers()) {
		auto name = entry.first;
		auto driver = entry.second;
		/**
		 * We currently only support devices that can deliver
		 * samples at a fixed samplerate i.e. oscilloscopes and
		 * logic analysers.
		 * @todo Add support for non-monotonic devices i.e. DMMs
		 * and sensors.
		 */
		const auto keys = driver->config_keys();

		bool supported_device = keys.count(ConfigKey::LOGIC_ANALYZER) |
			keys.count(ConfigKey::OSCILLOSCOPE);

		if (supported_device)
			drivers_.addItem(QString("%1 (%2)").arg(
				driver->long_name().c_str(), name.c_str()),
				qVariantFromValue(driver));
	}
}

void Connect::populate_serials(shared_ptr<Driver> driver)
{
	serial_devices_.clear();
	for (auto serial : device_manager_.context()->serials(driver))
		serial_devices_.addItem(QString("%1 (%2)").arg(
			serial.first.c_str(), serial.second.c_str()),
			QString::fromStdString(serial.first));
}

void Connect::unset_connection()
{
	device_list_.clear();
	serial_devices_.hide();
	form_layout_.labelForField(&serial_devices_)->hide();
	tcp_endpoint_.hide();
	form_layout_.labelForField(&tcp_endpoint_)->hide();
	button_box_.button(QDialogButtonBox::Ok)->setDisabled(true);
}

void Connect::set_serial_connection(shared_ptr<Driver> driver)
{
	populate_serials(driver);
	serial_devices_.show();
	form_layout_.labelForField(&serial_devices_)->show();
}

void Connect::set_tcp_connection(shared_ptr<Driver> driver)
{
	(void)driver;

	tcp_endpoint_.show();
	form_layout_.labelForField(&tcp_endpoint_)->show();
}

void Connect::scan_pressed()
{
	device_list_.clear();

	const int index = drivers_.currentIndex();
	if (index == -1)
		return;

	shared_ptr<Driver> driver =
		drivers_.itemData(index).value<shared_ptr<Driver>>();

	assert(driver);

	map<const ConfigKey *, VariantBase> drvopts;

	if (serial_devices_.isVisible()) {
		QString serial;
		const int index = serial_devices_.currentIndex();
		if (index >= 0 && index < serial_devices_.count() &&
		    serial_devices_.currentText() == serial_devices_.itemText(index))
			serial = serial_devices_.itemData(index).value<QString>();
		else
			serial = serial_devices_.currentText();
		drvopts[ConfigKey::CONN] = Variant<ustring>::create(
			serial.toUtf8().constData());
	}

	if (tcp_endpoint_.isVisible()) {
		QString host = tcp_host_.text();
		QString port = tcp_port_.text();
		if(!host.isEmpty()) {
			QString conn = QString("tcp-raw/%1/%2").arg(host, port);
			drvopts[ConfigKey::CONN] = Variant<ustring>::create(
				conn.toUtf8().constData());
		}
	}

	const list< shared_ptr<HardwareDevice> > devices =
		device_manager_.driver_scan(driver, drvopts);

	for (shared_ptr<HardwareDevice> device : devices) {
		assert(device);

		QString text = QString::fromStdString(
			device->display_name(device_manager_));
		text += QString(" with %1 channels").arg(
			device->device()->channels().size());

		QListWidgetItem *const item = new QListWidgetItem(text,
			&device_list_);
		item->setData(Qt::UserRole, qVariantFromValue(device));
		device_list_.addItem(item);
	}

	device_list_.setCurrentRow(0);
	button_box_.button(QDialogButtonBox::Ok)->setDisabled(device_list_.count() == 0);
}

void Connect::device_selected(int index)
{
	shared_ptr<Driver> driver =
		drivers_.itemData(index).value<shared_ptr<Driver>>();

	unset_connection();

	if (driver->scan_options().count(ConfigKey::SERIALCOMM))
		set_serial_connection(driver);

	if (driver->name() == "rigol-ds") // NBNB
		set_tcp_connection(driver);
}

} // namespace dialogs
} // namespace pv
