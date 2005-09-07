/* -*- Mode: C; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 *  Copyright (C) Hiroyuki Ikezoe <poincare@ikezoe.net>
 *  Copyright (C) 2005 Kazuki Ohta <mover@hct.zaq.ne.jp>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * The original code is scim_uim_imengine.cpp in scim-uim-0.1.3. 
 * Copyright (C) 2004 James Su <suzhe@tsinghua.org.cn>
 */

#define Uses_SCIM_UTILITY
#define Uses_SCIM_IMENGINE
#define Uses_SCIM_LOOKUP_TABLE
#define Uses_SCIM_CONFIG_BASE

#ifdef HAVE_CONFIG_H
  #include <config.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <scim.h>
#include "scim_sinhala_imengine_factory.h"
#include "scim_sinhala_imengine.h"
//#include "scim_sinhala_prefs.h"
#include "intl.h"

/*
 * A BIG NOTE:  Instead of working with three byte UTF-8 encoded
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
 * In the present context, we will be calling this LSB representation.
 */
/* Sinhala Related Definitions */
#define ZWNJ 0x0c
#define ZWJ  0x0d

static int sinhala_input = 1;
static int shift_r = 0;
static int shift_l = 0;
static int ctrl_r = 0;
static int ctrl_l = 0;

struct {
	unsigned char character;
	unsigned char mahaprana;
	unsigned char sagngnaka;
	int key;
} consonents[] = {
	{0xa4, 0x00, 0x00, SCIM_KEY_q},
	{0xa5, 0x00, 0x00, SCIM_KEY_Q},
	{0xc0, 0x00, 0x00, SCIM_KEY_w},
	{0x0c, 0x00, 0x00, SCIM_KEY_W},
	{0xbb, 0x00, 0x00, SCIM_KEY_r},
	{0xbb, 0x00, 0x00, SCIM_KEY_R},
	{0xad, 0xae, 0x00, SCIM_KEY_t},
	{0xa7, 0xa8, 0x00, SCIM_KEY_T},
	{0xba, 0x00, 0x00, SCIM_KEY_y},
	{0xba, 0x00, 0x00, SCIM_KEY_Y},
	{0xb4, 0xb5, 0x00, SCIM_KEY_p},
	{0xb5, 0xb5, 0x00, SCIM_KEY_P},
	{0xc3, 0xc2, 0x00, SCIM_KEY_s},
	{0xc1, 0xc2, 0x00, SCIM_KEY_S},
	{0xaf, 0xb0, 0xb3, SCIM_KEY_d},
	{0xa9, 0xaa, 0xac, SCIM_KEY_D},
	{0xc6, 0x00, 0x00, SCIM_KEY_f},
	{0xc6, 0x00, 0x00, SCIM_KEY_F},
	{0x9c, 0x9d, 0x9f, SCIM_KEY_g},
	{0x9f, 0x9d, 0x00, SCIM_KEY_G},
	{0xc4, 0x83, 0x00, SCIM_KEY_h},
	{0xc4, 0x00, 0x00, SCIM_KEY_H},
	{0xa2, 0xa3, 0xa6, SCIM_KEY_j},
	{0xa3, 0xa3, 0xa6, SCIM_KEY_J},
	{0x9a, 0x9b, 0x00, SCIM_KEY_k},
	{0x9b, 0x9b, 0x00, SCIM_KEY_K},
	{0xbd, 0x00, 0x00, SCIM_KEY_l},
	{0xc5, 0x00, 0x00, SCIM_KEY_L},
	{0x82, 0x00, 0x00, SCIM_KEY_x},
	{0x9e, 0x00, 0x00, SCIM_KEY_X},
	{0xa0, 0xa1, 0x00, SCIM_KEY_c},
	{0xa1, 0xa1, 0x00, SCIM_KEY_C},
	{0xc0, 0x00, 0x00, SCIM_KEY_v},
	{0xc0, 0x00, 0x00, SCIM_KEY_V},
	{0xb6, 0xb7, 0xb9, SCIM_KEY_b},
	{0xb7, 0xb7, 0xb9, SCIM_KEY_B},
	{0xb1, 0x00, 0x82, SCIM_KEY_n},
	{0xab, 0x00, 0x9e, SCIM_KEY_N},
	{0xb8, 0x00, 0x00, SCIM_KEY_m},
	{0xb9, 0x00, 0x00, SCIM_KEY_M},
	{0, 0, 0, 0}
};

struct {
	unsigned char single0;
	unsigned char double0;
	unsigned char single1;
	unsigned char double1;
	int key;
} vowels[] = {
	{0x85, 0x86, 0xcf, 0xcf, SCIM_KEY_a},
	{0x87, 0x88, 0xd0, 0xd1, SCIM_KEY_A},
	{0x91, 0x92, 0xd9, 0xda, SCIM_KEY_e},
	{0x91, 0x92, 0xd9, 0xda, SCIM_KEY_E},
	{0x89, 0x8a, 0xd2, 0xd3, SCIM_KEY_i},
	{0x93, 0x00, 0xdb, 0xdb, SCIM_KEY_I},
	{0x94, 0x95, 0xdc, 0xdd, SCIM_KEY_o},
	{0x96, 0x00, 0xde, 0xdf, SCIM_KEY_O},
	{0x8b, 0x8c, 0xd4, 0xd6, SCIM_KEY_u},
	{0x8d, 0x8e, 0xd8, 0xf2, SCIM_KEY_U},
	{0x8f, 0x90, 0x8f, 0x90, SCIM_KEY_Z},
	{0, 0, 0, 0, 0}
};

SinhalaInstance::SinhalaInstance (SinhalaFactory   *factory,
                                  const String   &encoding,
                                  int             id)
    : IMEngineInstanceBase (factory, encoding, id),
      m_factory (factory),
      m_prev_key (0,0)
{
    SCIM_DEBUG_IMENGINE(1) << "Create SINHALA Instance : ";
}

SinhalaInstance::~SinhalaInstance ()
{
}

bool
SinhalaInstance::process_key_event (const KeyEvent& key)
{
    SCIM_DEBUG_IMENGINE(2) << "process_key_event.\n";

    return sinhala_transliterated_filter_keypress(key);
}

void
SinhalaInstance::move_preedit_caret (unsigned int pos)
{
}

void
SinhalaInstance::select_candidate (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate.\n";

    select_candidate_no_direct (item);
}

void
SinhalaInstance::select_candidate_no_direct (unsigned int item)
{
    SCIM_DEBUG_IMENGINE(2) << "select_candidate_no_direct.\n";

    m_lookup_table.set_cursor_pos_in_current_page (item);
    update_lookup_table (m_lookup_table);
}

void
SinhalaInstance::update_lookup_table_page_size (unsigned int page_size)
{
    SCIM_DEBUG_IMENGINE(2) << "update_lookup_table_page_size.\n";

    m_lookup_table.set_page_size (page_size);
}

void
SinhalaInstance::lookup_table_page_up ()
{
    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_up.\n";

    m_lookup_table.page_up ();
    update_lookup_table (m_lookup_table);
}

void
SinhalaInstance::lookup_table_page_down ()
{
    SCIM_DEBUG_IMENGINE(2) << "lookup_table_page_down.\n";

    m_lookup_table.page_down ();
    update_lookup_table (m_lookup_table);
}

void
SinhalaInstance::reset ()
{
    SCIM_DEBUG_IMENGINE(2) << "reset.\n";

    m_lookup_table.clear ();
    update_preedit_caret (0);
    update_preedit_string (utf8_mbstowcs (""));
    hide_lookup_table ();
    hide_preedit_string ();
}

void
SinhalaInstance::focus_in ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_in.\n";

    hide_aux_string ();
}

void
SinhalaInstance::focus_out ()
{
    SCIM_DEBUG_IMENGINE(2) << "focus_out.\n";
}

void
SinhalaInstance::trigger_property (const String &property)
{
    String sinhala_prop = property.substr (property.find_last_of ('/') + 1);

    SCIM_DEBUG_IMENGINE(2) << "trigger_property : " << property << " - " << sinhala_prop << "\n";
}

/*
 * Sinhala Related Functions
 */
/*
 * Checks TEXT to see if it starts with a Sinhala Unicode
 * character or a Joiner, and returns the 8 least significant
 * bits of the unicode value.
 *
 * If the returned value XX larger than or equal to 128, it represents
 * the Sinhala character 0x0DXX.
 *
 * If it returns a non-zero value YY smaller than 128, it represents the
 * joiner 0x20YY.  Only "known" joiners, ZWJ and ZWNJ, will return
 * non-zero.
 *
 * Returns 0 for other cases. 
 */
int
SinhalaInstance::get_known_lsb_character(unsigned char *text)
{
	if (*text == 0xe0) {
		if (*++text == 0xb6) return *++text;
		if (*text == 0xb7) return *++text | 0x40;
	}
	if ((*text == 0xe2) && (*++text == 0x80)) {
		if (*++text == 0x8c) return 0x0c;
		if (*text == 0x8d) return 0x0d;
	}
	return 0;
}

/* Returns true if C is a consonent. */
int
SinhalaInstance::is_consonent(int c)
{
    return (c >= 0x9a) && (c <= 0xc6) ? 1 : 0;
}

/*
 * Search the consonent list to see if a key has a corresponding
 * entry. Returns the index in the list or -1 if not found.
 */
int
SinhalaInstance::find_consonent_by_key(int k)
{
   	int i = -1;
	while (consonents[++i].character)
		if (consonents[i].key == k)
		    return i;
	return -1;
}

/*
 * Search the consonent list to see if a character is in one
 * of its sublists.  Returns the index or -1 if not found.
 */
int
SinhalaInstance::find_consonent(int c)
{
	int i = -1;
	while (consonents[++i].character)
		if ((consonents[i].character == c)
            || (consonents[i].mahaprana == c)
            || (consonents[i].sagngnaka == c))
			return i;
	return -1;
}


/*
 * Search the vowel list to see if a key has a corresponding
 * entry. Returns the index in the list or -1 if not found.
 */
int
SinhalaInstance::find_vowel_by_key(int k)
{
	int i = -1;
	while (vowels[++i].single0)
		if (vowels[i].key == k)
			return i;
	return -1;
}

/*
 * Creates a UTF-8 sequence for a given LSB value.
 * IMPORTANT: the returned value must be free()ed.
 */
unsigned char*
SinhalaInstance::create_unicode_character_from_lsb(unsigned int lsb)
{
	unsigned char *u = (unsigned char*)malloc(4);

	if (lsb >= 128) {
		u[0] = 0xe0;
		if (lsb < 192) {
			u[1] = 0xb6;
			u[2] = lsb;
		}
		else {
			u[1] = 0xb7;
			u[2] = (lsb & 0x3f) | 0x80;
		}
		u[3] = 0;
	}
	else if ((lsb == 0x0c) || (lsb == 0x0d)) {
		u[0] = 0xe2;
		u[1] = 0x80;
		u[2] = 0x80 | lsb;
		u[3] = 0;
	}
	else {
		u[0] = 0;
	}

	return u;    
}

bool
SinhalaInstance::sinhala_transliterated_filter_keypress(const KeyEvent &event)
{
	int c;
	unsigned char *u = NULL;
    WideString widetext;
    char *text;
	int cursor, has_surrounding;

	if (event.code == 0)
        return false;

	if (event.is_key_release()) {
		if (event.code == SCIM_KEY_Shift_L) shift_l = 0;
		if (event.code == SCIM_KEY_Shift_R) shift_r = 0;
		if (event.code == SCIM_KEY_Control_L) ctrl_l = 0;
		if (event.code == SCIM_KEY_Control_R) ctrl_r = 0;
		return false;
	}

	if (event.code == SCIM_KEY_Shift_L) {
		shift_l = 1;
		return false;
	}
	if (event.code == SCIM_KEY_Shift_R) {
		shift_r = 1;
		return false;
	}
	if (event.code == SCIM_KEY_Control_L) {
		ctrl_l = 1;
		return false;
	}
	if (event.code == SCIM_KEY_Control_R) {
		ctrl_r = 1;
		return false;
	}

	if ((event.code == SCIM_KEY_space) && (shift_l || shift_r)) {
		/* FIXME: add non breaking space */
		return true;
	}

	if ((event.code == SCIM_KEY_space) && (ctrl_l || ctrl_r)) {
		sinhala_input = !sinhala_input;
		return true;
	}

    if (event.mask && (event.is_lock_down()
                       || event.is_control_down()
                       || event.is_alt_down()
                       || event.is_mod1_down()
                       || event.is_mod2_down()
                       || event.is_mod3_down()
                       || event.is_mod4_down()
                       || event.is_mod5_down()
                       || event.is_num_lock_down()
                       || event.is_caps_lock_down()
                       || event.is_scroll_lock_down()))
        return false;

	if (!sinhala_input && (event.code < 128)) {
		u = (unsigned char*)malloc(2);
		u[0] = event.code;
		u[1] = 0;

        commit_string(utf8_mbstowcs((const char*)u));

		free(u);
		return true;
	}

    has_surrounding = get_surrounding_text(widetext, cursor);
    WideString tmp_wide = widetext.substr (0, cursor);
    String tmp = utf8_wcstombs (tmp_wide);
    cursor = tmp.length ();
    text = const_cast<char*> (utf8_wcstombs(widetext).c_str());

	c = find_consonent_by_key(event.code);
	if (c >= 0) /* a consonent is pressed. */
        return handle_consonant_pressed (event, text, c, cursor);

	c = find_vowel_by_key(event.code);
	if (c >= 0)
        return handle_vowel_pressed (event, text, c, cursor);

	if (event.code < 128) {
		char u[2];
		u[0] = event.code;
		u[1] = 0;
        commit_string(utf8_mbstowcs((const char*)u));
		return true;
	}
	if (event.code == SCIM_KEY_BackSpace) {
        //delete_surrounding_text(-1, 1);
        return false;
	}

	return false;
}

bool
SinhalaInstance::handle_consonant_pressed(const KeyEvent &event,
                                          char *text,
                                          int c, int cursor)
{
	int c1, l1;
	unsigned char *u = NULL;

    /* do modifiers first. */
    if (text && *text && (cursor >= 3)) {
        c1 = get_known_lsb_character((unsigned char*)(text + cursor - 3));
        l1 = find_consonent(c1);
        /* do modifiers only if there is a valid character before */
        if (l1 >= 0) {
            if (event.code == SCIM_KEY_w) {
                u = create_unicode_character_from_lsb(0xca);
                commit_string(utf8_mbstowcs((const char*)u));
                free(u);
                return true;
            }
            if (event.code == SCIM_KEY_W) {
                /* bandi hal kireema */
                unsigned char u[7];
                u[0] = 0xe0; u[1] = 0xb7; u[2] = 0x8a;
                u[3] = 0xe2; u[4] = 0x80; u[5] = 0x8d;
                u[6] = 0;

                commit_string(utf8_mbstowcs((const char*)u));

                return true;
            }
            if ((event.code == SCIM_KEY_H) && (consonents[l1].mahaprana)) {
                delete_surrounding_text(-1, 1);
                u = create_unicode_character_from_lsb(consonents[l1].mahaprana);
                commit_string(utf8_mbstowcs((const char*)u));
                free(u);
                return true;
            }
            if ((event.code == SCIM_KEY_G) && (consonents[l1].sagngnaka)) {
                delete_surrounding_text(-1, 1);
                u = create_unicode_character_from_lsb(consonents[l1].sagngnaka);
                commit_string(utf8_mbstowcs((const char*)u));
                free(u);
                return true;
            }
            if (event.code == SCIM_KEY_R) {
                /* rakaraanshaya */
                unsigned char u[10];
                u[0] = 0xe0; u[1] = 0xb7; u[2] = 0x8a;
                u[3] = 0xe2; u[4] = 0x80; u[5] = 0x8d;
                u[6] = 0xe0; u[7] = 0xb6; u[8] = 0xbb;
                u[9] = 0;

                commit_string(utf8_mbstowcs((const char*)u));

                return true;
            }
            if (event.code == SCIM_KEY_Y) {
                /* yansaya */
                unsigned char u[10];
                u[0] = 0xe0; u[1] = 0xb7; u[2] = 0x8a;
                u[3] = 0xe2; u[4] = 0x80; u[5] = 0x8d;
                u[6] = 0xe0; u[7] = 0xb6; u[8] = 0xba;
                u[9] = 0;

                commit_string(utf8_mbstowcs((const char*)u));

                return true;
            }
        }
    }

    u = create_unicode_character_from_lsb(consonents[c].character);
    commit_string(utf8_mbstowcs((const char*)u));
    free(u);

    return true;
}

bool
SinhalaInstance::handle_vowel_pressed(const KeyEvent &event,
                                      char *text,
                                      int c, int cursor)
{
	int c1;
	unsigned char *u = NULL;

    /* look for a previous character first. */
    u = NULL;
    if (text && *text && (cursor >= 3)) {
        c1 = get_known_lsb_character((unsigned char*)(text + cursor - 3));
        if (is_consonent(c1)) {
            u = create_unicode_character_from_lsb(vowels[c].single1);
        }
        else if (c1 == vowels[c].single0) {
            delete_surrounding_text(-1, 1);
            u = create_unicode_character_from_lsb(vowels[c].double0);
        }
        else if (c1 == vowels[c].single1) {
            delete_surrounding_text(-1, 1);
            u = create_unicode_character_from_lsb(vowels[c].double1);
        }
    }

    if (u == NULL)
        u = create_unicode_character_from_lsb(vowels[c].single0);

    commit_string(utf8_mbstowcs((const char*)u));

    free(u);

    return true;
}

/*
vi:ts=4:nowrap:ai:expandtab
*/
