/*
 * Copyright (C) 2003 2004, Magnus Hjorth
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


#include <config.h>
#include <gdk/gdkkeysyms.h>

#include "gotodialog.h"
#include "inifile.h"
#include "main.h"
#include "gettext.h"

static gboolean goto_dialog_apply(GotoDialog *gd)
{
     off_t p=0,q;
     off_t o;
     ChunkView *v = gd->mw->view;
     int i;
     if (gd->mw->view->chunk == NULL) return FALSE;
     if (floatbox_check(gd->offset)) return TRUE;
     for (i=0; i<5; i++)
	  if (gtk_toggle_button_get_active(gd->relbuttons[i]))
	       break;
     switch (i) {
     case 0: p = 0; break;
     case 1: p = v->chunk->length-1; break;
     case 2: p = v->cursorpos; break;
     case 3: p = (v->selstart==v->selend)?0:v->selstart; break;
     case 4: 
	  p = (v->selstart==v->selend)?v->chunk->length-1:v->selend-1; 
	  break;
     default: g_assert_not_reached();
     }
     o = ((float)(v->chunk->format.samplerate))*gd->offset->val;
     q = p+o;
     if (q>=v->chunk->length) q=(o<0)?0:v->chunk->length-1;
     
     mainwindow_position_cursor(gd->mw,q);

     inifile_set_gfloat("gotoOffset",gd->offset->val);
     inifile_set_guint32("gotoRel",i);
     return FALSE;
}

static void goto_dialog_ok(GtkButton *button, GotoDialog *gd)
{
     if (!goto_dialog_apply(gd))
	  gtk_widget_destroy(GTK_WIDGET(gd));
}

static void goto_dialog_init(GotoDialog *gd)
{
     GtkWidget *a,*b,*c;
     guint32 i;
     GtkAccelGroup* ag;

     ag = gtk_accel_group_new();

     a = gtk_vbox_new(FALSE,0);
     gtk_container_set_border_width(GTK_CONTAINER(a),8);
     gtk_container_add(GTK_CONTAINER(gd),a);
     b = gtk_hbox_new(FALSE,5);
     gtk_box_pack_start(GTK_BOX(a),b,FALSE,FALSE,3);
     c = gtk_label_new(_("Place cursor "));
     gtk_box_pack_start(GTK_BOX(b),c,FALSE,FALSE,0);
     c = floatbox_new(inifile_get_gfloat("gotoOffset",0.0));
     gd->offset = FLOATBOX(c);
     gtk_box_pack_start(GTK_BOX(b),c,FALSE,FALSE,0);
     c = gtk_label_new(_(" seconds"));
     gtk_box_pack_start(GTK_BOX(b),c,FALSE,FALSE,0);
     b = gtk_radio_button_new_with_label(NULL,_("after beginning of file"));
     gd->relbuttons[0] = GTK_TOGGLE_BUTTON(b);
     gtk_box_pack_start(GTK_BOX(a),b,FALSE,FALSE,0);
     b = gtk_radio_button_new_with_label_from_widget(
	  GTK_RADIO_BUTTON(b),_("after end of file"));
     gd->relbuttons[1] = GTK_TOGGLE_BUTTON(b);
     gtk_box_pack_start(GTK_BOX(a),b,FALSE,FALSE,0);
     b = gtk_radio_button_new_with_label_from_widget(
	  GTK_RADIO_BUTTON(b),_("after current cursor position"));
     gd->relbuttons[2] = GTK_TOGGLE_BUTTON(b);
     gtk_box_pack_start(GTK_BOX(a),b,FALSE,FALSE,0);
     b = gtk_radio_button_new_with_label_from_widget(
	  GTK_RADIO_BUTTON(b),_("after selection start"));
     gd->relbuttons[3] = GTK_TOGGLE_BUTTON(b);
     gtk_box_pack_start(GTK_BOX(a),b,FALSE,FALSE,0);
     b = gtk_radio_button_new_with_label_from_widget(
	  GTK_RADIO_BUTTON(b),_("after selection end"));
     gd->relbuttons[4] = GTK_TOGGLE_BUTTON(b);
     gtk_box_pack_start(GTK_BOX(a),b,FALSE,FALSE,0);
     b = gtk_label_new(_("(use a negative number to place the cursor before "
		       "instead of after the selected point)"));
     gtk_label_set_justify(GTK_LABEL(b),GTK_JUSTIFY_LEFT);
     gtk_label_set_line_wrap(GTK_LABEL(b),TRUE);
     gtk_box_pack_start(GTK_BOX(a),b,FALSE,FALSE,0);
     b = gtk_hbutton_box_new();
     gtk_box_pack_end(GTK_BOX(a),b,FALSE,FALSE,0);
     c = gtk_button_new_with_label(_("OK"));
     gtk_signal_connect(GTK_OBJECT(c),"clicked",
			GTK_SIGNAL_FUNC(goto_dialog_ok),gd);
     gtk_container_add(GTK_CONTAINER(b),c);
     gtk_widget_add_accelerator (c, "clicked", ag, GDK_KP_Enter, 0, 
				 (GtkAccelFlags) 0);
     gtk_widget_add_accelerator (c, "clicked", ag, GDK_Return, 0, 
				 (GtkAccelFlags) 0);
     c = gtk_button_new_with_label(_("Apply"));
     gtk_signal_connect_object(GTK_OBJECT(c),"clicked",
			       GTK_SIGNAL_FUNC(goto_dialog_apply),
			       GTK_OBJECT(gd));
     gtk_container_add(GTK_CONTAINER(b),c);
     c = gtk_button_new_with_label(_("Close"));
     gtk_signal_connect_object(GTK_OBJECT(c),"clicked",
			       GTK_SIGNAL_FUNC(gtk_widget_destroy),
			       GTK_OBJECT(gd));
     gtk_container_add(GTK_CONTAINER(b),c);
     gtk_widget_add_accelerator (c, "clicked", ag, GDK_Escape, 0, 
				 (GtkAccelFlags) 0);
     b = gtk_hseparator_new();
     gtk_box_pack_end(GTK_BOX(a),b,FALSE,FALSE,0);

     i = inifile_get_guint32("gotoRel",0);
     if (i>4) i=0;
     gtk_toggle_button_set_active(gd->relbuttons[i],TRUE);

     gtk_widget_show_all(a);
     gtk_window_set_title(GTK_WINDOW(gd),_("Position cursor"));
     gtk_window_add_accel_group(GTK_WINDOW (gd), ag);
     gtk_window_set_position (GTK_WINDOW (gd), GTK_WIN_POS_CENTER);
}

static void goto_dialog_class_init(GtkObjectClass *klass)
{
}

GtkType goto_dialog_get_type(void)
{
     static GtkType id = 0;
     if (!id) {
	  GtkTypeInfo info = {
	       "GotoDialog",
	       sizeof(GotoDialog),
	       sizeof(GotoDialogClass),
	       (GtkClassInitFunc) goto_dialog_class_init,
	       (GtkObjectInitFunc) goto_dialog_init
	  };
	  id = gtk_type_unique(gtk_window_get_type(),&info);
     }
     return id;
}

GtkWidget *goto_dialog_new(Mainwindow *mw)
{     
     GtkWidget *w;
     w = gtk_type_new(goto_dialog_get_type());
     GOTO_DIALOG(w)->mw = mw;
     return w;
}
