/*************************************************************************/
/* Copyright (C) 2007-2009 sujith <m.sujith@gmail.com>                   */
/* Copyright (C) 2009-2013 matias <mati86dl@gmail.com>                   */
/*                                                                       */
/* This program is free software: you can redistribute it and/or modify  */
/* it under the terms of the GNU General Public License as published by  */
/* the Free Software Foundation, either version 3 of the License, or     */
/* (at your option) any later version.                                   */
/*                                                                       */
/* This program is distributed in the hope that it will be useful,       */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of        */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         */
/* GNU General Public License for more details.                          */
/*                                                                       */
/* You should have received a copy of the GNU General Public License     */
/* along with this program.  If not, see <http://www.gnu.org/licenses/>. */
/*************************************************************************/

#include <locale.h> /* require LC_ALL */
#include <libintl.h>

#include "pragha-playback.h"
#include "pragha-library-pane.h"
#include "pragha-menubar.h"
#include "pragha-toolbar.h"
#include "pragha-lastfm.h"
#include "pragha-keybinder.h"
#include "pragha-dbus.h"
#include "pragha-window.h"
#include "pragha-notify.h"
#include "pragha-preferences-dialog.h"
#include "pragha-glyr.h"
#include "pragha-utils.h"
#include "pragha-debug.h"
#include "pragha.h"

gint debug_level;
#ifdef DEBUG
GThread *pragha_main_thread = NULL;
#endif

PraghaBackend *
pragha_get_backend(struct con_win *cwin)
{
	return cwin->backend;
}

GtkWidget *
pragha_get_window(struct con_win *cwin)
{
	return cwin->mainwindow;
}

/* FIXME: Cleanup track refs */
static void common_cleanup(struct con_win *cwin)
{
	CDEBUG(DBG_INFO, "Cleaning up");

	pragha_playback_stop(cwin);

#ifdef HAVE_LIBGLYR
	pragha_glyr_free (cwin->glyr);
#endif
	pragha_playlist_free(cwin->cplaylist);
	pragha_library_pane_free(cwin->clibrary);
	pragha_sidebar_free(cwin->sidebar);
	pragha_toolbar_free(cwin->toolbar);
	mpris_free (cwin->cmpris2);
	g_object_unref (cwin->backend);
	gui_free (cwin);
	pragha_scanner_free(cwin->scanner);
	g_object_unref(G_OBJECT(cwin->preferences));
	g_object_unref(cwin->cdbase);
#ifdef HAVE_LIBCLASTFM
	lastfm_free (cwin->clastfm);
#endif
	dbus_handlers_free (cwin);
	if (cwin->notify)
		pragha_notify_free (cwin->notify);

	if (cwin->cgnome_media_keys)
		gnome_media_keys_free (cwin->cgnome_media_keys);
#ifdef HAVE_LIBKEYBINDER
	else
		keybinder_free ();
#endif
	pragha_cdda_free ();

	g_object_unref (cwin->bar_context_menu);
	g_object_unref (cwin->systray_menu);

	g_slice_free(struct con_win, cwin);
}

void pragha_quit(struct con_win *cwin)
{
	gtk_main_quit();

	CDEBUG(DBG_INFO, "Halt.");
}

gint main(gint argc, gchar *argv[])
{
	struct con_win *cwin;

	const GBindingFlags binding_flags =
		G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL;

#ifdef DEBUG
	g_print ("debug enabled\n");
	pragha_main_thread = g_thread_self ();
#endif

	cwin = g_slice_new0(struct con_win);
#ifdef HAVE_LIBCLASTFM
	cwin->clastfm = g_slice_new0(struct con_lastfm);
#endif
	cwin->cmpris2 = pragha_mpris_new();

	debug_level = 0;

	setlocale (LC_ALL, "");
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);

#if !GLIB_CHECK_VERSION(2,31,0)
	if (!g_thread_supported())
		g_thread_init(NULL);
#endif

#if !GLIB_CHECK_VERSION(2,35,1)
	g_type_init ();
#endif

	if (init_dbus(cwin) == -1) {
		g_critical("Unable to init dbus connection");
		return -1;
	}

	if (init_dbus_handlers(cwin) == -1) {
		g_critical("Unable to initialize DBUS filter handlers");
		return -1;
	}

	if (init_options(cwin, argc, argv) == -1)
		return -1;

	/* Allow only one instance */

	if (!cwin->unique_instance)
		return 0;

	cwin->preferences = pragha_preferences_get();

	if (string_is_empty(pragha_preferences_get_installed_version(cwin->preferences)))
		cwin->first_run = TRUE;

	taglib_set_strings_unicode(TRUE);

	cwin->cdbase = pragha_database_get();
	if (pragha_database_start_successfully(cwin->cdbase) == FALSE) {
		g_critical("Unable to init music dbase");
		return -1;
	}

	if (pragha_preferences_get_show_osd (cwin->preferences))
		cwin->notify = pragha_notify_new (cwin);
	else
		cwin->notify = NULL;

	#ifdef HAVE_LIBCLASTFM
	if (init_lastfm(cwin) == -1) {
		g_critical("Unable to initialize lastfm");
	}
	#endif

	cwin->backend = pragha_backend_new (cwin);

	g_signal_connect (cwin->backend, "notify::state",
	                  G_CALLBACK(pragha_backend_notificate_new_state), cwin);
	g_signal_connect (cwin->backend, "finished",
	                  G_CALLBACK(pragha_backend_finished_song), cwin);
	g_signal_connect (cwin->backend, "tags-changed",
	                  G_CALLBACK(pragha_backend_tags_changed), cwin);

	g_signal_connect (cwin->backend, "error",
	                 G_CALLBACK(gui_backend_error_show_dialog_cb), cwin);
	g_signal_connect (cwin->backend, "error",
	                  G_CALLBACK(gui_backend_error_update_current_playlist_cb), cwin);

	g_signal_connect (cwin->backend, "notify::state",
	                  G_CALLBACK (pragha_menubar_update_playback_state_cb), cwin);

	g_signal_connect (cwin->backend, "tick",
	                 G_CALLBACK(pragha_toolbar_update_playback_progress), cwin);
	g_signal_connect (cwin->backend, "notify::state",
	                  G_CALLBACK(pragha_toolbar_playback_state_cb), cwin);
	g_signal_connect (cwin->backend, "buffering",
	                  G_CALLBACK(pragha_toolbar_update_buffering_cb), cwin);

	if (mpris_init(cwin) == -1) {
		g_critical("Unable to initialize MPRIS");
		return -1;
	}

	/* Init the gui after bancked to sink volume. */

	init_gui(argc, argv, cwin);

	/* Bind properties to widgets after create it. */
	g_object_bind_property (cwin->backend, "volume",
	                        pragha_toolbar_get_volume_button(cwin->toolbar), "value",
	                        binding_flags);

	#ifdef HAVE_LIBGLYR
	cwin->glyr = pragha_glyr_new (cwin);
	#endif

	/* Init_gnome_media_keys requires constructed main window. */
	if (gnome_media_keys_will_be_useful()) {
		if (init_gnome_media_keys(cwin) == -1) {
			g_critical("Unable to initialize gnome media keys");
			return -1;
		}
	}
	#ifdef HAVE_LIBKEYBINDER
	else if (init_keybinder(cwin) == -1) {
		g_critical("Unable to initialize keybinder");
		return -1;
	}
	#endif

	CDEBUG(DBG_INFO, "Init done. Running ...");

	gtk_main();

	common_cleanup(cwin);

	return 0;
}
