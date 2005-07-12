/*
 * Copyright (C) 2002 2003 2004, Magnus Hjorth
 *
 * This file is part of mhWaveEdit.
 *
 * mhWaveEdit is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by        
 * the Free Software Foundation; either version 2 of the License, or  
 * (at your option) any later version.
 *
 * mhWaveEdit is distributed in the hope that it will be useful,   
 * but WITHOUT ANY WARRANTY; without even the implied warranty of  
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with mhWaveEdit; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307
 */


#ifndef RECORDDIALOG_H_INCLUDED
#define RECORDDIALOG_H_INCLUDED

#include <gtk/gtk.h>
#include "chunk.h"
#include "int_box.h"
#include "datasource.h"
#include "vu_meter.h"
#include "ringbuf.h"
#include "tempfile.h"
#include "combo.h"

#define RECORD_DIALOG(obj) GTK_CHECK_CAST(obj, record_dialog_get_type(), RecordDialog)
#define RECORD_DIALOG_CLASS(klass) GTK_CHECK_CLASS_CAST(klass,record_dialog_get_type(),RecordDialogClass)
#define IS_RECORD_DIALOG(obj) GTK_CHECK_TYPE(obj, record_dialog_get_type())

typedef struct {
     gint num;
     gchar *name;
     Dataformat fmt;
} RecordFormat;


typedef struct {

     GtkWindow window;

     GList *formats;
     GList *format_strings;
     /* This string is set for a custom format that is not saved under a name.
      * build_format_string will place it last in the format combo */
     Combo *format_combo;
     /* If this is TRUE, the element 0 in the format_combo is the 
      * "Please choose a format" string */
     gboolean format_combo_fresh;
     gboolean format_changed;
     GtkBin *vu_frame;
     GtkLabel *status_label,*time_label,*overruns_label,*overruns_title;
     GtkLabel *bytes_label,*bytes_text_label,*limit_label,*limit_text_label;
     GtkWidget *record_button,*reset_button,*close_button;     

     RecordFormat *current_format;
     gboolean limit_record;
     GtkEntry *limit_entry;
     GtkLabel *limit_set_label;
     GtkButton *disable_limit_button;
     gfloat limit_seconds;
     float maxpeak_values[8];
     GtkLabel **peak_labels,**maxpeak_labels,**clip_labels,**rms_labels;
     VuMeter **meters;

     TempFile *tf;
     gboolean paused;
     off_t written_bytes;
     off_t limit_bytes;
     Ringbuf *databuf;
     guint32 analysis_bytes;
     guint32 analysis_samples;
     gchar *analysis_buf;
     sample_t *analysis_sbuf;
     guint overruns_before_start;

} RecordDialog;

typedef struct {
     GtkWindowClass wc;
} RecordDialogClass;

GtkType record_dialog_get_type(void);
Chunk *record_dialog_execute(void);

#endif
