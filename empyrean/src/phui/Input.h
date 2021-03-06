/***************************************************************** phui-cpr beg
 *
 * phui - flexible user interface subsystem
 * phui is (C) Copyright 2002 by
 *      Chad Austin, Josh Carlson, Johnathan Gurley,
 *      Ben Scott, Levi Van Oort
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 * -----------------------------------------------------------------
 * File:          $RCSfile: Input.h,v $
 * Date modified: $Date: 2004-06-05 02:23:23 $
 * Version:       $Revision: 1.5 $
 * -----------------------------------------------------------------
 *
 ************************************************************** phui-cpr-end */
#ifndef PHUI_INPUT_H
#define PHUI_INPUT_H

namespace phui
{
   enum InputButton
   {
      BUTTON_LEFT,
      BUTTON_MIDDLE,
      BUTTON_RIGHT,

      BUTTON_UNKNOWN
   };

   enum InputKey
   {
      KEY_ESCAPE,
      KEY_F1,
      KEY_F2,
      KEY_F3,
      KEY_F4,
      KEY_F5,
      KEY_F6,
      KEY_F7,
      KEY_F8,
      KEY_F9,
      KEY_F10,
      KEY_F11,
      KEY_F12,
      KEY_TILDE,
      KEY_MINUS,
      KEY_EQUALS,
      KEY_BACKSPACE,
      KEY_TAB,
      KEY_A,
      KEY_B,
      KEY_C,
      KEY_D,
      KEY_E,
      KEY_F,
      KEY_G,
      KEY_H,
      KEY_I,
      KEY_J,
      KEY_K,
      KEY_L,
      KEY_M,
      KEY_N,
      KEY_O,
      KEY_P,
      KEY_Q,
      KEY_R,
      KEY_S,
      KEY_T,
      KEY_U,
      KEY_V,
      KEY_W,
      KEY_X,
      KEY_Y,
      KEY_Z,
      KEY_0,
      KEY_1,
      KEY_2,
      KEY_3,
      KEY_4,
      KEY_5,
      KEY_6,
      KEY_7,
      KEY_8,
      KEY_9,
      KEY_SHIFT,
      KEY_CTRL,
      KEY_ALT,
      KEY_SPACE,
      KEY_OPENBRACE,
      KEY_CLOSEBRACE,
      KEY_SEMICOLON,
      KEY_APOSTROPHE,
      KEY_COMMA,
      KEY_PERIOD,
      KEY_SLASH,
      KEY_BACKSLASH,
      KEY_ENTER,
      KEY_INSERT,
      KEY_DELETE,
      KEY_HOME,
      KEY_END,
      KEY_PAGE_UP,
      KEY_PAGE_DOWN,
      KEY_UP,
      KEY_RIGHT,
      KEY_DOWN,
      KEY_LEFT,
      KEY_CAPS_LOCK,
      KEY_NUM_LOCK,
      KEY_SCROLL_LOCK,

      KEY_UNKNOWN
   };
   
   enum
   {
      IMOD_NONE        = 0x00,

      IMOD_CTRL        = 0x01,
      IMOD_ALT         = 0x02,
      IMOD_SHIFT       = 0x04,

      IMOD_CAPS_LOCK   = 0x10,
      IMOD_NUM_LOCK    = 0x20,
      IMOD_SCROLL_LOCK = 0x40,
   };

   // because they're ANDed and ORed together
   typedef int InputModifiers;

}

#endif
