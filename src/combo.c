/*
 * Copyright (C) 2004, Magnus Hjorth
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

#include "combo.h"

enum { CHANGED_SIGNAL, LAST_SIGNAL };
static guint combo_signals[LAST_SIGNAL] = { 0 };

static gboolean updating_flag = FALSE;


#ifdef COMBO_OLDSCHOOL

static void combo_class_init(GtkObjectClass *klass)
{
     COMBO_CLASS(klass)->selection_changed = NULL;
     combo_signals[CHANGED_SIGNAL] = 
	  gtk_signal_new("selection_changed",GTK_RUN_LAST,
			 GTK_CLASS_TYPE(klass),
			 GTK_SIGNAL_OFFSET(ComboClass,selection_changed),
			 gtk_marshal_NONE__NONE,GTK_TYPE_NONE,0);
     gtk_object_class_add_signals(klass,combo_signals,LAST_SIGNAL);
}


static void list_select_child(GtkList *list, GtkWidget *child, 
			      gpointer user_data)
{
     Combo *combo = COMBO(user_data);     
     if (updating_flag) return;
     combo->chosen_index = gtk_list_child_position(list,child);
     gtk_signal_emit(GTK_OBJECT(combo),combo_signals[CHANGED_SIGNAL]);
}

static void combo_init(GtkObject *obj)
{
     GtkCombo *cbo = GTK_COMBO(obj);
     COMBO(obj)->chosen_index = 0;
     gtk_editable_set_editable(GTK_EDITABLE(cbo->entry),FALSE);     
     gtk_signal_connect(GTK_OBJECT(cbo->list),"select_child",list_select_child,
			obj);
}

void combo_set_items(Combo *combo, GList *item_strings, int default_index)
{
     updating_flag = TRUE;
     gtk_combo_set_popdown_strings(GTK_COMBO(combo),item_strings);
     if (default_index < g_list_length(item_strings) && default_index >= 0) {
	  gtk_entry_set_text(GTK_ENTRY(GTK_COMBO(combo)->entry),
			     (gchar *)g_list_nth_data(item_strings,
						      default_index));
	  combo->chosen_index = default_index;
     } else
	  combo->chosen_index = 0;
     updating_flag = FALSE;
     gtk_signal_emit(GTK_OBJECT(combo),combo_signals[CHANGED_SIGNAL]);
}

void combo_set_selection(Combo *combo, int item_index)
{
     gtk_list_select_item(GTK_LIST(GTK_COMBO(combo)->list),item_index);
}

int combo_selected_index(Combo *combo)
{
     return combo->chosen_index;
}

char *combo_selected_string(Combo *combo)
{
     return g_strdup(gtk_entry_get_text(GTK_ENTRY(GTK_COMBO(combo)->entry)));
}

void combo_remove_item(Combo *combo, int item_index)
{
     g_assert(item_index != combo->chosen_index);
     gtk_list_clear_items(GTK_LIST(GTK_COMBO(combo)->list),item_index,
			  item_index+1);
     if (item_index < combo->chosen_index) combo->chosen_index--;
}

gboolean combo_mouse_pressed(Combo *combo)
{
     return GTK_WIDGET_VISIBLE(GTK_COMBO(combo)->popwin);
}

#else

static GtkObjectClass *parent_class;


static void combo_destroy(GtkObject *obj)
{
     Combo *combo = COMBO(obj);
     if (combo->strings) {
	  g_list_foreach(combo->strings, (GFunc)g_free, NULL);
	  g_list_free(combo->strings);
	  combo->strings = NULL;
     }
     parent_class->destroy(obj);
}

static void combo_changed(GtkComboBox *combo)
{
     if (!updating_flag)
	  gtk_signal_emit(GTK_OBJECT(combo),combo_signals[CHANGED_SIGNAL]);
     if (GTK_COMBO_BOX_CLASS(parent_class)->changed)
	  GTK_COMBO_BOX_CLASS(parent_class)->changed(combo);
}

static void combo_class_init(GtkObjectClass *klass)
{
     parent_class = gtk_type_class(COMBO_PARENT_TYPE_FUNC());
     klass->destroy = combo_destroy;
     GTK_COMBO_BOX_CLASS(klass)->changed = combo_changed;
     COMBO_CLASS(klass)->selection_changed = NULL;
     combo_signals[CHANGED_SIGNAL] = 
	  gtk_signal_new("selection_changed",GTK_RUN_LAST,
			 GTK_CLASS_TYPE(klass),
			 GTK_SIGNAL_OFFSET(ComboClass,selection_changed),
			 gtk_marshal_NONE__NONE,GTK_TYPE_NONE,0);
     gtk_object_class_add_signals(klass,combo_signals,LAST_SIGNAL);     
}


static void combo_init(GtkObject *obj)
{
     /* Most of this was taken from the code for gtk_combo_box_new_text in 
      * GTK+ 2.4.13 */
  GtkWidget *combo_box = GTK_WIDGET(obj);
  GtkCellRenderer *cell;
  GtkListStore *store;
    
  store = gtk_list_store_new (1, G_TYPE_STRING);
  gtk_combo_box_set_model (GTK_COMBO_BOX(combo_box),GTK_TREE_MODEL (store));
  g_object_unref (store);

  cell = gtk_cell_renderer_text_new ();
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (combo_box), cell, TRUE);
  gtk_cell_layout_set_attributes (GTK_CELL_LAYOUT (combo_box), cell,
                                  "text", 0,
                                  NULL);

  COMBO(obj)->strings = NULL;     
}

void combo_set_items(Combo *combo, GList *item_strings, int default_index)
{
     GList *l;
     gchar *c;
     int len;
     updating_flag = TRUE;
     for (l=combo->strings; l!=NULL; l=l->next) {
	  g_free(l->data);
	  gtk_combo_box_remove_text(GTK_COMBO_BOX(combo),0);
     }
     g_list_free(combo->strings);
     combo->strings = NULL;
     for (l=item_strings,len=0; l!=NULL; l=l->next,len++) {
	  c = (gchar *)l->data;
	  gtk_combo_box_append_text(GTK_COMBO_BOX(combo),c);
	  combo->strings = g_list_append(combo->strings,g_strdup(c));
     }
     if (default_index >= len || default_index < 0) default_index = 0;
     gtk_combo_box_set_active(GTK_COMBO_BOX(combo),default_index);
     updating_flag = FALSE;
     gtk_signal_emit(GTK_OBJECT(combo),combo_signals[CHANGED_SIGNAL]);
}

void combo_set_selection(Combo *combo, int item_index)
{
     gtk_combo_box_set_active(GTK_COMBO_BOX(combo),item_index);
}

int combo_selected_index(Combo *combo)
{
     return gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
}

char *combo_selected_string(Combo *combo)
{
     int i; 
     char *c;
     i = combo_selected_index(combo);
     /* j = g_list_length(combo->strings);
	printf("combo_selected_string: i=%d, len=%d\n",i,j); */
     c = g_strdup(g_list_nth_data(combo->strings,i));
     /* printf("                       c=%s\n",c); */
     return c;
}

void combo_remove_item(Combo *combo, int item_index)
{
     int i;
     gchar *c;
     i = combo_selected_index(combo);
     g_assert(i != item_index);
     gtk_combo_box_remove_text(GTK_COMBO_BOX(combo),item_index);
     c = (gchar *)g_list_nth_data(combo->strings,item_index);
     /* printf("Removing:  selected_index %d, string %s\n",i,c); */
     combo->strings = g_list_remove(combo->strings,c);
     g_free(c);
}

#endif





GtkType combo_get_type(void)
{
     static GtkType id = 0;
     if (!id) {
	  GtkTypeInfo info = {
	       "Combo",
	       sizeof(Combo),
	       sizeof(ComboClass),
	       (GtkClassInitFunc)combo_class_init,
	       (GtkObjectInitFunc)combo_init
	  };
	  id = gtk_type_unique(COMBO_PARENT_TYPE_FUNC(),&info);
     }
     return id;
}

GtkWidget *combo_new(void)
{
     return (GtkWidget *)gtk_type_new(combo_get_type());
}
