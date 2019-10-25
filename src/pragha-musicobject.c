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

#include "pragha-musicobject.h"

struct _PraghaMusicobjectPrivate
{
	gchar *file;
	PraghaMusicSource source;
	gchar *provider;
	gchar *mime_type;
	gchar *title;
	gchar *artist;
	gchar *album;
	gchar *genre;
	gchar *comment;
	guint year;
	guint track_no;
	gint length;
	gint bitrate;
	gint channels;
	gint samplerate;
};

G_DEFINE_TYPE_WITH_PRIVATE(PraghaMusicobject, pragha_musicobject, G_TYPE_OBJECT)

enum
{
	PROP_0,
	PROP_FILE,
	PROP_SOURCE,
	PROP_PROVIDER,
	PROP_MIME_TYPE,
	PROP_TITLE,
	PROP_ARTIST,
	PROP_ALBUM,
	PROP_GENRE,
	PROP_COMMENT,
	PROP_YEAR,
	PROP_TRACK_NO,
	PROP_LENGTH,
	PROP_BITRATE,
	PROP_CHANNELS,
	PROP_SAMPLERATE,
	LAST_PROP
};

static GParamSpec *gParamSpecs[LAST_PROP];

/**
 * pragha_musicobject_new:
 *
 */
PraghaMusicobject *
pragha_musicobject_new (void)
{
	return g_object_new (PRAGHA_TYPE_MUSICOBJECT, NULL);
}

/**
 * pragha_musicobject_dup:
 *
 */
PraghaMusicobject *
pragha_musicobject_dup (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return g_object_new (PRAGHA_TYPE_MUSICOBJECT,
	                     "file", pragha_musicobject_get_file(musicobject),
	                     "source", pragha_musicobject_get_source (musicobject),
	                     "provider", pragha_musicobject_get_provider (musicobject),
	                     "mime-type", pragha_musicobject_get_mime_type(musicobject),
	                     "title", pragha_musicobject_get_title(musicobject),
	                     "artist", pragha_musicobject_get_artist(musicobject),
	                     "album", pragha_musicobject_get_album(musicobject),
	                     "genre", pragha_musicobject_get_genre(musicobject),
	                     "comment", pragha_musicobject_get_comment(musicobject),
	                     "year", pragha_musicobject_get_year(musicobject),
	                     "track-no", pragha_musicobject_get_track_no(musicobject),
	                     "length", pragha_musicobject_get_length(musicobject),
	                     "bitrate", pragha_musicobject_get_bitrate(musicobject),
	                     "channels", pragha_musicobject_get_channels(musicobject),
	                     "samplerate", pragha_musicobject_get_samplerate(musicobject),
	                     NULL);
}

/**
 * pragha_musicobject_clean:
 *
 */
void
pragha_musicobject_clean (PraghaMusicobject *musicobject)
{
	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	g_object_set (musicobject,
	              "file", "",
	              "source", FILE_NONE,
	              "provider", "",
	              "mime-type", "",
	              "title", "",
	              "artist", "",
	              "album", "",
	              "genre", "",
	              "comment", "",
	              "year", 0,
	              "track-no", 0,
	              "length", 0,
	              "bitrate", 0,
	              "channels", 0,
	              "samplerate", 0,
	              NULL);
}

/**
 * pragha_musicobject_compare:
 *
 */
gint
pragha_musicobject_compare (PraghaMusicobject *a, PraghaMusicobject *b)
{
	/* First compare the pointers */
	if(a == b)
		return 0;

	/* Then compare filenames. */
	return g_strcmp0(pragha_musicobject_get_file(a),
	                 pragha_musicobject_get_file(b));
}

/**
 * pragha_musicobject_compare_tags:
 *
 */
gint
pragha_musicobject_compare_tags (PraghaMusicobject *a, PraghaMusicobject *b)
{
	gint diff = 0;

	diff = g_strcmp0(pragha_musicobject_get_mime_type(a),
	                 pragha_musicobject_get_mime_type(b));
	if (diff) return diff;

	diff = g_strcmp0(pragha_musicobject_get_title(a),
	                 pragha_musicobject_get_title(b));
	if (diff) return diff;

	diff = g_strcmp0(pragha_musicobject_get_artist(a),
	                 pragha_musicobject_get_artist(b));
	if (diff) return diff;

	diff = g_strcmp0(pragha_musicobject_get_album(a),
	                 pragha_musicobject_get_album(b));
	if (diff) return diff;

	diff = g_strcmp0(pragha_musicobject_get_genre(a),
	                 pragha_musicobject_get_genre(b));
	if (diff) return diff;

	diff = g_strcmp0(pragha_musicobject_get_comment(a),
	                 pragha_musicobject_get_comment(b));
	if (diff) return diff;

	diff = pragha_musicobject_get_year(a) - pragha_musicobject_get_year(b);
	if (diff) return diff;

	diff = pragha_musicobject_get_track_no(a) - pragha_musicobject_get_track_no(b);
	if (diff) return diff;

	diff = pragha_musicobject_get_length(a) - pragha_musicobject_get_length(b);
	if (diff) return diff;

	diff = pragha_musicobject_get_bitrate(a) - pragha_musicobject_get_bitrate(b);
	if (diff) return diff;

	diff = pragha_musicobject_get_channels(a) - pragha_musicobject_get_channels(b);
	if (diff) return diff;

	diff = pragha_musicobject_get_samplerate(a) - pragha_musicobject_get_samplerate(b);
	if (diff) return diff;

	return diff;
}

/**
 * pragha_musicobject_get_file:
 *
 */
const gchar *
pragha_musicobject_get_file (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->file;
}

/**
 * pragha_musicobject_set_file:
 *
 */
void
pragha_musicobject_set_file (PraghaMusicobject *musicobject,
                             const gchar *file)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free(priv->file);
	priv->file = g_strdup(file);
}

/**
 * pragha_musicobject_is_local_file:
 *
 */
gboolean
pragha_musicobject_is_local_file (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), FALSE);

	PraghaMusicSource source = musicobject->priv->source;

	return (source == FILE_LOCAL);
}

/**
 * pragha_musicobject_get_source:
 *
 */
PraghaMusicSource
pragha_musicobject_get_source (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), FILE_NONE);

	return musicobject->priv->source;
}
/**
 * pragha_musicobject_set_source:
 *
 */
void
pragha_musicobject_set_source (PraghaMusicobject *musicobject,
                               PraghaMusicSource  source)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	priv->source = source;
}

/**
 * pragha_musicobject_get_provider:
 *
 */
const gchar *
pragha_musicobject_get_provider (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->provider;
}
/**
 * pragha_musicobject_set_provider:
 *
 */
void
pragha_musicobject_set_provider (PraghaMusicobject *musicobject,
                                 const gchar       *provider)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free(priv->provider);
	priv->provider = g_strdup(provider);
}


/**
 * pragha_musicobject_get_mime_type:
 *
 */
const gchar *
pragha_musicobject_get_mime_type (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->mime_type;
}
/**
 * pragha_musicobject_set_mime_type:
 *
 */
void
pragha_musicobject_set_mime_type (PraghaMusicobject *musicobject,
                                  const gchar       *mime_type)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free (priv->mime_type);
	priv->mime_type = g_strdup(mime_type);
}

/**
 * pragha_musicobject_get_title:
 *
 */
const gchar *
pragha_musicobject_get_title (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->title;
}
/**
 * pragha_musicobject_set_title:
 *
 */
void
pragha_musicobject_set_title (PraghaMusicobject *musicobject,
                              const gchar *title)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free(priv->title);
	priv->title = g_strdup(title);
}

/**
 * pragha_musicobject_get_artist:
 *
 */
const gchar *
pragha_musicobject_get_artist (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->artist;
}
/**
 * pragha_musicobject_set_artist:
 *
 */
void
pragha_musicobject_set_artist (PraghaMusicobject *musicobject,
                               const gchar *artist)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free(priv->artist);
	priv->artist = g_strdup(artist);
}

/**
 * pragha_musicobject_get_album:
 *
 */
const gchar *
pragha_musicobject_get_album (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->album;
}
/**
 * pragha_musicobject_set_album:
 *
 */
void
pragha_musicobject_set_album (PraghaMusicobject *musicobject,
                              const gchar *album)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free(priv->album);
	priv->album = g_strdup(album);
}

/**
 * pragha_musicobject_get_genre:
 *
 */
const gchar *
pragha_musicobject_get_genre (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->genre;
}
/**
 * pragha_musicobject_set_genre:
 *
 */
void
pragha_musicobject_set_genre (PraghaMusicobject *musicobject,
                              const gchar *genre)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free(priv->genre);
	priv->genre = g_strdup(genre);
}

/**
 * pragha_musicobject_get_comment:
 *
 */
const gchar *
pragha_musicobject_get_comment (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), NULL);

	return musicobject->priv->comment;
}
/**
 * pragha_musicobject_set_comment:
 *
 */
void
pragha_musicobject_set_comment (PraghaMusicobject *musicobject,
                                const gchar *comment)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	g_free(priv->comment);
	priv->comment = g_strdup(comment);
}

/**
 * pragha_musicobject_get_year:
 *
 */
guint
pragha_musicobject_get_year (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), 0);

	return musicobject->priv->year;
}
/**
 * pragha_musicobject_set_year:
 *
 */
void
pragha_musicobject_set_year (PraghaMusicobject *musicobject,
                             guint year)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	priv->year = year;
}

/**
 * pragha_musicobject_get_track_no:
 *
 */
guint
pragha_musicobject_get_track_no (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), 0);

	return musicobject->priv->track_no;
}
/**
 * pragha_musicobject_set_track_no:
 *
 */
void
pragha_musicobject_set_track_no (PraghaMusicobject *musicobject,
                                 guint track_no)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	priv->track_no = track_no;
}

/**
 * pragha_musicobject_get_length:
 *
 */
gint
pragha_musicobject_get_length (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), 0);

	return musicobject->priv->length;
}
/**
 * pragha_musicobject_set_length:
 *
 */
void
pragha_musicobject_set_length (PraghaMusicobject *musicobject,
                               gint length)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	priv->length = length;
}

/**
 * pragha_musicobject_get_bitrate:
 *
 */
gint
pragha_musicobject_get_bitrate (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), 0);

	return musicobject->priv->bitrate;
}
/**
 * pragha_musicobject_set_bitrate:
 *
 */
void
pragha_musicobject_set_bitrate (PraghaMusicobject *musicobject,
                                gint bitrate)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	priv->bitrate = bitrate;
}

/**
 * pragha_musicobject_get_channels:
 *
 */
gint
pragha_musicobject_get_channels (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), 0);

	return musicobject->priv->channels;
}
/**
 * pragha_musicobject_set_channels:
 *
 */
void
pragha_musicobject_set_channels (PraghaMusicobject *musicobject,
                                 gint channels)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	priv->channels = channels;
}

/**
 * pragha_musicobject_get_samplerate:
 *
 */
gint
pragha_musicobject_get_samplerate (PraghaMusicobject *musicobject)
{
	g_return_val_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject), 0);

	return musicobject->priv->samplerate;
}
/**
 * pragha_musicobject_set_samplerate:
 *
 */
void
pragha_musicobject_set_samplerate (PraghaMusicobject *musicobject,
                                   gint samplerate)
{
	PraghaMusicobjectPrivate *priv;

	g_return_if_fail(PRAGHA_IS_MUSICOBJECT(musicobject));

	priv = musicobject->priv;

	priv->samplerate = samplerate;
}

static void
pragha_musicobject_finalize (GObject *object)
{
	PraghaMusicobjectPrivate *priv;

	priv = PRAGHA_MUSICOBJECT(object)->priv;

	g_free(priv->file);
	g_free(priv->mime_type);
	g_free(priv->provider);
	g_free(priv->title);
	g_free(priv->artist);
	g_free(priv->album);
	g_free(priv->genre);
	g_free(priv->comment);

	G_OBJECT_CLASS(pragha_musicobject_parent_class)->finalize(object);
}

static void
pragha_musicobject_get_property (GObject *object,
                                 guint prop_id,
                                 GValue *value,
                                 GParamSpec *pspec)
{
	PraghaMusicobject *musicobject = PRAGHA_MUSICOBJECT(object);

	switch (prop_id) {
	case PROP_FILE:
		g_value_set_string (value, pragha_musicobject_get_file(musicobject));
		break;
	case PROP_SOURCE:
		g_value_set_int(value, pragha_musicobject_get_source(musicobject));
		break;
	case PROP_PROVIDER:
		g_value_set_string (value, pragha_musicobject_get_provider(musicobject));
		break;
	case PROP_MIME_TYPE:
		g_value_set_string (value, pragha_musicobject_get_mime_type(musicobject));
		break;
	case PROP_TITLE:
		g_value_set_string (value, pragha_musicobject_get_title(musicobject));
		break;
	case PROP_ARTIST:
		g_value_set_string (value, pragha_musicobject_get_artist(musicobject));
		break;
	case PROP_ALBUM:
		g_value_set_string (value, pragha_musicobject_get_album(musicobject));
		break;
	case PROP_GENRE:
		g_value_set_string (value, pragha_musicobject_get_genre(musicobject));
		break;
	case PROP_COMMENT:
		g_value_set_string (value, pragha_musicobject_get_comment(musicobject));
		break;
	case PROP_YEAR:
		g_value_set_uint (value, pragha_musicobject_get_year(musicobject));
		break;
	case PROP_TRACK_NO:
		g_value_set_uint (value, pragha_musicobject_get_track_no(musicobject));
		break;
	case PROP_LENGTH:
		g_value_set_int(value, pragha_musicobject_get_length(musicobject));
		break;
	case PROP_BITRATE:
		g_value_set_int(value, pragha_musicobject_get_bitrate(musicobject));
		break;
	case PROP_CHANNELS:
		g_value_set_int(value, pragha_musicobject_get_channels(musicobject));
		break;
	case PROP_SAMPLERATE:
		g_value_set_int(value, pragha_musicobject_get_samplerate(musicobject));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
	}
}

static void
pragha_musicobject_set_property (GObject *object,
                                 guint prop_id,
                                 const GValue *value,
                                 GParamSpec *pspec)
{
	PraghaMusicobject *musicobject = PRAGHA_MUSICOBJECT(object);

	switch (prop_id) {
	case PROP_FILE:
		pragha_musicobject_set_file(musicobject, g_value_get_string(value));
		break;
	case PROP_SOURCE:
		pragha_musicobject_set_source(musicobject, g_value_get_int(value));
		break;
	case PROP_PROVIDER:
		pragha_musicobject_set_provider(musicobject, g_value_get_string(value));
		break;
	case PROP_MIME_TYPE:
		pragha_musicobject_set_mime_type(musicobject, g_value_get_string(value));
		break;
	case PROP_TITLE:
		pragha_musicobject_set_title(musicobject, g_value_get_string(value));
		break;
	case PROP_ARTIST:
		pragha_musicobject_set_artist(musicobject, g_value_get_string(value));
		break;
	case PROP_ALBUM:
		pragha_musicobject_set_album(musicobject, g_value_get_string(value));
		break;
	case PROP_GENRE:
		pragha_musicobject_set_genre(musicobject, g_value_get_string(value));
		break;
	case PROP_COMMENT:
		pragha_musicobject_set_comment(musicobject, g_value_get_string(value));
		break;
	case PROP_YEAR:
		pragha_musicobject_set_year(musicobject, g_value_get_uint(value));
		break;
	case PROP_TRACK_NO:
		pragha_musicobject_set_track_no(musicobject, g_value_get_uint(value));
		break;
	case PROP_LENGTH:
		pragha_musicobject_set_length(musicobject, g_value_get_int(value));
		break;
	case PROP_BITRATE:
		pragha_musicobject_set_bitrate(musicobject, g_value_get_int(value));
		break;
	case PROP_CHANNELS:
		pragha_musicobject_set_channels(musicobject, g_value_get_int(value));
		break;
	case PROP_SAMPLERATE:
		pragha_musicobject_set_samplerate(musicobject, g_value_get_int(value));
		break;
	default:
		G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
   }
}

static void
pragha_musicobject_class_init (PraghaMusicobjectClass *klass)
{
	GObjectClass *object_class;

	object_class = G_OBJECT_CLASS(klass);
	object_class->finalize = pragha_musicobject_finalize;
	object_class->get_property = pragha_musicobject_get_property;
	object_class->set_property = pragha_musicobject_set_property;

	/**
	  * PraghaMusicobject:file:
	  *
	  */
	gParamSpecs[PROP_FILE] =
		g_param_spec_string("file",
		                    "File",
		                    "The File",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:source:
	  *
	  */
	gParamSpecs[PROP_SOURCE] =
		g_param_spec_int ("source",
		                  "Source",
		                  "Source of file",
		                  FILE_HTTP,
		                  FILE_USER_L,
		                  FILE_NONE,
		                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	  * PraghaMusicobject:provider:
	  *
	  */
	gParamSpecs[PROP_PROVIDER] =
		g_param_spec_string("provider",
		                    "Provider",
		                    "The Provider",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:mime_type:
	  *
	  */
	gParamSpecs[PROP_MIME_TYPE] =
		g_param_spec_string("mime-type",
		                    "MimeType",
		                    "The MimeType",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:title:
	  *
	  */
	gParamSpecs[PROP_TITLE] =
		g_param_spec_string("title",
		                    "Title",
		                    "The Title",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:artist:
	  *
	  */
	gParamSpecs[PROP_ARTIST] =
		g_param_spec_string("artist",
		                    "Artist",
		                    "The Artist",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:album:
	  *
	  */
	gParamSpecs[PROP_ALBUM] =
		g_param_spec_string("album",
		                    "Album",
		                    "The Album",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:genre:
	  *
	  */
	gParamSpecs[PROP_GENRE] =
		g_param_spec_string("genre",
		                    "Genre",
		                    "The Genre",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:comment:
	  *
	  */
	gParamSpecs[PROP_COMMENT] =
		g_param_spec_string("comment",
		                    "Comment",
		                    "The Comment",
		                    "",
		                    PRAGHA_MUSICOBJECT_PARAM_STRING);

	/**
	  * PraghaMusicobject:year:
	  *
	  */
	gParamSpecs[PROP_YEAR] =
		g_param_spec_uint ("year",
		                   "Year",
		                   "The Year",
		                   0,
		                   G_MAXUINT,
		                   0,
		                   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	  * PraghaMusicobject:track_no:
	  *
	  */
	gParamSpecs[PROP_TRACK_NO] =
		g_param_spec_uint ("track-no",
		                   "TrackNo",
		                   "The Track No",
		                   0,
		                   G_MAXUINT,
		                   0,
		                   G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	  * PraghaMusicobject:length:
	  *
	  */
	gParamSpecs[PROP_LENGTH] =
		g_param_spec_int ("length",
		                  "Length",
		                  "The Length",
		                  0,
		                  G_MAXINT,
		                  0,
		                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	  * PraghaMusicobject:bitrate:
	  *
	  */
	gParamSpecs[PROP_BITRATE] =
		g_param_spec_int ("bitrate",
		                  "Bitrate",
		                  "The Bitrate",
		                  0,
		                  G_MAXINT,
		                  0,
		                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	  * PraghaMusicobject:channels:
	  *
	  */
	gParamSpecs[PROP_CHANNELS] =
		g_param_spec_int ("channels",
		                  "Channels",
		                  "The Channels",
		                  0,
		                  G_MAXINT,
		                  0,
		                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	/**
	  * PraghaMusicobject:samplerate:
	  *
	  */
	gParamSpecs[PROP_SAMPLERATE] =
		g_param_spec_int ("samplerate",
		                  "Samplerate",
		                  "The Samplerate",
		                  0,
		                  G_MAXINT,
		                  0,
		                  G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS);

	g_object_class_install_properties(object_class, LAST_PROP, gParamSpecs);
}

static void
pragha_musicobject_init (PraghaMusicobject *musicobject)
{
   musicobject->priv = G_TYPE_INSTANCE_GET_PRIVATE(musicobject,
                                                   PRAGHA_TYPE_MUSICOBJECT,
                                                   PraghaMusicobjectPrivate);
}
