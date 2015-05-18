/* cdplay.h
 *
 * Copyright (c) 1998-2002  Mike Oliphant <oliphant@gtk.org>
 *
 *   http://www.nostatic.org/grip
 *
 * This program is free software; you can redistribute it and/or 
 * modify it under the terms of the GNU General Public License as 
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
 * USA
 */

#ifndef GRIP_CDPLAY_H
#define GRIP_CDPLAY_H

#include "grip.h"

/* Time display modes */
#define TIME_MODE_TRACK 0
#define TIME_MODE_DISC 1
#define TIME_MODE_LEFT_TRACK 2
#define TIME_MODE_LEFT_DISC 3

/* Play mode types */
#define PM_NORMAL 0
#define PM_SHUFFLE 1
#define PM_PLAYLIST 2
#define PM_LASTMODE 3

/* Some shortcuts */
#define PREV_TRACK (ginfo->tracks_prog[ginfo->current_track_index - 1])
#define CURRENT_TRACK (ginfo->tracks_prog[ginfo->current_track_index])
#define NEXT_TRACK (ginfo->tracks_prog[ginfo->current_track_index + 1])

void SetCurrentTrackIndex(GripInfo *ginfo,int track);
void SetChecked(GripGUI *uinfo,int track,gboolean checked);
gboolean TrackIsChecked(GripGUI *uinfo,int track);
void EjectDisc(GtkWidget *widget,gpointer data);
void PlaySegment(GripInfo *ginfo,int track);
void FastFwd(GripInfo *ginfo);
void Rewind(GripInfo *ginfo);
void LookupDisc(GripInfo *ginfo,gboolean manual);
void DoLookup(void *data);
gboolean DiscDBLookupDisc(GripInfo *ginfo,DiscDBServer *server);
GtkWidget *MakePlayOpts(GripInfo *ginfo);
GtkWidget *MakeControls(GripInfo *ginfo);
void MakeTrackPage(GripInfo *ginfo);
void NextTrack(GripInfo *ginfo);
void CheckNewDisc(GripInfo *ginfo);
void UpdateDisplay(GripInfo *ginfo);
void UpdateTracks(GripInfo *ginfo);
void SubmitEntry(gint reply,gpointer data);

#endif /* ifndef GRIP_CDPLAY_H */
