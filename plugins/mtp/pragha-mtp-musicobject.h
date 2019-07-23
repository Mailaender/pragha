/*************************************************************************/
/* Copyright (C) 2012-2019 matias <mati86dl@gmail.com>                   */
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

#ifndef __PRAGHA_MTP_MUSICOBJECT_H__
#define __PRAGHA_MTP_MUSICOBJECT_H__

#include <libmtp.h>

#include "src/pragha-musicobject.h"

G_BEGIN_DECLS

LIBMTP_track_t    *mtp_track_new_from_pragha_musicobject (LIBMTP_mtpdevice_t *mtp_device, PraghaMusicobject *mobj);
PraghaMusicobject *pragha_musicobject_new_from_mtp_track (LIBMTP_track_t *track);

gint               pragha_mtp_plugin_get_track_id        (PraghaMusicobject *mobj);
gchar             *pragha_mtp_plugin_get_temp_filename   (PraghaMusicobject *mobj);
gboolean           pragha_musicobject_is_mtp_file        (PraghaMusicobject *mobj);

G_END_DECLS

#endif /* __PRAGHA_MTP_MUSICOBJECT_H__ */
