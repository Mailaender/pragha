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

#ifndef PRAGHA_UTILS_H
#define PRAGHA_UTILS_H

#include <gtk/gtk.h>

#include "pragha-musicobject.h"
#include "pragha-preferences.h"

#define string_is_empty(s) (!(s) || !(s)[0])
#define string_is_not_empty(s) (s && (s)[0])

gchar *pragha_unescape_html_utf75 (const gchar *str);

gchar *e2_utf8_ndup (const gchar *str, glong num);
gsize levenshtein_strcmp(const gchar * s, const gchar * t);
gsize levenshtein_safe_strcmp(const gchar * s, const gchar * t);
gchar *g_strstr_lv (gchar *haystack, gchar *needle, gsize lv_distance);
gchar *pragha_strstr_lv(gchar *haystack, gchar *needle, PraghaPreferences *preferences);

void set_watch_cursor (GtkWidget *widget);
void remove_watch_cursor (GtkWidget *widget);

GdkPixbuf * pragha_gdk_pixbuf_new_from_memory (gconstpointer data, gsize size);
gchar* convert_length_str(gint length);

gboolean
pragha_string_list_is_present (GSList *list, const gchar *str);
gboolean
pragha_string_list_is_not_present (GSList *list, const gchar *str);
GSList *
pragha_string_list_get_added (GSList *list, GSList *new_list);
GSList *
pragha_string_list_get_removed (GSList *list, GSList *new_list);

gboolean is_present_str_list(const gchar *str, GSList *list);
GSList* delete_from_str_list(const gchar *str, GSList *list);
gchar * path_get_dir_as_uri (const gchar *path);
gchar* get_display_filename(const gchar *filename, gboolean get_folder);
gchar* get_display_name(PraghaMusicobject *mobj);
void free_str_list(GSList *list);
gint compare_utf8_str(const gchar *str1, const gchar *str2);
gchar * pragha_escape_slashes (const gchar *str);
gboolean validate_album_art_pattern(const gchar *pattern);
void pragha_process_gtk_events ();
void open_url(const gchar *url, GtkWidget *parent);

void
pragha_utils_set_menu_position (GtkMenu  *menu,
                                gint     *x,
                                gint     *y,
                                gboolean *push_in,
                                gpointer  user_data);

#endif /* PRAGHA_UTILS_H */
