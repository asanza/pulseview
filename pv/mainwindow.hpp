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

#ifndef PULSEVIEW_PV_MAINWINDOW_HPP
#define PULSEVIEW_PV_MAINWINDOW_HPP

#include <list>
#include <map>
#include <memory>

#include <glibmm/variant.h>

#include <QMainWindow>

#include "session.hpp"
#include "view/viewwidget.hpp"

struct srd_decoder;

class QVBoxLayout;

namespace sigrok {
class InputFormat;
class OutputFormat;
}

namespace pv {

class DeviceManager;

namespace toolbars {
class ContextBar;
class MainBar;
}

namespace view {
class View;
}

namespace widgets {
#ifdef ENABLE_DECODE
class DecoderMenu;
#endif
}

class MainWindow : public QMainWindow
{
	Q_OBJECT

private:
	/**
	 * Name of the setting used to remember the directory
	 * containing the last file that was opened.
	 */
	static const char *SettingOpenDirectory;

	/**
	 * Name of the setting used to remember the directory
	 * containing the last file that was saved.
	 */
	static const char *SettingSaveDirectory;

public:
	explicit MainWindow(DeviceManager &device_manager,
		std::string open_file_name = std::string(),
		std::string open_file_format = std::string(),
		QWidget *parent = 0);

	~MainWindow();

	QAction* action_open() const;
	QAction* action_save_as() const;
	QAction* action_save_selection_as() const;
	QAction* action_connect() const;
	QAction* action_quit() const;
	QAction* action_view_zoom_in() const;
	QAction* action_view_zoom_out() const;
	QAction* action_view_zoom_fit() const;
	QAction* action_view_zoom_one_to_one() const;
	QAction* action_view_sticky_scrolling() const;
	QAction* action_view_coloured_bg() const;
	QAction* action_view_show_cursors() const;
	QAction* action_about() const;

#ifdef ENABLE_DECODE
	QMenu* menu_decoder_add() const;
#endif

	std::shared_ptr<pv::view::View> get_active_view() const;

	std::shared_ptr<pv::view::View> add_view(const QString &title,
		view::ViewType type, Session &session);

	void run_stop();

	void select_device(std::shared_ptr<devices::Device> device);

public Q_SLOTS:
	void export_file(std::shared_ptr<sigrok::OutputFormat> format,
		bool selection_only = false);
	void import_file(std::shared_ptr<sigrok::InputFormat> format);

private:
	void setup_ui();

	void select_init_device();

	void load_init_file(const std::string &file_name,
		const std::string &format);

	void save_ui_settings();

	void restore_ui_settings();

	void session_error(const QString text, const QString info_text);

	/**
	 * Updates the device list in the toolbar
	 */
	void update_device_list();	

	void load_file(QString file_name,
		std::shared_ptr<sigrok::InputFormat> format = nullptr,
		const std::map<std::string, Glib::VariantBase> &options =
			std::map<std::string, Glib::VariantBase>());

	void save_selection_to_file();

private:
	void closeEvent(QCloseEvent *event);

	void keyReleaseEvent(QKeyEvent *event);

	virtual QMenu* createPopupMenu();

	virtual bool restoreState(const QByteArray &state, int version = 0);

private Q_SLOTS:
	void show_session_error(
		const QString text, const QString info_text);

	void on_actionOpen_triggered();
	void on_actionSaveAs_triggered();
	void on_actionSaveSelectionAs_triggered();
	void on_actionQuit_triggered();

	void on_actionConnect_triggered();

	void on_actionViewZoomIn_triggered();

	void on_actionViewZoomOut_triggered();

	void on_actionViewZoomFit_triggered();

	void on_actionViewZoomOneToOne_triggered();

	void on_actionViewStickyScrolling_triggered();

	void on_actionViewColouredBg_triggered();

	void on_actionViewShowCursors_triggered();

	void on_actionAbout_triggered();

	void add_decoder(srd_decoder *decoder);

	void capture_state_changed(int state);
	void device_selected();

	void sticky_scrolling_changed(bool state);

	void always_zoom_to_fit_changed(bool state);

private:
	DeviceManager &device_manager_;

	Session session_;

	std::map< std::shared_ptr<QDockWidget>,
		std::shared_ptr<pv::view::View> > view_docks_;

	toolbars::MainBar *main_bar_;

	QAction *const action_open_;
	QAction *const action_save_as_;
	QAction *const action_save_selection_as_;
	QAction *const action_connect_;
	QAction *const action_quit_;
	QAction *const action_view_zoom_in_;
	QAction *const action_view_zoom_out_;
	QAction *const action_view_zoom_fit_;
	QAction *const action_view_zoom_one_to_one_;
	QAction *const action_view_sticky_scrolling_;
	QAction *const action_view_coloured_bg_;
	QAction *const action_view_show_cursors_;
	QAction *const action_about_;

#ifdef ENABLE_DECODE
	QMenu *const menu_decoders_add_;
#endif
};

} // namespace pv

#endif // PULSEVIEW_PV_MAINWINDOW_HPP
