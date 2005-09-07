/*
 * Anuradha Ratnaweera (anuradha at gnu org)
 *
 * Copyright (C) 2004 by Anuradha Ratnaweera
 *
 * GTK+ Sinhala input method (transliterated)
 * http://sinhala.linux.lk
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtkimcontext.h>
#include <gtk/gtkimmodule.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <gtk/gtk.h>
#include <string.h>

/* A BIG NOTE:  Instead of working with three byte UTF-8 encoded
 * characters, we will be using the 8 least significant bits of each
 * character (including joiners).  So, ayanna will be 0x85 and so on.
 * Sinhala code page will need characters 128 to 255.
 *
 * This leaves us characters 0 to 127.  First 32 is used for control
 * characters. Only 0x0c and 0x0d is used which is also happened to be
 * the least significant bits of ZWNJ and ZWJ respectively.
 *
 * Values between 32 and 127 (0x20 to 0x7f) inclusive are used for
 * character classes.
 *
 * In the present context, we will be calling this LSB representation. */

/* FIXME: the following #definitions should go into imsinhala.h */

static GType type_sinhala = 0;




static void
sinhala_transliterated_reset(GtkIMContext *context)
{
	//gtk_im_context_get_surrounding(context, &editing_text, &cursor_ptr);
	/* do we need to check editing_text for not NULL */
	//g_signal_emit_by_name(context, "preedit-changed");
}


/* FIXME : made this non-static to avoid a warning, change later */

void     
sinhala_transliterated_get_preedit_string(GtkIMContext *context,
                           gchar **str,
                           PangoAttrList **attrs,
                           gint *cursor_pos)
{
}

static gboolean
sinhala_transliterated_filter_keypress(GtkIMContext *context,
                        GdkEventKey  *event)
{
	int c, c1, l1;
	guchar *u = NULL;
	gchar *text;
	gint cursor, has_surrounding;

	if (event->keyval == 0) return FALSE;

	if (event->type == GDK_KEY_RELEASE) {
		if (event->keyval == GDK_Shift_L) shift_l = 0;
		if (event->keyval == GDK_Shift_R) shift_r = 0;
		if (event->keyval == GDK_Control_L) ctrl_l = 0;
		if (event->keyval == GDK_Control_R) ctrl_r = 0;
		return FALSE;
	}

	if (event->keyval == GDK_Shift_L) {
		shift_l = 1;
		return FALSE;
	}
	if (event->keyval == GDK_Shift_R) {
		shift_r = 1;
		return FALSE;
	}
	if (event->keyval == GDK_Control_L) {
		ctrl_l = 1;
		return FALSE;
	}
	if (event->keyval == GDK_Control_R) {
		ctrl_r = 1;
		return FALSE;
	}

	if ((event->keyval == GDK_space) && (shift_l || shift_r)) {
		/* FIXME: add non breaking space */
		return TRUE;
	}

	if ((event->keyval == GDK_space) && (ctrl_l || ctrl_r)) {
		sinhala_input = !sinhala_input;
		return TRUE;
	}

	if (event->state & (gtk_accelerator_get_default_mod_mask() & ~GDK_SHIFT_MASK))
		return FALSE;

	if (!sinhala_input && (event->keyval < 128)) {
		u = malloc(2);
		u[0] = event->keyval;
		u[1] = 0;

		g_signal_emit_by_name (context, "commit", u);

		free(u);
		return TRUE;
	}

	has_surrounding = gtk_im_context_get_surrounding(context, &text, &cursor);
	c = find_consonent_by_key(event->keyval);

	if (c >= 0) { /* a consonent is pressed. */

		/* do modifiers first. */
		if (has_surrounding && (cursor >= 3)) {
			c1 = get_known_lsb_character(text + cursor - 3);
			l1 = find_consonent(c1);
			/* do modifiers only if there is a valid character before */
			if (l1 >= 0) {
				if (event->keyval == GDK_w) {
					u = create_unicode_character_from_lsb(0xca);
					g_signal_emit_by_name (context, "commit", u);
					free(u);
					free(text);
					return TRUE;
				}
				if (event->keyval == GDK_W) {
					/* bandi hal kireema */
					u = malloc(7);
					u[0] = 0xe0; u[1] = 0xb7; u[2] = 0x8a;
					u[3] = 0xe2; u[4] = 0x80; u[5] = 0x8d;
					u[6] = 0;

					g_signal_emit_by_name (context, "commit", u);

					free(u);
					free(text);
					return TRUE;
				}
				if ((event->keyval == GDK_H) && (consonents[l1].mahaprana)) {
					gtk_im_context_delete_surrounding(context, -1, 1);
					u = create_unicode_character_from_lsb(consonents[l1].mahaprana);
					g_signal_emit_by_name (context, "commit", u);
					free(u);
					free(text);
					return TRUE;
				}
				if ((event->keyval == GDK_G) && (consonents[l1].sagngnaka)) {
					gtk_im_context_delete_surrounding(context, -1, 1);
					u = create_unicode_character_from_lsb(consonents[l1].sagngnaka);
					g_signal_emit_by_name (context, "commit", u);
					free(u);
					free(text);
					return TRUE;
				}
				if (event->keyval == GDK_R) {
					/* rakaraanshaya */
					u = malloc(10);
					u[0] = 0xe0; u[1] = 0xb7; u[2] = 0x8a;
					u[3] = 0xe2; u[4] = 0x80; u[5] = 0x8d;
					u[6] = 0xe0; u[7] = 0xb6; u[8] = 0xbb;
					u[9] = 0;

					g_signal_emit_by_name (context, "commit", u);

					free(u);
					free(text);
					return TRUE;
				}
				if (event->keyval == GDK_Y) {
					/* yansaya */
					u = malloc(10);
					u[0] = 0xe0; u[1] = 0xb7; u[2] = 0x8a;
					u[3] = 0xe2; u[4] = 0x80; u[5] = 0x8d;
					u[6] = 0xe0; u[7] = 0xb6; u[8] = 0xba;
					u[9] = 0;

					g_signal_emit_by_name (context, "commit", u);

					free(u);
					free(text);
					return TRUE;
				}
			}
		}

		u = create_unicode_character_from_lsb(consonents[c].character);
         	g_signal_emit_by_name (context, "commit", u);
		free(u);
		if (has_surrounding) free(text);
		return TRUE;
		/* end of consonent handling. */
	}

	c = find_vowel_by_key(event->keyval);
	if (c >= 0) {
		/* a vowel is pressed. */

		/* look for a previous character first. */
		u = NULL;
		if (has_surrounding && (cursor >= 3)) {
			c1 = get_known_lsb_character(text + cursor - 3);
			if (is_consonent(c1)) {
				u = create_unicode_character_from_lsb(vowels[c].single1);
			}
			else if (c1 == vowels[c].single0) {
				gtk_im_context_delete_surrounding(context, -1, 1);
				u = create_unicode_character_from_lsb(vowels[c].double0);
			}
			else if (c1 == vowels[c].single1) {
				gtk_im_context_delete_surrounding(context, -1, 1);
				u = create_unicode_character_from_lsb(vowels[c].double1);
			}
		}

		if (u == NULL)
			u = create_unicode_character_from_lsb(vowels[c].single0);

		g_signal_emit_by_name (context, "commit", u);

		free(u);
		if (has_surrounding) free(text);
		return TRUE;
		/* end of vowel handling. */
	}
	if (event->keyval < 128) {
		u = malloc(2);
		u[0] = event->keyval;
		u[1] = 0;
		g_signal_emit_by_name (context, "commit", u);
		free(u);
		if (has_surrounding) free(text);
		return TRUE;
	}
	if (event->keyval == GDK_BackSpace) {
	    gtk_im_context_delete_surrounding(context, -1, 1);
	    if (has_surrounding) free(text);
	    return TRUE;
	}
	if (has_surrounding) free(text);

	return FALSE;
}

static void
sinhala_transliterated_class_init(GtkIMContextClass *clazz)
{
	clazz->filter_keypress = sinhala_transliterated_filter_keypress;
//	clazz->get_preedit_string = sinhala_transliterated_get_preedit_string;
	clazz->reset = sinhala_transliterated_reset;
}

void 
im_module_exit()
{
}

static void
sinhala_transliterated_init(GtkIMContext *im_context)
{
}

static void
sinhala_transliterated_register_type(GTypeModule *module)
{
	static const GTypeInfo object_info = {
		sizeof(GtkIMContextClass),
		(GBaseInitFunc)NULL,
		(GBaseFinalizeFunc)NULL,
		(GClassInitFunc)sinhala_transliterated_class_init,
		NULL,           /* class_finalize */
		NULL,           /* class_data */
		sizeof(GtkIMContext),
		0,
		(GInstanceInitFunc)sinhala_transliterated_init,
	};

	type_sinhala = 
		g_type_module_register_type(module, GTK_TYPE_IM_CONTEXT,
				"GtkIMContextSinhalaTransliterated", &object_info, 0);
}

static const
GtkIMContextInfo sinhala_transliterated_info = 
{
	"sinhala-transliterated",     /* ID */
	"Sinhala (Transliterated)",  /* Human readable name */
	GETTEXT_PACKAGE,     /* Translation domain */
	LOCALEDIR,           /* Dir for bindtextdomain */
	"si",                /* Languages for which this module is the default */
};

static const
GtkIMContextInfo *info_list[] = 
{
	&sinhala_transliterated_info,
};

void
im_module_init(GTypeModule *module)
{
	sinhala_transliterated_register_type(module);
}

void 
im_module_list(const GtkIMContextInfo ***contexts, gint *n_contexts)
{
	*contexts = info_list;
	*n_contexts = G_N_ELEMENTS(info_list);
}

GtkIMContext *
im_module_create(const gchar *context_id)
{
	if (strcmp(context_id, "sinhala-transliterated") == 0)
		return GTK_IM_CONTEXT(g_object_new(type_sinhala, NULL));
	else
		return NULL;
}

