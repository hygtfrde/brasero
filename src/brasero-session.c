/*
 * Brasero is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Brasero is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */
 
/***************************************************************************
 *            brasero-session.c
 *
 *  Thu May 18 18:32:37 2006
 *  Copyright  2006  Philippe Rouquier
 *  <brasero-app@wanadoo.fr>
 ****************************************************************************/

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>

#include <glib.h>
#include <glib/gstdio.h>

#include <gtk/gtk.h>

#include <libxml/xmlerror.h>
#include <libxml/xmlwriter.h>
#include <libxml/parser.h>
#include <libxml/xmlstring.h>

#include "brasero-app.h"
#include "brasero-session.h"
#include "brasero-project-manager.h"

#define SESSION_VERSION "0.1"

static gchar *
brasero_session_get_path (const gchar *name)
{
	gchar *directory;
	gchar *retval;

	directory = g_build_filename (g_get_user_config_dir (),
				      "brasero",
				      NULL);
	if (!g_file_test (directory, G_FILE_TEST_EXISTS))
		g_mkdir_with_parents (directory, S_IRWXU);

	retval = g_build_filename (directory, name, NULL);
	g_free (directory);
	return retval;
}

gboolean
brasero_session_load (BraseroApp *app, gboolean load_project)
{
	gchar *height_str = NULL;
	gchar *width_str = NULL;
	gchar *state_str = NULL;
	gchar *version = NULL;
    	gchar *project_path;
	GtkWidget *manager;
	gint height;
	gint width;
	gint state = 0;

	gchar *session_path;
	xmlNodePtr item;
	xmlDocPtr session = NULL;

	GdkScreen *screen;
	GdkRectangle rect;
	gint monitor;

	/* Make sure that on first run the window has a default size of at least
	 * 85% of the screen (hardware not GTK+) */
	screen = gtk_window_get_screen (GTK_WINDOW (app));
	monitor = gdk_screen_get_monitor_at_window (screen, 
						    GTK_WIDGET (app)->window);
	gdk_screen_get_monitor_geometry (screen, monitor, &rect);
	width = rect.width / 100 * 85;
	height = rect.height / 100 * 85;

	session_path = brasero_session_get_path (BRASERO_SESSION_TMP_SESSION_PATH);
	if (!session_path)
		goto end;

	session = xmlParseFile (session_path);
	g_free (session_path);

	if (!session)
		goto end;

	item = xmlDocGetRootElement (session);
	if (!item)
		goto end;

	if (xmlStrcmp (item->name, (const xmlChar *) "Session") || item->next)
		goto end;

	item = item->children;
	while (item) {
		if (!xmlStrcmp (item->name, (const xmlChar *) "version")) {
			if (version)
				goto end;

			version = (char *) xmlNodeListGetString (session,
								 item->xmlChildrenNode,
								 1);
		}
		else if (!xmlStrcmp (item->name, (const xmlChar *) "width")) {
			if (width_str)
				goto end;

			width_str = (char *) xmlNodeListGetString (session,
								   item->xmlChildrenNode,
								   1);
		}
		else if (!xmlStrcmp (item->name, (const xmlChar *) "height")) {
			if (height_str)
				goto end;

			height_str = (char *) xmlNodeListGetString (session,
								    item->xmlChildrenNode,
								    1);
		}
		else if (!xmlStrcmp (item->name, (const xmlChar *) "state")) {
			if (state_str)
				goto end;

			state_str = (char *) xmlNodeListGetString (session,
								   item->xmlChildrenNode,
								   1);
		}
		else if (item->type == XML_ELEMENT_NODE)
			goto end;

		item = item->next;
	}

	if (!version || strcmp (version, SESSION_VERSION))
		goto end;

	/* restore the window state */
	if (height_str)
		height = (int) g_strtod (height_str, NULL);

	if (width_str)
		width = (int) g_strtod (width_str, NULL);

	if (state_str)
		state = (int) g_strtod (state_str, NULL);

end:
	if (height_str)
		g_free (height_str);

	if (width_str)
		g_free (width_str);

	if (state_str)
		g_free (state_str);

	if (version)
		g_free (version);

	xmlFreeDoc (session);

	gtk_window_resize (GTK_WINDOW (app),
			   width,
			   height);

	if (state)
		gtk_window_maximize (GTK_WINDOW (app));

	/* now we start the project if any */
	project_path = brasero_session_get_path (BRASERO_SESSION_TMP_PROJECT_PATH);
    	if (!load_project
	||  !g_file_test (project_path,G_FILE_TEST_EXISTS)) {
    		g_free (project_path);
		project_path = NULL;
	}

	manager = brasero_app_get_project_manager (app);
    	brasero_project_manager_load_session (BRASERO_PROJECT_MANAGER (manager), project_path);

    	if (project_path) {
    		/* remove the project file not to have it next time */
    		g_remove (project_path);
    		g_free (project_path);
	}

	return TRUE;
}

gboolean
brasero_session_save (BraseroApp *app,
		      gboolean save_project,
		      gboolean cancellable)
{
	gint width;
	gint height;
	gint success;
	gboolean cancel;
	GtkWidget *manager;
	gboolean maximised;
	gchar *project_path;
	gchar *session_path;
	xmlTextWriter *session;

    	if (save_project)
		project_path = brasero_session_get_path (BRASERO_SESSION_TMP_PROJECT_PATH);
	else
		project_path = NULL;

	manager = brasero_app_get_project_manager (app);
    	cancel = brasero_project_manager_save_session (BRASERO_PROJECT_MANAGER (manager),
						       project_path,
						       cancellable);
    	g_free (project_path);

	/* see if we should try to cancel shutdown */
	if (cancel)
		return TRUE;

	/* now save the state of the window */
	session_path = brasero_session_get_path (BRASERO_SESSION_TMP_SESSION_PATH);
	if (!session_path)
		return FALSE;

	/* write information */
	session = xmlNewTextWriterFilename (session_path, 0);
	if (!session) {
		g_free (session_path);
		return FALSE;
	}

	xmlTextWriterSetIndent (session, 1);
	xmlTextWriterSetIndentString (session, (xmlChar *) "\t");

	success = xmlTextWriterStartDocument (session,
					      NULL,
					      NULL,
					      NULL);
	if (success < 0)
		goto error;

	success = xmlTextWriterStartElement (session,
					     (xmlChar *) "Session");
	if (success < 0)
		goto error;

	success = xmlTextWriterWriteElement (session,
					     (xmlChar *) "version",
					     (xmlChar *) SESSION_VERSION);
	if (success < 0)
		goto error;

	brasero_app_get_geometry (app,
				  &width,
				  &height,
				  &maximised);

	success = xmlTextWriterWriteFormatElement (session,
						   (xmlChar *) "width",
						   "%i",
						   width);
	if (success < 0)
		goto error;

	success = xmlTextWriterWriteFormatElement (session,
						   (xmlChar *) "height",
						   "%i",
						   height);
	if (success < 0)
		goto error;

	success = xmlTextWriterWriteFormatElement (session,
						   (xmlChar *) "state",
						   "%i",
						   maximised);
	if (success < 0)
		goto error;

	success = xmlTextWriterEndElement (session);
	if (success < 0)
		goto error;

	xmlTextWriterEndDocument (session);
	xmlFreeTextWriter (session);
	g_free (session_path);

	return FALSE;

error:
	xmlTextWriterEndDocument (session);
	xmlFreeTextWriter (session);
	g_remove (session_path);
	g_free (session_path);

	return FALSE;
}

#ifdef BUILD_GNOME2

/**
 * This code is for session management
 */

#include <libgnomeui/gnome-client.h>

static GnomeClient *client = NULL;

static void
brasero_session_die_cb (GnomeClient *client_loc,
			BraseroApp *app)
{
	brasero_session_save (app, FALSE, FALSE);
	gtk_widget_destroy (app->mainwin);
}

static gboolean
brasero_session_save_yourself_cb (GnomeClient *client_loc,
				  gint phase,
				  GnomeSaveStyle arg2,
				  gboolean is_shutting_down,
				  GnomeInteractStyle allowed_interaction,
				  gboolean fast_shutdown,
				  BraseroApp *app)
{
    	const gint argc = 1;
    	gchar *argv [] = { 	"brasero",
				NULL	};

    	brasero_session_save (app, TRUE, FALSE);
	gnome_client_set_clone_command (client_loc,
					argc,
					argv);

    	gtk_widget_destroy (app->mainwin);
	return TRUE; /* successs */
}

gboolean
brasero_session_connect (BraseroApp *app)
{
	/* connect to the session manager */
	if (client)
		return TRUE;

	client = gnome_master_client ();
	if (client) {
		g_signal_connect (client,
				  "die",
				  G_CALLBACK (brasero_session_die_cb),
				  app);
		g_signal_connect (client,
				  "save-yourself",
				  G_CALLBACK (brasero_session_save_yourself_cb),
				  app);

		return TRUE;
	}

	return FALSE;
}

void
brasero_session_disconnect (BraseroApp *app)
{
	g_signal_handlers_disconnect_by_func (client,
					      brasero_session_die_cb,
					      app);
	g_signal_handlers_disconnect_by_func (client,
					      brasero_session_save_yourself_cb,
					      app);

	client = NULL;
}

#endif
