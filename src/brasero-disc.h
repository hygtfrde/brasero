/***************************************************************************
 *            disc.h
 *
 *  dim nov 27 14:58:13 2005
 *  Copyright  2005  Rouquier Philippe
 *  brasero-app@wanadoo.fr
 ***************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DISC_H
#define DISC_H

#include <glib.h>
#include <glib-object.h>

#include <gtk/gtktoolbar.h>
#include <gtk/gtkuimanager.h>

#include "burn-basics.h"
#include "burn-session.h"

G_BEGIN_DECLS

#define BRASERO_TYPE_DISC         (brasero_disc_get_type ())
#define BRASERO_DISC(o)           (G_TYPE_CHECK_INSTANCE_CAST ((o), BRASERO_TYPE_DISC, BraseroDisc))
#define BRASERO_IS_DISC(o)        (G_TYPE_CHECK_INSTANCE_TYPE ((o), BRASERO_TYPE_DISC))
#define BRASERO_DISC_GET_IFACE(o) (G_TYPE_INSTANCE_GET_INTERFACE ((o), BRASERO_TYPE_DISC, BraseroDiscIface))

#define BRASERO_DISC_ACTION "DiscAction"


typedef enum {
	BRASERO_DISC_OK = 0,
	BRASERO_DISC_NOT_IN_TREE,
	BRASERO_DISC_NOT_READY,
	BRASERO_DISC_LOADING,
	BRASERO_DISC_BROKEN_SYMLINK,
	BRASERO_DISC_CANCELLED,
	BRASERO_DISC_ERROR_SIZE,
	BRASERO_DISC_ERROR_EMPTY_SELECTION,
	BRASERO_DISC_ERROR_FILE_NOT_FOUND,
	BRASERO_DISC_ERROR_UNREADABLE,
	BRASERO_DISC_ERROR_ALREADY_IN_TREE,
	BRASERO_DISC_ERROR_JOLIET,
	BRASERO_DISC_ERROR_FILE_TYPE,
	BRASERO_DISC_ERROR_THREAD,
	BRASERO_DISC_ERROR_UNKNOWN
} BraseroDiscResult;

typedef enum {
	BRASERO_DISC_TRACK_NONE,
	BRASERO_DISC_TRACK_AUDIO,
	BRASERO_DISC_TRACK_DATA,
} BraseroDiscTrackType;

struct _BraseroDiscSong {
	gchar *uri;
	gint64 gap;
	gint64 start;
	gint64 end;

	BraseroSongInfo *info;
};
typedef struct _BraseroDiscSong BraseroDiscSong;

typedef struct {
	BraseroDiscTrackType type;

	union  {
		struct {
			GSList *grafts;
			GSList *excluded;
			GSList *restored;
		} data;

		GSList *tracks; /* BraseroDiscSong */
	} contents;
} BraseroDiscTrack;

typedef struct _BraseroDisc        BraseroDisc;
typedef struct _BraseroDiscIface   BraseroDiscIface;

struct _BraseroDiscIface {
	GTypeInterface g_iface;

	/* signals */
	void	(*selection_changed)			(BraseroDisc *disc);
	void	(*contents_changed)			(BraseroDisc *disc,
							 gint nb_files);
	void	(*size_changed)				(BraseroDisc *disc,
							 gint64 size);
	void	(*flags_changed)			(BraseroDisc *disc,
							 BraseroBurnFlag flags);

	/* virtual functions */
	BraseroDiscResult	(*get_status)		(BraseroDisc *disc);

	BraseroDiscResult	(*load_track)		(BraseroDisc *disc,
							 BraseroDiscTrack *track);
	BraseroDiscResult	(*get_track)		(BraseroDisc *disc,
							 BraseroDiscTrack *track);

	BraseroDiscResult	(*set_session_param)	(BraseroDisc *disc,
							 BraseroBurnSession *session);
	BraseroDiscResult	(*set_session_contents)	(BraseroDisc *disc,
							 BraseroBurnSession *session);

	BraseroDiscResult	(*can_add_uri)		(BraseroDisc *disc,
							 const gchar *uri);
	BraseroDiscResult	(*add_uri)		(BraseroDisc *disc,
							 const gchar *uri);

	gboolean		(*get_selected_uri)	(BraseroDisc *disc,
							 gchar **uri);
	gboolean		(*get_boundaries)	(BraseroDisc *disc,
							 gint64 *start,
							 gint64 *end);

	void			(*delete_selected)	(BraseroDisc *disc);
	void			(*clear)		(BraseroDisc *disc);
	void			(*reset)		(BraseroDisc *disc);

	guint			(*add_ui)		(BraseroDisc *disc,
							 GtkUIManager *manager,
							 GtkWidget *message);

	void			(*set_drive)		(BraseroDisc *disc,
							 BraseroDrive *drive);
};

GType brasero_disc_get_type ();

guint
brasero_disc_add_ui (BraseroDisc *disc,
		     GtkUIManager *manager,
		     GtkWidget *message);

BraseroDiscResult
brasero_disc_add_uri (BraseroDisc *disc, const gchar *escaped_uri);

BraseroDiscResult
brasero_disc_can_add_uri (BraseroDisc *disc, const gchar *escaped_uri);

gboolean
brasero_disc_get_selected_uri (BraseroDisc *disc, gchar **uri);

gboolean
brasero_disc_get_boundaries (BraseroDisc *disc,
			     gint64 *start,
			     gint64 *end);

void
brasero_disc_delete_selected (BraseroDisc *disc);
void
brasero_disc_clear (BraseroDisc *disc);
void
brasero_disc_reset (BraseroDisc *disc);

BraseroDiscResult
brasero_disc_get_status (BraseroDisc *disc);

BraseroDiscResult
brasero_disc_get_track (BraseroDisc *disc,
			BraseroDiscTrack *track);
BraseroDiscResult
brasero_disc_load_track (BraseroDisc *disc,
			 BraseroDiscTrack *track);

BraseroDiscResult
brasero_disc_set_session_param (BraseroDisc *disc,
				BraseroBurnSession *session);
BraseroDiscResult
brasero_disc_set_session_contents (BraseroDisc *disc,
				   BraseroBurnSession *session);

void
brasero_disc_size_changed (BraseroDisc *disc,
			   gint64 size);
void
brasero_disc_flags_changed (BraseroDisc *disc,
			    BraseroBurnFlag flags);
void
brasero_disc_contents_changed (BraseroDisc *disc,
			       gint nb_files);
void
brasero_disc_selection_changed (BraseroDisc *disc);

void
brasero_track_clear (BraseroDiscTrack *track);
void
brasero_track_free (BraseroDiscTrack *track);

void
brasero_disc_set_current_drive (BraseroDisc *disc,
				BraseroDrive *drive);

GtkWidget *
brasero_disc_get_use_info_notebook (void);

#endif /* DISC_H */
