/*************************************************************************/
/* Copyright (C) 2009-2019 matias <mati86dl@gmail.com>                   */
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#if defined(GETTEXT_PACKAGE)
#include <glib/gi18n-lib.h>
#else
#include <glib/gi18n.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <gmodule.h>
#include <gtk/gtk.h>

#include <gudev/gudev.h>

#include <libpeas/peas.h>
#include <libpeas-gtk/peas-gtk.h>

#include "plugins/pragha-plugin-macros.h"

#include "plugins/devices/pragha-devices-plugin.h"
#include "plugins/devices/pragha-device-client.h"

#include "pragha-mtp-musicobject.h"

#include "src/pragha-database-provider.h"
#include "src/pragha-music-enum.h"
#include "src/pragha-menubar.h"
#include "src/pragha-utils.h"
#include "src/pragha-simple-widgets.h"
#include "src/pragha-background-task-bar.h"
#include "src/pragha-background-task-widget.h"
#include "src/pragha-window.h"
#include "src/pragha-hig.h"
#include "src/pragha.h"

#define PRAGHA_TYPE_MTP_PLUGIN         (pragha_mtp_plugin_get_type ())
#define PRAGHA_MTP_PLUGIN(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), PRAGHA_TYPE_MTP_PLUGIN, PraghaMtpPlugin))
#define PRAGHA_MTP_PLUGIN_CLASS(k)     (G_TYPE_CHECK_CLASS_CAST((k), PRAGHA_TYPE_MTP_PLUGIN, PraghaMtpPlugin))
#define PRAGHA_IS_MTP_PLUGIN(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), PRAGHA_TYPE_MTP_PLUGIN))
#define PRAGHA_IS_MTP_PLUGIN_CLASS(k)  (G_TYPE_CHECK_CLASS_TYPE ((k), PRAGHA_TYPE_MTP_PLUGIN))
#define PRAGHA_MTP_PLUGIN_GET_CLASS(o) (G_TYPE_INSTANCE_GET_CLASS ((o), PRAGHA_TYPE_MTP_PLUGIN, PraghaMtpPluginClass))

typedef struct _PraghaMtpPluginPrivate PraghaMtpPluginPrivate;

struct _PraghaMtpPluginPrivate {
	PraghaApplication  *pragha;

	guint64             bus_hooked;
	guint64             device_hooked;
	GUdevDevice        *u_device;
	LIBMTP_mtpdevice_t *mtp_device;

	GCancellable       *cancellable;

	gchar              *friend_name;
	gchar              *device_id;

	GHashTable         *tracks_table;

	GtkActionGroup     *action_group_menu;
	guint               merge_id_menu;

	GtkActionGroup     *action_group_playlist;
	guint               merge_id_playlist;

	GtkWidget                  *ask_dialog;
	PraghaBackgroundTaskWidget *task_widget;
};

PRAGHA_PLUGIN_REGISTER (PRAGHA_TYPE_MTP_PLUGIN,
                        PraghaMtpPlugin,
                        pragha_mtp_plugin)

/*
 * Prototypes
 */
static void
pragha_mtp_plugin_remove_menu_action (PraghaMtpPlugin *plugin);
static void
pragha_mtp_clear_hook_device (PraghaMtpPlugin *plugin);

/*
 * Menu Actions.
 */
static void pragha_mtp_action_send_to_device    (GtkAction *action, PraghaMtpPlugin *plugin);
static void pragha_mtp_action_disconnect_device (GtkAction *action, PraghaMtpPlugin *plugin);
static void pragha_mtp_action_show_device_info  (GtkAction *action, PraghaMtpPlugin *plugin);

static const GtkActionEntry mtp_sendto_actions [] = {
	{"Send to MTP", NULL, "Fake MTP device",
	 "", "Send to MTP", G_CALLBACK(pragha_mtp_action_send_to_device)},
};

static const gchar *mtp_sendto_xml = "<ui>					\
	<popup name=\"SelectionPopup\">						\
	<menu action=\"SendToMenu\">						\
		<placeholder name=\"pragha-sendto-placeholder\">		\
			<menuitem action=\"Send to MTP\"/>			\
			<separator/>						\
		</placeholder>							\
	</menu>									\
	</popup>								\
</ui>";

static const GtkActionEntry mtp_menu_actions [] = {
	{"MtpDevice", "multimedia-player", "Fake MTP device"},
	{"Disconnect device", NULL, N_("Disconnect device"),
	"", "Disconnect device", G_CALLBACK(pragha_mtp_action_disconnect_device)},
	{"Show device info", "dialog-information", N_("Show device info"),
	"", "Show device info", G_CALLBACK(pragha_mtp_action_show_device_info)},
};

static const gchar *mtp_menu_xml = "<ui>					\
	<menubar name=\"Menubar\">						\
		<menu action=\"ToolsMenu\">					\
			<placeholder name=\"pragha-plugins-placeholder\">	\
				<menu action=\"MtpDevice\">			\
					<menuitem action=\"Disconnect device\"/>	\
					<separator/>				\
					<menuitem action=\"Show device info\"/>	\
				</menu>						\
				<separator/>					\
			</placeholder>						\
		</menu>								\
	</menubar>								\
</ui>";

/*
 * Gear Menu.
 */
static void
pragha_gmenu_mtp_disconnect_device_action (GSimpleAction *action,
                                           GVariant      *parameter,
                                           gpointer       user_data)
{
	pragha_mtp_action_disconnect_device (NULL, PRAGHA_MTP_PLUGIN(user_data));
}

static void
pragha_gmenu_mtp_show_device_info_action (GSimpleAction *action,
                                          GVariant      *parameter,
                                          gpointer       user_data)
{
	pragha_mtp_action_show_device_info (NULL, PRAGHA_MTP_PLUGIN(user_data));
}

static GActionEntry mtp_entries[] = {
	{ "mtp-disconnect", pragha_gmenu_mtp_disconnect_device_action, NULL, NULL, NULL },
	{ "mtp-info",       pragha_gmenu_mtp_show_device_info_action,  NULL, NULL, NULL }
};

static const gchar *mtp_menu_ui = \
	"<interface>" \
	"	<menu id='menubar'>" \
	"		<section>" \
	"			<section id='pragha-plugins-placeholder'>"	\
	"				<submenu id='mtp-sudmenu'>" \
	"					<attribute name='label' translatable='yes'>Unknown MTP device</attribute>" \
	"					<section>" \
	"						<item>" \
	"							<attribute name='label' translatable='yes'>Disconnect library</attribute>" \
	"							<attribute name='action'>win.mtp-disconnect</attribute>" \
	"						</item>" \
	"						<item>" \
	"							<attribute name='label' translatable='yes'>Show device info</attribute>" \
	"							<attribute name='action'>win.mtp-info</attribute>" \
	"						</item>" \
	"					</section>" \
	"				</submenu>" \
	"			</section>" \
	"		</section>" \
	"	</menu>" \
	"</interface>";

/*
 * Basic Cache..
 */
static void
pragha_mtp_plugin_add_track_db (gpointer key,
                                gpointer value,
                                gpointer user_data)
{
	PraghaMusicobject *mobj = value;
	PraghaDatabase *database = user_data;

	pragha_database_add_new_musicobject (database, mobj);

	pragha_process_gtk_events ();
}

static void
pragha_mtp_save_cache (PraghaMtpPlugin *plugin)
{
	PraghaDatabase *database;
	PraghaMtpPluginPrivate *priv = plugin->priv;

	database = pragha_database_get ();
	g_hash_table_foreach (priv->tracks_table,
	                      pragha_mtp_plugin_add_track_db,
	                      database);
	g_object_unref (database);
}

static void
pragha_mtp_cache_clear (PraghaMtpPlugin *plugin)
{
	PraghaMtpPluginPrivate *priv = plugin->priv;

	g_hash_table_remove_all (priv->tracks_table);
}

static void
pragha_mtp_cache_insert_track (PraghaMtpPlugin *plugin, PraghaMusicobject *mobj)
{
	PraghaMtpPluginPrivate *priv = plugin->priv;

	const gchar *file = pragha_musicobject_get_file(mobj);

	if (string_is_empty(file))
		return;

	g_hash_table_insert (priv->tracks_table,
	                     g_strdup(file),
	                     mobj);
}

static void
pragha_mtp_plugin_cache_storage_recursive (LIBMTP_mtpdevice_t *device,
                                           guint               storageid,
                                           gint                leaf,
                                           PraghaMtpPlugin    *plugin)
{
	PraghaMusicobject *mobj = NULL;
	LIBMTP_file_t *folders = NULL, *lfolder = NULL, *audios = NULL, *laudio = NULL;
	LIBMTP_file_t *files, *file, *tmp;
	gboolean nomedia = FALSE;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	files = LIBMTP_Get_Files_And_Folders (device,
	                                      storageid,
	                                      leaf);

	if (files == NULL)
		return;

	file = files;
	while (file != NULL)
	{
		if (g_cancellable_is_cancelled (priv->cancellable))
			break;

		if (file->filetype == LIBMTP_FILETYPE_FOLDER)
		{
			if (folders == NULL)
				folders = lfolder = file;
			else {
				lfolder->next = file;
				lfolder = lfolder->next;
			}
		}
		else if (LIBMTP_FILETYPE_IS_AUDIO(file->filetype))
		{
			if (audios == NULL)
				audios = laudio = file;
			else {
				laudio->next = file;
				laudio = laudio->next;
			}
		}
		else {
			if (g_ascii_strcasecmp(file->filename, ".nomedia") == 0) {
				nomedia = TRUE;
				break;
			}
		}

		pragha_process_gtk_events ();

		file = file->next;
	}

	if (nomedia == FALSE)
	{
		/* Add folders recursively */
		file = folders;
		while (file != NULL) {
			pragha_mtp_plugin_cache_storage_recursive (device, storageid, file->item_id, plugin);
			pragha_process_gtk_events ();
			file = file->next;
		}

		/* Add music files */
		file = audios;
		while (file != NULL)
		{
			LIBMTP_track_t *track;
			track = LIBMTP_Get_Trackmetadata(device, file->item_id);
			if (G_LIKELY(track))
			{
				mobj = pragha_musicobject_new_from_mtp_track (track);
				if (G_LIKELY(mobj))
				{
					pragha_musicobject_set_provider (mobj, priv->device_id);
					pragha_mtp_cache_insert_track (plugin, mobj);
				}
				LIBMTP_destroy_track_t(track);
			}
			pragha_process_gtk_events ();
			file = file->next;
		}
	}

	/* Clean memory. */
	file = files;
	while (file != NULL)
	{
		tmp = file;
		file = file->next;
		LIBMTP_destroy_file_t(tmp);
	}
}

/*
 * Menu actions
 */
static void
pragha_mtp_action_send_to_device (GtkAction *action, PraghaMtpPlugin *plugin)
{
	PraghaPlaylist *playlist;
	PraghaDatabase *database;
	PraghaDatabaseProvider *provider;
	PraghaMusicobject *mobj = NULL;
	LIBMTP_track_t *mtp_track;
	LIBMTP_error_t *stack;
	const gchar *file;
	gint ret;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	playlist = pragha_application_get_playlist (priv->pragha);
	mobj = pragha_playlist_get_selected_musicobject (playlist);

	if (!mobj)
		return;

	file = pragha_musicobject_get_file (mobj);
	mtp_track = mtp_track_new_from_pragha_musicobject (priv->mtp_device, mobj);

	ret = LIBMTP_Send_Track_From_File (priv->mtp_device, file, mtp_track, NULL, NULL);

	if (ret != 0) {
		stack = LIBMTP_Get_Errorstack (priv->mtp_device);
		CDEBUG(DBG_INFO, "unable to send track: %s", stack->error_text);

		if (stack->errornumber == LIBMTP_ERROR_STORAGE_FULL) {
			CDEBUG(DBG_PLUGIN, "No space left on MTP device");
		}
		else {
			CDEBUG(DBG_PLUGIN, "Unable to send file to MTP device: %s", file);
		}

		LIBMTP_Dump_Errorstack(priv->mtp_device);
		LIBMTP_Clear_Errorstack(priv->mtp_device);
	}
	else
	{
		mobj = pragha_musicobject_new_from_mtp_track (mtp_track);
		if (G_LIKELY(mobj))
		{
			pragha_musicobject_set_provider (mobj, priv->device_id);

			database = pragha_database_get ();
			pragha_database_add_new_musicobject (database, mobj);
			g_object_unref (database);

			provider = pragha_database_provider_get ();
			pragha_provider_update_done (provider);
			g_object_unref (provider);
		}

		CDEBUG(DBG_INFO, "Added %s to MTP device", file);
	}

	LIBMTP_destroy_track_t(mtp_track);
}

static void
pragha_mtp_action_disconnect_device (GtkAction *action, PraghaMtpPlugin *plugin)
{
	PraghaDatabaseProvider *provider;
	PraghaMusicEnum *enum_map = NULL;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	/* Remove music on playlist */

	enum_map = pragha_music_enum_get ();
	pragha_music_enum_map_remove (enum_map, "MTP");
	g_object_unref (enum_map);

	/* Hide the provider but leave it as backup */

	provider = pragha_database_provider_get ();
	pragha_provider_set_visible (provider, priv->device_id, FALSE);
	pragha_provider_update_done (provider);
	g_object_unref (provider);

	/* Remove cache and clear the rest */

	pragha_mtp_plugin_remove_menu_action (plugin);
	pragha_mtp_cache_clear (plugin);
	pragha_mtp_clear_hook_device (plugin);
}

static void
pragha_mtp_action_show_device_info (GtkAction *action, PraghaMtpPlugin *plugin)
{
	PraghaHeader *header;
	GtkWidget *dialog, *table, *label;
	LIBMTP_devicestorage_t *storage;
	gchar *storage_size = NULL;
	gchar *storage_free = NULL;
	gchar *storage_string = NULL;
	guint row = 0;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	dialog = gtk_dialog_new_with_buttons (priv->friend_name,
	                                      GTK_WINDOW(pragha_application_get_window (priv->pragha)),
	                                      GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
	                                      _("_Ok"), GTK_RESPONSE_OK,
	                                      NULL);

	header = pragha_header_new ();
	pragha_header_set_title (header, priv->friend_name);
	pragha_header_set_icon_name (header, "multimedia-player");

	table = pragha_hig_workarea_table_new ();

	LIBMTP_Get_Storage (priv->mtp_device, LIBMTP_STORAGE_SORTBY_FREESPACE);
	for (storage = priv->mtp_device->storage; storage != 0; storage = storage->next) {
		pragha_hig_workarea_table_add_section_title (table, &row, storage->StorageDescription);

		storage_free = g_format_size (storage->FreeSpaceInBytes);
		storage_size = g_format_size (storage->MaxCapacity);

		storage_string = g_strdup_printf (_("%s free of %s (%d%% used)"),
		                                  storage_free, storage_size,
		                                  (gint) ((storage->MaxCapacity - storage->FreeSpaceInBytes) * 100 / storage->MaxCapacity));

		label = gtk_label_new_with_mnemonic (storage_string);

		pragha_hig_workarea_table_add_wide_control (table, &row, label);

		g_free (storage_free);
		g_free (storage_size);
		g_free (storage_string);
	}

	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), GTK_WIDGET(header), FALSE, FALSE, 0);
	gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), table, TRUE, TRUE, 0);

	g_signal_connect (G_OBJECT(dialog), "response",
	                  G_CALLBACK(gtk_widget_destroy), NULL);

	gtk_widget_show_all (dialog);
}

/*
 * MTP plugin.
 */
static void
pragha_mtp_plugin_append_menu_action (PraghaMtpPlugin *plugin)
{
	PraghaPlaylist *playlist;
	GtkActionGroup *action_group;
	GtkAction *action;
	GActionMap *map;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	/* Menubar tools. */

	action_group = gtk_action_group_new ("PraghaMenubarMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_menu_actions,
	                              G_N_ELEMENTS (mtp_menu_actions),
	                              plugin);

	action = gtk_action_group_get_action (action_group, "MtpDevice");
	gtk_action_set_label(GTK_ACTION(action), priv->friend_name);

	priv->merge_id_menu = pragha_menubar_append_plugin_action (priv->pragha,
	                                                           action_group,
	                                                           mtp_menu_xml);
	priv->action_group_menu = action_group;

	/* Gear Menu */

	pragha_menubar_append_submenu (priv->pragha, "pragha-plugins-placeholder",
	                               mtp_menu_ui,
	                               "mtp-sudmenu",
	                               priv->friend_name,
	                               plugin);

	map = G_ACTION_MAP (pragha_application_get_window(priv->pragha));
	g_action_map_add_action_entries (G_ACTION_MAP (map),
	                                 mtp_entries, G_N_ELEMENTS(mtp_entries),
	                                 plugin);

	/* Playlist sendto */

	action_group = gtk_action_group_new ("PraghaPlaylistMtpActions");
	gtk_action_group_set_translation_domain (action_group, GETTEXT_PACKAGE);

	gtk_action_group_add_actions (action_group,
	                              mtp_sendto_actions,
	                              G_N_ELEMENTS (mtp_sendto_actions),
	                              plugin);

	action = gtk_action_group_get_action (action_group, "Send to MTP");
	gtk_action_set_label(GTK_ACTION(action), priv->friend_name);

	playlist = pragha_application_get_playlist (priv->pragha);
	priv->merge_id_playlist = pragha_playlist_append_plugin_action (playlist,
	                                                                action_group,
	                                                                mtp_sendto_xml);
	priv->action_group_playlist = action_group;
}

void
pragha_mtp_plugin_clean_source (PraghaBackend *backend, gpointer user_data)
{
	PraghaMusicobject *mobj;
	gchar *tmp_filename = NULL;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_mtp_file (mobj))
		return;

	tmp_filename = pragha_mtp_plugin_get_temp_filename(mobj);
	g_unlink (tmp_filename);
	g_free (tmp_filename);
}

static void
pragha_mtp_plugin_prepare_source (PraghaBackend *backend, gpointer user_data)
{
	PraghaMusicobject *mobj;
	gchar *tmp_filename = NULL, *uri = NULL;
	gint track_id, ret = -1;

	PraghaMtpPlugin *plugin = user_data;
	PraghaMtpPluginPrivate *priv = plugin->priv;

	mobj = pragha_backend_get_musicobject (backend);
	if (!pragha_musicobject_is_mtp_file (mobj))
		return;

	tmp_filename = pragha_mtp_plugin_get_temp_filename(mobj);
	track_id = pragha_mtp_plugin_get_track_id(mobj);

	ret = LIBMTP_Get_Track_To_File (priv->mtp_device,
	                                track_id, tmp_filename,
	                                NULL, NULL);

	if (ret == 0) {
		uri = g_filename_to_uri (tmp_filename, NULL, NULL);
		pragha_backend_set_playback_uri (backend, uri);
		g_free(uri);
	}
	g_free (tmp_filename);
}


static void
pragha_mtp_plugin_remove_menu_action (PraghaMtpPlugin *plugin)
{
	PraghaPlaylist *playlist;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	if (!priv->merge_id_menu)
		return;

	pragha_menubar_remove_plugin_action (priv->pragha,
	                                     priv->action_group_menu,
	                                     priv->merge_id_menu);
	priv->merge_id_menu = 0;

	if (!priv->merge_id_playlist)
		return;

	playlist = pragha_application_get_playlist (priv->pragha);
	pragha_playlist_remove_plugin_action (playlist,
	                                      priv->action_group_playlist,
	                                      priv->merge_id_playlist);
	priv->merge_id_playlist = 0;

	pragha_menubar_remove_by_id (priv->pragha,
	                             "pragha-plugins-placeholder",
	                             "mtp-sudmenu");
}

static void
pragha_mtp_plugin_cache_tracks (PraghaMtpPlugin *plugin)
{
	PraghaBackgroundTaskBar *taskbar;
	PraghaDatabaseProvider *provider;
	LIBMTP_devicestorage_t *storage;
	GSList *provider_list = NULL;
	gchar *description = NULL;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	provider = pragha_database_provider_get ();
	provider_list = pragha_database_provider_get_list_by_type (provider, "MTP");

	if (pragha_string_list_is_not_present (provider_list, priv->device_id))
	{
		/* Add the taks manager */

		taskbar = pragha_background_task_bar_get ();
		pragha_background_task_bar_prepend_widget (taskbar, GTK_WIDGET(priv->task_widget));
		g_object_unref(G_OBJECT(taskbar));

		description = g_strdup_printf ("%s: %s",
		                               _("Searching files to analyze"),
		                               priv->friend_name);
		pragha_background_task_widget_set_description (priv->task_widget, description);
		g_free (description);
		pragha_background_task_widget_set_job_progress (priv->task_widget, 0);

		/* Get music recursively */

		for (storage = priv->mtp_device->storage; storage != 0; storage = storage->next)
		{
			if (g_cancellable_is_cancelled (priv->cancellable))
				break;

			pragha_mtp_plugin_cache_storage_recursive (priv->mtp_device, storage->id, 0xffffffff, plugin);
		}

		if (!g_cancellable_is_cancelled (priv->cancellable))
		{
			/* Add provider */

			pragha_provider_add_new (provider,
			                         priv->device_id,
			                         "MTP",
			                         priv->friend_name,
			                         "multimedia-player");

			/* Save on database and clear cache */

			pragha_mtp_save_cache (plugin);
		}
		else
		{
			g_cancellable_reset (priv->cancellable);
		}

		/* Clear cache */

		pragha_mtp_cache_clear (plugin);

		/* Remove task widget */

		taskbar = pragha_background_task_bar_get ();
		pragha_background_task_bar_remove_widget (taskbar, GTK_WIDGET(priv->task_widget));
		g_object_unref(G_OBJECT(taskbar));

	}
	g_slist_free_full (provider_list, g_free);

	/* Update library view. */

	pragha_provider_set_visible (provider, priv->device_id, TRUE);
	pragha_provider_update_done (provider);
	g_object_unref (provider);
}

static void
pragha_mtp_clear_hook_device (PraghaMtpPlugin *plugin)
{
	PraghaMtpPluginPrivate *priv = plugin->priv;

	if (priv->bus_hooked)
		priv->bus_hooked = 0;
	if (priv->device_hooked)
		priv->device_hooked = 0;

	if (priv->u_device) {
		g_object_unref (priv->u_device);
		priv->u_device = NULL;
	}
	if (priv->mtp_device) {
		LIBMTP_Release_Device (priv->mtp_device);
		priv->mtp_device = NULL;
	}

	if (priv->friend_name) {
		g_free (priv->friend_name);
		priv->friend_name = NULL;
	}
	if (priv->device_id) {
		g_free (priv->device_id);
		priv->device_id = NULL;
	}
}

static void
pragha_mtp_detected_ask_action_response (GtkWidget *dialog,
                                         gint       response,
                                         gpointer   user_data)
{
	PraghaMtpPlugin *plugin = user_data;

	/* Destroy first due cache track is slow in main thread */
	gtk_widget_destroy (dialog);

	switch (response)
	{
		case PRAGHA_DEVICE_RESPONSE_PLAY:
			pragha_mtp_plugin_cache_tracks (plugin);
			pragha_mtp_plugin_append_menu_action (plugin);
			break;
		case PRAGHA_DEVICE_RESPONSE_NONE:
		default:
			pragha_mtp_clear_hook_device (plugin);
			break;
	}
}

static void
pragha_mtp_detected_ask_action (PraghaMtpPlugin *plugin)
{
	GtkWidget *dialog;

	PraghaMtpPluginPrivate *priv = plugin->priv;

	dialog = pragha_gudev_dialog_new (pragha_application_get_window (priv->pragha),
	                                  _("MTP Device"), "multimedia-player",
	                                  _("Was inserted an MTP Device"), NULL,
	                                  _("Append songs of device"), PRAGHA_DEVICE_RESPONSE_PLAY);

	g_signal_connect (G_OBJECT (dialog), "response",
	                  G_CALLBACK (pragha_mtp_detected_ask_action_response), plugin);

	priv->ask_dialog = dialog;

	gtk_widget_show_all (dialog);
}

static void
pragha_mtp_plugin_device_added (PraghaDeviceClient *device_client,
                                PraghaDeviceType    device_type,
                                GUdevDevice        *u_device,
                                gpointer            user_data)
{
	LIBMTP_raw_device_t *device_list, *raw_device = NULL;
	LIBMTP_mtpdevice_t *mtp_device;
	LIBMTP_devicestorage_t *storage;
	gint busnum = 0, devnum = 0, numdevs = 0, i = 0;
	guint64 freeSpace = 0;

	PraghaMtpPlugin *plugin = user_data;
	PraghaMtpPluginPrivate *priv = plugin->priv;

	if (priv->mtp_device != NULL)
		return;

	if (device_type != PRAGHA_DEVICE_MTP)
		return;

	/* Get devices.. */

	if (LIBMTP_Detect_Raw_Devices (&device_list, &numdevs) != LIBMTP_ERROR_NONE)
		return;

	busnum = g_udev_device_get_property_as_int (u_device, "BUSNUM");
	devnum = pragha_gudev_get_property_as_int (u_device, "DEVNUM", 10);

	for (i = 0; i < numdevs; i++) {
		if (device_list[i].bus_location == busnum &&
		    device_list[i].devnum == devnum) {
			raw_device = &device_list[i];
			break;
		}
	}

	if (!raw_device) {
		g_warning("No mach any mtp device with bus, testing first.");
		raw_device = &device_list[0];
	}

	if (!raw_device) {
		g_free (device_list);
		return;
	}

	mtp_device = LIBMTP_Open_Raw_Device_Uncached (raw_device);
	if (!LIBMTP_Get_Storage (mtp_device, LIBMTP_STORAGE_SORTBY_FREESPACE)) {
		LIBMTP_Dump_Errorstack (mtp_device);
		LIBMTP_Clear_Errorstack (mtp_device);
	}

	for (storage = mtp_device->storage; storage != 0; storage = storage->next) {
		freeSpace += storage->FreeSpaceInBytes;
	}

	if (!freeSpace) {
		LIBMTP_Release_Device (mtp_device);
		return;
	}

	/* Hook device */

	priv->bus_hooked = busnum;
	priv->device_hooked = devnum;
	priv->u_device = g_object_ref (u_device);
	priv->mtp_device = mtp_device;

	priv->friend_name = LIBMTP_Get_Friendlyname (priv->mtp_device);
	if (!priv->friend_name)
		priv->friend_name = LIBMTP_Get_Modelname (priv->mtp_device);
	priv->device_id = LIBMTP_Get_Serialnumber (priv->mtp_device);

	pragha_mtp_detected_ask_action (plugin);

	g_free (device_list);
}

void
pragha_mtp_plugin_device_removed (PraghaDeviceClient *device_client,
                                  PraghaDeviceType    device_type,
                                  GUdevDevice        *u_device,
                                  gpointer            user_data)
{
	PraghaDatabaseProvider *provider;
	PraghaMusicEnum *enum_map = NULL;
	guint64 busnum = 0;
	guint64 devnum = 0;

	PraghaMtpPlugin *plugin = user_data;
	PraghaMtpPluginPrivate *priv = plugin->priv;

	if (device_type != PRAGHA_DEVICE_MTP)
		return;

	if (priv->ask_dialog) {
		gtk_widget_destroy (priv->ask_dialog);
		priv->ask_dialog = NULL;
	}

	busnum = g_udev_device_get_property_as_uint64(u_device, "BUSNUM");
	devnum = pragha_gudev_get_property_as_int (u_device, "DEVNUM", 10);

	if (busnum == priv->bus_hooked && devnum == priv->device_hooked)
	{
		pragha_mtp_plugin_remove_menu_action (plugin);

		provider = pragha_database_provider_get ();
		pragha_provider_set_visible (provider, priv->device_id, FALSE);
		pragha_provider_update_done (provider);
		g_object_unref (provider);

		pragha_mtp_cache_clear (plugin);

		pragha_mtp_clear_hook_device (plugin);

		enum_map = pragha_music_enum_get ();
		pragha_music_enum_map_remove (enum_map, "MTP");
		g_object_unref (enum_map);
	}
}

static void
pragha_plugin_activate (PeasActivatable *activatable)
{
	PraghaDeviceClient *device_client;
	PraghaBackend *backend;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN (activatable);
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	priv->pragha = g_object_get_data (G_OBJECT (plugin), "object");

	priv->tracks_table = g_hash_table_new_full (g_str_hash,
	                                            g_str_equal,
	                                            g_free,
	                                            g_object_unref);

	/* New Task widget */

	priv->cancellable = g_cancellable_new ();

	priv->task_widget = pragha_background_task_widget_new (_("Searching files to analyze"),
	                                                       "multimedia-player",
	                                                       0,
	                                                       priv->cancellable);
	g_object_ref (G_OBJECT(priv->task_widget));

	/* Connect signals */

	backend = pragha_application_get_backend (priv->pragha);
	g_signal_connect (backend, "prepare-source",
	                  G_CALLBACK(pragha_mtp_plugin_prepare_source), plugin);
	g_signal_connect (backend, "clean-source",
	                  G_CALLBACK(pragha_mtp_plugin_clean_source), plugin);

	device_client = pragha_device_client_get();
	g_signal_connect (G_OBJECT(device_client), "device-added",
	                  G_CALLBACK(pragha_mtp_plugin_device_added), plugin);
	g_signal_connect (G_OBJECT(device_client), "device-removed",
	                  G_CALLBACK(pragha_mtp_plugin_device_removed), plugin);
	g_object_unref (device_client);

	LIBMTP_Init ();
}

static void
pragha_plugin_deactivate (PeasActivatable *activatable)
{
	PraghaDatabaseProvider *provider;
	PraghaDeviceClient *device_client;
	PraghaBackend *backend;

	PraghaMtpPlugin *plugin = PRAGHA_MTP_PLUGIN (activatable);
	PraghaMtpPluginPrivate *priv = plugin->priv;

	CDEBUG(DBG_PLUGIN, "Mtp plugin %s", G_STRFUNC);

	/* Remove provider if user disable the plugin or hide it */

	provider = pragha_database_provider_get ();
	if (!pragha_plugins_engine_is_shutdown(pragha_application_get_plugins_engine(priv->pragha)))
	{
		if (priv->device_id)
		{
			pragha_provider_remove (provider,
			                        priv->device_id);
			pragha_provider_update_done (provider);
		}
	}
	else
	{
		if (priv->device_id)
		{
			pragha_provider_set_visible (provider, priv->device_id, FALSE);
		}
	}
	g_object_unref (provider);

	/* Remove cache and clear the rest */

	pragha_mtp_plugin_remove_menu_action (plugin);
	pragha_mtp_cache_clear (plugin);
	pragha_mtp_clear_hook_device (plugin);

	g_hash_table_destroy (priv->tracks_table);
	g_object_unref(priv->cancellable);

	/* Disconnect signals */

	backend = pragha_application_get_backend (priv->pragha);
	g_signal_handlers_disconnect_by_func (backend, pragha_mtp_plugin_prepare_source, plugin);
	g_signal_handlers_disconnect_by_func (backend, pragha_mtp_plugin_clean_source, plugin);

	device_client = pragha_device_client_get();
	g_signal_handlers_disconnect_by_func (device_client,
	                                      pragha_mtp_plugin_device_added,
	                                      plugin);
	g_signal_handlers_disconnect_by_func (device_client,
	                                      pragha_mtp_plugin_device_removed,
	                                      plugin);
	g_object_unref (device_client);

	priv->pragha = NULL;
}
