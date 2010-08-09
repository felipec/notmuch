/* notmuch - Not much of an email program, (just index and search)
 *
 * Copyright © 2009 Carl Worth
 * Copyright © 2009 Keith Packard
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see http://www.gnu.org/licenses/ .
 *
 * Authors: Carl Worth <cworth@cworth.org>
 *	    Keith Packard <keithp@keithp.com>
 *	    Felipe Contreras <felipe.contreras@gmail.com>
 */

#include "notmuch-client.h"
#include "gmime-filter-headers.h"

static void
show_message_headers (GMimeMessage *message)
{
    GMimeStream *stream_stdout = NULL, *stream_filter = NULL;

    stream_stdout = g_mime_stream_file_new (stdout);
    if (stream_stdout) {
	g_mime_stream_file_set_owner (GMIME_STREAM_FILE (stream_stdout), FALSE);
	stream_filter = g_mime_stream_filter_new(stream_stdout);
	if (stream_filter) {
		g_mime_stream_filter_add(GMIME_STREAM_FILTER(stream_filter),
					 g_mime_filter_headers_new());
		g_mime_object_write_to_stream(GMIME_OBJECT(message), stream_filter);
		g_object_unref(stream_filter);
	}
	g_object_unref(stream_stdout);
    }
}

static int
notmuch_compose (void *ctx, notmuch_config_t *config)
{
    GMimeMessage *msg;
    const char *from_addr = NULL;
    const char *message_id, *user_agent;
    char *simple_from;

    /* The 1 means we want headers in a "pretty" order. */
    msg = g_mime_message_new (1);
    if (msg == NULL) {
	fprintf (stderr, "Out of memory\n");
	return 1;
    }

    g_mime_message_set_subject (msg, "");

    g_mime_object_set_header (GMIME_OBJECT (msg), "To", "");

    if (from_addr == NULL)
	from_addr = notmuch_config_get_user_primary_email (config);

    simple_from = talloc_strdup (ctx, from_addr);

    from_addr = talloc_asprintf (ctx, "%s <%s>",
				 notmuch_config_get_user_name (config),
				 from_addr);
    g_mime_object_set_header (GMIME_OBJECT (msg),
			      "From", from_addr);

    g_mime_object_set_header (GMIME_OBJECT (msg), "Bcc",
			      notmuch_config_get_user_primary_email (config));

    user_agent = talloc_asprintf (ctx, "notmuch %s",
				  STRINGIFY(NOTMUCH_VERSION));
    g_mime_object_set_header (GMIME_OBJECT (msg),
			      "User-Agent", user_agent);

    message_id = talloc_asprintf (ctx, "<%lu-notmuch-%s>",
				  time(NULL),
				  simple_from);
    g_mime_object_set_header (GMIME_OBJECT (msg),
			      "Message-ID", message_id);
    talloc_free (simple_from);

    show_message_headers (msg);

    g_object_unref (G_OBJECT (msg));

    return 0;
}

int
notmuch_compose_command (void *ctx, unused (int argc), unused (char *argv[]))
{
    notmuch_config_t *config;
    int ret = 0;

    config = notmuch_config_open (ctx, NULL, NULL);
    if (config == NULL)
	return 1;

    ret = notmuch_compose (ctx, config);

    return ret;
}
