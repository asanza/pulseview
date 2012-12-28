/*
 * This file is part of the PulseView project.
 *
 * Copyright (C) 2012 Joel Holdsworth <joel@airwebreathe.org.uk>
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

#ifndef PULSEVIEW_PV_PROP_BINDING_HWCAP_H
#define PULSEVIEW_PV_PROP_BINDING_HWCAP_H

#include <QString>

extern "C" {
#include <libsigrok/libsigrok.h>
}

#include "binding.h"

namespace pv {
namespace prop {
namespace binding {

class HwCap : public Binding
{
public:
	HwCap(struct sr_dev_inst *sdi);

private:
	void expose_enum(const struct sr_hwcap_option *hwo,
		const std::vector<std::pair<const void*, QString> > &values,
		int opt);

	void bind_stropt(const struct sr_hwcap_option *hwo, int id,
		int opt);

	void bind_buffer_size(const struct sr_hwcap_option *hwo);
	void bind_time_base(const struct sr_hwcap_option *hwo);
	void bind_vdiv(const struct sr_hwcap_option *hwo);

protected:
	const struct sr_dev_inst *_sdi;
};

} // binding
} // prop
} // pv

#endif // PULSEVIEW_PV_PROP_BINDING_HWCAP_H
